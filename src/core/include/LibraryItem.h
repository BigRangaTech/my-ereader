#pragma once

#include <QString>

struct LibraryItem {
  int id = 0;
  QString title;
  QString authors;
  QString path;
  QString format;
  QString fileHash;
  QString addedAt;
};
