#pragma once

#include <QString>

class Logger {
public:
  static void init();
  static QString logDirectory();
  static QString logFilePath();
};
