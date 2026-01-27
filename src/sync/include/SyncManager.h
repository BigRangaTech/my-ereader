#pragma once

#include <QObject>
#include <QString>

class SyncManager : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
  Q_PROPERTY(QString status READ status NOTIFY statusChanged)

public:
  explicit SyncManager(QObject *parent = nullptr);

  bool enabled() const;
  void setEnabled(bool enabled);

  QString status() const;

  Q_INVOKABLE void startDiscovery();
  Q_INVOKABLE void stopDiscovery();
  Q_INVOKABLE void requestPairing(const QString &pin);

signals:
  void enabledChanged();
  void statusChanged();

private:
  void setStatus(const QString &status);

  bool m_enabled = false;
  QString m_status = "Idle";
};
