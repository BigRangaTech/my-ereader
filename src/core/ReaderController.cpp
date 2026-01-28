#include "include/ReaderController.h"

#include <QFileInfo>
#include <QDebug>

ReaderController::ReaderController(QObject *parent) : QObject(parent) {
  m_registry = FormatRegistry::createDefault();
}

bool ReaderController::openFile(const QString &path) {
  if (!m_registry) {
    setLastError("Format registry not available");
    qWarning() << "ReaderController: registry missing";
    return false;
  }

  QString error;
  m_document = m_registry->open(path, &error);
  if (!m_document) {
    setLastError(error.isEmpty() ? "Failed to open document" : error);
    qWarning() << "ReaderController: failed to open" << path << m_lastError;
    return false;
  }

  setLastError("");
  m_currentTitle = m_document->title();
  m_chapterTitles = m_document->chapterTitles();
  m_chapterTexts = m_document->chaptersText();
  m_imagePaths = m_document->imagePaths();
  m_coverPath = m_document->coverPath();
  if (!m_imagePaths.isEmpty()) {
    m_currentImageIndex = 0;
    const QString firstImage = m_imagePaths.at(0);
    qInfo() << "ReaderController: loaded" << m_imagePaths.size()
            << "image(s), first:" << firstImage
            << "exists:" << QFileInfo::exists(firstImage);
    m_document->ensureImage(0);
    m_document->ensureImage(1);
  } else {
    m_currentImageIndex = -1;
  }
  if (!m_chapterTexts.isEmpty()) {
    m_currentChapterIndex = 0;
    m_currentText = m_chapterTexts.at(0);
  } else {
    m_currentChapterIndex = -1;
    m_currentText = m_document->readAllText();
  }
  m_currentPath = QFileInfo(path).absoluteFilePath();
  if (!m_coverPath.isEmpty()) {
    qInfo() << "ReaderController: cover" << m_coverPath
            << "exists:" << QFileInfo::exists(m_coverPath);
  }
  m_isOpen = true;
  qInfo() << "ReaderController: opened" << m_currentTitle << m_currentPath;
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
  m_chapterTitles.clear();
  m_chapterTexts.clear();
  m_currentChapterIndex = -1;
  m_imagePaths.clear();
  m_currentImageIndex = -1;
  m_coverPath.clear();
  m_isOpen = false;
  qInfo() << "ReaderController: closed";
  emit currentChanged();
}

QString ReaderController::currentTitle() const { return m_currentTitle; }
QString ReaderController::currentText() const { return m_currentText; }
QString ReaderController::currentPath() const { return m_currentPath; }
bool ReaderController::isOpen() const { return m_isOpen; }
int ReaderController::currentChapterIndex() const { return m_currentChapterIndex; }
QString ReaderController::currentChapterTitle() const {
  if (m_currentChapterIndex >= 0 && m_currentChapterIndex < m_chapterTitles.size()) {
    return m_chapterTitles.at(m_currentChapterIndex);
  }
  return {};
}
bool ReaderController::hasImages() const { return !m_imagePaths.isEmpty(); }
int ReaderController::currentImageIndex() const { return m_currentImageIndex; }
int ReaderController::imageCount() const { return m_imagePaths.size(); }
QString ReaderController::currentImagePath() const {
  if (m_currentImageIndex >= 0 && m_currentImageIndex < m_imagePaths.size()) {
    return m_imagePaths.at(m_currentImageIndex);
  }
  return {};
}
QUrl ReaderController::currentImageUrl() const {
  const QString path = currentImagePath();
  if (path.isEmpty()) {
    return {};
  }
  QFileInfo info(path);
  if (info.isAbsolute()) {
    return QUrl::fromLocalFile(path);
  }
  return QUrl(path);
}
QString ReaderController::currentCoverPath() const { return m_coverPath; }
QUrl ReaderController::currentCoverUrl() const {
  if (m_coverPath.isEmpty()) {
    return {};
  }
  QFileInfo info(m_coverPath);
  if (info.isAbsolute()) {
    return QUrl::fromLocalFile(m_coverPath);
  }
  return QUrl(m_coverPath);
}
QString ReaderController::lastError() const { return m_lastError; }

void ReaderController::setLastError(const QString &error) {
  if (m_lastError == error) {
    return;
  }
  m_lastError = error;
  emit lastErrorChanged();
}

bool ReaderController::jumpToLocator(const QString &locator) {
  if (m_chapterTexts.isEmpty()) {
    setLastError("No chapter navigation available");
    return false;
  }
  QString trimmed = locator.trimmed();
  bool ok = false;
  int index = trimmed.toInt(&ok);
  if (ok) {
    index -= 1; // user-friendly 1-based
    if (index < 0 || index >= m_chapterTexts.size()) {
      setLastError("Chapter index out of range");
      return false;
    }
    m_currentChapterIndex = index;
    m_currentText = m_chapterTexts.at(index);
    emit currentChanged();
    return true;
  }
  // Try match by title (case-insensitive contains)
  for (int i = 0; i < m_chapterTitles.size(); ++i) {
    if (m_chapterTitles.at(i).contains(trimmed, Qt::CaseInsensitive)) {
      m_currentChapterIndex = i;
      m_currentText = m_chapterTexts.at(i);
      emit currentChanged();
      return true;
    }
  }
  setLastError("Locator not found");
  return false;
}

bool ReaderController::nextImage() {
  if (m_imagePaths.isEmpty()) {
    return false;
  }
  if (m_currentImageIndex + 1 >= m_imagePaths.size()) {
    return false;
  }
  m_currentImageIndex++;
  m_document->ensureImage(m_currentImageIndex);
  m_document->ensureImage(m_currentImageIndex + 1);
  emit currentChanged();
  return true;
}

bool ReaderController::prevImage() {
  if (m_imagePaths.isEmpty()) {
    return false;
  }
  if (m_currentImageIndex - 1 < 0) {
    return false;
  }
  m_currentImageIndex--;
  m_document->ensureImage(m_currentImageIndex);
  m_document->ensureImage(m_currentImageIndex + 1);
  emit currentChanged();
  return true;
}

bool ReaderController::goToImage(int index) {
  if (m_imagePaths.isEmpty()) {
    return false;
  }
  if (index < 0 || index >= m_imagePaths.size()) {
    return false;
  }
  if (m_currentImageIndex == index) {
    return true;
  }
  m_currentImageIndex = index;
  m_document->ensureImage(m_currentImageIndex);
  m_document->ensureImage(m_currentImageIndex + 1);
  emit currentChanged();
  return true;
}
