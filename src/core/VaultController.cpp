#include "include/VaultController.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDebug>

#include "LibraryModel.h"

namespace {
QString appDataDir() {
  return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}
} // namespace

VaultController::VaultController(QObject *parent) : QObject(parent) {
  const QString base = appDataDir();
  m_vaultPath = QDir(base).filePath("library.vault");
  m_dbPath = ":memory:";
}

VaultController::State VaultController::state() const { return m_state; }

QString VaultController::lastError() const { return m_lastError; }

QString VaultController::vaultPath() const { return m_vaultPath; }

QString VaultController::dbPath() const { return m_dbPath; }

QObject *VaultController::libraryModel() const { return m_libraryModel; }

void VaultController::setLibraryModel(QObject *model) {
  if (m_libraryModel == model) {
    return;
  }
  m_libraryModel = model;
  emit libraryModelChanged();
}

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
  auto *model = qobject_cast<LibraryModel *>(m_libraryModel);
  if (!model) {
    setLastError("Library model not set");
    setState(Error);
    return false;
  }
  if (!model->openEncryptedVault(m_vaultPath, passphrase)) {
    setLastError(model->lastError());
    setState(Error);
    return false;
  }
  setLastError("");
  setState(Unlocked);
  qInfo() << "VaultController: unlocked";
  return true;
}

bool VaultController::setupNew(const QString &passphrase) {
  auto *model = qobject_cast<LibraryModel *>(m_libraryModel);
  if (!model) {
    setLastError("Library model not set");
    setState(Error);
    return false;
  }
  if (!model->openAt(":memory:")) {
    setLastError(model->lastError());
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
