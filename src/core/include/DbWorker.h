#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QVector>

#include "LibraryItem.h"
#include "AnnotationModel.h"

class DbWorker : public QObject {
  Q_OBJECT

public:
  explicit DbWorker(QObject *parent = nullptr);

public slots:
  void openAt(const QString &dbPath);
  void openEncryptedVault(const QString &vaultPath, const QString &passphrase);
  void saveEncryptedVault(const QString &vaultPath, const QString &passphrase);
  void closeDatabase();
  void addBook(const QString &filePath);
  void loadLibrary();
  void loadAnnotations(int libraryItemId);
  void addAnnotation(int libraryItemId,
                     const QString &locator,
                     const QString &type,
                     const QString &text,
                     const QString &color);
  void deleteAnnotation(int annotationId, int libraryItemId);

signals:
  void openFinished(bool ok, const QString &error);
  void saveFinished(bool ok, const QString &error);
  void libraryLoaded(const QVector<LibraryItem> &items);
  void annotationsLoaded(int libraryItemId, const QVector<AnnotationItem> &items);
  void addBookFinished(bool ok, const QString &error);
  void addAnnotationFinished(bool ok, const QString &error);
  void deleteAnnotationFinished(bool ok, const QString &error);

private:
  bool openDatabase(const QString &dbPath, QString *error);
  bool ensureSchema(QString *error);
  QVector<LibraryItem> fetchLibrary(QString *error);
  QVector<AnnotationItem> fetchAnnotations(int libraryItemId, QString *error);
  bool insertLibraryItem(const LibraryItem &item, QString *error);
  LibraryItem makeItemFromFile(const QString &filePath);
  QString computeFileHash(const QString &filePath);
  void *sqliteHandle() const;
  bool deserializeToMemory(const QByteArray &dbBytes, QString *error);
  QByteArray serializeFromMemory(QString *error);

  QSqlDatabase m_db;
  QString m_connectionName;
};

DbWorker *dbWorker();
