#pragma once

#include <QAbstractListModel>
#include <QMetaType>
#include <QString>
#include <QVariant>
#include <QVector>

struct AnnotationItem {
  int id = 0;
  int libraryItemId = 0;
  QString locator;
  QString type;
  QString text;
  QString color;
  QString createdAt;
};

Q_DECLARE_METATYPE(AnnotationItem)
Q_DECLARE_METATYPE(QVector<AnnotationItem>)

class AnnotationModel : public QAbstractListModel {
  Q_OBJECT
  Q_PROPERTY(int libraryItemId READ libraryItemId WRITE setLibraryItemId NOTIFY libraryItemIdChanged)
  Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
  Q_PROPERTY(int count READ count NOTIFY countChanged)
  Q_PROPERTY(int revision READ revision NOTIFY revisionChanged)

public:
  enum Roles {
    IdRole = Qt::UserRole + 1,
    LibraryItemIdRole,
    LocatorRole,
    TypeRole,
    TextRole,
    ColorRole,
    CreatedAtRole
  };

  explicit AnnotationModel(QObject *parent = nullptr);

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;

  int libraryItemId() const;
  void setLibraryItemId(int id);
  QString lastError() const;
  int count() const;
  int revision() const;

  Q_INVOKABLE QVariantMap get(int index) const;
  Q_INVOKABLE QVariantList highlightRangesForChapter(int chapterIndex) const;
  Q_INVOKABLE QVariantList anchorsForPage(int pageIndex) const;
  Q_INVOKABLE bool exportAnnotations(const QString &path);

  Q_INVOKABLE bool addAnnotation(const QString &locator,
                                 const QString &type,
                                 const QString &text,
                                 const QString &color);
  Q_INVOKABLE bool updateAnnotation(int id,
                                    const QString &locator,
                                    const QString &type,
                                    const QString &text,
                                    const QString &color);

  Q_INVOKABLE bool deleteAnnotation(int id);
  Q_INVOKABLE bool attachDatabase(const QString &dbPath);
  Q_INVOKABLE bool attachConnection(const QString &connectionName);

signals:
  void libraryItemIdChanged();
  void lastErrorChanged();
  void countChanged();
  void revisionChanged();

private:
  void reload();
  void setLastError(const QString &error);

  int m_libraryItemId = 0;
  QString m_lastError;
  QVector<AnnotationItem> m_items;
  int m_revision = 0;
};
