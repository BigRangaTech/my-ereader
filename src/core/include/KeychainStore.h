#pragma once

#include <QString>

class KeychainStore {
public:
  bool isAvailable() const;

  bool storePassphrase(const QString &passphrase, QString *error);
  QString loadPassphrase(QString *error);
  bool clearPassphrase(QString *error);
};
