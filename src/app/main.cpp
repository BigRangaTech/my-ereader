#if !defined(Q_OS_ANDROID)
#include <QApplication>
#else
#include <QGuiApplication>
#endif
#include <QIcon>
#include <QDir>
#include <QFileInfo>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQmlContext>
#include <QStandardPaths>

#include "AppInfo.h"
#include "AnnotationModel.h"
#include "LibraryModel.h"
#include "Logger.h"
#include "LicenseManager.h"
#include "ReaderController.h"
#include "SettingsManager.h"
#include "UpdateManager.h"
#include "SyncManager.h"
#include "TtsController.h"
#include "VaultController.h"

namespace {
QString findIconPath() {
  QDir dir(QCoreApplication::applicationDirPath());
  for (int i = 0; i < 6; ++i) {
    const QString candidate = dir.filePath("icon/icon.png");
    if (QFileInfo::exists(candidate)) {
      return candidate;
    }
    if (!dir.cdUp()) {
      break;
    }
  }
  return {};
}
} // namespace

int main(int argc, char *argv[]) {
#if !defined(Q_OS_ANDROID)
  QApplication app(argc, argv);
#else
  QGuiApplication app(argc, argv);
#endif
  QQuickStyle::setStyle("Basic");
  Logger::init();

  QCoreApplication::setOrganizationName("MyEreader");
  QCoreApplication::setApplicationName(AppInfo::kName);
  QCoreApplication::setApplicationVersion(AppInfo::kVersion);

  qmlRegisterType<LibraryModel>("Ereader", 1, 0, "LibraryModel");
  qmlRegisterType<AnnotationModel>("Ereader", 1, 0, "AnnotationModel");
  qmlRegisterType<LicenseManager>("Ereader", 1, 0, "LicenseManager");
  qmlRegisterType<ReaderController>("Ereader", 1, 0, "ReaderController");
  qmlRegisterType<SettingsManager>("Ereader", 1, 0, "SettingsManager");
  qmlRegisterType<UpdateManager>("Ereader", 1, 0, "UpdateManager");
  qmlRegisterType<SyncManager>("Ereader", 1, 0, "SyncManager");
  qmlRegisterType<TtsController>("Ereader", 1, 0, "TtsController");
  qmlRegisterType<VaultController>("Ereader", 1, 0, "VaultController");

  const QString iconPath = findIconPath();
  if (!iconPath.isEmpty()) {
    app.setWindowIcon(QIcon(iconPath));
  }

  QQmlApplicationEngine engine;
  const QString flatpakQmlPath = "/app/share/my-ereader/qml";
  if (QFileInfo::exists(flatpakQmlPath)) {
    engine.addImportPath(flatpakQmlPath);
  }
  const QStringList dataQmlPaths =
      QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                "my-ereader/qml",
                                QStandardPaths::LocateDirectory);
  for (const auto &path : dataQmlPaths) {
    engine.addImportPath(path);
  }
  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreated, &app,
      [](QObject *obj, const QUrl &) {
        if (!obj) {
          QCoreApplication::exit(-1);
        }
      },
      Qt::QueuedConnection);

  engine.loadFromModule("Ereader", "Main");

  return app.exec();
}
