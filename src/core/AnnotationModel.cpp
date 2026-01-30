#include "include/AnnotationModel.h"

#include <QFile>
#include <QFileInfo>
#include <QMetaObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

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
            emit countChanged();
            m_revision++;
            emit revisionChanged();
          });
  connect(worker, &DbWorker::addAnnotationFinished, this,
          [this](bool ok, const QString &error) {
            if (!ok) {
              setLastError(error);
            } else {
              setLastError("");
            }
          });
  connect(worker, &DbWorker::updateAnnotationFinished, this,
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

int AnnotationModel::count() const { return m_items.size(); }

int AnnotationModel::revision() const { return m_revision; }

QVariantMap AnnotationModel::get(int index) const {
  if (index < 0 || index >= m_items.size()) {
    return {};
  }
  const auto &item = m_items.at(index);
  return {
      {"id", item.id},
      {"libraryItemId", item.libraryItemId},
      {"locator", item.locator},
      {"type", item.type},
      {"text", item.text},
      {"color", item.color},
      {"createdAt", item.createdAt},
  };
}

QVariantList AnnotationModel::highlightRangesForChapter(int chapterIndex) const {
  QVariantList ranges;
  if (chapterIndex < 0) {
    return ranges;
  }
  const QRegularExpression re("^hl:c=(\\d+);s=(\\d+);e=(\\d+)");
  for (const auto &item : m_items) {
    if (item.type != "highlight") {
      continue;
    }
    const auto match = re.match(item.locator);
    if (!match.hasMatch()) {
      continue;
    }
    const int chapter = match.captured(1).toInt() - 1;
    const int start = match.captured(2).toInt();
    const int end = match.captured(3).toInt();
    if (chapter != chapterIndex || start >= end) {
      continue;
    }
    QVariantMap entry;
    entry["start"] = start;
    entry["end"] = end;
    entry["color"] = item.color;
    ranges.append(entry);
  }
  return ranges;
}

QVariantList AnnotationModel::anchorsForPage(int pageIndex) const {
  QVariantList anchors;
  if (pageIndex < 0) {
    return anchors;
  }
  const QRegularExpression re("^pos:p=(\\d+);x=([0-9.]+);y=([0-9.]+)(?:;w=([0-9.]+);h=([0-9.]+))?");
  for (const auto &item : m_items) {
    const auto match = re.match(item.locator);
    if (!match.hasMatch()) {
      continue;
    }
    const int page = match.captured(1).toInt() - 1;
    if (page != pageIndex) {
      continue;
    }
    const double x = match.captured(2).toDouble();
    const double y = match.captured(3).toDouble();
    const double w = match.captured(4).isEmpty() ? 0.0 : match.captured(4).toDouble();
    const double h = match.captured(5).isEmpty() ? 0.0 : match.captured(5).toDouble();
    QVariantMap entry;
    entry["id"] = item.id;
    entry["locator"] = item.locator;
    entry["type"] = item.type;
    entry["text"] = item.text;
    entry["color"] = item.color;
    entry["x"] = x;
    entry["y"] = y;
    entry["w"] = w;
    entry["h"] = h;
    anchors.append(entry);
  }
  return anchors;
}

bool AnnotationModel::exportAnnotations(const QString &path) {
  if (path.isEmpty()) {
    setLastError("Export path is empty");
    return false;
  }
  QFile out(path);
  if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    setLastError("Failed to write export file");
    return false;
  }
  const QString ext = QFileInfo(path).suffix().toLower();
  if (ext == "csv") {
    auto escape = [](const QString &value) {
      QString v = value;
      v.replace("\"", "\"\"");
      return QString("\"%1\"").arg(v);
    };
    QByteArray content;
    content.append("locator,type,text,color,created_at\n");
    for (const auto &item : m_items) {
      content.append(escape(item.locator).toUtf8());
      content.append(",");
      content.append(escape(item.type).toUtf8());
      content.append(",");
      content.append(escape(item.text).toUtf8());
      content.append(",");
      content.append(escape(item.color).toUtf8());
      content.append(",");
      content.append(escape(item.createdAt).toUtf8());
      content.append("\n");
    }
    out.write(content);
  } else if (ext == "md" || ext == "markdown") {
    QString content;
    content.append("# Annotations\n\n");
    for (const auto &item : m_items) {
      content.append("- **").append(item.type).append("** ");
      content.append("(").append(item.locator).append(")\n\n");
      if (!item.text.isEmpty()) {
        content.append("  ").append(item.text).append("\n\n");
      }
      if (!item.createdAt.isEmpty()) {
        content.append("  _").append(item.createdAt).append("_\n\n");
      }
    }
    out.write(content.toUtf8());
  } else {
    QJsonArray array;
    for (const auto &item : m_items) {
      QJsonObject obj;
      obj["locator"] = item.locator;
      obj["type"] = item.type;
      obj["text"] = item.text;
      obj["color"] = item.color;
      obj["createdAt"] = item.createdAt;
      array.append(obj);
    }
    QJsonDocument doc(array);
    out.write(doc.toJson(QJsonDocument::Indented));
  }
  out.close();
  setLastError("");
  return true;
}

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

bool AnnotationModel::updateAnnotation(int id,
                                       const QString &locator,
                                       const QString &type,
                                       const QString &text,
                                       const QString &color) {
  if (m_libraryItemId <= 0) {
    setLastError("No book selected");
    return false;
  }
  if (id <= 0) {
    setLastError("No annotation selected");
    return false;
  }
  QMetaObject::invokeMethod(dbWorker(), "updateAnnotation", Qt::QueuedConnection,
                            Q_ARG(int, id),
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
  emit countChanged();
  m_revision++;
  emit revisionChanged();
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
