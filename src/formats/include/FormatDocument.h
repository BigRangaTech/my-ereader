#pragma once

#include <QString>
#include <QStringList>
#include <QVector>
#include <functional>

class FormatDocument {
public:
  virtual ~FormatDocument() = default;

  virtual QString title() const = 0;
  virtual QStringList chapterTitles() const = 0;
  virtual QString readAllText() const = 0;
  virtual QString readAllPlainText() const { return readAllText(); }
  virtual QStringList chaptersText() const { return {}; }
  virtual QStringList chaptersPlainText() const { return chaptersText(); }
  virtual QStringList imagePaths() const { return {}; }
  virtual QString coverPath() const { return {}; }
  virtual QString authors() const { return {}; }
  virtual QString series() const { return {}; }
  virtual QString publisher() const { return {}; }
  virtual QString description() const { return {}; }
  virtual QStringList tocTitles() const { return {}; }
  virtual QVector<int> tocChapterIndices() const { return {}; }
  virtual bool isRichText() const { return false; }
  virtual bool ttsDisabled() const { return false; }
  virtual bool ensureImage(int index) { Q_UNUSED(index) return true; }
  virtual void setImageReadyCallback(std::function<void(int)> callback) { Q_UNUSED(callback) }
};
