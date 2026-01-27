#pragma once

#include <QString>

#include "CryptoBackend.h"

class CryptoVault {
public:
  explicit CryptoVault(std::unique_ptr<CryptoBackend> backend);

  bool encryptFile(const QString &inputPath,
                   const QString &outputPath,
                   const QString &passphrase,
                   QString *error);

  bool decryptFile(const QString &inputPath,
                   const QString &outputPath,
                   const QString &passphrase,
                   QString *error);

private:
  bool writeVault(const QString &outputPath,
                  const QByteArray &salt,
                  const QByteArray &nonce,
                  const QByteArray &ciphertext,
                  const CryptoKdfParams &params,
                  QString *error);

  bool readVault(const QString &inputPath,
                 QByteArray *salt,
                 QByteArray *nonce,
                 QByteArray *ciphertext,
                 CryptoKdfParams *params,
                 QString *error);

  std::unique_ptr<CryptoBackend> m_backend;
};
