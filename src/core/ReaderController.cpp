#include "include/ReaderController.h"

#include <QFileInfo>
#include <QRegularExpression>
#include <QDebug>
#include <QMetaObject>
#include <QPointer>
#include <QSettings>
#include <algorithm>

#include "AsyncUtil.h"
#include "include/AppPaths.h"

namespace {
bool isMobiFormat(const QString &format) {
  const QString f = format.trimmed().toLower();
  return f == "mobi" || f == "azw" || f == "azw3" || f == "azw4" || f == "prc";
}

int clampInt(int value, int minValue, int maxValue) {
  return std::max(minValue, std::min(maxValue, value));
}

int preRenderPagesForFormat(const QString &format) {
  QString key = format.trimmed().toLower();
  if (key.isEmpty()) {
    return 2;
  }
  if (key == "djv") {
    key = "djvu";
  }
  const QString path = AppPaths::configFile(QString("%1.ini").arg(key));
  QSettings settings(path, QSettings::IniFormat);
  return clampInt(settings.value("render/pre_render_pages", 2).toInt(), 1, 12);
}
} // namespace

ReaderController::ReaderController(QObject *parent) : QObject(parent) {
  m_registry = FormatRegistry::createDefault();
}

void ReaderController::clearImageState() {
  if (!m_imagePaths.isEmpty() || m_currentImageIndex != -1) {
    m_imagePaths.clear();
    m_currentImageIndex = -1;
    m_imageReloadToken++;
    emit imageReloadTokenChanged();
    emit currentChanged();
  }
}

bool ReaderController::openFile(const QString &path) {
  if (!m_registry) {
    setLastError("Format registry not available");
    qWarning() << "ReaderController: registry missing";
    return false;
  }

  QString error;
  auto document = m_registry->open(path, &error);
  return applyDocument(std::move(document), path, &error);
}

void ReaderController::openFileAsync(const QString &path) {
  if (path.isEmpty()) {
    setLastError("Path is empty");
    return;
  }
  setBusy(true);
  const int requestId = ++m_openRequestId;
  const QString absPath = QFileInfo(path).absoluteFilePath();
  runInBackground([this, requestId, absPath]() {
    QString error;
    auto registry = FormatRegistry::createDefault();
    auto document = registry->open(absPath, &error);
    QMetaObject::invokeMethod(this, [this, requestId, absPath, error, doc = std::move(document)]() mutable {
      if (requestId != m_openRequestId) {
        return;
      }
      QString localError = error;
      applyDocument(std::move(doc), absPath, &localError);
      setBusy(false);
    }, Qt::QueuedConnection);
  });
}

void ReaderController::close() {
  if (!m_isOpen) {
    return;
  }
  m_document.reset();
  m_currentTitle.clear();
  m_currentText.clear();
  m_currentPlainText.clear();
  m_currentPath.clear();
  m_currentFormat.clear();
  m_chapterTitles.clear();
  m_chapterTexts.clear();
  m_chapterPlainTexts.clear();
  m_tocTitles.clear();
  m_tocChapterIndices.clear();
  m_currentChapterIndex = -1;
  m_imagePaths.clear();
  m_currentImageIndex = -1;
  m_imageReloadToken = 0;
  m_coverPath.clear();
  m_textIsRich = false;
  m_isOpen = false;
  m_ttsAllowed = true;
  qInfo() << "ReaderController: closed";
  emit currentChanged();
}

QString ReaderController::currentTitle() const { return m_currentTitle; }
QString ReaderController::currentText() const { return m_currentText; }
QString ReaderController::currentPlainText() const { return m_currentPlainText; }
bool ReaderController::currentTextIsRich() const { return m_textIsRich; }
QString ReaderController::currentPath() const { return m_currentPath; }
QString ReaderController::currentFormat() const { return m_currentFormat; }
bool ReaderController::isOpen() const { return m_isOpen; }
int ReaderController::currentChapterIndex() const { return m_currentChapterIndex; }
QString ReaderController::currentChapterTitle() const {
  if (m_currentChapterIndex >= 0 && m_currentChapterIndex < m_chapterTitles.size()) {
    return m_chapterTitles.at(m_currentChapterIndex);
  }
  return {};
}
int ReaderController::chapterCount() const { return m_chapterTexts.size(); }
int ReaderController::tocCount() const { return m_tocTitles.size(); }
QString ReaderController::chapterTitle(int index) const {
  if (index >= 0 && index < m_chapterTitles.size()) {
    return m_chapterTitles.at(index);
  }
  return {};
}
QString ReaderController::tocTitle(int index) const {
  if (index >= 0 && index < m_tocTitles.size()) {
    return m_tocTitles.at(index);
  }
  return {};
}
int ReaderController::tocChapterIndex(int index) const {
  if (index >= 0 && index < m_tocChapterIndices.size()) {
    return m_tocChapterIndices.at(index);
  }
  return -1;
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

QUrl ReaderController::imageUrlAt(int index) const {
  if (index < 0 || index >= m_imagePaths.size()) {
    return {};
  }
  const QString path = m_imagePaths.at(index);
  QFileInfo info(path);
  if (info.isAbsolute()) {
    return QUrl::fromLocalFile(path);
  }
  return QUrl(path);
}
int ReaderController::imageReloadToken() const { return m_imageReloadToken; }
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
bool ReaderController::busy() const { return m_busy; }
QString ReaderController::lastError() const { return m_lastError; }
bool ReaderController::ttsAllowed() const { return m_ttsAllowed; }

void ReaderController::setLastError(const QString &error) {
  if (m_lastError == error) {
    return;
  }
  m_lastError = error;
  emit lastErrorChanged();
}

bool ReaderController::applyDocument(std::unique_ptr<FormatDocument> document,
                                     const QString &path,
                                     QString *error) {
  if (!document) {
    setLastError(error && !error->isEmpty() ? *error : "Failed to open document");
    qWarning() << "ReaderController: failed to open" << path << m_lastError;
    return false;
  }

  m_document = std::move(document);
  QPointer<ReaderController> self(this);
  m_document->setImageReadyCallback([self](int index) {
    if (!self) {
      return;
    }
    QMetaObject::invokeMethod(self, [self, index]() {
      if (!self) {
        return;
      }
      if (index == self->m_currentImageIndex) {
        self->m_imageReloadToken++;
        emit self->imageReloadTokenChanged();
        emit self->currentChanged();
      }
    }, Qt::QueuedConnection);
  });
  setLastError("");
  const QFileInfo fileInfo(path);
  m_currentPath = fileInfo.absoluteFilePath();
  m_currentFormat = fileInfo.suffix().trimmed().toLower();
  m_currentTitle = m_document->title();
  m_chapterTitles = m_document->chapterTitles();
  m_chapterTexts = m_document->chaptersText();
  m_chapterPlainTexts = m_document->chaptersPlainText();
  m_tocTitles = m_document->tocTitles();
  m_tocChapterIndices = m_document->tocChapterIndices();
  m_imagePaths = m_document->imagePaths();
  m_coverPath = m_document->coverPath();
  m_textIsRich = m_document->isRichText();
  m_ttsAllowed = !m_document->ttsDisabled();
  if (!m_ttsAllowed) {
    qInfo() << "ReaderController: TTS disabled for this book";
  }
  if (isMobiFormat(m_currentFormat)) {
    m_imagePaths.clear();
  }
  if (!m_imagePaths.isEmpty()) {
    m_currentImageIndex = 0;
    m_imageReloadToken = 0;
    const QString firstImage = m_imagePaths.at(0);
    qInfo() << "ReaderController: loaded" << m_imagePaths.size()
            << "image(s), first:" << firstImage
            << "exists:" << QFileInfo::exists(firstImage);
    const int warmCount = std::min(preRenderPagesForFormat(m_currentFormat),
                                   static_cast<int>(m_imagePaths.size()));
    for (int i = 0; i < warmCount; ++i) {
      m_document->ensureImage(i);
    }
  } else {
    m_currentImageIndex = -1;
    m_imageReloadToken = 0;
  }
  if (!m_chapterTexts.isEmpty()) {
    m_currentChapterIndex = 0;
    m_currentText = m_chapterTexts.at(0);
    if (!m_chapterPlainTexts.isEmpty()) {
      m_currentPlainText = m_chapterPlainTexts.at(0);
    } else {
      m_currentPlainText = m_document->readAllPlainText();
    }
  } else {
    m_currentChapterIndex = -1;
    m_currentText = m_document->readAllText();
    m_currentPlainText = m_document->readAllPlainText();
  }
  if (m_tocTitles.isEmpty()) {
    m_tocTitles = m_chapterTitles;
    m_tocChapterIndices.clear();
    for (int i = 0; i < m_chapterTitles.size(); ++i) {
      m_tocChapterIndices.append(i);
    }
  } else {
    qInfo() << "ReaderController: TOC entries" << m_tocTitles.size()
            << "first" << m_tocTitles.value(0);
  }
  if (!m_coverPath.isEmpty()) {
    qInfo() << "ReaderController: cover" << m_coverPath
            << "exists:" << QFileInfo::exists(m_coverPath);
  }
  qInfo() << "ReaderController: format" << m_currentFormat
          << "hasImages" << !m_imagePaths.isEmpty()
          << "textRich" << m_textIsRich
          << "chapters" << m_chapterTexts.size();
  m_isOpen = true;
  qInfo() << "ReaderController: opened" << m_currentTitle << m_currentPath;
  emit currentChanged();
  return true;
}

void ReaderController::setBusy(bool busy) {
  if (m_busy == busy) {
    return;
  }
  m_busy = busy;
  emit busyChanged();
}

bool ReaderController::jumpToLocator(const QString &locator) {
  QString trimmed = locator.trimmed();
  if (trimmed.isEmpty()) {
    setLastError("Locator not found");
    return false;
  }
  auto parseIndex = [&trimmed](int *outIndex) -> bool {
    bool ok = false;
    int index = trimmed.toInt(&ok);
    if (ok) {
      *outIndex = index;
      return true;
    }
    const QRegularExpression re("(\\d+)");
    const auto match = re.match(trimmed);
    if (match.hasMatch()) {
      *outIndex = match.captured(1).toInt(&ok);
      return ok;
    }
    return false;
  };

  if (!m_chapterTexts.isEmpty()) {
    int index = 0;
    if (parseIndex(&index)) {
      index -= 1; // user-friendly 1-based
      if (index < 0 || index >= m_chapterTexts.size()) {
        setLastError("Chapter index out of range");
        return false;
      }
      m_currentChapterIndex = index;
      m_currentText = m_chapterTexts.at(index);
      if (index < m_chapterPlainTexts.size()) {
        m_currentPlainText = m_chapterPlainTexts.at(index);
      } else {
        m_currentPlainText = m_document ? m_document->readAllPlainText() : m_currentText;
      }
      emit currentChanged();
      return true;
    }
    // Try match by title (case-insensitive contains)
    for (int i = 0; i < m_chapterTitles.size(); ++i) {
      if (m_chapterTitles.at(i).contains(trimmed, Qt::CaseInsensitive)) {
        m_currentChapterIndex = i;
        m_currentText = m_chapterTexts.at(i);
        if (i < m_chapterPlainTexts.size()) {
          m_currentPlainText = m_chapterPlainTexts.at(i);
        } else {
          m_currentPlainText = m_document ? m_document->readAllPlainText() : m_currentText;
        }
        emit currentChanged();
        return true;
      }
    }
  }

  if (!m_imagePaths.isEmpty()) {
    int index = 0;
    if (parseIndex(&index)) {
      index -= 1;
      if (index < 0 || index >= m_imagePaths.size()) {
        setLastError("Page index out of range");
        return false;
      }
      return goToImage(index);
    }
  }

  setLastError("Locator not found");
  return false;
}

bool ReaderController::nextChapter() {
  if (m_chapterTexts.isEmpty()) {
    return false;
  }
  if (m_currentChapterIndex + 1 >= m_chapterTexts.size()) {
    return false;
  }
  return goToChapter(m_currentChapterIndex + 1);
}

bool ReaderController::prevChapter() {
  if (m_chapterTexts.isEmpty()) {
    return false;
  }
  if (m_currentChapterIndex - 1 < 0) {
    return false;
  }
  return goToChapter(m_currentChapterIndex - 1);
}

bool ReaderController::goToChapter(int index) {
  if (m_chapterTexts.isEmpty()) {
    return false;
  }
  if (index < 0 || index >= m_chapterTexts.size()) {
    setLastError("Chapter index out of range");
    return false;
  }
  m_currentChapterIndex = index;
  m_currentText = m_chapterTexts.at(index);
  if (index < m_chapterPlainTexts.size()) {
    m_currentPlainText = m_chapterPlainTexts.at(index);
  } else {
    m_currentPlainText = m_document ? m_document->readAllPlainText() : m_currentText;
  }
  emit currentChanged();
  return true;
}

bool ReaderController::nextImage() {
  if (m_imagePaths.isEmpty()) {
    return false;
  }
  if (m_currentImageIndex + 1 >= m_imagePaths.size()) {
    return false;
  }
  m_currentImageIndex++;
  if (m_currentImageIndex > 0) {
    m_document->ensureImage(m_currentImageIndex - 1);
  }
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
  if (m_currentImageIndex > 0) {
    m_document->ensureImage(m_currentImageIndex - 1);
  }
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
  if (m_currentImageIndex > 0) {
    m_document->ensureImage(m_currentImageIndex - 1);
  }
  m_document->ensureImage(m_currentImageIndex);
  m_document->ensureImage(m_currentImageIndex + 1);
  emit currentChanged();
  return true;
}
