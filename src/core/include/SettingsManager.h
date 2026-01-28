#pragma once

#include <QObject>
#include <QSettings>

class SettingsManager : public QObject {
  Q_OBJECT
  Q_PROPERTY(int readingFontSize READ readingFontSize WRITE setReadingFontSize NOTIFY readingFontSizeChanged)
  Q_PROPERTY(double readingLineHeight READ readingLineHeight WRITE setReadingLineHeight NOTIFY readingLineHeightChanged)
  Q_PROPERTY(int pdfDpi READ pdfDpi WRITE setPdfDpi NOTIFY pdfDpiChanged)
  Q_PROPERTY(int pdfCacheLimit READ pdfCacheLimit WRITE setPdfCacheLimit NOTIFY pdfCacheLimitChanged)
  Q_PROPERTY(double comicMinZoom READ comicMinZoom WRITE setComicMinZoom NOTIFY comicMinZoomChanged)
  Q_PROPERTY(double comicMaxZoom READ comicMaxZoom WRITE setComicMaxZoom NOTIFY comicMaxZoomChanged)
  Q_PROPERTY(QString settingsPath READ settingsPath CONSTANT)

public:
  explicit SettingsManager(QObject *parent = nullptr);

  int readingFontSize() const;
  double readingLineHeight() const;
  int pdfDpi() const;
  int pdfCacheLimit() const;
  double comicMinZoom() const;
  double comicMaxZoom() const;
  QString settingsPath() const;

  void setReadingFontSize(int value);
  void setReadingLineHeight(double value);
  void setPdfDpi(int value);
  void setPdfCacheLimit(int value);
  void setComicMinZoom(double value);
  void setComicMaxZoom(double value);

  Q_INVOKABLE void resetDefaults();
  Q_INVOKABLE void reload();

signals:
  void readingFontSizeChanged();
  void readingLineHeightChanged();
  void pdfDpiChanged();
  void pdfCacheLimitChanged();
  void comicMinZoomChanged();
  void comicMaxZoomChanged();

private:
  void loadFromSettings();
  void saveValue(const QString &key, const QVariant &value);
  static QString resolveSettingsPath();

  QSettings m_settings;
  int m_readingFontSize = 20;
  double m_readingLineHeight = 1.4;
  int m_pdfDpi = 120;
  int m_pdfCacheLimit = 30;
  double m_comicMinZoom = 0.5;
  double m_comicMaxZoom = 4.0;
};
