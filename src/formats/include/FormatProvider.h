#pragma once

#include <QString>
#include <QStringList>
#include <memory>

#include "FormatDocument.h"

class FormatProvider {
public:
  virtual ~FormatProvider() = default;

  virtual QString name() const = 0;
  virtual QStringList supportedExtensions() const = 0;
  virtual std::unique_ptr<FormatDocument> open(const QString &path, QString *error) = 0;
};
