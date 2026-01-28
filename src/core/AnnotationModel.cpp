#include "include/AnnotationModel.h"

#include <QMetaObject>

#include "DbWorker.h"

AnnotationModel::AnnotationModel(QObject *parent) : QAbstractListModel(parent) {
  auto *worker = dbWorker();
  connect(worker, &DbWorker::annotationsLoaded, this,
          [this](int libraryItemId, const QVector<AnnotationItem> &items) {
            if (libraryItemId != m_libraryItemId) {
              return;
            }
            beginResetModel();
            m_items = items;
            endResetModel();
          });
  connect(worker, &DbWorker::addAnnotationFinished, this,
          [this](bool ok, const QString &error) {
            if (!ok) {
              setLastError(error);
            } else {
              setLastError("");
            }
          });
  connect(worker, &DbWorker::deleteAnnotationFinished, this,
          [this](bool ok, const QString &error) {
            if (!ok) {
              setLastError(error);
            } else {
              setLastError("");
            }
          });
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
  if (m_libraryItemId <= 0) {
    setLastError("No book selected");
    return false;
  }
  QMetaObject::invokeMethod(dbWorker(), "addAnnotation", Qt::QueuedConnection,
                            Q_ARG(int, m_libraryItemId),
                            Q_ARG(QString, locator),
                            Q_ARG(QString, type),
                            Q_ARG(QString, text),
                            Q_ARG(QString, color));
  return true;
}

bool AnnotationModel::deleteAnnotation(int id) {
  if (m_libraryItemId <= 0) {
    setLastError("No book selected");
    return false;
  }
  QMetaObject::invokeMethod(dbWorker(), "deleteAnnotation", Qt::QueuedConnection,
                            Q_ARG(int, id),
                            Q_ARG(int, m_libraryItemId));
  return true;
}

bool AnnotationModel::attachDatabase(const QString &dbPath) {
  Q_UNUSED(dbPath)
  // No-op: database access handled by worker thread.
  setLastError("");
  return true;
}

bool AnnotationModel::attachConnection(const QString &connectionName) {
  Q_UNUSED(connectionName)
  // No-op: database access handled by worker thread.
  setLastError("");
  return true;
}

void AnnotationModel::reload() {
  beginResetModel();
  m_items.clear();
  endResetModel();
  if (m_libraryItemId > 0) {
    QMetaObject::invokeMethod(dbWorker(), "loadAnnotations", Qt::QueuedConnection,
                              Q_ARG(int, m_libraryItemId));
  }
}

void AnnotationModel::setLastError(const QString &error) {
  if (m_lastError == error) {
    return;
  }
  m_lastError = error;
  emit lastErrorChanged();
}
