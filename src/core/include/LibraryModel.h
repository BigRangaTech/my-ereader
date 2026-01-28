#pragma once

#include <QAbstractListModel>
#include <QSqlDatabase>
#include <QVector>

#include "LibraryItem.h"

class LibraryModel : public QAbstractListModel {
  Q_OBJECT
  Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)
  Q_PROPERTY(int count READ count NOTIFY countChanged)
  Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
  enum Roles {
    IdRole = Qt::UserRole + 1,
    TitleRole,
    AuthorsRole,
    PathRole,
    FormatRole,
    AddedAtRole
  };

  explicit LibraryModel(QObject *parent = nullptr);
  ~LibraryModel() override;

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;

  Q_INVOKABLE bool openDefault();
  Q_INVOKABLE bool openAt(const QString &dbPath);
  Q_INVOKABLE bool addBook(const QString &filePath);
  Q_INVOKABLE void close();

  bool ready() const;
  int count() const;
  QString lastError() const;

signals:
  void readyChanged();
  void countChanged();
  void lastErrorChanged();

private:
  bool openDatabase(const QString &dbPath);
  bool ensureSchema();
  void reload();
  LibraryItem makeItemFromFile(const QString &filePath);
  QString computeFileHash(const QString &filePath);
  void setLastError(const QString &error);

  bool m_ready = false;
  QString m_lastError;
  QSqlDatabase m_db;
  QString m_connectionName;
  QVector<LibraryItem> m_items;
};
