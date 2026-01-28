#include "PdfProvider.h"

#ifdef HAVE_POPPLER_QT6
#include <poppler-qt6.h>
#endif

#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QStandardPaths>
#include <QDebug>

namespace {
class PdfDocument final : public FormatDocument {
public:
  PdfDocument(QString title, QString text, QStringList images)
      : m_title(std::move(title)), m_text(std::move(text)), m_images(std::move(images)) {}

  QString title() const override { return m_title; }
  QStringList chapterTitles() const override { return {}; }
  QString readAllText() const override { return m_text; }
  QStringList imagePaths() const override { return m_images; }

private:
  QString m_title;
  QString m_text;
  QStringList m_images;
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
  for (int i = 0; i < pageCount; ++i) {
    std::unique_ptr<Poppler::Page> page(doc->page(i));
    if (!page) {
      continue;
    }
    pages.append(page->text(QRectF()));

    const QImage image = page->renderToImage(150.0, 150.0);
    if (!image.isNull()) {
      const QString outPath = QDir(outDir).filePath(QString("page_%1.png").arg(i + 1, 4, 10, QLatin1Char('0')));
      if (image.save(outPath, "PNG")) {
        images.append(outPath);
      }
    }
  }

  const QString title = !doc->info("Title").isEmpty() ? doc->info("Title")
                                                     : QFileInfo(path).completeBaseName();
  return std::make_unique<PdfDocument>(title, pages.join("\n\n"), images);
#endif
}
