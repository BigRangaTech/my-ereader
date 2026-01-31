#include "include/SettingsManager.h"
#include "include/AppPaths.h"

#include <algorithm>
#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>

namespace {
int clampInt(int value, int minValue, int maxValue) {
  return std::max(minValue, std::min(maxValue, value));
}

double clampDouble(double value, double minValue, double maxValue) {
  return std::max(minValue, std::min(maxValue, value));
}

QString normalizeTextAlign(const QString &value) {
  const QString normalized = value.trimmed().toLower();
  if (normalized == "left" || normalized == "right" ||
      normalized == "center" || normalized == "justify") {
    return normalized;
  }
  return "left";
}
} // namespace

SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent),
      m_settings(resolveSettingsPath(), QSettings::IniFormat) {
  loadFromSettings();
}

QString SettingsManager::resolveSettingsPath() {
  return AppPaths::configFile("settings.ini");
}

QString SettingsManager::resolveFormatSettingsPath(const QString &format) {
  const QString key = normalizeFormatKey(format);
  return AppPaths::configFile(QString("%1.ini").arg(key));
}

QString SettingsManager::settingsPath() const {
  return m_settings.fileName();
}

QString SettingsManager::iconPath() const {
  const QString root = AppPaths::repoRoot();
  const QString path = QDir(root).filePath("icon/icon.png");
  if (QFileInfo::exists(path)) {
    return path;
  }
  return {};
}

int SettingsManager::readingFontSize() const { return m_readingFontSize; }
double SettingsManager::readingLineHeight() const { return m_readingLineHeight; }
double SettingsManager::ttsRate() const { return m_ttsRate; }
double SettingsManager::ttsPitch() const { return m_ttsPitch; }
double SettingsManager::ttsVolume() const { return m_ttsVolume; }
QString SettingsManager::ttsVoiceKey() const { return m_ttsVoiceKey; }
int SettingsManager::epubFontSize() const { return m_epubFontSize; }
double SettingsManager::epubLineHeight() const { return m_epubLineHeight; }
bool SettingsManager::epubShowImages() const { return m_epubShowImages; }
QString SettingsManager::epubTextAlign() const { return m_epubTextAlign; }
double SettingsManager::epubParagraphSpacing() const { return m_epubParagraphSpacing; }
double SettingsManager::epubParagraphIndent() const { return m_epubParagraphIndent; }
int SettingsManager::epubImageMaxWidth() const { return m_epubImageMaxWidth; }
double SettingsManager::epubImageSpacing() const { return m_epubImageSpacing; }
int SettingsManager::fb2FontSize() const { return m_fb2FontSize; }
double SettingsManager::fb2LineHeight() const { return m_fb2LineHeight; }
bool SettingsManager::fb2ShowImages() const { return m_fb2ShowImages; }
QString SettingsManager::fb2TextAlign() const { return m_fb2TextAlign; }
double SettingsManager::fb2ParagraphSpacing() const { return m_fb2ParagraphSpacing; }
double SettingsManager::fb2ParagraphIndent() const { return m_fb2ParagraphIndent; }
int SettingsManager::fb2ImageMaxWidth() const { return m_fb2ImageMaxWidth; }
double SettingsManager::fb2ImageSpacing() const { return m_fb2ImageSpacing; }
int SettingsManager::txtFontSize() const { return m_txtFontSize; }
double SettingsManager::txtLineHeight() const { return m_txtLineHeight; }
bool SettingsManager::txtMonospace() const { return m_txtMonospace; }
QString SettingsManager::txtEncoding() const { return m_txtEncoding; }
int SettingsManager::txtTabWidth() const { return m_txtTabWidth; }
bool SettingsManager::txtTrimWhitespace() const { return m_txtTrimWhitespace; }
bool SettingsManager::txtAutoChapters() const { return m_txtAutoChapters; }
int SettingsManager::mobiFontSize() const { return m_mobiFontSize; }
double SettingsManager::mobiLineHeight() const { return m_mobiLineHeight; }
bool SettingsManager::mobiShowImages() const { return m_mobiShowImages; }
QString SettingsManager::mobiTextAlign() const { return m_mobiTextAlign; }
double SettingsManager::mobiParagraphSpacing() const { return m_mobiParagraphSpacing; }
double SettingsManager::mobiParagraphIndent() const { return m_mobiParagraphIndent; }
int SettingsManager::mobiImageMaxWidth() const { return m_mobiImageMaxWidth; }
double SettingsManager::mobiImageSpacing() const { return m_mobiImageSpacing; }
int SettingsManager::pdfDpi() const { return m_pdfDpi; }
int SettingsManager::pdfCacheLimit() const { return m_pdfCacheLimit; }
int SettingsManager::pdfPrefetchDistance() const { return m_pdfPrefetchDistance; }
QString SettingsManager::pdfPrefetchStrategy() const { return m_pdfPrefetchStrategy; }
QString SettingsManager::pdfCachePolicy() const { return m_pdfCachePolicy; }
QString SettingsManager::pdfRenderPreset() const { return m_pdfRenderPreset; }
QString SettingsManager::pdfColorMode() const { return m_pdfColorMode; }
QString SettingsManager::pdfBackgroundMode() const { return m_pdfBackgroundMode; }
QString SettingsManager::pdfBackgroundColor() const { return m_pdfBackgroundColor; }
int SettingsManager::pdfMaxWidth() const { return m_pdfMaxWidth; }
int SettingsManager::pdfMaxHeight() const { return m_pdfMaxHeight; }
QString SettingsManager::pdfImageFormat() const { return m_pdfImageFormat; }
int SettingsManager::pdfJpegQuality() const { return m_pdfJpegQuality; }
bool SettingsManager::pdfExtractText() const { return m_pdfExtractText; }
int SettingsManager::pdfTileSize() const { return m_pdfTileSize; }
bool SettingsManager::pdfProgressiveRendering() const { return m_pdfProgressiveRendering; }
int SettingsManager::pdfProgressiveDpi() const { return m_pdfProgressiveDpi; }
int SettingsManager::djvuDpi() const { return m_djvuDpi; }
int SettingsManager::djvuCacheLimit() const { return m_djvuCacheLimit; }
int SettingsManager::djvuPrefetchDistance() const { return m_djvuPrefetchDistance; }
QString SettingsManager::djvuCachePolicy() const { return m_djvuCachePolicy; }
QString SettingsManager::djvuImageFormat() const { return m_djvuImageFormat; }
bool SettingsManager::djvuExtractText() const { return m_djvuExtractText; }
int SettingsManager::djvuRotation() const { return m_djvuRotation; }
double SettingsManager::comicMinZoom() const { return m_comicMinZoom; }
double SettingsManager::comicMaxZoom() const { return m_comicMaxZoom; }
QString SettingsManager::comicSortMode() const { return m_comicSortMode; }
bool SettingsManager::comicSortDescending() const { return m_comicSortDescending; }

QString SettingsManager::formatSettingsPath(const QString &format) const {
  return resolveFormatSettingsPath(format);
}

void SettingsManager::setReadingFontSize(int value) {
  value = clampInt(value, 12, 36);
  if (m_readingFontSize == value) {
    return;
  }
  m_readingFontSize = value;
  saveValue("reading/font_size", value);
  emit readingFontSizeChanged();
}

void SettingsManager::setReadingLineHeight(double value) {
  value = clampDouble(value, 1.0, 2.0);
  if (qFuzzyCompare(m_readingLineHeight, value)) {
    return;
  }
  m_readingLineHeight = value;
  saveValue("reading/line_height", value);
  emit readingLineHeightChanged();
}

void SettingsManager::setTtsRate(double value) {
  value = clampDouble(value, -1.0, 1.0);
  if (qFuzzyCompare(m_ttsRate, value)) {
    return;
  }
  m_ttsRate = value;
  saveValue("tts/rate", value);
  emit ttsRateChanged();
}

void SettingsManager::setTtsPitch(double value) {
  value = clampDouble(value, -1.0, 1.0);
  if (qFuzzyCompare(m_ttsPitch, value)) {
    return;
  }
  m_ttsPitch = value;
  saveValue("tts/pitch", value);
  emit ttsPitchChanged();
}

void SettingsManager::setTtsVolume(double value) {
  value = clampDouble(value, 0.0, 1.0);
  if (qFuzzyCompare(m_ttsVolume, value)) {
    return;
  }
  m_ttsVolume = value;
  saveValue("tts/volume", value);
  emit ttsVolumeChanged();
}

void SettingsManager::setTtsVoiceKey(const QString &value) {
  if (m_ttsVoiceKey == value) {
    return;
  }
  m_ttsVoiceKey = value;
  saveValue("tts/voice_key", value);
  emit ttsVoiceKeyChanged();
}

void SettingsManager::setEpubFontSize(int value) {
  value = clampInt(value, 12, 36);
  if (m_epubFontSize == value) {
    return;
  }
  m_epubFontSize = value;
  saveFormatValue("epub", "reading/font_size", value);
  emit epubFontSizeChanged();
}

void SettingsManager::setEpubLineHeight(double value) {
  value = clampDouble(value, 1.0, 2.0);
  if (qFuzzyCompare(m_epubLineHeight, value)) {
    return;
  }
  m_epubLineHeight = value;
  saveFormatValue("epub", "reading/line_height", value);
  emit epubLineHeightChanged();
}

void SettingsManager::setEpubShowImages(bool value) {
  if (m_epubShowImages == value) {
    return;
  }
  m_epubShowImages = value;
  saveFormatValue("epub", "render/show_images", value);
  emit epubShowImagesChanged();
}

void SettingsManager::setEpubTextAlign(const QString &value) {
  const QString normalized = normalizeTextAlign(value);
  if (m_epubTextAlign == normalized) {
    return;
  }
  m_epubTextAlign = normalized;
  saveFormatValue("epub", "render/text_align", normalized);
  emit epubTextAlignChanged();
}

void SettingsManager::setEpubParagraphSpacing(double value) {
  value = clampDouble(value, 0.0, 3.0);
  if (qFuzzyCompare(m_epubParagraphSpacing, value)) {
    return;
  }
  m_epubParagraphSpacing = value;
  saveFormatValue("epub", "render/paragraph_spacing_em", value);
  emit epubParagraphSpacingChanged();
}

void SettingsManager::setEpubParagraphIndent(double value) {
  value = clampDouble(value, 0.0, 3.0);
  if (qFuzzyCompare(m_epubParagraphIndent, value)) {
    return;
  }
  m_epubParagraphIndent = value;
  saveFormatValue("epub", "render/paragraph_indent_em", value);
  emit epubParagraphIndentChanged();
}

void SettingsManager::setEpubImageMaxWidth(int value) {
  value = clampInt(value, 10, 100);
  if (m_epubImageMaxWidth == value) {
    return;
  }
  m_epubImageMaxWidth = value;
  saveFormatValue("epub", "render/image_max_width_percent", value);
  emit epubImageMaxWidthChanged();
}

void SettingsManager::setEpubImageSpacing(double value) {
  value = clampDouble(value, 0.0, 4.0);
  if (qFuzzyCompare(m_epubImageSpacing, value)) {
    return;
  }
  m_epubImageSpacing = value;
  saveFormatValue("epub", "render/image_spacing_em", value);
  emit epubImageSpacingChanged();
}

void SettingsManager::setFb2FontSize(int value) {
  value = clampInt(value, 12, 36);
  if (m_fb2FontSize == value) {
    return;
  }
  m_fb2FontSize = value;
  saveFormatValue("fb2", "reading/font_size", value);
  emit fb2FontSizeChanged();
}

void SettingsManager::setFb2LineHeight(double value) {
  value = clampDouble(value, 1.0, 2.0);
  if (qFuzzyCompare(m_fb2LineHeight, value)) {
    return;
  }
  m_fb2LineHeight = value;
  saveFormatValue("fb2", "reading/line_height", value);
  emit fb2LineHeightChanged();
}

void SettingsManager::setFb2ShowImages(bool value) {
  if (m_fb2ShowImages == value) {
    return;
  }
  m_fb2ShowImages = value;
  saveFormatValue("fb2", "render/show_images", value);
  emit fb2ShowImagesChanged();
}

void SettingsManager::setFb2TextAlign(const QString &value) {
  const QString normalized = normalizeTextAlign(value);
  if (m_fb2TextAlign == normalized) {
    return;
  }
  m_fb2TextAlign = normalized;
  saveFormatValue("fb2", "render/text_align", normalized);
  emit fb2TextAlignChanged();
}

void SettingsManager::setFb2ParagraphSpacing(double value) {
  value = clampDouble(value, 0.0, 3.0);
  if (qFuzzyCompare(m_fb2ParagraphSpacing, value)) {
    return;
  }
  m_fb2ParagraphSpacing = value;
  saveFormatValue("fb2", "render/paragraph_spacing_em", value);
  emit fb2ParagraphSpacingChanged();
}

void SettingsManager::setFb2ParagraphIndent(double value) {
  value = clampDouble(value, 0.0, 3.0);
  if (qFuzzyCompare(m_fb2ParagraphIndent, value)) {
    return;
  }
  m_fb2ParagraphIndent = value;
  saveFormatValue("fb2", "render/paragraph_indent_em", value);
  emit fb2ParagraphIndentChanged();
}

void SettingsManager::setFb2ImageMaxWidth(int value) {
  value = clampInt(value, 10, 100);
  if (m_fb2ImageMaxWidth == value) {
    return;
  }
  m_fb2ImageMaxWidth = value;
  saveFormatValue("fb2", "render/image_max_width_percent", value);
  emit fb2ImageMaxWidthChanged();
}

void SettingsManager::setFb2ImageSpacing(double value) {
  value = clampDouble(value, 0.0, 4.0);
  if (qFuzzyCompare(m_fb2ImageSpacing, value)) {
    return;
  }
  m_fb2ImageSpacing = value;
  saveFormatValue("fb2", "render/image_spacing_em", value);
  emit fb2ImageSpacingChanged();
}

void SettingsManager::setTxtFontSize(int value) {
  value = clampInt(value, 12, 36);
  if (m_txtFontSize == value) {
    return;
  }
  m_txtFontSize = value;
  saveFormatValue("txt", "reading/font_size", value);
  emit txtFontSizeChanged();
}

void SettingsManager::setTxtLineHeight(double value) {
  value = clampDouble(value, 1.0, 2.0);
  if (qFuzzyCompare(m_txtLineHeight, value)) {
    return;
  }
  m_txtLineHeight = value;
  saveFormatValue("txt", "reading/line_height", value);
  emit txtLineHeightChanged();
}

void SettingsManager::setTxtMonospace(bool value) {
  if (m_txtMonospace == value) {
    return;
  }
  m_txtMonospace = value;
  saveFormatValue("txt", "render/monospace", value);
  emit txtMonospaceChanged();
}

void SettingsManager::setTxtEncoding(const QString &value) {
  const QString normalized = value.trimmed().toLower();
  const QString encoding = normalized.isEmpty() ? QString("auto") : normalized;
  if (m_txtEncoding == encoding) {
    return;
  }
  m_txtEncoding = encoding;
  saveFormatValue("txt", "render/encoding", encoding);
  emit txtEncodingChanged();
}

void SettingsManager::setTxtTabWidth(int value) {
  value = clampInt(value, 0, 16);
  if (m_txtTabWidth == value) {
    return;
  }
  m_txtTabWidth = value;
  saveFormatValue("txt", "render/tab_width", value);
  emit txtTabWidthChanged();
}

void SettingsManager::setTxtTrimWhitespace(bool value) {
  if (m_txtTrimWhitespace == value) {
    return;
  }
  m_txtTrimWhitespace = value;
  saveFormatValue("txt", "render/trim_trailing_whitespace", value);
  emit txtTrimWhitespaceChanged();
}

void SettingsManager::setTxtAutoChapters(bool value) {
  if (m_txtAutoChapters == value) {
    return;
  }
  m_txtAutoChapters = value;
  saveFormatValue("txt", "render/auto_chapters", value);
  emit txtAutoChaptersChanged();
}

void SettingsManager::setMobiFontSize(int value) {
  value = clampInt(value, 12, 36);
  if (m_mobiFontSize == value) {
    return;
  }
  m_mobiFontSize = value;
  saveMobiFamilyValue("reading/font_size", value);
  emit mobiFontSizeChanged();
}

void SettingsManager::setMobiLineHeight(double value) {
  value = clampDouble(value, 1.0, 2.0);
  if (qFuzzyCompare(m_mobiLineHeight, value)) {
    return;
  }
  m_mobiLineHeight = value;
  saveMobiFamilyValue("reading/line_height", value);
  emit mobiLineHeightChanged();
}

void SettingsManager::setMobiShowImages(bool value) {
  if (m_mobiShowImages == value) {
    return;
  }
  m_mobiShowImages = value;
  saveMobiFamilyValue("render/show_images", value);
  emit mobiShowImagesChanged();
}

void SettingsManager::setMobiTextAlign(const QString &value) {
  const QString normalized = normalizeTextAlign(value);
  if (m_mobiTextAlign == normalized) {
    return;
  }
  m_mobiTextAlign = normalized;
  saveMobiFamilyValue("render/text_align", normalized);
  emit mobiTextAlignChanged();
}

void SettingsManager::setMobiParagraphSpacing(double value) {
  value = clampDouble(value, 0.0, 3.0);
  if (qFuzzyCompare(m_mobiParagraphSpacing, value)) {
    return;
  }
  m_mobiParagraphSpacing = value;
  saveMobiFamilyValue("render/paragraph_spacing_em", value);
  emit mobiParagraphSpacingChanged();
}

void SettingsManager::setMobiParagraphIndent(double value) {
  value = clampDouble(value, 0.0, 3.0);
  if (qFuzzyCompare(m_mobiParagraphIndent, value)) {
    return;
  }
  m_mobiParagraphIndent = value;
  saveMobiFamilyValue("render/paragraph_indent_em", value);
  emit mobiParagraphIndentChanged();
}

void SettingsManager::setMobiImageMaxWidth(int value) {
  value = clampInt(value, 10, 100);
  if (m_mobiImageMaxWidth == value) {
    return;
  }
  m_mobiImageMaxWidth = value;
  saveMobiFamilyValue("render/image_max_width_percent", value);
  emit mobiImageMaxWidthChanged();
}

void SettingsManager::setMobiImageSpacing(double value) {
  value = clampDouble(value, 0.0, 4.0);
  if (qFuzzyCompare(m_mobiImageSpacing, value)) {
    return;
  }
  m_mobiImageSpacing = value;
  saveMobiFamilyValue("render/image_spacing_em", value);
  emit mobiImageSpacingChanged();
}

void SettingsManager::setPdfDpi(int value) {
  value = clampInt(value, 72, 240);
  if (m_pdfDpi == value) {
    return;
  }
  m_pdfDpi = value;
  saveFormatValue("pdf", "render/dpi", value);
  emit pdfDpiChanged();
}

void SettingsManager::setPdfCacheLimit(int value) {
  value = clampInt(value, 5, 120);
  if (m_pdfCacheLimit == value) {
    return;
  }
  m_pdfCacheLimit = value;
  saveFormatValue("pdf", "render/cache_limit", value);
  emit pdfCacheLimitChanged();
}

void SettingsManager::setPdfPrefetchDistance(int value) {
  value = clampInt(value, 0, 6);
  if (m_pdfPrefetchDistance == value) {
    return;
  }
  m_pdfPrefetchDistance = value;
  saveFormatValue("pdf", "render/prefetch_distance", value);
  emit pdfPrefetchDistanceChanged();
}

void SettingsManager::setPdfPrefetchStrategy(const QString &value) {
  QString normalized = value.trimmed().toLower();
  if (normalized != "forward" && normalized != "symmetric" && normalized != "backward") {
    normalized = "symmetric";
  }
  if (m_pdfPrefetchStrategy == normalized) {
    return;
  }
  m_pdfPrefetchStrategy = normalized;
  saveFormatValue("pdf", "render/prefetch_strategy", normalized);
  emit pdfPrefetchStrategyChanged();
}

void SettingsManager::setPdfCachePolicy(const QString &value) {
  QString normalized = value.trimmed().toLower();
  if (normalized != "fifo" && normalized != "lru") {
    normalized = "fifo";
  }
  if (m_pdfCachePolicy == normalized) {
    return;
  }
  m_pdfCachePolicy = normalized;
  saveFormatValue("pdf", "render/cache_policy", normalized);
  emit pdfCachePolicyChanged();
}

void SettingsManager::setPdfRenderPreset(const QString &value) {
  QString normalized = value.trimmed().toLower();
  if (normalized != "custom" && normalized != "fast" && normalized != "balanced" && normalized != "high") {
    normalized = "custom";
  }
  if (m_pdfRenderPreset == normalized) {
    return;
  }
  m_pdfRenderPreset = normalized;
  saveFormatValue("pdf", "render/preset", normalized);
  emit pdfRenderPresetChanged();
}

void SettingsManager::setPdfColorMode(const QString &value) {
  QString normalized = value.trimmed().toLower();
  if (normalized != "color" && normalized != "grayscale") {
    normalized = "color";
  }
  if (m_pdfColorMode == normalized) {
    return;
  }
  m_pdfColorMode = normalized;
  saveFormatValue("pdf", "render/color_mode", normalized);
  emit pdfColorModeChanged();
}

void SettingsManager::setPdfBackgroundMode(const QString &value) {
  QString normalized = value.trimmed().toLower();
  if (normalized != "white" && normalized != "transparent" && normalized != "theme" && normalized != "custom") {
    normalized = "white";
  }
  if (m_pdfBackgroundMode == normalized) {
    return;
  }
  m_pdfBackgroundMode = normalized;
  saveFormatValue("pdf", "render/background_mode", normalized);
  emit pdfBackgroundModeChanged();
}

void SettingsManager::setPdfBackgroundColor(const QString &value) {
  QString normalized = value.trimmed();
  if (!normalized.startsWith('#')) {
    normalized.prepend('#');
  }
  if (m_pdfBackgroundColor == normalized) {
    return;
  }
  m_pdfBackgroundColor = normalized;
  saveFormatValue("pdf", "render/background_color", normalized);
  emit pdfBackgroundColorChanged();
}

void SettingsManager::setPdfMaxWidth(int value) {
  value = clampInt(value, 0, 20000);
  if (m_pdfMaxWidth == value) {
    return;
  }
  m_pdfMaxWidth = value;
  saveFormatValue("pdf", "render/max_width", value);
  emit pdfMaxWidthChanged();
}

void SettingsManager::setPdfMaxHeight(int value) {
  value = clampInt(value, 0, 20000);
  if (m_pdfMaxHeight == value) {
    return;
  }
  m_pdfMaxHeight = value;
  saveFormatValue("pdf", "render/max_height", value);
  emit pdfMaxHeightChanged();
}

void SettingsManager::setPdfImageFormat(const QString &value) {
  QString normalized = value.trimmed().toLower();
  if (normalized != "png" && normalized != "jpeg" && normalized != "jpg") {
    normalized = "png";
  }
  if (normalized == "jpg") {
    normalized = "jpeg";
  }
  if (m_pdfImageFormat == normalized) {
    return;
  }
  m_pdfImageFormat = normalized;
  saveFormatValue("pdf", "render/image_format", normalized);
  emit pdfImageFormatChanged();
}

void SettingsManager::setPdfJpegQuality(int value) {
  value = clampInt(value, 1, 100);
  if (m_pdfJpegQuality == value) {
    return;
  }
  m_pdfJpegQuality = value;
  saveFormatValue("pdf", "render/jpeg_quality", value);
  emit pdfJpegQualityChanged();
}

void SettingsManager::setPdfExtractText(bool value) {
  if (m_pdfExtractText == value) {
    return;
  }
  m_pdfExtractText = value;
  saveFormatValue("pdf", "render/extract_text", value);
  emit pdfExtractTextChanged();
}

void SettingsManager::setPdfTileSize(int value) {
  value = clampInt(value, 0, 8192);
  if (m_pdfTileSize == value) {
    return;
  }
  m_pdfTileSize = value;
  saveFormatValue("pdf", "render/tile_size", value);
  emit pdfTileSizeChanged();
}

void SettingsManager::setPdfProgressiveRendering(bool value) {
  if (m_pdfProgressiveRendering == value) {
    return;
  }
  m_pdfProgressiveRendering = value;
  saveFormatValue("pdf", "render/progressive", value);
  emit pdfProgressiveRenderingChanged();
}

void SettingsManager::setPdfProgressiveDpi(int value) {
  value = clampInt(value, 48, m_pdfDpi);
  if (m_pdfProgressiveDpi == value) {
    return;
  }
  m_pdfProgressiveDpi = value;
  saveFormatValue("pdf", "render/progressive_dpi", value);
  emit pdfProgressiveDpiChanged();
}

void SettingsManager::setDjvuDpi(int value) {
  value = clampInt(value, 72, 240);
  if (m_djvuDpi == value) {
    return;
  }
  m_djvuDpi = value;
  saveFormatValue("djvu", "render/dpi", value);
  emit djvuDpiChanged();
}

void SettingsManager::setDjvuCacheLimit(int value) {
  value = clampInt(value, 5, 120);
  if (m_djvuCacheLimit == value) {
    return;
  }
  m_djvuCacheLimit = value;
  saveFormatValue("djvu", "render/cache_limit", value);
  emit djvuCacheLimitChanged();
}

void SettingsManager::setDjvuPrefetchDistance(int value) {
  value = clampInt(value, 0, 6);
  if (m_djvuPrefetchDistance == value) {
    return;
  }
  m_djvuPrefetchDistance = value;
  saveFormatValue("djvu", "render/prefetch_distance", value);
  emit djvuPrefetchDistanceChanged();
}

void SettingsManager::setDjvuCachePolicy(const QString &value) {
  QString normalized = value.trimmed().toLower();
  if (normalized != "fifo" && normalized != "lru") {
    normalized = "fifo";
  }
  if (m_djvuCachePolicy == normalized) {
    return;
  }
  m_djvuCachePolicy = normalized;
  saveFormatValue("djvu", "render/cache_policy", normalized);
  emit djvuCachePolicyChanged();
}

void SettingsManager::setDjvuImageFormat(const QString &value) {
  QString normalized = value.trimmed().toLower();
  if (normalized != "ppm" && normalized != "tiff") {
    normalized = "ppm";
  }
  if (m_djvuImageFormat == normalized) {
    return;
  }
  m_djvuImageFormat = normalized;
  saveFormatValue("djvu", "render/format", normalized);
  emit djvuImageFormatChanged();
}

void SettingsManager::setDjvuExtractText(bool value) {
  if (m_djvuExtractText == value) {
    return;
  }
  m_djvuExtractText = value;
  saveFormatValue("djvu", "render/extract_text", value);
  emit djvuExtractTextChanged();
}

void SettingsManager::setDjvuRotation(int value) {
  int normalized = value;
  if (normalized != 0 && normalized != 90 && normalized != 180 && normalized != 270) {
    normalized = 0;
  }
  if (m_djvuRotation == normalized) {
    return;
  }
  m_djvuRotation = normalized;
  saveFormatValue("djvu", "render/rotation", normalized);
  emit djvuRotationChanged();
}

void SettingsManager::setComicMinZoom(double value) {
  value = clampDouble(value, 0.2, m_comicMaxZoom - 0.1);
  if (qFuzzyCompare(m_comicMinZoom, value)) {
    return;
  }
  m_comicMinZoom = value;
  saveComicValue("zoom/min", value);
  emit comicMinZoomChanged();
}

void SettingsManager::setComicMaxZoom(double value) {
  value = clampDouble(value, m_comicMinZoom + 0.1, 8.0);
  if (qFuzzyCompare(m_comicMaxZoom, value)) {
    return;
  }
  m_comicMaxZoom = value;
  saveComicValue("zoom/max", value);
  emit comicMaxZoomChanged();
}

void SettingsManager::setComicSortMode(const QString &value) {
  const QString normalized = value.trimmed().toLower();
  QString mode = normalized;
  if (mode != "path" && mode != "filename" && mode != "archive") {
    mode = "path";
  }
  if (m_comicSortMode == mode) {
    return;
  }
  m_comicSortMode = mode;
  saveComicValue("render/sort_mode", mode);
  emit comicSortModeChanged();
}

void SettingsManager::setComicSortDescending(bool value) {
  if (m_comicSortDescending == value) {
    return;
  }
  m_comicSortDescending = value;
  saveComicValue("render/sort_desc", value);
  emit comicSortDescendingChanged();
}

void SettingsManager::resetDefaults() {
  setReadingFontSize(20);
  setReadingLineHeight(1.4);
  setTtsRate(0.0);
  setTtsPitch(0.0);
  setTtsVolume(1.0);
  setTtsVoiceKey("");
  setEpubFontSize(20);
  setEpubLineHeight(1.4);
  setEpubShowImages(true);
  setEpubTextAlign("left");
  setEpubParagraphSpacing(0.6);
  setEpubParagraphIndent(0.0);
  setEpubImageMaxWidth(100);
  setEpubImageSpacing(0.6);
  setFb2FontSize(20);
  setFb2LineHeight(1.4);
  setFb2ShowImages(true);
  setFb2TextAlign("left");
  setFb2ParagraphSpacing(0.6);
  setFb2ParagraphIndent(0.0);
  setFb2ImageMaxWidth(100);
  setFb2ImageSpacing(0.6);
  setTxtFontSize(20);
  setTxtLineHeight(1.4);
  setTxtMonospace(false);
  setTxtEncoding("auto");
  setTxtTabWidth(4);
  setTxtTrimWhitespace(false);
  setTxtAutoChapters(true);
  setMobiFontSize(20);
  setMobiLineHeight(1.4);
  setMobiShowImages(true);
  setMobiTextAlign("left");
  setMobiParagraphSpacing(0.6);
  setMobiParagraphIndent(0.0);
  setMobiImageMaxWidth(100);
  setMobiImageSpacing(0.6);
  setPdfDpi(120);
  setPdfCacheLimit(30);
  setPdfPrefetchDistance(1);
  setPdfPrefetchStrategy("symmetric");
  setPdfCachePolicy("fifo");
  setPdfRenderPreset("custom");
  setPdfColorMode("color");
  setPdfBackgroundMode("white");
  setPdfBackgroundColor("#202633");
  setPdfMaxWidth(0);
  setPdfMaxHeight(0);
  setPdfImageFormat("png");
  setPdfJpegQuality(85);
  setPdfExtractText(true);
  setPdfTileSize(0);
  setPdfProgressiveRendering(false);
  setPdfProgressiveDpi(72);
  setDjvuDpi(120);
  setDjvuCacheLimit(30);
  setDjvuPrefetchDistance(1);
  setDjvuCachePolicy("fifo");
  setDjvuImageFormat("ppm");
  setDjvuExtractText(true);
  setDjvuRotation(0);
  setComicMinZoom(0.5);
  setComicMaxZoom(4.0);
  setComicSortMode("path");
  setComicSortDescending(false);
}

void SettingsManager::resetPdfDefaults() {
  setPdfRenderPreset("custom");
  setPdfDpi(120);
  setPdfCacheLimit(30);
  setPdfCachePolicy("fifo");
  setPdfPrefetchDistance(1);
  setPdfPrefetchStrategy("symmetric");
  setPdfProgressiveRendering(false);
  setPdfProgressiveDpi(72);
  setPdfColorMode("color");
  setPdfBackgroundMode("white");
  setPdfBackgroundColor("#202633");
  setPdfMaxWidth(0);
  setPdfMaxHeight(0);
  setPdfImageFormat("png");
  setPdfJpegQuality(85);
  setPdfExtractText(true);
  setPdfTileSize(0);
}

void SettingsManager::resetEpubDefaults() {
  setEpubFontSize(20);
  setEpubLineHeight(1.4);
  setEpubShowImages(true);
  setEpubTextAlign("left");
  setEpubParagraphSpacing(0.6);
  setEpubParagraphIndent(0.0);
  setEpubImageMaxWidth(100);
  setEpubImageSpacing(0.6);
}

void SettingsManager::resetFb2Defaults() {
  setFb2FontSize(20);
  setFb2LineHeight(1.4);
  setFb2ShowImages(true);
  setFb2TextAlign("left");
  setFb2ParagraphSpacing(0.6);
  setFb2ParagraphIndent(0.0);
  setFb2ImageMaxWidth(100);
  setFb2ImageSpacing(0.6);
}

void SettingsManager::resetTxtDefaults() {
  setTxtFontSize(20);
  setTxtLineHeight(1.4);
  setTxtMonospace(false);
  setTxtEncoding("auto");
  setTxtTabWidth(4);
  setTxtTrimWhitespace(false);
  setTxtAutoChapters(true);
}

void SettingsManager::resetMobiDefaults() {
  setMobiFontSize(20);
  setMobiLineHeight(1.4);
  setMobiShowImages(true);
  setMobiTextAlign("left");
  setMobiParagraphSpacing(0.6);
  setMobiParagraphIndent(0.0);
  setMobiImageMaxWidth(100);
  setMobiImageSpacing(0.6);
}

void SettingsManager::resetComicDefaults() {
  setComicMinZoom(0.5);
  setComicMaxZoom(4.0);
  setComicSortMode("path");
  setComicSortDescending(false);
}

void SettingsManager::resetDjvuDefaults() {
  setDjvuDpi(120);
  setDjvuCacheLimit(30);
  setDjvuPrefetchDistance(1);
  setDjvuCachePolicy("fifo");
  setDjvuImageFormat("ppm");
  setDjvuExtractText(true);
  setDjvuRotation(0);
}

void SettingsManager::reload() {
  loadFromSettings();
}

void SettingsManager::loadFromSettings() {
  m_readingFontSize = clampInt(m_settings.value("reading/font_size", 20).toInt(), 12, 36);
  m_readingLineHeight = clampDouble(m_settings.value("reading/line_height", 1.4).toDouble(), 1.0, 2.0);
  m_ttsRate = clampDouble(m_settings.value("tts/rate", 0.0).toDouble(), -1.0, 1.0);
  m_ttsPitch = clampDouble(m_settings.value("tts/pitch", 0.0).toDouble(), -1.0, 1.0);
  m_ttsVolume = clampDouble(m_settings.value("tts/volume", 1.0).toDouble(), 0.0, 1.0);
  m_ttsVoiceKey = m_settings.value("tts/voice_key", "").toString();

  m_epubFontSize = clampInt(readFormatValue("epub", "reading/font_size", m_readingFontSize).toInt(), 12, 36);
  m_epubLineHeight =
      clampDouble(readFormatValue("epub", "reading/line_height", m_readingLineHeight).toDouble(), 1.0, 2.0);
  m_epubShowImages = readFormatValue("epub", "render/show_images", true).toBool();
  m_epubTextAlign = normalizeTextAlign(readFormatValue("epub", "render/text_align", "left").toString());
  m_epubParagraphSpacing =
      clampDouble(readFormatValue("epub", "render/paragraph_spacing_em", 0.6).toDouble(), 0.0, 3.0);
  m_epubParagraphIndent =
      clampDouble(readFormatValue("epub", "render/paragraph_indent_em", 0.0).toDouble(), 0.0, 3.0);
  m_epubImageMaxWidth =
      clampInt(readFormatValue("epub", "render/image_max_width_percent", 100).toInt(), 10, 100);
  m_epubImageSpacing =
      clampDouble(readFormatValue("epub", "render/image_spacing_em", 0.6).toDouble(), 0.0, 4.0);

  m_fb2FontSize = clampInt(readFormatValue("fb2", "reading/font_size", m_readingFontSize).toInt(), 12, 36);
  m_fb2LineHeight =
      clampDouble(readFormatValue("fb2", "reading/line_height", m_readingLineHeight).toDouble(), 1.0, 2.0);
  m_fb2ShowImages = readFormatValue("fb2", "render/show_images", true).toBool();
  m_fb2TextAlign = normalizeTextAlign(readFormatValue("fb2", "render/text_align", "left").toString());
  m_fb2ParagraphSpacing =
      clampDouble(readFormatValue("fb2", "render/paragraph_spacing_em", 0.6).toDouble(), 0.0, 3.0);
  m_fb2ParagraphIndent =
      clampDouble(readFormatValue("fb2", "render/paragraph_indent_em", 0.0).toDouble(), 0.0, 3.0);
  m_fb2ImageMaxWidth =
      clampInt(readFormatValue("fb2", "render/image_max_width_percent", 100).toInt(), 10, 100);
  m_fb2ImageSpacing =
      clampDouble(readFormatValue("fb2", "render/image_spacing_em", 0.6).toDouble(), 0.0, 4.0);

  m_txtFontSize = clampInt(readFormatValue("txt", "reading/font_size", m_readingFontSize).toInt(), 12, 36);
  m_txtLineHeight =
      clampDouble(readFormatValue("txt", "reading/line_height", m_readingLineHeight).toDouble(), 1.0, 2.0);
  m_txtMonospace = readFormatValue("txt", "render/monospace", false).toBool();
  m_txtEncoding = readFormatValue("txt", "render/encoding", "auto").toString().trimmed().toLower();
  if (m_txtEncoding.isEmpty()) {
    m_txtEncoding = "auto";
  }
  m_txtTabWidth = clampInt(readFormatValue("txt", "render/tab_width", 4).toInt(), 0, 16);
  m_txtTrimWhitespace = readFormatValue("txt", "render/trim_trailing_whitespace", false).toBool();
  m_txtAutoChapters = readFormatValue("txt", "render/auto_chapters", true).toBool();

  m_mobiFontSize = clampInt(readFormatValue("mobi", "reading/font_size", m_readingFontSize).toInt(), 12, 36);
  m_mobiLineHeight =
      clampDouble(readFormatValue("mobi", "reading/line_height", m_readingLineHeight).toDouble(), 1.0, 2.0);
  m_mobiShowImages = readFormatValue("mobi", "render/show_images", true).toBool();
  m_mobiTextAlign = normalizeTextAlign(readFormatValue("mobi", "render/text_align", "left").toString());
  m_mobiParagraphSpacing =
      clampDouble(readFormatValue("mobi", "render/paragraph_spacing_em", 0.6).toDouble(), 0.0, 3.0);
  m_mobiParagraphIndent =
      clampDouble(readFormatValue("mobi", "render/paragraph_indent_em", 0.0).toDouble(), 0.0, 3.0);
  m_mobiImageMaxWidth =
      clampInt(readFormatValue("mobi", "render/image_max_width_percent", 100).toInt(), 10, 100);
  m_mobiImageSpacing =
      clampDouble(readFormatValue("mobi", "render/image_spacing_em", 0.6).toDouble(), 0.0, 4.0);

  m_pdfDpi = clampInt(readFormatValue("pdf", "render/dpi", m_settings.value("pdf/dpi", 120)).toInt(), 72, 240);
  m_pdfCacheLimit =
      clampInt(readFormatValue("pdf", "render/cache_limit", m_settings.value("pdf/cache_limit", 30)).toInt(), 5, 120);
  m_pdfPrefetchDistance =
      clampInt(readFormatValue("pdf", "render/prefetch_distance", 1).toInt(), 0, 6);
  m_pdfPrefetchStrategy =
      readFormatValue("pdf", "render/prefetch_strategy", "symmetric").toString().toLower();
  if (m_pdfPrefetchStrategy != "forward" && m_pdfPrefetchStrategy != "symmetric" &&
      m_pdfPrefetchStrategy != "backward") {
    m_pdfPrefetchStrategy = "symmetric";
  }
  m_pdfCachePolicy =
      readFormatValue("pdf", "render/cache_policy", "fifo").toString().toLower();
  if (m_pdfCachePolicy != "fifo" && m_pdfCachePolicy != "lru") {
    m_pdfCachePolicy = "fifo";
  }
  m_pdfRenderPreset =
      readFormatValue("pdf", "render/preset", "custom").toString().toLower();
  if (m_pdfRenderPreset != "custom" && m_pdfRenderPreset != "fast" &&
      m_pdfRenderPreset != "balanced" && m_pdfRenderPreset != "high") {
    m_pdfRenderPreset = "custom";
  }
  m_pdfColorMode =
      readFormatValue("pdf", "render/color_mode", "color").toString().toLower();
  if (m_pdfColorMode != "color" && m_pdfColorMode != "grayscale") {
    m_pdfColorMode = "color";
  }
  m_pdfBackgroundMode =
      readFormatValue("pdf", "render/background_mode", "white").toString().toLower();
  if (m_pdfBackgroundMode != "white" && m_pdfBackgroundMode != "transparent" &&
      m_pdfBackgroundMode != "theme" && m_pdfBackgroundMode != "custom") {
    m_pdfBackgroundMode = "white";
  }
  m_pdfBackgroundColor =
      readFormatValue("pdf", "render/background_color", "#202633").toString();
  m_pdfMaxWidth =
      clampInt(readFormatValue("pdf", "render/max_width", 0).toInt(), 0, 20000);
  m_pdfMaxHeight =
      clampInt(readFormatValue("pdf", "render/max_height", 0).toInt(), 0, 20000);
  m_pdfImageFormat =
      readFormatValue("pdf", "render/image_format", "png").toString().toLower();
  if (m_pdfImageFormat == "jpg") {
    m_pdfImageFormat = "jpeg";
  }
  if (m_pdfImageFormat != "png" && m_pdfImageFormat != "jpeg") {
    m_pdfImageFormat = "png";
  }
  m_pdfJpegQuality =
      clampInt(readFormatValue("pdf", "render/jpeg_quality", 85).toInt(), 1, 100);
  m_pdfExtractText =
      readFormatValue("pdf", "render/extract_text", true).toBool();
  m_pdfTileSize =
      clampInt(readFormatValue("pdf", "render/tile_size", 0).toInt(), 0, 8192);
  m_pdfProgressiveRendering =
      readFormatValue("pdf", "render/progressive", false).toBool();
  m_pdfProgressiveDpi =
      clampInt(readFormatValue("pdf", "render/progressive_dpi", 72).toInt(), 48, m_pdfDpi);

  m_djvuDpi =
      clampInt(readFormatValue("djvu", "render/dpi", 120).toInt(), 72, 240);
  m_djvuCacheLimit =
      clampInt(readFormatValue("djvu", "render/cache_limit", 30).toInt(), 5, 120);
  m_djvuPrefetchDistance =
      clampInt(readFormatValue("djvu", "render/prefetch_distance", 1).toInt(), 0, 6);
  m_djvuCachePolicy =
      readFormatValue("djvu", "render/cache_policy", "fifo").toString().toLower();
  if (m_djvuCachePolicy != "fifo" && m_djvuCachePolicy != "lru") {
    m_djvuCachePolicy = "fifo";
  }
  m_djvuImageFormat =
      readFormatValue("djvu", "render/format", "ppm").toString().toLower();
  if (m_djvuImageFormat != "ppm" && m_djvuImageFormat != "tiff") {
    m_djvuImageFormat = "ppm";
  }
  m_djvuExtractText =
      readFormatValue("djvu", "render/extract_text", true).toBool();
  m_djvuRotation =
      clampInt(readFormatValue("djvu", "render/rotation", 0).toInt(), 0, 270);
  if (m_djvuRotation != 0 && m_djvuRotation != 90 &&
      m_djvuRotation != 180 && m_djvuRotation != 270) {
    m_djvuRotation = 0;
  }

  m_comicMinZoom =
      clampDouble(readFormatValue("cbz", "zoom/min", m_settings.value("comics/min_zoom", 0.5)).toDouble(), 0.2, 7.0);
  m_comicMaxZoom =
      clampDouble(readFormatValue("cbz", "zoom/max", m_settings.value("comics/max_zoom", 4.0)).toDouble(),
                  m_comicMinZoom + 0.1, 8.0);
  m_comicSortMode =
      readFormatValue("cbz", "render/sort_mode", "path").toString().trimmed().toLower();
  if (m_comicSortMode != "path" && m_comicSortMode != "filename" && m_comicSortMode != "archive") {
    m_comicSortMode = "path";
  }
  m_comicSortDescending = readFormatValue("cbz", "render/sort_desc", false).toBool();

  saveFormatValue("epub", "reading/font_size", m_epubFontSize);
  saveFormatValue("epub", "reading/line_height", m_epubLineHeight);
  saveFormatValue("epub", "render/show_images", m_epubShowImages);
  saveFormatValue("epub", "render/text_align", m_epubTextAlign);
  saveFormatValue("epub", "render/paragraph_spacing_em", m_epubParagraphSpacing);
  saveFormatValue("epub", "render/paragraph_indent_em", m_epubParagraphIndent);
  saveFormatValue("epub", "render/image_max_width_percent", m_epubImageMaxWidth);
  saveFormatValue("epub", "render/image_spacing_em", m_epubImageSpacing);
  saveFormatValue("fb2", "reading/font_size", m_fb2FontSize);
  saveFormatValue("fb2", "reading/line_height", m_fb2LineHeight);
  saveFormatValue("fb2", "render/show_images", m_fb2ShowImages);
  saveFormatValue("fb2", "render/text_align", m_fb2TextAlign);
  saveFormatValue("fb2", "render/paragraph_spacing_em", m_fb2ParagraphSpacing);
  saveFormatValue("fb2", "render/paragraph_indent_em", m_fb2ParagraphIndent);
  saveFormatValue("fb2", "render/image_max_width_percent", m_fb2ImageMaxWidth);
  saveFormatValue("fb2", "render/image_spacing_em", m_fb2ImageSpacing);
  saveFormatValue("txt", "reading/font_size", m_txtFontSize);
  saveFormatValue("txt", "reading/line_height", m_txtLineHeight);
  saveFormatValue("txt", "render/monospace", m_txtMonospace);
  saveFormatValue("txt", "render/encoding", m_txtEncoding);
  saveFormatValue("txt", "render/tab_width", m_txtTabWidth);
  saveFormatValue("txt", "render/trim_trailing_whitespace", m_txtTrimWhitespace);
  saveFormatValue("txt", "render/auto_chapters", m_txtAutoChapters);
  saveFormatValue("mobi", "reading/font_size", m_mobiFontSize);
  saveFormatValue("mobi", "reading/line_height", m_mobiLineHeight);
  saveFormatValue("azw", "reading/font_size", m_mobiFontSize);
  saveFormatValue("azw", "reading/line_height", m_mobiLineHeight);
  saveFormatValue("azw3", "reading/font_size", m_mobiFontSize);
  saveFormatValue("azw3", "reading/line_height", m_mobiLineHeight);
  saveFormatValue("azw4", "reading/font_size", m_mobiFontSize);
  saveFormatValue("azw4", "reading/line_height", m_mobiLineHeight);
  saveFormatValue("prc", "reading/font_size", m_mobiFontSize);
  saveFormatValue("prc", "reading/line_height", m_mobiLineHeight);
  saveMobiFamilyValue("render/show_images", m_mobiShowImages);
  saveMobiFamilyValue("render/text_align", m_mobiTextAlign);
  saveMobiFamilyValue("render/paragraph_spacing_em", m_mobiParagraphSpacing);
  saveMobiFamilyValue("render/paragraph_indent_em", m_mobiParagraphIndent);
  saveMobiFamilyValue("render/image_max_width_percent", m_mobiImageMaxWidth);
  saveMobiFamilyValue("render/image_spacing_em", m_mobiImageSpacing);
  saveFormatValue("pdf", "render/dpi", m_pdfDpi);
  saveFormatValue("pdf", "render/cache_limit", m_pdfCacheLimit);
  saveFormatValue("pdf", "render/prefetch_distance", m_pdfPrefetchDistance);
  saveFormatValue("pdf", "render/prefetch_strategy", m_pdfPrefetchStrategy);
  saveFormatValue("pdf", "render/cache_policy", m_pdfCachePolicy);
  saveFormatValue("pdf", "render/preset", m_pdfRenderPreset);
  saveFormatValue("pdf", "render/color_mode", m_pdfColorMode);
  saveFormatValue("pdf", "render/background_mode", m_pdfBackgroundMode);
  saveFormatValue("pdf", "render/background_color", m_pdfBackgroundColor);
  saveFormatValue("pdf", "render/max_width", m_pdfMaxWidth);
  saveFormatValue("pdf", "render/max_height", m_pdfMaxHeight);
  saveFormatValue("pdf", "render/image_format", m_pdfImageFormat);
  saveFormatValue("pdf", "render/jpeg_quality", m_pdfJpegQuality);
  saveFormatValue("pdf", "render/extract_text", m_pdfExtractText);
  saveFormatValue("pdf", "render/tile_size", m_pdfTileSize);
  saveFormatValue("pdf", "render/progressive", m_pdfProgressiveRendering);
  saveFormatValue("pdf", "render/progressive_dpi", m_pdfProgressiveDpi);
  saveFormatValue("djvu", "render/dpi", m_djvuDpi);
  saveFormatValue("djvu", "render/cache_limit", m_djvuCacheLimit);
  saveFormatValue("djvu", "render/prefetch_distance", m_djvuPrefetchDistance);
  saveFormatValue("djvu", "render/cache_policy", m_djvuCachePolicy);
  saveFormatValue("djvu", "render/format", m_djvuImageFormat);
  saveFormatValue("djvu", "render/extract_text", m_djvuExtractText);
  saveFormatValue("djvu", "render/rotation", m_djvuRotation);
  saveComicValue("zoom/min", m_comicMinZoom);
  saveComicValue("zoom/max", m_comicMaxZoom);
  saveComicValue("render/sort_mode", m_comicSortMode);
  saveComicValue("render/sort_desc", m_comicSortDescending);

  emit readingFontSizeChanged();
  emit readingLineHeightChanged();
  emit epubFontSizeChanged();
  emit epubLineHeightChanged();
  emit epubShowImagesChanged();
  emit epubTextAlignChanged();
  emit epubParagraphSpacingChanged();
  emit epubParagraphIndentChanged();
  emit epubImageMaxWidthChanged();
  emit epubImageSpacingChanged();
  emit fb2FontSizeChanged();
  emit fb2LineHeightChanged();
  emit fb2ShowImagesChanged();
  emit fb2TextAlignChanged();
  emit fb2ParagraphSpacingChanged();
  emit fb2ParagraphIndentChanged();
  emit fb2ImageMaxWidthChanged();
  emit fb2ImageSpacingChanged();
  emit txtFontSizeChanged();
  emit txtLineHeightChanged();
  emit txtMonospaceChanged();
  emit txtEncodingChanged();
  emit txtTabWidthChanged();
  emit txtTrimWhitespaceChanged();
  emit txtAutoChaptersChanged();
  emit mobiFontSizeChanged();
  emit mobiLineHeightChanged();
  emit mobiShowImagesChanged();
  emit mobiTextAlignChanged();
  emit mobiParagraphSpacingChanged();
  emit mobiParagraphIndentChanged();
  emit mobiImageMaxWidthChanged();
  emit mobiImageSpacingChanged();
  emit pdfDpiChanged();
  emit pdfCacheLimitChanged();
  emit pdfPrefetchDistanceChanged();
  emit pdfPrefetchStrategyChanged();
  emit pdfCachePolicyChanged();
  emit pdfRenderPresetChanged();
  emit pdfColorModeChanged();
  emit pdfBackgroundModeChanged();
  emit pdfBackgroundColorChanged();
  emit pdfMaxWidthChanged();
  emit pdfMaxHeightChanged();
  emit pdfImageFormatChanged();
  emit pdfJpegQualityChanged();
  emit pdfExtractTextChanged();
  emit pdfTileSizeChanged();
  emit pdfProgressiveRenderingChanged();
  emit pdfProgressiveDpiChanged();
  emit djvuDpiChanged();
  emit djvuCacheLimitChanged();
  emit djvuPrefetchDistanceChanged();
  emit djvuCachePolicyChanged();
  emit djvuImageFormatChanged();
  emit djvuExtractTextChanged();
  emit djvuRotationChanged();
  emit comicMinZoomChanged();
  emit comicMaxZoomChanged();
  emit comicSortModeChanged();
  emit comicSortDescendingChanged();
}

void SettingsManager::saveValue(const QString &key, const QVariant &value) {
  m_settings.setValue(key, value);
  m_settings.sync();
}

QString SettingsManager::normalizeFormatKey(const QString &format) {
  const QString lower = format.trimmed().toLower();
  if (lower == "cbz" || lower == "cbr") {
    return lower;
  }
  return lower.isEmpty() ? QString("general") : lower;
}

QVariant SettingsManager::readFormatValue(const QString &format, const QString &key, const QVariant &fallback) const {
  QSettings settings(resolveFormatSettingsPath(format), QSettings::IniFormat);
  return settings.value(key, fallback);
}

void SettingsManager::saveFormatValue(const QString &format, const QString &key, const QVariant &value) const {
  QSettings settings(resolveFormatSettingsPath(format), QSettings::IniFormat);
  settings.setValue(key, value);
  settings.sync();
}

void SettingsManager::saveComicValue(const QString &key, const QVariant &value) const {
  saveFormatValue("cbz", key, value);
  saveFormatValue("cbr", key, value);
}

void SettingsManager::saveMobiFamilyValue(const QString &key, const QVariant &value) const {
  saveFormatValue("mobi", key, value);
  saveFormatValue("azw", key, value);
  saveFormatValue("azw3", key, value);
  saveFormatValue("azw4", key, value);
  saveFormatValue("prc", key, value);
}

QString SettingsManager::sidebarModeForPath(const QString &path) const {
  const QByteArray hash = QCryptographicHash::hash(path.toUtf8(), QCryptographicHash::Sha1).toHex();
  return m_settings.value(QString("reader/sidebar/%1").arg(QString::fromUtf8(hash)), "toc").toString();
}

void SettingsManager::setSidebarModeForPath(const QString &path, const QString &mode) {
  if (path.isEmpty()) {
    return;
  }
  const QByteArray hash = QCryptographicHash::hash(path.toUtf8(), QCryptographicHash::Sha1).toHex();
  m_settings.setValue(QString("reader/sidebar/%1").arg(QString::fromUtf8(hash)), mode);
  m_settings.sync();
}
