#include "include/Logger.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QMutex>
#include <QStandardPaths>

#include "include/AppPaths.h"

namespace {
QMutex g_logMutex;
QFile g_logFile;

QString typeLabel(QtMsgType type) {
  switch (type) {
  case QtDebugMsg:
    return "DEBUG";
  case QtInfoMsg:
    return "INFO";
  case QtWarningMsg:
    return "WARN";
  case QtCriticalMsg:
    return "ERROR";
  case QtFatalMsg:
    return "FATAL";
  }
  return "LOG";
}

void logHandler(QtMsgType type, const QMessageLogContext &, const QString &msg) {
  QMutexLocker locker(&g_logMutex);
  if (!g_logFile.isOpen()) {
    return;
  }
  const QString line = QString("%1 [%2] %3\n")
                           .arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate))
                           .arg(typeLabel(type))
                           .arg(msg);
  g_logFile.write(line.toUtf8());
  g_logFile.flush();
}
} // namespace

void Logger::init() {
  const QString dirPath = logDirectory();
  QDir().mkpath(dirPath);
  g_logFile.setFileName(logFilePath());
  if (g_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
    qInstallMessageHandler(logHandler);
    qInfo() << "Logging to" << g_logFile.fileName();
  }
}

QString Logger::logDirectory() {
  QDir dir(QCoreApplication::applicationDirPath());
  for (int i = 0; i < 5; ++i) {
    if (QFileInfo::exists(dir.filePath("README.md"))) {
      return dir.filePath("logs");
    }
    if (!dir.cdUp()) {
      break;
    }
  }
  QDir dataDir(AppPaths::dataRoot());
  dataDir.mkpath("logs");
  return dataDir.filePath("logs");
}

QString Logger::logFilePath() {
  return QDir(logDirectory()).filePath("app.log");
}
