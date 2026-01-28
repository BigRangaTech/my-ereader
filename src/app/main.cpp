#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQmlContext>

#include "AppInfo.h"
#include "LibraryModel.h"
#include "Logger.h"
#include "LicenseManager.h"
#include "ReaderController.h"
#include "SyncManager.h"
#include "TtsController.h"
#include "VaultController.h"

int main(int argc, char *argv[]) {
  QGuiApplication app(argc, argv);
  QQuickStyle::setStyle("Basic");
  Logger::init();

  QCoreApplication::setOrganizationName("MyEreader");
  QCoreApplication::setApplicationName(AppInfo::kName);
  QCoreApplication::setApplicationVersion(AppInfo::kVersion);

  qmlRegisterType<LibraryModel>("Ereader", 1, 0, "LibraryModel");
  qmlRegisterType<LicenseManager>("Ereader", 1, 0, "LicenseManager");
  qmlRegisterType<ReaderController>("Ereader", 1, 0, "ReaderController");
  qmlRegisterType<SyncManager>("Ereader", 1, 0, "SyncManager");
  qmlRegisterType<TtsController>("Ereader", 1, 0, "TtsController");
  qmlRegisterType<VaultController>("Ereader", 1, 0, "VaultController");

  QQmlApplicationEngine engine;
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
