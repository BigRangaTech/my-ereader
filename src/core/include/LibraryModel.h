#pragma once

#include <QAbstractListModel>
#include <QVector>
#include <QVariant>
#include <QVariantMap>

#include "LibraryItem.h"

class LibraryModel : public QAbstractListModel {
  Q_OBJECT
  Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)
  Q_PROPERTY(int count READ count NOTIFY countChanged)
  Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
  Q_PROPERTY(QString searchQuery READ searchQuery WRITE setSearchQuery NOTIFY searchQueryChanged)
  Q_PROPERTY(QString sortKey READ sortKey WRITE setSortKey NOTIFY sortKeyChanged)
  Q_PROPERTY(bool sortDescending READ sortDescending WRITE setSortDescending NOTIFY sortDescendingChanged)
  Q_PROPERTY(QString filterTag READ filterTag WRITE setFilterTag NOTIFY filterTagChanged)
  Q_PROPERTY(QString filterCollection READ filterCollection WRITE setFilterCollection NOTIFY filterCollectionChanged)
  Q_PROPERTY(bool bulkImportActive READ bulkImportActive NOTIFY bulkImportChanged)
  Q_PROPERTY(int bulkImportTotal READ bulkImportTotal NOTIFY bulkImportChanged)
  Q_PROPERTY(int bulkImportDone READ bulkImportDone NOTIFY bulkImportChanged)

public:
  enum Roles {
    IdRole = Qt::UserRole + 1,
    TitleRole,
    AuthorsRole,
    SeriesRole,
    PublisherRole,
    DescriptionRole,
    TagsRole,
    CollectionRole,
    CoverPathRole,
    PathRole,
    FormatRole,
    AddedAtRole,
    AnnotationCountRole
  };

  explicit LibraryModel(QObject *parent = nullptr);
  ~LibraryModel() override;

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;

  Q_INVOKABLE bool openDefault();
  Q_INVOKABLE bool openAt(const QString &dbPath);
  Q_INVOKABLE bool addBook(const QString &filePath);
  Q_INVOKABLE bool addBooks(const QStringList &filePaths);
  Q_INVOKABLE bool addFolder(const QString &folderPath, bool recursive);
  Q_INVOKABLE bool updateMetadata(int id,
                                  const QString &title,
                                  const QString &authors,
                                  const QString &series,
                                  const QString &publisher,
                                  const QString &description,
                                  const QString &tags,
                                  const QString &collection);
  Q_INVOKABLE bool removeBook(int id);
  Q_INVOKABLE bool updateTagsCollection(const QVariantList &ids,
                                        const QString &tags,
                                        const QString &collection,
                                        bool updateTags,
                                        bool updateCollection);
  Q_INVOKABLE bool removeBooks(const QVariantList &ids);
  Q_INVOKABLE QVariantMap get(int index) const;

  QString searchQuery() const;
  QString sortKey() const;
  bool sortDescending() const;
  QString filterTag() const;
  QString filterCollection() const;
  bool bulkImportActive() const;
  int bulkImportTotal() const;
  int bulkImportDone() const;
  void setSearchQuery(const QString &query);
  void setSortKey(const QString &key);
  void setSortDescending(bool descending);
  void setFilterTag(const QString &tag);
  void setFilterCollection(const QString &collection);
  Q_INVOKABLE void close();
  Q_INVOKABLE bool openEncryptedVault(const QString &vaultPath, const QString &passphrase);
  Q_INVOKABLE bool saveEncryptedVault(const QString &vaultPath, const QString &passphrase);
  Q_INVOKABLE QString connectionName() const;

  bool ready() const;
  int count() const;
  QString lastError() const;

signals:
  void readyChanged();
  void countChanged();
  void lastErrorChanged();
  void searchQueryChanged();
  void sortKeyChanged();
  void sortDescendingChanged();
  void filterTagChanged();
  void filterCollectionChanged();
  void bulkImportChanged();

private:
  void reload();
  void setLastError(const QString &error);

  bool m_ready = false;
  QString m_lastError;
  QString m_searchQuery;
  QString m_sortKey = "title";
  bool m_sortDescending = false;
  QString m_filterTag;
  QString m_filterCollection;
  bool m_bulkImportActive = false;
  int m_bulkImportTotal = 0;
  int m_bulkImportDone = 0;
  QVector<LibraryItem> m_items;
};
