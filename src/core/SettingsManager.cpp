#include "include/SettingsManager.h"

#include <algorithm>
#include <QCoreApplication>
#include <QDir>
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

QString SettingsManager::settingsPath() const {
  return m_settings.fileName();
}

int SettingsManager::readingFontSize() const { return m_readingFontSize; }
double SettingsManager::readingLineHeight() const { return m_readingLineHeight; }
int SettingsManager::pdfDpi() const { return m_pdfDpi; }
int SettingsManager::pdfCacheLimit() const { return m_pdfCacheLimit; }
double SettingsManager::comicMinZoom() const { return m_comicMinZoom; }
double SettingsManager::comicMaxZoom() const { return m_comicMaxZoom; }

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

void SettingsManager::setPdfDpi(int value) {
  value = clampInt(value, 72, 240);
  if (m_pdfDpi == value) {
    return;
  }
  m_pdfDpi = value;
  saveValue("pdf/dpi", value);
  emit pdfDpiChanged();
}

void SettingsManager::setPdfCacheLimit(int value) {
  value = clampInt(value, 5, 120);
  if (m_pdfCacheLimit == value) {
    return;
  }
  m_pdfCacheLimit = value;
  saveValue("pdf/cache_limit", value);
  emit pdfCacheLimitChanged();
}

void SettingsManager::setComicMinZoom(double value) {
  value = clampDouble(value, 0.2, m_comicMaxZoom - 0.1);
  if (qFuzzyCompare(m_comicMinZoom, value)) {
    return;
  }
  m_comicMinZoom = value;
  saveValue("comics/min_zoom", value);
  emit comicMinZoomChanged();
}

void SettingsManager::setComicMaxZoom(double value) {
  value = clampDouble(value, m_comicMinZoom + 0.1, 8.0);
  if (qFuzzyCompare(m_comicMaxZoom, value)) {
    return;
  }
  m_comicMaxZoom = value;
  saveValue("comics/max_zoom", value);
  emit comicMaxZoomChanged();
}

void SettingsManager::resetDefaults() {
  setReadingFontSize(20);
  setReadingLineHeight(1.4);
  setPdfDpi(120);
  setPdfCacheLimit(30);
  setComicMinZoom(0.5);
  setComicMaxZoom(4.0);
}

void SettingsManager::reload() {
  loadFromSettings();
}

void SettingsManager::loadFromSettings() {
  m_readingFontSize = clampInt(m_settings.value("reading/font_size", 20).toInt(), 12, 36);
  m_readingLineHeight = clampDouble(m_settings.value("reading/line_height", 1.4).toDouble(), 1.0, 2.0);
  m_pdfDpi = clampInt(m_settings.value("pdf/dpi", 120).toInt(), 72, 240);
  m_pdfCacheLimit = clampInt(m_settings.value("pdf/cache_limit", 30).toInt(), 5, 120);
  m_comicMinZoom = clampDouble(m_settings.value("comics/min_zoom", 0.5).toDouble(), 0.2, 7.0);
  m_comicMaxZoom = clampDouble(m_settings.value("comics/max_zoom", 4.0).toDouble(),
                               m_comicMinZoom + 0.1, 8.0);
  emit readingFontSizeChanged();
  emit readingLineHeightChanged();
  emit pdfDpiChanged();
  emit pdfCacheLimitChanged();
  emit comicMinZoomChanged();
  emit comicMaxZoomChanged();
}

void SettingsManager::saveValue(const QString &key, const QVariant &value) {
  m_settings.setValue(key, value);
  m_settings.sync();
}
