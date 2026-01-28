#include "PdfProvider.h"

#ifdef HAVE_POPPLER_QT6
#include <poppler-qt6.h>
#endif

#include <QFileInfo>
#include <QDebug>

namespace {
class PdfDocument final : public FormatDocument {
public:
  PdfDocument(QString title, QString text)
      : m_title(std::move(title)), m_text(std::move(text)) {}

  QString title() const override { return m_title; }
  QStringList chapterTitles() const override { return {}; }
  QString readAllText() const override { return m_text; }

private:
  QString m_title;
  QString m_text;
};
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

  QStringList pages;
  const int pageCount = doc->numPages();
  pages.reserve(pageCount);
  for (int i = 0; i < pageCount; ++i) {
    std::unique_ptr<Poppler::Page> page(doc->page(i));
    if (!page) {
      continue;
    }
    pages.append(page->text(QRectF()));
  }

  const QString title = !doc->info("Title").isEmpty() ? doc->info("Title")
                                                     : QFileInfo(path).completeBaseName();
  return std::make_unique<PdfDocument>(title, pages.join("\n\n"));
#endif
}
