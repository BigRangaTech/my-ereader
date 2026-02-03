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

  void setDeviceName(const QString &name);
  void setPin(const QString &pin);
  void setDiscoveryPort(int port);
  void setListenPort(int port);
  void setLibraryModel(QObject *model);

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

  bool m_enabled = false;
  QString m_status = "Idle";
  QString m_deviceName;
  QString m_deviceId;
  QString m_pin;
  int m_discoveryPort = 45454;
  int m_listenPort = 45455;
  bool m_discovering = false;

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
