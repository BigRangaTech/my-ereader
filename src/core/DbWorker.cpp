#include "include/DbWorker.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMetaType>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlDriver>
#include <QStandardPaths>
#include <QThread>
#include <QVariant>

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
} // namespace

DbWorker::DbWorker(QObject *parent) : QObject(parent) {
  qRegisterMetaType<LibraryItem>("LibraryItem");
  qRegisterMetaType<QVector<LibraryItem>>("QVector<LibraryItem>");
  qRegisterMetaType<AnnotationItem>("AnnotationItem");
  qRegisterMetaType<QVector<AnnotationItem>>("QVector<AnnotationItem>");
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
  const QVector<LibraryItem> items = fetchLibrary(&error);
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
  const QVector<LibraryItem> items = fetchLibrary(&error);
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
  emit libraryLoaded(fetchLibrary(&error));
}

void DbWorker::loadLibrary() {
  QString error;
  const QVector<LibraryItem> items = fetchLibrary(&error);
  emit libraryLoaded(items);
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
  QVector<LibraryItem> items;
  if (!m_db.isOpen()) {
    if (error) {
      *error = "Database not open";
    }
    return items;
  }
  QSqlQuery query(m_db);
  if (query.exec("SELECT id, title, authors, series, publisher, description, path, format, file_hash, added_at "
                 "FROM library_items ORDER BY title")) {
    while (query.next()) {
      LibraryItem item;
      item.id = query.value(0).toInt();
      item.title = query.value(1).toString();
      item.authors = query.value(2).toString();
      item.series = query.value(3).toString();
      item.publisher = query.value(4).toString();
      item.description = query.value(5).toString();
      item.path = query.value(6).toString();
      item.format = query.value(7).toString();
      item.fileHash = query.value(8).toString();
      item.addedAt = query.value(9).toString();
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
      "(title, authors, series, publisher, description, path, format, file_hash, added_at) "
      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");
  query.addBindValue(item.title);
  query.addBindValue(item.authors);
  query.addBindValue(item.series);
  query.addBindValue(item.publisher);
  query.addBindValue(item.description);
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
  item.path = info.absoluteFilePath();
  item.format = info.suffix().toLower();
  item.fileHash = computeFileHash(filePath);
  item.addedAt = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
  const QString ext = item.format;
  const bool wantsMetadata =
      (ext == "mobi" || ext == "azw" || ext == "azw3" || ext == "azw4" || ext == "prc" ||
       ext == "fb2");
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
