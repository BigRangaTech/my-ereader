#include "include/KeychainStore.h"

#ifdef HAVE_LIBSECRET
#include <QByteArray>
#include <QDebug>

extern "C" {
#include <libsecret/secret.h>
}

namespace {
constexpr const char *kServiceLabel = "My Ereader";
constexpr const char *kAttributeApp = "my-ereader";
constexpr const char *kAttributeVault = "library";

const SecretSchema *schema() {
  static const SecretSchema kSchema = {
      "com.bigrangatech.myereader", SECRET_SCHEMA_NONE,
      {
          {"application", SECRET_SCHEMA_ATTRIBUTE_STRING},
          {"vault", SECRET_SCHEMA_ATTRIBUTE_STRING},
          {nullptr, 0},
      }};
  return &kSchema;
}
} // namespace
#endif

bool KeychainStore::isAvailable() const {
#ifdef HAVE_LIBSECRET
  return true;
#else
  return false;
#endif
}

bool KeychainStore::storePassphrase(const QString &passphrase, QString *error) {
#ifdef HAVE_LIBSECRET
  GError *gerror = nullptr;
  const QByteArray utf8 = passphrase.toUtf8();
  const gboolean ok = secret_password_store_sync(
      schema(),
      SECRET_COLLECTION_DEFAULT,
      kServiceLabel,
      utf8.constData(),
      nullptr,
      &gerror,
      "application",
      kAttributeApp,
      "vault",
      kAttributeVault,
      nullptr);
  if (!ok) {
    if (error) {
      *error = gerror ? QString::fromUtf8(gerror->message) : "Failed to store passphrase";
    }
    if (gerror) {
      g_error_free(gerror);
    }
    return false;
  }
  return true;
#else
  if (error) {
    *error = "Keychain unavailable";
  }
  Q_UNUSED(passphrase);
  return false;
#endif
}

QString KeychainStore::loadPassphrase(QString *error) {
#ifdef HAVE_LIBSECRET
  GError *gerror = nullptr;
  gchar *secret = secret_password_lookup_sync(
      schema(),
      nullptr,
      &gerror,
      "application",
      kAttributeApp,
      "vault",
      kAttributeVault,
      nullptr);
  if (gerror) {
    if (error) {
      *error = QString::fromUtf8(gerror->message);
    }
    g_error_free(gerror);
    return {};
  }
  if (!secret) {
    return {};
  }
  QString result = QString::fromUtf8(secret);
  secret_password_free(secret);
  return result;
#else
  if (error) {
    *error = "Keychain unavailable";
  }
  return {};
#endif
}

bool KeychainStore::clearPassphrase(QString *error) {
#ifdef HAVE_LIBSECRET
  GError *gerror = nullptr;
  const gboolean ok = secret_password_clear_sync(
      schema(),
      nullptr,
      &gerror,
      "application",
      kAttributeApp,
      "vault",
      kAttributeVault,
      nullptr);
  if (!ok) {
    if (error) {
      *error = gerror ? QString::fromUtf8(gerror->message) : "Failed to clear passphrase";
    }
    if (gerror) {
      g_error_free(gerror);
    }
    return false;
  }
  return true;
#else
  if (error) {
    *error = "Keychain unavailable";
  }
  return false;
#endif
}
