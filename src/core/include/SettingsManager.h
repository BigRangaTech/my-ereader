#pragma once

#include <QObject>
#include <QSettings>

class SettingsManager : public QObject {
  Q_OBJECT
  Q_PROPERTY(int readingFontSize READ readingFontSize WRITE setReadingFontSize NOTIFY readingFontSizeChanged)
  Q_PROPERTY(double readingLineHeight READ readingLineHeight WRITE setReadingLineHeight NOTIFY readingLineHeightChanged)
  Q_PROPERTY(int epubFontSize READ epubFontSize WRITE setEpubFontSize NOTIFY epubFontSizeChanged)
  Q_PROPERTY(double epubLineHeight READ epubLineHeight WRITE setEpubLineHeight NOTIFY epubLineHeightChanged)
  Q_PROPERTY(int fb2FontSize READ fb2FontSize WRITE setFb2FontSize NOTIFY fb2FontSizeChanged)
  Q_PROPERTY(double fb2LineHeight READ fb2LineHeight WRITE setFb2LineHeight NOTIFY fb2LineHeightChanged)
  Q_PROPERTY(int txtFontSize READ txtFontSize WRITE setTxtFontSize NOTIFY txtFontSizeChanged)
  Q_PROPERTY(double txtLineHeight READ txtLineHeight WRITE setTxtLineHeight NOTIFY txtLineHeightChanged)
  Q_PROPERTY(bool txtMonospace READ txtMonospace WRITE setTxtMonospace NOTIFY txtMonospaceChanged)
  Q_PROPERTY(int mobiFontSize READ mobiFontSize WRITE setMobiFontSize NOTIFY mobiFontSizeChanged)
  Q_PROPERTY(double mobiLineHeight READ mobiLineHeight WRITE setMobiLineHeight NOTIFY mobiLineHeightChanged)
  Q_PROPERTY(int pdfDpi READ pdfDpi WRITE setPdfDpi NOTIFY pdfDpiChanged)
  Q_PROPERTY(int pdfCacheLimit READ pdfCacheLimit WRITE setPdfCacheLimit NOTIFY pdfCacheLimitChanged)
  Q_PROPERTY(int pdfPrefetchDistance READ pdfPrefetchDistance WRITE setPdfPrefetchDistance NOTIFY pdfPrefetchDistanceChanged)
  Q_PROPERTY(QString pdfPrefetchStrategy READ pdfPrefetchStrategy WRITE setPdfPrefetchStrategy NOTIFY pdfPrefetchStrategyChanged)
  Q_PROPERTY(QString pdfCachePolicy READ pdfCachePolicy WRITE setPdfCachePolicy NOTIFY pdfCachePolicyChanged)
  Q_PROPERTY(QString pdfRenderPreset READ pdfRenderPreset WRITE setPdfRenderPreset NOTIFY pdfRenderPresetChanged)
  Q_PROPERTY(QString pdfColorMode READ pdfColorMode WRITE setPdfColorMode NOTIFY pdfColorModeChanged)
  Q_PROPERTY(QString pdfBackgroundMode READ pdfBackgroundMode WRITE setPdfBackgroundMode NOTIFY pdfBackgroundModeChanged)
  Q_PROPERTY(QString pdfBackgroundColor READ pdfBackgroundColor WRITE setPdfBackgroundColor NOTIFY pdfBackgroundColorChanged)
  Q_PROPERTY(int pdfMaxWidth READ pdfMaxWidth WRITE setPdfMaxWidth NOTIFY pdfMaxWidthChanged)
  Q_PROPERTY(int pdfMaxHeight READ pdfMaxHeight WRITE setPdfMaxHeight NOTIFY pdfMaxHeightChanged)
  Q_PROPERTY(QString pdfImageFormat READ pdfImageFormat WRITE setPdfImageFormat NOTIFY pdfImageFormatChanged)
  Q_PROPERTY(int pdfJpegQuality READ pdfJpegQuality WRITE setPdfJpegQuality NOTIFY pdfJpegQualityChanged)
  Q_PROPERTY(bool pdfExtractText READ pdfExtractText WRITE setPdfExtractText NOTIFY pdfExtractTextChanged)
  Q_PROPERTY(int pdfTileSize READ pdfTileSize WRITE setPdfTileSize NOTIFY pdfTileSizeChanged)
  Q_PROPERTY(bool pdfProgressiveRendering READ pdfProgressiveRendering WRITE setPdfProgressiveRendering NOTIFY pdfProgressiveRenderingChanged)
  Q_PROPERTY(int pdfProgressiveDpi READ pdfProgressiveDpi WRITE setPdfProgressiveDpi NOTIFY pdfProgressiveDpiChanged)
  Q_PROPERTY(int djvuDpi READ djvuDpi WRITE setDjvuDpi NOTIFY djvuDpiChanged)
  Q_PROPERTY(int djvuCacheLimit READ djvuCacheLimit WRITE setDjvuCacheLimit NOTIFY djvuCacheLimitChanged)
  Q_PROPERTY(int djvuPrefetchDistance READ djvuPrefetchDistance WRITE setDjvuPrefetchDistance NOTIFY djvuPrefetchDistanceChanged)
  Q_PROPERTY(QString djvuCachePolicy READ djvuCachePolicy WRITE setDjvuCachePolicy NOTIFY djvuCachePolicyChanged)
  Q_PROPERTY(QString djvuImageFormat READ djvuImageFormat WRITE setDjvuImageFormat NOTIFY djvuImageFormatChanged)
  Q_PROPERTY(bool djvuExtractText READ djvuExtractText WRITE setDjvuExtractText NOTIFY djvuExtractTextChanged)
  Q_PROPERTY(int djvuRotation READ djvuRotation WRITE setDjvuRotation NOTIFY djvuRotationChanged)
  Q_PROPERTY(double comicMinZoom READ comicMinZoom WRITE setComicMinZoom NOTIFY comicMinZoomChanged)
  Q_PROPERTY(double comicMaxZoom READ comicMaxZoom WRITE setComicMaxZoom NOTIFY comicMaxZoomChanged)
  Q_PROPERTY(QString settingsPath READ settingsPath CONSTANT)
  Q_PROPERTY(QString iconPath READ iconPath CONSTANT)

public:
  explicit SettingsManager(QObject *parent = nullptr);

  int readingFontSize() const;
  double readingLineHeight() const;
  int epubFontSize() const;
  double epubLineHeight() const;
  int fb2FontSize() const;
  double fb2LineHeight() const;
  int txtFontSize() const;
  double txtLineHeight() const;
  bool txtMonospace() const;
  int mobiFontSize() const;
  double mobiLineHeight() const;
  int pdfDpi() const;
  int pdfCacheLimit() const;
  int pdfPrefetchDistance() const;
  QString pdfPrefetchStrategy() const;
  QString pdfCachePolicy() const;
  QString pdfRenderPreset() const;
  QString pdfColorMode() const;
  QString pdfBackgroundMode() const;
  QString pdfBackgroundColor() const;
  int pdfMaxWidth() const;
  int pdfMaxHeight() const;
  QString pdfImageFormat() const;
  int pdfJpegQuality() const;
  bool pdfExtractText() const;
  int pdfTileSize() const;
  bool pdfProgressiveRendering() const;
  int pdfProgressiveDpi() const;
  int djvuDpi() const;
  int djvuCacheLimit() const;
  int djvuPrefetchDistance() const;
  QString djvuCachePolicy() const;
  QString djvuImageFormat() const;
  bool djvuExtractText() const;
  int djvuRotation() const;
  double comicMinZoom() const;
  double comicMaxZoom() const;
  QString settingsPath() const;
  QString iconPath() const;
  Q_INVOKABLE QString formatSettingsPath(const QString &format) const;

  void setReadingFontSize(int value);
  void setReadingLineHeight(double value);
  void setEpubFontSize(int value);
  void setEpubLineHeight(double value);
  void setFb2FontSize(int value);
  void setFb2LineHeight(double value);
  void setTxtFontSize(int value);
  void setTxtLineHeight(double value);
  void setTxtMonospace(bool value);
  void setMobiFontSize(int value);
  void setMobiLineHeight(double value);
  void setPdfDpi(int value);
  void setPdfCacheLimit(int value);
  void setPdfPrefetchDistance(int value);
  void setPdfPrefetchStrategy(const QString &value);
  void setPdfCachePolicy(const QString &value);
  void setPdfRenderPreset(const QString &value);
  void setPdfColorMode(const QString &value);
  void setPdfBackgroundMode(const QString &value);
  void setPdfBackgroundColor(const QString &value);
  void setPdfMaxWidth(int value);
  void setPdfMaxHeight(int value);
  void setPdfImageFormat(const QString &value);
  void setPdfJpegQuality(int value);
  void setPdfExtractText(bool value);
  void setPdfTileSize(int value);
  void setPdfProgressiveRendering(bool value);
  void setPdfProgressiveDpi(int value);
  void setDjvuDpi(int value);
  void setDjvuCacheLimit(int value);
  void setDjvuPrefetchDistance(int value);
  void setDjvuCachePolicy(const QString &value);
  void setDjvuImageFormat(const QString &value);
  void setDjvuExtractText(bool value);
  void setDjvuRotation(int value);
  void setComicMinZoom(double value);
  void setComicMaxZoom(double value);

  Q_INVOKABLE void resetDefaults();
  Q_INVOKABLE void resetPdfDefaults();
  Q_INVOKABLE void resetEpubDefaults();
  Q_INVOKABLE void resetFb2Defaults();
  Q_INVOKABLE void resetTxtDefaults();
  Q_INVOKABLE void resetMobiDefaults();
  Q_INVOKABLE void resetComicDefaults();
  Q_INVOKABLE void resetDjvuDefaults();
  Q_INVOKABLE void reload();
  Q_INVOKABLE QString sidebarModeForPath(const QString &path) const;
  Q_INVOKABLE void setSidebarModeForPath(const QString &path, const QString &mode);

signals:
  void readingFontSizeChanged();
  void readingLineHeightChanged();
  void epubFontSizeChanged();
  void epubLineHeightChanged();
  void fb2FontSizeChanged();
  void fb2LineHeightChanged();
  void txtFontSizeChanged();
  void txtLineHeightChanged();
  void txtMonospaceChanged();
  void mobiFontSizeChanged();
  void mobiLineHeightChanged();
  void pdfDpiChanged();
  void pdfCacheLimitChanged();
  void pdfPrefetchDistanceChanged();
  void pdfPrefetchStrategyChanged();
  void pdfCachePolicyChanged();
  void pdfRenderPresetChanged();
  void pdfColorModeChanged();
  void pdfBackgroundModeChanged();
  void pdfBackgroundColorChanged();
  void pdfMaxWidthChanged();
  void pdfMaxHeightChanged();
  void pdfImageFormatChanged();
  void pdfJpegQualityChanged();
  void pdfExtractTextChanged();
  void pdfTileSizeChanged();
  void pdfProgressiveRenderingChanged();
  void pdfProgressiveDpiChanged();
  void djvuDpiChanged();
  void djvuCacheLimitChanged();
  void djvuPrefetchDistanceChanged();
  void djvuCachePolicyChanged();
  void djvuImageFormatChanged();
  void djvuExtractTextChanged();
  void djvuRotationChanged();
  void comicMinZoomChanged();
  void comicMaxZoomChanged();

private:
  void loadFromSettings();
  void saveValue(const QString &key, const QVariant &value);
  static QString resolveSettingsPath();
  static QString resolveFormatSettingsPath(const QString &format);
  QVariant readFormatValue(const QString &format, const QString &key, const QVariant &fallback) const;
  void saveFormatValue(const QString &format, const QString &key, const QVariant &value) const;
  void saveComicValue(const QString &key, const QVariant &value) const;
  static QString normalizeFormatKey(const QString &format);

  QSettings m_settings;
  int m_readingFontSize = 20;
  double m_readingLineHeight = 1.4;
  int m_epubFontSize = 20;
  double m_epubLineHeight = 1.4;
  int m_fb2FontSize = 20;
  double m_fb2LineHeight = 1.4;
  int m_txtFontSize = 20;
  double m_txtLineHeight = 1.4;
  bool m_txtMonospace = false;
  int m_mobiFontSize = 20;
  double m_mobiLineHeight = 1.4;
  int m_pdfDpi = 120;
  int m_pdfCacheLimit = 30;
  int m_pdfPrefetchDistance = 1;
  QString m_pdfPrefetchStrategy = "symmetric";
  QString m_pdfCachePolicy = "fifo";
  QString m_pdfRenderPreset = "custom";
  QString m_pdfColorMode = "color";
  QString m_pdfBackgroundMode = "white";
  QString m_pdfBackgroundColor = "#202633";
  int m_pdfMaxWidth = 0;
  int m_pdfMaxHeight = 0;
  QString m_pdfImageFormat = "png";
  int m_pdfJpegQuality = 85;
  bool m_pdfExtractText = true;
  int m_pdfTileSize = 0;
  bool m_pdfProgressiveRendering = false;
  int m_pdfProgressiveDpi = 72;
  int m_djvuDpi = 120;
  int m_djvuCacheLimit = 30;
  int m_djvuPrefetchDistance = 1;
  QString m_djvuCachePolicy = "fifo";
  QString m_djvuImageFormat = "ppm";
  bool m_djvuExtractText = true;
  int m_djvuRotation = 0;
  double m_comicMinZoom = 0.5;
  double m_comicMaxZoom = 4.0;
};
