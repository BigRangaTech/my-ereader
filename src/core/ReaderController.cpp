#include "include/ReaderController.h"

#include <QFileInfo>

ReaderController::ReaderController(QObject *parent) : QObject(parent) {
  m_registry = FormatRegistry::createDefault();
}

bool ReaderController::openFile(const QString &path) {
  if (!m_registry) {
    setLastError("Format registry not available");
    return false;
  }

  QString error;
  m_document = m_registry->open(path, &error);
  if (!m_document) {
    setLastError(error.isEmpty() ? "Failed to open document" : error);
    return false;
  }

  setLastError("");
  m_currentTitle = m_document->title();
  m_currentText = m_document->readAllText();
  m_currentPath = QFileInfo(path).absoluteFilePath();
  m_isOpen = true;
  emit currentChanged();
  return true;
}

void ReaderController::close() {
  if (!m_isOpen) {
    return;
  }
  m_document.reset();
  m_currentTitle.clear();
  m_currentText.clear();
  m_currentPath.clear();
  m_isOpen = false;
  emit currentChanged();
}

QString ReaderController::currentTitle() const { return m_currentTitle; }
QString ReaderController::currentText() const { return m_currentText; }
QString ReaderController::currentPath() const { return m_currentPath; }
bool ReaderController::isOpen() const { return m_isOpen; }
QString ReaderController::lastError() const { return m_lastError; }

void ReaderController::setLastError(const QString &error) {
  if (m_lastError == error) {
    return;
  }
  m_lastError = error;
  emit lastErrorChanged();
}
