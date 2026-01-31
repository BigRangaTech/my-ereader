#pragma once

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QProcessEnvironment>
#include <QStandardPaths>

namespace AppPaths {

inline QString repoRoot() {
  QDir dir(QCoreApplication::applicationDirPath());
  for (int i = 0; i < 6; ++i) {
    if (QFileInfo::exists(dir.filePath("README.md"))) {
      return dir.absolutePath();
    }
    if (!dir.cdUp()) {
      break;
    }
  }
  return QCoreApplication::applicationDirPath();
}

inline QString configRoot() {
  const QString env = QProcessEnvironment::systemEnvironment().value("MYEREADER_CONFIG_DIR").trimmed();
  if (!env.isEmpty()) {
    QDir dir(env);
    dir.mkpath(".");
    return dir.absolutePath();
  }
  const QString root = repoRoot();
  if (QFileInfo::exists(QDir(root).filePath("README.md"))) {
    QDir dir(root);
    dir.mkpath("config");
    return dir.filePath("config");
  }
  const QString base = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
  QDir dir(base);
  dir.mkpath(".");
  return dir.absolutePath();
}

inline QString configFile(const QString &name) {
  return QDir(configRoot()).filePath(name);
}

} // namespace AppPaths
