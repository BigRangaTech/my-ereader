#include "MobiProvider.h"
#include "../core/include/AppPaths.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImageReader>
#include <QHash>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>
#include <QUrl>
#include <QXmlStreamReader>
#include <cstdlib>
#include <optional>
#include <algorithm>

extern "C" {
#include "index.h"
#include "mobi.h"
}

namespace {

class MobiDocument : public FormatDocument {
public:
  MobiDocument(QString title,
               QStringList chapterTitles,
               QStringList chapterDisplayTexts,
               QStringList chapterPlainTexts,
               QStringList imagePaths,
               QString coverPath,
               QString authors,
               QString series,
               QString publisher,
               QString description,
               bool richText,
               bool ttsDisabled)
      : m_title(std::move(title)),
        m_chapterTitles(std::move(chapterTitles)),
        m_chapterDisplayTexts(std::move(chapterDisplayTexts)),
        m_chapterPlainTexts(std::move(chapterPlainTexts)),
        m_imagePaths(std::move(imagePaths)),
        m_coverPath(std::move(coverPath)),
        m_authors(std::move(authors)),
        m_series(std::move(series)),
        m_publisher(std::move(publisher)),
        m_description(std::move(description)),
        m_isRichText(richText),
        m_ttsDisabled(ttsDisabled) {}

  QString title() const override { return m_title; }
  QStringList chapterTitles() const override { return m_chapterTitles; }
  QString readAllText() const override {
    if (!m_allDisplayText.isEmpty()) {
      return m_allDisplayText;
    }
    QString joined;
    for (const QString &chapter : m_chapterDisplayTexts) {
      if (!joined.isEmpty()) {
        joined.append("\n\n");
      }
      joined.append(chapter);
    }
    m_allDisplayText = joined;
    return m_allDisplayText;
  }
  QString readAllPlainText() const override {
    if (!m_allPlainText.isEmpty()) {
      return m_allPlainText;
    }
    QString joined;
    for (const QString &chapter : m_chapterPlainTexts) {
      if (!joined.isEmpty()) {
        joined.append("\n\n");
      }
      joined.append(chapter);
    }
    m_allPlainText = joined;
    return m_allPlainText;
  }
  QStringList chaptersText() const override { return m_chapterDisplayTexts; }
  QStringList chaptersPlainText() const override { return m_chapterPlainTexts; }
  QStringList imagePaths() const override { return m_imagePaths; }
  QString coverPath() const override { return m_coverPath; }
  QString authors() const override { return m_authors; }
  QString series() const override { return m_series; }
  QString publisher() const override { return m_publisher; }
  QString description() const override { return m_description; }
  bool isRichText() const override { return m_isRichText; }
  bool ttsDisabled() const override { return m_ttsDisabled; }

private:
  QString m_title;
  QStringList m_chapterTitles;
  QStringList m_chapterDisplayTexts;
  QStringList m_chapterPlainTexts;
  QStringList m_imagePaths;
  mutable QString m_allDisplayText;
  mutable QString m_allPlainText;
  QString m_coverPath;
  QString m_authors;
  QString m_series;
  QString m_publisher;
  QString m_description;
  bool m_isRichText = false;
  bool m_ttsDisabled = false;
};

QString stripXhtml(const QByteArray &xhtml) {
  QByteArray cleaned = xhtml;
  cleaned.replace("&nbsp;", " ");
  cleaned.replace("&#160;", " ");
  cleaned.replace("&shy;", "");
  cleaned.replace("&#173;", "");
  cleaned.replace("data:image", "data:image-blocked");
  QXmlStreamReader xml(cleaned);
  QString out;
  bool lastWasSpace = true;
  bool inEmphasis = false;
  int ignoreDepth = 0;

  auto appendText = [&out, &lastWasSpace](const QString &text) {
    QString t = text;
    t.replace(QChar(0x00A0), QChar(' '));
    t.replace(QChar(0x00AD), QChar());
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
          name == QLatin1String("title") || name == QLatin1String("img") ||
          name == QLatin1String("image") || name == QLatin1String("svg")) {
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
           name == QLatin1String("title") || name == QLatin1String("img") ||
           name == QLatin1String("image") || name == QLatin1String("svg"))) {
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

struct ImageAsset {
  QString path;
  int width = 0;
  int height = 0;
};

QString tempDirForMobi(const QFileInfo &info);
QString coverExtensionFromBytes(const unsigned char *data, size_t size);

bool isImageType(MOBIFiletype type) {
  return type == T_JPG || type == T_GIF || type == T_PNG || type == T_BMP;
}

struct RescMetadata {
  QString coverHref;
  QString tocId;
  QString language;
  QString pageDirection;
  QString writingMode;
  QString fixedLayout;
};

bool rescPayloadFromPart(const MOBIPart *part, const unsigned char **payload, size_t *payloadSize) {
  if (!part || !payload || !payloadSize) {
    return false;
  }
  *payload = nullptr;
  *payloadSize = 0;
  if (!part->data || part->size < 16) {
    return false;
  }
  if (memcmp(part->data, "RESC", 4) != 0) {
    return false;
  }
  const uint32_t headerLen = (static_cast<uint32_t>(part->data[4]) << 24) |
                             (static_cast<uint32_t>(part->data[5]) << 16) |
                             (static_cast<uint32_t>(part->data[6]) << 8) |
                             (static_cast<uint32_t>(part->data[7]));
  const uint32_t infoLen = (static_cast<uint32_t>(part->data[12]) << 24) |
                           (static_cast<uint32_t>(part->data[13]) << 16) |
                           (static_cast<uint32_t>(part->data[14]) << 8) |
                           (static_cast<uint32_t>(part->data[15]));
  if (headerLen < 16 || headerLen > part->size) {
    return false;
  }
  if (infoLen > part->size - static_cast<size_t>(headerLen)) {
    return false;
  }
  const size_t offset = static_cast<size_t>(headerLen) + static_cast<size_t>(infoLen);
  if (offset > part->size) {
    return false;
  }
  size_t size = part->size - offset;
  while (size > 0 && part->data[offset + size - 1] == '\0') {
    size--;
  }
  *payload = part->data + offset;
  *payloadSize = size;
  return true;
}

RescMetadata extractRescMetadata(const MOBIRawml *rawml) {
  RescMetadata meta;
  if (!rawml || !rawml->resources) {
    return meta;
  }
  for (MOBIPart *part = rawml->resources; part != nullptr; part = part->next) {
    if (part->type != T_RESC) {
      continue;
    }
    const unsigned char *payload = nullptr;
    size_t payloadSize = 0;
    if (!rescPayloadFromPart(part, &payload, &payloadSize) || payloadSize == 0) {
      continue;
    }
    const QString xmlText =
        QString::fromUtf8(reinterpret_cast<const char *>(payload), static_cast<int>(payloadSize));
    QXmlStreamReader xml(xmlText);
    while (!xml.atEnd()) {
      xml.readNext();
      if (!xml.isStartElement()) {
        continue;
      }
      const QString localName = xml.name().toString().toLower();
      if (localName == QLatin1String("spine")) {
        const QString toc = xml.attributes().value("toc").toString();
        if (!toc.isEmpty()) {
          meta.tocId = toc;
        }
        continue;
      }
      if (localName == QLatin1String("meta")) {
        QString key = xml.attributes().value("name").toString();
        if (key.isEmpty()) {
          key = xml.attributes().value("property").toString();
        }
        QString value = xml.attributes().value("content").toString();
        if (value.isEmpty()) {
          value = xml.readElementText().trimmed();
        }
        const QString k = key.trimmed().toLower();
        if (value.isEmpty() || k.isEmpty()) {
          continue;
        }
        if (k == QLatin1String("cover") || k == QLatin1String("opf:cover")) {
          meta.coverHref = value;
        } else if (k == QLatin1String("language") || k.endsWith(QLatin1String(":language"))) {
          meta.language = value;
        } else if (k == QLatin1String("page-progression-direction") ||
                   k == QLatin1String("rendition:page-progression-direction") ||
                   k == QLatin1String("page-dir")) {
          meta.pageDirection = value;
        } else if (k == QLatin1String("writing-mode") ||
                   k == QLatin1String("rendition:writing-mode")) {
          meta.writingMode = value;
        } else if (k == QLatin1String("fixed-layout") ||
                   k == QLatin1String("rendition:layout")) {
          meta.fixedLayout = value;
        }
        continue;
      }
      if (localName.endsWith(QLatin1String("language")) && meta.language.isEmpty()) {
        meta.language = xml.readElementText().trimmed();
      }
    }
    break;
  }
  return meta;
}

QString normalizeHtmlFragment(const QString &html) {
  QString out = html;
  out.replace(QRegularExpression("(?is)<script[^>]*>.*?</script>"), "");
  out.replace(QRegularExpression("(?is)<style[^>]*>.*?</style>"), "");
  out.replace("data:image", "data:image-blocked");
  out.replace("&nbsp;", " ");
  out.replace("&#160;", " ");
  out.replace("&shy;", "");
  out.replace("&#173;", "");
  out.replace(QChar(0x00A0), QChar(' '));
  out.replace(QChar(0x00AD), QChar());
  return out;
}

QString formatSettingsPath(const QString &format) {
  return AppPaths::configFile(QString("%1.ini").arg(format));
}

int clampInt(int value, int minValue, int maxValue) {
  return std::max(minValue, std::min(maxValue, value));
}

double clampDouble(double value, double minValue, double maxValue) {
  return std::max(minValue, std::min(maxValue, value));
}

struct MobiRenderSettings {
  bool showImages = true;
  QString textAlign = "left";
  double paragraphSpacingEm = 0.6;
  double paragraphIndentEm = 0.0;
  int imageMaxWidthPercent = 100;
  double imageSpacingEm = 0.6;
};

MobiRenderSettings loadMobiSettings(const QString &formatKey) {
  QSettings settings(formatSettingsPath(formatKey), QSettings::IniFormat);
  MobiRenderSettings out;
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

QString applyMobiStyles(const QString &html, const MobiRenderSettings &settings) {
  if (html.trimmed().isEmpty()) {
    return html;
  }
  QString out = html;
  const QString pStyle = QString("margin:0 0 %1em 0; text-indent:%2em; text-align:%3;")
                              .arg(settings.paragraphSpacingEm, 0, 'f', 2)
                              .arg(settings.paragraphIndentEm, 0, 'f', 2)
                              .arg(settings.textAlign);
  const QString hStyle = QString("margin:0 0 %1em 0; text-align:%2;")
                              .arg(settings.paragraphSpacingEm, 0, 'f', 2)
                              .arg(settings.textAlign);
  out.replace(QRegularExpression("<p\\s*>", QRegularExpression::CaseInsensitiveOption),
              QString("<p style=\"%1\">").arg(pStyle));
  out.replace(QRegularExpression("<h([1-6])\\s*>", QRegularExpression::CaseInsensitiveOption),
              QString("<h\\1 style=\"%1\">").arg(hStyle));
  out = QString("<div style=\"text-align:%1;\">%2</div>").arg(settings.textAlign, out);
  return out;
}

QHash<size_t, ImageAsset> exportImageResources(const MOBIRawml *rawml, const QFileInfo &info) {
  QHash<size_t, ImageAsset> assets;
  if (!rawml || !rawml->resources) {
    return assets;
  }
  const QString outDir = tempDirForMobi(info);
  QDir().mkpath(outDir);
  for (MOBIPart *part = rawml->resources; part != nullptr; part = part->next) {
    if (!part->data || part->size == 0) {
      continue;
    }
    if (!isImageType(part->type)) {
      continue;
    }
    MOBIFileMeta meta = mobi_get_filemeta_by_type(part->type);
    QString ext = QString::fromLatin1(meta.extension);
    if (ext.isEmpty()) {
      ext = coverExtensionFromBytes(part->data, part->size);
    }
    const QString outPath =
        QDir(outDir).filePath(QString("res_%1.%2").arg(part->uid).arg(ext));
    if (!QFileInfo::exists(outPath)) {
      QFile outFile(outPath);
      if (outFile.open(QIODevice::WriteOnly)) {
        outFile.write(reinterpret_cast<const char *>(part->data),
                      static_cast<qint64>(part->size));
        outFile.close();
      }
    }
    ImageAsset asset;
    asset.path = outPath;
    QImageReader reader(outPath);
    const QSize size = reader.size();
    asset.width = size.width();
    asset.height = size.height();
    assets.insert(part->uid, asset);
  }
  return assets;
}

std::optional<size_t> resolveImageUidFromSrc(const QString &src, const MOBIRawml *rawml) {
  if (!rawml) {
    return std::nullopt;
  }
  const QString trimmed = src.trimmed();
  if (trimmed.startsWith("kindle:embed:", Qt::CaseInsensitive)) {
    QString fid = trimmed.mid(QString("kindle:embed:").size());
    const int cut = fid.indexOf(QRegularExpression("[?#]"));
    if (cut >= 0) {
      fid = fid.left(cut);
    }
    const QByteArray fidBytes = fid.toLatin1();
    MOBIPart *part = mobi_get_resource_by_fid(rawml, fidBytes.constData());
    if (part) {
      return part->uid;
    }
  }
  bool ok = false;
  const size_t direct = trimmed.toULongLong(&ok);
  if (ok) {
    return direct;
  }
  QRegularExpression digitsRe("(\\d+)");
  auto match = digitsRe.match(trimmed);
  if (match.hasMatch()) {
    const size_t number = match.captured(1).toULongLong(&ok);
    if (ok) {
      return number;
    }
  }
  return std::nullopt;
}

QString replaceImageSources(const QString &html,
                            const MOBIRawml *rawml,
                            const QHash<size_t, ImageAsset> &assets,
                            const MobiRenderSettings &settings) {
  QString out;
  QRegularExpression imgTagRe("<img\\b[^>]*>", QRegularExpression::CaseInsensitiveOption);
  QRegularExpression srcRe("src\\s*=\\s*(['\"])([^'\"]+)\\1",
                           QRegularExpression::CaseInsensitiveOption);
  QRegularExpression sizeRe("\\b(width|height)\\s*=", QRegularExpression::CaseInsensitiveOption);

  int last = 0;
  auto it = imgTagRe.globalMatch(html);
  while (it.hasNext()) {
    auto match = it.next();
    out.append(html.mid(last, match.capturedStart() - last));
    QString tag = match.captured(0);
    auto srcMatch = srcRe.match(tag);
    if (!srcMatch.hasMatch()) {
      if (settings.showImages) {
        out.append(tag);
      }
      last = match.capturedEnd();
      continue;
    }
    const QString src = srcMatch.captured(2);
    const auto uid = resolveImageUidFromSrc(src, rawml);
    if (!uid.has_value() || !assets.contains(uid.value())) {
      if (settings.showImages) {
        out.append(tag);
      }
      last = match.capturedEnd();
      continue;
    }
    const ImageAsset asset = assets.value(uid.value());
    if (asset.path.isEmpty()) {
      if (settings.showImages) {
        out.append(tag);
      }
      last = match.capturedEnd();
      continue;
    }
    if (!settings.showImages) {
      last = match.capturedEnd();
      continue;
    }
    const QString fileUrl = QUrl::fromLocalFile(asset.path).toString();
    const int targetWidth =
        asset.width > 0 ? std::min(asset.width, 720) : 720;
    const QString style = QString("max-width:%1%%; height:auto; margin:0 0 %2em 0;")
                              .arg(settings.imageMaxWidthPercent)
                              .arg(settings.imageSpacingEm, 0, 'f', 2);
    const QString rebuilt =
        QString("<img src=\"%1\" width=\"%2\" style=\"%3\" />")
            .arg(fileUrl)
            .arg(targetWidth)
            .arg(style);
    out.append(rebuilt);
    last = match.capturedEnd();
  }
  out.append(html.mid(last));
  return out;
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

QStringList extractNcxTitles(const MOBIRawml *rawml) {
  QStringList titles;
  if (!rawml || !rawml->ncx) {
    return titles;
  }
  const MOBIIndx *ncx = rawml->ncx;
  for (size_t i = 0; i < ncx->entries_count; ++i) {
    const MOBIIndexEntry &entry = ncx->entries[i];
    if (!entry.label) {
      continue;
    }
    QString label = QString::fromUtf8(entry.label).trimmed();
    if (!label.isEmpty()) {
      titles.append(label);
    }
  }
  return titles;
}

QStringList extractGuideTitles(const MOBIRawml *rawml) {
  QStringList titles;
  if (!rawml || !rawml->guide || !rawml->guide->cncx_record) {
    return titles;
  }
  const MOBIIndx *guide = rawml->guide;
  for (size_t i = 0; i < guide->entries_count; ++i) {
    const MOBIIndexEntry &entry = guide->entries[i];
    uint32_t cncXOffset = 0;
    if (mobi_get_indxentry_tagvalue(&cncXOffset, &entry, INDX_TAG_GUIDE_TITLE_CNCX) != MOBI_SUCCESS) {
      continue;
    }
    char *title = mobi_get_cncx_string_utf8(guide->cncx_record, cncXOffset, guide->encoding);
    if (!title) {
      continue;
    }
    const QString label = QString::fromUtf8(title).trimmed();
    free(title);
    if (!label.isEmpty()) {
      titles.append(label);
    }
  }
  return titles;
}

QStringList decodeExthStrings(const MOBIData *data, MOBIExthTag tag) {
  QStringList out;
  if (!data) {
    return out;
  }
  MOBIExthHeader *start = nullptr;
  MOBIExthHeader *record = mobi_next_exthrecord_by_tag(data, tag, &start);
  while (record) {
    char *value = mobi_decode_exthstring(data,
                                         static_cast<const unsigned char *>(record->data),
                                         record->size);
    if (value) {
      const QString decoded = QString::fromUtf8(value).trimmed();
      free(value);
      if (!decoded.isEmpty()) {
        out.append(decoded);
      }
    }
    record = mobi_next_exthrecord_by_tag(data, tag, &start);
  }
  return out;
}

QString decodeFirstExthString(const MOBIData *data, MOBIExthTag tag) {
  MOBIExthHeader *record = mobi_get_exthrecord_by_tag(data, tag);
  if (!record) {
    return {};
  }
  char *value = mobi_decode_exthstring(data,
                                       static_cast<const unsigned char *>(record->data),
                                       record->size);
  if (!value) {
    return {};
  }
  const QString decoded = QString::fromUtf8(value).trimmed();
  free(value);
  return decoded;
}

std::optional<uint32_t> decodeExthNumeric(const MOBIData *data, MOBIExthTag tag) {
  if (!data) {
    return std::nullopt;
  }
  MOBIExthHeader *record = mobi_get_exthrecord_by_tag(data, tag);
  if (!record || !record->data || record->size == 0) {
    return std::nullopt;
  }
  const uint32_t value =
      mobi_decode_exthvalue(static_cast<const unsigned char *>(record->data), record->size);
  return value;
}

QString pageDirFromExth(const std::optional<uint32_t> &value) {
  if (!value.has_value()) {
    return {};
  }
  if (value.value() == 1) {
    return "rtl";
  }
  if (value.value() == 0) {
    return "ltr";
  }
  return QString::number(value.value());
}

struct OpfMetadata {
  QString series;
  QString coverHref;
  QString title;
  QStringList creators;
  QString publisher;
  QString description;
};

OpfMetadata extractOpfMetadata(const MOBIRawml *rawml) {
  OpfMetadata meta;
  if (!rawml || !rawml->resources) {
    return meta;
  }
  const MOBIPart *opfPart = nullptr;
  for (MOBIPart *part = rawml->resources; part != nullptr; part = part->next) {
    if (part->type == T_OPF) {
      opfPart = part;
      break;
    }
  }
  if (!opfPart || !opfPart->data || opfPart->size == 0) {
    return meta;
  }
  const QString xmlText = QString::fromUtf8(reinterpret_cast<const char *>(opfPart->data),
                                            static_cast<int>(opfPart->size));
  QXmlStreamReader xml(xmlText);
  QString coverId;
  QHash<QString, QString> idToHref;
  while (!xml.atEnd()) {
    xml.readNext();
    if (!xml.isStartElement()) {
      continue;
    }
    const QString localName = xml.name().toString().toLower();
    if (localName == QLatin1String("meta")) {
      const QString metaName = xml.attributes().value("name").toString();
      const QString metaContent = xml.attributes().value("content").toString();
      if (metaName == QLatin1String("cover") && !metaContent.isEmpty()) {
        coverId = metaContent;
      }
      if ((metaName == QLatin1String("calibre:series") || metaName == QLatin1String("series")) &&
          !metaContent.isEmpty()) {
        meta.series = metaContent;
      }
    } else if (localName == QLatin1String("item")) {
      const QString id = xml.attributes().value("id").toString();
      const QString href = xml.attributes().value("href").toString();
      if (!id.isEmpty() && !href.isEmpty()) {
        idToHref.insert(id, href);
      }
    } else if (localName.endsWith(QLatin1String("title"))) {
      const QString t = xml.readElementText().trimmed();
      if (!t.isEmpty()) {
        meta.title = t;
      }
    } else if (localName.endsWith(QLatin1String("creator"))) {
      const QString t = xml.readElementText().trimmed();
      if (!t.isEmpty()) {
        meta.creators.append(t);
      }
    } else if (localName.endsWith(QLatin1String("publisher"))) {
      const QString t = xml.readElementText().trimmed();
      if (!t.isEmpty()) {
        meta.publisher = t;
      }
    } else if (localName.endsWith(QLatin1String("description"))) {
      const QString t = xml.readElementText().trimmed();
      if (!t.isEmpty()) {
        meta.description = t;
      }
    }
  }
  if (!coverId.isEmpty() && idToHref.contains(coverId)) {
    meta.coverHref = idToHref.value(coverId);
  }
  return meta;
}

struct ChapterPayload {
  QStringList display;
  QStringList plain;
  QStringList headings;
};

ChapterPayload extractMarkupContent(const MOBIRawml *rawml,
                                    const QHash<size_t, ImageAsset> &assets,
                                    bool richText,
                                    const MobiRenderSettings &settings) {
  ChapterPayload payload;
  if (!rawml || !rawml->markup) {
    return payload;
  }
  const MOBIPart *part = rawml->markup;
  while (part) {
    if (part->data && part->size > 0) {
      const QByteArray htmlBytes(reinterpret_cast<const char *>(part->data),
                                 static_cast<int>(part->size));
      const QString heading = extractHeading(htmlBytes);
      const QString plain = stripXhtml(htmlBytes);
      QString display;
      if (richText) {
        display = QString::fromUtf8(htmlBytes);
        display = normalizeHtmlFragment(display);
        display = replaceImageSources(display, rawml, assets, settings);
        if (!display.contains("<html", Qt::CaseInsensitive)) {
          display = QString("<div>%1</div>").arg(display);
        }
        display = applyMobiStyles(display, settings);
      } else {
        display = plain;
      }
      QString displayTrimmed = display.trimmed();
      QString plainTrimmed = plain.trimmed();
      if (!displayTrimmed.isEmpty() || !plainTrimmed.isEmpty()) {
        if (displayTrimmed.isEmpty()) {
          displayTrimmed = plainTrimmed;
        }
        payload.display.append(displayTrimmed);
        payload.plain.append(plainTrimmed);
        payload.headings.append(heading.trimmed());
      }
    }
    part = part->next;
  }
  return payload;
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

QString decodeMetaString(char *value) {
  if (!value) {
    return {};
  }
  QString out = QString::fromUtf8(value).trimmed();
  free(value);
  return out;
}

QString describeMobiError(MOBI_RET ret) {
  switch (ret) {
  case MOBI_SUCCESS:
    return "Success";
  case MOBI_FILE_NOT_FOUND:
    return "File not found";
  case MOBI_FILE_ENCRYPTED:
    return "File is encrypted (DRM)";
  case MOBI_FILE_UNSUPPORTED:
    return "Unsupported MOBI file type";
  case MOBI_DATA_CORRUPT:
    return "Corrupted MOBI data";
  case MOBI_DRM_UNSUPPORTED:
    return "DRM support not included";
  case MOBI_DRM_KEYNOTFOUND:
    return "DRM key not found";
  case MOBI_DRM_EXPIRED:
    return "DRM license expired";
  case MOBI_PARAM_ERR:
    return "Invalid MOBI parameters";
  case MOBI_INIT_FAILED:
    return "MOBI init failed";
  case MOBI_MALLOC_FAILED:
    return "Out of memory";
  default:
    return "Unknown MOBI error";
  }
}

bool isSupportedCompression(uint16_t compression) {
  return compression == MOBI_COMPRESSION_NONE ||
         compression == MOBI_COMPRESSION_PALMDOC ||
         compression == MOBI_COMPRESSION_HUFFCDIC;
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

QString extractCover(const MOBIData *data,
                     const MOBIRawml *rawml,
                     const QFileInfo &info,
                     const RescMetadata &rescMeta,
                     const QHash<size_t, ImageAsset> &assets) {
  if (!data) {
    return {};
  }
  MOBIExthHeader *exth = mobi_get_exthrecord_by_tag(data, EXTH_COVEROFFSET);
  if (!exth) {
    if (!rescMeta.coverHref.isEmpty()) {
      const auto uid = resolveImageUidFromSrc(rescMeta.coverHref, rawml);
      if (uid.has_value() && assets.contains(uid.value())) {
        return assets.value(uid.value()).path;
      }
    }
    // Try KF8 cover uri
    const QString kf8Cover = decodeFirstExthString(data, EXTH_KF8COVERURI);
    if (!kf8Cover.isEmpty()) {
      const auto uid = resolveImageUidFromSrc(kf8Cover, rawml);
      if (uid.has_value() && assets.contains(uid.value())) {
        return assets.value(uid.value()).path;
      }
    }
    // Try OPF cover meta
    const OpfMetadata opf = extractOpfMetadata(rawml);
    if (!opf.coverHref.isEmpty()) {
      const auto uid = resolveImageUidFromSrc(opf.coverHref, rawml);
      if (uid.has_value() && assets.contains(uid.value())) {
        return assets.value(uid.value()).path;
      }
    }
    // Fallback to first extracted image
    if (!assets.isEmpty()) {
      return assets.constBegin().value().path;
    }
    return {};
  }
  const uint32_t offset = mobi_decode_exthvalue(
      static_cast<const unsigned char *>(exth->data), exth->size);
  const size_t first_resource = mobi_get_first_resource_record(data);
  const size_t uid = first_resource + offset;
  if (assets.contains(uid)) {
    return assets.value(uid).path;
  }
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
      *error = QString("Failed to load MOBI: %1 (code %2)")
                   .arg(describeMobiError(ret))
                   .arg(ret);
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
  if (data->rh && !isSupportedCompression(data->rh->compression_type)) {
    if (error) {
      *error = QString("Unsupported MOBI compression type: %1")
                   .arg(data->rh->compression_type);
    }
    mobi_free(data);
    return nullptr;
  }

  const bool isHybrid = mobi_is_hybrid(data);
  bool usingKf8 = false;
  if (isHybrid) {
    data->use_kf8 = true;
    MOBI_RET kf8ret = mobi_parse_kf8(data);
    if (kf8ret == MOBI_SUCCESS) {
      usingKf8 = true;
    } else {
      data->use_kf8 = false;
      MOBI_RET kf7ret = mobi_parse_kf7(data);
      if (kf7ret != MOBI_SUCCESS) {
        if (error) {
          *error = QString("Failed to parse KF8/KF7 parts (KF8: %1, KF7: %2)")
                       .arg(describeMobiError(kf8ret))
                       .arg(describeMobiError(kf7ret));
        }
        mobi_free(data);
        return nullptr;
      }
    }
  } else if (mobi_is_kf8(data)) {
    usingKf8 = true;
  }

  MOBIRawml *rawml = mobi_init_rawml(data);
  if (!rawml) {
    if (error) {
      *error = "Failed to initialize MOBI rawml";
    }
    mobi_free(data);
    return nullptr;
  }

  ret = mobi_parse_rawml_opt(rawml, data, true, false, true);
  if (ret != MOBI_SUCCESS) {
    if (error) {
      *error = QString("Failed to parse MOBI: %1 (code %2)")
                   .arg(describeMobiError(ret))
                   .arg(ret);
    }
    mobi_free_rawml(rawml);
    mobi_free(data);
    return nullptr;
  }

  const bool rawmlKf8 = mobi_is_rawml_kf8(rawml);
  const RescMetadata rescMeta = extractRescMetadata(rawml);
  if (isHybrid) {
    qInfo() << "MobiProvider:" << (usingKf8 ? "using KF8" : "using KF7") << "for hybrid file";
  } else {
    qInfo() << "MobiProvider:" << (rawmlKf8 ? "KF8" : "KF7") << "content detected";
  }
  if (!rescMeta.tocId.isEmpty()) {
    qInfo() << "MobiProvider: RESC spine toc id" << rescMeta.tocId;
  }

  const OpfMetadata opfMeta = extractOpfMetadata(rawml);
  QString title = decodeTitle(data, info.completeBaseName());
  if (title.isEmpty() && !opfMeta.title.isEmpty()) {
    title = opfMeta.title;
  }

  QStringList authorList = decodeExthStrings(data, EXTH_AUTHOR);
  if (authorList.isEmpty()) {
    const QString author = decodeMetaString(mobi_meta_get_author(data));
    if (!author.isEmpty()) {
      authorList.append(author);
    } else if (!opfMeta.creators.isEmpty()) {
      authorList = opfMeta.creators;
    }
  }
  const QString authors = authorList.join(", ");

  QString publisher = decodeFirstExthString(data, EXTH_PUBLISHER);
  if (publisher.isEmpty()) {
    publisher = decodeMetaString(mobi_meta_get_publisher(data));
  }
  if (publisher.isEmpty()) {
    publisher = opfMeta.publisher;
  }

  QString description = decodeFirstExthString(data, EXTH_DESCRIPTION);
  if (description.isEmpty()) {
    description = decodeMetaString(mobi_meta_get_description(data));
  }
  if (description.isEmpty()) {
    description = opfMeta.description;
  }

  const QString series = opfMeta.series;

  const QString formatKey = info.suffix().toLower().trimmed().isEmpty()
                                ? QString("mobi")
                                : info.suffix().toLower().trimmed();
  const MobiRenderSettings renderSettings = loadMobiSettings(formatKey);

  const auto assets = exportImageResources(rawml, info);
  QString coverPath = extractCover(data, rawml, info, rescMeta, assets);

  const auto ttsDisableVal = decodeExthNumeric(data, EXTH_TTSDISABLE);
  const bool ttsDisabled = ttsDisableVal.has_value() && ttsDisableVal.value() != 0;

  const QString exthLanguage = decodeFirstExthString(data, EXTH_LANGUAGE);
  const QString pageDir = pageDirFromExth(decodeExthNumeric(data, EXTH_PAGEDIR));
  const auto fixedLayoutVal = decodeExthNumeric(data, EXTH_FIXEDLAYOUT);
  const QString fixedLayout = fixedLayoutVal.has_value()
                                  ? (fixedLayoutVal.value() != 0 ? "true" : "false")
                                  : rescMeta.fixedLayout;
  const QString language = !exthLanguage.isEmpty() ? exthLanguage : rescMeta.language;
  const QString writingMode = rescMeta.writingMode;
  const QString pageDirection = !pageDir.isEmpty() ? pageDir : rescMeta.pageDirection;
  if (!language.isEmpty() || !pageDirection.isEmpty() || !writingMode.isEmpty() ||
      !fixedLayout.isEmpty()) {
    qInfo() << "MobiProvider: EXTH/RESC metadata"
            << "lang" << language
            << "page-dir" << pageDirection
            << "writing-mode" << writingMode
            << "fixed-layout" << fixedLayout;
  }
  if (ttsDisabled) {
    qInfo() << "MobiProvider: EXTH_TTSDISABLE set";
  }

  ChapterPayload payload = extractMarkupContent(rawml, assets, true, renderSettings);
  QStringList chapterDisplay = payload.display;
  QStringList chapterPlain = payload.plain;
  QStringList chapterTitles;
  QStringList tocTitles = extractNcxTitles(rawml);
  if (tocTitles.isEmpty()) {
    tocTitles = extractGuideTitles(rawml);
  }
  if (!tocTitles.isEmpty()) {
    chapterTitles = tocTitles;
  } else if (!payload.headings.isEmpty()) {
    chapterTitles = payload.headings;
  }

  if (chapterDisplay.isEmpty() && chapterPlain.isEmpty()) {
    const QString fallback = fallbackRawmlText(data);
    if (!fallback.isEmpty()) {
      chapterDisplay.append(fallback);
      chapterPlain.append(fallback);
    }
  }

  mobi_free_rawml(rawml);
  mobi_free(data);

  if (chapterDisplay.isEmpty() && chapterPlain.isEmpty()) {
    if (error) {
      *error = "No readable text found in MOBI";
    }
    return nullptr;
  }

  const int chapterCount = std::max(chapterDisplay.size(), chapterPlain.size());
  if (chapterDisplay.size() != chapterCount) {
    chapterDisplay.resize(chapterCount);
  }
  if (chapterPlain.size() != chapterCount) {
    chapterPlain.resize(chapterCount);
  }
  if (chapterTitles.size() > chapterCount) {
    chapterTitles = chapterTitles.mid(0, chapterCount);
  }
  if (chapterTitles.size() < chapterCount) {
    for (int i = chapterTitles.size(); i < chapterCount; ++i) {
      QString fallback = (i < payload.headings.size()) ? payload.headings.at(i) : QString();
      if (fallback.isEmpty()) {
        fallback = QString("Section %1").arg(i + 1);
      }
      chapterTitles.append(fallback);
    }
  }
  for (int i = 0; i < chapterTitles.size(); ++i) {
    if (chapterTitles.at(i).isEmpty()) {
      chapterTitles[i] = QString("Section %1").arg(i + 1);
    }
  }

  QStringList imagePaths;

  const bool richText = true;
  return std::make_unique<MobiDocument>(title,
                                        chapterTitles,
                                        chapterDisplay,
                                        chapterPlain,
                                        imagePaths,
                                        coverPath,
                                        authors,
                                        series,
                                        publisher,
                                        description,
                                        richText,
                                        ttsDisabled);
}
