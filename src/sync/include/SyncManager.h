#pragma once

#include <QByteArray>
#include <QHash>
#include <QHostAddress>
#include <QObject>
#include <QString>
#include <QVariantList>

class LibraryModel;

class SyncManager : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
  Q_PROPERTY(QString status READ status NOTIFY statusChanged)
  Q_PROPERTY(QString deviceName READ deviceName WRITE setDeviceName NOTIFY deviceNameChanged)
  Q_PROPERTY(QString deviceId READ deviceId CONSTANT)
  Q_PROPERTY(QString pin READ pin WRITE setPin NOTIFY pinChanged)
  Q_PROPERTY(int discoveryPort READ discoveryPort WRITE setDiscoveryPort NOTIFY discoveryPortChanged)
  Q_PROPERTY(int listenPort READ listenPort WRITE setListenPort NOTIFY listenPortChanged)
  Q_PROPERTY(bool discovering READ discovering NOTIFY discoveringChanged)
  Q_PROPERTY(QVariantList devices READ devices NOTIFY devicesChanged)
  Q_PROPERTY(QObject *libraryModel READ libraryModel WRITE setLibraryModel NOTIFY libraryModelChanged)
  Q_PROPERTY(QString conflictPolicy READ conflictPolicy WRITE setConflictPolicy NOTIFY conflictPolicyChanged)
  Q_PROPERTY(bool transferEnabled READ transferEnabled WRITE setTransferEnabled NOTIFY transferEnabledChanged)
  Q_PROPERTY(int transferMaxMb READ transferMaxMb WRITE setTransferMaxMb NOTIFY transferMaxMbChanged)
  Q_PROPERTY(bool transferActive READ transferActive NOTIFY transferProgressChanged)
  Q_PROPERTY(int transferTotal READ transferTotal NOTIFY transferProgressChanged)
  Q_PROPERTY(int transferDone READ transferDone NOTIFY transferProgressChanged)
  Q_PROPERTY(bool uploadActive READ uploadActive NOTIFY uploadProgressChanged)
  Q_PROPERTY(int uploadTotal READ uploadTotal NOTIFY uploadProgressChanged)
  Q_PROPERTY(int uploadDone READ uploadDone NOTIFY uploadProgressChanged)

public:
  explicit SyncManager(QObject *parent = nullptr);

  bool enabled() const;
  void setEnabled(bool enabled);

  QString status() const;
  QString deviceName() const;
  QString deviceId() const;
  QString pin() const;
  int discoveryPort() const;
  int listenPort() const;
  bool discovering() const;
  QVariantList devices() const;
  QObject *libraryModel() const;
  QString conflictPolicy() const;
  bool transferEnabled() const;
  int transferMaxMb() const;
  bool transferActive() const;
  int transferTotal() const;
  int transferDone() const;
  bool uploadActive() const;
  int uploadTotal() const;
  int uploadDone() const;

  void setDeviceName(const QString &name);
  void setPin(const QString &pin);
  void setDiscoveryPort(int port);
  void setListenPort(int port);
  void setLibraryModel(QObject *model);
  void setConflictPolicy(const QString &policy);
  void setTransferEnabled(bool enabled);
  void setTransferMaxMb(int mb);

  Q_INVOKABLE void startDiscovery();
  Q_INVOKABLE void stopDiscovery();
  Q_INVOKABLE void requestPairing(const QString &deviceId);
  Q_INVOKABLE void unpair(const QString &deviceId);
  Q_INVOKABLE void syncNow(const QString &deviceId);

signals:
  void enabledChanged();
  void statusChanged();
  void deviceNameChanged();
  void pinChanged();
  void discoveryPortChanged();
  void listenPortChanged();
  void discoveringChanged();
  void devicesChanged();
  void libraryModelChanged();
  void conflictPolicyChanged();
  void transferEnabledChanged();
  void transferMaxMbChanged();
  void transferProgressChanged();
  void uploadProgressChanged();

private:
  void setStatus(const QString &status);
  void loadSettings();
  void saveSettings();
  void ensureUdpSocket();
  void ensureServer();
  void sendDiscovery();
  void sendAnnounce(const QHostAddress &address, quint16 port);
  void handleDatagram(const QHostAddress &address, quint16 port, const QByteArray &payload);
  void updateDevice(const QString &id,
                    const QString &name,
                    const QString &address,
                    int port,
                    bool discovered);
  void pruneDevices();
  void loadPairedDevices();
  void savePairedDevices();
  void setDiscovering(bool discovering);
  QVariantList annotationPayload() const;
  int applyAnnotationPayload(const QVariantList &payload);
  QVariantList libraryPayload() const;
  int applyLibraryPayload(const QVariantList &payload);
  QString syncInboxDir() const;

  bool m_enabled = false;
  QString m_status = "Idle";
  QString m_deviceName;
  QString m_deviceId;
  QString m_pin;
  int m_discoveryPort = 45454;
  int m_listenPort = 45455;
  bool m_discovering = false;
  QString m_conflictPolicy = "newer";
  bool m_transferEnabled = true;
  int m_transferMaxMb = 50;
  bool m_transferActive = false;
  int m_transferTotal = 0;
  int m_transferDone = 0;
  bool m_uploadActive = false;
  int m_uploadTotal = 0;
  int m_uploadDone = 0;

  struct DeviceInfo {
    QString id;
    QString name;
    QString address;
    int port = 0;
    bool paired = false;
    qint64 lastSeen = 0;
    QString lastSync;
  };
  struct PairedInfo {
    QString id;
    QString name;
    QString address;
    int port = 0;
    QString token;
    QString lastSync;
  };
  QHash<QString, DeviceInfo> m_devices;
  QHash<QString, PairedInfo> m_paired;
  class LibraryModel *m_libraryModel = nullptr;

  class QUdpSocket *m_udp = nullptr;
  class QTcpServer *m_server = nullptr;
  class QTimer *m_discoveryTimer = nullptr;
  class QTimer *m_pruneTimer = nullptr;
  class QSettings *m_settings = nullptr;
};
