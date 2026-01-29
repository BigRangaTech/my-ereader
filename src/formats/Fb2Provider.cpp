#include "Fb2Provider.h"

#include <QByteArray>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QXmlStreamReader>
#include <QVector>
#include <utility>

namespace {
class Fb2Document final : public FormatDocument {
public:
  Fb2Document(QString title,
              QString text,
              QStringList chapters,
              QStringList chapterTexts,
              QString authors,
              QString series,
              QString publisher,
              QString description,
              QString coverPath)
      : m_title(std::move(title)),
        m_text(std::move(text)),
        m_chapters(std::move(chapters)),
        m_chapterTexts(std::move(chapterTexts)),
        m_authors(std::move(authors)),
        m_series(std::move(series)),
        m_publisher(std::move(publisher)),
        m_description(std::move(description)),
        m_coverPath(std::move(coverPath)) {}

  QString title() const override { return m_title; }
  QStringList chapterTitles() const override { return m_chapters; }
  QString readAllText() const override { return m_text; }
  QStringList chaptersText() const override { return m_chapterTexts; }
  QString authors() const override { return m_authors; }
  QString series() const override { return m_series; }
  QString publisher() const override { return m_publisher; }
  QString description() const override { return m_description; }
  QString coverPath() const override { return m_coverPath; }

private:
  QString m_title;
  QString m_text;
  QStringList m_chapters;
  QStringList m_chapterTexts;
  QString m_authors;
  QString m_series;
  QString m_publisher;
  QString m_description;
  QString m_coverPath;
};

QString stripText(const QString &text) {
  QString t = text;
  t.replace(QChar(0x00A0), QChar(' '));
  return t.trimmed();
}

QString joinParts(const QStringList &parts) {
  QStringList filtered;
  filtered.reserve(parts.size());
  for (const QString &part : parts) {
    const QString trimmed = stripText(part);
    if (!trimmed.isEmpty()) {
      filtered.append(trimmed);
    }
  }
  return filtered.join(' ');
}

QString tempDirFor(const QFileInfo &info) {
  const QString key = QString("%1|%2|%3")
                          .arg(info.absoluteFilePath())
                          .arg(info.size())
                          .arg(info.lastModified().toSecsSinceEpoch());
  const QByteArray hash = QCryptographicHash::hash(key.toUtf8(), QCryptographicHash::Sha1).toHex();
  return QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation))
      .filePath(QString("ereader_fb2_%1").arg(QString::fromUtf8(hash)));
}

QString contentTypeToExtension(const QString &contentType) {
  const QString type = contentType.trimmed().toLower();
  if (type.contains("jpeg") || type.contains("jpg")) {
    return "jpg";
  }
  if (type.contains("png")) {
    return "png";
  }
  if (type.contains("gif")) {
    return "gif";
  }
  if (type.contains("webp")) {
    return "webp";
  }
  if (type.contains("bmp")) {
    return "bmp";
  }
  return "img";
}

QString findHrefAttribute(const QXmlStreamAttributes &attrs) {
  for (const QXmlStreamAttribute &attr : attrs) {
    const QString name = attr.name().toString().toLower();
    if (name == QLatin1String("href") || name.endsWith(QLatin1String(":href"))) {
      return attr.value().toString();
    }
  }
  return attrs.value("href").toString();
}

QString extractCoverImage(const QByteArray &data, const QString &coverId, const QFileInfo &info) {
  const QString wantedId = coverId;
  if (wantedId.isEmpty()) {
    return {};
  }

  QXmlStreamReader xml(data);
  QString currentId;
  QString currentType;
  QByteArray base64;
  bool inBinary = false;

  while (!xml.atEnd()) {
    xml.readNext();
    if (xml.isStartElement()) {
      const QString name = xml.name().toString().toLower();
      if (name == QLatin1String("binary")) {
        currentId = xml.attributes().value("id").toString();
        currentType = xml.attributes().value("content-type").toString();
        base64.clear();
        inBinary = true;
      }
    } else if (xml.isCharacters()) {
      if (inBinary) {
        base64.append(xml.text().toUtf8());
      }
    } else if (xml.isEndElement()) {
      const QString name = xml.name().toString().toLower();
      if (name == QLatin1String("binary")) {
        inBinary = false;
        if (!wantedId.isEmpty() && currentId == wantedId) {
          QByteArray bytes = QByteArray::fromBase64(base64);
          if (!bytes.isEmpty()) {
            const QString outDir = tempDirFor(info);
            QDir().mkpath(outDir);
            const QString ext = contentTypeToExtension(currentType);
            const QString outPath = QDir(outDir).filePath(QString("cover.%1").arg(ext));
            QFile out(outPath);
            if (out.open(QIODevice::WriteOnly)) {
              out.write(bytes);
              out.close();
              return outPath;
            }
          }
          return {};
        }
      }
    }
  }

  return {};
}

QString extractFallbackImage(const QByteArray &data, const QFileInfo &info) {
  QXmlStreamReader xml(data);
  QString currentId;
  QString currentType;
  QByteArray base64;
  bool inBinary = false;

  while (!xml.atEnd()) {
    xml.readNext();
    if (xml.isStartElement()) {
      const QString name = xml.name().toString().toLower();
      if (name == QLatin1String("binary")) {
        currentId = xml.attributes().value("id").toString();
        currentType = xml.attributes().value("content-type").toString();
        base64.clear();
        inBinary = true;
      }
    } else if (xml.isCharacters()) {
      if (inBinary) {
        base64.append(xml.text().toUtf8());
      }
    } else if (xml.isEndElement()) {
      const QString name = xml.name().toString().toLower();
      if (name == QLatin1String("binary")) {
        inBinary = false;
        if (currentType.toLower().startsWith("image/")) {
          QByteArray bytes = QByteArray::fromBase64(base64);
          if (!bytes.isEmpty()) {
            const QString outDir = tempDirFor(info);
            QDir().mkpath(outDir);
            const QString ext = contentTypeToExtension(currentType);
            const QString outPath = QDir(outDir).filePath(QString("cover.%1").arg(ext));
            QFile out(outPath);
            if (out.open(QIODevice::WriteOnly)) {
              out.write(bytes);
              out.close();
              return outPath;
            }
          }
          return {};
        }
      }
    }
  }

  return {};
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
  if (!file.open(QIODevice::ReadOnly)) {
    if (error) {
      *error = "Failed to open FB2";
    }
    return nullptr;
  }

  const QByteArray data = file.readAll();
  if (data.isEmpty()) {
    if (error) {
      *error = "FB2 file is empty";
    }
    return nullptr;
  }

  const QFileInfo info(path);
  QXmlStreamReader xml(data);
  QString title;
  QStringList authors;
  QString series;
  QString publisher;
  QString description;
  QString coverId;

  QStringList chapters;
  QStringList chapterTexts;
  QStringList sections;
  QVector<SectionContext> stack;
  QString titleBuffer;
  QString paragraphBuffer;
  QString annotationBuffer;

  bool inBinary = false;
  bool inTitleInfo = false;
  bool inPublishInfo = false;
  bool inBookTitle = false;
  bool inPublisher = false;
  bool inAnnotation = false;
  bool inAnnotationParagraph = false;
  bool inAuthor = false;
  bool inCoverpage = false;
  bool inBody = false;
  bool inBodyNotes = false;
  bool inSectionTitle = false;
  bool inParagraph = false;

  QString authorFirst;
  QString authorMiddle;
  QString authorLast;
  QString authorNick;
  QString authorField;
  int sectionDepth = 0;

  while (!xml.atEnd()) {
    xml.readNext();
    if (xml.isStartElement()) {
      const QString name = xml.name().toString().toLower();
      if (name == QLatin1String("binary")) {
        inBinary = true;
      } else if (name == QLatin1String("title-info")) {
        inTitleInfo = true;
      } else if (name == QLatin1String("publish-info")) {
        inPublishInfo = true;
      } else if (!inBinary && inTitleInfo && name == QLatin1String("book-title")) {
        inBookTitle = true;
      } else if (!inBinary && inPublishInfo && name == QLatin1String("publisher")) {
        inPublisher = true;
      } else if (!inBinary && inTitleInfo && name == QLatin1String("annotation")) {
        inAnnotation = true;
      } else if (!inBinary && inTitleInfo && name == QLatin1String("coverpage")) {
        inCoverpage = true;
      } else if (!inBinary && inCoverpage && name == QLatin1String("image")) {
        const QString href = findHrefAttribute(xml.attributes());
        QString id = href;
        if (id.startsWith('#')) {
          id = id.mid(1);
        }
        if (!id.isEmpty()) {
          coverId = id;
        }
      } else if (!inBinary && inTitleInfo && name == QLatin1String("author")) {
        inAuthor = true;
        authorFirst.clear();
        authorMiddle.clear();
        authorLast.clear();
        authorNick.clear();
        authorField.clear();
      } else if (!inBinary && inAuthor && (name == QLatin1String("first-name") ||
                                           name == QLatin1String("middle-name") ||
                                           name == QLatin1String("last-name") ||
                                           name == QLatin1String("nickname"))) {
        authorField = name;
        titleBuffer.clear();
      } else if (!inBinary && inTitleInfo && name == QLatin1String("sequence")) {
        const QString seqName = xml.attributes().value("name").toString();
        const QString seqNumber = xml.attributes().value("number").toString();
        if (!seqName.isEmpty()) {
          series = seqName;
          if (!seqNumber.isEmpty()) {
            series = QString("%1 #%2").arg(seqName, seqNumber);
          }
        }
      } else if (!inBinary && name == QLatin1String("body")) {
        const QString bodyType = xml.attributes().value("type").toString().toLower();
        inBodyNotes = (bodyType == QLatin1String("notes"));
        inBody = !inBodyNotes;
      } else if (!inBinary && inBody && name == QLatin1String("section")) {
        SectionContext ctx;
        ctx.depth = ++sectionDepth;
        stack.append(ctx);
      } else if (!inBinary && inBody && name == QLatin1String("title") && !stack.isEmpty()) {
        inSectionTitle = true;
        titleBuffer.clear();
      } else if (!inBinary && inBody && isParagraphElement(name) && !stack.isEmpty()) {
        inParagraph = true;
        paragraphBuffer.clear();
      } else if (!inBinary && inBody && name == QLatin1String("empty-line") && !stack.isEmpty()) {
        stack.last().paragraphs.append(QString());
      } else if (!inBinary && inAnnotation && name == QLatin1String("p")) {
        inAnnotationParagraph = true;
        annotationBuffer.clear();
      }
    } else if (xml.isCharacters()) {
      if (inBinary) {
        continue;
      }
      const QString text = xml.text().toString();
      if (text.trimmed().isEmpty()) {
        continue;
      }
      if (inBookTitle) {
        if (!title.isEmpty()) {
          title.append(' ');
        }
        title.append(stripText(text));
      }
      if (inAuthor && !authorField.isEmpty()) {
        QString &field = (authorField == QLatin1String("first-name")) ? authorFirst
                             : (authorField == QLatin1String("middle-name")) ? authorMiddle
                             : (authorField == QLatin1String("last-name")) ? authorLast
                             : authorNick;
        if (!field.isEmpty()) {
          field.append(' ');
        }
        field.append(stripText(text));
      }
      if (inPublisher) {
        if (!publisher.isEmpty()) {
          publisher.append(' ');
        }
        publisher.append(stripText(text));
      }
      if (inAnnotation && inAnnotationParagraph) {
        if (!annotationBuffer.isEmpty()) {
          annotationBuffer.append(' ');
        }
        annotationBuffer.append(stripText(text));
      }
      if (!stack.isEmpty()) {
        if (inSectionTitle) {
          if (!titleBuffer.isEmpty()) {
            titleBuffer.append(' ');
          }
          titleBuffer.append(stripText(text));
        } else if (inParagraph) {
          if (!paragraphBuffer.isEmpty()) {
            paragraphBuffer.append(' ');
          }
          paragraphBuffer.append(stripText(text));
        }
      }
    } else if (xml.isEndElement()) {
      const QString name = xml.name().toString().toLower();
      if (name == QLatin1String("binary")) {
        inBinary = false;
      } else if (name == QLatin1String("title-info")) {
        inTitleInfo = false;
      } else if (name == QLatin1String("publish-info")) {
        inPublishInfo = false;
      } else if (name == QLatin1String("coverpage")) {
        inCoverpage = false;
      } else if (name == QLatin1String("book-title")) {
        inBookTitle = false;
      } else if (name == QLatin1String("publisher")) {
        inPublisher = false;
      } else if (name == QLatin1String("annotation")) {
        inAnnotation = false;
      } else if (name == QLatin1String("p") && inAnnotationParagraph) {
        const QString paragraph = stripText(annotationBuffer);
        if (!paragraph.isEmpty()) {
          if (!description.isEmpty()) {
            description.append("\n\n");
          }
          description.append(paragraph);
        }
        annotationBuffer.clear();
        inAnnotationParagraph = false;
      } else if (name == QLatin1String("author") && inAuthor) {
        inAuthor = false;
        authorField.clear();
        QString fullName = joinParts({authorFirst, authorMiddle, authorLast});
        if (fullName.isEmpty()) {
          fullName = stripText(authorNick);
        } else if (!authorNick.isEmpty()) {
          fullName = QString("%1 (%2)").arg(fullName, stripText(authorNick));
        }
        if (!fullName.isEmpty()) {
          authors.append(fullName);
        }
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
        const QString sectionText = ctx.paragraphs.join("\n\n").trimmed();
        if (!sectionText.isEmpty()) {
          if (!stack.isEmpty()) {
            stack.last().paragraphs.append(sectionText);
          } else {
            sections.append(sectionText);
          }
        }
        if (ctx.depth == 1 && !sectionText.isEmpty()) {
          QString chapterTitle = ctx.title.trimmed();
          if (chapterTitle.isEmpty()) {
            chapterTitle = QString("Section %1").arg(chapterTexts.size() + 1);
          }
          chapters.append(chapterTitle);
          chapterTexts.append(sectionText);
        }
      } else if (name == QLatin1String("body")) {
        inBody = false;
        inBodyNotes = false;
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
    title = info.completeBaseName();
  }

  QString fullText;
  if (!chapterTexts.isEmpty()) {
    fullText = chapterTexts.join("\n\n");
  } else {
    fullText = sections.join("\n\n");
  }

  if (fullText.isEmpty()) {
    if (error) {
      *error = "No readable text in FB2";
    }
    return nullptr;
  }

  QString coverPath = extractCoverImage(data, coverId, info);
  if (coverPath.isEmpty()) {
    coverPath = extractFallbackImage(data, info);
  }

  return std::make_unique<Fb2Document>(title,
                                       fullText,
                                       chapters,
                                       chapterTexts,
                                       authors.join(", "),
                                       series,
                                       publisher,
                                       description,
                                       coverPath);
}
