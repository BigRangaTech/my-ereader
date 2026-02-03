#pragma once

#include <QObject>
#include <QString>

class VaultController : public QObject {
  Q_OBJECT
  Q_PROPERTY(State state READ state NOTIFY stateChanged)
  Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
  Q_PROPERTY(QString vaultPath READ vaultPath CONSTANT)
  Q_PROPERTY(QString dbPath READ dbPath CONSTANT)
  Q_PROPERTY(bool keychainAvailable READ keychainAvailable CONSTANT)
  Q_PROPERTY(QObject *libraryModel READ libraryModel WRITE setLibraryModel NOTIFY libraryModelChanged)

public:
  enum State { Locked, Unlocked, NeedsSetup, Error };
  Q_ENUM(State)

  explicit VaultController(QObject *parent = nullptr);

  State state() const;
  QString lastError() const;
  QString vaultPath() const;
  QString dbPath() const;
  bool keychainAvailable() const;
  QObject *libraryModel() const;
  void setLibraryModel(QObject *model);

  Q_INVOKABLE void initialize();
  Q_INVOKABLE bool unlock(const QString &passphrase);
  Q_INVOKABLE bool setupNew(const QString &passphrase);
  Q_INVOKABLE bool lock(const QString &passphrase);
  Q_INVOKABLE QString loadStoredPassphrase();
  Q_INVOKABLE bool storePassphrase(const QString &passphrase);
  Q_INVOKABLE bool clearStoredPassphrase();

signals:
  void stateChanged();
  void lastErrorChanged();
  void libraryModelChanged();

private:
  void setState(State state);
  void setLastError(const QString &error);

  State m_state = Locked;
  QString m_lastError;
  QString m_vaultPath;
  QString m_dbPath;
  QObject *m_libraryModel = nullptr;
  bool m_keychainAvailable = false;
};
