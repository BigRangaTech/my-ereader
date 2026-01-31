#include "TxtProvider.h"
#include "../core/include/AppPaths.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QStringDecoder>
#include <QDebug>
#include <QRegularExpression>
#include <QSet>
#include <algorithm>

namespace {
QString formatSettingsPath() {
  return AppPaths::configFile("txt.ini");
}

struct TxtSettings {
  QString encoding = "auto";
  bool normalizeLineEndings = true;
  bool trimTrailingWhitespace = false;
  int tabWidth = 4;
  int maxBlankLines = 0;
  bool splitOnFormFeed = true;
  bool autoChapters = true;
};

int clampInt(int value, int minValue, int maxValue) {
  return std::max(minValue, std::min(maxValue, value));
}

TxtSettings loadTxtSettings() {
  QSettings settings(formatSettingsPath(), QSettings::IniFormat);
  TxtSettings out;
  out.encoding = settings.value("render/encoding", "auto").toString().trimmed().toLower();
  out.normalizeLineEndings = settings.value("render/normalize_line_endings", true).toBool();
  out.trimTrailingWhitespace = settings.value("render/trim_trailing_whitespace", false).toBool();
  out.tabWidth = clampInt(settings.value("render/tab_width", 4).toInt(), 0, 16);
  out.maxBlankLines = clampInt(settings.value("render/max_blank_lines", 0).toInt(), 0, 20);
  out.splitOnFormFeed = settings.value("render/split_on_formfeed", true).toBool();
  out.autoChapters = settings.value("render/auto_chapters", true).toBool();
  return out;
}

struct DecodedText {
  QString text;
  QString encoding;
};

QString rtrim(const QString &line) {
  int end = line.size();
  while (end > 0) {
    const QChar ch = line.at(end - 1);
    if (ch != QLatin1Char(' ') && ch != QLatin1Char('\t')) {
      break;
    }
    --end;
  }
  if (end == line.size()) {
    return line;
  }
  return line.left(end);
}

QString expandTabs(const QString &line, int tabWidth) {
  if (tabWidth <= 0 || !line.contains(QLatin1Char('\t'))) {
    return line;
  }
  QString out;
  out.reserve(line.size());
  int column = 0;
  for (const QChar ch : line) {
    if (ch == QLatin1Char('\t')) {
      const int spaces = tabWidth - (column % tabWidth);
      out.append(QString(spaces, QLatin1Char(' ')));
      column += spaces;
    } else {
      out.append(ch);
      column += 1;
    }
  }
  return out;
}

DecodedText decodeText(const QByteArray &data, const TxtSettings &settings) {
  QByteArray bytes = data;
  QString encoding = settings.encoding;
  if (encoding.isEmpty()) {
    encoding = "auto";
  }

  auto decodeWith = [&](const QString &name, const QByteArray &payload) -> DecodedText {
    QStringDecoder decoder(name.toUtf8());
    if (!decoder.isValid()) {
      QStringDecoder fallback(QStringDecoder::Utf8);
      const QString fallbackText = fallback.decode(payload);
      if (fallback.hasError()) {
        return {QString::fromLatin1(payload), "latin1"};
      }
      return {fallbackText, "utf-8"};
    }
    QString text = decoder.decode(payload);
    if (decoder.hasError() && name == "utf-8") {
      return {QString::fromLatin1(payload), "latin1"};
    }
    if (decoder.hasError() && name == "auto") {
      return {QString::fromLatin1(payload), "latin1"};
    }
    if (decoder.hasError()) {
      return {text, name + " (errors)"};
    }
    return {text, name};
  };

  if (encoding != "auto") {
    return decodeWith(encoding, bytes);
  }

  if (bytes.size() >= 3
      && static_cast<unsigned char>(bytes.at(0)) == 0xEF
      && static_cast<unsigned char>(bytes.at(1)) == 0xBB
      && static_cast<unsigned char>(bytes.at(2)) == 0xBF) {
    bytes.remove(0, 3);
    return decodeWith("utf-8", bytes);
  }
  if (bytes.size() >= 4
      && static_cast<unsigned char>(bytes.at(0)) == 0x00
      && static_cast<unsigned char>(bytes.at(1)) == 0x00
      && static_cast<unsigned char>(bytes.at(2)) == 0xFE
      && static_cast<unsigned char>(bytes.at(3)) == 0xFF) {
    bytes.remove(0, 4);
    return decodeWith("utf-32be", bytes);
  }
  if (bytes.size() >= 4
      && static_cast<unsigned char>(bytes.at(0)) == 0xFF
      && static_cast<unsigned char>(bytes.at(1)) == 0xFE
      && static_cast<unsigned char>(bytes.at(2)) == 0x00
      && static_cast<unsigned char>(bytes.at(3)) == 0x00) {
    bytes.remove(0, 4);
    return decodeWith("utf-32le", bytes);
  }
  if (bytes.size() >= 2
      && static_cast<unsigned char>(bytes.at(0)) == 0xFE
      && static_cast<unsigned char>(bytes.at(1)) == 0xFF) {
    bytes.remove(0, 2);
    return decodeWith("utf-16be", bytes);
  }
  if (bytes.size() >= 2
      && static_cast<unsigned char>(bytes.at(0)) == 0xFF
      && static_cast<unsigned char>(bytes.at(1)) == 0xFE) {
    bytes.remove(0, 2);
    return decodeWith("utf-16le", bytes);
  }

  return decodeWith("utf-8", bytes);
}

QString normalizeText(const QString &input, const TxtSettings &settings) {
  QString text = input;
  if (settings.normalizeLineEndings) {
    text.replace("\r\n", "\n");
    text.replace('\r', '\n');
  }
  if (!settings.trimTrailingWhitespace && settings.tabWidth <= 0 && settings.maxBlankLines <= 0) {
    return text;
  }

  const QStringList lines = text.split('\n');
  QStringList out;
  out.reserve(lines.size());
  int blankRun = 0;
  for (const QString &line : lines) {
    QString processed = line;
    if (settings.tabWidth > 0) {
      processed = expandTabs(processed, settings.tabWidth);
    }
    if (settings.trimTrailingWhitespace) {
      processed = rtrim(processed);
    }
    const bool isBlank = processed.trimmed().isEmpty();
    if (settings.maxBlankLines > 0) {
      if (isBlank) {
        blankRun += 1;
        if (blankRun > settings.maxBlankLines) {
          continue;
        }
      } else {
        blankRun = 0;
      }
    }
    out.append(processed);
  }
  return out.join('\n');
}

struct ChapterSplit {
  QStringList titles;
  QStringList texts;
};

QString cleanHeadingTitle(QString title) {
  title = title.trimmed();
  title.remove(QRegularExpression("^#+\\s*"));
  title.remove(QRegularExpression("\\s*#+\\s*$"));
  return title.trimmed();
}

ChapterSplit splitChaptersFromHeadings(const QString &text) {
  ChapterSplit out;
  const QStringList lines = text.split('\n');
  if (lines.size() < 4) {
    return out;
  }

  struct Heading {
    int line = 0;
    QString title;
  };

  const QRegularExpression mdHeading("^\\s{0,3}(#{1,6})\\s+(.+)$");
  const QRegularExpression chapterHeading(
      "^\\s*(chapter|book|part|section|appendix)\\s+([0-9]+|[ivxlcdm]+)\\b\\s*[:\\-\\.]*\\s*(.*)$",
      QRegularExpression::CaseInsensitiveOption);
  const QRegularExpression underlineEq("^\\s*=\\s*=+\\s*$");
  const QRegularExpression underlineDash("^\\s*-\\s*-+\\s*$");

  QVector<Heading> headings;
  QSet<int> usedLines;
  QSet<int> skipLines;

  for (int i = 0; i < lines.size(); ++i) {
    const QString line = lines.at(i);
    const QString trimmed = line.trimmed();
    if (trimmed.isEmpty()) {
      continue;
    }

    const auto mdMatch = mdHeading.match(line);
    if (mdMatch.hasMatch()) {
      const QString title = cleanHeadingTitle(mdMatch.captured(2));
      if (!title.isEmpty() && !usedLines.contains(i)) {
        headings.append({i, title});
        usedLines.insert(i);
      }
      continue;
    }

    const auto chapterMatch = chapterHeading.match(line);
    if (chapterMatch.hasMatch() && !usedLines.contains(i)) {
      headings.append({i, trimmed});
      usedLines.insert(i);
      continue;
    }

    if ((underlineEq.match(line).hasMatch() || underlineDash.match(line).hasMatch()) && i > 0) {
      const QString prevLine = lines.at(i - 1);
      const QString prevTrimmed = prevLine.trimmed();
      if (!prevTrimmed.isEmpty() && !usedLines.contains(i - 1)) {
        headings.append({i - 1, prevTrimmed});
        usedLines.insert(i - 1);
        skipLines.insert(i);
      }
    }
  }

  if (headings.size() < 2) {
    return out;
  }

  std::sort(headings.begin(), headings.end(), [](const Heading &a, const Heading &b) {
    return a.line < b.line;
  });

  QVector<Heading> filtered;
  for (const Heading &heading : headings) {
    if (filtered.isEmpty() || heading.line - filtered.last().line >= 2) {
      filtered.append(heading);
    }
  }
  if (filtered.size() < 2) {
    return out;
  }

  auto appendChapter = [&](int startLine, int endLine, const QString &title) {
    if (endLine <= startLine) {
      return;
    }
    QStringList segmentLines;
    segmentLines.reserve(endLine - startLine);
    for (int lineIndex = startLine; lineIndex < endLine; ++lineIndex) {
      if (skipLines.contains(lineIndex)) {
        continue;
      }
      segmentLines.append(lines.at(lineIndex));
    }
    const QString segmentText = segmentLines.join('\n');
    if (segmentText.trimmed().isEmpty()) {
      return;
    }
    out.texts.append(segmentText);
    out.titles.append(title);
  };

  const int firstHeadingLine = filtered.first().line;
  if (firstHeadingLine > 0) {
    QStringList introLines;
    introLines.reserve(firstHeadingLine);
    for (int lineIndex = 0; lineIndex < firstHeadingLine; ++lineIndex) {
      if (skipLines.contains(lineIndex)) {
        continue;
      }
      introLines.append(lines.at(lineIndex));
    }
    const QString introText = introLines.join('\n');
    if (!introText.trimmed().isEmpty()) {
      out.texts.append(introText);
      out.titles.append("Intro");
    }
  }

  for (int i = 0; i < filtered.size(); ++i) {
    const int startLine = filtered.at(i).line;
    const int endLine = (i + 1 < filtered.size()) ? filtered.at(i + 1).line : lines.size();
    appendChapter(startLine, endLine, filtered.at(i).title);
  }

  if (out.texts.size() < 2) {
    out.texts.clear();
    out.titles.clear();
  }
  return out;
}

class TxtDocument final : public FormatDocument {
public:
  TxtDocument(QString title, QString text, QStringList chapterTitles, QStringList chapterTexts)
      : m_title(std::move(title)),
        m_text(std::move(text)),
        m_chapterTitles(std::move(chapterTitles)),
        m_chapterTexts(std::move(chapterTexts)) {}

  QString title() const override { return m_title; }
  QStringList chapterTitles() const override { return m_chapterTitles; }
  QString readAllText() const override { return m_text; }
  QStringList chaptersText() const override { return m_chapterTexts; }

private:
  QString m_title;
  QString m_text;
  QStringList m_chapterTitles;
  QStringList m_chapterTexts;
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
  if (!file.open(QIODevice::ReadOnly)) {
    if (error) {
      *error = QString("Failed to open %1").arg(path);
    }
    return nullptr;
  }

  const TxtSettings settings = loadTxtSettings();
  const QByteArray bytes = file.readAll();
  const DecodedText decoded = decodeText(bytes, settings);
  QString text = normalizeText(decoded.text, settings);
  const QString textForChapters = text;
  text.replace('\f', '\n');
  const QString title = QFileInfo(path).completeBaseName();

  if (!decoded.encoding.isEmpty()) {
    qInfo() << "TxtProvider: decoded using" << decoded.encoding << "bytes" << bytes.size();
  }

  QStringList chapterTexts;
  QStringList chapterTitles;
  if (settings.splitOnFormFeed && textForChapters.contains(QLatin1Char('\f'))) {
    const QStringList parts = textForChapters.split(QLatin1Char('\f'));
    int index = 0;
    for (const QString &part : parts) {
      if (part.trimmed().isEmpty()) {
        continue;
      }
      chapterTexts.append(part);
      chapterTitles.append(QString("Page %1").arg(++index));
    }
  }
  if (chapterTexts.isEmpty() && settings.autoChapters) {
    const ChapterSplit split = splitChaptersFromHeadings(text);
    chapterTexts = split.texts;
    chapterTitles = split.titles;
  }

  return std::make_unique<TxtDocument>(title, text, chapterTitles, chapterTexts);
}
