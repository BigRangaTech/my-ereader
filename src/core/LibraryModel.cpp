#include "include/LibraryModel.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>

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
    return false;
  }

  QDir dir(baseDir);
  if (!dir.exists() && !dir.mkpath(".")) {
    setLastError("Failed to create app data directory");
    return false;
  }

  const QString dbPath = dir.filePath("library.db");
  return openDatabase(dbPath);
}

bool LibraryModel::addBook(const QString &filePath) {
  if (!m_ready) {
    setLastError("Library database not ready");
    return false;
  }

  if (!QFileInfo::exists(filePath)) {
    setLastError("File does not exist");
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
    return false;
  }

  reload();
  setLastError("");
  return true;
}

bool LibraryModel::ready() const { return m_ready; }

int LibraryModel::count() const { return m_items.size(); }

QString LibraryModel::lastError() const { return m_lastError; }

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
