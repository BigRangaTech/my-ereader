#include "include/LibraryModel.h"

#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QEventLoop>
#include <QMetaObject>
#include <QSet>
#include <QStandardPaths>
#include <algorithm>

#include "include/AppPaths.h"
#include "DbWorker.h"

LibraryModel::LibraryModel(QObject *parent) : QAbstractListModel(parent) {
  auto *worker = dbWorker();
  connect(worker, &DbWorker::openFinished, this,
          [this](bool ok, const QString &error) {
            if (!ok) {
              setLastError(error);
              m_ready = false;
              qWarning() << "LibraryModel: openFinished failed" << error;
            } else {
              setLastError("");
              m_ready = true;
              qInfo() << "LibraryModel: openFinished ok";
            }
            emit readyChanged();
          });
  connect(worker, &DbWorker::libraryLoaded, this,
          [this](const QVector<LibraryItem> &items, int totalCount) {
            const int newTotal = std::max(0, totalCount);
            const int newPageCount =
                (newTotal == 0 || m_pageSize <= 0) ? 1 : ((newTotal + m_pageSize - 1) / m_pageSize);
            if (m_pageIndex >= newPageCount) {
              const int newIndex = std::max(0, newPageCount - 1);
              if (m_pageIndex != newIndex) {
                m_pageIndex = newIndex;
                emit pageIndexChanged();
              }
              reload();
              return;
            }
            beginResetModel();
            m_items = items;
            endResetModel();
            if (m_totalCount != newTotal) {
              m_totalCount = newTotal;
              emit totalCountChanged();
            }
            if (m_pageCount != newPageCount) {
              m_pageCount = newPageCount;
              emit pageCountChanged();
            }
            emit countChanged();
          });
  connect(worker, &DbWorker::facetsLoaded, this,
          [this](const QStringList &collections, const QStringList &tags) {
            if (m_availableCollections != collections) {
              m_availableCollections = collections;
              emit availableCollectionsChanged();
            }
            if (m_availableTags != tags) {
              m_availableTags = tags;
              emit availableTagsChanged();
            }
          });
  connect(worker, &DbWorker::addBookFinished, this,
          [this](bool ok, const QString &error) {
            if (!ok) {
              setLastError(error);
            } else {
              setLastError("");
            }
            if (m_bulkImportActive) {
              m_bulkImportDone++;
              if (m_bulkImportDone >= m_bulkImportTotal) {
                m_bulkImportActive = false;
                m_bulkImportTotal = 0;
                m_bulkImportDone = 0;
              }
              emit bulkImportChanged();
            }
          });
  connect(worker, &DbWorker::updateBookFinished, this,
          [this](bool ok, const QString &error) {
            if (!ok) {
              setLastError(error);
            } else {
              setLastError("");
            }
          });
  connect(worker, &DbWorker::deleteBookFinished, this,
          [this](bool ok, const QString &error) {
            if (!ok) {
              setLastError(error);
            } else {
              setLastError("");
            }
          });
  connect(worker, &DbWorker::annotationCountChanged, this,
          [this](int libraryItemId, int count) {
            for (int i = 0; i < m_items.size(); ++i) {
              if (m_items.at(i).id == libraryItemId) {
                m_items[i].annotationCount = count;
                emit dataChanged(index(i), index(i), {AnnotationCountRole});
                break;
              }
            }
          });
}

LibraryModel::~LibraryModel() {
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
  case SeriesRole:
    return item.series;
  case PublisherRole:
    return item.publisher;
  case DescriptionRole:
    return item.description;
  case TagsRole:
    return item.tags;
  case CollectionRole:
    return item.collection;
  case CoverPathRole:
    return item.coverPath;
  case PathRole:
    return item.path;
  case FormatRole:
    return item.format;
  case AddedAtRole:
    return item.addedAt;
  case AnnotationCountRole:
    return item.annotationCount;
  default:
    return {};
  }
}

QHash<int, QByteArray> LibraryModel::roleNames() const {
  return {
      {IdRole, "id"},           {TitleRole, "title"},
      {AuthorsRole, "authors"}, {SeriesRole, "series"},
      {PublisherRole, "publisher"}, {DescriptionRole, "description"},
      {TagsRole, "tags"},       {CollectionRole, "collection"},
      {CoverPathRole, "coverPath"},
      {PathRole, "path"},       {FormatRole, "format"},
      {AddedAtRole, "addedAt"}, {AnnotationCountRole, "annotationCount"}};
}

bool LibraryModel::openDefault() {
  const QString baseDir = AppPaths::dataRoot();
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
  return openAt(dbPath);
}

bool LibraryModel::openAt(const QString &dbPath) {
  if (dbPath.isEmpty()) {
    setLastError("Database path is empty");
    qWarning() << "LibraryModel: db path empty";
    return false;
  }
  setLastError("");
  bool ok = false;
  QString error;
  QEventLoop loop;
  QMetaObject::Connection conn =
      connect(dbWorker(), &DbWorker::openFinished, this,
              [&](bool success, const QString &err) {
                ok = success;
                error = err;
                loop.quit();
              });
  QMetaObject::invokeMethod(dbWorker(), "openAt", Qt::QueuedConnection, Q_ARG(QString, dbPath));
  loop.exec();
  disconnect(conn);
  if (!ok) {
    setLastError(error);
  }
  return ok;
}

bool LibraryModel::openEncryptedVault(const QString &vaultPath, const QString &passphrase) {
  qInfo() << "LibraryModel: openEncryptedVault" << vaultPath;
  if (vaultPath.isEmpty()) {
    setLastError("Vault path is empty");
    return false;
  }
  setLastError("");

  bool ok = false;
  QString error;
  QEventLoop loop;
  QMetaObject::Connection conn =
      connect(dbWorker(), &DbWorker::openFinished, this,
              [&](bool success, const QString &err) {
                ok = success;
                error = err;
                loop.quit();
              });
  QMetaObject::invokeMethod(dbWorker(), "openEncryptedVault", Qt::QueuedConnection,
                            Q_ARG(QString, vaultPath),
                            Q_ARG(QString, passphrase));
  loop.exec();
  disconnect(conn);
  if (!ok) {
    setLastError(error);
    qWarning() << "LibraryModel: openEncryptedVault failed" << error;
  } else {
    qInfo() << "LibraryModel: openEncryptedVault ok";
  }
  return ok;
}

bool LibraryModel::saveEncryptedVault(const QString &vaultPath, const QString &passphrase) {
  if (vaultPath.isEmpty()) {
    setLastError("Vault path is empty");
    return false;
  }
  setLastError("");
  bool ok = false;
  QString error;
  QEventLoop loop;
  QMetaObject::Connection conn =
      connect(dbWorker(), &DbWorker::saveFinished, this,
              [&](bool success, const QString &err) {
                ok = success;
                error = err;
                loop.quit();
              });
  QMetaObject::invokeMethod(dbWorker(), "saveEncryptedVault", Qt::QueuedConnection,
                            Q_ARG(QString, vaultPath),
                            Q_ARG(QString, passphrase));
  loop.exec();
  disconnect(conn);
  if (!ok) {
    setLastError(error);
  }
  return ok;
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

  setLastError("");
  QMetaObject::invokeMethod(dbWorker(), "addBook", Qt::QueuedConnection, Q_ARG(QString, filePath));
  return true;
}

bool LibraryModel::addBooks(const QStringList &filePaths) {
  if (!m_ready) {
    setLastError("Library database not ready");
    return false;
  }
  if (filePaths.isEmpty()) {
    setLastError("No files selected");
    return false;
  }

  QSet<QString> uniquePaths;
  int queued = 0;
  for (const QString &path : filePaths) {
    const QString trimmed = path.trimmed();
    if (trimmed.isEmpty()) {
      continue;
    }
    const QString absolute = QFileInfo(trimmed).absoluteFilePath();
    if (uniquePaths.contains(absolute)) {
      continue;
    }
    uniquePaths.insert(absolute);
    if (!QFileInfo::exists(absolute)) {
      qWarning() << "LibraryModel: file not found" << absolute;
      continue;
    }
    QMetaObject::invokeMethod(dbWorker(), "addBook", Qt::QueuedConnection,
                              Q_ARG(QString, absolute));
    queued++;
  }

  if (queued == 0) {
    setLastError("No valid files to add");
    return false;
  }
  if (!m_bulkImportActive) {
    m_bulkImportDone = 0;
    m_bulkImportTotal = 0;
  }
  m_bulkImportActive = true;
  m_bulkImportTotal += queued;
  setLastError("");
  emit bulkImportChanged();
  qInfo() << "LibraryModel: queued" << queued << "book(s)";
  return true;
}

bool LibraryModel::addFolder(const QString &folderPath, bool recursive) {
  if (!m_ready) {
    setLastError("Library database not ready");
    return false;
  }
  const QString trimmed = folderPath.trimmed();
  if (trimmed.isEmpty()) {
    setLastError("Folder path is empty");
    return false;
  }
  const QDir dir(trimmed);
  if (!dir.exists()) {
    setLastError("Folder does not exist");
    return false;
  }

  const QStringList extensions = {
      "epub", "pdf", "mobi", "azw", "azw3", "azw4", "prc",
      "fb2", "cbz", "cbr", "djvu", "djv", "txt"
  };
  QStringList found;
  QDirIterator it(trimmed,
                  QDir::Files | QDir::NoDotAndDotDot,
                  recursive ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags);
  while (it.hasNext()) {
    const QString path = it.next();
    const QString ext = QFileInfo(path).suffix().toLower();
    if (extensions.contains(ext)) {
      found.append(path);
    }
  }

  if (found.isEmpty()) {
    setLastError("No supported books found in folder");
    return false;
  }
  return addBooks(found);
}

bool LibraryModel::updateMetadata(int id,
                                  const QString &title,
                                  const QString &authors,
                                  const QString &series,
                                  const QString &publisher,
                                  const QString &description,
                                  const QString &tags,
                                  const QString &collection) {
  if (!m_ready) {
    setLastError("Library database not ready");
    return false;
  }
  if (id <= 0) {
    setLastError("Invalid library item");
    return false;
  }
  setLastError("");
  QMetaObject::invokeMethod(dbWorker(), "updateLibraryItem", Qt::QueuedConnection,
                            Q_ARG(int, id),
                            Q_ARG(QString, title),
                            Q_ARG(QString, authors),
                            Q_ARG(QString, series),
                            Q_ARG(QString, publisher),
                            Q_ARG(QString, description),
                            Q_ARG(QString, tags),
                            Q_ARG(QString, collection));
  return true;
}

bool LibraryModel::removeBook(int id) {
  if (!m_ready) {
    setLastError("Library database not ready");
    return false;
  }
  if (id <= 0) {
    setLastError("Invalid library item");
    return false;
  }
  setLastError("");
  QMetaObject::invokeMethod(dbWorker(), "deleteLibraryItem", Qt::QueuedConnection,
                            Q_ARG(int, id));
  return true;
}

bool LibraryModel::updateTagsCollection(const QVariantList &ids,
                                        const QString &tags,
                                        const QString &collection,
                                        bool updateTags,
                                        bool updateCollection) {
  if (!m_ready) {
    setLastError("Library database not ready");
    return false;
  }
  if (ids.isEmpty()) {
    setLastError("No items selected");
    return false;
  }
  QVector<int> converted;
  converted.reserve(ids.size());
  for (const auto &value : ids) {
    bool ok = false;
    const int id = value.toInt(&ok);
    if (ok && id > 0) {
      converted.push_back(id);
    }
  }
  if (converted.isEmpty()) {
    setLastError("Invalid library items");
    return false;
  }
  setLastError("");
  QMetaObject::invokeMethod(dbWorker(), "bulkUpdateTagsCollection", Qt::QueuedConnection,
                            Q_ARG(QVector<int>, converted),
                            Q_ARG(QString, tags),
                            Q_ARG(QString, collection),
                            Q_ARG(bool, updateTags),
                            Q_ARG(bool, updateCollection));
  return true;
}

bool LibraryModel::removeBooks(const QVariantList &ids) {
  if (!m_ready) {
    setLastError("Library database not ready");
    return false;
  }
  if (ids.isEmpty()) {
    setLastError("No items selected");
    return false;
  }
  QVector<int> converted;
  converted.reserve(ids.size());
  for (const auto &value : ids) {
    bool ok = false;
    const int id = value.toInt(&ok);
    if (ok && id > 0) {
      converted.push_back(id);
    }
  }
  if (converted.isEmpty()) {
    setLastError("Invalid library items");
    return false;
  }
  setLastError("");
  QMetaObject::invokeMethod(dbWorker(), "deleteLibraryItems", Qt::QueuedConnection,
                            Q_ARG(QVector<int>, converted));
  return true;
}

QVariantMap LibraryModel::get(int index) const {
  QVariantMap map;
  if (index < 0 || index >= m_items.size()) {
    return map;
  }
  const auto &item = m_items.at(index);
  map.insert("id", item.id);
  map.insert("title", item.title);
  map.insert("authors", item.authors);
  map.insert("series", item.series);
  map.insert("publisher", item.publisher);
  map.insert("description", item.description);
  map.insert("tags", item.tags);
  map.insert("collection", item.collection);
  map.insert("coverPath", item.coverPath);
  map.insert("path", item.path);
  map.insert("format", item.format);
  map.insert("addedAt", item.addedAt);
  map.insert("annotationCount", item.annotationCount);
  return map;
}

QVariantList LibraryModel::exportAnnotationSync() const {
  QVariantList payload;
  QMetaObject::invokeMethod(dbWorker(), "exportAnnotationSync",
                            Qt::BlockingQueuedConnection,
                            Q_RETURN_ARG(QVariantList, payload));
  return payload;
}

int LibraryModel::importAnnotationSync(const QVariantList &payload) {
  int added = 0;
  QMetaObject::invokeMethod(dbWorker(), "importAnnotationSync",
                            Qt::BlockingQueuedConnection,
                            Q_RETURN_ARG(int, added),
                            Q_ARG(QVariantList, payload));
  if (added > 0) {
    QMetaObject::invokeMethod(dbWorker(), "loadLibrary", Qt::QueuedConnection);
  }
  return added;
}

QVariantList LibraryModel::exportLibrarySync() const {
  QVariantList payload;
  QMetaObject::invokeMethod(dbWorker(), "exportLibrarySync",
                            Qt::BlockingQueuedConnection,
                            Q_RETURN_ARG(QVariantList, payload));
  return payload;
}

int LibraryModel::importLibrarySync(const QVariantList &payload, const QString &conflictPolicy) {
  int applied = 0;
  QMetaObject::invokeMethod(dbWorker(), "importLibrarySync",
                            Qt::BlockingQueuedConnection,
                            Q_RETURN_ARG(int, applied),
                            Q_ARG(QVariantList, payload),
                            Q_ARG(QString, conflictPolicy));
  return applied;
}

bool LibraryModel::hasFileHash(const QString &fileHash) const {
  bool exists = false;
  QMetaObject::invokeMethod(dbWorker(), "hasFileHash",
                            Qt::BlockingQueuedConnection,
                            Q_RETURN_ARG(bool, exists),
                            Q_ARG(QString, fileHash));
  return exists;
}

QString LibraryModel::pathForHash(const QString &fileHash) const {
  QString path;
  QMetaObject::invokeMethod(dbWorker(), "pathForHash",
                            Qt::BlockingQueuedConnection,
                            Q_RETURN_ARG(QString, path),
                            Q_ARG(QString, fileHash));
  return path;
}

bool LibraryModel::ready() const { return m_ready; }

int LibraryModel::count() const { return m_items.size(); }

QString LibraryModel::lastError() const { return m_lastError; }

QString LibraryModel::searchQuery() const { return m_searchQuery; }

QString LibraryModel::sortKey() const { return m_sortKey; }

bool LibraryModel::sortDescending() const { return m_sortDescending; }

QString LibraryModel::filterTag() const { return m_filterTag; }

QString LibraryModel::filterCollection() const { return m_filterCollection; }
bool LibraryModel::bulkImportActive() const { return m_bulkImportActive; }
int LibraryModel::bulkImportTotal() const { return m_bulkImportTotal; }
int LibraryModel::bulkImportDone() const { return m_bulkImportDone; }
int LibraryModel::totalCount() const { return m_totalCount; }
int LibraryModel::pageSize() const { return m_pageSize; }
int LibraryModel::pageIndex() const { return m_pageIndex; }
int LibraryModel::pageCount() const { return m_pageCount; }
QStringList LibraryModel::availableCollections() const { return m_availableCollections; }
QStringList LibraryModel::availableTags() const { return m_availableTags; }

void LibraryModel::setSearchQuery(const QString &query) {
  if (m_searchQuery == query) {
    return;
  }
  m_searchQuery = query;
  emit searchQueryChanged();
  setPageIndex(0);
  reload();
}

void LibraryModel::setSortKey(const QString &key) {
  if (m_sortKey == key) {
    return;
  }
  m_sortKey = key;
  emit sortKeyChanged();
  setPageIndex(0);
  reload();
}

void LibraryModel::setSortDescending(bool descending) {
  if (m_sortDescending == descending) {
    return;
  }
  m_sortDescending = descending;
  emit sortDescendingChanged();
  setPageIndex(0);
  reload();
}

void LibraryModel::setFilterTag(const QString &tag) {
  if (m_filterTag == tag) {
    return;
  }
  m_filterTag = tag;
  emit filterTagChanged();
  setPageIndex(0);
  reload();
}

void LibraryModel::setFilterCollection(const QString &collection) {
  if (m_filterCollection == collection) {
    return;
  }
  m_filterCollection = collection;
  emit filterCollectionChanged();
  setPageIndex(0);
  reload();
}

void LibraryModel::setPageSize(int size) {
  const int normalized = std::max(10, std::min(500, size));
  if (m_pageSize == normalized) {
    return;
  }
  m_pageSize = normalized;
  emit pageSizeChanged();
  setPageIndex(0);
  reload();
}

void LibraryModel::setPageIndex(int index) {
  const int normalized = std::max(0, index);
  if (m_pageIndex == normalized) {
    return;
  }
  m_pageIndex = normalized;
  emit pageIndexChanged();
}

void LibraryModel::nextPage() {
  if (m_pageIndex + 1 >= m_pageCount) {
    return;
  }
  m_pageIndex++;
  emit pageIndexChanged();
  reload();
}

void LibraryModel::prevPage() {
  if (m_pageIndex <= 0) {
    return;
  }
  m_pageIndex--;
  emit pageIndexChanged();
  reload();
}

void LibraryModel::goToPage(int index) {
  const int normalized = std::max(0, std::min(index, m_pageCount - 1));
  if (m_pageIndex == normalized) {
    return;
  }
  m_pageIndex = normalized;
  emit pageIndexChanged();
  reload();
}

QString LibraryModel::connectionName() const { return QString(); }

void LibraryModel::reload() {
  QMetaObject::invokeMethod(dbWorker(), "loadLibraryFiltered", Qt::QueuedConnection,
                            Q_ARG(QString, m_searchQuery),
                            Q_ARG(QString, m_sortKey),
                            Q_ARG(bool, m_sortDescending),
                            Q_ARG(QString, m_filterTag),
                            Q_ARG(QString, m_filterCollection),
                            Q_ARG(int, m_pageSize),
                            Q_ARG(int, m_pageIndex));
}

void LibraryModel::close() {
  QMetaObject::invokeMethod(dbWorker(), "closeDatabase", Qt::QueuedConnection);
  beginResetModel();
  m_items.clear();
  endResetModel();
  if (!m_availableCollections.isEmpty()) {
    m_availableCollections.clear();
    emit availableCollectionsChanged();
  }
  if (!m_availableTags.isEmpty()) {
    m_availableTags.clear();
    emit availableTagsChanged();
  }
  m_ready = false;
  emit readyChanged();
  emit countChanged();
}

void LibraryModel::setLastError(const QString &error) {
  if (m_lastError == error) {
    return;
  }
  m_lastError = error;
  emit lastErrorChanged();
}
