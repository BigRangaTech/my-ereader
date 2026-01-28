#pragma once

#include <QString>
#include <QMetaType>
#include <QVector>

struct LibraryItem {
  int id = 0;
  QString title;
  QString authors;
  QString series;
  QString publisher;
  QString description;
  QString path;
  QString format;
  QString fileHash;
  QString addedAt;
};

Q_DECLARE_METATYPE(LibraryItem)
Q_DECLARE_METATYPE(QVector<LibraryItem>)
