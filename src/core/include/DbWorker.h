#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QVariantList>
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
  void updateLibraryItem(int id,
                         const QString &title,
                         const QString &authors,
                         const QString &series,
                         const QString &publisher,
                         const QString &description,
                         const QString &tags,
                         const QString &collection);
  void deleteLibraryItem(int id);
  void bulkUpdateTagsCollection(const QVector<int> &ids,
                                const QString &tags,
                                const QString &collection,
                                bool updateTags,
                                bool updateCollection);
  void deleteLibraryItems(const QVector<int> &ids);
  void loadLibrary();
  void loadLibraryFiltered(const QString &searchQuery,
                           const QString &sortKey,
                           bool sortDescending,
                           const QString &filterTag,
                           const QString &filterCollection);
  void loadAnnotations(int libraryItemId);
  void addAnnotation(int libraryItemId,
                     const QString &locator,
                     const QString &type,
                     const QString &text,
                     const QString &color);
  void updateAnnotation(int annotationId,
                        int libraryItemId,
                        const QString &locator,
                        const QString &type,
                        const QString &text,
                        const QString &color);
  void deleteAnnotation(int annotationId, int libraryItemId);
  QVariantList exportAnnotationSync();
  int importAnnotationSync(const QVariantList &payload);
  QVariantList exportLibrarySync();
  int importLibrarySync(const QVariantList &payload);

signals:
  void openFinished(bool ok, const QString &error);
  void saveFinished(bool ok, const QString &error);
  void libraryLoaded(const QVector<LibraryItem> &items);
  void annotationsLoaded(int libraryItemId, const QVector<AnnotationItem> &items);
  void addBookFinished(bool ok, const QString &error);
  void updateBookFinished(bool ok, const QString &error);
  void deleteBookFinished(bool ok, const QString &error);
  void addAnnotationFinished(bool ok, const QString &error);
  void updateAnnotationFinished(bool ok, const QString &error);
  void deleteAnnotationFinished(bool ok, const QString &error);
  void annotationsChanged();

private:
  bool openDatabase(const QString &dbPath, QString *error);
  bool ensureSchema(QString *error);
  bool ensureColumn(const QString &table, const QString &column, const QString &type, QString *error);
  bool ensureAnnotationUuids(QString *error);
  bool ensureLibraryUpdatedAt(QString *error);
  QVector<LibraryItem> fetchLibrary(QString *error);
  QVector<LibraryItem> fetchLibraryFiltered(const QString &searchQuery,
                                            const QString &sortKey,
                                            bool sortDescending,
                                            const QString &filterTag,
                                            const QString &filterCollection,
                                            QString *error);
  QVector<AnnotationItem> fetchAnnotations(int libraryItemId, QString *error);
  bool insertLibraryItem(const LibraryItem &item, QString *error);
  LibraryItem makeItemFromFile(const QString &filePath);
  QString computeFileHash(const QString &filePath);
  void *sqliteHandle() const;
  bool deserializeToMemory(const QByteArray &dbBytes, QString *error);
  QByteArray serializeFromMemory(QString *error);

  QSqlDatabase m_db;
  QString m_connectionName;
  QString m_searchQuery;
  QString m_sortKey = "title";
  bool m_sortDescending = false;
  QString m_filterTag;
  QString m_filterCollection;
};

DbWorker *dbWorker();
