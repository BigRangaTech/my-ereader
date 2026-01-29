#include "include/FormatRegistry.h"

#include <QFileInfo>

#include "TxtProvider.h"
#include "PdfProvider.h"
#include "EpubProvider.h"
#include "Fb2Provider.h"
#include "CbzProvider.h"
#include "MobiProvider.h"
#include "DjvuProvider.h"

std::unique_ptr<FormatRegistry> FormatRegistry::createDefault() {
  auto registry = std::make_unique<FormatRegistry>();
  registry->registerProvider(std::make_unique<TxtProvider>());
  registry->registerProvider(std::make_unique<EpubProvider>());
  registry->registerProvider(std::make_unique<PdfProvider>());
  registry->registerProvider(std::make_unique<MobiProvider>());
  registry->registerProvider(std::make_unique<Fb2Provider>());
  registry->registerProvider(std::make_unique<CbzProvider>());
  registry->registerProvider(std::make_unique<DjvuProvider>());
  return registry;
}

void FormatRegistry::registerProvider(std::unique_ptr<FormatProvider> provider) {
  if (!provider) {
    return;
  }
  m_providers.push_back(std::move(provider));
}

std::unique_ptr<FormatDocument> FormatRegistry::open(const QString &path, QString *error) const {
  const QString extension = QFileInfo(path).suffix().toLower();
  for (const auto &provider : m_providers) {
    const auto supported = provider->supportedExtensions();
    if (supported.contains(extension)) {
      return provider->open(path, error);
    }
  }

  if (error) {
    *error = QString("No provider for extension: %1").arg(extension);
  }
  return nullptr;
}
