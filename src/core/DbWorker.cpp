#include "include/DbWorker.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QUuid>
#include <QStringList>
#include <QMetaType>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlDriver>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QThread>
#include <QVariant>
#include <QUrl>

#include <sqlite3.h>

#include "FormatRegistry.h"
#include "CryptoBackend.h"
#include "CryptoVault.h"
#include "include/AppPaths.h"

namespace {
QThread *workerThread() {
  static QThread *thread = []() {
    auto *t = new QThread();
    t->setObjectName("db-worker");
    t->start();
    QObject::connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, t, [t]() {
      t->quit();
      t->wait(3000);
    });
    return t;
  }();
  return thread;
}

QString defaultDbPath() {
  const QString base = AppPaths::dataRoot();
  QDir dir(base);
  if (!dir.exists()) {
    dir.mkpath(".");
  }
  return dir.filePath("library.db");
}

QString coverCacheDir() {
  const QString base = AppPaths::dataRoot();
  QDir dir(base);
  if (!dir.exists()) {
    dir.mkpath(".");
  }
  dir.mkpath("covers");
  return dir.filePath("covers");
}

QString normalizeCoverSource(const QString &path) {
  if (path.startsWith("file:")) {
    return QUrl(path).toLocalFile();
  }
  return path;
}

QString cacheCoverImage(const QString &coverPath, const QString &fileHash) {
  if (coverPath.isEmpty() || fileHash.isEmpty()) {
    return "";
  }
  const QString sourcePath = normalizeCoverSource(coverPath);
  if (sourcePath.isEmpty() || !QFileInfo::exists(sourcePath)) {
    return "";
  }
  const QString ext = QFileInfo(sourcePath).suffix().isEmpty() ? "jpg" : QFileInfo(sourcePath).suffix();
  const QString destPath = QDir(coverCacheDir()).filePath(fileHash + "." + ext);
  if (QFileInfo::exists(destPath)) {
    return destPath;
  }
  if (QFile::copy(sourcePath, destPath)) {
    return destPath;
  }
  return "";
}
} // namespace

DbWorker::DbWorker(QObject *parent) : QObject(parent) {
  qRegisterMetaType<LibraryItem>("LibraryItem");
  qRegisterMetaType<QVector<LibraryItem>>("QVector<LibraryItem>");
  qRegisterMetaType<AnnotationItem>("AnnotationItem");
  qRegisterMetaType<QVector<AnnotationItem>>("QVector<AnnotationItem>");
  qRegisterMetaType<QVector<int>>("QVector<int>");
}

DbWorker *dbWorker() {
  static DbWorker *worker = []() {
    auto *w = new DbWorker();
    w->moveToThread(workerThread());
    return w;
  }();
  return worker;
}

void DbWorker::openAt(const QString &dbPath) {
  QString error;
  if (!openDatabase(dbPath, &error)) {
    emit openFinished(false, error);
    return;
  }
  const QVector<LibraryItem> items =
      fetchLibraryFiltered(m_searchQuery, m_sortKey, m_sortDescending, m_filterTag, m_filterCollection, &error);
  if (!error.isEmpty()) {
    emit openFinished(false, error);
    return;
  }
  emit openFinished(true, "");
  emit libraryLoaded(items);
}

void DbWorker::openEncryptedVault(const QString &vaultPath, const QString &passphrase) {
  if (vaultPath.isEmpty()) {
    emit openFinished(false, "Vault path is empty");
    return;
  }
  auto backend = CryptoBackendFactory::createDefault();
  CryptoVault vault(std::move(backend));
  QString error;
  QByteArray dbBytes;
  if (!vault.decryptToBytes(vaultPath, passphrase, &dbBytes, &error)) {
    emit openFinished(false, error.isEmpty() ? "Failed to decrypt vault" : error);
    return;
  }
  if (!openDatabase(":memory:", &error)) {
    emit openFinished(false, error);
    return;
  }
  if (!deserializeToMemory(dbBytes, &error)) {
    qWarning() << "DbWorker: deserialize failed" << error;
    if (!ensureSchema(&error)) {
      emit openFinished(false, error);
      return;
    }
    qWarning() << "DbWorker: falling back to empty in-memory db";
  } else {
    if (!ensureSchema(&error)) {
      emit openFinished(false, error);
      return;
    }
  }
  const QVector<LibraryItem> items =
      fetchLibraryFiltered(m_searchQuery, m_sortKey, m_sortDescending, m_filterTag, m_filterCollection, &error);
  if (!error.isEmpty()) {
    emit openFinished(false, error);
    return;
  }
  emit openFinished(true, "");
  emit libraryLoaded(items);
}

void DbWorker::saveEncryptedVault(const QString &vaultPath, const QString &passphrase) {
  if (vaultPath.isEmpty()) {
    emit saveFinished(false, "Vault path is empty");
    return;
  }
  QString error;
  const QByteArray dbBytes = serializeFromMemory(&error);
  if (dbBytes.isEmpty()) {
    qWarning() << "DbWorker: serialize failed" << error;
    emit saveFinished(false, error.isEmpty() ? "Failed to serialize database" : error);
    return;
  }
  auto backend = CryptoBackendFactory::createDefault();
  CryptoVault vault(std::move(backend));
  if (!vault.encryptFromBytes(vaultPath, passphrase, dbBytes, &error)) {
    emit saveFinished(false, error.isEmpty() ? "Failed to encrypt vault" : error);
    return;
  }
  emit saveFinished(true, "");
}

void DbWorker::closeDatabase() {
  if (m_db.isOpen()) {
    m_db.close();
  }
  if (!m_connectionName.isEmpty()) {
    QSqlDatabase::removeDatabase(m_connectionName);
    m_connectionName.clear();
  }
  m_db = QSqlDatabase();
}

void DbWorker::addBook(const QString &filePath) {
  QString error;
  if (!m_db.isOpen()) {
    emit addBookFinished(false, "Database not open");
    return;
  }
  if (!QFileInfo::exists(filePath)) {
    emit addBookFinished(false, "File does not exist");
    return;
  }
  LibraryItem item = makeItemFromFile(filePath);
  if (!insertLibraryItem(item, &error)) {
    emit addBookFinished(false, error);
    return;
  }
  emit addBookFinished(true, "");
  emit libraryLoaded(fetchLibraryFiltered(m_searchQuery, m_sortKey, m_sortDescending,
                                          m_filterTag, m_filterCollection, &error));
}

void DbWorker::updateLibraryItem(int id,
                                 const QString &title,
                                 const QString &authors,
                                 const QString &series,
                                 const QString &publisher,
                                 const QString &description,
                                 const QString &tags,
                                 const QString &collection) {
  QString error;
  if (!m_db.isOpen()) {
    emit updateBookFinished(false, "Database not open");
    return;
  }
  if (id <= 0) {
    emit updateBookFinished(false, "Invalid library item");
    return;
  }
  QSqlQuery query(m_db);
  query.prepare(
      "UPDATE library_items SET title = ?, authors = ?, series = ?, publisher = ?, description = ?, "
      "tags = ?, collection = ?, updated_at = ? WHERE id = ?");
  query.addBindValue(title);
  query.addBindValue(authors);
  query.addBindValue(series);
  query.addBindValue(publisher);
  query.addBindValue(description);
  query.addBindValue(tags);
  query.addBindValue(collection);
  query.addBindValue(QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
  query.addBindValue(id);
  if (!query.exec()) {
    emit updateBookFinished(false, query.lastError().text());
    return;
  }
  emit updateBookFinished(true, "");
  emit libraryLoaded(fetchLibraryFiltered(m_searchQuery, m_sortKey, m_sortDescending,
                                          m_filterTag, m_filterCollection, &error));
}

void DbWorker::deleteLibraryItem(int id) {
  QString error;
  if (!m_db.isOpen()) {
    emit deleteBookFinished(false, "Database not open");
    return;
  }
  if (id <= 0) {
    emit deleteBookFinished(false, "Invalid library item");
    return;
  }
  QSqlQuery query(m_db);
  query.prepare("DELETE FROM annotations WHERE library_item_id = ?");
  query.addBindValue(id);
  if (!query.exec()) {
    emit deleteBookFinished(false, query.lastError().text());
    return;
  }
  QSqlQuery remove(m_db);
  remove.prepare("DELETE FROM library_items WHERE id = ?");
  remove.addBindValue(id);
  if (!remove.exec()) {
    emit deleteBookFinished(false, remove.lastError().text());
    return;
  }
  emit deleteBookFinished(true, "");
  emit libraryLoaded(fetchLibraryFiltered(m_searchQuery, m_sortKey, m_sortDescending,
                                          m_filterTag, m_filterCollection, &error));
}

void DbWorker::bulkUpdateTagsCollection(const QVector<int> &ids,
                                        const QString &tags,
                                        const QString &collection,
                                        bool updateTags,
                                        bool updateCollection) {
  QString error;
  if (!m_db.isOpen()) {
    emit updateBookFinished(false, "Database not open");
    return;
  }
  if (ids.isEmpty()) {
    emit updateBookFinished(false, "No items selected");
    return;
  }
  if (!updateTags && !updateCollection) {
    emit updateBookFinished(false, "Nothing to update");
    return;
  }
  if (!m_db.transaction()) {
    emit updateBookFinished(false, "Failed to start transaction");
    return;
  }
  for (int id : ids) {
    if (id <= 0) {
      continue;
    }
    QSqlQuery query(m_db);
    const QString now = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    if (updateTags && updateCollection) {
      query.prepare("UPDATE library_items SET tags = ?, collection = ?, updated_at = ? WHERE id = ?");
      query.addBindValue(tags);
      query.addBindValue(collection);
      query.addBindValue(now);
      query.addBindValue(id);
    } else if (updateTags) {
      query.prepare("UPDATE library_items SET tags = ?, updated_at = ? WHERE id = ?");
      query.addBindValue(tags);
      query.addBindValue(now);
      query.addBindValue(id);
    } else {
      query.prepare("UPDATE library_items SET collection = ?, updated_at = ? WHERE id = ?");
      query.addBindValue(collection);
      query.addBindValue(now);
      query.addBindValue(id);
    }
    if (!query.exec()) {
      m_db.rollback();
      emit updateBookFinished(false, query.lastError().text());
      return;
    }
  }
  if (!m_db.commit()) {
    emit updateBookFinished(false, "Failed to commit changes");
    return;
  }
  emit updateBookFinished(true, "");
  emit libraryLoaded(fetchLibraryFiltered(m_searchQuery, m_sortKey, m_sortDescending,
                                          m_filterTag, m_filterCollection, &error));
}

void DbWorker::deleteLibraryItems(const QVector<int> &ids) {
  QString error;
  if (!m_db.isOpen()) {
    emit deleteBookFinished(false, "Database not open");
    return;
  }
  if (ids.isEmpty()) {
    emit deleteBookFinished(false, "No items selected");
    return;
  }
  if (!m_db.transaction()) {
    emit deleteBookFinished(false, "Failed to start transaction");
    return;
  }
  for (int id : ids) {
    if (id <= 0) {
      continue;
    }
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM annotations WHERE library_item_id = ?");
    query.addBindValue(id);
    if (!query.exec()) {
      m_db.rollback();
      emit deleteBookFinished(false, query.lastError().text());
      return;
    }
    QSqlQuery remove(m_db);
    remove.prepare("DELETE FROM library_items WHERE id = ?");
    remove.addBindValue(id);
    if (!remove.exec()) {
      m_db.rollback();
      emit deleteBookFinished(false, remove.lastError().text());
      return;
    }
  }
  if (!m_db.commit()) {
    emit deleteBookFinished(false, "Failed to commit changes");
    return;
  }
  emit deleteBookFinished(true, "");
  emit libraryLoaded(fetchLibraryFiltered(m_searchQuery, m_sortKey, m_sortDescending,
                                          m_filterTag, m_filterCollection, &error));
}

void DbWorker::loadLibrary() {
  QString error;
  const QVector<LibraryItem> items =
      fetchLibraryFiltered(m_searchQuery, m_sortKey, m_sortDescending, m_filterTag, m_filterCollection, &error);
  emit libraryLoaded(items);
}

void DbWorker::loadLibraryFiltered(const QString &searchQuery,
                                   const QString &sortKey,
                                   bool sortDescending,
                                   const QString &filterTag,
                                   const QString &filterCollection) {
  m_searchQuery = searchQuery;
  m_sortKey = sortKey.isEmpty() ? QStringLiteral("title") : sortKey;
  m_sortDescending = sortDescending;
  m_filterTag = filterTag;
  m_filterCollection = filterCollection;
  loadLibrary();
}

void DbWorker::loadAnnotations(int libraryItemId) {
  QString error;
  const QVector<AnnotationItem> items = fetchAnnotations(libraryItemId, &error);
  emit annotationsLoaded(libraryItemId, items);
}

void DbWorker::addAnnotation(int libraryItemId,
                             const QString &locator,
                             const QString &type,
                             const QString &text,
                             const QString &color) {
  if (!m_db.isOpen()) {
    emit addAnnotationFinished(false, "Database not open");
    return;
  }
  if (libraryItemId <= 0) {
    emit addAnnotationFinished(false, "No book selected");
    return;
  }
  QSqlQuery query(m_db);
  const QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
  query.prepare("INSERT INTO annotations (uuid, library_item_id, locator, type, text, color, created_at) "
                "VALUES (?, ?, ?, ?, ?, ?, ?)");
  query.addBindValue(uuid);
  query.addBindValue(libraryItemId);
  query.addBindValue(locator);
  query.addBindValue(type);
  query.addBindValue(text);
  query.addBindValue(color);
  query.addBindValue(QDateTime::currentDateTimeUtc().toString(Qt::ISODate));

  if (!query.exec()) {
    emit addAnnotationFinished(false, query.lastError().text());
    return;
  }
  emit addAnnotationFinished(true, "");
  loadAnnotations(libraryItemId);
  emit annotationsChanged();
}

void DbWorker::updateAnnotation(int annotationId,
                                int libraryItemId,
                                const QString &locator,
                                const QString &type,
                                const QString &text,
                                const QString &color) {
  if (!m_db.isOpen()) {
    emit updateAnnotationFinished(false, "Database not open");
    return;
  }
  if (libraryItemId <= 0 || annotationId <= 0) {
    emit updateAnnotationFinished(false, "No annotation selected");
    return;
  }
  QSqlQuery query(m_db);
  query.prepare("UPDATE annotations SET locator = ?, type = ?, text = ?, color = ? WHERE id = ? AND library_item_id = ?");
  query.addBindValue(locator);
  query.addBindValue(type);
  query.addBindValue(text);
  query.addBindValue(color);
  query.addBindValue(annotationId);
  query.addBindValue(libraryItemId);
  if (!query.exec()) {
    emit updateAnnotationFinished(false, query.lastError().text());
    return;
  }
  emit updateAnnotationFinished(true, "");
  loadAnnotations(libraryItemId);
  emit annotationsChanged();
}

void DbWorker::deleteAnnotation(int annotationId, int libraryItemId) {
  if (!m_db.isOpen()) {
    emit deleteAnnotationFinished(false, "Database not open");
    return;
  }
  QSqlQuery query(m_db);
  query.prepare("DELETE FROM annotations WHERE id = ?");
  query.addBindValue(annotationId);
  if (!query.exec()) {
    emit deleteAnnotationFinished(false, query.lastError().text());
    return;
  }
  emit deleteAnnotationFinished(true, "");
  loadAnnotations(libraryItemId);
  emit annotationsChanged();
}

QVariantList DbWorker::exportAnnotationSync() {
  QVariantList list;
  if (!m_db.isOpen()) {
    return list;
  }
  QSqlQuery query(m_db);
  query.prepare(
      "SELECT library_items.file_hash, annotations.uuid, annotations.locator, annotations.type, "
      "annotations.text, annotations.color, annotations.created_at "
      "FROM annotations "
      "JOIN library_items ON annotations.library_item_id = library_items.id "
      "WHERE library_items.file_hash IS NOT NULL AND library_items.file_hash != ''");
  if (!query.exec()) {
    return list;
  }
  while (query.next()) {
    QVariantMap item;
    item.insert("file_hash", query.value(0).toString());
    item.insert("uuid", query.value(1).toString());
    item.insert("locator", query.value(2).toString());
    item.insert("type", query.value(3).toString());
    item.insert("text", query.value(4).toString());
    item.insert("color", query.value(5).toString());
    item.insert("created_at", query.value(6).toString());
    list.append(item);
  }
  return list;
}

int DbWorker::importAnnotationSync(const QVariantList &payload) {
  if (!m_db.isOpen() || payload.isEmpty()) {
    return 0;
  }
  int added = 0;
  QSqlQuery findItem(m_db);
  QSqlQuery check(m_db);
  QSqlQuery insert(m_db);
  findItem.prepare("SELECT id FROM library_items WHERE file_hash = ? LIMIT 1");
  check.prepare("SELECT id FROM annotations WHERE uuid = ? LIMIT 1");
  insert.prepare("INSERT INTO annotations (uuid, library_item_id, locator, type, text, color, created_at) "
                 "VALUES (?, ?, ?, ?, ?, ?, ?)");
  for (const auto &entry : payload) {
    const QVariantMap map = entry.toMap();
    const QString fileHash = map.value("file_hash").toString();
    if (fileHash.isEmpty()) {
      continue;
    }
    findItem.bindValue(0, fileHash);
    if (!findItem.exec() || !findItem.next()) {
      findItem.finish();
      continue;
    }
    const int libraryItemId = findItem.value(0).toInt();
    findItem.finish();
    QString uuid = map.value("uuid").toString();
    if (uuid.isEmpty()) {
      uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    }
    check.bindValue(0, uuid);
    if (check.exec() && check.next()) {
      check.finish();
      continue;
    }
    check.finish();
    insert.addBindValue(uuid);
    insert.addBindValue(libraryItemId);
    insert.addBindValue(map.value("locator").toString());
    insert.addBindValue(map.value("type").toString());
    insert.addBindValue(map.value("text").toString());
    insert.addBindValue(map.value("color").toString());
    const QString createdAt = map.value("created_at").toString();
    insert.addBindValue(createdAt.isEmpty()
                            ? QDateTime::currentDateTimeUtc().toString(Qt::ISODate)
                            : createdAt);
    if (insert.exec()) {
      added++;
    }
    insert.finish();
  }
  if (added > 0) {
    emit annotationsChanged();
  }
  return added;
}

QVariantList DbWorker::exportLibrarySync() {
  QVariantList list;
  if (!m_db.isOpen()) {
    return list;
  }
  QSqlQuery query(m_db);
  query.prepare("SELECT file_hash, title, authors, series, publisher, description, tags, collection, format, updated_at "
                "FROM library_items WHERE file_hash IS NOT NULL AND file_hash != ''");
  if (!query.exec()) {
    return list;
  }
  while (query.next()) {
    QVariantMap item;
    item.insert("file_hash", query.value(0).toString());
    item.insert("title", query.value(1).toString());
    item.insert("authors", query.value(2).toString());
    item.insert("series", query.value(3).toString());
    item.insert("publisher", query.value(4).toString());
    item.insert("description", query.value(5).toString());
    item.insert("tags", query.value(6).toString());
    item.insert("collection", query.value(7).toString());
    item.insert("format", query.value(8).toString());
    item.insert("updated_at", query.value(9).toString());
    list.append(item);
  }
  return list;
}

int DbWorker::importLibrarySync(const QVariantList &payload, const QString &conflictPolicy) {
  if (!m_db.isOpen() || payload.isEmpty()) {
    return 0;
  }
  int applied = 0;
  QSqlQuery findItem(m_db);
  QSqlQuery update(m_db);
  findItem.prepare("SELECT id, updated_at FROM library_items WHERE file_hash = ? LIMIT 1");
  update.prepare(
      "UPDATE library_items SET title = ?, authors = ?, series = ?, publisher = ?, description = ?, "
      "tags = ?, collection = ?, updated_at = ? WHERE id = ?");
  for (const auto &entry : payload) {
    const QVariantMap map = entry.toMap();
    const QString fileHash = map.value("file_hash").toString();
    if (fileHash.isEmpty()) {
      continue;
    }
    findItem.bindValue(0, fileHash);
    if (!findItem.exec() || !findItem.next()) {
      findItem.finish();
      continue;
    }
    const int id = findItem.value(0).toInt();
    const QString localUpdated = findItem.value(1).toString();
    findItem.finish();

    const QString remoteUpdated = map.value("updated_at").toString();
    const qint64 localTime = QDateTime::fromString(localUpdated, Qt::ISODate).toMSecsSinceEpoch();
    const qint64 remoteTime = QDateTime::fromString(remoteUpdated, Qt::ISODate).toMSecsSinceEpoch();
    const QString policy = conflictPolicy.trimmed().toLower();
    if (policy == "prefer_local") {
      continue;
    }
    if (policy != "prefer_remote") {
      if (remoteTime <= 0 || remoteTime <= localTime) {
        continue;
      }
    }

    update.addBindValue(map.value("title").toString());
    update.addBindValue(map.value("authors").toString());
    update.addBindValue(map.value("series").toString());
    update.addBindValue(map.value("publisher").toString());
    update.addBindValue(map.value("description").toString());
    update.addBindValue(map.value("tags").toString());
    update.addBindValue(map.value("collection").toString());
    update.addBindValue(remoteUpdated);
    update.addBindValue(id);
    if (update.exec()) {
      applied++;
    }
    update.finish();
  }
  if (applied > 0) {
    emit libraryLoaded(fetchLibraryFiltered(m_searchQuery, m_sortKey, m_sortDescending,
                                            m_filterTag, m_filterCollection, nullptr));
  }
  return applied;
}

bool DbWorker::hasFileHash(const QString &fileHash) {
  if (!m_db.isOpen() || fileHash.trimmed().isEmpty()) {
    return false;
  }
  QSqlQuery query(m_db);
  query.prepare("SELECT id FROM library_items WHERE file_hash = ? LIMIT 1");
  query.addBindValue(fileHash);
  if (!query.exec()) {
    return false;
  }
  return query.next();
}

QString DbWorker::pathForHash(const QString &fileHash) {
  if (!m_db.isOpen() || fileHash.trimmed().isEmpty()) {
    return {};
  }
  QSqlQuery query(m_db);
  query.prepare("SELECT path FROM library_items WHERE file_hash = ? LIMIT 1");
  query.addBindValue(fileHash);
  if (!query.exec() || !query.next()) {
    return {};
  }
  return query.value(0).toString();
}

bool DbWorker::openDatabase(const QString &dbPath, QString *error) {
  if (m_db.isOpen()) {
    m_db.close();
  }
  if (!m_connectionName.isEmpty()) {
    const QString oldName = m_connectionName;
    m_db = QSqlDatabase();
    QSqlDatabase::removeDatabase(oldName);
  }
  m_connectionName = QString("library_worker_%1").arg(reinterpret_cast<quintptr>(this));
  m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
  const QString resolvedPath = dbPath.isEmpty() ? defaultDbPath() : dbPath;
  m_db.setDatabaseName(resolvedPath);
  qInfo() << "DbWorker: opening database" << resolvedPath;
  if (!m_db.open()) {
    if (error) {
      *error = m_db.lastError().text();
    }
    qWarning() << "DbWorker: open failed" << m_db.lastError().text();
    return false;
  }
  const QVariant driverHandle = m_db.driver()->handle();
  const bool handleValid = driverHandle.isValid();
  qInfo() << "DbWorker: driver handle valid" << handleValid << driverHandle.typeName();
  if (!ensureSchema(error)) {
    if (error && !error->isEmpty()) {
      qWarning() << "DbWorker: ensureSchema failed" << *error;
    } else {
      qWarning() << "DbWorker: ensureSchema failed";
    }
    return false;
  }
  qInfo() << "DbWorker: open ok";
  return true;
}

bool DbWorker::ensureSchema(QString *error) {
  QSqlQuery query(m_db);
  if (!query.exec(
          "CREATE TABLE IF NOT EXISTS library_items ("
          "id INTEGER PRIMARY KEY AUTOINCREMENT,"
          "title TEXT,"
          "authors TEXT,"
          "series TEXT,"
          "publisher TEXT,"
          "description TEXT,"
          "tags TEXT,"
          "collection TEXT,"
          "cover_path TEXT,"
          "path TEXT UNIQUE,"
          "format TEXT,"
          "file_hash TEXT,"
          "added_at TEXT,"
          "updated_at TEXT"
          ")")) {
    if (error) {
      *error = query.lastError().text();
    }
    return false;
  }
  if (!query.exec(
          "CREATE TABLE IF NOT EXISTS annotations ("
          "id INTEGER PRIMARY KEY AUTOINCREMENT,"
          "library_item_id INTEGER NOT NULL,"
          "locator TEXT NOT NULL,"
          "type TEXT NOT NULL,"
          "text TEXT,"
          "color TEXT,"
          "uuid TEXT,"
          "created_at TEXT,"
          "FOREIGN KEY(library_item_id) REFERENCES library_items(id)"
          ")")) {
    if (error) {
      *error = query.lastError().text();
    }
    return false;
  }
  if (!ensureColumn("library_items", "series", "TEXT", error)) {
    return false;
  }
  if (!ensureColumn("library_items", "publisher", "TEXT", error)) {
    return false;
  }
  if (!ensureColumn("library_items", "description", "TEXT", error)) {
    return false;
  }
  if (!ensureColumn("library_items", "tags", "TEXT", error)) {
    return false;
  }
  if (!ensureColumn("library_items", "collection", "TEXT", error)) {
    return false;
  }
  if (!ensureColumn("library_items", "cover_path", "TEXT", error)) {
    return false;
  }
  if (!ensureColumn("library_items", "updated_at", "TEXT", error)) {
    return false;
  }
  if (!ensureColumn("annotations", "uuid", "TEXT", error)) {
    return false;
  }
  if (!ensureLibraryUpdatedAt(error)) {
    return false;
  }
  if (!ensureAnnotationUuids(error)) {
    return false;
  }
  return true;
}

bool DbWorker::attachDatabase(const QString &path, const QString &schema, QString *error) {
  if (!m_db.isOpen()) {
    if (error) {
      *error = "Database not open";
    }
    return false;
  }
  QSqlQuery query(m_db);
  QString escapedPath = path;
  escapedPath.replace("'", "''");
  const QString sql = QString("ATTACH DATABASE '%1' AS %2")
                          .arg(escapedPath)
                          .arg(schema);
  if (!query.exec(sql)) {
    if (error) {
      *error = query.lastError().text();
    }
    return false;
  }
  return true;
}

void DbWorker::detachDatabase(const QString &schema) {
  if (!m_db.isOpen()) {
    return;
  }
  QSqlQuery query(m_db);
  query.exec(QString("DETACH DATABASE %1").arg(schema));
}

bool DbWorker::ensureColumn(const QString &table,
                            const QString &column,
                            const QString &type,
                            QString *error) {
  QSqlQuery query(m_db);
  if (!query.exec(QString("PRAGMA table_info(%1)").arg(table))) {
    if (error) {
      *error = query.lastError().text();
    }
    return false;
  }
  bool exists = false;
  while (query.next()) {
    if (query.value(1).toString() == column) {
      exists = true;
      break;
    }
  }
  if (exists) {
    return true;
  }
  QSqlQuery alter(m_db);
  if (!alter.exec(QString("ALTER TABLE %1 ADD COLUMN %2 %3").arg(table, column, type))) {
    if (error) {
      *error = alter.lastError().text();
    }
    return false;
  }
  return true;
}

bool DbWorker::ensureAnnotationUuids(QString *error) {
  QSqlQuery query(m_db);
  if (!query.exec("SELECT id FROM annotations WHERE uuid IS NULL OR uuid = ''")) {
    if (error) {
      *error = query.lastError().text();
    }
    return false;
  }
  while (query.next()) {
    const int id = query.value(0).toInt();
    const QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QSqlQuery update(m_db);
    update.prepare("UPDATE annotations SET uuid = ? WHERE id = ?");
    update.addBindValue(uuid);
    update.addBindValue(id);
    if (!update.exec()) {
      if (error) {
        *error = update.lastError().text();
      }
      return false;
    }
  }
  return true;
}

bool DbWorker::ensureLibraryUpdatedAt(QString *error) {
  QSqlQuery query(m_db);
  if (!query.exec("SELECT id, added_at FROM library_items WHERE updated_at IS NULL OR updated_at = ''")) {
    if (error) {
      *error = query.lastError().text();
    }
    return false;
  }
  while (query.next()) {
    const int id = query.value(0).toInt();
    const QString addedAt = query.value(1).toString();
    const QString updatedAt =
        addedAt.isEmpty() ? QDateTime::currentDateTimeUtc().toString(Qt::ISODate) : addedAt;
    QSqlQuery update(m_db);
    update.prepare("UPDATE library_items SET updated_at = ? WHERE id = ?");
    update.addBindValue(updatedAt);
    update.addBindValue(id);
    if (!update.exec()) {
      if (error) {
        *error = update.lastError().text();
      }
      return false;
    }
  }
  return true;
}

QVector<LibraryItem> DbWorker::fetchLibrary(QString *error) {
  return fetchLibraryFiltered(QString(), QStringLiteral("title"), false, QString(), QString(), error);
}

QVector<LibraryItem> DbWorker::fetchLibraryFiltered(const QString &searchQuery,
                                                    const QString &sortKey,
                                                    bool sortDescending,
                                                    const QString &filterTag,
                                                    const QString &filterCollection,
                                                    QString *error) {
  QVector<LibraryItem> items;
  if (!m_db.isOpen()) {
    if (error) {
      *error = "Database not open";
    }
    return items;
  }

  QString sortColumn = "title";
  if (sortKey == "authors") sortColumn = "authors";
  else if (sortKey == "series") sortColumn = "series";
  else if (sortKey == "publisher") sortColumn = "publisher";
  else if (sortKey == "format") sortColumn = "format";
  else if (sortKey == "added") sortColumn = "added_at";
  else if (sortKey == "collection") sortColumn = "collection";

  const bool sortIsText =
      (sortColumn == "title" || sortColumn == "authors" || sortColumn == "series" ||
       sortColumn == "publisher" || sortColumn == "format" || sortColumn == "collection");

  QString sql =
      "SELECT id, title, authors, series, publisher, description, tags, collection, cover_path, path, format, file_hash, added_at, updated_at, "
      "(SELECT COUNT(*) FROM annotations WHERE library_item_id = library_items.id) "
      "FROM library_items";

  const QString trimmed = searchQuery.trimmed();
  const QString trimmedTag = filterTag.trimmed();
  const QString trimmedCollection = filterCollection.trimmed();
  QStringList whereParts;
  if (!trimmed.isEmpty()) {
    whereParts << "(title LIKE ? OR authors LIKE ? OR series LIKE ? OR publisher LIKE ? "
                  "OR description LIKE ? OR tags LIKE ? OR collection LIKE ? OR path LIKE ?)";
  }
  if (!trimmedTag.isEmpty() && trimmedTag != "__all__") {
    if (trimmedTag == "__none__") {
      whereParts << "(tags IS NULL OR tags = '')";
    } else {
      whereParts << "tags LIKE ?";
    }
  }
  if (!trimmedCollection.isEmpty() && trimmedCollection != "__all__") {
    if (trimmedCollection == "__none__") {
      whereParts << "(collection IS NULL OR collection = '')";
    } else {
      whereParts << "collection = ?";
    }
  }
  if (!whereParts.isEmpty()) {
    sql += " WHERE " + whereParts.join(" AND ");
  }

  sql += " ORDER BY " + sortColumn;
  if (sortIsText) {
    sql += " COLLATE NOCASE";
  }
  if (sortDescending) {
    sql += " DESC";
  }

  QSqlQuery query(m_db);
  if (!query.prepare(sql)) {
    if (error) {
      *error = query.lastError().text();
    }
    return items;
  }

  if (!trimmed.isEmpty()) {
    const QString like = "%" + trimmed + "%";
    for (int i = 0; i < 8; ++i) {
      query.addBindValue(like);
    }
  }
  if (!trimmedTag.isEmpty() && trimmedTag != "__all__" && trimmedTag != "__none__") {
    query.addBindValue("%" + trimmedTag + "%");
  }
  if (!trimmedCollection.isEmpty() && trimmedCollection != "__all__" && trimmedCollection != "__none__") {
    query.addBindValue(trimmedCollection);
  }

  if (query.exec()) {
    while (query.next()) {
      LibraryItem item;
      item.id = query.value(0).toInt();
      item.title = query.value(1).toString();
      item.authors = query.value(2).toString();
      item.series = query.value(3).toString();
      item.publisher = query.value(4).toString();
      item.description = query.value(5).toString();
      item.tags = query.value(6).toString();
      item.collection = query.value(7).toString();
      item.coverPath = query.value(8).toString();
      item.path = query.value(9).toString();
      item.format = query.value(10).toString();
      item.fileHash = query.value(11).toString();
      item.addedAt = query.value(12).toString();
      item.updatedAt = query.value(13).toString();
      item.annotationCount = query.value(14).toInt();
      items.push_back(std::move(item));
    }
  } else if (error) {
    *error = query.lastError().text();
  }
  return items;
}

QVector<AnnotationItem> DbWorker::fetchAnnotations(int libraryItemId, QString *error) {
  QVector<AnnotationItem> items;
  if (!m_db.isOpen()) {
    if (error) {
      *error = "Database not open";
    }
    return items;
  }
  if (libraryItemId <= 0) {
    return items;
  }
  QSqlQuery query(m_db);
  query.prepare("SELECT id, library_item_id, locator, type, text, color, created_at "
                "FROM annotations WHERE library_item_id = ? ORDER BY created_at");
  query.addBindValue(libraryItemId);
  if (query.exec()) {
    while (query.next()) {
      AnnotationItem item;
      item.id = query.value(0).toInt();
      item.libraryItemId = query.value(1).toInt();
      item.locator = query.value(2).toString();
      item.type = query.value(3).toString();
      item.text = query.value(4).toString();
      item.color = query.value(5).toString();
      item.createdAt = query.value(6).toString();
      items.push_back(std::move(item));
    }
  } else if (error) {
    *error = query.lastError().text();
  }
  return items;
}

bool DbWorker::insertLibraryItem(const LibraryItem &item, QString *error) {
  QSqlQuery query(m_db);
  query.prepare(
      "INSERT OR IGNORE INTO library_items "
      "(title, authors, series, publisher, description, tags, collection, cover_path, path, format, file_hash, added_at, updated_at) "
      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
  query.addBindValue(item.title);
  query.addBindValue(item.authors);
  query.addBindValue(item.series);
  query.addBindValue(item.publisher);
  query.addBindValue(item.description);
  query.addBindValue(item.tags);
  query.addBindValue(item.collection);
  query.addBindValue(item.coverPath);
  query.addBindValue(item.path);
  query.addBindValue(item.format);
  query.addBindValue(item.fileHash);
  query.addBindValue(item.addedAt);
  query.addBindValue(item.updatedAt.isEmpty() ? item.addedAt : item.updatedAt);
  if (!query.exec()) {
    if (error) {
      *error = query.lastError().text();
    }
    return false;
  }
  return true;
}

LibraryItem DbWorker::makeItemFromFile(const QString &filePath) {
  const QFileInfo info(filePath);
  LibraryItem item;
  item.title = info.completeBaseName();
  item.authors = "";
  item.series = "";
  item.publisher = "";
  item.description = "";
  item.tags = "";
  item.collection = "";
  item.coverPath = "";
  item.path = info.absoluteFilePath();
  item.format = info.suffix().toLower();
  item.fileHash = computeFileHash(filePath);
  item.addedAt = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
  item.updatedAt = item.addedAt;
  const QString ext = item.format;
  const bool wantsMetadata =
      (ext == "mobi" || ext == "azw" || ext == "azw3" || ext == "azw4" || ext == "prc" ||
       ext == "fb2" || ext == "epub");
  if (wantsMetadata) {
    auto registry = FormatRegistry::createDefault();
    QString metaError;
    auto doc = registry->open(item.path, &metaError);
    if (doc) {
      const QString docTitle = doc->title().trimmed();
      if (!docTitle.isEmpty()) {
        item.title = docTitle;
      }
      item.authors = doc->authors().trimmed();
      item.series = doc->series().trimmed();
      item.publisher = doc->publisher().trimmed();
      item.description = doc->description().trimmed();
      const QString coverPath = doc->coverPath();
      const QString cachedCover = cacheCoverImage(coverPath, item.fileHash);
      if (!cachedCover.isEmpty()) {
        item.coverPath = cachedCover;
      }
    }
  }
  return item;
}

QString DbWorker::computeFileHash(const QString &filePath) {
  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly)) {
    return "";
  }
  QCryptographicHash hash(QCryptographicHash::Sha256);
  constexpr qint64 kChunkSize = 1 << 20;
  while (!file.atEnd()) {
    hash.addData(file.read(kChunkSize));
  }
  return hash.result().toHex();
}

void *DbWorker::sqliteHandle() const {
  if (!m_db.isOpen()) {
    return nullptr;
  }
  QVariant handle = m_db.driver()->handle();
  if (!handle.isValid()) {
    QSqlQuery ping(m_db);
    ping.exec("SELECT 1");
    handle = m_db.driver()->handle();
  }
  if (!handle.isValid()) {
    return nullptr;
  }
  return handle.value<void *>();
}

bool DbWorker::deserializeToMemory(const QByteArray &dbBytes, QString *error) {
  if (dbBytes.isEmpty()) {
    if (error) {
      *error = "Decrypted database is empty";
    }
    return false;
  }

  QTemporaryFile temp(QDir(AppPaths::dataRoot()).filePath("vault-import-XXXXXX.db"));
  temp.setAutoRemove(false);
  if (!temp.open()) {
    if (error) {
      *error = "Failed to create temp SQLite file";
    }
    return false;
  }
  const QString tempPath = temp.fileName();
  temp.write(dbBytes);
  temp.flush();
  temp.close();

  QString attachError;
  const bool attachOk = attachDatabase(tempPath, "vault_import", &attachError);
  if (!attachOk) {
    if (error) {
      *error = attachError.isEmpty() ? "Failed to attach temp vault" : attachError;
    }
    QFile::remove(tempPath);
    return false;
  }

  QSqlQuery query(m_db);
  bool ok = true;
  ok = ok && query.exec("PRAGMA foreign_keys=OFF");
  ok = ok && query.exec("BEGIN IMMEDIATE");
  ok = ok && query.exec("DROP TABLE IF EXISTS library_items");
  ok = ok && query.exec("DROP TABLE IF EXISTS annotations");
  ok = ok && query.exec("CREATE TABLE library_items AS SELECT * FROM vault_import.library_items");
  ok = ok && query.exec("CREATE TABLE annotations AS SELECT * FROM vault_import.annotations");
  ok = ok && query.exec("COMMIT");

  QString lastError;
  if (!ok) {
    lastError = query.lastError().text();
    query.exec("ROLLBACK");
  }

  detachDatabase("vault_import");
  QFile::remove(tempPath);

  if (!ok) {
    if (error) {
      *error = lastError.isEmpty() ? "Failed to import vault" : lastError;
    }
    return false;
  }
  return true;
}

QByteArray DbWorker::serializeFromMemory(QString *error) {
  sqlite3 *db = static_cast<sqlite3 *>(sqliteHandle());
  if (!db) {
    QTemporaryFile temp(QDir(AppPaths::dataRoot()).filePath("vault-export-XXXXXX.db"));
    temp.setAutoRemove(false);
    if (!temp.open()) {
      if (error) {
        *error = "SQLite handle missing and temp file could not be created";
      }
      return {};
    }
    const QString tempPath = temp.fileName();
    temp.close();
    QSqlQuery vacuum(m_db);
    QString escapedPath = tempPath;
    escapedPath.replace("'", "''");
    const QString sql = QString("VACUUM INTO '%1'").arg(escapedPath);
    if (!vacuum.exec(sql)) {
      if (error) {
        *error = QString("SQLite handle missing and VACUUM INTO failed: %1").arg(vacuum.lastError().text());
      }
      QFile::remove(tempPath);
      return {};
    }
    QFile file(tempPath);
    if (!file.open(QIODevice::ReadOnly)) {
      if (error) {
        *error = "Failed to read temp SQLite export";
      }
      QFile::remove(tempPath);
      return {};
    }
    const QByteArray data = file.readAll();
    file.close();
    QFile::remove(tempPath);
    return data;
  }
  sqlite3_int64 size = 0;
  unsigned char *data = sqlite3_serialize(db, "main", &size, 0);
  if (!data || size <= 0) {
    if (error) {
      *error = "SQLite serialize failed";
    }
    return {};
  }
  QByteArray out(reinterpret_cast<const char *>(data), static_cast<int>(size));
  sqlite3_free(data);
  return out;
}
