#pragma once

#include <QAbstractListModel>
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
    SeriesRole,
    PublisherRole,
    DescriptionRole,
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

private:
  void reload();
  void setLastError(const QString &error);

  bool m_ready = false;
  QString m_lastError;
  QVector<LibraryItem> m_items;
};
