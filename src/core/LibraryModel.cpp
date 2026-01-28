#include "include/LibraryModel.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlDriver>
#include <QStandardPaths>

#include <cstring>

#include <sqlite3.h>

#include "CryptoBackend.h"
#include "CryptoVault.h"

LibraryModel::LibraryModel(QObject *parent) : QAbstractListModel(parent) {}

LibraryModel::~LibraryModel() {
  if (m_db.isOpen()) {
    m_db.close();
  }
  if (!m_connectionName.isEmpty()) {
    m_db = QSqlDatabase();
    QSqlDatabase::removeDatabase(m_connectionName);
  }
}

int LibraryModel::rowCount(const QModelIndex &parent) const {
  if (parent.isValid()) {
    return 0;
  }
  return m_items.size();
}

QVariant LibraryModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size()) {
    return {};
  }

  const auto &item = m_items.at(index.row());
  switch (role) {
  case IdRole:
    return item.id;
  case TitleRole:
    return item.title;
  case AuthorsRole:
    return item.authors;
  case PathRole:
    return item.path;
  case FormatRole:
    return item.format;
  case AddedAtRole:
    return item.addedAt;
  default:
    return {};
  }
}

QHash<int, QByteArray> LibraryModel::roleNames() const {
  return {
      {IdRole, "id"},        {TitleRole, "title"},
      {AuthorsRole, "authors"}, {PathRole, "path"},
      {FormatRole, "format"},   {AddedAtRole, "addedAt"}};
}

bool LibraryModel::openDefault() {
  const QString baseDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  if (baseDir.isEmpty()) {
    setLastError("No writable AppDataLocation available");
    qWarning() << "LibraryModel: no AppDataLocation";
    return false;
  }

  QDir dir(baseDir);
  if (!dir.exists() && !dir.mkpath(".")) {
    setLastError("Failed to create app data directory");
    qWarning() << "LibraryModel: failed to create app data directory" << baseDir;
    return false;
  }

  const QString dbPath = dir.filePath("library.db");
  qInfo() << "LibraryModel: using db" << dbPath;
  return openDatabase(dbPath);
}

bool LibraryModel::openAt(const QString &dbPath) {
  if (dbPath.isEmpty()) {
    setLastError("Database path is empty");
    qWarning() << "LibraryModel: db path empty";
    return false;
  }
  return openDatabase(dbPath);
}

bool LibraryModel::openEncryptedVault(const QString &vaultPath, const QString &passphrase) {
  if (vaultPath.isEmpty()) {
    setLastError("Vault path is empty");
    return false;
  }
  auto backend = CryptoBackendFactory::createDefault();
  CryptoVault vault(std::move(backend));
  QString error;
  QByteArray dbBytes;
  if (!vault.decryptToBytes(vaultPath, passphrase, &dbBytes, &error)) {
    setLastError(error.isEmpty() ? "Failed to decrypt vault" : error);
    qWarning() << "LibraryModel: decrypt failed" << m_lastError;
    return false;
  }

  if (!openInMemory()) {
    return false;
  }
  if (!deserializeToMemory(dbBytes)) {
    setLastError("Failed to load decrypted database");
    return false;
  }
  if (!ensureSchema()) {
    return false;
  }
  reload();
  setLastError("");
  qInfo() << "LibraryModel: in-memory DB ready";
  qInfo() << "LibraryModel: opened encrypted vault in memory";
  return true;
}

bool LibraryModel::saveEncryptedVault(const QString &vaultPath, const QString &passphrase) {
  if (vaultPath.isEmpty()) {
    setLastError("Vault path is empty");
    return false;
  }
  if (!m_db.isOpen()) {
    setLastError("Database not open");
    return false;
  }
  const QByteArray dbBytes = serializeFromMemory();
  if (dbBytes.isEmpty()) {
    setLastError("Failed to serialize database");
    return false;
  }
  auto backend = CryptoBackendFactory::createDefault();
  CryptoVault vault(std::move(backend));
  QString error;
  if (!vault.encryptFromBytes(vaultPath, passphrase, dbBytes, &error)) {
    setLastError(error.isEmpty() ? "Failed to encrypt vault" : error);
    return false;
  }
  qInfo() << "LibraryModel: saved encrypted vault";
  return true;
}

bool LibraryModel::addBook(const QString &filePath) {
  if (!m_ready) {
    setLastError("Library database not ready");
    qWarning() << "LibraryModel: addBook called before ready";
    return false;
  }

  if (!QFileInfo::exists(filePath)) {
    setLastError("File does not exist");
    qWarning() << "LibraryModel: file not found" << filePath;
    return false;
  }

  const LibraryItem item = makeItemFromFile(filePath);

  QSqlQuery query(m_db);
  query.prepare(
      "INSERT OR IGNORE INTO library_items (title, authors, path, format, file_hash, added_at) "
      "VALUES (?, ?, ?, ?, ?, ?)");
  query.addBindValue(item.title);
  query.addBindValue(item.authors);
  query.addBindValue(item.path);
  query.addBindValue(item.format);
  query.addBindValue(item.fileHash);
  query.addBindValue(item.addedAt);

  if (!query.exec()) {
    setLastError(query.lastError().text());
    qWarning() << "LibraryModel: insert failed" << query.lastError().text();
    return false;
  }

  reload();
  setLastError("");
  qInfo() << "LibraryModel: added book" << item.title;
  return true;
}

bool LibraryModel::ready() const { return m_ready; }

int LibraryModel::count() const { return m_items.size(); }

QString LibraryModel::lastError() const { return m_lastError; }

QString LibraryModel::connectionName() const { return m_connectionName; }

bool LibraryModel::openDatabase(const QString &dbPath) {
  m_connectionName = QString("library_%1").arg(reinterpret_cast<quintptr>(this));
  m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
  m_db.setDatabaseName(dbPath);

  if (!m_db.open()) {
    setLastError(m_db.lastError().text());
    return false;
  }

  if (!ensureSchema()) {
    return false;
  }

  reload();
  m_ready = true;
  setLastError("");
  emit readyChanged();
  return true;
}

bool LibraryModel::ensureSchema() {
  QSqlQuery query(m_db);
  if (!query.exec(
          "CREATE TABLE IF NOT EXISTS library_items ("
          "id INTEGER PRIMARY KEY AUTOINCREMENT,"
          "title TEXT,"
          "authors TEXT,"
          "path TEXT UNIQUE,"
          "format TEXT,"
          "file_hash TEXT,"
          "added_at TEXT"
          ")")) {
    setLastError(query.lastError().text());
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
    setLastError(query.lastError().text());
    return false;
  }
  return true;
}

void LibraryModel::reload() {
  beginResetModel();
  m_items.clear();

  QSqlQuery query(m_db);
  if (query.exec("SELECT id, title, authors, path, format, file_hash, added_at FROM library_items ORDER BY title")) {
    while (query.next()) {
      LibraryItem item;
      item.id = query.value(0).toInt();
      item.title = query.value(1).toString();
      item.authors = query.value(2).toString();
      item.path = query.value(3).toString();
      item.format = query.value(4).toString();
      item.fileHash = query.value(5).toString();
      item.addedAt = query.value(6).toString();
      m_items.push_back(std::move(item));
    }
  }

  endResetModel();
  emit countChanged();
}

void LibraryModel::close() {
  if (!m_db.isOpen()) {
    return;
  }
  m_db.close();
  if (!m_connectionName.isEmpty()) {
    m_db = QSqlDatabase();
    QSqlDatabase::removeDatabase(m_connectionName);
    m_connectionName.clear();
  }
  beginResetModel();
  m_items.clear();
  endResetModel();
  m_ready = false;
  emit readyChanged();
  emit countChanged();
}

bool LibraryModel::openInMemory() {
  return openDatabase(":memory:");
}

void *LibraryModel::sqliteHandle() const {
  if (!m_db.isOpen()) {
    return nullptr;
  }
  QVariant handle = m_db.driver()->handle();
  if (!handle.isValid()) {
    return nullptr;
  }
  return handle.value<void *>();
}

bool LibraryModel::deserializeToMemory(const QByteArray &dbBytes) {
  sqlite3 *db = static_cast<sqlite3 *>(sqliteHandle());
  if (!db) {
    qWarning() << "LibraryModel: sqlite handle missing";
    return false;
  }
  if (dbBytes.isEmpty()) {
    return false;
  }
  unsigned char *buffer = static_cast<unsigned char *>(sqlite3_malloc(dbBytes.size()));
  if (!buffer) {
    return false;
  }
  memcpy(buffer, dbBytes.constData(), static_cast<size_t>(dbBytes.size()));
  const int rc = sqlite3_deserialize(db, "main", buffer, dbBytes.size(), dbBytes.size(),
                                     SQLITE_DESERIALIZE_FREEONCLOSE);
  if (rc != SQLITE_OK) {
    qWarning() << "LibraryModel: sqlite deserialize failed" << rc;
    return false;
  }
  return true;
}

QByteArray LibraryModel::serializeFromMemory() {
  sqlite3 *db = static_cast<sqlite3 *>(sqliteHandle());
  if (!db) {
    return {};
  }
  sqlite3_int64 size = 0;
  unsigned char *data = sqlite3_serialize(db, "main", &size, 0);
  if (!data || size <= 0) {
    return {};
  }
  QByteArray out(reinterpret_cast<const char *>(data), static_cast<int>(size));
  sqlite3_free(data);
  return out;
}

LibraryItem LibraryModel::makeItemFromFile(const QString &filePath) {
  const QFileInfo info(filePath);
  LibraryItem item;
  item.title = info.completeBaseName();
  item.authors = "";
  item.path = info.absoluteFilePath();
  item.format = info.suffix().toLower();
  item.fileHash = computeFileHash(filePath);
  item.addedAt = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
  return item;
}

QString LibraryModel::computeFileHash(const QString &filePath) {
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

void LibraryModel::setLastError(const QString &error) {
  if (m_lastError == error) {
    return;
  }
  m_lastError = error;
  emit lastErrorChanged();
}
