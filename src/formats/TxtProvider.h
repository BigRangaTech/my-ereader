#pragma once

#include <QString>

#include "include/FormatProvider.h"

class TxtProvider : public FormatProvider {
public:
  QString name() const override;
  QStringList supportedExtensions() const override;
  std::unique_ptr<FormatDocument> open(const QString &path, QString *error) override;
};
