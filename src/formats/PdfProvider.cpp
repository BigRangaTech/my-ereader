#include "PdfProvider.h"

#ifdef HAVE_POPPLER_QT6
#include <poppler-qt6.h>
#endif

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QCoreApplication>
#include <QSettings>
#include <QStandardPaths>
#include <QDebug>
#include <QList>
#include <QMutex>
#include <QMutexLocker>
#include <QSet>
#include <memory>
#include <functional>
#include <algorithm>
#include <QRunnable>
#include <QThreadPool>

namespace {
QString settingsPath() {
  QDir dir(QCoreApplication::applicationDirPath());
  for (int i = 0; i < 6; ++i) {
    if (QFileInfo::exists(dir.filePath("README.md"))) {
      return dir.filePath("config/settings.ini");
    }
    if (!dir.cdUp()) {
      break;
    }
  }
  return QDir(QCoreApplication::applicationDirPath()).filePath("settings.ini");
}

int clampInt(int value, int minValue, int maxValue) {
  return std::max(minValue, std::min(maxValue, value));
}

struct PdfSettings {
  int dpi = 120;
  int cacheLimit = 30;
};

PdfSettings loadPdfSettings() {
  QSettings settings(settingsPath(), QSettings::IniFormat);
  PdfSettings out;
  out.dpi = clampInt(settings.value("pdf/dpi", 120).toInt(), 72, 240);
  out.cacheLimit = clampInt(settings.value("pdf/cache_limit", 30).toInt(), 5, 120);
  return out;
}

class PdfRunnable final : public QRunnable {
public:
  explicit PdfRunnable(std::function<void()> task) : m_task(std::move(task)) {}
  void run() override {
    if (m_task) {
      m_task();
    }
  }

private:
  std::function<void()> m_task;
};

void runPdfTask(std::function<void()> task) {
  auto *runnable = new PdfRunnable(std::move(task));
  runnable->setAutoDelete(true);
  QThreadPool::globalInstance()->start(runnable);
}

struct PdfRenderState {
  std::unique_ptr<Poppler::Document> doc;
  QStringList images;
  QString tempDir;
  int cacheLimit = 30;
  double renderDpi = 120.0;
  QSet<int> cached;
  QList<int> cacheOrder;
  QSet<int> inFlight;
  std::function<void(int)> onImageReady;
  QMutex mutex;
  bool alive = true;
};

class PdfDocument final : public FormatDocument {
public:
  PdfDocument(QString title,
              QString text,
              std::shared_ptr<PdfRenderState> state)
      : m_title(std::move(title)),
        m_text(std::move(text)),
        m_state(std::move(state)) {}

  ~PdfDocument() override {
    if (m_state) {
      QMutexLocker locker(&m_state->mutex);
      m_state->alive = false;
      m_state->onImageReady = nullptr;
    }
  }

  QString title() const override { return m_title; }
  QStringList chapterTitles() const override { return {}; }
  QString readAllText() const override { return m_text; }
  QStringList imagePaths() const override { return m_state ? m_state->images : QStringList{}; }
  bool ensureImage(int index) override {
    if (!m_state) {
      return false;
    }
    if (index < 0 || index >= m_state->images.size()) {
      return false;
    }
    {
      QMutexLocker locker(&m_state->mutex);
      if (!m_state->alive) {
        return false;
      }
      const QString path = m_state->images.at(index);
      if (m_state->cached.contains(index) && QFileInfo::exists(path)) {
        return true;
      }
      if (m_state->inFlight.contains(index)) {
        return false;
      }
      m_state->inFlight.insert(index);
    }
    runPdfTask([this, index]() {
      std::shared_ptr<PdfRenderState> state = m_state;
      if (!state) {
        return;
      }
      std::unique_ptr<Poppler::Page> page;
      {
        QMutexLocker locker(&state->mutex);
        if (!state->alive || !state->doc) {
          state->inFlight.remove(index);
          return;
        }
        page = std::unique_ptr<Poppler::Page>(state->doc->page(index));
      }
      if (!page) {
        QMutexLocker locker(&state->mutex);
        state->inFlight.remove(index);
        return;
      }
      const QImage image = page->renderToImage(state->renderDpi, state->renderDpi);
      if (image.isNull()) {
        QMutexLocker locker(&state->mutex);
        state->inFlight.remove(index);
        return;
      }
      const QString path = state->images.at(index);
      QDir().mkpath(state->tempDir);
      const bool saved = image.save(path, "PNG");
      std::function<void(int)> callback;
      {
        QMutexLocker locker(&state->mutex);
        if (!state->alive) {
          state->inFlight.remove(index);
          return;
        }
        if (saved) {
          addToCache(state, index);
        }
        state->inFlight.remove(index);
        callback = state->onImageReady;
      }
      if (callback) {
        callback(index);
      }
    });
    return false;
  }
  void setImageReadyCallback(std::function<void(int)> callback) override {
    if (!m_state) {
      return;
    }
    QMutexLocker locker(&m_state->mutex);
    m_state->onImageReady = std::move(callback);
  }

private:
  static void addToCache(const std::shared_ptr<PdfRenderState> &state, int index) {
    if (!state || state->cached.contains(index)) {
      return;
    }
    state->cached.insert(index);
    state->cacheOrder.append(index);
    while (state->cacheOrder.size() > state->cacheLimit) {
      const int evict = state->cacheOrder.takeFirst();
      state->cached.remove(evict);
      if (evict >= 0 && evict < state->images.size()) {
        QFile::remove(state->images.at(evict));
      }
    }
  }

  QString m_title;
  QString m_text;
  std::shared_ptr<PdfRenderState> m_state;
};

QString tempDirForPdf(const QFileInfo &info) {
  const QString key = QString("%1|%2|%3")
                          .arg(info.absoluteFilePath())
                          .arg(info.size())
                          .arg(info.lastModified().toSecsSinceEpoch());
  const QByteArray hash = QCryptographicHash::hash(key.toUtf8(), QCryptographicHash::Sha1).toHex();
  return QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation))
      .filePath(QString("ereader_pdf_%1").arg(QString::fromUtf8(hash)));
}
} // namespace

QString PdfProvider::name() const { return "PDF"; }

QStringList PdfProvider::supportedExtensions() const { return {"pdf"}; }

std::unique_ptr<FormatDocument> PdfProvider::open(const QString &path, QString *error) {
#ifndef HAVE_POPPLER_QT6
  if (error) {
    *error = "Poppler Qt6 backend not available";
  }
  qWarning() << "PdfProvider: Poppler Qt6 not available";
  Q_UNUSED(path)
  return nullptr;
#else
  std::unique_ptr<Poppler::Document> doc(Poppler::Document::load(path));
  if (!doc) {
    if (error) {
      *error = "Failed to open PDF";
    }
    qWarning() << "PdfProvider: failed to open" << path;
    return nullptr;
  }

  doc->setRenderHint(Poppler::Document::TextAntialiasing, true);
  doc->setRenderHint(Poppler::Document::Antialiasing, true);

  QStringList pages;
  QStringList images;
  const int pageCount = doc->numPages();
  pages.reserve(pageCount);
  images.reserve(pageCount);
  const QFileInfo info(path);
  const QString outDir = tempDirForPdf(info);
  QDir().mkpath(outDir);
  const PdfSettings pdfSettings = loadPdfSettings();
  const int cacheLimit = pdfSettings.cacheLimit;
  const double renderDpi = static_cast<double>(pdfSettings.dpi);
  for (int i = 0; i < pageCount; ++i) {
    std::unique_ptr<Poppler::Page> page(doc->page(i));
    if (!page) {
      continue;
    }
    pages.append(page->text(QRectF()));
    const QString outPath =
        QDir(outDir).filePath(QString("page_%1.png").arg(i + 1, 4, 10, QLatin1Char('0')));
    images.append(outPath);
  }

  const QString title = !doc->info("Title").isEmpty() ? doc->info("Title")
                                                     : QFileInfo(path).completeBaseName();
  auto state = std::make_shared<PdfRenderState>();
  state->doc = std::move(doc);
  state->images = images;
  state->tempDir = outDir;
  state->cacheLimit = cacheLimit;
  state->renderDpi = renderDpi;

  auto pdfDoc = std::make_unique<PdfDocument>(title, pages.join("\n\n"), state);
  pdfDoc->ensureImage(0);
  pdfDoc->ensureImage(1);
  return pdfDoc;
#endif
}
