#include "include/AnnotationModel.h"

#include <QDateTime>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QDir>

AnnotationModel::AnnotationModel(QObject *parent) : QAbstractListModel(parent) {
  openDatabase();
}

int AnnotationModel::rowCount(const QModelIndex &parent) const {
  if (parent.isValid()) {
    return 0;
  }
  return m_items.size();
}

QVariant AnnotationModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size()) {
    return {};
  }

  const auto &item = m_items.at(index.row());
  switch (role) {
  case IdRole:
    return item.id;
  case LibraryItemIdRole:
    return item.libraryItemId;
  case LocatorRole:
    return item.locator;
  case TypeRole:
    return item.type;
  case TextRole:
    return item.text;
  case ColorRole:
    return item.color;
  case CreatedAtRole:
    return item.createdAt;
  default:
    return {};
  }
}

QHash<int, QByteArray> AnnotationModel::roleNames() const {
  return {
      {IdRole, "id"},         {LibraryItemIdRole, "libraryItemId"},
      {LocatorRole, "locator"}, {TypeRole, "type"},
      {TextRole, "text"},     {ColorRole, "color"},
      {CreatedAtRole, "createdAt"}};
}

int AnnotationModel::libraryItemId() const { return m_libraryItemId; }

void AnnotationModel::setLibraryItemId(int id) {
  if (m_libraryItemId == id) {
    return;
  }
  m_libraryItemId = id;
  emit libraryItemIdChanged();
  reload();
}

QString AnnotationModel::lastError() const { return m_lastError; }

bool AnnotationModel::addAnnotation(const QString &locator,
                                    const QString &type,
                                    const QString &text,
                                    const QString &color) {
  if (!m_db.isOpen()) {
    setLastError("Database not open");
    return false;
  }
  if (m_libraryItemId <= 0) {
    setLastError("No book selected");
    return false;
  }

  QSqlQuery query(m_db);
  query.prepare("INSERT INTO annotations (library_item_id, locator, type, text, color, created_at) "
                "VALUES (?, ?, ?, ?, ?, ?)");
  query.addBindValue(m_libraryItemId);
  query.addBindValue(locator);
  query.addBindValue(type);
  query.addBindValue(text);
  query.addBindValue(color);
  query.addBindValue(QDateTime::currentDateTimeUtc().toString(Qt::ISODate));

  if (!query.exec()) {
    setLastError(query.lastError().text());
    return false;
  }

  reload();
  setLastError("");
  return true;
}

bool AnnotationModel::deleteAnnotation(int id) {
  if (!m_db.isOpen()) {
    setLastError("Database not open");
    return false;
  }

  QSqlQuery query(m_db);
  query.prepare("DELETE FROM annotations WHERE id = ?");
  query.addBindValue(id);
  if (!query.exec()) {
    setLastError(query.lastError().text());
    return false;
  }
  reload();
  setLastError("");
  return true;
}

bool AnnotationModel::openDatabase() {
  m_connectionName = QString("annotations_%1").arg(reinterpret_cast<quintptr>(this));
  m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
  return true;
}

bool AnnotationModel::attachDatabase(const QString &dbPath) {
  if (!openDatabase()) {
    setLastError("Failed to init database connection");
    return false;
  }
  if (dbPath.isEmpty()) {
    setLastError("Database path empty");
    return false;
  }
  m_db.setDatabaseName(dbPath);
  if (!m_db.open()) {
    setLastError(m_db.lastError().text());
    return false;
  }
  reload();
  return true;
}

bool AnnotationModel::attachConnection(const QString &connectionName) {
  if (connectionName.isEmpty()) {
    setLastError("Connection name empty");
    return false;
  }
  if (!QSqlDatabase::contains(connectionName)) {
    setLastError("Connection not found");
    return false;
  }
  m_db = QSqlDatabase::database(connectionName);
  if (!m_db.isOpen()) {
    if (!m_db.open()) {
      setLastError(m_db.lastError().text());
      return false;
    }
  }
  reload();
  return true;
}

void AnnotationModel::reload() {
  beginResetModel();
  m_items.clear();

  if (!m_db.isOpen() || m_libraryItemId <= 0) {
    endResetModel();
    return;
  }

  QSqlQuery query(m_db);
  query.prepare("SELECT id, library_item_id, locator, type, text, color, created_at "
                "FROM annotations WHERE library_item_id = ? ORDER BY created_at");
  query.addBindValue(m_libraryItemId);

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
      m_items.push_back(std::move(item));
    }
  }

  endResetModel();
}

void AnnotationModel::setLastError(const QString &error) {
  if (m_lastError == error) {
    return;
  }
  m_lastError = error;
  emit lastErrorChanged();
}
