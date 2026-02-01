#include "include/DbWorker.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QMetaType>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlDriver>
#include <QStandardPaths>
#include <QThread>
#include <QVariant>
#include <QUrl>

#include <sqlite3.h>

#include "FormatRegistry.h"
#include "CryptoBackend.h"
#include "CryptoVault.h"

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
  const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QDir dir(base);
  if (!dir.exists()) {
    dir.mkpath(".");
  }
  return dir.filePath("library.db");
}

QString coverCacheDir() {
  const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
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
    emit openFinished(false, error);
    return;
  }
  if (!ensureSchema(&error)) {
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

void DbWorker::saveEncryptedVault(const QString &vaultPath, const QString &passphrase) {
  if (vaultPath.isEmpty()) {
    emit saveFinished(false, "Vault path is empty");
    return;
  }
  QString error;
  const QByteArray dbBytes = serializeFromMemory(&error);
  if (dbBytes.isEmpty()) {
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
      "tags = ?, collection = ? WHERE id = ?");
  query.addBindValue(title);
  query.addBindValue(authors);
  query.addBindValue(series);
  query.addBindValue(publisher);
  query.addBindValue(description);
  query.addBindValue(tags);
  query.addBindValue(collection);
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
    if (updateTags && updateCollection) {
      query.prepare("UPDATE library_items SET tags = ?, collection = ? WHERE id = ?");
      query.addBindValue(tags);
      query.addBindValue(collection);
      query.addBindValue(id);
    } else if (updateTags) {
      query.prepare("UPDATE library_items SET tags = ? WHERE id = ?");
      query.addBindValue(tags);
      query.addBindValue(id);
    } else {
      query.prepare("UPDATE library_items SET collection = ? WHERE id = ?");
      query.addBindValue(collection);
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
  query.prepare("INSERT INTO annotations (library_item_id, locator, type, text, color, created_at) "
                "VALUES (?, ?, ?, ?, ?, ?)");
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

bool DbWorker::openDatabase(const QString &dbPath, QString *error) {
  if (m_db.isOpen()) {
    m_db.close();
  }
  if (!m_connectionName.isEmpty()) {
    QSqlDatabase::removeDatabase(m_connectionName);
  }
  m_connectionName = QString("library_worker_%1").arg(reinterpret_cast<quintptr>(this));
  m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
  m_db.setDatabaseName(dbPath.isEmpty() ? defaultDbPath() : dbPath);
  if (!m_db.open()) {
    if (error) {
      *error = m_db.lastError().text();
    }
    return false;
  }
  return ensureSchema(error);
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
          "added_at TEXT"
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
  return true;
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
      "SELECT id, title, authors, series, publisher, description, tags, collection, cover_path, path, format, file_hash, added_at, "
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
      item.annotationCount = query.value(13).toInt();
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
      "(title, authors, series, publisher, description, tags, collection, cover_path, path, format, file_hash, added_at) "
      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
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
    return nullptr;
  }
  return handle.value<void *>();
}

bool DbWorker::deserializeToMemory(const QByteArray &dbBytes, QString *error) {
  sqlite3 *db = static_cast<sqlite3 *>(sqliteHandle());
  if (!db) {
    if (error) {
      *error = "SQLite handle missing";
    }
    return false;
  }
  if (dbBytes.isEmpty()) {
    if (error) {
      *error = "Decrypted database is empty";
    }
    return false;
  }
  unsigned char *buffer = static_cast<unsigned char *>(sqlite3_malloc(dbBytes.size()));
  if (!buffer) {
    if (error) {
      *error = "Memory allocation failed";
    }
    return false;
  }
  memcpy(buffer, dbBytes.constData(), static_cast<size_t>(dbBytes.size()));
  const int rc = sqlite3_deserialize(db, "main", buffer, dbBytes.size(), dbBytes.size(),
                                     SQLITE_DESERIALIZE_FREEONCLOSE);
  if (rc != SQLITE_OK) {
    if (error) {
      *error = QString("SQLite deserialize failed (%1)").arg(rc);
    }
    return false;
  }
  return true;
}

QByteArray DbWorker::serializeFromMemory(QString *error) {
  sqlite3 *db = static_cast<sqlite3 *>(sqliteHandle());
  if (!db) {
    if (error) {
      *error = "SQLite handle missing";
    }
    return {};
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
