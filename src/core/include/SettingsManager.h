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
  Q_PROPERTY(int mobiFontSize READ mobiFontSize WRITE setMobiFontSize NOTIFY mobiFontSizeChanged)
  Q_PROPERTY(double mobiLineHeight READ mobiLineHeight WRITE setMobiLineHeight NOTIFY mobiLineHeightChanged)
  Q_PROPERTY(int pdfDpi READ pdfDpi WRITE setPdfDpi NOTIFY pdfDpiChanged)
  Q_PROPERTY(int pdfCacheLimit READ pdfCacheLimit WRITE setPdfCacheLimit NOTIFY pdfCacheLimitChanged)
  Q_PROPERTY(int pdfPrefetchDistance READ pdfPrefetchDistance WRITE setPdfPrefetchDistance NOTIFY pdfPrefetchDistanceChanged)
  Q_PROPERTY(bool pdfProgressiveRendering READ pdfProgressiveRendering WRITE setPdfProgressiveRendering NOTIFY pdfProgressiveRenderingChanged)
  Q_PROPERTY(int pdfProgressiveDpi READ pdfProgressiveDpi WRITE setPdfProgressiveDpi NOTIFY pdfProgressiveDpiChanged)
  Q_PROPERTY(double comicMinZoom READ comicMinZoom WRITE setComicMinZoom NOTIFY comicMinZoomChanged)
  Q_PROPERTY(double comicMaxZoom READ comicMaxZoom WRITE setComicMaxZoom NOTIFY comicMaxZoomChanged)
  Q_PROPERTY(QString settingsPath READ settingsPath CONSTANT)

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
  int mobiFontSize() const;
  double mobiLineHeight() const;
  int pdfDpi() const;
  int pdfCacheLimit() const;
  int pdfPrefetchDistance() const;
  bool pdfProgressiveRendering() const;
  int pdfProgressiveDpi() const;
  double comicMinZoom() const;
  double comicMaxZoom() const;
  QString settingsPath() const;
  Q_INVOKABLE QString formatSettingsPath(const QString &format) const;

  void setReadingFontSize(int value);
  void setReadingLineHeight(double value);
  void setEpubFontSize(int value);
  void setEpubLineHeight(double value);
  void setFb2FontSize(int value);
  void setFb2LineHeight(double value);
  void setTxtFontSize(int value);
  void setTxtLineHeight(double value);
  void setMobiFontSize(int value);
  void setMobiLineHeight(double value);
  void setPdfDpi(int value);
  void setPdfCacheLimit(int value);
  void setPdfPrefetchDistance(int value);
  void setPdfProgressiveRendering(bool value);
  void setPdfProgressiveDpi(int value);
  void setComicMinZoom(double value);
  void setComicMaxZoom(double value);

  Q_INVOKABLE void resetDefaults();
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
  void mobiFontSizeChanged();
  void mobiLineHeightChanged();
  void pdfDpiChanged();
  void pdfCacheLimitChanged();
  void pdfPrefetchDistanceChanged();
  void pdfProgressiveRenderingChanged();
  void pdfProgressiveDpiChanged();
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
  int m_mobiFontSize = 20;
  double m_mobiLineHeight = 1.4;
  int m_pdfDpi = 120;
  int m_pdfCacheLimit = 30;
  int m_pdfPrefetchDistance = 1;
  bool m_pdfProgressiveRendering = false;
  int m_pdfProgressiveDpi = 72;
  double m_comicMinZoom = 0.5;
  double m_comicMaxZoom = 4.0;
};
