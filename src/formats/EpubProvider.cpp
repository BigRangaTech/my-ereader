#include "EpubProvider.h"

#include <QFileInfo>
#include <QDebug>
#include <QXmlStreamReader>

#include "miniz.h"

namespace {
class EpubDocument final : public FormatDocument {
public:
  EpubDocument(QString title, QString text, QStringList chapters, QStringList sections)
      : m_title(std::move(title)),
        m_text(std::move(text)),
        m_chapters(std::move(chapters)),
        m_sections(std::move(sections)) {}

  QString title() const override { return m_title; }
  QStringList chapterTitles() const override { return m_chapters; }
  QString readAllText() const override { return m_text; }
  QStringList chaptersText() const override { return m_sections; }

private:
  QString m_title;
  QString m_text;
  QStringList m_chapters;
  QStringList m_sections;
};

struct ZipReader {
  mz_zip_archive archive{};
  bool ok = false;

  explicit ZipReader(const QString &path) {
    memset(&archive, 0, sizeof(archive));
    ok = mz_zip_reader_init_file(&archive, path.toUtf8().constData(), 0) != 0;
  }

  ~ZipReader() {
    if (ok) {
      mz_zip_reader_end(&archive);
    }
  }

  QByteArray readFile(const QString &name) {
    if (!ok) {
      return {};
    }
    const QByteArray nameUtf8 = name.toUtf8();
    int fileIndex = mz_zip_reader_locate_file(&archive, nameUtf8.constData(), nullptr, 0);
    if (fileIndex < 0) {
      return {};
    }
    size_t size = 0;
    void *data = mz_zip_reader_extract_to_heap(&archive, fileIndex, &size, 0);
    if (!data || size == 0) {
      if (data) {
        mz_free(data);
      }
      return {};
    }
    QByteArray out(static_cast<const char *>(data), static_cast<int>(size));
    mz_free(data);
    return out;
  }
};

QString extractRootfile(const QByteArray &containerXml) {
  QXmlStreamReader xml(containerXml);
  while (!xml.atEnd()) {
    xml.readNext();
    if (xml.isStartElement() && xml.name() == QLatin1String("rootfile")) {
      const auto attrs = xml.attributes();
      const QString fullPath = attrs.value(QLatin1String("full-path")).toString();
      if (!fullPath.isEmpty()) {
        return fullPath;
      }
    }
  }
  return {};
}

struct OpfData {
  QString title;
  QHash<QString, QString> manifest; // id -> href
  QHash<QString, QString> manifestProps; // id -> properties
  QStringList spine;
  QString navHref;
  QString ncxHref;
};

OpfData parseOpf(const QByteArray &opfXml) {
  OpfData data;
  QXmlStreamReader xml(opfXml);
  QString currentElement;
  while (!xml.atEnd()) {
    xml.readNext();
    if (xml.isStartElement()) {
      currentElement = xml.name().toString();
      if (xml.name() == QLatin1String("title") || xml.name() == QLatin1String("dc:title")) {
        data.title = xml.readElementText().trimmed();
      } else if (xml.name() == QLatin1String("item")) {
        const auto attrs = xml.attributes();
        const QString id = attrs.value(QLatin1String("id")).toString();
        const QString href = attrs.value(QLatin1String("href")).toString();
        const QString props = attrs.value(QLatin1String("properties")).toString();
        const QString mediaType = attrs.value(QLatin1String("media-type")).toString();
        if (!id.isEmpty() && !href.isEmpty()) {
          data.manifest.insert(id, href);
          if (!props.isEmpty()) {
            data.manifestProps.insert(id, props);
            if (props.contains(QLatin1String("nav"))) {
              data.navHref = href;
            }
          }
          if (mediaType == QLatin1String("application/x-dtbncx+xml")) {
            data.ncxHref = href;
          }
        }
      } else if (xml.name() == QLatin1String("itemref")) {
        const QString idref = xml.attributes().value(QLatin1String("idref")).toString();
        if (!idref.isEmpty()) {
          data.spine.append(idref);
        }
      }
    }
  }
  return data;
}

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

QString joinPath(const QString &baseDir, const QString &relative) {
  if (baseDir.isEmpty()) {
    return relative;
  }
  if (relative.startsWith('/')) {
    return relative.mid(1);
  }
  return baseDir + "/" + relative;
}

QString dirOf(const QString &path) {
  const int slash = path.lastIndexOf('/');
  if (slash < 0) {
    return {};
  }
  return path.left(slash);
}

QHash<QString, QString> parseNavTitles(const QByteArray &navXhtml) {
  QHash<QString, QString> titles;
  QXmlStreamReader xml(navXhtml);
  QString currentHref;
  bool inNav = false;
  bool inLink = false;
  while (!xml.atEnd()) {
    xml.readNext();
    if (xml.isStartElement()) {
      if (xml.name() == QLatin1String("nav")) {
        const QString type = xml.attributes().value(QLatin1String("epub:type")).toString();
        inNav = (type == QLatin1String("toc"));
      } else if (inNav && xml.name() == QLatin1String("a")) {
        currentHref = xml.attributes().value(QLatin1String("href")).toString();
        inLink = true;
      }
    } else if (xml.isCharacters() && inNav && inLink) {
      const QString label = xml.text().toString().trimmed();
      if (!currentHref.isEmpty() && !label.isEmpty()) {
        titles.insert(currentHref, label);
      }
    } else if (xml.isEndElement()) {
      if (xml.name() == QLatin1String("nav")) {
        inNav = false;
      } else if (xml.name() == QLatin1String("a")) {
        inLink = false;
        currentHref.clear();
      }
    }
  }
  return titles;
}

QHash<QString, QString> parseNcxTitles(const QByteArray &ncxXml) {
  QHash<QString, QString> titles;
  QXmlStreamReader xml(ncxXml);
  QString currentSrc;
  bool inNavPoint = false;
  bool inText = false;
  while (!xml.atEnd()) {
    xml.readNext();
    if (xml.isStartElement()) {
      if (xml.name() == QLatin1String("navPoint")) {
        inNavPoint = true;
      } else if (inNavPoint && xml.name() == QLatin1String("content")) {
        currentSrc = xml.attributes().value(QLatin1String("src")).toString();
      } else if (inNavPoint && xml.name() == QLatin1String("text")) {
        inText = true;
      }
    } else if (xml.isCharacters() && inNavPoint && inText) {
      const QString label = xml.text().toString().trimmed();
      if (!currentSrc.isEmpty() && !label.isEmpty()) {
        titles.insert(currentSrc, label);
      }
    } else if (xml.isEndElement()) {
      if (xml.name() == QLatin1String("navPoint")) {
        inNavPoint = false;
        inText = false;
        currentSrc.clear();
      } else if (xml.name() == QLatin1String("text")) {
        inText = false;
      }
    }
  }
  return titles;
}
} // namespace

QString EpubProvider::name() const { return "EPUB"; }

QStringList EpubProvider::supportedExtensions() const { return {"epub"}; }

std::unique_ptr<FormatDocument> EpubProvider::open(const QString &path, QString *error) {
  ZipReader zip(path);
  if (!zip.ok) {
    if (error) {
      *error = "Failed to open EPUB (zip)";
    }
    qWarning() << "EpubProvider: failed to open zip" << path;
    return nullptr;
  }

  const QByteArray containerXml = zip.readFile("META-INF/container.xml");
  if (containerXml.isEmpty()) {
    if (error) {
      *error = "Missing container.xml";
    }
    qWarning() << "EpubProvider: missing container.xml";
    return nullptr;
  }

  const QString rootfile = extractRootfile(containerXml);
  if (rootfile.isEmpty()) {
    if (error) {
      *error = "Invalid container.xml";
    }
    qWarning() << "EpubProvider: invalid container.xml";
    return nullptr;
  }

  const QByteArray opfXml = zip.readFile(rootfile);
  if (opfXml.isEmpty()) {
    if (error) {
      *error = "Missing OPF";
    }
    qWarning() << "EpubProvider: missing OPF" << rootfile;
    return nullptr;
  }

  const OpfData opf = parseOpf(opfXml);
  const QString baseDir = dirOf(rootfile);
  QHash<QString, QString> navTitles;
  if (!opf.navHref.isEmpty()) {
    const QByteArray navXhtml = zip.readFile(joinPath(baseDir, opf.navHref));
    if (!navXhtml.isEmpty()) {
      navTitles = parseNavTitles(navXhtml);
    }
  }
  if (navTitles.isEmpty() && !opf.ncxHref.isEmpty()) {
    const QByteArray ncxXml = zip.readFile(joinPath(baseDir, opf.ncxHref));
    if (!ncxXml.isEmpty()) {
      navTitles = parseNcxTitles(ncxXml);
    }
  }

  QStringList sections;
  QStringList chapterTitles;
  for (const QString &idref : opf.spine) {
    const QString href = opf.manifest.value(idref);
    if (href.isEmpty()) {
      continue;
    }
    const QString itemPath = joinPath(baseDir, href);
    const QByteArray xhtml = zip.readFile(itemPath);
    if (xhtml.isEmpty()) {
      continue;
    }
    const QString text = stripXhtml(xhtml);
    if (!text.isEmpty()) {
      const QString navKey = href;
      QString chapterTitle = navTitles.value(navKey);
      if (chapterTitle.isEmpty()) {
        const int hashIndex = navKey.indexOf('#');
        if (hashIndex > 0) {
          chapterTitle = navTitles.value(navKey.left(hashIndex));
        }
      }
      if (chapterTitle.isEmpty()) {
        chapterTitle = QFileInfo(href).completeBaseName();
      }
      chapterTitles.append(chapterTitle);
      sections.append(text);
    }
  }

  const QString title = !opf.title.isEmpty() ? opf.title : QFileInfo(path).completeBaseName();
  const QString fullText = sections.join("\n\n");
  if (fullText.isEmpty()) {
    if (error) {
      *error = "No readable text in EPUB";
    }
    qWarning() << "EpubProvider: no readable text";
    return nullptr;
  }

  return std::make_unique<EpubDocument>(title, fullText, chapterTitles, sections);
}
