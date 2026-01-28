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

struct SectionContext {
  QString title;
  QStringList paragraphs;
  int depth = 0;
};

bool isParagraphElement(const QString &name) {
  return name == QLatin1String("p") || name == QLatin1String("subtitle") ||
         name == QLatin1String("v") || name == QLatin1String("text-author") ||
         name == QLatin1String("cite") || name == QLatin1String("annotation");
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
  QVector<SectionContext> stack;
  QString titleBuffer;
  QString paragraphBuffer;
  bool inTitle = false;
  bool inSectionTitle = false;
  bool inParagraph = false;
  bool inBinary = false;
  int sectionDepth = 0;

  while (!xml.atEnd()) {
    xml.readNext();
    if (xml.isStartElement()) {
      const QString name = xml.name().toString().toLower();
      if (name == QLatin1String("binary")) {
        inBinary = true;
      } else if (!inBinary && name == QLatin1String("book-title")) {
        inTitle = true;
      } else if (!inBinary && name == QLatin1String("section")) {
        SectionContext ctx;
        ctx.depth = ++sectionDepth;
        stack.append(ctx);
      } else if (!inBinary && name == QLatin1String("title") && !stack.isEmpty()) {
        inSectionTitle = true;
        titleBuffer.clear();
      } else if (!inBinary && isParagraphElement(name) && !stack.isEmpty()) {
        inParagraph = true;
        paragraphBuffer.clear();
      } else if (!inBinary && name == QLatin1String("empty-line") && !stack.isEmpty()) {
        stack.last().paragraphs.append(QString());
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
        if (!title.isEmpty()) {
          title.append(' ');
        }
        title.append(text);
      }
      if (!stack.isEmpty()) {
        if (inSectionTitle) {
          if (!titleBuffer.isEmpty()) {
            titleBuffer.append(' ');
          }
          titleBuffer.append(text);
        } else if (inParagraph) {
          if (!paragraphBuffer.isEmpty()) {
            paragraphBuffer.append(' ');
          }
          paragraphBuffer.append(text);
        }
      }
    } else if (xml.isEndElement()) {
      const QString name = xml.name().toString().toLower();
      if (name == QLatin1String("binary")) {
        inBinary = false;
      } else if (name == QLatin1String("book-title")) {
        inTitle = false;
      } else if (name == QLatin1String("title") && !stack.isEmpty()) {
        inSectionTitle = false;
        if (!titleBuffer.trimmed().isEmpty()) {
          if (stack.last().title.isEmpty()) {
            stack.last().title = titleBuffer.trimmed();
          }
          stack.last().paragraphs.append(titleBuffer.trimmed());
        }
        titleBuffer.clear();
      } else if (isParagraphElement(name) && !stack.isEmpty()) {
        if (inParagraph && !paragraphBuffer.trimmed().isEmpty()) {
          stack.last().paragraphs.append(paragraphBuffer.trimmed());
        }
        paragraphBuffer.clear();
        inParagraph = false;
      } else if (name == QLatin1String("section") && !stack.isEmpty()) {
        SectionContext ctx = stack.takeLast();
        sectionDepth--;
        if (!ctx.paragraphs.isEmpty()) {
          const QString sectionText = ctx.paragraphs.join("\n\n").trimmed();
          if (!sectionText.isEmpty()) {
            sections.append(sectionText);
          }
        }
        if (ctx.depth == 1 && !ctx.title.isEmpty()) {
          chapters.append(ctx.title);
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
