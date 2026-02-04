#include "include/KeychainStore.h"

#include <QByteArray>
#include <QCryptographicHash>
#include <QEventLoop>
#include <QFile>
#include <QSettings>
#include <QCoreApplication>
#include <QTimer>
#include <QVariantMap>

#include "include/AppPaths.h"
#include "../crypto/include/CryptoBackend.h"

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusObjectPath>
#include <QtDBus/QDBusPendingCall>
#include <QtDBus/QDBusPendingCallWatcher>
#include <QtDBus/QDBusPendingReply>
#include <QtDBus/QDBusUnixFileDescriptor>

#include <fcntl.h>
#include <unistd.h>

#ifdef HAVE_LIBSECRET
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

namespace {
QString portalConfigPath() {
  return AppPaths::configFile("vault.ini");
}

bool portalDataPresent() {
  QSettings settings(portalConfigPath(), QSettings::IniFormat);
  const QString token = settings.value("portal/token").toString();
  const QByteArray ciphertext = QByteArray::fromBase64(settings.value("portal/ciphertext").toByteArray());
  return !token.isEmpty() && !ciphertext.isEmpty();
}

class PortalResponseWatcher : public QObject {
  Q_OBJECT

public:
  QEventLoop *loop = nullptr;
  int *responseCode = nullptr;
  QVariantMap *results = nullptr;

public slots:
  void onResponse(uint code, const QVariantMap &result) {
    if (responseCode) {
      *responseCode = static_cast<int>(code);
    }
    if (results) {
      *results = result;
    }
    if (loop) {
      loop->quit();
    }
  }
};

QByteArray deriveKey(const QByteArray &secret, int keyBytes) {
  QByteArray seed = secret + QByteArrayLiteral("my-ereader-key-v1");
  QByteArray hash = QCryptographicHash::hash(seed, QCryptographicHash::Sha256);
  if (keyBytes <= hash.size()) {
    return hash.left(keyBytes);
  }
  QByteArray out;
  out.reserve(keyBytes);
  QByteArray current = hash;
  while (out.size() < keyBytes) {
    out.append(current);
    current = QCryptographicHash::hash(current, QCryptographicHash::Sha256);
  }
  return out.left(keyBytes);
}

QByteArray portalSecret(QString *outToken, QString *error) {
  if (!QCoreApplication::instance()) {
    if (error) {
      *error = "Secret portal unavailable (no Qt application)";
    }
    return {};
  }
  if (!QDBusConnection::sessionBus().isConnected()) {
    if (error) {
      *error = "Secret portal unavailable (no DBus session)";
    }
    return {};
  }
  QSettings settings(portalConfigPath(), QSettings::IniFormat);
  const QString token = settings.value("portal/token").toString();

  QDBusInterface portal("org.freedesktop.portal.Desktop",
                        "/org/freedesktop/portal/desktop",
                        "org.freedesktop.portal.Secret",
                        QDBusConnection::sessionBus());
  if (!portal.isValid()) {
    if (error) {
      *error = "Secret portal unavailable";
    }
    return {};
  }

  int fds[2];
  if (::pipe(fds) != 0) {
    if (error) {
      *error = "Failed to create secret pipe";
    }
    return {};
  }

  const int readFlags = fcntl(fds[0], F_GETFL, 0);
  if (readFlags >= 0) {
    fcntl(fds[0], F_SETFL, readFlags | O_NONBLOCK);
  }

  QDBusUnixFileDescriptor writeFd(fds[1]);
  QVariantMap options;
  if (!token.isEmpty()) {
    options.insert("token", token);
  }

  QDBusPendingCall call = portal.asyncCall("RetrieveSecret", QVariant::fromValue(writeFd), options);
  QDBusPendingCallWatcher watcher(call);
  QEventLoop loop;
  QTimer retrieveTimer;
  retrieveTimer.setSingleShot(true);
  QObject::connect(&retrieveTimer, &QTimer::timeout, &loop, &QEventLoop::quit);
  QObject::connect(&watcher, &QDBusPendingCallWatcher::finished, &loop, &QEventLoop::quit);
  retrieveTimer.start(3000);
  loop.exec();

  if (retrieveTimer.isActive()) {
    retrieveTimer.stop();
  } else {
    if (error) {
      *error = "Secret portal timeout";
    }
    ::close(fds[0]);
    ::close(fds[1]);
    return {};
  }

  QDBusPendingReply<QDBusObjectPath> reply = watcher.reply();
  if (reply.isError()) {
    if (error) {
      *error = reply.error().message();
    }
    ::close(fds[0]);
    ::close(fds[1]);
    return {};
  }

  QByteArray secret;
  int responseCode = 1;
  QVariantMap results;
  const QDBusObjectPath requestPath = reply.value();
  if (requestPath.path().isEmpty()) {
    if (error) {
      *error = "Secret portal returned empty request path";
    }
    ::close(fds[0]);
    ::close(fds[1]);
    return {};
  }
  QEventLoop responseLoop;
  QTimer responseTimer;
  responseTimer.setSingleShot(true);
  PortalResponseWatcher responseWatcher;
  responseWatcher.loop = &responseLoop;
  responseWatcher.responseCode = &responseCode;
  responseWatcher.results = &results;
  QDBusConnection::sessionBus().connect("org.freedesktop.portal.Desktop",
                                        requestPath.path(),
                                        "org.freedesktop.portal.Request",
                                        "Response",
                                        &responseWatcher,
                                        SLOT(onResponse(uint,QVariantMap)));
  QObject::connect(&responseTimer, &QTimer::timeout, &responseLoop, &QEventLoop::quit);
  responseTimer.start(3000);
  responseLoop.exec();

  if (!responseTimer.isActive()) {
    if (error) {
      *error = "Secret portal response timeout";
    }
  }
  if (responseCode == 0) {
    QFile readFile;
    readFile.open(fds[0], QIODevice::ReadOnly);
    secret = readFile.readAll();
    readFile.close();
  }

  ::close(fds[0]);
  ::close(fds[1]);

  if (!responseTimer.isActive()) {
    if (error) {
      *error = "Secret portal response timeout";
    }
  }

  if (responseCode != 0 || secret.isEmpty()) {
    if (error) {
      *error = secret.isEmpty() ? "Secret portal returned no data" : "Secret portal denied";
    }
    return {};
  }

  if (results.contains("token")) {
    const QString newToken = results.value("token").toString();
    if (!newToken.isEmpty()) {
      settings.setValue("portal/token", newToken);
      settings.sync();
      if (outToken) {
        *outToken = newToken;
      }
    }
  }

  return secret;
}
} // namespace

bool KeychainStore::isAvailable() const {
#ifdef HAVE_LIBSECRET
  return true;
#else
  return portalDataPresent();
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
  QString token;
  QByteArray secret = portalSecret(&token, error);
  if (secret.isEmpty()) {
    return false;
  }
  auto backend = CryptoBackendFactory::createDefault();
  if (!backend || !backend->isAvailable()) {
    if (error) {
      *error = "Crypto backend unavailable";
    }
    return false;
  }
  QByteArray key = deriveKey(secret, backend->keyBytes());
  QByteArray nonce = backend->generateNonce();
  QByteArray ciphertext;
  if (!backend->encrypt(key, nonce, passphrase.toUtf8(), &ciphertext, error)) {
    return false;
  }
  QSettings settings(portalConfigPath(), QSettings::IniFormat);
  settings.setValue("portal/nonce", nonce.toBase64());
  settings.setValue("portal/ciphertext", ciphertext.toBase64());
  settings.sync();
  return true;
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
  QString token;
  QByteArray secret = portalSecret(&token, error);
  if (secret.isEmpty()) {
    return {};
  }
  QSettings settings(portalConfigPath(), QSettings::IniFormat);
  const QByteArray nonce = QByteArray::fromBase64(settings.value("portal/nonce").toByteArray());
  const QByteArray ciphertext = QByteArray::fromBase64(settings.value("portal/ciphertext").toByteArray());
  if (nonce.isEmpty() || ciphertext.isEmpty()) {
    return {};
  }
  auto backend = CryptoBackendFactory::createDefault();
  if (!backend || !backend->isAvailable()) {
    if (error) {
      *error = "Crypto backend unavailable";
    }
    return {};
  }
  QByteArray key = deriveKey(secret, backend->keyBytes());
  QByteArray plaintext;
  if (!backend->decrypt(key, nonce, ciphertext, &plaintext, error)) {
    return {};
  }
  return QString::fromUtf8(plaintext);
#endif
}

#include "KeychainStore.moc"

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
  if (!portalDataPresent()) {
    return {};
  }
  QSettings settings(portalConfigPath(), QSettings::IniFormat);
  settings.remove("portal/nonce");
  settings.remove("portal/ciphertext");
  settings.sync();
  return true;
#endif
}
