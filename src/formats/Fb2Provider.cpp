#include "Fb2Provider.h"

#include <QByteArray>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>
#include <QStringList>
#include <QUrl>
#include <QXmlStreamReader>
#include <QVector>
#include <algorithm>
#include <utility>

namespace {
class Fb2Document final : public FormatDocument {
public:
  Fb2Document(QString title,
              QString htmlText,
              QString plainText,
              QStringList chapters,
              QStringList chapterHtml,
              QStringList chapterPlain,
              QStringList tocTitles,
              QVector<int> tocChapterIndices,
              QString authors,
              QString series,
              QString publisher,
              QString description,
              QString coverPath)
      : m_title(std::move(title)),
        m_htmlText(std::move(htmlText)),
        m_plainText(std::move(plainText)),
        m_chapters(std::move(chapters)),
        m_chapterHtml(std::move(chapterHtml)),
        m_chapterPlain(std::move(chapterPlain)),
        m_tocTitles(std::move(tocTitles)),
        m_tocChapterIndices(std::move(tocChapterIndices)),
        m_authors(std::move(authors)),
        m_series(std::move(series)),
        m_publisher(std::move(publisher)),
        m_description(std::move(description)),
        m_coverPath(std::move(coverPath)) {}

  QString title() const override { return m_title; }
  QStringList chapterTitles() const override { return m_chapters; }
  QString readAllText() const override { return m_htmlText; }
  QString readAllPlainText() const override { return m_plainText; }
  QStringList chaptersText() const override { return m_chapterHtml; }
  QStringList chaptersPlainText() const override { return m_chapterPlain; }
  QStringList tocTitles() const override { return m_tocTitles; }
  QVector<int> tocChapterIndices() const override { return m_tocChapterIndices; }
  QString authors() const override { return m_authors; }
  QString series() const override { return m_series; }
  QString publisher() const override { return m_publisher; }
  QString description() const override { return m_description; }
  QString coverPath() const override { return m_coverPath; }
  bool isRichText() const override { return true; }

private:
  QString m_title;
  QString m_htmlText;
  QString m_plainText;
  QStringList m_chapters;
  QStringList m_chapterHtml;
  QStringList m_chapterPlain;
  QStringList m_tocTitles;
  QVector<int> m_tocChapterIndices;
  QString m_authors;
  QString m_series;
  QString m_publisher;
  QString m_description;
  QString m_coverPath;
};

struct BinaryAsset {
  QByteArray bytes;
  QString contentType;
  QString path;
  bool saved = false;
};

QString escapeHtml(const QString &input) {
  QString out = input;
  out.replace('&', "&amp;");
  out.replace('<', "&lt;");
  out.replace('>', "&gt;");
  out.replace('"', "&quot;");
  out.replace('\'', "&#39;");
  return out;
}

QString normalizeWhitespace(const QString &text) {
  QString out = text;
  out.replace(QChar(0x00A0), QChar(' '));
  return out;
}

void appendText(QString &plain, QString &html, const QString &text) {
  const QString cleaned = normalizeWhitespace(text);
  if (cleaned.trimmed().isEmpty()) {
    return;
  }
  const QString trimmed = cleaned.trimmed();
  if (!plain.isEmpty() && !plain.endsWith(' ')) {
    plain.append(' ');
  }
  plain.append(trimmed);
  if (!html.isEmpty() && !html.endsWith(' ')) {
    html.append(' ');
  }
  html.append(escapeHtml(trimmed));
}

void appendPlain(QString &target, const QString &text) {
  const QString cleaned = normalizeWhitespace(text).trimmed();
  if (cleaned.isEmpty()) {
    return;
  }
  if (!target.isEmpty()) {
    target.append(' ');
  }
  target.append(cleaned);
}

QString joinParts(const QStringList &parts) {
  QStringList filtered;
  filtered.reserve(parts.size());
  for (const QString &part : parts) {
    const QString trimmed = normalizeWhitespace(part).trimmed();
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

QString formatSettingsPath() {
  QDir dir(QCoreApplication::applicationDirPath());
  for (int i = 0; i < 6; ++i) {
    if (QFileInfo::exists(dir.filePath("README.md"))) {
      return dir.filePath("config/fb2.ini");
    }
    if (!dir.cdUp()) {
      break;
    }
  }
  return QDir(QCoreApplication::applicationDirPath()).filePath("fb2.ini");
}

int clampInt(int value, int minValue, int maxValue) {
  return std::max(minValue, std::min(maxValue, value));
}

double clampDouble(double value, double minValue, double maxValue) {
  return std::max(minValue, std::min(maxValue, value));
}

struct Fb2RenderSettings {
  bool showImages = true;
  QString textAlign = "left";
  double paragraphSpacingEm = 0.6;
  double paragraphIndentEm = 0.0;
  int imageMaxWidthPercent = 100;
  double imageSpacingEm = 0.6;
};

Fb2RenderSettings loadFb2Settings() {
  QSettings settings(formatSettingsPath(), QSettings::IniFormat);
  Fb2RenderSettings out;
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

QString sanitizeId(const QString &id) {
  QString out = id.trimmed();
  if (out.isEmpty()) {
    return "image";
  }
  out.replace(QRegularExpression("[^A-Za-z0-9_-]"), "_");
  return out;
}

QString applyStyles(const QString &html, const Fb2RenderSettings &settings) {
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
  out.replace("<p>", QString("<p style=\"%1\">").arg(pStyle));
  out.replace("<h2>", QString("<h2 style=\"%1\">").arg(hStyle));
  out = QString("<div style=\"text-align:%1;\">%2</div>").arg(settings.textAlign, out);
  return out;
}

QString ensureImageFile(const QString &id, QHash<QString, BinaryAsset> &assets, const QString &outDir) {
  if (id.isEmpty()) {
    return {};
  }
  auto it = assets.find(id);
  if (it == assets.end()) {
    return {};
  }
  BinaryAsset &asset = it.value();
  if (asset.saved && !asset.path.isEmpty() && QFileInfo::exists(asset.path)) {
    return asset.path;
  }
  const QString ext = contentTypeToExtension(asset.contentType);
  const QString fileName = QString("%1.%2").arg(sanitizeId(id), ext);
  const QString outPath = QDir(outDir).filePath(fileName);
  QDir().mkpath(QFileInfo(outPath).absolutePath());
  QFile out(outPath);
  if (out.open(QIODevice::WriteOnly)) {
    out.write(asset.bytes);
    out.close();
    asset.path = outPath;
    asset.saved = true;
    return outPath;
  }
  return {};
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

QHash<QString, BinaryAsset> extractBinaryAssets(const QByteArray &data) {
  QHash<QString, BinaryAsset> assets;
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
        if (!currentId.isEmpty() && currentType.toLower().startsWith("image/")) {
          QByteArray bytes = QByteArray::fromBase64(base64);
          if (!bytes.isEmpty()) {
            BinaryAsset asset;
            asset.bytes = std::move(bytes);
            asset.contentType = currentType;
            assets.insert(currentId, std::move(asset));
          }
        }
      }
    }
  }
  return assets;
}

struct SectionContext {
  QString title;
  QStringList htmlBlocks;
  QStringList plainBlocks;
  int depth = 0;
  int topIndex = -1;
};

bool isParagraphElement(const QString &name) {
  return name == QLatin1String("p") || name == QLatin1String("subtitle") ||
         name == QLatin1String("v") || name == QLatin1String("text-author") ||
         name == QLatin1String("cite") || name == QLatin1String("annotation");
}

bool isInlineTag(const QString &name) {
  return name == QLatin1String("strong") || name == QLatin1String("b") ||
         name == QLatin1String("em") || name == QLatin1String("i") ||
         name == QLatin1String("sub") || name == QLatin1String("sup") ||
         name == QLatin1String("a");
}

QString openInlineTag(const QString &name, const QXmlStreamAttributes &attrs) {
  if (name == QLatin1String("strong") || name == QLatin1String("b")) {
    return "<b>";
  }
  if (name == QLatin1String("em") || name == QLatin1String("i")) {
    return "<i>";
  }
  if (name == QLatin1String("sub")) {
    return "<sub>";
  }
  if (name == QLatin1String("sup")) {
    return "<sup>";
  }
  if (name == QLatin1String("a")) {
    const QString href = attrs.value("href").toString();
    if (!href.isEmpty()) {
      return QString("<a href=\"%1\">").arg(escapeHtml(href));
    }
    return "<a>";
  }
  return {};
}

QString closeInlineTag(const QString &name) {
  if (name == QLatin1String("strong") || name == QLatin1String("b")) {
    return "</b>";
  }
  if (name == QLatin1String("em") || name == QLatin1String("i")) {
    return "</i>";
  }
  if (name == QLatin1String("sub")) {
    return "</sub>";
  }
  if (name == QLatin1String("sup")) {
    return "</sup>";
  }
  if (name == QLatin1String("a")) {
    return "</a>";
  }
  return {};
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
  const QString outDir = tempDirFor(info);
  QDir().mkpath(outDir);

  const Fb2RenderSettings renderSettings = loadFb2Settings();
  QHash<QString, BinaryAsset> assets = extractBinaryAssets(data);
  const QString fallbackImageId = assets.isEmpty() ? QString() : assets.constBegin().key();

  QXmlStreamReader xml(data);
  QString title;
  QStringList authors;
  QString series;
  QString publisher;
  QString description;
  QString coverId;

  QStringList chapterTitles;
  QStringList chapterHtml;
  QStringList chapterPlain;
  QStringList tocTitles;
  QVector<int> tocIndices;

  QVector<SectionContext> stack;
  QString titleBuffer;
  QString currentParagraphPlain;
  QString currentParagraphHtml;
  bool inBinary = false;
  bool inTitleInfo = false;
  bool inPublishInfo = false;
  bool inBookTitle = false;
  bool inPublisher = false;
  bool inAnnotation = false;
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

  auto flushParagraph = [&]() {
    if (!inParagraph) {
      return;
    }
    inParagraph = false;
    if (stack.isEmpty()) {
      currentParagraphPlain.clear();
      currentParagraphHtml.clear();
      return;
    }
    SectionContext &ctx = stack.last();
    const QString plain = currentParagraphPlain.trimmed();
    const QString html = currentParagraphHtml.trimmed();
    if (!plain.isEmpty() || !html.isEmpty()) {
      QString blockHtml = html.isEmpty() ? escapeHtml(plain) : html;
      if (!blockHtml.startsWith('<')) {
        blockHtml = QString("<p>%1</p>").arg(blockHtml);
      } else if (!blockHtml.startsWith("<p") && !blockHtml.startsWith("<h")) {
        blockHtml = QString("<p>%1</p>").arg(blockHtml);
      }
      ctx.htmlBlocks.append(blockHtml);
      if (!plain.isEmpty()) {
        ctx.plainBlocks.append(plain);
      }
    }
    currentParagraphPlain.clear();
    currentParagraphHtml.clear();
  };

  const QString imageStyle = QString("display:block; max-width:%1%%; height:auto; margin:0 0 %2em 0;")
                                  .arg(renderSettings.imageMaxWidthPercent)
                                  .arg(renderSettings.imageSpacingEm, 0, 'f', 2);
  auto appendImage = [&](const QString &id) {
    if (!renderSettings.showImages) {
      return;
    }
    if (stack.isEmpty()) {
      return;
    }
    QString imgPath = ensureImageFile(id, assets, outDir);
    if (imgPath.isEmpty()) {
      return;
    }
    const QString imgTag = QString("<img src=\"%1\" style=\"%2\"/>")
                                .arg(QUrl::fromLocalFile(imgPath).toString(), imageStyle);
    if (inParagraph) {
      currentParagraphHtml.append(imgTag);
    } else {
      stack.last().htmlBlocks.append(QString("<p>%1</p>").arg(imgTag));
    }
  };

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
        QString id = findHrefAttribute(xml.attributes());
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
        ctx.topIndex = stack.isEmpty() ? chapterTitles.size() : stack.last().topIndex;
        stack.append(ctx);
      } else if (!inBinary && inBody && name == QLatin1String("title") && !stack.isEmpty()) {
        inSectionTitle = true;
        titleBuffer.clear();
      } else if (!inBinary && inBody && !inSectionTitle && isParagraphElement(name) && !stack.isEmpty()) {
        inParagraph = true;
        currentParagraphPlain.clear();
        currentParagraphHtml.clear();
      } else if (!inBinary && inBody && name == QLatin1String("empty-line") && !stack.isEmpty()) {
        stack.last().htmlBlocks.append("<br/>");
        stack.last().plainBlocks.append(QString());
      } else if (!inBinary && inBody && name == QLatin1String("image")) {
        QString id = findHrefAttribute(xml.attributes());
        if (id.startsWith('#')) {
          id = id.mid(1);
        }
        appendImage(id);
      } else if (!inBinary && inBody && inParagraph && isInlineTag(name)) {
        currentParagraphHtml.append(openInlineTag(name, xml.attributes()));
      } else if (!inBinary && inBody && inParagraph && name == QLatin1String("br")) {
        currentParagraphHtml.append("<br/>");
        currentParagraphPlain.append("\n");
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
        title.append(normalizeWhitespace(text).trimmed());
      }
      if (inAuthor && !authorField.isEmpty()) {
        QString &field = (authorField == QLatin1String("first-name")) ? authorFirst
                             : (authorField == QLatin1String("middle-name")) ? authorMiddle
                             : (authorField == QLatin1String("last-name")) ? authorLast
                             : authorNick;
        appendPlain(field, text);
      }
      if (inPublisher) {
        if (!publisher.isEmpty()) {
          publisher.append(' ');
        }
        publisher.append(normalizeWhitespace(text).trimmed());
      }
      if (inAnnotation) {
        appendPlain(description, text);
      }
      if (inSectionTitle) {
        appendPlain(titleBuffer, text);
      } else if (inParagraph) {
        appendText(currentParagraphPlain, currentParagraphHtml, text);
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
      } else if (name == QLatin1String("author") && inAuthor) {
        inAuthor = false;
        authorField.clear();
        QString fullName = joinParts({authorFirst, authorMiddle, authorLast});
        if (fullName.isEmpty()) {
          fullName = normalizeWhitespace(authorNick).trimmed();
        } else if (!authorNick.isEmpty()) {
          fullName = QString("%1 (%2)").arg(fullName, normalizeWhitespace(authorNick).trimmed());
        }
        if (!fullName.isEmpty()) {
          authors.append(fullName);
        }
      } else if (name == QLatin1String("title") && !stack.isEmpty()) {
        inSectionTitle = false;
        const QString sectionTitle = titleBuffer.trimmed();
        if (!sectionTitle.isEmpty()) {
          if (stack.last().title.isEmpty()) {
            stack.last().title = sectionTitle;
          }
          stack.last().htmlBlocks.append(QString("<h2>%1</h2>").arg(escapeHtml(sectionTitle)));
          stack.last().plainBlocks.append(sectionTitle);
        }
        titleBuffer.clear();
      } else if (isParagraphElement(name)) {
        flushParagraph();
      } else if (inBody && inParagraph && isInlineTag(name)) {
        currentParagraphHtml.append(closeInlineTag(name));
      } else if (name == QLatin1String("section") && !stack.isEmpty()) {
        flushParagraph();
        SectionContext ctx = stack.takeLast();
        sectionDepth--;
        const QString sectionHtml = ctx.htmlBlocks.join("\n").trimmed();
        const QString sectionPlain = ctx.plainBlocks.join("\n\n").trimmed();
        if (ctx.depth == 1) {
          QString chapterTitle = ctx.title.trimmed();
          if (chapterTitle.isEmpty()) {
            chapterTitle = QString("Section %1").arg(chapterTitles.size() + 1);
          }
          chapterTitles.append(chapterTitle);
          if (!sectionHtml.isEmpty()) {
            chapterHtml.append(sectionHtml);
          } else {
            chapterHtml.append(escapeHtml(sectionPlain));
          }
          chapterPlain.append(sectionPlain);
        }
        if (!ctx.title.trimmed().isEmpty() && ctx.topIndex >= 0) {
          tocTitles.append(ctx.title.trimmed());
          tocIndices.append(ctx.topIndex);
        }
        if (!stack.isEmpty()) {
          if (!sectionHtml.isEmpty()) {
            stack.last().htmlBlocks.append(sectionHtml);
          }
          if (!sectionPlain.isEmpty()) {
            stack.last().plainBlocks.append(sectionPlain);
          }
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

  if (chapterHtml.isEmpty() && !chapterPlain.isEmpty()) {
    for (const QString &plain : chapterPlain) {
      chapterHtml.append(escapeHtml(plain));
    }
  }

  for (int i = 0; i < chapterHtml.size(); ++i) {
    chapterHtml[i] = applyStyles(chapterHtml.at(i), renderSettings);
  }

  QString fullHtml;
  QString fullPlain;
  if (!chapterHtml.isEmpty()) {
    fullHtml = chapterHtml.join("\n\n");
    fullPlain = chapterPlain.join("\n\n");
  }

  if (fullPlain.trimmed().isEmpty()) {
    if (error) {
      *error = "No readable text in FB2";
    }
    return nullptr;
  }

  if (tocTitles.isEmpty()) {
    tocTitles = chapterTitles;
    tocIndices.clear();
    for (int i = 0; i < chapterTitles.size(); ++i) {
      tocIndices.append(i);
    }
  }

  QString coverPath;
  if (!coverId.isEmpty()) {
    coverPath = ensureImageFile(coverId, assets, outDir);
  }
  if (coverPath.isEmpty() && !fallbackImageId.isEmpty()) {
    coverPath = ensureImageFile(fallbackImageId, assets, outDir);
  }

  return std::make_unique<Fb2Document>(title,
                                       fullHtml,
                                       fullPlain,
                                       chapterTitles,
                                       chapterHtml,
                                       chapterPlain,
                                       tocTitles,
                                       tocIndices,
                                       authors.join(", "),
                                       series,
                                       publisher,
                                       description,
                                       coverPath);
}
