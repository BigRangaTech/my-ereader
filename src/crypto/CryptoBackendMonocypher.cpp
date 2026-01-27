#include "include/CryptoBackend.h"

#ifdef HAVE_MONOCYPHER
#include <QRandomGenerator>
#include <vector>

#include <cstring>
#include <limits>

extern "C" {
#include "monocypher.h"
}

namespace {
constexpr int kKeyBytes = 32;
constexpr int kSaltBytes = 16;
constexpr int kNonceBytes = 24;
constexpr int kMacBytes = 16;
constexpr quint64 kDefaultMemLimit = 64ull * 1024ull * 1024ull;
constexpr quint64 kDefaultOpsLimit = 3;
constexpr quint32 kDefaultLanes = 1;

quint32 blocksFromBytes(quint64 memBytes) {
  quint64 blocks = memBytes / 1024ull;
  const quint64 minBlocks = 8ull * kDefaultLanes;
  if (blocks < minBlocks) {
    blocks = minBlocks;
  }
  const quint64 align = kDefaultLanes * 4ull;
  blocks = (blocks / align) * align;
  if (blocks < minBlocks) {
    blocks = minBlocks;
  }
  if (blocks > std::numeric_limits<quint32>::max()) {
    blocks = std::numeric_limits<quint32>::max();
  }
  return static_cast<quint32>(blocks);
}

QByteArray randomBytes(int size) {
  QByteArray data;
  data.resize(size);
  auto *generator = QRandomGenerator::system();
  int offset = 0;
  while (offset + 4 <= data.size()) {
    quint32 chunk = generator->generate();
    memcpy(data.data() + offset, &chunk, sizeof(chunk));
    offset += 4;
  }
  if (offset < data.size()) {
    quint32 chunk = generator->generate();
    memcpy(data.data() + offset, &chunk, static_cast<size_t>(data.size() - offset));
  }
  return data;
}
}

class CryptoBackendMonocypher final : public CryptoBackend {
public:
  QString name() const override { return "monocypher"; }
  bool isAvailable() const override { return true; }

  int keyBytes() const override { return kKeyBytes; }
  int saltBytes() const override { return kSaltBytes; }
  int nonceBytes() const override { return kNonceBytes; }

  CryptoKdfParams defaultKdfParams() const override {
    CryptoKdfParams params;
    params.opsLimit = kDefaultOpsLimit;
    params.memLimit = kDefaultMemLimit;
    return params;
  }

  QByteArray generateSalt() override { return randomBytes(kSaltBytes); }
  QByteArray generateNonce() override { return randomBytes(kNonceBytes); }

  bool deriveKey(const QString &passphrase,
                 const QByteArray &salt,
                 const CryptoKdfParams &params,
                 QByteArray *outKey,
                 QString *error) override {
    if (salt.size() != kSaltBytes) {
      if (error) {
        *error = "Invalid salt length";
      }
      return false;
    }

    const QByteArray passUtf8 = passphrase.toUtf8();
    crypto_argon2_config config;
    config.algorithm = CRYPTO_ARGON2_ID;
    config.nb_lanes = kDefaultLanes;
    config.nb_passes = params.opsLimit > 0 ? static_cast<quint32>(params.opsLimit) : kDefaultOpsLimit;
    config.nb_blocks = blocksFromBytes(params.memLimit > 0 ? params.memLimit : kDefaultMemLimit);

    crypto_argon2_inputs inputs;
    inputs.pass = reinterpret_cast<const uint8_t *>(passUtf8.constData());
    inputs.salt = reinterpret_cast<const uint8_t *>(salt.constData());
    inputs.pass_size = static_cast<uint32_t>(passUtf8.size());
    inputs.salt_size = static_cast<uint32_t>(salt.size());

    const size_t workBytes = static_cast<size_t>(config.nb_blocks) * 1024u;
    std::vector<uint64_t> workArea((workBytes + 7) / 8);

    QByteArray key;
    key.resize(kKeyBytes);
    crypto_argon2(reinterpret_cast<uint8_t *>(key.data()),
                  static_cast<uint32_t>(key.size()),
                  workArea.data(),
                  config,
                  inputs,
                  crypto_argon2_no_extras);

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
    if (key.size() != kKeyBytes || nonce.size() != kNonceBytes) {
      if (error) {
        *error = "Invalid key or nonce length";
      }
      return false;
    }

    QByteArray cipher;
    cipher.resize(kMacBytes + plaintext.size());
    uint8_t *mac = reinterpret_cast<uint8_t *>(cipher.data());
    uint8_t *cipherText = reinterpret_cast<uint8_t *>(cipher.data() + kMacBytes);

    crypto_aead_lock(cipherText,
                     mac,
                     reinterpret_cast<const uint8_t *>(key.constData()),
                     reinterpret_cast<const uint8_t *>(nonce.constData()),
                     nullptr,
                     0,
                     reinterpret_cast<const uint8_t *>(plaintext.constData()),
                     static_cast<size_t>(plaintext.size()));

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
    if (key.size() != kKeyBytes || nonce.size() != kNonceBytes) {
      if (error) {
        *error = "Invalid key or nonce length";
      }
      return false;
    }
    if (ciphertext.size() < kMacBytes) {
      if (error) {
        *error = "Ciphertext too short";
      }
      return false;
    }

    const uint8_t *mac = reinterpret_cast<const uint8_t *>(ciphertext.constData());
    const uint8_t *cipherText = reinterpret_cast<const uint8_t *>(ciphertext.constData() + kMacBytes);
    const size_t cipherSize = static_cast<size_t>(ciphertext.size() - kMacBytes);

    QByteArray plain;
    plain.resize(static_cast<int>(cipherSize));
    const int ok = crypto_aead_unlock(reinterpret_cast<uint8_t *>(plain.data()),
                                      mac,
                                      reinterpret_cast<const uint8_t *>(key.constData()),
                                      reinterpret_cast<const uint8_t *>(nonce.constData()),
                                      nullptr,
                                      0,
                                      cipherText,
                                      cipherSize);
    if (ok != 0) {
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
};

std::unique_ptr<CryptoBackend> createMonocypherBackend() {
  return std::make_unique<CryptoBackendMonocypher>();
}
#endif
