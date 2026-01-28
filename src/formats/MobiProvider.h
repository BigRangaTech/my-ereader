#pragma once

#include "include/FormatProvider.h"

class MobiProvider : public FormatProvider {
public:
  QString name() const override;
  QStringList supportedExtensions() const override;
  std::unique_ptr<FormatDocument> open(const QString &path, QString *error) override;
};
