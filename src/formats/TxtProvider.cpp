#include "TxtProvider.h"

#include <QFile>
#include <QFileInfo>
#include <QTextStream>

namespace {
class TxtDocument final : public FormatDocument {
public:
  TxtDocument(QString title, QString text)
      : m_title(std::move(title)), m_text(std::move(text)) {}

  QString title() const override { return m_title; }
  QStringList chapterTitles() const override { return {}; }
  QString readAllText() const override { return m_text; }

private:
  QString m_title;
  QString m_text;
};
} // namespace

QString TxtProvider::name() const {
  return "Plain Text";
}

QStringList TxtProvider::supportedExtensions() const {
  return {"txt"};
}

std::unique_ptr<FormatDocument> TxtProvider::open(const QString &path, QString *error) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    if (error) {
      *error = QString("Failed to open %1").arg(path);
    }
    return nullptr;
  }

  QTextStream stream(&file);
  stream.setCodec("UTF-8");
  const QString text = stream.readAll();
  const QString title = QFileInfo(path).completeBaseName();

  return std::make_unique<TxtDocument>(title, text);
}
