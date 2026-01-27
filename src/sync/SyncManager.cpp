#include "include/SyncManager.h"

SyncManager::SyncManager(QObject *parent) : QObject(parent) {}

bool SyncManager::enabled() const { return m_enabled; }

void SyncManager::setEnabled(bool enabled) {
  if (m_enabled == enabled) {
    return;
  }
  m_enabled = enabled;
  emit enabledChanged();
}

QString SyncManager::status() const { return m_status; }

void SyncManager::startDiscovery() {
  if (!m_enabled) {
    setStatus("Sync disabled");
    return;
  }
  setStatus("Discovering devices...");
}

void SyncManager::stopDiscovery() {
  setStatus("Idle");
}

void SyncManager::requestPairing(const QString &pin) {
  if (!m_enabled) {
    setStatus("Sync disabled");
    return;
  }
  if (pin.trimmed().isEmpty()) {
    setStatus("Invalid PIN");
    return;
  }
  setStatus("Pairing requested");
}

void SyncManager::setStatus(const QString &status) {
  if (m_status == status) {
    return;
  }
  m_status = status;
  emit statusChanged();
}
