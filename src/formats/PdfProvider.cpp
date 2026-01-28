#include "PdfProvider.h"

#ifdef HAVE_POPPLER_QT6
#include <poppler-qt6.h>
#endif

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QStandardPaths>
#include <QDebug>
#include <QList>
#include <QSet>

namespace {
class PdfDocument final : public FormatDocument {
public:
  PdfDocument(QString title,
              QString text,
              QStringList images,
              std::unique_ptr<Poppler::Document> doc,
              QString tempDir,
              int cacheLimit,
              double renderDpi)
      : m_title(std::move(title)),
        m_text(std::move(text)),
        m_images(std::move(images)),
        m_doc(std::move(doc)),
        m_tempDir(std::move(tempDir)),
        m_cacheLimit(cacheLimit),
        m_renderDpi(renderDpi) {}

  QString title() const override { return m_title; }
  QStringList chapterTitles() const override { return {}; }
  QString readAllText() const override { return m_text; }
  QStringList imagePaths() const override { return m_images; }
  bool ensureImage(int index) override {
    if (!m_doc) {
      return false;
    }
    if (index < 0 || index >= m_images.size()) {
      return false;
    }
    const QString path = m_images.at(index);
    if (m_cached.contains(index) && QFileInfo::exists(path)) {
      return true;
    }
    std::unique_ptr<Poppler::Page> page(m_doc->page(index));
    if (!page) {
      return false;
    }
    const QImage image = page->renderToImage(m_renderDpi, m_renderDpi);
    if (image.isNull()) {
      return false;
    }
    QDir().mkpath(m_tempDir);
    if (!image.save(path, "PNG")) {
      return false;
    }
    addToCache(index);
    return true;
  }

private:
  void addToCache(int index) {
    if (m_cached.contains(index)) {
      return;
    }
    m_cached.insert(index);
    m_cacheOrder.append(index);
    while (m_cacheOrder.size() > m_cacheLimit) {
      const int evict = m_cacheOrder.takeFirst();
      m_cached.remove(evict);
      if (evict >= 0 && evict < m_images.size()) {
        QFile::remove(m_images.at(evict));
      }
    }
  }

  QString m_title;
  QString m_text;
  QStringList m_images;
  std::unique_ptr<Poppler::Document> m_doc;
  QString m_tempDir;
  int m_cacheLimit = 30;
  double m_renderDpi = 120.0;
  QSet<int> m_cached;
  QList<int> m_cacheOrder;
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
  const int cacheLimit = 30;
  const double renderDpi = 120.0;
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
  auto pdfDoc = std::make_unique<PdfDocument>(title, pages.join("\n\n"), images, std::move(doc),
                                              outDir, cacheLimit, renderDpi);
  pdfDoc->ensureImage(0);
  pdfDoc->ensureImage(1);
  return pdfDoc;
#endif
}
