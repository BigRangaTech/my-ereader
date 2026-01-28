#include "MobiProvider.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QXmlStreamReader>
#include <cstdlib>

extern "C" {
#include "mobi.h"
}

namespace {

class MobiDocument : public FormatDocument {
public:
  MobiDocument(QString title,
               QStringList chapterTitles,
               QStringList chapterTexts,
               QString coverPath)
      : m_title(std::move(title)),
        m_chapterTitles(std::move(chapterTitles)),
        m_chapterTexts(std::move(chapterTexts)),
        m_coverPath(std::move(coverPath)) {}

  QString title() const override { return m_title; }
  QStringList chapterTitles() const override { return m_chapterTitles; }
  QString readAllText() const override {
    if (!m_allText.isEmpty()) {
      return m_allText;
    }
    QString joined;
    for (const QString &chapter : m_chapterTexts) {
      if (!joined.isEmpty()) {
        joined.append("\n\n");
      }
      joined.append(chapter);
    }
    m_allText = joined;
    return m_allText;
  }
  QStringList chaptersText() const override { return m_chapterTexts; }
  QString coverPath() const override { return m_coverPath; }

private:
  QString m_title;
  QStringList m_chapterTitles;
  QStringList m_chapterTexts;
  mutable QString m_allText;
  QString m_coverPath;
};

QString stripXhtml(const QByteArray &xhtml) {
  QXmlStreamReader xml(xhtml);
  QString out;
  bool lastWasSpace = true;
  bool inEmphasis = false;
  int ignoreDepth = 0;

  auto appendText = [&out, &lastWasSpace](const QString &text) {
    QString t = text;
    t.replace(QChar(0x00A0), QChar(' '));
    if (t.isEmpty()) {
      return;
    }
    const bool startsSpace = t.at(0).isSpace();
    if (!lastWasSpace && !startsSpace) {
      out.append(' ');
    }
    out.append(t);
    lastWasSpace = out.isEmpty() ? true : out.at(out.size() - 1).isSpace();
  };

  auto appendBreak = [&out, &lastWasSpace]() {
    if (!out.endsWith('\n')) {
      out.append('\n');
    }
    lastWasSpace = true;
  };

  while (!xml.atEnd()) {
    xml.readNext();
    if (xml.isStartElement()) {
      const QString name = xml.name().toString().toLower();
      if (name == QLatin1String("style") || name == QLatin1String("script") ||
          name == QLatin1String("head") || name == QLatin1String("metadata") ||
          name == QLatin1String("title")) {
        ignoreDepth++;
      }
      if (ignoreDepth > 0) {
        continue;
      }
      if (name == QLatin1String("em") || name == QLatin1String("i")) {
        out.append('*');
        inEmphasis = true;
      }
      if (name == QLatin1String("strong") || name == QLatin1String("b")) {
        out.append("**");
      }
      if (name == QLatin1String("br") || name == QLatin1String("p") || name == QLatin1String("div") ||
          name == QLatin1String("li") || name == QLatin1String("tr") || name == QLatin1String("h1") ||
          name == QLatin1String("h2") || name == QLatin1String("h3") || name == QLatin1String("h4") ||
          name == QLatin1String("h5") || name == QLatin1String("h6")) {
        appendBreak();
      }
    }
    if (xml.isCharacters() && !xml.isWhitespace()) {
      if (ignoreDepth > 0) {
        continue;
      }
      const QString text = xml.text().toString().trimmed();
      if (!text.isEmpty()) {
        appendText(text);
      }
    }
    if (xml.isEndElement()) {
      const QString name = xml.name().toString().toLower();
      if (ignoreDepth > 0 &&
          (name == QLatin1String("style") || name == QLatin1String("script") ||
           name == QLatin1String("head") || name == QLatin1String("metadata") ||
           name == QLatin1String("title"))) {
        ignoreDepth--;
        continue;
      }
      if (ignoreDepth > 0) {
        continue;
      }
      if (name == QLatin1String("em") || name == QLatin1String("i")) {
        if (inEmphasis) {
          out.append('*');
          inEmphasis = false;
        }
      }
      if (name == QLatin1String("strong") || name == QLatin1String("b")) {
        out.append("**");
      }
      if (name == QLatin1String("p") || name == QLatin1String("div") || name == QLatin1String("li") ||
          name == QLatin1String("tr") || name == QLatin1String("h1") || name == QLatin1String("h2") ||
          name == QLatin1String("h3") || name == QLatin1String("h4") || name == QLatin1String("h5") ||
          name == QLatin1String("h6")) {
        appendBreak();
      }
    }
  }
  return out.trimmed();
}

QString extractHeading(const QByteArray &xhtml) {
  QXmlStreamReader xml(xhtml);
  bool inHeading = false;
  QString heading;
  while (!xml.atEnd()) {
    xml.readNext();
    if (xml.isStartElement()) {
      const QString name = xml.name().toString().toLower();
      if (name == QLatin1String("h1") || name == QLatin1String("h2") || name == QLatin1String("h3") ||
          name == QLatin1String("h4") || name == QLatin1String("h5") || name == QLatin1String("h6")) {
        inHeading = true;
      }
    } else if (xml.isCharacters() && inHeading) {
      const QString text = xml.text().toString().trimmed();
      if (!text.isEmpty()) {
        if (!heading.isEmpty()) {
          heading.append(' ');
        }
        heading.append(text);
      }
    } else if (xml.isEndElement() && inHeading) {
      const QString name = xml.name().toString().toLower();
      if (name == QLatin1String("h1") || name == QLatin1String("h2") || name == QLatin1String("h3") ||
          name == QLatin1String("h4") || name == QLatin1String("h5") || name == QLatin1String("h6")) {
        break;
      }
    }
  }
  return heading.trimmed();
}

QString decodeTitle(const MOBIData *data, const QString &fallback) {
  char *title = mobi_meta_get_title(data);
  if (title) {
    QString t = QString::fromUtf8(title).trimmed();
    free(title);
    if (!t.isEmpty()) {
      return t;
    }
  }
  return fallback;
}

QStringList extractMarkupText(const MOBIRawml *rawml, QStringList *chapterTitles) {
  QStringList chapters;
  if (!rawml || !rawml->markup) {
    return chapters;
  }
  const MOBIPart *part = rawml->markup;
  while (part) {
    if (part->data && part->size > 0) {
      const QByteArray html(reinterpret_cast<const char *>(part->data),
                            static_cast<int>(part->size));
      if (chapterTitles) {
        const QString heading = extractHeading(html);
        chapterTitles->append(heading);
      }
      const QString text = stripXhtml(html);
      if (!text.isEmpty()) {
        chapters.append(text);
      }
    }
    part = part->next;
  }
  return chapters;
}

QString fallbackRawmlText(const MOBIData *data) {
  if (!data) {
    return {};
  }
  size_t maxSize = mobi_get_text_maxsize(data);
  if (maxSize == 0 || maxSize > (1024U * 1024U * 64U)) {
    return {};
  }
  QByteArray buffer(static_cast<int>(maxSize), 0);
  size_t outLen = maxSize;
  MOBI_RET ret = mobi_get_rawml(data, buffer.data(), &outLen);
  if (ret != MOBI_SUCCESS || outLen == 0) {
    return {};
  }
  buffer.resize(static_cast<int>(outLen));
  return stripXhtml(buffer);
}

QStringList autoChapterTitles(int count) {
  QStringList titles;
  if (count <= 0) {
    return titles;
  }
  if (count == 1) {
    titles.append("Content");
    return titles;
  }
  titles.reserve(count);
  for (int i = 0; i < count; ++i) {
    titles.append(QString("Section %1").arg(i + 1));
  }
  return titles;
}

QString tempDirForMobi(const QFileInfo &info) {
  const QString key = QString("%1|%2|%3")
                          .arg(info.absoluteFilePath())
                          .arg(info.size())
                          .arg(info.lastModified().toSecsSinceEpoch());
  const QByteArray hash = QCryptographicHash::hash(key.toUtf8(), QCryptographicHash::Sha1).toHex();
  return QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation))
      .filePath(QString("ereader_mobi_%1").arg(QString::fromUtf8(hash)));
}

QString coverExtensionFromBytes(const unsigned char *data, size_t size) {
  if (!data || size < 4) {
    return "raw";
  }
  const unsigned char jpg_magic[] = "\xff\xd8\xff";
  const unsigned char gif_magic[] = "\x47\x49\x46\x38";
  const unsigned char png_magic[] = "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a";
  const unsigned char bmp_magic[] = "\x42\x4d";
  if (memcmp(data, jpg_magic, 3) == 0) {
    return "jpg";
  }
  if (memcmp(data, gif_magic, 4) == 0) {
    return "gif";
  }
  if (size >= 8 && memcmp(data, png_magic, 8) == 0) {
    return "png";
  }
  if (memcmp(data, bmp_magic, 2) == 0) {
    return "bmp";
  }
  return "raw";
}

QString extractCover(const MOBIData *data, const QFileInfo &info) {
  if (!data) {
    return {};
  }
  MOBIExthHeader *exth = mobi_get_exthrecord_by_tag(data, EXTH_COVEROFFSET);
  if (!exth) {
    return {};
  }
  const uint32_t offset = mobi_decode_exthvalue(
      static_cast<const unsigned char *>(exth->data), exth->size);
  const size_t first_resource = mobi_get_first_resource_record(data);
  const size_t uid = first_resource + offset;
  MOBIPdbRecord *record = mobi_get_record_by_seqnumber(data, uid);
  if (!record || !record->data || record->size < 4) {
    return {};
  }
  const QString ext = coverExtensionFromBytes(record->data, record->size);
  const QString outDir = tempDirForMobi(info);
  QDir().mkpath(outDir);
  const QString outPath = QDir(outDir).filePath(QString("cover.%1").arg(ext));
  QFile outFile(outPath);
  if (!outFile.open(QIODevice::WriteOnly)) {
    return {};
  }
  outFile.write(reinterpret_cast<const char *>(record->data),
                static_cast<qint64>(record->size));
  outFile.close();
  return outPath;
}

} // namespace

QString MobiProvider::name() const { return "MOBI"; }

QStringList MobiProvider::supportedExtensions() const {
  return {"mobi", "azw", "azw3", "azw4", "prc"};
}

std::unique_ptr<FormatDocument> MobiProvider::open(const QString &path, QString *error) {
  const QFileInfo info(path);
  if (!info.exists()) {
    if (error) {
      *error = "File not found";
    }
    return nullptr;
  }

  MOBIData *data = mobi_init();
  if (!data) {
    if (error) {
      *error = "Failed to initialize MOBI parser";
    }
    return nullptr;
  }

  const QByteArray encodedPath = QFile::encodeName(info.absoluteFilePath());
  MOBI_RET ret = mobi_load_filename(data, encodedPath.constData());
  if (ret != MOBI_SUCCESS) {
    if (error) {
      *error = QString("Failed to load MOBI (code %1)").arg(ret);
    }
    mobi_free(data);
    return nullptr;
  }

  if (mobi_is_encrypted(data)) {
    if (error) {
      *error = "MOBI is DRM encrypted (unsupported)";
    }
    mobi_free(data);
    return nullptr;
  }

  MOBIRawml *rawml = mobi_init_rawml(data);
  if (!rawml) {
    if (error) {
      *error = "Failed to initialize MOBI rawml";
    }
    mobi_free(data);
    return nullptr;
  }

  ret = mobi_parse_rawml(rawml, data);
  if (ret != MOBI_SUCCESS) {
    if (error) {
      *error = QString("Failed to parse MOBI (code %1)").arg(ret);
    }
    mobi_free_rawml(rawml);
    mobi_free(data);
    return nullptr;
  }

  QString title = decodeTitle(data, info.completeBaseName());
  QStringList chapterTitles;
  QStringList chapters = extractMarkupText(rawml, &chapterTitles);
  if (chapters.isEmpty()) {
    const QString fallback = fallbackRawmlText(data);
    if (!fallback.isEmpty()) {
      chapters.append(fallback);
    }
  }

  QString coverPath = extractCover(data, info);

  mobi_free_rawml(rawml);
  mobi_free(data);

  if (chapters.isEmpty()) {
    if (error) {
      *error = "No readable text found in MOBI";
    }
    return nullptr;
  }

  if (chapterTitles.size() != chapters.size()) {
    chapterTitles = autoChapterTitles(chapters.size());
  } else {
    for (int i = 0; i < chapterTitles.size(); ++i) {
      if (chapterTitles.at(i).isEmpty()) {
        chapterTitles[i] = QString("Section %1").arg(i + 1);
      }
    }
  }
  return std::make_unique<MobiDocument>(title, chapterTitles, chapters, coverPath);
}
