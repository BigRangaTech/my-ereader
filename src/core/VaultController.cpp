#include "include/VaultController.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDebug>

#include "CryptoBackend.h"
#include "CryptoVault.h"

namespace {
QString appDataDir() {
  return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}
} // namespace

VaultController::VaultController(QObject *parent) : QObject(parent) {
  const QString base = appDataDir();
  m_vaultPath = QDir(base).filePath("library.vault");
  m_dbPath = QDir(base).filePath("library.db");
}

VaultController::State VaultController::state() const { return m_state; }

QString VaultController::lastError() const { return m_lastError; }

QString VaultController::vaultPath() const { return m_vaultPath; }

QString VaultController::dbPath() const { return m_dbPath; }

void VaultController::initialize() {
  const bool vaultExists = QFile::exists(m_vaultPath);
  if (!vaultExists) {
    setState(NeedsSetup);
    qInfo() << "VaultController: needs setup";
    return;
  }
  setState(Locked);
  qInfo() << "VaultController: locked";
}

bool VaultController::unlock(const QString &passphrase) {
  if (!QFile::exists(m_vaultPath)) {
    setLastError("Vault not found");
    setState(NeedsSetup);
    qWarning() << "VaultController: vault missing" << m_vaultPath;
    return false;
  }
  auto backend = CryptoBackendFactory::createDefault();
  CryptoVault vault(std::move(backend));
  QString error;
  if (!vault.decryptFile(m_vaultPath, m_dbPath, passphrase, &error)) {
    setLastError(error.isEmpty() ? "Failed to decrypt vault" : error);
    setState(Error);
    qWarning() << "VaultController: decrypt failed" << m_lastError;
    return false;
  }
  setLastError("");
  setState(Unlocked);
  qInfo() << "VaultController: unlocked";
  return true;
}

bool VaultController::setupNew(const QString &passphrase) {
  QDir().mkpath(QFileInfo(m_dbPath).absolutePath());
  QFile dbFile(m_dbPath);
  if (!dbFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    setLastError("Failed to create database");
    setState(Error);
    qWarning() << "VaultController: failed to create db" << m_dbPath;
    return false;
  }
  dbFile.close();

  auto backend = CryptoBackendFactory::createDefault();
  CryptoVault vault(std::move(backend));
  QString error;
  if (!vault.encryptFile(m_dbPath, m_vaultPath, passphrase, &error)) {
    setLastError(error.isEmpty() ? "Failed to encrypt vault" : error);
    setState(Error);
    qWarning() << "VaultController: encrypt failed" << m_lastError;
    return false;
  }
  QFile::remove(m_dbPath);
  setLastError("");
  setState(Locked);
  qInfo() << "VaultController: setup complete";
  return true;
}

bool VaultController::lock(const QString &passphrase) {
  if (!QFile::exists(m_dbPath)) {
    setState(Locked);
    return true;
  }
  auto backend = CryptoBackendFactory::createDefault();
  CryptoVault vault(std::move(backend));
  QString error;
  if (!vault.encryptFile(m_dbPath, m_vaultPath, passphrase, &error)) {
    setLastError(error.isEmpty() ? "Failed to encrypt vault" : error);
    setState(Error);
    qWarning() << "VaultController: lock encrypt failed" << m_lastError;
    return false;
  }
  QFile::remove(m_dbPath);
  setLastError("");
  setState(Locked);
  qInfo() << "VaultController: locked";
  return true;
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
