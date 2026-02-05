#include "include/SyncManager.h"
#include "../core/include/AppPaths.h"
#include "../core/include/LibraryModel.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHostAddress>
#include <QHostInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>
#include <QSet>
#include <QSharedPointer>
#include <QSettings>
#include <QStandardPaths>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QUdpSocket>
#include <QUuid>

namespace {
QString resolveSyncSettingsPath() {
  return AppPaths::configFile("sync.ini");
}

QString normalizeAddress(const QString &address) {
  if (address.startsWith("::ffff:")) {
    return address.mid(7);
  }
  return address;
}

QString randomPin() {
  const int pin = QRandomGenerator::global()->bounded(1000, 9999);
  return QString::number(pin);
}
} // namespace

SyncManager::SyncManager(QObject *parent) : QObject(parent) {
  m_settings = new QSettings(resolveSyncSettingsPath(), QSettings::IniFormat, this);
  loadSettings();
  ensureUdpSocket();
  ensureServer();

  m_discoveryTimer = new QTimer(this);
  m_discoveryTimer->setInterval(4000);
  connect(m_discoveryTimer, &QTimer::timeout, this, [this]() { sendDiscovery(); });

  m_pruneTimer = new QTimer(this);
  m_pruneTimer->setInterval(5000);
  connect(m_pruneTimer, &QTimer::timeout, this, [this]() { pruneDevices(); });
  m_pruneTimer->start();
}

bool SyncManager::enabled() const { return m_enabled; }

void SyncManager::setEnabled(bool enabled) {
  if (m_enabled == enabled) {
    return;
  }
  m_enabled = enabled;
  saveSettings();
  emit enabledChanged();
  if (m_enabled) {
    ensureUdpSocket();
    ensureServer();
  } else {
    stopDiscovery();
    if (m_server && m_server->isListening()) {
      m_server->close();
    }
    if (m_udp) {
      m_udp->close();
    }
  }
}

QString SyncManager::status() const { return m_status; }

QString SyncManager::deviceName() const { return m_deviceName; }

QString SyncManager::deviceId() const { return m_deviceId; }

QString SyncManager::pin() const { return m_pin; }

int SyncManager::discoveryPort() const { return m_discoveryPort; }

int SyncManager::listenPort() const { return m_listenPort; }

bool SyncManager::discovering() const { return m_discovering; }

QVariantList SyncManager::devices() const {
  QVariantList list;
  list.reserve(m_devices.size());
  for (const auto &device : m_devices) {
    QVariantMap entry;
    entry.insert("id", device.id);
    entry.insert("name", device.name);
    entry.insert("address", device.address);
    entry.insert("port", device.port);
    entry.insert("paired", device.paired);
    entry.insert("lastSeen", QDateTime::fromMSecsSinceEpoch(device.lastSeen).toString(Qt::ISODate));
    entry.insert("lastSync", device.lastSync);
    list.append(entry);
  }
  return list;
}

QObject *SyncManager::libraryModel() const {
  return m_libraryModel;
}

QString SyncManager::conflictPolicy() const { return m_conflictPolicy; }
bool SyncManager::transferEnabled() const { return m_transferEnabled; }
int SyncManager::transferMaxMb() const { return m_transferMaxMb; }
bool SyncManager::transferActive() const { return m_transferActive; }
int SyncManager::transferTotal() const { return m_transferTotal; }
int SyncManager::transferDone() const { return m_transferDone; }
bool SyncManager::uploadActive() const { return m_uploadActive; }
int SyncManager::uploadTotal() const { return m_uploadTotal; }
int SyncManager::uploadDone() const { return m_uploadDone; }

void SyncManager::setDeviceName(const QString &name) {
  const QString trimmed = name.trimmed();
  if (trimmed.isEmpty() || m_deviceName == trimmed) {
    return;
  }
  m_deviceName = trimmed;
  saveSettings();
  emit deviceNameChanged();
}

void SyncManager::setPin(const QString &pin) {
  const QString trimmed = pin.trimmed();
  if (trimmed.isEmpty() || m_pin == trimmed) {
    return;
  }
  m_pin = trimmed;
  saveSettings();
  emit pinChanged();
}

void SyncManager::setDiscoveryPort(int port) {
  if (port < 1024 || port > 65535 || m_discoveryPort == port) {
    return;
  }
  m_discoveryPort = port;
  saveSettings();
  emit discoveryPortChanged();
  if (m_udp) {
    m_udp->close();
    m_udp->deleteLater();
    m_udp = nullptr;
  }
  if (m_enabled) {
    ensureUdpSocket();
  }
}

void SyncManager::setListenPort(int port) {
  if (port < 1024 || port > 65535 || m_listenPort == port) {
    return;
  }
  m_listenPort = port;
  saveSettings();
  emit listenPortChanged();
  if (m_server && m_server->isListening()) {
    m_server->close();
  }
  ensureServer();
}

void SyncManager::setLibraryModel(QObject *model) {
  if (m_libraryModel == model) {
    return;
  }
  m_libraryModel = qobject_cast<LibraryModel *>(model);
  emit libraryModelChanged();
}

void SyncManager::setConflictPolicy(const QString &policy) {
  const QString normalized = policy.trimmed().toLower();
  if (normalized.isEmpty() || m_conflictPolicy == normalized) {
    return;
  }
  m_conflictPolicy = normalized;
  saveSettings();
  emit conflictPolicyChanged();
}

void SyncManager::setTransferEnabled(bool enabled) {
  if (m_transferEnabled == enabled) {
    return;
  }
  m_transferEnabled = enabled;
  saveSettings();
  emit transferEnabledChanged();
}

void SyncManager::setTransferMaxMb(int mb) {
  if (mb < 1 || mb > 1024 || m_transferMaxMb == mb) {
    return;
  }
  m_transferMaxMb = mb;
  saveSettings();
  emit transferMaxMbChanged();
}

void SyncManager::startDiscovery() {
  qInfo() << "SyncManager: start discovery" << "enabled" << m_enabled
          << "discovering" << m_discovering << "port" << m_discoveryPort;
  if (!m_enabled) {
    setStatus("Sync disabled");
    return;
  }
  setDiscovering(true);
  ensureUdpSocket();
  sendDiscovery();
  m_discoveryTimer->start();
  setStatus("Discovering devices...");
}

void SyncManager::stopDiscovery() {
  qInfo() << "SyncManager: stop discovery";
  setDiscovering(false);
  if (m_discoveryTimer) {
    m_discoveryTimer->stop();
  }
  setStatus("Idle");
}

void SyncManager::requestPairing(const QString &deviceId) {
  if (!m_enabled) {
    setStatus("Sync disabled");
    return;
  }
  if (deviceId.trimmed().isEmpty()) {
    setStatus("Invalid device");
    return;
  }
  if (!m_devices.contains(deviceId)) {
    setStatus("Device not found");
    return;
  }

  const auto device = m_devices.value(deviceId);
  auto *socket = new QTcpSocket(this);
  auto gotResponse = QSharedPointer<bool>::create(false);
  connect(socket, &QTcpSocket::connected, this, [this, socket, device]() {
    qInfo() << "SyncManager: pairing connect"
            << "to" << device.address
            << "port" << device.port;
    QJsonObject payload{
        {"type", "pair_request"},
        {"id", m_deviceId},
        {"name", m_deviceName},
        {"pin", m_pin},
        {"port", m_listenPort},
    };
    const QByteArray data = QJsonDocument(payload).toJson(QJsonDocument::Compact);
    socket->write(data);
  });
  connect(socket, &QTcpSocket::readyRead, this, [this, socket, deviceId, gotResponse]() {
    const QByteArray data = socket->readAll();
    const auto doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
      setStatus("Pairing failed");
      qWarning() << "SyncManager: pairing failed (invalid response)";
      socket->disconnectFromHost();
      socket->deleteLater();
      return;
    }
    const auto obj = doc.object();
    if (obj.value("type").toString() != "pair_ok") {
      setStatus("Pairing rejected");
      qWarning() << "SyncManager: pairing rejected"
                 << obj.value("type").toString()
                 << obj.value("error").toString();
      socket->disconnectFromHost();
      socket->deleteLater();
      return;
    }
    *gotResponse = true;
    const QString remoteId = obj.value("id").toString();
    PairedInfo info;
    info.id = remoteId;
    info.name = obj.value("name").toString();
    info.address = normalizeAddress(socket->peerAddress().toString());
    info.port = obj.value("port").toInt();
    info.token = obj.value("token").toString();
    m_paired.insert(remoteId, info);
    savePairedDevices();
    updateDevice(remoteId, info.name, info.address, info.port, true);
    setStatus("Paired with " + info.name);
    qInfo() << "SyncManager: paired with"
            << info.name
            << "id" << remoteId
            << "address" << info.address
            << "port" << info.port;
    socket->disconnectFromHost();
    socket->deleteLater();
  });
  connect(socket, &QTcpSocket::errorOccurred, this, [this, socket, gotResponse](QAbstractSocket::SocketError) {
    qWarning() << "SyncManager: pairing socket error"
               << socket->errorString()
               << "peer" << socket->peerAddress().toString()
               << "port" << socket->peerPort();
    setStatus("Pairing failed");
    socket->deleteLater();
  });
  connect(socket, &QTcpSocket::disconnected, this, [this, socket, gotResponse]() {
    if (!*gotResponse) {
      qWarning() << "SyncManager: pairing socket closed without response";
      setStatus("Pairing failed");
    }
    socket->deleteLater();
  });
  socket->connectToHost(device.address, device.port);
}

void SyncManager::unpair(const QString &deviceId) {
  if (deviceId.trimmed().isEmpty()) {
    return;
  }
  m_paired.remove(deviceId);
  savePairedDevices();
  if (m_devices.contains(deviceId)) {
  auto device = m_devices.value(deviceId);
  device.paired = false;
  m_devices.insert(deviceId, device);
  emit devicesChanged();
  }
  setStatus("Unpaired");
}

void SyncManager::syncNow(const QString &deviceId) {
  if (!m_enabled) {
    setStatus("Sync disabled");
    return;
  }
  if (!m_libraryModel) {
    setStatus("Library unavailable");
    return;
  }
  const QString trimmed = deviceId.trimmed();
  if (trimmed.isEmpty() || !m_paired.contains(trimmed)) {
    setStatus("Device not paired");
    return;
  }
  const auto info = m_paired.value(trimmed);
  qInfo() << "SyncManager: starting sync to" << info.name << info.address << info.port;
  auto *socket = new QTcpSocket(this);
  connect(socket, &QTcpSocket::connected, this, [this, socket, info]() {
    m_transferActive = false;
    m_transferTotal = 0;
    m_transferDone = 0;
    emit transferProgressChanged();
    qInfo() << "SyncManager: connected to" << info.name;
    const QVariantList annotations = annotationPayload();
    const QVariantList library = libraryPayload();
    QJsonArray hashes;
    for (const auto &entry : library) {
      const QString hash = entry.toMap().value("file_hash").toString();
      if (!hash.isEmpty()) {
        hashes.append(hash);
      }
    }
    QJsonObject payload{
        {"type", "sync_request"},
        {"id", m_deviceId},
        {"token", info.token},
        {"annotations", QJsonDocument::fromVariant(annotations).array()},
        {"library", QJsonDocument::fromVariant(library).array()},
        {"have_hashes", hashes},
        {"transfer_enabled", m_transferEnabled},
        {"transfer_max_mb", m_transferMaxMb},
    };
    socket->write(QJsonDocument(payload).toJson(QJsonDocument::Compact));
    qInfo() << "SyncManager: sent sync_request (notes" << annotations.size()
            << "meta" << library.size() << "have" << hashes.size()
            << "transfer" << m_transferEnabled << "maxMb" << m_transferMaxMb << ")";
    setStatus("Syncing with " + info.name);
  });
  connect(socket, &QTcpSocket::readyRead, this, [this, socket, info]() {
    QByteArray buffer = socket->property("syncBuffer").toByteArray();
    buffer.append(socket->readAll());
    socket->setProperty("syncBuffer", buffer);

    const qint64 maxBytes = static_cast<qint64>(m_transferMaxMb) * 1024 * 1024;
    if (maxBytes > 0 && buffer.size() > maxBytes) {
      qWarning() << "SyncManager: sync_response exceeded limit"
                 << buffer.size() << "bytes (max" << maxBytes << ")";
      setStatus("Sync failed");
      socket->disconnectFromHost();
      socket->deleteLater();
      return;
    }

    QJsonParseError parseError{};
    const auto doc = QJsonDocument::fromJson(buffer, &parseError);
    if (doc.isNull() || !doc.isObject()) {
      if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "SyncManager: waiting for full sync_response"
                   << parseError.errorString()
                   << "bytes" << buffer.size();
      }
      return;
    }
    const auto obj = doc.object();
    const QString type = obj.value("type").toString();
    if (type == "sync_error") {
      qWarning() << "SyncManager: sync_error"
                 << obj.value("error").toString();
      setStatus("Sync failed");
      socket->disconnectFromHost();
      socket->deleteLater();
      return;
    }
    if (type != "sync_response") {
      qWarning() << "SyncManager: unexpected response type" << type;
      setStatus("Sync failed");
      socket->disconnectFromHost();
      socket->deleteLater();
      return;
    }
    const QJsonArray annotations = obj.value("annotations").toArray();
    const QJsonArray library = obj.value("library").toArray();
    const QJsonArray files = obj.value("files").toArray();
    qInfo() << "SyncManager: received sync_response (notes" << annotations.size()
            << "meta" << library.size() << "files" << files.size() << ")";
    const int added = applyAnnotationPayload(annotations.toVariantList());
    const int metaUpdated = applyLibraryPayload(library.toVariantList());
    int filesAdded = 0;
    m_transferTotal = files.size();
    m_transferDone = 0;
    m_transferActive = m_transferTotal > 0;
    emit transferProgressChanged();
    for (const auto &fileEntry : files) {
      const QVariantMap map = fileEntry.toObject().toVariantMap();
      const QString hash = map.value("file_hash").toString();
      const QString name = map.value("name").toString();
      const QString format = map.value("format").toString();
      const QByteArray data = QByteArray::fromBase64(map.value("data").toByteArray());
      if (hash.isEmpty() || data.isEmpty()) {
        qWarning() << "SyncManager: skipped file with missing hash/data" << name;
        m_transferDone++;
        emit transferProgressChanged();
        continue;
      }
      const QByteArray digest = QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex();
      if (QString::fromLatin1(digest) != hash) {
        qWarning() << "SyncManager: checksum mismatch for" << name << "expected" << hash;
        m_transferDone++;
        emit transferProgressChanged();
        continue;
      }
      if (m_libraryModel && m_libraryModel->hasFileHash(hash)) {
        qInfo() << "SyncManager: already have file" << name;
        m_transferDone++;
        emit transferProgressChanged();
        continue;
      }
      const QString base = syncInboxDir();
      QDir().mkpath(base);
      const QString fileName =
          name.isEmpty() ? (hash.left(12) + "." + format) : name;
      const QString filePath = QDir(base).filePath(fileName);
      QFile file(filePath);
      if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "SyncManager: failed to write file" << filePath;
        m_transferDone++;
        emit transferProgressChanged();
        continue;
      }
      file.write(data);
      file.close();
      if (m_libraryModel) {
        if (m_libraryModel->addBook(filePath)) {
          filesAdded++;
          qInfo() << "SyncManager: imported file" << filePath;
        } else {
          qWarning() << "SyncManager: failed to import" << filePath;
        }
      }
      m_transferDone++;
      emit transferProgressChanged();
    }
    m_transferActive = false;
    emit transferProgressChanged();
    PairedInfo updated = info;
    updated.lastSync = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    m_paired.insert(updated.id, updated);
    savePairedDevices();
    updateDevice(updated.id, updated.name, updated.address, updated.port, true);
    setStatus(QString("Synced with %1 (%2 notes, %3 meta, %4 files)")
                  .arg(info.name)
                  .arg(added)
                  .arg(metaUpdated)
                  .arg(filesAdded));
    qInfo() << "SyncManager: sync complete" << info.name << "notes" << added
            << "meta" << metaUpdated << "files" << filesAdded;
    socket->disconnectFromHost();
    socket->deleteLater();
  });
  connect(socket, &QTcpSocket::errorOccurred, this, [this, socket](QAbstractSocket::SocketError) {
    qWarning() << "SyncManager: sync socket error"
               << socket->errorString()
               << "state" << socket->state()
               << "peer" << socket->peerAddress().toString()
               << "port" << socket->peerPort();
    setStatus("Sync failed");
    socket->deleteLater();
  });
  socket->connectToHost(info.address, info.port);
}

void SyncManager::setStatus(const QString &status) {
  if (m_status == status) {
    return;
  }
  m_status = status;
  emit statusChanged();
}

void SyncManager::loadSettings() {
  m_enabled = m_settings->value("sync/enabled", false).toBool();
  m_deviceId = m_settings->value("device/id", "").toString();
  if (m_deviceId.isEmpty()) {
    m_deviceId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    m_settings->setValue("device/id", m_deviceId);
  }
  const QString defaultName = QHostInfo::localHostName();
  m_deviceName = m_settings->value("device/name", defaultName).toString();
  if (m_deviceName.trimmed().isEmpty()) {
    m_deviceName = defaultName;
    m_settings->setValue("device/name", m_deviceName);
  }
  m_pin = m_settings->value("device/pin", "").toString();
  if (m_pin.trimmed().isEmpty()) {
    m_pin = randomPin();
    m_settings->setValue("device/pin", m_pin);
  }
  m_discoveryPort = m_settings->value("network/discovery_port", 45454).toInt();
  m_listenPort = m_settings->value("network/listen_port", 45455).toInt();
  m_conflictPolicy = m_settings->value("sync/conflict_policy", "newer").toString();
  m_transferEnabled = m_settings->value("sync/transfer_enabled", true).toBool();
  m_transferMaxMb = m_settings->value("sync/transfer_max_mb", 5120).toInt();
  loadPairedDevices();
  m_settings->sync();
  qInfo() << "SyncManager: settings loaded"
          << "deviceId" << m_deviceId
          << "deviceName" << m_deviceName
          << "enabled" << m_enabled
          << "discoverPort" << m_discoveryPort
          << "listenPort" << m_listenPort;
}

void SyncManager::saveSettings() {
  m_settings->setValue("sync/enabled", m_enabled);
  m_settings->setValue("device/name", m_deviceName);
  m_settings->setValue("device/pin", m_pin);
  m_settings->setValue("network/discovery_port", m_discoveryPort);
  m_settings->setValue("network/listen_port", m_listenPort);
  m_settings->setValue("sync/conflict_policy", m_conflictPolicy);
  m_settings->setValue("sync/transfer_enabled", m_transferEnabled);
  m_settings->setValue("sync/transfer_max_mb", m_transferMaxMb);
  m_settings->sync();
}

void SyncManager::ensureUdpSocket() {
  if (m_udp) {
    return;
  }
  m_udp = new QUdpSocket(this);
  if (!m_udp->bind(QHostAddress::AnyIPv4, static_cast<quint16>(m_discoveryPort),
                   QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
    qWarning() << "SyncManager: discovery bind failed"
               << m_udp->errorString()
               << "port" << m_discoveryPort;
    setStatus("Discovery bind failed");
    return;
  }
  qInfo() << "SyncManager: discovery socket bound"
          << "port" << m_discoveryPort
          << "local" << m_udp->localAddress().toString();
  connect(m_udp, &QUdpSocket::readyRead, this, [this]() {
    while (m_udp->hasPendingDatagrams()) {
      QHostAddress sender;
      quint16 port = 0;
      QByteArray buffer;
      buffer.resize(static_cast<int>(m_udp->pendingDatagramSize()));
      m_udp->readDatagram(buffer.data(), buffer.size(), &sender, &port);
      qInfo() << "SyncManager: datagram received"
              << "from" << sender.toString()
              << "port" << port
              << "bytes" << buffer.size();
      handleDatagram(sender, port, buffer);
    }
  });
}

void SyncManager::ensureServer() {
  if (!m_enabled) {
    return;
  }
  if (!m_server) {
    m_server = new QTcpServer(this);
    connect(m_server, &QTcpServer::newConnection, this, [this]() {
      while (m_server->hasPendingConnections()) {
        auto *socket = m_server->nextPendingConnection();
        qInfo() << "SyncManager: incoming connection"
                << socket->peerAddress().toString()
                << socket->peerPort();
        connect(socket, &QTcpSocket::readyRead, this, [this, socket]() {
          const QByteArray data = socket->readAll();
          const auto doc = QJsonDocument::fromJson(data);
          if (!doc.isObject()) {
            socket->disconnectFromHost();
            socket->deleteLater();
            return;
          }
          const auto obj = doc.object();
          const QString type = obj.value("type").toString();
          if (type == "pair_request") {
            const QString pin = obj.value("pin").toString();
            const QString remoteId = obj.value("id").toString();
            const QString remoteName = obj.value("name").toString();
            const int remotePort = obj.value("port").toInt();
            qInfo() << "SyncManager: pair_request from"
                    << remoteName
                    << remoteId
                    << socket->peerAddress().toString()
                    << "port" << remotePort
                    << "pinMatch" << (!pin.isEmpty() && pin == m_pin);
            if (pin.isEmpty() || pin != m_pin) {
              qWarning() << "SyncManager: pairing rejected for" << remoteName;
              QJsonObject resp{{"type", "pair_reject"}};
              socket->write(QJsonDocument(resp).toJson(QJsonDocument::Compact));
              socket->flush();
              socket->waitForBytesWritten(500);
              socket->disconnectFromHost();
              socket->deleteLater();
              return;
            }
            const QString token = QUuid::createUuid().toString(QUuid::WithoutBraces);
            PairedInfo info;
            info.id = remoteId;
            info.name = remoteName;
            info.address = socket->peerAddress().toString();
            info.port = remotePort;
            info.token = token;
            m_paired.insert(remoteId, info);
            savePairedDevices();
            updateDevice(remoteId, remoteName, info.address, remotePort, true);
            QJsonObject resp{
                {"type", "pair_ok"},
                {"id", m_deviceId},
                {"name", m_deviceName},
                {"address", socket->localAddress().toString()},
                {"port", m_listenPort},
                {"token", token}};
            socket->write(QJsonDocument(resp).toJson(QJsonDocument::Compact));
            socket->flush();
            socket->waitForBytesWritten(500);
            socket->disconnectFromHost();
            socket->deleteLater();
            setStatus("Paired with " + remoteName);
            qInfo() << "SyncManager: paired with" << remoteName;
            return;
          }
          if (type == "sync_request") {
            const QString remoteId = obj.value("id").toString();
            const QString token = obj.value("token").toString();
            if (remoteId.isEmpty() || !m_paired.contains(remoteId) ||
                m_paired.value(remoteId).token != token) {
              qWarning() << "SyncManager: sync_request unauthorized from" << remoteId
                         << "paired" << m_paired.contains(remoteId)
                         << "tokenMatch" << (m_paired.contains(remoteId) && m_paired.value(remoteId).token == token);
            QJsonObject resp{{"type", "sync_error"}, {"error", "unauthorized"}};
            socket->write(QJsonDocument(resp).toJson(QJsonDocument::Compact));
            if (socket->bytesToWrite() > 0) {
              connect(socket, &QTcpSocket::bytesWritten, socket, [socket]() {
                if (socket->bytesToWrite() == 0) {
                  socket->disconnectFromHost();
                  socket->deleteLater();
                }
              });
            } else {
              socket->disconnectFromHost();
              socket->deleteLater();
            }
            return;
          }
            m_uploadActive = false;
            m_uploadTotal = 0;
            m_uploadDone = 0;
            emit uploadProgressChanged();
            const QJsonArray annotations = obj.value("annotations").toArray();
            const QJsonArray library = obj.value("library").toArray();
            const QJsonArray haveHashes = obj.value("have_hashes").toArray();
            const bool transferAllowed = obj.value("transfer_enabled").toBool(false);
            const int maxMb = obj.value("transfer_max_mb").toInt(m_transferMaxMb);
            qInfo() << "SyncManager: sync_request from" << remoteId
                    << "notes" << annotations.size()
                    << "meta" << library.size()
                    << "transfer" << transferAllowed
                    << "maxMb" << maxMb;
            const int added = applyAnnotationPayload(annotations.toVariantList());
            const int metaUpdated = applyLibraryPayload(library.toVariantList());
            QJsonArray files;
            if (transferAllowed && m_transferEnabled && m_libraryModel) {
              const qint64 maxBytes = static_cast<qint64>(maxMb) * 1024 * 1024;
              qint64 totalBytes = 0;
              QSet<QString> haveSet;
              for (const auto &v : haveHashes) {
                const QString hash = v.toString();
                if (!hash.isEmpty()) {
                  haveSet.insert(hash);
                }
              }
              const QVariantList localLibrary = libraryPayload();
              m_uploadTotal = localLibrary.size();
              m_uploadDone = 0;
              m_uploadActive = m_uploadTotal > 0;
              emit uploadProgressChanged();
              for (const auto &entry : localLibrary) {
                const QVariantMap map = entry.toMap();
                const QString hash = map.value("file_hash").toString();
                if (hash.isEmpty() || haveSet.contains(hash)) {
                  m_uploadDone++;
                  emit uploadProgressChanged();
                  continue;
                }
                const QString path = m_libraryModel->pathForHash(hash);
                if (path.isEmpty() || !QFileInfo::exists(path)) {
                  qWarning() << "SyncManager: missing file for hash" << hash;
                  m_uploadDone++;
                  emit uploadProgressChanged();
                  continue;
                }
                QFile file(path);
                if (!file.open(QIODevice::ReadOnly)) {
                  qWarning() << "SyncManager: failed to read" << path;
                  m_uploadDone++;
                  emit uploadProgressChanged();
                  continue;
                }
                const QByteArray data = file.readAll();
                file.close();
                if (data.isEmpty()) {
                  m_uploadDone++;
                  emit uploadProgressChanged();
                  continue;
                }
                if (totalBytes + data.size() > maxBytes) {
                  qInfo() << "SyncManager: transfer size limit reached";
                  break;
                }
                QVariantMap fileEntry;
                fileEntry.insert("file_hash", hash);
                fileEntry.insert("name", QFileInfo(path).fileName());
                fileEntry.insert("format", QFileInfo(path).suffix().toLower());
                fileEntry.insert("data", data.toBase64());
                files.append(QJsonDocument::fromVariant(fileEntry).object());
                totalBytes += data.size();
                m_uploadDone++;
                emit uploadProgressChanged();
              }
              m_uploadActive = false;
              emit uploadProgressChanged();
            }
            QJsonObject resp{
                {"type", "sync_response"},
                {"annotations", QJsonDocument::fromVariant(annotationPayload()).array()},
                {"library", QJsonDocument::fromVariant(libraryPayload()).array()},
                {"files", files},
                {"added", added},
                {"meta_updated", metaUpdated}};
            socket->write(QJsonDocument(resp).toJson(QJsonDocument::Compact));
            if (socket->bytesToWrite() > 0) {
              connect(socket, &QTcpSocket::bytesWritten, socket, [socket]() {
                if (socket->bytesToWrite() == 0) {
                  socket->disconnectFromHost();
                  socket->deleteLater();
                }
              });
            } else {
              socket->disconnectFromHost();
              socket->deleteLater();
            }
            PairedInfo updated = m_paired.value(remoteId);
            updated.lastSync = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
            m_paired.insert(remoteId, updated);
            savePairedDevices();
            updateDevice(remoteId, updated.name, updated.address, updated.port, true);
            setStatus("Synced with " + updated.name);
            qInfo() << "SyncManager: sync_response sent to" << updated.name
                    << "notes" << added << "meta" << metaUpdated << "files" << files.size();
            return;
          }
          socket->disconnectFromHost();
          socket->deleteLater();
        });
      }
    });
  }
  if (!m_server->isListening()) {
    if (!m_server->listen(QHostAddress::AnyIPv4, static_cast<quint16>(m_listenPort))) {
      qWarning() << "SyncManager: listen failed"
                 << m_server->errorString()
                 << "port" << m_listenPort;
      setStatus("Sync listen failed");
    } else {
      qInfo() << "SyncManager: listening"
              << "port" << m_listenPort;
    }
  }
}

void SyncManager::sendDiscovery() {
  if (!m_udp || !m_enabled) {
    qWarning() << "SyncManager: sendDiscovery skipped"
               << "udp" << (m_udp != nullptr)
               << "enabled" << m_enabled;
    return;
  }
  if (m_deviceId.trimmed().isEmpty()) {
    m_deviceId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    m_settings->setValue("device/id", m_deviceId);
    m_settings->sync();
    qWarning() << "SyncManager: deviceId missing, regenerated" << m_deviceId;
  }
  QJsonObject payload{
      {"type", "discover"},
      {"id", m_deviceId},
      {"name", m_deviceName},
      {"port", m_listenPort},
      {"time", QDateTime::currentDateTimeUtc().toString(Qt::ISODate)}
  };
  const QByteArray data = QJsonDocument(payload).toJson(QJsonDocument::Compact);
  const qint64 written = m_udp->writeDatagram(data, QHostAddress::Broadcast,
                                              static_cast<quint16>(m_discoveryPort));
  if (written < 0) {
    qWarning() << "SyncManager: sendDiscovery failed"
               << m_udp->errorString()
               << "port" << m_discoveryPort;
  } else {
    qInfo() << "SyncManager: sent discovery"
            << "bytes" << written
            << "port" << m_discoveryPort;
  }
}

void SyncManager::sendAnnounce(const QHostAddress &address, quint16 port) {
  if (!m_udp || !m_enabled) {
    qWarning() << "SyncManager: sendAnnounce skipped"
               << "udp" << (m_udp != nullptr)
               << "enabled" << m_enabled;
    return;
  }
  QJsonObject payload{
      {"type", "announce"},
      {"id", m_deviceId},
      {"name", m_deviceName},
      {"port", m_listenPort},
      {"time", QDateTime::currentDateTimeUtc().toString(Qt::ISODate)}
  };
  const QByteArray data = QJsonDocument(payload).toJson(QJsonDocument::Compact);
  const qint64 written = m_udp->writeDatagram(data, address, port);
  if (written < 0) {
    qWarning() << "SyncManager: sendAnnounce failed"
               << m_udp->errorString()
               << "to" << address.toString()
               << "port" << port;
  } else {
    qInfo() << "SyncManager: sent announce"
            << "to" << address.toString()
            << "port" << port
            << "bytes" << written;
  }
}

void SyncManager::handleDatagram(const QHostAddress &address, quint16 port, const QByteArray &payload) {
  const auto doc = QJsonDocument::fromJson(payload);
  if (!doc.isObject()) {
    qWarning() << "SyncManager: datagram ignored (not object)"
               << "from" << address.toString()
               << "port" << port;
    return;
  }
  const auto obj = doc.object();
  const QString type = obj.value("type").toString();
  const QString id = obj.value("id").toString();
  const QString name = obj.value("name").toString();
  const int remotePort = obj.value("port").toInt();
  if (id.isEmpty() || id == m_deviceId) {
    qInfo() << "SyncManager: datagram ignored (self/empty id)"
            << "type" << type
            << "from" << address.toString()
            << "payload" << QString::fromUtf8(payload);
    return;
  }
  qInfo() << "SyncManager: datagram"
          << "type" << type
          << "id" << id
          << "name" << name
          << "addr" << address.toString()
          << "port" << remotePort;
  if (type == "discover") {
    updateDevice(id, name, address.toString(), remotePort, true);
    sendAnnounce(address, port);
  } else if (type == "announce") {
    updateDevice(id, name, address.toString(), remotePort, true);
  } else {
    qWarning() << "SyncManager: datagram ignored (unknown type)"
               << type
               << "from" << address.toString();
  }
}

void SyncManager::updateDevice(const QString &id,
                               const QString &name,
                               const QString &address,
                               int port,
                               bool discovered) {
  DeviceInfo info = m_devices.value(id);
  info.id = id;
  info.name = name.isEmpty() ? info.name : name;
  info.address = normalizeAddress(address);
  info.port = port;
  info.lastSeen = QDateTime::currentMSecsSinceEpoch();
  info.paired = m_paired.contains(id);
  if (info.paired) {
    info.lastSync = m_paired.value(id).lastSync;
  } else {
    info.lastSync.clear();
  }
  m_devices.insert(id, info);
  if (discovered) {
    emit devicesChanged();
  }
  qInfo() << "SyncManager: device updated"
          << "id" << id
          << "name" << info.name
          << "address" << info.address
          << "port" << info.port
          << "paired" << info.paired
          << "devices" << m_devices.size();
}

void SyncManager::pruneDevices() {
  const qint64 cutoff = QDateTime::currentMSecsSinceEpoch() - 15000;
  bool changed = false;
  auto it = m_devices.begin();
  while (it != m_devices.end()) {
    if (it->lastSeen > 0 && it->lastSeen < cutoff && !it->paired) {
      qInfo() << "SyncManager: pruned device"
              << it->id
              << it->name
              << it->address
              << it->port;
      it = m_devices.erase(it);
      changed = true;
    } else {
      ++it;
    }
  }
  if (changed) {
    emit devicesChanged();
  }
}

void SyncManager::loadPairedDevices() {
  m_paired.clear();
  m_devices.clear();
  m_settings->beginGroup("paired");
  const QStringList groups = m_settings->childGroups();
  qInfo() << "SyncManager: load paired devices" << groups.size();
  for (const auto &group : groups) {
    m_settings->beginGroup(group);
    PairedInfo info;
    info.id = group;
    info.name = m_settings->value("name").toString();
    info.address = m_settings->value("address").toString();
    info.port = m_settings->value("port").toInt();
    info.token = m_settings->value("token").toString();
    info.lastSync = m_settings->value("last_sync").toString();
    m_paired.insert(info.id, info);
    DeviceInfo device;
    device.id = info.id;
    device.name = info.name;
    device.address = info.address;
    device.port = info.port;
    device.paired = true;
    device.lastSeen = 0;
    device.lastSync = info.lastSync;
    m_devices.insert(info.id, device);
    m_settings->endGroup();
    qInfo() << "SyncManager: paired device loaded"
            << info.id
            << info.name
            << info.address
            << info.port;
  }
  m_settings->endGroup();
  if (!m_devices.isEmpty()) {
    emit devicesChanged();
  }
}

void SyncManager::savePairedDevices() {
  m_settings->remove("paired");
  m_settings->beginGroup("paired");
  for (const auto &info : m_paired) {
    m_settings->beginGroup(info.id);
    m_settings->setValue("name", info.name);
    m_settings->setValue("address", info.address);
    m_settings->setValue("port", info.port);
    m_settings->setValue("token", info.token);
    m_settings->setValue("last_sync", info.lastSync);
    m_settings->endGroup();
  }
  m_settings->endGroup();
  m_settings->sync();
  qInfo() << "SyncManager: saved paired devices" << m_paired.size();
}

void SyncManager::setDiscovering(bool discovering) {
  if (m_discovering == discovering) {
    return;
  }
  m_discovering = discovering;
  emit discoveringChanged();
}

QVariantList SyncManager::annotationPayload() const {
  if (!m_libraryModel) {
    return {};
  }
  return m_libraryModel->exportAnnotationSync();
}

int SyncManager::applyAnnotationPayload(const QVariantList &payload) {
  if (!m_libraryModel) {
    return 0;
  }
  return m_libraryModel->importAnnotationSync(payload);
}

QVariantList SyncManager::libraryPayload() const {
  if (!m_libraryModel) {
    return {};
  }
  return m_libraryModel->exportLibrarySync();
}

int SyncManager::applyLibraryPayload(const QVariantList &payload) {
  if (!m_libraryModel) {
    return 0;
  }
  return m_libraryModel->importLibrarySync(payload, m_conflictPolicy);
}

QString SyncManager::syncInboxDir() const {
  const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QDir dir(base);
  dir.mkpath("sync_inbox");
  return dir.filePath("sync_inbox");
}
