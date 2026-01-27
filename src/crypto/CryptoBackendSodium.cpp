#include "include/CryptoBackend.h"

#ifdef HAVE_LIBSODIUM
#include <sodium.h>

class CryptoBackendSodium final : public CryptoBackend {
public:
  CryptoBackendSodium() {
    m_available = (sodium_init() >= 0);
  }

  QString name() const override { return "libsodium"; }
  bool isAvailable() const override { return m_available; }

  int keyBytes() const override { return crypto_secretbox_KEYBYTES; }
  int saltBytes() const override { return crypto_pwhash_SALTBYTES; }
  int nonceBytes() const override { return crypto_secretbox_NONCEBYTES; }

  CryptoKdfParams defaultKdfParams() const override {
    CryptoKdfParams params;
    params.opsLimit = crypto_pwhash_OPSLIMIT_MODERATE;
    params.memLimit = crypto_pwhash_MEMLIMIT_MODERATE;
    return params;
  }

  QByteArray generateSalt() override {
    QByteArray salt;
    salt.resize(saltBytes());
    randombytes_buf(salt.data(), static_cast<size_t>(salt.size()));
    return salt;
  }

  QByteArray generateNonce() override {
    QByteArray nonce;
    nonce.resize(nonceBytes());
    randombytes_buf(nonce.data(), static_cast<size_t>(nonce.size()));
    return nonce;
  }

  bool deriveKey(const QString &passphrase,
                 const QByteArray &salt,
                 const CryptoKdfParams &params,
                 QByteArray *outKey,
                 QString *error) override {
    if (!m_available) {
      if (error) {
        *error = "libsodium not available";
      }
      return false;
    }
    if (salt.size() != saltBytes()) {
      if (error) {
        *error = "Invalid salt length";
      }
      return false;
    }
    QByteArray key;
    key.resize(keyBytes());
    const QByteArray passUtf8 = passphrase.toUtf8();
    if (crypto_pwhash(reinterpret_cast<unsigned char *>(key.data()),
                      static_cast<unsigned long long>(key.size()),
                      passUtf8.constData(),
                      static_cast<unsigned long long>(passUtf8.size()),
                      reinterpret_cast<const unsigned char *>(salt.constData()),
                      params.opsLimit,
                      params.memLimit,
                      crypto_pwhash_ALG_DEFAULT) != 0) {
      if (error) {
        *error = "Key derivation failed";
      }
      return false;
    }
    if (outKey) {
      *outKey = key;
    }
    return true;
  }

  bool encrypt(const QByteArray &key,
               const QByteArray &nonce,
               const QByteArray &plaintext,
               QByteArray *outCiphertext,
               QString *error) override {
    if (!m_available) {
      if (error) {
        *error = "libsodium not available";
      }
      return false;
    }
    if (key.size() != keyBytes() || nonce.size() != nonceBytes()) {
      if (error) {
        *error = "Invalid key or nonce length";
      }
      return false;
    }
    QByteArray cipher;
    cipher.resize(plaintext.size() + crypto_secretbox_MACBYTES);
    if (crypto_secretbox_easy(reinterpret_cast<unsigned char *>(cipher.data()),
                              reinterpret_cast<const unsigned char *>(plaintext.constData()),
                              static_cast<unsigned long long>(plaintext.size()),
                              reinterpret_cast<const unsigned char *>(nonce.constData()),
                              reinterpret_cast<const unsigned char *>(key.constData())) != 0) {
      if (error) {
        *error = "Encryption failed";
      }
      return false;
    }
    if (outCiphertext) {
      *outCiphertext = cipher;
    }
    return true;
  }

  bool decrypt(const QByteArray &key,
               const QByteArray &nonce,
               const QByteArray &ciphertext,
               QByteArray *outPlaintext,
               QString *error) override {
    if (!m_available) {
      if (error) {
        *error = "libsodium not available";
      }
      return false;
    }
    if (key.size() != keyBytes() || nonce.size() != nonceBytes()) {
      if (error) {
        *error = "Invalid key or nonce length";
      }
      return false;
    }
    if (ciphertext.size() < crypto_secretbox_MACBYTES) {
      if (error) {
        *error = "Ciphertext too short";
      }
      return false;
    }
    QByteArray plain;
    plain.resize(ciphertext.size() - crypto_secretbox_MACBYTES);
    if (crypto_secretbox_open_easy(reinterpret_cast<unsigned char *>(plain.data()),
                                   reinterpret_cast<const unsigned char *>(ciphertext.constData()),
                                   static_cast<unsigned long long>(ciphertext.size()),
                                   reinterpret_cast<const unsigned char *>(nonce.constData()),
                                   reinterpret_cast<const unsigned char *>(key.constData())) != 0) {
      if (error) {
        *error = "Decryption failed";
      }
      return false;
    }
    if (outPlaintext) {
      *outPlaintext = plain;
    }
    return true;
  }

private:
  bool m_available = false;
};

std::unique_ptr<CryptoBackend> createSodiumBackend() {
  return std::make_unique<CryptoBackendSodium>();
}
#endif
