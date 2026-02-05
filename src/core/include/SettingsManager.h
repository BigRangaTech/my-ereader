#pragma once

#include <QObject>
#include <QSettings>

class SettingsManager : public QObject {
  Q_OBJECT
  Q_PROPERTY(int readingFontSize READ readingFontSize WRITE setReadingFontSize NOTIFY readingFontSizeChanged)
  Q_PROPERTY(double readingLineHeight READ readingLineHeight WRITE setReadingLineHeight NOTIFY readingLineHeightChanged)
  Q_PROPERTY(double ttsRate READ ttsRate WRITE setTtsRate NOTIFY ttsRateChanged)
  Q_PROPERTY(double ttsPitch READ ttsPitch WRITE setTtsPitch NOTIFY ttsPitchChanged)
  Q_PROPERTY(double ttsVolume READ ttsVolume WRITE setTtsVolume NOTIFY ttsVolumeChanged)
  Q_PROPERTY(QString ttsVoiceKey READ ttsVoiceKey WRITE setTtsVoiceKey NOTIFY ttsVoiceKeyChanged)
  Q_PROPERTY(bool autoLockEnabled READ autoLockEnabled WRITE setAutoLockEnabled NOTIFY autoLockEnabledChanged)
  Q_PROPERTY(int autoLockMinutes READ autoLockMinutes WRITE setAutoLockMinutes NOTIFY autoLockMinutesChanged)
  Q_PROPERTY(bool rememberPassphrase READ rememberPassphrase WRITE setRememberPassphrase NOTIFY rememberPassphraseChanged)
  Q_PROPERTY(int epubFontSize READ epubFontSize WRITE setEpubFontSize NOTIFY epubFontSizeChanged)
  Q_PROPERTY(double epubLineHeight READ epubLineHeight WRITE setEpubLineHeight NOTIFY epubLineHeightChanged)
  Q_PROPERTY(bool epubShowImages READ epubShowImages WRITE setEpubShowImages NOTIFY epubShowImagesChanged)
  Q_PROPERTY(QString epubTextAlign READ epubTextAlign WRITE setEpubTextAlign NOTIFY epubTextAlignChanged)
  Q_PROPERTY(double epubParagraphSpacing READ epubParagraphSpacing WRITE setEpubParagraphSpacing NOTIFY epubParagraphSpacingChanged)
  Q_PROPERTY(double epubParagraphIndent READ epubParagraphIndent WRITE setEpubParagraphIndent NOTIFY epubParagraphIndentChanged)
  Q_PROPERTY(int epubImageMaxWidth READ epubImageMaxWidth WRITE setEpubImageMaxWidth NOTIFY epubImageMaxWidthChanged)
  Q_PROPERTY(double epubImageSpacing READ epubImageSpacing WRITE setEpubImageSpacing NOTIFY epubImageSpacingChanged)
  Q_PROPERTY(int fb2FontSize READ fb2FontSize WRITE setFb2FontSize NOTIFY fb2FontSizeChanged)
  Q_PROPERTY(double fb2LineHeight READ fb2LineHeight WRITE setFb2LineHeight NOTIFY fb2LineHeightChanged)
  Q_PROPERTY(bool fb2ShowImages READ fb2ShowImages WRITE setFb2ShowImages NOTIFY fb2ShowImagesChanged)
  Q_PROPERTY(QString fb2TextAlign READ fb2TextAlign WRITE setFb2TextAlign NOTIFY fb2TextAlignChanged)
  Q_PROPERTY(double fb2ParagraphSpacing READ fb2ParagraphSpacing WRITE setFb2ParagraphSpacing NOTIFY fb2ParagraphSpacingChanged)
  Q_PROPERTY(double fb2ParagraphIndent READ fb2ParagraphIndent WRITE setFb2ParagraphIndent NOTIFY fb2ParagraphIndentChanged)
  Q_PROPERTY(int fb2ImageMaxWidth READ fb2ImageMaxWidth WRITE setFb2ImageMaxWidth NOTIFY fb2ImageMaxWidthChanged)
  Q_PROPERTY(double fb2ImageSpacing READ fb2ImageSpacing WRITE setFb2ImageSpacing NOTIFY fb2ImageSpacingChanged)
  Q_PROPERTY(int txtFontSize READ txtFontSize WRITE setTxtFontSize NOTIFY txtFontSizeChanged)
  Q_PROPERTY(double txtLineHeight READ txtLineHeight WRITE setTxtLineHeight NOTIFY txtLineHeightChanged)
  Q_PROPERTY(bool txtMonospace READ txtMonospace WRITE setTxtMonospace NOTIFY txtMonospaceChanged)
  Q_PROPERTY(QString txtEncoding READ txtEncoding WRITE setTxtEncoding NOTIFY txtEncodingChanged)
  Q_PROPERTY(int txtTabWidth READ txtTabWidth WRITE setTxtTabWidth NOTIFY txtTabWidthChanged)
  Q_PROPERTY(bool txtTrimWhitespace READ txtTrimWhitespace WRITE setTxtTrimWhitespace NOTIFY txtTrimWhitespaceChanged)
  Q_PROPERTY(bool txtAutoChapters READ txtAutoChapters WRITE setTxtAutoChapters NOTIFY txtAutoChaptersChanged)
  Q_PROPERTY(int mobiFontSize READ mobiFontSize WRITE setMobiFontSize NOTIFY mobiFontSizeChanged)
  Q_PROPERTY(double mobiLineHeight READ mobiLineHeight WRITE setMobiLineHeight NOTIFY mobiLineHeightChanged)
  Q_PROPERTY(bool mobiShowImages READ mobiShowImages WRITE setMobiShowImages NOTIFY mobiShowImagesChanged)
  Q_PROPERTY(QString mobiTextAlign READ mobiTextAlign WRITE setMobiTextAlign NOTIFY mobiTextAlignChanged)
  Q_PROPERTY(double mobiParagraphSpacing READ mobiParagraphSpacing WRITE setMobiParagraphSpacing NOTIFY mobiParagraphSpacingChanged)
  Q_PROPERTY(double mobiParagraphIndent READ mobiParagraphIndent WRITE setMobiParagraphIndent NOTIFY mobiParagraphIndentChanged)
  Q_PROPERTY(int mobiImageMaxWidth READ mobiImageMaxWidth WRITE setMobiImageMaxWidth NOTIFY mobiImageMaxWidthChanged)
  Q_PROPERTY(double mobiImageSpacing READ mobiImageSpacing WRITE setMobiImageSpacing NOTIFY mobiImageSpacingChanged)
  Q_PROPERTY(int pdfDpi READ pdfDpi WRITE setPdfDpi NOTIFY pdfDpiChanged)
  Q_PROPERTY(int pdfCacheLimit READ pdfCacheLimit WRITE setPdfCacheLimit NOTIFY pdfCacheLimitChanged)
  Q_PROPERTY(int pdfPrefetchDistance READ pdfPrefetchDistance WRITE setPdfPrefetchDistance NOTIFY pdfPrefetchDistanceChanged)
  Q_PROPERTY(int pdfPreRenderPages READ pdfPreRenderPages WRITE setPdfPreRenderPages NOTIFY pdfPreRenderPagesChanged)
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
  Q_PROPERTY(int djvuPreRenderPages READ djvuPreRenderPages WRITE setDjvuPreRenderPages NOTIFY djvuPreRenderPagesChanged)
  Q_PROPERTY(QString djvuCachePolicy READ djvuCachePolicy WRITE setDjvuCachePolicy NOTIFY djvuCachePolicyChanged)
  Q_PROPERTY(QString djvuImageFormat READ djvuImageFormat WRITE setDjvuImageFormat NOTIFY djvuImageFormatChanged)
  Q_PROPERTY(bool djvuExtractText READ djvuExtractText WRITE setDjvuExtractText NOTIFY djvuExtractTextChanged)
  Q_PROPERTY(int djvuRotation READ djvuRotation WRITE setDjvuRotation NOTIFY djvuRotationChanged)
  Q_PROPERTY(double comicMinZoom READ comicMinZoom WRITE setComicMinZoom NOTIFY comicMinZoomChanged)
  Q_PROPERTY(double comicMaxZoom READ comicMaxZoom WRITE setComicMaxZoom NOTIFY comicMaxZoomChanged)
  Q_PROPERTY(QString comicSortMode READ comicSortMode WRITE setComicSortMode NOTIFY comicSortModeChanged)
  Q_PROPERTY(bool comicSortDescending READ comicSortDescending WRITE setComicSortDescending NOTIFY comicSortDescendingChanged)
  Q_PROPERTY(QString settingsPath READ settingsPath CONSTANT)
  Q_PROPERTY(QString iconPath READ iconPath CONSTANT)

public:
  explicit SettingsManager(QObject *parent = nullptr);

  int readingFontSize() const;
  double readingLineHeight() const;
  double ttsRate() const;
  double ttsPitch() const;
  double ttsVolume() const;
  QString ttsVoiceKey() const;
  bool autoLockEnabled() const;
  int autoLockMinutes() const;
  bool rememberPassphrase() const;
  int epubFontSize() const;
  double epubLineHeight() const;
  bool epubShowImages() const;
  QString epubTextAlign() const;
  double epubParagraphSpacing() const;
  double epubParagraphIndent() const;
  int epubImageMaxWidth() const;
  double epubImageSpacing() const;
  int fb2FontSize() const;
  double fb2LineHeight() const;
  bool fb2ShowImages() const;
  QString fb2TextAlign() const;
  double fb2ParagraphSpacing() const;
  double fb2ParagraphIndent() const;
  int fb2ImageMaxWidth() const;
  double fb2ImageSpacing() const;
  int txtFontSize() const;
  double txtLineHeight() const;
  bool txtMonospace() const;
  QString txtEncoding() const;
  int txtTabWidth() const;
  bool txtTrimWhitespace() const;
  bool txtAutoChapters() const;
  int mobiFontSize() const;
  double mobiLineHeight() const;
  bool mobiShowImages() const;
  QString mobiTextAlign() const;
  double mobiParagraphSpacing() const;
  double mobiParagraphIndent() const;
  int mobiImageMaxWidth() const;
  double mobiImageSpacing() const;
  int pdfDpi() const;
  int pdfCacheLimit() const;
  int pdfPrefetchDistance() const;
  int pdfPreRenderPages() const;
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
  int djvuPreRenderPages() const;
  QString djvuCachePolicy() const;
  QString djvuImageFormat() const;
  bool djvuExtractText() const;
  int djvuRotation() const;
  double comicMinZoom() const;
  double comicMaxZoom() const;
  QString comicSortMode() const;
  bool comicSortDescending() const;
  QString settingsPath() const;
  QString iconPath() const;
  Q_INVOKABLE QString formatSettingsPath(const QString &format) const;

  void setReadingFontSize(int value);
  void setReadingLineHeight(double value);
  void setTtsRate(double value);
  void setTtsPitch(double value);
  void setTtsVolume(double value);
  void setTtsVoiceKey(const QString &value);
  void setAutoLockEnabled(bool value);
  void setAutoLockMinutes(int value);
  void setRememberPassphrase(bool value);
  void setEpubFontSize(int value);
  void setEpubLineHeight(double value);
  void setEpubShowImages(bool value);
  void setEpubTextAlign(const QString &value);
  void setEpubParagraphSpacing(double value);
  void setEpubParagraphIndent(double value);
  void setEpubImageMaxWidth(int value);
  void setEpubImageSpacing(double value);
  void setFb2FontSize(int value);
  void setFb2LineHeight(double value);
  void setFb2ShowImages(bool value);
  void setFb2TextAlign(const QString &value);
  void setFb2ParagraphSpacing(double value);
  void setFb2ParagraphIndent(double value);
  void setFb2ImageMaxWidth(int value);
  void setFb2ImageSpacing(double value);
  void setTxtFontSize(int value);
  void setTxtLineHeight(double value);
  void setTxtMonospace(bool value);
  void setTxtEncoding(const QString &value);
  void setTxtTabWidth(int value);
  void setTxtTrimWhitespace(bool value);
  void setTxtAutoChapters(bool value);
  void setMobiFontSize(int value);
  void setMobiLineHeight(double value);
  void setMobiShowImages(bool value);
  void setMobiTextAlign(const QString &value);
  void setMobiParagraphSpacing(double value);
  void setMobiParagraphIndent(double value);
  void setMobiImageMaxWidth(int value);
  void setMobiImageSpacing(double value);
  void setPdfDpi(int value);
  void setPdfCacheLimit(int value);
  void setPdfPrefetchDistance(int value);
  void setPdfPreRenderPages(int value);
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
  void setDjvuPreRenderPages(int value);
  void setDjvuCachePolicy(const QString &value);
  void setDjvuImageFormat(const QString &value);
  void setDjvuExtractText(bool value);
  void setDjvuRotation(int value);
  void setComicMinZoom(double value);
  void setComicMaxZoom(double value);
  void setComicSortMode(const QString &value);
  void setComicSortDescending(bool value);

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
  void ttsRateChanged();
  void ttsPitchChanged();
  void ttsVolumeChanged();
  void ttsVoiceKeyChanged();
  void autoLockEnabledChanged();
  void autoLockMinutesChanged();
  void rememberPassphraseChanged();
  void epubFontSizeChanged();
  void epubLineHeightChanged();
  void epubShowImagesChanged();
  void epubTextAlignChanged();
  void epubParagraphSpacingChanged();
  void epubParagraphIndentChanged();
  void epubImageMaxWidthChanged();
  void epubImageSpacingChanged();
  void fb2FontSizeChanged();
  void fb2LineHeightChanged();
  void fb2ShowImagesChanged();
  void fb2TextAlignChanged();
  void fb2ParagraphSpacingChanged();
  void fb2ParagraphIndentChanged();
  void fb2ImageMaxWidthChanged();
  void fb2ImageSpacingChanged();
  void txtFontSizeChanged();
  void txtLineHeightChanged();
  void txtMonospaceChanged();
  void txtEncodingChanged();
  void txtTabWidthChanged();
  void txtTrimWhitespaceChanged();
  void txtAutoChaptersChanged();
  void mobiFontSizeChanged();
  void mobiLineHeightChanged();
  void mobiShowImagesChanged();
  void mobiTextAlignChanged();
  void mobiParagraphSpacingChanged();
  void mobiParagraphIndentChanged();
  void mobiImageMaxWidthChanged();
  void mobiImageSpacingChanged();
  void pdfDpiChanged();
  void pdfCacheLimitChanged();
  void pdfPrefetchDistanceChanged();
  void pdfPreRenderPagesChanged();
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
  void djvuPreRenderPagesChanged();
  void djvuCachePolicyChanged();
  void djvuImageFormatChanged();
  void djvuExtractTextChanged();
  void djvuRotationChanged();
  void comicMinZoomChanged();
  void comicMaxZoomChanged();
  void comicSortModeChanged();
  void comicSortDescendingChanged();

private:
  void loadFromSettings();
  void saveValue(const QString &key, const QVariant &value);
  static QString resolveSettingsPath();
  static QString resolveFormatSettingsPath(const QString &format);
  QVariant readFormatValue(const QString &format, const QString &key, const QVariant &fallback) const;
  void saveFormatValue(const QString &format, const QString &key, const QVariant &value) const;
  void saveComicValue(const QString &key, const QVariant &value) const;
  void saveMobiFamilyValue(const QString &key, const QVariant &value) const;
  static QString normalizeFormatKey(const QString &format);

  QSettings m_settings;
  int m_readingFontSize = 20;
  double m_readingLineHeight = 1.4;
  double m_ttsRate = 0.0;
  double m_ttsPitch = 0.0;
  double m_ttsVolume = 1.0;
  QString m_ttsVoiceKey;
  bool m_autoLockEnabled = true;
  int m_autoLockMinutes = 10;
  bool m_rememberPassphrase = true;
  int m_epubFontSize = 20;
  double m_epubLineHeight = 1.4;
  bool m_epubShowImages = true;
  QString m_epubTextAlign = "left";
  double m_epubParagraphSpacing = 0.6;
  double m_epubParagraphIndent = 0.0;
  int m_epubImageMaxWidth = 100;
  double m_epubImageSpacing = 0.6;
  int m_fb2FontSize = 20;
  double m_fb2LineHeight = 1.4;
  bool m_fb2ShowImages = true;
  QString m_fb2TextAlign = "left";
  double m_fb2ParagraphSpacing = 0.6;
  double m_fb2ParagraphIndent = 0.0;
  int m_fb2ImageMaxWidth = 100;
  double m_fb2ImageSpacing = 0.6;
  int m_txtFontSize = 20;
  double m_txtLineHeight = 1.4;
  bool m_txtMonospace = false;
  QString m_txtEncoding = "auto";
  int m_txtTabWidth = 4;
  bool m_txtTrimWhitespace = false;
  bool m_txtAutoChapters = true;
  int m_mobiFontSize = 20;
  double m_mobiLineHeight = 1.4;
  bool m_mobiShowImages = true;
  QString m_mobiTextAlign = "left";
  double m_mobiParagraphSpacing = 0.6;
  double m_mobiParagraphIndent = 0.0;
  int m_mobiImageMaxWidth = 100;
  double m_mobiImageSpacing = 0.6;
  int m_pdfDpi = 120;
  int m_pdfCacheLimit = 30;
  int m_pdfPrefetchDistance = 1;
  int m_pdfPreRenderPages = 2;
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
  int m_djvuPreRenderPages = 2;
  QString m_djvuCachePolicy = "fifo";
  QString m_djvuImageFormat = "ppm";
  bool m_djvuExtractText = true;
  int m_djvuRotation = 0;
  double m_comicMinZoom = 0.5;
  double m_comicMaxZoom = 4.0;
  QString m_comicSortMode = "path";
  bool m_comicSortDescending = false;
};
