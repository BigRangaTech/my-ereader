#pragma once

#include <QObject>
#include <QString>
#include <QUrl>
#include <memory>

#include "FormatRegistry.h"

class ReaderController : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString currentTitle READ currentTitle NOTIFY currentChanged)
  Q_PROPERTY(QString currentText READ currentText NOTIFY currentChanged)
  Q_PROPERTY(QString currentPath READ currentPath NOTIFY currentChanged)
  Q_PROPERTY(bool isOpen READ isOpen NOTIFY currentChanged)
  Q_PROPERTY(int currentChapterIndex READ currentChapterIndex NOTIFY currentChanged)
  Q_PROPERTY(QString currentChapterTitle READ currentChapterTitle NOTIFY currentChanged)
  Q_PROPERTY(bool hasImages READ hasImages NOTIFY currentChanged)
  Q_PROPERTY(int currentImageIndex READ currentImageIndex NOTIFY currentChanged)
  Q_PROPERTY(int imageCount READ imageCount NOTIFY currentChanged)
  Q_PROPERTY(QString currentImagePath READ currentImagePath NOTIFY currentChanged)
  Q_PROPERTY(QUrl currentImageUrl READ currentImageUrl NOTIFY currentChanged)
  Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
  explicit ReaderController(QObject *parent = nullptr);

  Q_INVOKABLE bool openFile(const QString &path);
  Q_INVOKABLE void close();
  Q_INVOKABLE bool jumpToLocator(const QString &locator);
  Q_INVOKABLE bool nextImage();
  Q_INVOKABLE bool prevImage();

  QString currentTitle() const;
  QString currentText() const;
  QString currentPath() const;
  bool isOpen() const;
  int currentChapterIndex() const;
  QString currentChapterTitle() const;
  bool hasImages() const;
  int currentImageIndex() const;
  int imageCount() const;
  QString currentImagePath() const;
  QUrl currentImageUrl() const;
  QString lastError() const;

signals:
  void currentChanged();
  void lastErrorChanged();

private:
  void setLastError(const QString &error);

  std::unique_ptr<FormatRegistry> m_registry;
  std::unique_ptr<FormatDocument> m_document;
  QString m_currentTitle;
  QString m_currentText;
  QString m_currentPath;
  QStringList m_chapterTitles;
  QStringList m_chapterTexts;
  int m_currentChapterIndex = -1;
  QStringList m_imagePaths;
  int m_currentImageIndex = -1;
  QString m_lastError;
  bool m_isOpen = false;
};
