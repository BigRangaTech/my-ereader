#include "include/CryptoVault.h"

#include <QDataStream>
#include <QFile>

#include "include/CryptoBackend.h"

namespace {
constexpr char kMagic[] = "MYEVAULT";
constexpr quint8 kVersion = 1;
}

CryptoVault::CryptoVault(std::unique_ptr<CryptoBackend> backend)
    : m_backend(std::move(backend)) {}

bool CryptoVault::encryptFile(const QString &inputPath,
                              const QString &outputPath,
                              const QString &passphrase,
                              QString *error) {
  if (!m_backend || !m_backend->isAvailable()) {
    if (error) {
      *error = "Crypto backend unavailable";
    }
    return false;
  }

  QFile inputFile(inputPath);
  if (!inputFile.open(QIODevice::ReadOnly)) {
    if (error) {
      *error = "Failed to read input file";
    }
    return false;
  }

  const QByteArray plaintext = inputFile.readAll();
  const QByteArray salt = m_backend->generateSalt();
  const QByteArray nonce = m_backend->generateNonce();
  const CryptoKdfParams params = m_backend->defaultKdfParams();

  QByteArray key;
  QString localError;
  if (!m_backend->deriveKey(passphrase, salt, params, &key, &localError)) {
    if (error) {
      *error = localError.isEmpty() ? "Key derivation failed" : localError;
    }
    return false;
  }

  QByteArray ciphertext;
  if (!m_backend->encrypt(key, nonce, plaintext, &ciphertext, &localError)) {
    if (error) {
      *error = localError.isEmpty() ? "Encryption failed" : localError;
    }
    return false;
  }

  return writeVault(outputPath, salt, nonce, ciphertext, params, error);
}

bool CryptoVault::decryptFile(const QString &inputPath,
                              const QString &outputPath,
                              const QString &passphrase,
                              QString *error) {
  if (!m_backend || !m_backend->isAvailable()) {
    if (error) {
      *error = "Crypto backend unavailable";
    }
    return false;
  }

  QByteArray salt;
  QByteArray nonce;
  QByteArray ciphertext;
  CryptoKdfParams params;
  if (!readVault(inputPath, &salt, &nonce, &ciphertext, &params, error)) {
    return false;
  }

  QByteArray key;
  QString localError;
  if (!m_backend->deriveKey(passphrase, salt, params, &key, &localError)) {
    if (error) {
      *error = localError.isEmpty() ? "Key derivation failed" : localError;
    }
    return false;
  }

  QByteArray plaintext;
  if (!m_backend->decrypt(key, nonce, ciphertext, &plaintext, &localError)) {
    if (error) {
      *error = localError.isEmpty() ? "Decryption failed" : localError;
    }
    return false;
  }

  QFile outputFile(outputPath);
  if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    if (error) {
      *error = "Failed to write output file";
    }
    return false;
  }
  if (outputFile.write(plaintext) != plaintext.size()) {
    if (error) {
      *error = "Failed to write full output";
    }
    return false;
  }
  return true;
}

bool CryptoVault::writeVault(const QString &outputPath,
                             const QByteArray &salt,
                             const QByteArray &nonce,
                             const QByteArray &ciphertext,
                             const CryptoKdfParams &params,
                             QString *error) {
  QFile outputFile(outputPath);
  if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    if (error) {
      *error = "Failed to open vault for writing";
    }
    return false;
  }

  QDataStream stream(&outputFile);
  stream.setByteOrder(QDataStream::LittleEndian);
  stream.writeRawData(kMagic, static_cast<int>(sizeof(kMagic) - 1));
  stream << kVersion;
  stream << static_cast<quint32>(salt.size());
  stream << static_cast<quint32>(nonce.size());
  stream << static_cast<quint64>(ciphertext.size());
  stream << static_cast<quint64>(params.opsLimit);
  stream << static_cast<quint64>(params.memLimit);

  if (stream.status() != QDataStream::Ok) {
    if (error) {
      *error = "Failed to write vault header";
    }
    return false;
  }

  if (outputFile.write(salt) != salt.size() || outputFile.write(nonce) != nonce.size() ||
      outputFile.write(ciphertext) != ciphertext.size()) {
    if (error) {
      *error = "Failed to write vault payload";
    }
    return false;
  }

  return true;
}

bool CryptoVault::readVault(const QString &inputPath,
                            QByteArray *salt,
                            QByteArray *nonce,
                            QByteArray *ciphertext,
                            CryptoKdfParams *params,
                            QString *error) {
  QFile inputFile(inputPath);
  if (!inputFile.open(QIODevice::ReadOnly)) {
    if (error) {
      *error = "Failed to open vault";
    }
    return false;
  }

  QDataStream stream(&inputFile);
  stream.setByteOrder(QDataStream::LittleEndian);

  char magic[sizeof(kMagic) - 1] = {};
  if (stream.readRawData(magic, static_cast<int>(sizeof(magic))) != static_cast<int>(sizeof(magic))) {
    if (error) {
      *error = "Invalid vault header";
    }
    return false;
  }
  if (QByteArray(magic, static_cast<int>(sizeof(magic))) != QByteArray(kMagic)) {
    if (error) {
      *error = "Unrecognized vault magic";
    }
    return false;
  }

  quint8 version = 0;
  quint32 saltSize = 0;
  quint32 nonceSize = 0;
  quint64 ciphertextSize = 0;
  quint64 opsLimit = 0;
  quint64 memLimit = 0;

  stream >> version;
  stream >> saltSize;
  stream >> nonceSize;
  stream >> ciphertextSize;
  stream >> opsLimit;
  stream >> memLimit;

  if (stream.status() != QDataStream::Ok || version != kVersion) {
    if (error) {
      *error = "Unsupported vault version";
    }
    return false;
  }

  QByteArray saltData = inputFile.read(static_cast<qint64>(saltSize));
  QByteArray nonceData = inputFile.read(static_cast<qint64>(nonceSize));
  QByteArray cipherData = inputFile.read(static_cast<qint64>(ciphertextSize));

  if (saltData.size() != static_cast<int>(saltSize) || nonceData.size() != static_cast<int>(nonceSize) ||
      cipherData.size() != static_cast<int>(ciphertextSize)) {
    if (error) {
      *error = "Vault payload truncated";
    }
    return false;
  }

  if (salt) {
    *salt = saltData;
  }
  if (nonce) {
    *nonce = nonceData;
  }
  if (ciphertext) {
    *ciphertext = cipherData;
  }
  if (params) {
    params->opsLimit = opsLimit;
    params->memLimit = memLimit;
  }

  return true;
}
