#include "Fb2Provider.h"

#include <QFile>
#include <QFileInfo>
#include <QXmlStreamReader>

namespace {
class Fb2Document final : public FormatDocument {
public:
  Fb2Document(QString title, QString text, QStringList chapters)
      : m_title(std::move(title)), m_text(std::move(text)), m_chapters(std::move(chapters)) {}

  QString title() const override { return m_title; }
  QStringList chapterTitles() const override { return m_chapters; }
  QString readAllText() const override { return m_text; }

private:
  QString m_title;
  QString m_text;
  QStringList m_chapters;
};

QString stripText(const QString &text) {
  QString t = text;
  t.replace(QChar(0x00A0), QChar(' '));
  return t.trimmed();
}
} // namespace

QString Fb2Provider::name() const { return "FB2"; }

QStringList Fb2Provider::supportedExtensions() const { return {"fb2"}; }

std::unique_ptr<FormatDocument> Fb2Provider::open(const QString &path, QString *error) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    if (error) {
      *error = "Failed to open FB2";
    }
    return nullptr;
  }

  QXmlStreamReader xml(&file);
  QString title;
  QStringList chapters;
  QStringList sections;
  QString currentSection;
  bool inTitle = false;
  bool inSection = false;
  bool inBinary = false;

  while (!xml.atEnd()) {
    xml.readNext();
    if (xml.isStartElement()) {
      const QString name = xml.name().toString().toLower();
      if (name == QLatin1String("binary")) {
        inBinary = true;
      } else if (!inBinary && name == QLatin1String("book-title")) {
        inTitle = true;
      } else if (!inBinary && name == QLatin1String("section")) {
        inSection = true;
        currentSection.clear();
      } else if (!inBinary && name == QLatin1String("title")) {
        // section title captured in text
      } else if (!inBinary && name == QLatin1String("p")) {
        // paragraph text handled in characters
      }
    } else if (xml.isCharacters() && !xml.isWhitespace()) {
      if (inBinary) {
        continue;
      }
      const QString text = stripText(xml.text().toString());
      if (text.isEmpty()) {
        continue;
      }
      if (inTitle) {
        title = text;
      }
      if (inSection) {
        currentSection.append(text);
        currentSection.append('\n');
      }
    } else if (xml.isEndElement()) {
      const QString name = xml.name().toString().toLower();
      if (name == QLatin1String("binary")) {
        inBinary = false;
      } else if (name == QLatin1String("book-title")) {
        inTitle = false;
      } else if (name == QLatin1String("section")) {
        inSection = false;
        if (!currentSection.trimmed().isEmpty()) {
          sections.append(currentSection.trimmed());
        }
      } else if (name == QLatin1String("title") && inSection) {
        const QString sectionTitle = stripText(currentSection.split('\n').value(0));
        if (!sectionTitle.isEmpty()) {
          chapters.append(sectionTitle);
        }
      }
    }
  }

  if (xml.hasError()) {
    if (error) {
      *error = "Invalid FB2";
    }
    return nullptr;
  }

  if (title.isEmpty()) {
    title = QFileInfo(path).completeBaseName();
  }

  const QString fullText = sections.join("\n\n");
  if (fullText.isEmpty()) {
    if (error) {
      *error = "No readable text in FB2";
    }
    return nullptr;
  }

  return std::make_unique<Fb2Document>(title, fullText, chapters);
}
