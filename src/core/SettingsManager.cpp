#include "include/SettingsManager.h"

#include <algorithm>
#include <QCoreApplication>
#include <QDir>
#include <QCryptographicHash>
#include <QFileInfo>

namespace {
QString findRepoRoot() {
  QDir dir(QCoreApplication::applicationDirPath());
  for (int i = 0; i < 6; ++i) {
    if (QFileInfo::exists(dir.filePath("README.md"))) {
      return dir.absolutePath();
    }
    if (!dir.cdUp()) {
      break;
    }
  }
  return QCoreApplication::applicationDirPath();
}

int clampInt(int value, int minValue, int maxValue) {
  return std::max(minValue, std::min(maxValue, value));
}

double clampDouble(double value, double minValue, double maxValue) {
  return std::max(minValue, std::min(maxValue, value));
}
} // namespace

SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent),
      m_settings(resolveSettingsPath(), QSettings::IniFormat) {
  loadFromSettings();
}

QString SettingsManager::resolveSettingsPath() {
  const QString root = findRepoRoot();
  QDir dir(root);
  dir.mkpath("config");
  return dir.filePath("config/settings.ini");
}

QString SettingsManager::resolveFormatSettingsPath(const QString &format) {
  const QString root = findRepoRoot();
  QDir dir(root);
  dir.mkpath("config");
  const QString key = normalizeFormatKey(format);
  return dir.filePath(QString("config/%1.ini").arg(key));
}

QString SettingsManager::settingsPath() const {
  return m_settings.fileName();
}

int SettingsManager::readingFontSize() const { return m_readingFontSize; }
double SettingsManager::readingLineHeight() const { return m_readingLineHeight; }
int SettingsManager::epubFontSize() const { return m_epubFontSize; }
double SettingsManager::epubLineHeight() const { return m_epubLineHeight; }
int SettingsManager::fb2FontSize() const { return m_fb2FontSize; }
double SettingsManager::fb2LineHeight() const { return m_fb2LineHeight; }
int SettingsManager::txtFontSize() const { return m_txtFontSize; }
double SettingsManager::txtLineHeight() const { return m_txtLineHeight; }
int SettingsManager::mobiFontSize() const { return m_mobiFontSize; }
double SettingsManager::mobiLineHeight() const { return m_mobiLineHeight; }
int SettingsManager::pdfDpi() const { return m_pdfDpi; }
int SettingsManager::pdfCacheLimit() const { return m_pdfCacheLimit; }
int SettingsManager::pdfPrefetchDistance() const { return m_pdfPrefetchDistance; }
bool SettingsManager::pdfProgressiveRendering() const { return m_pdfProgressiveRendering; }
int SettingsManager::pdfProgressiveDpi() const { return m_pdfProgressiveDpi; }
double SettingsManager::comicMinZoom() const { return m_comicMinZoom; }
double SettingsManager::comicMaxZoom() const { return m_comicMaxZoom; }

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

void SettingsManager::setMobiFontSize(int value) {
  value = clampInt(value, 12, 36);
  if (m_mobiFontSize == value) {
    return;
  }
  m_mobiFontSize = value;
  saveFormatValue("mobi", "reading/font_size", value);
  emit mobiFontSizeChanged();
}

void SettingsManager::setMobiLineHeight(double value) {
  value = clampDouble(value, 1.0, 2.0);
  if (qFuzzyCompare(m_mobiLineHeight, value)) {
    return;
  }
  m_mobiLineHeight = value;
  saveFormatValue("mobi", "reading/line_height", value);
  emit mobiLineHeightChanged();
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

void SettingsManager::resetDefaults() {
  setReadingFontSize(20);
  setReadingLineHeight(1.4);
  setEpubFontSize(20);
  setEpubLineHeight(1.4);
  setFb2FontSize(20);
  setFb2LineHeight(1.4);
  setTxtFontSize(20);
  setTxtLineHeight(1.4);
  setMobiFontSize(20);
  setMobiLineHeight(1.4);
  setPdfDpi(120);
  setPdfCacheLimit(30);
  setPdfPrefetchDistance(1);
  setPdfProgressiveRendering(false);
  setPdfProgressiveDpi(72);
  setComicMinZoom(0.5);
  setComicMaxZoom(4.0);
}

void SettingsManager::reload() {
  loadFromSettings();
}

void SettingsManager::loadFromSettings() {
  m_readingFontSize = clampInt(m_settings.value("reading/font_size", 20).toInt(), 12, 36);
  m_readingLineHeight = clampDouble(m_settings.value("reading/line_height", 1.4).toDouble(), 1.0, 2.0);

  m_epubFontSize = clampInt(readFormatValue("epub", "reading/font_size", m_readingFontSize).toInt(), 12, 36);
  m_epubLineHeight =
      clampDouble(readFormatValue("epub", "reading/line_height", m_readingLineHeight).toDouble(), 1.0, 2.0);

  m_fb2FontSize = clampInt(readFormatValue("fb2", "reading/font_size", m_readingFontSize).toInt(), 12, 36);
  m_fb2LineHeight =
      clampDouble(readFormatValue("fb2", "reading/line_height", m_readingLineHeight).toDouble(), 1.0, 2.0);

  m_txtFontSize = clampInt(readFormatValue("txt", "reading/font_size", m_readingFontSize).toInt(), 12, 36);
  m_txtLineHeight =
      clampDouble(readFormatValue("txt", "reading/line_height", m_readingLineHeight).toDouble(), 1.0, 2.0);

  m_mobiFontSize = clampInt(readFormatValue("mobi", "reading/font_size", m_readingFontSize).toInt(), 12, 36);
  m_mobiLineHeight =
      clampDouble(readFormatValue("mobi", "reading/line_height", m_readingLineHeight).toDouble(), 1.0, 2.0);

  m_pdfDpi = clampInt(readFormatValue("pdf", "render/dpi", m_settings.value("pdf/dpi", 120)).toInt(), 72, 240);
  m_pdfCacheLimit =
      clampInt(readFormatValue("pdf", "render/cache_limit", m_settings.value("pdf/cache_limit", 30)).toInt(), 5, 120);
  m_pdfPrefetchDistance =
      clampInt(readFormatValue("pdf", "render/prefetch_distance", 1).toInt(), 0, 6);
  m_pdfProgressiveRendering =
      readFormatValue("pdf", "render/progressive", false).toBool();
  m_pdfProgressiveDpi =
      clampInt(readFormatValue("pdf", "render/progressive_dpi", 72).toInt(), 48, m_pdfDpi);

  m_comicMinZoom =
      clampDouble(readFormatValue("cbz", "zoom/min", m_settings.value("comics/min_zoom", 0.5)).toDouble(), 0.2, 7.0);
  m_comicMaxZoom =
      clampDouble(readFormatValue("cbz", "zoom/max", m_settings.value("comics/max_zoom", 4.0)).toDouble(),
                  m_comicMinZoom + 0.1, 8.0);

  saveFormatValue("epub", "reading/font_size", m_epubFontSize);
  saveFormatValue("epub", "reading/line_height", m_epubLineHeight);
  saveFormatValue("fb2", "reading/font_size", m_fb2FontSize);
  saveFormatValue("fb2", "reading/line_height", m_fb2LineHeight);
  saveFormatValue("txt", "reading/font_size", m_txtFontSize);
  saveFormatValue("txt", "reading/line_height", m_txtLineHeight);
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
  saveFormatValue("pdf", "render/dpi", m_pdfDpi);
  saveFormatValue("pdf", "render/cache_limit", m_pdfCacheLimit);
  saveFormatValue("pdf", "render/prefetch_distance", m_pdfPrefetchDistance);
  saveFormatValue("pdf", "render/progressive", m_pdfProgressiveRendering);
  saveFormatValue("pdf", "render/progressive_dpi", m_pdfProgressiveDpi);
  saveFormatValue("djvu", "render/dpi", m_pdfDpi);
  saveFormatValue("djvu", "render/cache_limit", m_pdfCacheLimit);
  saveComicValue("zoom/min", m_comicMinZoom);
  saveComicValue("zoom/max", m_comicMaxZoom);

  emit readingFontSizeChanged();
  emit readingLineHeightChanged();
  emit epubFontSizeChanged();
  emit epubLineHeightChanged();
  emit fb2FontSizeChanged();
  emit fb2LineHeightChanged();
  emit txtFontSizeChanged();
  emit txtLineHeightChanged();
  emit mobiFontSizeChanged();
  emit mobiLineHeightChanged();
  emit pdfDpiChanged();
  emit pdfCacheLimitChanged();
  emit pdfPrefetchDistanceChanged();
  emit pdfProgressiveRenderingChanged();
  emit pdfProgressiveDpiChanged();
  emit comicMinZoomChanged();
  emit comicMaxZoomChanged();
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
