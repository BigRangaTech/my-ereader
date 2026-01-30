#include "EpubProvider.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QVector>
#include <QUrl>
#include <QXmlStreamReader>
#include <QCryptographicHash>
#include <QSettings>
#include <QRegularExpression>
#include <algorithm>

#include "miniz.h"

namespace {
QString formatSettingsPath() {
  QDir dir(QCoreApplication::applicationDirPath());
  for (int i = 0; i < 6; ++i) {
    if (QFileInfo::exists(dir.filePath("README.md"))) {
      return dir.filePath("config/epub.ini");
    }
    if (!dir.cdUp()) {
      break;
    }
  }
  return QDir(QCoreApplication::applicationDirPath()).filePath("epub.ini");
}

int clampInt(int value, int minValue, int maxValue) {
  return std::max(minValue, std::min(maxValue, value));
}

double clampDouble(double value, double minValue, double maxValue) {
  return std::max(minValue, std::min(maxValue, value));
}

struct EpubRenderSettings {
  bool showImages = true;
  QString textAlign = "left";
  double paragraphSpacingEm = 0.6;
  double paragraphIndentEm = 0.0;
  int imageMaxWidthPercent = 100;
  double imageSpacingEm = 0.6;
};

EpubRenderSettings loadEpubSettings() {
  QSettings settings(formatSettingsPath(), QSettings::IniFormat);
  EpubRenderSettings out;
  out.showImages = settings.value("render/show_images", true).toBool();
  out.textAlign = settings.value("render/text_align", "left").toString().toLower();
  if (out.textAlign != "left" && out.textAlign != "right" &&
      out.textAlign != "center" && out.textAlign != "justify") {
    out.textAlign = "left";
  }
  out.paragraphSpacingEm =
      clampDouble(settings.value("render/paragraph_spacing_em", 0.6).toDouble(), 0.0, 3.0);
  out.paragraphIndentEm =
      clampDouble(settings.value("render/paragraph_indent_em", 0.0).toDouble(), 0.0, 3.0);
  out.imageMaxWidthPercent =
      clampInt(settings.value("render/image_max_width_percent", 100).toInt(), 10, 100);
  out.imageSpacingEm =
      clampDouble(settings.value("render/image_spacing_em", 0.6).toDouble(), 0.0, 4.0);
  return out;
}

QString applyEpubStyles(const QString &html, const EpubRenderSettings &settings) {
  if (html.trimmed().isEmpty()) {
    return html;
  }
  QString out = html;
  const QString pStyle = QString("margin:0 0 %1em 0; text-indent:%2em; text-align:%3;")
                              .arg(settings.paragraphSpacingEm, 0, 'f', 2)
                              .arg(settings.paragraphIndentEm, 0, 'f', 2)
                              .arg(settings.textAlign);
  out.replace(QRegularExpression("<p\\s*>", QRegularExpression::CaseInsensitiveOption),
              QString("<p style=\"%1\">").arg(pStyle));
  out = QString("<div style=\"text-align:%1;\">%2</div>").arg(settings.textAlign, out);
  return out;
}

class EpubDocument final : public FormatDocument {
public:
  EpubDocument(QString title,
               QString text,
               QString plainText,
               QStringList chapters,
               QStringList sections,
               QStringList plainSections,
               QString coverPath,
               QStringList tocTitles,
               QVector<int> tocChapterIndices,
               QString authors,
               QString series,
               QString publisher,
               QString description,
               bool richText)
      : m_title(std::move(title)),
        m_text(std::move(text)),
        m_plainText(std::move(plainText)),
        m_chapters(std::move(chapters)),
        m_sections(std::move(sections)),
        m_plainSections(std::move(plainSections)),
        m_coverPath(std::move(coverPath)),
        m_tocTitles(std::move(tocTitles)),
        m_tocChapterIndices(std::move(tocChapterIndices)),
        m_authors(std::move(authors)),
        m_series(std::move(series)),
        m_publisher(std::move(publisher)),
        m_description(std::move(description)),
        m_isRichText(richText) {}

  QString title() const override { return m_title; }
  QStringList chapterTitles() const override { return m_chapters; }
  QString readAllText() const override { return m_text; }
  QString readAllPlainText() const override { return m_plainText.isEmpty() ? m_text : m_plainText; }
  QStringList chaptersText() const override { return m_sections; }
  QStringList chaptersPlainText() const override {
    return m_plainSections.isEmpty() ? m_sections : m_plainSections;
  }
  QString coverPath() const override { return m_coverPath; }
  QStringList tocTitles() const override { return m_tocTitles; }
  QVector<int> tocChapterIndices() const override { return m_tocChapterIndices; }
  QString authors() const override { return m_authors; }
  QString series() const override { return m_series; }
  QString publisher() const override { return m_publisher; }
  QString description() const override { return m_description; }
  bool isRichText() const override { return m_isRichText; }

private:
  QString m_title;
  QString m_text;
  QString m_plainText;
  QStringList m_chapters;
  QStringList m_sections;
  QStringList m_plainSections;
  QString m_coverPath;
  QStringList m_tocTitles;
  QVector<int> m_tocChapterIndices;
  QString m_authors;
  QString m_series;
  QString m_publisher;
  QString m_description;
  bool m_isRichText = false;
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
  QStringList authors;
  QString publisher;
  QString description;
  QString series;
  QHash<QString, QString> manifest; // id -> href
  QHash<QString, QString> manifestProps; // id -> properties
  QHash<QString, QString> manifestTypes; // id -> media-type
  QString coverId;
  QString coverHref;
  struct SpineItem {
    QString idref;
    bool linear = true;
  };
  QVector<SpineItem> spine;
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
      } else if (xml.name() == QLatin1String("creator") || xml.name() == QLatin1String("dc:creator")) {
        const QString author = xml.readElementText().trimmed();
        if (!author.isEmpty()) {
          data.authors.append(author);
        }
      } else if (xml.name() == QLatin1String("publisher") || xml.name() == QLatin1String("dc:publisher")) {
        data.publisher = xml.readElementText().trimmed();
      } else if (xml.name() == QLatin1String("description") || xml.name() == QLatin1String("dc:description")) {
        data.description = xml.readElementText().trimmed();
      } else if (xml.name() == QLatin1String("meta")) {
        const auto attrs = xml.attributes();
        const QString name = attrs.value(QLatin1String("name")).toString();
        const QString property = attrs.value(QLatin1String("property")).toString();
        if (name.compare(QLatin1String("cover"), Qt::CaseInsensitive) == 0) {
          data.coverId = attrs.value(QLatin1String("content")).toString();
        } else if (name.compare(QLatin1String("calibre:series"), Qt::CaseInsensitive) == 0 ||
                   name.compare(QLatin1String("series"), Qt::CaseInsensitive) == 0) {
          data.series = attrs.value(QLatin1String("content")).toString();
        } else if (property.compare(QLatin1String("belongs-to-collection"), Qt::CaseInsensitive) == 0) {
          data.series = xml.readElementText().trimmed();
        }
      } else if (xml.name() == QLatin1String("item")) {
        const auto attrs = xml.attributes();
        const QString id = attrs.value(QLatin1String("id")).toString();
        const QString href = attrs.value(QLatin1String("href")).toString();
        const QString props = attrs.value(QLatin1String("properties")).toString();
        const QString mediaType = attrs.value(QLatin1String("media-type")).toString();
        if (!id.isEmpty() && !href.isEmpty()) {
          data.manifest.insert(id, href);
          if (!mediaType.isEmpty()) {
            data.manifestTypes.insert(id, mediaType);
          }
          if (!props.isEmpty()) {
            data.manifestProps.insert(id, props);
            if (props.contains(QLatin1String("nav"))) {
              data.navHref = href;
            }
            if (props.contains(QLatin1String("cover-image"))) {
              data.coverHref = href;
            }
          }
          if (mediaType == QLatin1String("application/x-dtbncx+xml")) {
            data.ncxHref = href;
          }
        }
      } else if (xml.name() == QLatin1String("itemref")) {
        const QString idref = xml.attributes().value(QLatin1String("idref")).toString();
        if (!idref.isEmpty()) {
          OpfData::SpineItem item;
          item.idref = idref;
          const QString linear = xml.attributes().value(QLatin1String("linear")).toString();
          if (!linear.isEmpty() && linear.compare(QLatin1String("no"), Qt::CaseInsensitive) == 0) {
            item.linear = false;
          }
          data.spine.append(item);
        }
      } else if (xml.name() == QLatin1String("reference")) {
        const auto attrs = xml.attributes();
        const QString type = attrs.value(QLatin1String("type")).toString();
        const QString href = attrs.value(QLatin1String("href")).toString();
        if (type.compare(QLatin1String("cover"), Qt::CaseInsensitive) == 0 && !href.isEmpty()) {
          data.coverHref = href;
        }
      }
    }
  }
  return data;
}

QString extractXhtmlText(const QByteArray &xhtml, QString *headingOut) {
  QXmlStreamReader xml(xhtml);
  QString out;
  bool lastWasSpace = true;
  bool inEmphasis = false;
  int ignoreDepth = 0;
  bool inHeading = false;
  QString headingBuffer;

  auto appendText = [&out, &lastWasSpace](const QString &text) {
    QString t = text;
    t.replace(QChar(0x00A0), QChar(' '));
    t.remove(QChar(0x00AD));
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
      if (name == QLatin1String("h1") || name == QLatin1String("h2") || name == QLatin1String("h3") ||
          name == QLatin1String("h4") || name == QLatin1String("h5") || name == QLatin1String("h6")) {
        inHeading = true;
        headingBuffer.clear();
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
        if (inHeading) {
          if (!headingBuffer.isEmpty()) {
            headingBuffer.append(' ');
          }
          headingBuffer.append(text);
        }
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
      if (name == QLatin1String("h1") || name == QLatin1String("h2") || name == QLatin1String("h3") ||
          name == QLatin1String("h4") || name == QLatin1String("h5") || name == QLatin1String("h6")) {
        if (headingOut && headingOut->isEmpty()) {
          *headingOut = headingBuffer.trimmed();
        }
        inHeading = false;
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

QString tempDirForEpub(const QFileInfo &info);
QString resolveHref(const QString &currentFile, const QString &href);

QString escapeHtmlText(const QString &text) {
  QString out = text;
  out.replace('&', "&amp;");
  out.replace('<', "&lt;");
  out.replace('>', "&gt;");
  return out;
}

QString escapeHtmlAttribute(const QString &text) {
  QString out = escapeHtmlText(text);
  out.replace('"', "&quot;");
  return out;
}

QString writeAssetToTemp(const QFileInfo &info, const QString &href, const QByteArray &data) {
  if (href.isEmpty() || data.isEmpty()) {
    return {};
  }
  QString safePath = QDir::cleanPath(href);
  safePath.replace("..", "");
  if (safePath.startsWith('/')) {
    safePath = safePath.mid(1);
  }
  const QString outDir = tempDirForEpub(info);
  const QString outPath = QDir(outDir).filePath(safePath);
  QDir().mkpath(QFileInfo(outPath).path());
  QFile outFile(outPath);
  if (!outFile.open(QIODevice::WriteOnly)) {
    return {};
  }
  outFile.write(data);
  outFile.close();
  return outPath;
}

QString extractXhtmlRichText(const QByteArray &xhtml,
                             const QString &currentPath,
                             ZipReader &zip,
                             const QFileInfo &info,
                             const EpubRenderSettings &settings,
                             QString *headingOut) {
  QXmlStreamReader xml(xhtml);
  QString out;
  int ignoreDepth = 0;
  bool inHeading = false;
  bool inPre = false;
  QString headingBuffer;
  int pendingImageWidth = 0;
  int pendingImageHeight = 0;

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
      if (name == QLatin1String("h1") || name == QLatin1String("h2") || name == QLatin1String("h3") ||
          name == QLatin1String("h4") || name == QLatin1String("h5") || name == QLatin1String("h6")) {
        inHeading = true;
        headingBuffer.clear();
        out.append("<p><b>");
      } else if (name == QLatin1String("p") || name == QLatin1String("div")) {
        out.append("<p>");
      } else if (name == QLatin1String("ul") || name == QLatin1String("ol")) {
        out.append("<").append(name).append(">");
      } else if (name == QLatin1String("li")) {
        out.append("<li>");
      } else if (name == QLatin1String("blockquote")) {
        out.append("<blockquote>");
      } else if (name == QLatin1String("hr")) {
        out.append("<hr/>");
      } else if (name == QLatin1String("br")) {
        out.append("<br/>");
      } else if (name == QLatin1String("pre")) {
        inPre = true;
        out.append("<pre>");
      } else if (name == QLatin1String("code")) {
        out.append("<code>");
      } else if (name == QLatin1String("sup") || name == QLatin1String("sub")) {
        out.append("<").append(name).append(">");
      } else if (name == QLatin1String("em") || name == QLatin1String("i")) {
        out.append("<i>");
      } else if (name == QLatin1String("strong") || name == QLatin1String("b")) {
        out.append("<b>");
      } else if (name == QLatin1String("a")) {
        const QString href = xml.attributes().value(QLatin1String("href")).toString();
        out.append("<a href=\"").append(escapeHtmlAttribute(href))
            .append("\" style=\"color:#7fb3ff; text-decoration:underline;\">");
      } else if (name == QLatin1String("img")) {
        if (!settings.showImages) {
          continue;
        }
        const QString src = xml.attributes().value(QLatin1String("src")).toString();
        const QString widthAttr = xml.attributes().value(QLatin1String("width")).toString();
        const QString heightAttr = xml.attributes().value(QLatin1String("height")).toString();
        bool okWidth = false;
        bool okHeight = false;
        pendingImageWidth = widthAttr.toInt(&okWidth);
        pendingImageHeight = heightAttr.toInt(&okHeight);
        if (!okWidth) {
          pendingImageWidth = 0;
        }
        if (!okHeight) {
          pendingImageHeight = 0;
        }
        const QString resolved = resolveHref(currentPath, src);
        if (!resolved.isEmpty()) {
          const QByteArray imageData = zip.readFile(resolved);
          const QString outPath = writeAssetToTemp(info, resolved, imageData);
          if (!outPath.isEmpty()) {
            QString style = QString("max-width:%1%; height:auto; display:block; margin:%2em auto;")
                                .arg(settings.imageMaxWidthPercent)
                                .arg(settings.imageSpacingEm, 0, 'f', 2);
            if (pendingImageWidth > 0) {
              style.prepend(QString("width:%1px; ").arg(pendingImageWidth));
            }
            if (pendingImageHeight > 0) {
              style.prepend(QString("height:%1px; ").arg(pendingImageHeight));
            }
            out.append("<img src=\"");
            out.append(QUrl::fromLocalFile(outPath).toString());
            out.append("\" style=\"");
            out.append(style);
            out.append("\"/>");
          }
        }
      } else if (name == QLatin1String("table")) {
        out.append("<table>");
      } else if (name == QLatin1String("thead") || name == QLatin1String("tbody")) {
        out.append("<").append(name).append(">");
      } else if (name == QLatin1String("tr")) {
        out.append("<tr>");
      } else if (name == QLatin1String("td") || name == QLatin1String("th")) {
        out.append("<").append(name).append(">");
      }
    }
    if (xml.isCharacters() && !xml.isWhitespace()) {
      if (ignoreDepth > 0) {
        continue;
      }
      QString text = xml.text().toString();
      text.replace(QChar(0x00A0), QChar(' '));
      text.remove(QChar(0x00AD));
      if (!inPre) {
        text = text.trimmed();
      }
      if (!text.isEmpty()) {
        out.append(escapeHtmlText(text));
        if (inHeading) {
          if (!headingBuffer.isEmpty()) {
            headingBuffer.append(' ');
          }
          headingBuffer.append(text);
        }
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
      if (name == QLatin1String("h1") || name == QLatin1String("h2") || name == QLatin1String("h3") ||
          name == QLatin1String("h4") || name == QLatin1String("h5") || name == QLatin1String("h6")) {
        if (headingOut && headingOut->isEmpty()) {
          *headingOut = headingBuffer.trimmed();
        }
        inHeading = false;
        out.append("</b></p>");
      } else if (name == QLatin1String("p") || name == QLatin1String("div")) {
        out.append("</p>");
      } else if (name == QLatin1String("ul") || name == QLatin1String("ol")) {
        out.append("</").append(name).append(">");
      } else if (name == QLatin1String("li")) {
        out.append("</li>");
      } else if (name == QLatin1String("blockquote")) {
        out.append("</blockquote>");
      } else if (name == QLatin1String("pre")) {
        out.append("</pre>");
        inPre = false;
      } else if (name == QLatin1String("code")) {
        out.append("</code>");
      } else if (name == QLatin1String("sup") || name == QLatin1String("sub")) {
        out.append("</").append(name).append(">");
      } else if (name == QLatin1String("em") || name == QLatin1String("i")) {
        out.append("</i>");
      } else if (name == QLatin1String("strong") || name == QLatin1String("b")) {
        out.append("</b>");
      } else if (name == QLatin1String("a")) {
        out.append("</a>");
      } else if (name == QLatin1String("table")) {
        out.append("</table>");
      } else if (name == QLatin1String("thead") || name == QLatin1String("tbody")) {
        out.append("</").append(name).append(">");
      } else if (name == QLatin1String("tr")) {
        out.append("</tr>");
      } else if (name == QLatin1String("td") || name == QLatin1String("th")) {
        out.append("</").append(name).append(">");
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

QString resolveHref(const QString &currentFile, const QString &href) {
  if (href.isEmpty()) {
    return {};
  }
  QString target = href;
  const int hashIndex = target.indexOf('#');
  if (hashIndex >= 0) {
    target = target.left(hashIndex);
  }
  if (target.isEmpty()) {
    return {};
  }
  return QDir::cleanPath(joinPath(dirOf(currentFile), target));
}

QString normalizeTitle(const QString &title) {
  QString out = title;
  out.replace(QChar(0x00A0), QChar(' '));
  out = out.simplified();
  return out;
}

QString normalizeDescription(const QString &text) {
  QString out = text;
  out.replace(QChar(0x00A0), QChar(' '));
  out.remove(QChar(0x00AD));
  out = out.trimmed();
  return out;
}

QString cleanHeading(const QString &text) {
  QString out = normalizeTitle(text);
  if (out.startsWith("chapter ", Qt::CaseInsensitive)) {
    out = out.mid(7).trimmed();
  }
  return out;
}

bool looksLikeBoilerplate(const QString &text) {
  const QString lower = text.toLower();
  if (lower.contains("copyright") || lower.contains("all rights reserved")) {
    return true;
  }
  if (lower.contains("table of contents") || lower.contains("toc")) {
    return true;
  }
  if (lower.contains("isbn") || lower.contains("publisher")) {
    return true;
  }
  return false;
}

QHash<QString, QString> parseNavTitles(const QByteArray &navXhtml, const QString &navPath) {
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
      const QString label = normalizeTitle(xml.text().toString());
      if (!currentHref.isEmpty() && !label.isEmpty()) {
        const QString resolved = resolveHref(navPath, currentHref);
        if (!resolved.isEmpty()) {
          titles.insert(resolved, label);
        }
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

struct TocEntry {
  QString href;
  QString title;
};

QVector<TocEntry> parseNavEntries(const QByteArray &navXhtml, const QString &navPath) {
  QVector<TocEntry> entries;
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
      const QString label = normalizeTitle(xml.text().toString());
      if (!currentHref.isEmpty() && !label.isEmpty()) {
        const QString resolved = resolveHref(navPath, currentHref);
        if (!resolved.isEmpty()) {
          entries.append({resolved, label});
        }
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
  return entries;
}

QHash<QString, QString> parseNcxTitles(const QByteArray &ncxXml, const QString &ncxPath) {
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
      const QString label = normalizeTitle(xml.text().toString());
      if (!currentSrc.isEmpty() && !label.isEmpty()) {
        const QString resolved = resolveHref(ncxPath, currentSrc);
        if (!resolved.isEmpty()) {
          titles.insert(resolved, label);
        }
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

QVector<TocEntry> parseNcxEntries(const QByteArray &ncxXml, const QString &ncxPath) {
  QVector<TocEntry> entries;
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
      const QString label = normalizeTitle(xml.text().toString());
      if (!currentSrc.isEmpty() && !label.isEmpty()) {
        const QString resolved = resolveHref(ncxPath, currentSrc);
        if (!resolved.isEmpty()) {
          entries.append({resolved, label});
        }
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
  return entries;
}
bool isXhtmlType(const QString &mediaType, const QString &href) {
  if (mediaType == QLatin1String("application/xhtml+xml") ||
      mediaType == QLatin1String("text/html") ||
      mediaType == QLatin1String("application/x-dtbook+xml")) {
    return true;
  }
  const QString lower = href.toLower();
  return lower.endsWith(".xhtml") || lower.endsWith(".html") || lower.endsWith(".htm") ||
         lower.endsWith(".xml");
}

QString tempDirForEpub(const QFileInfo &info) {
  const QString key = QString("%1|%2|%3")
                          .arg(info.absoluteFilePath())
                          .arg(info.size())
                          .arg(info.lastModified().toSecsSinceEpoch());
  const QByteArray hash = QCryptographicHash::hash(key.toUtf8(), QCryptographicHash::Sha1).toHex();
  return QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation))
      .filePath(QString("ereader_epub_%1").arg(QString::fromUtf8(hash)));
}

QString extractFirstImageHref(const QByteArray &xhtml) {
  QXmlStreamReader xml(xhtml);
  while (!xml.atEnd()) {
    xml.readNext();
    if (xml.isStartElement() && xml.name() == QLatin1String("img")) {
      const QString src = xml.attributes().value(QLatin1String("src")).toString();
      if (!src.isEmpty()) {
        return src;
      }
    }
  }
  return {};
}

bool isImageMediaType(const QString &mediaType, const QString &href) {
  if (mediaType.startsWith("image/")) {
    return true;
  }
  const QString lower = href.toLower();
  return lower.endsWith(".jpg") || lower.endsWith(".jpeg") || lower.endsWith(".png") ||
         lower.endsWith(".webp") || lower.endsWith(".bmp") || lower.endsWith(".gif");
}

QString writeCoverToTemp(const QFileInfo &info, const QString &href, const QByteArray &data) {
  if (href.isEmpty() || data.isEmpty()) {
    return {};
  }
  const QString outDir = tempDirForEpub(info);
  QDir().mkpath(outDir);
  const QString filename = QFileInfo(href).fileName();
  const QString outPath = QDir(outDir).filePath(filename.isEmpty() ? "cover.bin" : filename);
  QFile outFile(outPath);
  if (!outFile.open(QIODevice::WriteOnly)) {
    return {};
  }
  outFile.write(data);
  outFile.close();
  return outPath;
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
  const QFileInfo info(path);
  const QString fallbackTitle = normalizeTitle(QFileInfo(path).completeBaseName());
  QHash<QString, QString> navTitles;
  QVector<TocEntry> navEntries;
  if (!opf.navHref.isEmpty()) {
    const QString navPath = joinPath(baseDir, opf.navHref);
    const QByteArray navXhtml = zip.readFile(navPath);
    if (!navXhtml.isEmpty()) {
      navTitles = parseNavTitles(navXhtml, navPath);
      navEntries = parseNavEntries(navXhtml, navPath);
      qInfo() << "EpubProvider: nav" << navPath << "entries" << navEntries.size();
    }
  }
  if (navTitles.isEmpty() && !opf.ncxHref.isEmpty()) {
    const QString ncxPath = joinPath(baseDir, opf.ncxHref);
    const QByteArray ncxXml = zip.readFile(ncxPath);
    if (!ncxXml.isEmpty()) {
      navTitles = parseNcxTitles(ncxXml, ncxPath);
      navEntries = parseNcxEntries(ncxXml, ncxPath);
      qInfo() << "EpubProvider: ncx" << ncxPath << "entries" << navEntries.size();
    }
  }

  QString coverHref = opf.coverHref;
  QString coverMediaType;
  if (coverHref.isEmpty() && !opf.coverId.isEmpty()) {
    coverHref = opf.manifest.value(opf.coverId);
    coverMediaType = opf.manifestTypes.value(opf.coverId);
  }
  if (coverHref.isEmpty()) {
    const auto keys = opf.manifest.keys();
    for (const QString &id : keys) {
      const QString href = opf.manifest.value(id);
      const QString mediaType = opf.manifestTypes.value(id);
      const QString lowerHref = href.toLower();
      if ((id.contains("cover", Qt::CaseInsensitive) || lowerHref.contains("cover")) &&
          mediaType.startsWith("image/")) {
        coverHref = href;
        coverMediaType = mediaType;
        break;
      }
    }
  }
  QString coverPath;
  if (!coverHref.isEmpty()) {
    if (coverMediaType.isEmpty()) {
      for (auto it = opf.manifest.constBegin(); it != opf.manifest.constEnd(); ++it) {
        if (it.value() == coverHref) {
          coverMediaType = opf.manifestTypes.value(it.key());
          break;
        }
      }
    }
    QString coverItemPath = joinPath(baseDir, coverHref);
    QByteArray coverData = zip.readFile(coverItemPath);
    if (!isImageMediaType(coverMediaType, coverHref)) {
      const QString imgHref = extractFirstImageHref(coverData);
      if (!imgHref.isEmpty()) {
        const QString resolved = resolveHref(coverItemPath, imgHref);
        if (!resolved.isEmpty()) {
          coverHref = resolved;
          coverItemPath = coverHref;
          coverData = zip.readFile(coverItemPath);
          coverMediaType.clear();
        }
      }
    }
    if (isImageMediaType(coverMediaType, coverHref) && !coverData.isEmpty()) {
      coverPath = writeCoverToTemp(info, coverHref, coverData);
    }
  }
  if (coverPath.isEmpty()) {
    for (const auto &item : opf.spine) {
      const QString href = opf.manifest.value(item.idref);
      if (href.isEmpty()) {
        continue;
      }
      const QString itemPath = QDir::cleanPath(joinPath(baseDir, href));
      const QByteArray xhtml = zip.readFile(itemPath);
      if (xhtml.isEmpty()) {
        continue;
      }
      const QString imgHref = extractFirstImageHref(xhtml);
      if (imgHref.isEmpty()) {
        continue;
      }
      const QString resolved = resolveHref(itemPath, imgHref);
      if (resolved.isEmpty()) {
        continue;
      }
      const QByteArray imageData = zip.readFile(resolved);
      if (imageData.isEmpty()) {
        continue;
      }
      coverPath = writeCoverToTemp(info, resolved, imageData);
      if (!coverPath.isEmpty()) {
        break;
      }
    }
  }

  const EpubRenderSettings renderSettings = loadEpubSettings();
  QStringList sections;
  QStringList plainSections;
  QStringList chapterTitles;
  QHash<QString, int> chapterIndexByPath;
  auto readSpine = [&](bool includeNonLinear) {
    for (const auto &item : opf.spine) {
      if (!includeNonLinear && !item.linear) {
        continue;
      }
      const QString href = opf.manifest.value(item.idref);
      if (href.isEmpty()) {
        continue;
      }
      const QString mediaType = opf.manifestTypes.value(item.idref);
      if (!isXhtmlType(mediaType, href)) {
        continue;
      }
      const QString itemPath = QDir::cleanPath(joinPath(baseDir, href));
      const QByteArray xhtml = zip.readFile(itemPath);
      if (xhtml.isEmpty()) {
        continue;
      }
      QString heading;
      QString richText = extractXhtmlRichText(xhtml, itemPath, zip, info, renderSettings, &heading);
      const QString plainText = extractXhtmlText(xhtml, &heading);
      if (!plainText.isEmpty() || !richText.isEmpty()) {
        const QString plainNormalized = plainText.simplified();
        if (plainNormalized.isEmpty()) {
          continue;
        }
        if (looksLikeBoilerplate(plainNormalized)) {
          continue;
        }
        QString chapterTitle = navTitles.value(itemPath);
        if (chapterTitle.isEmpty()) {
          chapterTitle = cleanHeading(heading);
        }
        if (chapterTitle.isEmpty()) {
          chapterTitle = normalizeTitle(QFileInfo(href).completeBaseName());
        }
        chapterTitle = normalizeTitle(chapterTitle);
        chapterTitles.append(chapterTitle);
        if (!richText.isEmpty()) {
          richText = applyEpubStyles(richText, renderSettings);
          sections.append(richText);
        } else {
          sections.append(plainText);
        }
        plainSections.append(plainText);
        chapterIndexByPath.insert(itemPath, sections.size() - 1);
      }
    }
  };

  readSpine(false);
  if (sections.isEmpty()) {
    readSpine(true);
  }

  const QString title = !opf.title.isEmpty() ? normalizeTitle(opf.title) : fallbackTitle;
  const QString fullText = sections.join("\n\n");
  const QString fullPlainText = plainSections.join("\n\n");
  if (fullText.isEmpty()) {
    if (error) {
      *error = "No readable text in EPUB";
    }
    qWarning() << "EpubProvider: no readable text";
    return nullptr;
  }

  QStringList authorsList = opf.authors;
  for (QString &author : authorsList) {
    author = normalizeTitle(author);
  }
  authorsList.removeAll(QString());
  const QString authors = authorsList.join("; ");
  QStringList tocTitles;
  QVector<int> tocIndices;
  if (!navEntries.isEmpty()) {
    for (const auto &entry : navEntries) {
      if (entry.title.isEmpty()) {
        continue;
      }
      const int chapterIndex = chapterIndexByPath.value(entry.href, -1);
      if (chapterIndex < 0) {
        continue;
      }
      tocTitles.append(entry.title);
      tocIndices.append(chapterIndex);
    }
  }
  if (!navEntries.isEmpty()) {
    qInfo() << "EpubProvider: toc mapped" << tocTitles.size()
            << "chapters" << chapterTitles.size();
  }
  return std::make_unique<EpubDocument>(title,
                                        fullText,
                                        fullPlainText,
                                        chapterTitles,
                                        sections,
                                        plainSections,
                                        coverPath,
                                        tocTitles,
                                        tocIndices,
                                        authors,
                                        normalizeTitle(opf.series),
                                        opf.publisher,
                                        normalizeDescription(opf.description),
                                        true);
}
