#pragma once

#include <QObject>
#include <QString>
#include <memory>

#include "FormatRegistry.h"

class ReaderController : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString currentTitle READ currentTitle NOTIFY currentChanged)
  Q_PROPERTY(QString currentText READ currentText NOTIFY currentChanged)
  Q_PROPERTY(QString currentPath READ currentPath NOTIFY currentChanged)
  Q_PROPERTY(bool isOpen READ isOpen NOTIFY currentChanged)
  Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
  explicit ReaderController(QObject *parent = nullptr);

  Q_INVOKABLE bool openFile(const QString &path);
  Q_INVOKABLE void close();

  QString currentTitle() const;
  QString currentText() const;
  QString currentPath() const;
  bool isOpen() const;
  QString lastError() const;

signals:
  void currentChanged();
  void lastErrorChanged();

private:
  void setLastError(const QString &error);

  std::unique_ptr<FormatRegistry> m_registry;
  std::unique_ptr<FormatDocument> m_document;
  QString m_currentTitle;
  QString m_currentText;
  QString m_currentPath;
  QString m_lastError;
  bool m_isOpen = false;
};
