#include "include/SyncManager.h"
#include "../core/include/AppPaths.h"
#include "../core/include/LibraryModel.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QHostAddress>
#include <QHostInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>
#include <QSettings>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QUdpSocket>
#include <QUuid>

namespace {
QString resolveSyncSettingsPath() {
  return AppPaths::configFile("sync.ini");
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
    list.append(entry);
  }
  return list;
}

QObject *SyncManager::libraryModel() const {
  return m_libraryModel;
}

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

void SyncManager::startDiscovery() {
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
  connect(socket, &QTcpSocket::connected, this, [this, socket, device]() {
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
  connect(socket, &QTcpSocket::readyRead, this, [this, socket, deviceId]() {
    const QByteArray data = socket->readAll();
    const auto doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
      setStatus("Pairing failed");
      socket->disconnectFromHost();
      socket->deleteLater();
      return;
    }
    const auto obj = doc.object();
    if (obj.value("type").toString() != "pair_ok") {
      setStatus("Pairing rejected");
      socket->disconnectFromHost();
      socket->deleteLater();
      return;
    }
    const QString remoteId = obj.value("id").toString();
    PairedInfo info;
    info.id = remoteId;
    info.name = obj.value("name").toString();
    info.address = obj.value("address").toString();
    info.port = obj.value("port").toInt();
    info.token = obj.value("token").toString();
    m_paired.insert(remoteId, info);
    savePairedDevices();
    updateDevice(remoteId, info.name, info.address, info.port, true);
    setStatus("Paired with " + info.name);
    socket->disconnectFromHost();
    socket->deleteLater();
  });
  connect(socket, &QTcpSocket::errorOccurred, this, [this, socket](QAbstractSocket::SocketError) {
    setStatus("Pairing failed");
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
  auto *socket = new QTcpSocket(this);
  connect(socket, &QTcpSocket::connected, this, [this, socket, info]() {
    const QVariantList annotations = annotationPayload();
    QJsonObject payload{
        {"type", "sync_request"},
        {"id", m_deviceId},
        {"token", info.token},
        {"annotations", QJsonDocument::fromVariant(annotations).array()},
    };
    socket->write(QJsonDocument(payload).toJson(QJsonDocument::Compact));
    setStatus("Syncing with " + info.name);
  });
  connect(socket, &QTcpSocket::readyRead, this, [this, socket, info]() {
    const QByteArray data = socket->readAll();
    const auto doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
      setStatus("Sync failed");
      socket->disconnectFromHost();
      socket->deleteLater();
      return;
    }
    const auto obj = doc.object();
    if (obj.value("type").toString() != "sync_response") {
      setStatus("Sync failed");
      socket->disconnectFromHost();
      socket->deleteLater();
      return;
    }
    const QJsonArray annotations = obj.value("annotations").toArray();
    const int added = applyAnnotationPayload(annotations.toVariantList());
    setStatus(QString("Synced with %1 (%2 new)").arg(info.name).arg(added));
    socket->disconnectFromHost();
    socket->deleteLater();
  });
  connect(socket, &QTcpSocket::errorOccurred, this, [this, socket](QAbstractSocket::SocketError) {
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
  loadPairedDevices();
  m_settings->sync();
}

void SyncManager::saveSettings() {
  m_settings->setValue("sync/enabled", m_enabled);
  m_settings->setValue("device/name", m_deviceName);
  m_settings->setValue("device/pin", m_pin);
  m_settings->setValue("network/discovery_port", m_discoveryPort);
  m_settings->setValue("network/listen_port", m_listenPort);
  m_settings->sync();
}

void SyncManager::ensureUdpSocket() {
  if (m_udp) {
    return;
  }
  m_udp = new QUdpSocket(this);
  if (!m_udp->bind(QHostAddress::AnyIPv4, static_cast<quint16>(m_discoveryPort),
                   QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
    setStatus("Discovery bind failed");
    return;
  }
  connect(m_udp, &QUdpSocket::readyRead, this, [this]() {
    while (m_udp->hasPendingDatagrams()) {
      QHostAddress sender;
      quint16 port = 0;
      QByteArray buffer;
      buffer.resize(static_cast<int>(m_udp->pendingDatagramSize()));
      m_udp->readDatagram(buffer.data(), buffer.size(), &sender, &port);
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
            if (pin.isEmpty() || pin != m_pin) {
              QJsonObject resp{{"type", "pair_reject"}};
              socket->write(QJsonDocument(resp).toJson(QJsonDocument::Compact));
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
            socket->disconnectFromHost();
            socket->deleteLater();
            setStatus("Paired with " + remoteName);
            return;
          }
          if (type == "sync_request") {
            const QString remoteId = obj.value("id").toString();
            const QString token = obj.value("token").toString();
            if (remoteId.isEmpty() || !m_paired.contains(remoteId) ||
                m_paired.value(remoteId).token != token) {
              QJsonObject resp{{"type", "sync_error"}, {"error", "unauthorized"}};
              socket->write(QJsonDocument(resp).toJson(QJsonDocument::Compact));
              socket->disconnectFromHost();
              socket->deleteLater();
              return;
            }
            const QJsonArray annotations = obj.value("annotations").toArray();
            const int added = applyAnnotationPayload(annotations.toVariantList());
            QJsonObject resp{
                {"type", "sync_response"},
                {"annotations", QJsonDocument::fromVariant(annotationPayload()).array()},
                {"added", added}};
            socket->write(QJsonDocument(resp).toJson(QJsonDocument::Compact));
            socket->disconnectFromHost();
            socket->deleteLater();
            setStatus("Synced with " + m_paired.value(remoteId).name);
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
      setStatus("Sync listen failed");
    }
  }
}

void SyncManager::sendDiscovery() {
  if (!m_udp || !m_enabled) {
    return;
  }
  QJsonObject payload{
      {"type", "discover"},
      {"id", m_deviceId},
      {"name", m_deviceName},
      {"port", m_listenPort},
      {"time", QDateTime::currentDateTimeUtc().toString(Qt::ISODate)}
  };
  const QByteArray data = QJsonDocument(payload).toJson(QJsonDocument::Compact);
  m_udp->writeDatagram(data, QHostAddress::Broadcast, static_cast<quint16>(m_discoveryPort));
}

void SyncManager::sendAnnounce(const QHostAddress &address, quint16 port) {
  if (!m_udp || !m_enabled) {
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
  m_udp->writeDatagram(data, address, port);
}

void SyncManager::handleDatagram(const QHostAddress &address, quint16 port, const QByteArray &payload) {
  const auto doc = QJsonDocument::fromJson(payload);
  if (!doc.isObject()) {
    return;
  }
  const auto obj = doc.object();
  const QString type = obj.value("type").toString();
  const QString id = obj.value("id").toString();
  const QString name = obj.value("name").toString();
  const int remotePort = obj.value("port").toInt();
  if (id.isEmpty() || id == m_deviceId) {
    return;
  }
  if (type == "discover") {
    updateDevice(id, name, address.toString(), remotePort, true);
    sendAnnounce(address, port);
  } else if (type == "announce") {
    updateDevice(id, name, address.toString(), remotePort, true);
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
  info.address = address;
  info.port = port;
  info.lastSeen = QDateTime::currentMSecsSinceEpoch();
  info.paired = m_paired.contains(id);
  m_devices.insert(id, info);
  if (discovered) {
    emit devicesChanged();
  }
}

void SyncManager::pruneDevices() {
  const qint64 cutoff = QDateTime::currentMSecsSinceEpoch() - 15000;
  bool changed = false;
  auto it = m_devices.begin();
  while (it != m_devices.end()) {
    if (it->lastSeen > 0 && it->lastSeen < cutoff && !it->paired) {
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
  for (const auto &group : groups) {
    m_settings->beginGroup(group);
    PairedInfo info;
    info.id = group;
    info.name = m_settings->value("name").toString();
    info.address = m_settings->value("address").toString();
    info.port = m_settings->value("port").toInt();
    info.token = m_settings->value("token").toString();
    m_paired.insert(info.id, info);
    DeviceInfo device;
    device.id = info.id;
    device.name = info.name;
    device.address = info.address;
    device.port = info.port;
    device.paired = true;
    device.lastSeen = 0;
    m_devices.insert(info.id, device);
    m_settings->endGroup();
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
    m_settings->endGroup();
  }
  m_settings->endGroup();
  m_settings->sync();
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
