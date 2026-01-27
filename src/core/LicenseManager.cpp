#include "include/LicenseManager.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

LicenseManager::LicenseManager(QObject *parent) : QObject(parent), m_basePath(findBasePath()) {}

QString LicenseManager::basePath() const { return m_basePath; }

QVariantList LicenseManager::licenses() const {
  QVariantList list;
  auto addEntry = [&list](const QString &name, const QString &path) {
    QVariantMap entry;
    entry.insert("name", name);
    entry.insert("path", path);
    list.push_back(entry);
  };

  addEntry("My Ereader (GPL-2.0-or-later)", QDir(m_basePath).filePath("licenses/PROJECT_LICENSE.txt"));
  addEntry("Monocypher", QDir(m_basePath).filePath("licenses/Monocypher.txt"));
  addEntry("Poppler COPYING", QDir(m_basePath).filePath("licenses/Poppler_COPYING.txt"));
  addEntry("Poppler COPYING3", QDir(m_basePath).filePath("licenses/Poppler_COPYING3.txt"));

  return list;
}

QString LicenseManager::readFile(const QString &path) const {
  QFile file(path);
  if (!file.exists()) {
    return QString("File not found: %1").arg(path);
  }
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return QString("Unable to open: %1").arg(path);
  }

  QTextStream stream(&file);
  stream.setCodec("UTF-8");
  return stream.readAll();
}

QString LicenseManager::findBasePath() const {
  QDir dir(QCoreApplication::applicationDirPath());
  for (int i = 0; i < 5; ++i) {
    if (QDir(dir.filePath("licenses")).exists()) {
      return dir.absolutePath();
    }
    if (!dir.cdUp()) {
      break;
    }
  }
  return QCoreApplication::applicationDirPath();
}
