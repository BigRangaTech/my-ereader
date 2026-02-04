#include "include/VaultController.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDebug>

#include "LibraryModel.h"
#include "KeychainStore.h"
#include "include/AppPaths.h"

namespace {
QString appDataDir() {
  return AppPaths::dataRoot();
}
} // namespace

VaultController::VaultController(QObject *parent) : QObject(parent) {
  const QString base = appDataDir();
  m_vaultPath = QDir(base).filePath("library.vault");
  m_dbPath = ":memory:";
  KeychainStore store;
  m_keychainAvailable = store.isAvailable();
}

VaultController::State VaultController::state() const { return m_state; }

QString VaultController::lastError() const { return m_lastError; }

QString VaultController::vaultPath() const { return m_vaultPath; }

QString VaultController::dbPath() const { return m_dbPath; }

bool VaultController::keychainAvailable() const { return m_keychainAvailable; }

QObject *VaultController::libraryModel() const { return m_libraryModel; }

void VaultController::setLibraryModel(QObject *model) {
  if (m_libraryModel == model) {
    return;
  }
  m_libraryModel = model;
  qInfo() << "VaultController: library model set" << (m_libraryModel != nullptr);
  emit libraryModelChanged();
}

void VaultController::initialize() {
  qInfo() << "VaultController: initialize" << m_vaultPath;
  const QString base = appDataDir();
  QDir baseDir(base);
  if (!baseDir.exists()) {
    if (!baseDir.mkpath(".")) {
      setLastError("Failed to create app data directory");
      setState(Error);
      qWarning() << "VaultController: failed to create app data dir" << base;
      return;
    }
  }
  const bool vaultExists = QFile::exists(m_vaultPath);
  if (!vaultExists) {
    qInfo() << "VaultController: vault missing" << m_vaultPath;
    setState(NeedsSetup);
    qInfo() << "VaultController: needs setup";
    return;
  }
  if (m_state != Locked) {
    setState(Locked);
  } else {
    emit stateChanged();
  }
  qInfo() << "VaultController: locked";
}

bool VaultController::unlock(const QString &passphrase) {
  qInfo() << "VaultController: unlock attempt" << m_vaultPath;
  if (!QFile::exists(m_vaultPath)) {
    setLastError("Vault not found");
    setState(NeedsSetup);
    qWarning() << "VaultController: vault missing" << m_vaultPath;
    return false;
  }
  auto *model = qobject_cast<LibraryModel *>(m_libraryModel);
  if (!model) {
    setLastError("Library model not set");
    setState(Error);
    qWarning() << "VaultController: missing library model";
    return false;
  }
  if (!model->openEncryptedVault(m_vaultPath, passphrase)) {
    setLastError(model->lastError());
    setState(Error);
    qWarning() << "VaultController: unlock failed" << m_lastError;
    return false;
  }
  setLastError("");
  setState(Unlocked);
  qInfo() << "VaultController: unlocked";
  return true;
}

bool VaultController::setupNew(const QString &passphrase) {
  qInfo() << "VaultController: setup new vault" << m_vaultPath;
  const QString base = appDataDir();
  QDir baseDir(base);
  if (!baseDir.exists()) {
    if (!baseDir.mkpath(".")) {
      setLastError("Failed to create app data directory");
      setState(Error);
      qWarning() << "VaultController: failed to create app data dir" << base;
      return false;
    }
  }
  auto *model = qobject_cast<LibraryModel *>(m_libraryModel);
  if (!model) {
    setLastError("Library model not set");
    setState(Error);
    qWarning() << "VaultController: missing library model";
    return false;
  }
  if (!model->openAt(":memory:")) {
    setLastError(model->lastError());
    setState(Error);
    qWarning() << "VaultController: openAt failed" << m_lastError;
    return false;
  }
  if (!model->saveEncryptedVault(m_vaultPath, passphrase)) {
    setLastError(model->lastError());
    setState(Error);
    qWarning() << "VaultController: saveEncryptedVault failed" << m_lastError;
    return false;
  }
  model->close();
  setLastError("");
  setState(Locked);
  qInfo() << "VaultController: setup complete";
  return true;
}

bool VaultController::lock(const QString &passphrase) {
  auto *model = qobject_cast<LibraryModel *>(m_libraryModel);
  if (!model) {
    setLastError("Library model not set");
    setState(Error);
    return false;
  }
  if (!model->saveEncryptedVault(m_vaultPath, passphrase)) {
    setLastError(model->lastError());
    setState(Error);
    return false;
  }
  model->close();
  setLastError("");
  setState(Locked);
  qInfo() << "VaultController: locked";
  return true;
}

QString VaultController::loadStoredPassphrase() {
  KeychainStore store;
  if (!store.isAvailable()) {
    setLastError("Keychain unavailable");
    return {};
  }
  QString error;
  const QString pass = store.loadPassphrase(&error);
  if (!error.isEmpty()) {
    setLastError(error);
  }
  return pass;
}

bool VaultController::storePassphrase(const QString &passphrase) {
  KeychainStore store;
  if (!store.isAvailable()) {
    setLastError("Keychain unavailable");
    qWarning() << "VaultController: keychain unavailable for store";
    return false;
  }
  QString error;
  const bool ok = store.storePassphrase(passphrase, &error);
  if (!ok) {
    setLastError(error);
    qWarning() << "VaultController: storePassphrase failed" << error;
  }
  return ok;
}

bool VaultController::clearStoredPassphrase() {
  KeychainStore store;
  if (!store.isAvailable()) {
    setLastError("Keychain unavailable");
    qWarning() << "VaultController: keychain unavailable for clear";
    return false;
  }
  QString error;
  const bool ok = store.clearPassphrase(&error);
  if (!ok) {
    setLastError(error);
    qWarning() << "VaultController: clearStoredPassphrase failed" << error;
  }
  return ok;
}

void VaultController::setState(State state) {
  if (m_state == state) {
    return;
  }
  m_state = state;
  emit stateChanged();
}

void VaultController::setLastError(const QString &error) {
  if (m_lastError == error) {
    return;
  }
  m_lastError = error;
  emit lastErrorChanged();
}
