#pragma once

#include <QObject>
#include <QVariantList>

class LicenseManager : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString basePath READ basePath CONSTANT)

public:
  explicit LicenseManager(QObject *parent = nullptr);

  QString basePath() const;

  Q_INVOKABLE QVariantList licenses() const;
  Q_INVOKABLE QString readFile(const QString &path) const;

private:
  QString findBasePath() const;
  QString m_basePath;
};
