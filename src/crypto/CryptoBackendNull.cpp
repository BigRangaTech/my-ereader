#include "include/CryptoBackend.h"

std::unique_ptr<CryptoBackend> createMonocypherBackend();
std::unique_ptr<CryptoBackend> createSodiumBackend();

class CryptoBackendNull final : public CryptoBackend {
public:
  QString name() const override { return "none"; }
  bool isAvailable() const override { return false; }

  int keyBytes() const override { return 32; }
  int saltBytes() const override { return 16; }
  int nonceBytes() const override { return 24; }

  CryptoKdfParams defaultKdfParams() const override { return {}; }
  QByteArray generateSalt() override { return {}; }
  QByteArray generateNonce() override { return {}; }

  bool deriveKey(const QString &, const QByteArray &, const CryptoKdfParams &, QByteArray *, QString *error) override {
    if (error) {
      *error = "No crypto backend available";
    }
    return false;
  }

  bool encrypt(const QByteArray &, const QByteArray &, const QByteArray &, QByteArray *, QString *error) override {
    if (error) {
      *error = "No crypto backend available";
    }
    return false;
  }

  bool decrypt(const QByteArray &, const QByteArray &, const QByteArray &, QByteArray *, QString *error) override {
    if (error) {
      *error = "No crypto backend available";
    }
    return false;
  }
};

std::unique_ptr<CryptoBackend> CryptoBackendFactory::createDefault() {
#ifdef HAVE_MONOCYPHER
  std::unique_ptr<CryptoBackend> createMonocypherBackend();
  return createMonocypherBackend();
#elif defined(HAVE_LIBSODIUM)
  std::unique_ptr<CryptoBackend> createSodiumBackend();
  return createSodiumBackend();
#else
  return std::make_unique<CryptoBackendNull>();
#endif
}
