#pragma once

#include <QString>
#include <QStringList>

class FormatDocument {
public:
  virtual ~FormatDocument() = default;

  virtual QString title() const = 0;
  virtual QStringList chapterTitles() const = 0;
  virtual QString readAllText() const = 0;
  virtual QStringList chaptersText() const { return {}; }
  virtual QStringList imagePaths() const { return {}; }
};
