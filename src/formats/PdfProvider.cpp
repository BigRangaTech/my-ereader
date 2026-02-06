#include "PdfProvider.h"
#include "../core/include/AppPaths.h"

#ifdef HAVE_POPPLER_QT6
#include <poppler-qt6.h>
#endif
#ifdef HAVE_QT_PDF
#include <QPdfDocument>
#include <QPdfDocumentRenderOptions>
#include <QPdfSelection>
#endif

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QPainter>
#include <QSettings>
#include <QStandardPaths>
#include <QDebug>
#include <QList>
#include <QMutex>
#include <QMutexLocker>
#include <QSet>
#include <QColor>
#include <QRect>
#include <QSizeF>
#include <QVariant>
#include <memory>
#include <functional>
#include <algorithm>
#include <cmath>
#include <QRunnable>
#include <QThreadPool>

namespace {
QString settingsPath() {
  return AppPaths::configFile("settings.ini");
}

QString formatSettingsPath() {
  return AppPaths::configFile("pdf.ini");
}

int clampInt(int value, int minValue, int maxValue) {
  return std::max(minValue, std::min(maxValue, value));
}

struct PdfSettings {
  int dpi = 120;
  int cacheLimit = 30;
  int prefetchDistance = 1;
  QString prefetchStrategy = "symmetric";
  QString cachePolicy = "fifo";
  QString renderPreset = "custom";
  bool antialias = true;
  bool textAntialias = true;
  QString colorMode = "color";
  QString backgroundMode = "white";
  QColor backgroundColor = QColor("#202633");
  int maxWidth = 0;
  int maxHeight = 0;
  QString imageFormat = "png";
  int jpegQuality = 85;
  bool extractText = true;
  int tileSize = 0;
  bool progressive = false;
  int progressiveDpi = 72;
};

PdfSettings loadPdfSettings() {
  QSettings formatSettings(formatSettingsPath(), QSettings::IniFormat);
  QSettings legacySettings(settingsPath(), QSettings::IniFormat);
  PdfSettings out;
  out.renderPreset = formatSettings.value("render/preset", "custom").toString().toLower();
  if (out.renderPreset != "custom" && out.renderPreset != "fast" &&
      out.renderPreset != "balanced" && out.renderPreset != "high") {
    out.renderPreset = "custom";
  }

  out.dpi = clampInt(
      formatSettings.value("render/dpi", legacySettings.value("pdf/dpi", 120)).toInt(), 72, 240);
  if (out.renderPreset == "fast") {
    out.dpi = 90;
    out.antialias = false;
    out.textAntialias = false;
  } else if (out.renderPreset == "balanced") {
    out.dpi = 120;
    out.antialias = true;
    out.textAntialias = true;
  } else if (out.renderPreset == "high") {
    out.dpi = 180;
    out.antialias = true;
    out.textAntialias = true;
  }

  out.cacheLimit = clampInt(
      formatSettings.value("render/cache_limit", legacySettings.value("pdf/cache_limit", 30)).toInt(), 5, 120);
  out.prefetchDistance = clampInt(formatSettings.value("render/prefetch_distance", 1).toInt(), 0, 6);
  out.prefetchStrategy = formatSettings.value("render/prefetch_strategy", "symmetric").toString().toLower();
  if (out.prefetchStrategy != "forward" && out.prefetchStrategy != "symmetric" &&
      out.prefetchStrategy != "backward") {
    out.prefetchStrategy = "symmetric";
  }
  out.cachePolicy = formatSettings.value("render/cache_policy", "fifo").toString().toLower();
  if (out.cachePolicy != "fifo" && out.cachePolicy != "lru") {
    out.cachePolicy = "fifo";
  }
  out.colorMode = formatSettings.value("render/color_mode", "color").toString().toLower();
  if (out.colorMode != "color" && out.colorMode != "grayscale") {
    out.colorMode = "color";
  }
  out.backgroundMode = formatSettings.value("render/background_mode", "white").toString().toLower();
  if (out.backgroundMode != "white" && out.backgroundMode != "transparent" &&
      out.backgroundMode != "theme" && out.backgroundMode != "custom") {
    out.backgroundMode = "white";
  }
  const QString bgColor = formatSettings.value("render/background_color", "#202633").toString();
  out.backgroundColor = QColor(bgColor);
  if (!out.backgroundColor.isValid()) {
    out.backgroundColor = QColor("#202633");
  }
  out.maxWidth = clampInt(formatSettings.value("render/max_width", 0).toInt(), 0, 20000);
  out.maxHeight = clampInt(formatSettings.value("render/max_height", 0).toInt(), 0, 20000);
  out.imageFormat = formatSettings.value("render/image_format", "png").toString().toLower();
  if (out.imageFormat == "jpg") {
    out.imageFormat = "jpeg";
  }
  if (out.imageFormat != "png" && out.imageFormat != "jpeg") {
    out.imageFormat = "png";
  }
  out.jpegQuality = clampInt(formatSettings.value("render/jpeg_quality", 85).toInt(), 1, 100);
  out.extractText = formatSettings.value("render/extract_text", true).toBool();
  out.tileSize = clampInt(formatSettings.value("render/tile_size", 0).toInt(), 0, 8192);
  out.progressive = formatSettings.value("render/progressive", false).toBool();
  out.progressiveDpi = clampInt(formatSettings.value("render/progressive_dpi", 72).toInt(), 48, out.dpi);
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
#ifdef HAVE_POPPLER_QT6
  std::unique_ptr<Poppler::Document> doc;
#elif defined(HAVE_QT_PDF)
  std::unique_ptr<QPdfDocument> doc;
  QMutex renderMutex;
#else
  void *doc = nullptr;
#endif
  QStringList images;
  QString tempDir;
  int cacheLimit = 30;
  double renderDpi = 120.0;
  double progressiveDpi = 72.0;
  int prefetchDistance = 1;
  QString prefetchStrategy = "symmetric";
  QString cachePolicy = "fifo";
  QString renderPreset = "custom";
  bool antialias = true;
  bool textAntialias = true;
  QString colorMode = "color";
  QString backgroundMode = "white";
  QColor backgroundColor = QColor("#202633");
  int maxWidth = 0;
  int maxHeight = 0;
  QString imageFormat = "png";
  int jpegQuality = 85;
  int tileSize = 0;
  bool progressive = false;
  QSet<int> cached;
  QSet<int> highResCached;
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
    const int total = m_state->images.size();
    if (index < 0 || index >= total) {
      return false;
    }
    int start = index;
    int end = index;
    {
      QMutexLocker locker(&m_state->mutex);
      if (!m_state->alive) {
        return false;
      }
      const int dist = m_state->prefetchDistance;
      if (m_state->prefetchStrategy == "forward") {
        start = index;
        end = std::min(total - 1, index + dist);
      } else if (m_state->prefetchStrategy == "backward") {
        start = std::max(0, index - dist);
        end = index;
      } else {
        start = std::max(0, index - dist);
        end = std::min(total - 1, index + dist);
      }
    }
    bool queued = false;
    for (int i = start; i <= end; ++i) {
      queued = queueRender(i) || queued;
    }
    return queued;
  }
  void setImageReadyCallback(std::function<void(int)> callback) override {
    if (!m_state) {
      return;
    }
    QMutexLocker locker(&m_state->mutex);
    m_state->onImageReady = std::move(callback);
  }

private:
  static void touchCache(const std::shared_ptr<PdfRenderState> &state, int index) {
    if (!state || !state->cached.contains(index)) {
      return;
    }
    if (state->cachePolicy != "lru") {
      return;
    }
    state->cacheOrder.removeAll(index);
    state->cacheOrder.append(index);
  }

  bool queueRender(int index) {
    std::shared_ptr<PdfRenderState> state = m_state;
    if (!state) {
      return false;
    }
    if (index < 0 || index >= state->images.size()) {
      return false;
    }
    {
      QMutexLocker locker(&state->mutex);
      if (!state->alive || !state->doc) {
        return false;
      }
      const QString path = state->images.at(index);
      const bool needHigh = state->progressive && !state->highResCached.contains(index);
      if (state->cached.contains(index) && QFileInfo::exists(path) && !needHigh) {
        touchCache(state, index);
        return true;
      }
      if (state->inFlight.contains(index)) {
        return false;
      }
      state->inFlight.insert(index);
    }
    runPdfTask([state, index]() {
      if (!state) {
        return;
      }
#ifdef HAVE_POPPLER_QT6
      std::unique_ptr<Poppler::Page> page;
#endif
      QString path;
      double highDpi = 120.0;
      double lowDpi = 72.0;
      bool progressive = false;
      bool haveHigh = false;
      bool antialias = true;
      bool textAntialias = true;
      QString colorMode;
      QString backgroundMode;
      QColor backgroundColor;
      int maxWidth = 0;
      int maxHeight = 0;
      QString imageFormat;
      int jpegQuality = 85;
      int tileSize = 0;
      {
        QMutexLocker locker(&state->mutex);
        if (!state->alive || !state->doc) {
          state->inFlight.remove(index);
          return;
        }
#ifdef HAVE_POPPLER_QT6
        page = std::unique_ptr<Poppler::Page>(state->doc->page(index));
#endif
        path = state->images.at(index);
        highDpi = state->renderDpi;
        lowDpi = state->progressiveDpi;
        progressive = state->progressive;
        haveHigh = state->highResCached.contains(index);
        antialias = state->antialias;
        textAntialias = state->textAntialias;
        colorMode = state->colorMode;
        backgroundMode = state->backgroundMode;
        backgroundColor = state->backgroundColor;
        maxWidth = state->maxWidth;
        maxHeight = state->maxHeight;
        imageFormat = state->imageFormat;
        jpegQuality = state->jpegQuality;
        tileSize = state->tileSize;
      }
#ifdef HAVE_POPPLER_QT6
      if (!page) {
        QMutexLocker locker(&state->mutex);
        state->inFlight.remove(index);
        return;
      }
#endif

      auto notifyReady = [state, index]() {
        std::function<void(int)> callback;
        {
          QMutexLocker locker(&state->mutex);
          if (!state->alive) {
            return;
          }
          callback = state->onImageReady;
        }
        if (callback) {
          callback(index);
        }
      };

      const bool fileExists = QFileInfo::exists(path);
      const bool renderLow = progressive && !fileExists;
      const bool renderHigh = !progressive || !haveHigh;

      auto renderPageImage = [&](double dpi) -> QImage {
#ifdef HAVE_POPPLER_QT6
        const QSizeF pageSize = page->pageSizeF();
        const int pixelWidth = std::max(1, static_cast<int>(std::ceil(pageSize.width() * dpi / 72.0)));
        const int pixelHeight = std::max(1, static_cast<int>(std::ceil(pageSize.height() * dpi / 72.0)));
        QImage image;
        if (tileSize > 0 && (pixelWidth > tileSize || pixelHeight > tileSize)) {
          image = QImage(pixelWidth, pixelHeight, QImage::Format_ARGB32_Premultiplied);
          image.fill(Qt::transparent);
          QPainter painter(&image);
          for (int y = 0; y < pixelHeight; y += tileSize) {
            for (int x = 0; x < pixelWidth; x += tileSize) {
              const int w = std::min(tileSize, pixelWidth - x);
              const int h = std::min(tileSize, pixelHeight - y);
              const QImage tile = page->renderToImage(dpi, dpi, x, y, w, h);
              if (!tile.isNull()) {
                painter.drawImage(x, y, tile);
              }
            }
          }
        } else {
          image = page->renderToImage(dpi, dpi);
        }

        if (image.isNull()) {
          return image;
        }

        if (backgroundMode != "transparent") {
          QColor fill = Qt::white;
          if (backgroundMode == "theme" || backgroundMode == "custom") {
            if (backgroundColor.isValid()) {
              fill = backgroundColor;
            }
          }
          QImage composed(image.size(), QImage::Format_ARGB32_Premultiplied);
          composed.fill(fill);
          QPainter painter(&composed);
          painter.drawImage(0, 0, image);
          image = composed;
        }

        if (colorMode == "grayscale") {
          image = image.convertToFormat(QImage::Format_Grayscale8);
        }

        if ((maxWidth > 0 && image.width() > maxWidth) ||
            (maxHeight > 0 && image.height() > maxHeight)) {
          const int targetW = maxWidth > 0 ? maxWidth : image.width();
          const int targetH = maxHeight > 0 ? maxHeight : image.height();
          image = image.scaled(targetW, targetH, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        return image;
#elif defined(HAVE_QT_PDF)
        if (!state->doc) {
          return {};
        }
        const QSizeF pageSize = state->doc->pagePointSize(index);
        if (pageSize.isEmpty()) {
          return {};
        }
        const int pixelWidth = std::max(1, static_cast<int>(std::ceil(pageSize.width() * dpi / 72.0)));
        const int pixelHeight = std::max(1, static_cast<int>(std::ceil(pageSize.height() * dpi / 72.0)));

        QPdfDocumentRenderOptions options;
        QPdfDocumentRenderOptions::RenderFlags flags = {};
        if (colorMode == "grayscale") {
          flags |= QPdfDocumentRenderOptions::RenderFlag::Grayscale;
        }
        if (!antialias) {
          flags |= QPdfDocumentRenderOptions::RenderFlag::ImageAliased;
          flags |= QPdfDocumentRenderOptions::RenderFlag::PathAliased;
        }
        if (!textAntialias) {
          flags |= QPdfDocumentRenderOptions::RenderFlag::TextAliased;
        }
        if (flags != QPdfDocumentRenderOptions::RenderFlags()) {
          options.setRenderFlags(flags);
        }

        auto renderWithOptions = [&](const QSize &size,
                                     const QPdfDocumentRenderOptions &renderOptions) -> QImage {
          QMutexLocker renderLock(&state->renderMutex);
          return state->doc->render(index, size, renderOptions);
        };

        QImage image;
        if (tileSize > 0 && (pixelWidth > tileSize || pixelHeight > tileSize)) {
          image = QImage(pixelWidth, pixelHeight, QImage::Format_ARGB32_Premultiplied);
          image.fill(Qt::transparent);
          QPainter painter(&image);
          const QSize fullSize(pixelWidth, pixelHeight);
          for (int y = 0; y < pixelHeight; y += tileSize) {
            for (int x = 0; x < pixelWidth; x += tileSize) {
              const int w = std::min(tileSize, pixelWidth - x);
              const int h = std::min(tileSize, pixelHeight - y);
              QPdfDocumentRenderOptions tileOptions = options;
              tileOptions.setScaledSize(fullSize);
              tileOptions.setScaledClipRect(QRect(x, y, w, h));
              const QImage tile = renderWithOptions(QSize(w, h), tileOptions);
              if (!tile.isNull()) {
                painter.drawImage(x, y, tile);
              }
            }
          }
        } else {
          image = renderWithOptions(QSize(pixelWidth, pixelHeight), options);
        }

        if (image.isNull()) {
          return image;
        }

        if (backgroundMode != "transparent") {
          QColor fill = Qt::white;
          if (backgroundMode == "theme" || backgroundMode == "custom") {
            if (backgroundColor.isValid()) {
              fill = backgroundColor;
            }
          }
          QImage composed(image.size(), QImage::Format_ARGB32_Premultiplied);
          composed.fill(fill);
          QPainter painter(&composed);
          painter.drawImage(0, 0, image);
          image = composed;
        }

        if (colorMode == "grayscale") {
          image = image.convertToFormat(QImage::Format_Grayscale8);
        }

        if ((maxWidth > 0 && image.width() > maxWidth) ||
            (maxHeight > 0 && image.height() > maxHeight)) {
          const int targetW = maxWidth > 0 ? maxWidth : image.width();
          const int targetH = maxHeight > 0 ? maxHeight : image.height();
          image = image.scaled(targetW, targetH, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        return image;
#else
        Q_UNUSED(dpi)
        return {};
#endif
      };

      if (renderLow) {
        const QImage image = renderPageImage(lowDpi);
        if (!image.isNull()) {
          QDir().mkpath(state->tempDir);
          const QByteArray format = imageFormat == "jpeg" ? QByteArray("JPEG") : QByteArray("PNG");
          const bool saved = (imageFormat == "jpeg")
                                 ? image.save(path, format.constData(), jpegQuality)
                                 : image.save(path, format.constData());
          if (saved) {
            QMutexLocker locker(&state->mutex);
            if (state->alive) {
              addToCache(state, index);
            }
          }
          notifyReady();
        }
      }

      if (renderHigh) {
        const QImage image = renderPageImage(highDpi);
        if (!image.isNull()) {
          QDir().mkpath(state->tempDir);
          const QByteArray format = imageFormat == "jpeg" ? QByteArray("JPEG") : QByteArray("PNG");
          const bool saved = (imageFormat == "jpeg")
                                 ? image.save(path, format.constData(), jpegQuality)
                                 : image.save(path, format.constData());
          if (saved) {
            QMutexLocker locker(&state->mutex);
            if (state->alive) {
              addToCache(state, index);
              state->highResCached.insert(index);
            }
          }
          notifyReady();
        }
      }

      QMutexLocker locker(&state->mutex);
      state->inFlight.remove(index);
    });
    return false;
  }

  static void addToCache(const std::shared_ptr<PdfRenderState> &state, int index) {
    if (!state || state->cached.contains(index)) {
      touchCache(state, index);
      return;
    }
    state->cached.insert(index);
    state->cacheOrder.append(index);
    if (state->cachePolicy == "lru") {
      state->cacheOrder.removeAll(index);
      state->cacheOrder.append(index);
    }
    while (state->cacheOrder.size() > state->cacheLimit) {
      const int evict = state->cacheOrder.takeFirst();
      state->cached.remove(evict);
      state->highResCached.remove(evict);
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
#if defined(HAVE_POPPLER_QT6)
  std::unique_ptr<Poppler::Document> doc(Poppler::Document::load(path));
  if (!doc) {
    if (error) {
      *error = "Failed to open PDF";
    }
    qWarning() << "PdfProvider: failed to open" << path;
    return nullptr;
  }

  const PdfSettings pdfSettings = loadPdfSettings();
  doc->setRenderHint(Poppler::Document::TextAntialiasing, pdfSettings.textAntialias);
  doc->setRenderHint(Poppler::Document::Antialiasing, pdfSettings.antialias);

  QStringList pages;
  QStringList images;
  const int pageCount = doc->numPages();
  pages.reserve(pageCount);
  images.reserve(pageCount);
  const QFileInfo info(path);
  const QString outDir = tempDirForPdf(info);
  QDir().mkpath(outDir);
  const int cacheLimit = pdfSettings.cacheLimit;
  const double renderDpi = static_cast<double>(pdfSettings.dpi);
  const QString imageExt = pdfSettings.imageFormat == "jpeg" ? "jpg" : "png";
  for (int i = 0; i < pageCount; ++i) {
    std::unique_ptr<Poppler::Page> page(doc->page(i));
    if (!page) {
      continue;
    }
    if (pdfSettings.extractText) {
      pages.append(page->text(QRectF()));
    }
    const QString outPath =
        QDir(outDir).filePath(QString("page_%1.%2").arg(i + 1, 4, 10, QLatin1Char('0')).arg(imageExt));
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
  state->prefetchDistance = pdfSettings.prefetchDistance;
  state->prefetchStrategy = pdfSettings.prefetchStrategy;
  state->cachePolicy = pdfSettings.cachePolicy;
  state->renderPreset = pdfSettings.renderPreset;
  state->antialias = pdfSettings.antialias;
  state->textAntialias = pdfSettings.textAntialias;
  state->colorMode = pdfSettings.colorMode;
  state->backgroundMode = pdfSettings.backgroundMode;
  state->backgroundColor = pdfSettings.backgroundColor;
  state->maxWidth = pdfSettings.maxWidth;
  state->maxHeight = pdfSettings.maxHeight;
  state->imageFormat = pdfSettings.imageFormat;
  state->jpegQuality = pdfSettings.jpegQuality;
  state->tileSize = pdfSettings.tileSize;
  state->progressive = pdfSettings.progressive;
  state->progressiveDpi = pdfSettings.progressiveDpi;

  return std::make_unique<PdfDocument>(title, pages.join("\n\n"), state);
#elif defined(HAVE_QT_PDF)
  auto doc = std::make_unique<QPdfDocument>();
  const QPdfDocument::Error loadError = doc->load(path);
  if (loadError != QPdfDocument::Error::None || doc->status() != QPdfDocument::Status::Ready) {
    if (error) {
      *error = "Failed to open PDF";
    }
    qWarning() << "PdfProvider: failed to open" << path << "error:" << static_cast<int>(loadError);
    return nullptr;
  }

  const PdfSettings pdfSettings = loadPdfSettings();

  QStringList pages;
  QStringList images;
  const int pageCount = doc->pageCount();
  pages.reserve(pageCount);
  images.reserve(pageCount);
  const QFileInfo info(path);
  const QString outDir = tempDirForPdf(info);
  QDir().mkpath(outDir);
  const int cacheLimit = pdfSettings.cacheLimit;
  const double renderDpi = static_cast<double>(pdfSettings.dpi);
  const QString imageExt = pdfSettings.imageFormat == "jpeg" ? "jpg" : "png";
  for (int i = 0; i < pageCount; ++i) {
    if (pdfSettings.extractText) {
      const QPdfSelection selection = doc->getAllText(i);
      pages.append(selection.text());
    }
    const QString outPath =
        QDir(outDir).filePath(QString("page_%1.%2").arg(i + 1, 4, 10, QLatin1Char('0')).arg(imageExt));
    images.append(outPath);
  }

  QString title;
  const QVariant metaTitle = doc->metaData(QPdfDocument::MetaDataField::Title);
  if (!metaTitle.toString().isEmpty()) {
    title = metaTitle.toString();
  } else {
    title = QFileInfo(path).completeBaseName();
  }

  auto state = std::make_shared<PdfRenderState>();
  state->doc = std::move(doc);
  state->images = images;
  state->tempDir = outDir;
  state->cacheLimit = cacheLimit;
  state->renderDpi = renderDpi;
  state->prefetchDistance = pdfSettings.prefetchDistance;
  state->prefetchStrategy = pdfSettings.prefetchStrategy;
  state->cachePolicy = pdfSettings.cachePolicy;
  state->renderPreset = pdfSettings.renderPreset;
  state->antialias = pdfSettings.antialias;
  state->textAntialias = pdfSettings.textAntialias;
  state->colorMode = pdfSettings.colorMode;
  state->backgroundMode = pdfSettings.backgroundMode;
  state->backgroundColor = pdfSettings.backgroundColor;
  state->maxWidth = pdfSettings.maxWidth;
  state->maxHeight = pdfSettings.maxHeight;
  state->imageFormat = pdfSettings.imageFormat;
  state->jpegQuality = pdfSettings.jpegQuality;
  state->tileSize = pdfSettings.tileSize;
  state->progressive = pdfSettings.progressive;
  state->progressiveDpi = pdfSettings.progressiveDpi;

  return std::make_unique<PdfDocument>(title, pages.join("\n\n"), state);
#else
  if (error) {
    *error = "No PDF backend available (Poppler Qt6 or QtPdf required)";
  }
  qWarning() << "PdfProvider: No PDF backend available";
  Q_UNUSED(path)
  return nullptr;
#endif
}
