#pragma once

#include <memory>
#include <vector>

#include "FormatProvider.h"

class FormatRegistry {
public:
  static std::unique_ptr<FormatRegistry> createDefault();

  void registerProvider(std::unique_ptr<FormatProvider> provider);
  std::unique_ptr<FormatDocument> open(const QString &path, QString *error) const;

private:
  std::vector<std::unique_ptr<FormatProvider>> m_providers;
};
