#pragma once

#include <QObject>
#include <QString>
#include <QUrl>
#include <QVector>
#include <memory>

#include "FormatRegistry.h"

class ReaderController : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString currentTitle READ currentTitle NOTIFY currentChanged)
  Q_PROPERTY(QString currentText READ currentText NOTIFY currentChanged)
  Q_PROPERTY(QString currentPlainText READ currentPlainText NOTIFY currentChanged)
  Q_PROPERTY(bool currentTextIsRich READ currentTextIsRich NOTIFY currentChanged)
  Q_PROPERTY(QString currentPath READ currentPath NOTIFY currentChanged)
  Q_PROPERTY(QString currentFormat READ currentFormat NOTIFY currentChanged)
  Q_PROPERTY(bool isOpen READ isOpen NOTIFY currentChanged)
  Q_PROPERTY(int currentChapterIndex READ currentChapterIndex NOTIFY currentChanged)
  Q_PROPERTY(QString currentChapterTitle READ currentChapterTitle NOTIFY currentChanged)
  Q_PROPERTY(int chapterCount READ chapterCount NOTIFY currentChanged)
  Q_PROPERTY(int tocCount READ tocCount NOTIFY currentChanged)
  Q_PROPERTY(bool hasImages READ hasImages NOTIFY currentChanged)
  Q_PROPERTY(int currentImageIndex READ currentImageIndex NOTIFY currentChanged)
  Q_PROPERTY(int imageCount READ imageCount NOTIFY currentChanged)
  Q_PROPERTY(QString currentImagePath READ currentImagePath NOTIFY currentChanged)
  Q_PROPERTY(QUrl currentImageUrl READ currentImageUrl NOTIFY currentChanged)
  Q_PROPERTY(int imageReloadToken READ imageReloadToken NOTIFY imageReloadTokenChanged)
  Q_PROPERTY(QString currentCoverPath READ currentCoverPath NOTIFY currentChanged)
  Q_PROPERTY(QUrl currentCoverUrl READ currentCoverUrl NOTIFY currentChanged)
  Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
  Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
  Q_PROPERTY(bool ttsAllowed READ ttsAllowed NOTIFY currentChanged)

public:
  explicit ReaderController(QObject *parent = nullptr);

  Q_INVOKABLE bool openFile(const QString &path);
  Q_INVOKABLE void openFileAsync(const QString &path);
  Q_INVOKABLE void close();
  Q_INVOKABLE bool jumpToLocator(const QString &locator);
  Q_INVOKABLE bool nextChapter();
  Q_INVOKABLE bool prevChapter();
  Q_INVOKABLE bool goToChapter(int index);
  Q_INVOKABLE QString chapterTitle(int index) const;
  Q_INVOKABLE QString tocTitle(int index) const;
  Q_INVOKABLE int tocChapterIndex(int index) const;
  Q_INVOKABLE bool nextImage();
  Q_INVOKABLE bool prevImage();
  Q_INVOKABLE bool goToImage(int index);

  QString currentTitle() const;
  QString currentText() const;
  QString currentPlainText() const;
  bool currentTextIsRich() const;
  QString currentPath() const;
  QString currentFormat() const;
  bool isOpen() const;
  int currentChapterIndex() const;
  QString currentChapterTitle() const;
  int chapterCount() const;
  int tocCount() const;
  bool hasImages() const;
  int currentImageIndex() const;
  int imageCount() const;
  QString currentImagePath() const;
  QUrl currentImageUrl() const;
  int imageReloadToken() const;
  QString currentCoverPath() const;
  QUrl currentCoverUrl() const;
  bool busy() const;
  QString lastError() const;
  bool ttsAllowed() const;

signals:
  void currentChanged();
  void imageReloadTokenChanged();
  void busyChanged();
  void lastErrorChanged();

private:
  void setLastError(const QString &error);
  bool applyDocument(std::unique_ptr<FormatDocument> document, const QString &path, QString *error);
  void setBusy(bool busy);
  void clearImageState();

  std::unique_ptr<FormatRegistry> m_registry;
  std::unique_ptr<FormatDocument> m_document;
  QString m_currentTitle;
  QString m_currentText;
  QString m_currentPlainText;
  QString m_currentPath;
  QString m_currentFormat;
  QStringList m_chapterTitles;
  QStringList m_chapterTexts;
  QStringList m_chapterPlainTexts;
  QStringList m_tocTitles;
  QVector<int> m_tocChapterIndices;
  bool m_textIsRich = false;
  int m_currentChapterIndex = -1;
  QStringList m_imagePaths;
  int m_currentImageIndex = -1;
  int m_imageReloadToken = 0;
  QString m_coverPath;
  QString m_lastError;
  bool m_ttsAllowed = true;
  bool m_isOpen = false;
  bool m_busy = false;
  int m_openRequestId = 0;
};
