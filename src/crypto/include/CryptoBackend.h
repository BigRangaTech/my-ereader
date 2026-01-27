#pragma once

#include <QByteArray>
#include <QString>

#include <memory>

struct CryptoKdfParams {
  quint64 opsLimit = 0;
  quint64 memLimit = 0;
};

class CryptoBackend {
public:
  virtual ~CryptoBackend() = default;

  virtual QString name() const = 0;
  virtual bool isAvailable() const = 0;

  virtual int keyBytes() const = 0;
  virtual int saltBytes() const = 0;
  virtual int nonceBytes() const = 0;

  virtual CryptoKdfParams defaultKdfParams() const = 0;
  virtual QByteArray generateSalt() = 0;
  virtual QByteArray generateNonce() = 0;

  virtual bool deriveKey(const QString &passphrase,
                         const QByteArray &salt,
                         const CryptoKdfParams &params,
                         QByteArray *outKey,
                         QString *error) = 0;

  virtual bool encrypt(const QByteArray &key,
                       const QByteArray &nonce,
                       const QByteArray &plaintext,
                       QByteArray *outCiphertext,
                       QString *error) = 0;

  virtual bool decrypt(const QByteArray &key,
                       const QByteArray &nonce,
                       const QByteArray &ciphertext,
                       QByteArray *outPlaintext,
                       QString *error) = 0;
};

class CryptoBackendFactory {
public:
  static std::unique_ptr<CryptoBackend> createDefault();
};
