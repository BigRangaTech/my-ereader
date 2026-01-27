#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "AppInfo.h"
#include "LibraryModel.h"
#include "LicenseManager.h"
#include "ReaderController.h"
#include "SyncManager.h"
#include "TtsController.h"

int main(int argc, char *argv[]) {
  QGuiApplication app(argc, argv);

  QCoreApplication::setOrganizationName("MyEreader");
  QCoreApplication::setApplicationName(AppInfo::kName);
  QCoreApplication::setApplicationVersion(AppInfo::kVersion);

  qmlRegisterType<LibraryModel>("Ereader", 1, 0, "LibraryModel");
  qmlRegisterType<LicenseManager>("Ereader", 1, 0, "LicenseManager");
  qmlRegisterType<ReaderController>("Ereader", 1, 0, "ReaderController");
  qmlRegisterType<SyncManager>("Ereader", 1, 0, "SyncManager");
  qmlRegisterType<TtsController>("Ereader", 1, 0, "TtsController");

  QQmlApplicationEngine engine;
  const QUrl url(QStringLiteral("qrc:/Ereader/Main.qml"));
  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreated, &app,
      [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) {
          QCoreApplication::exit(-1);
        }
      },
      Qt::QueuedConnection);
  engine.load(url);

  return app.exec();
}
