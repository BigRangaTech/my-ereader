#include "DjvuProvider.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QList>
#include <QMutex>
#include <QMutexLocker>
#include <QProcess>
#include <QSet>
#include <QSettings>
#include <QStandardPaths>
#include <QThreadPool>
#include <QDebug>
#include <QRunnable>
#include <QTransform>
#include <algorithm>
#include <functional>
#include <memory>

namespace {
QString formatSettingsPath() {
  QDir dir(QCoreApplication::applicationDirPath());
  for (int i = 0; i < 6; ++i) {
    if (QFileInfo::exists(dir.filePath("README.md"))) {
      return dir.filePath("config/djvu.ini");
    }
    if (!dir.cdUp()) {
      break;
    }
  }
  return QDir(QCoreApplication::applicationDirPath()).filePath("djvu.ini");
}

QString repoRoot() {
  QDir dir(QCoreApplication::applicationDirPath());
  for (int i = 0; i < 6; ++i) {
    if (QFileInfo::exists(dir.filePath("README.md"))) {
      return dir.absolutePath();
    }
    if (!dir.cdUp()) {
      break;
    }
  }
  return QCoreApplication::applicationDirPath();
}

int clampInt(int value, int minValue, int maxValue) {
  return std::max(minValue, std::min(maxValue, value));
}

struct DjvuSettings {
  int dpi = 120;
  int cacheLimit = 30;
  int prefetchDistance = 1;
  QString cachePolicy = "fifo";
  QString format = "ppm";
  bool extractText = true;
  int rotation = 0;
};

DjvuSettings loadDjvuSettings() {
  QSettings settings(formatSettingsPath(), QSettings::IniFormat);
  DjvuSettings out;
  out.dpi = clampInt(settings.value("render/dpi", 120).toInt(), 72, 240);
  out.cacheLimit = clampInt(settings.value("render/cache_limit", 30).toInt(), 5, 120);
  out.prefetchDistance = clampInt(settings.value("render/prefetch_distance", 1).toInt(), 0, 6);
  out.cachePolicy = settings.value("render/cache_policy", "fifo").toString().toLower();
  if (out.cachePolicy != "fifo" && out.cachePolicy != "lru") {
    out.cachePolicy = "fifo";
  }
  out.format = settings.value("render/format", "ppm").toString().toLower();
  if (out.format != "ppm" && out.format != "tiff") {
    out.format = "ppm";
  }
  out.extractText = settings.value("render/extract_text", true).toBool();
  out.rotation = clampInt(settings.value("render/rotation", 0).toInt(), 0, 270);
  if (out.rotation != 0 && out.rotation != 90 && out.rotation != 180 && out.rotation != 270) {
    out.rotation = 0;
  }
  return out;
}

QString tempDirFor(const QFileInfo &info) {
  const QString key = QString("%1|%2|%3")
                          .arg(info.absoluteFilePath())
                          .arg(info.size())
                          .arg(info.lastModified().toSecsSinceEpoch());
  const QByteArray hash = QCryptographicHash::hash(key.toUtf8(), QCryptographicHash::Sha1).toHex();
  return QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation))
      .filePath(QString("ereader_djvu_%1").arg(QString::fromUtf8(hash)));
}

QString formatExtension(const QString &format) {
  const QString normalized = format.trimmed().toLower();
  if (normalized == "tiff") {
    return "tiff";
  }
  return "ppm";
}

QString findTool(const QString &name) {
  const QString root = repoRoot();
  const QString appDir = QCoreApplication::applicationDirPath();
  const QStringList searchDirs = {
      QDir(root).filePath("third_party/install/djvulibre/bin"),
      QDir(root).filePath("third_party/djvulibre/bin"),
      QDir(root).filePath("third_party/djvulibre-bin/bin"),
      QDir(appDir).filePath("tools/djvulibre/bin"),
      QDir(appDir).filePath("djvulibre/bin"),
  };

  for (const QString &dir : searchDirs) {
    const QString candidate = QDir(dir).filePath(name);
    if (QFileInfo::exists(candidate)) {
      return candidate;
    }
  }
  return QStandardPaths::findExecutable(name);
}

int parseFirstInt(const QString &text) {
  QString digits;
  for (const QChar &c : text) {
    if (c.isDigit()) {
      digits.append(c);
    } else if (!digits.isEmpty()) {
      break;
    }
  }
  bool ok = false;
  const int value = digits.toInt(&ok);
  return ok ? value : 0;
}

int djvuPageCount(const QString &djvusedPath, const QString &path) {
  if (djvusedPath.isEmpty()) {
    return 0;
  }
  QProcess proc;
  proc.start(djvusedPath, {"-e", "n", path});
  if (!proc.waitForFinished(10000)) {
    proc.kill();
    return 0;
  }
  if (proc.exitStatus() != QProcess::NormalExit || proc.exitCode() != 0) {
    return 0;
  }
  const QString out = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
  return parseFirstInt(out);
}

QString djvuText(const QString &djvutxtPath, const QString &path) {
  if (djvutxtPath.isEmpty()) {
    return {};
  }
  QProcess proc;
  proc.start(djvutxtPath, {path});
  if (!proc.waitForFinished(15000)) {
    proc.kill();
    return {};
  }
  if (proc.exitStatus() != QProcess::NormalExit || proc.exitCode() != 0) {
    return {};
  }
  return QString::fromUtf8(proc.readAllStandardOutput());
}

class DjvuRunnable final : public QRunnable {
public:
  explicit DjvuRunnable(std::function<void()> task) : m_task(std::move(task)) {}
  void run() override {
    if (m_task) {
      m_task();
    }
  }

private:
  std::function<void()> m_task;
};

void runDjvuTask(std::function<void()> task) {
  auto *runnable = new DjvuRunnable(std::move(task));
  runnable->setAutoDelete(true);
  QThreadPool::globalInstance()->start(runnable);
}

struct DjvuRenderState {
  QString sourcePath;
  QStringList images;
  QString tempDir;
  QString ddjvuPath;
  int dpi = 120;
  int cacheLimit = 30;
  int prefetchDistance = 1;
  QString cachePolicy = "fifo";
  QString format = "ppm";
  int rotation = 0;
  QSet<int> cached;
  QSet<int> inFlight;
  QList<int> cacheOrder;
  std::function<void(int)> onImageReady;
  QMutex mutex;
  bool alive = true;
};

bool renderDjvuPage(const DjvuRenderState &state, int index) {
  if (index < 0 || index >= state.images.size()) {
    return false;
  }
  const QString outPath = state.images.at(index);
  if (QFileInfo::exists(outPath)) {
    return true;
  }
  QDir().mkpath(QFileInfo(outPath).absolutePath());

  const QString format = state.format.trimmed().toLower();
  QStringList args = {QString("-format=%1").arg(format.isEmpty() ? "ppm" : format),
                      QString("-page=%1").arg(index + 1)};
  if (state.dpi > 0) {
    args << "-dpi" << QString::number(state.dpi);
  }
  args << state.sourcePath << outPath;

  QProcess proc;
  proc.start(state.ddjvuPath, args);
  if (!proc.waitForFinished(30000)) {
    proc.kill();
    return false;
  }
  if (proc.exitStatus() == QProcess::NormalExit && proc.exitCode() == 0) {
    if (!QFileInfo::exists(outPath)) {
      return false;
    }
    if (state.rotation == 0) {
      return true;
    }
    QImage image(outPath);
    if (image.isNull()) {
      qWarning() << "DjvuProvider: rotation load failed" << outPath;
      return true;
    }
    QTransform transform;
    transform.rotate(state.rotation);
    QImage rotated = image.transformed(transform);
    const QByteArray fmt = (format == "tiff") ? QByteArray("TIFF") : QByteArray("PPM");
    if (!rotated.save(outPath, fmt.constData())) {
      qWarning() << "DjvuProvider: rotation save failed" << outPath;
    }
    return true;
  }
  qWarning() << "DjvuProvider: ddjvu failed"
             << "page" << (index + 1)
             << "exit" << proc.exitCode()
             << proc.readAllStandardError();

  // Retry without dpi if the tool rejects the option.
  QStringList fallbackArgs = {QString("-format=%1").arg(format.isEmpty() ? "ppm" : format),
                              QString("-page=%1").arg(index + 1),
                              state.sourcePath,
                              outPath};
  QProcess fallback;
  fallback.start(state.ddjvuPath, fallbackArgs);
  if (!fallback.waitForFinished(30000)) {
    fallback.kill();
    return false;
  }
  if (fallback.exitStatus() == QProcess::NormalExit && fallback.exitCode() == 0) {
    if (!QFileInfo::exists(outPath)) {
      return false;
    }
    if (state.rotation == 0) {
      return true;
    }
    QImage image(outPath);
    if (image.isNull()) {
      qWarning() << "DjvuProvider: rotation load failed" << outPath;
      return true;
    }
    QTransform transform;
    transform.rotate(state.rotation);
    QImage rotated = image.transformed(transform);
    const QByteArray fmt = (format == "tiff") ? QByteArray("TIFF") : QByteArray("PPM");
    if (!rotated.save(outPath, fmt.constData())) {
      qWarning() << "DjvuProvider: rotation save failed" << outPath;
    }
    return true;
  }
  qWarning() << "DjvuProvider: ddjvu fallback failed"
             << "page" << (index + 1)
             << "exit" << fallback.exitCode()
             << fallback.readAllStandardError();
  return false;
}

void touchCache(DjvuRenderState &state, int index) {
  if (state.cached.contains(index)) {
    if (state.cachePolicy == "lru") {
      state.cacheOrder.removeAll(index);
      state.cacheOrder.append(index);
    }
    return;
  }
  state.cached.insert(index);
  state.cacheOrder.append(index);
  while (state.cacheOrder.size() > state.cacheLimit && !state.cacheOrder.isEmpty()) {
    const int drop = state.cacheOrder.takeFirst();
    state.cached.remove(drop);
    if (drop >= 0 && drop < state.images.size()) {
      QFile::remove(state.images.at(drop));
    }
  }
}

class DjvuDocument final : public FormatDocument {
public:
  DjvuDocument(QString title, QString text, std::shared_ptr<DjvuRenderState> state)
      : m_title(std::move(title)), m_text(std::move(text)), m_state(std::move(state)) {}

  ~DjvuDocument() override {
    if (m_state) {
      QMutexLocker locker(&m_state->mutex);
      m_state->alive = false;
      m_state->onImageReady = nullptr;
    }
  }

  QString title() const override { return m_title; }
  QStringList chapterTitles() const override { return {}; }
  QString readAllText() const override { return m_text; }
  QStringList imagePaths() const override { return m_state ? m_state->images : QStringList{}; }

  bool ensureImage(int index) override {
    if (!m_state) {
      return false;
    }
    const int total = m_state->images.size();
    if (index < 0 || index >= total) {
      return false;
    }

    int start = std::max(0, index - m_state->prefetchDistance);
    int end = std::min(total - 1, index + m_state->prefetchDistance);

    bool queued = false;
    for (int i = start; i <= end; ++i) {
      queued = queueRender(i) || queued;
    }
    return queued;
  }

  void setImageReadyCallback(std::function<void(int)> callback) override {
    if (!m_state) {
      return;
    }
    QMutexLocker locker(&m_state->mutex);
    m_state->onImageReady = std::move(callback);
  }

private:
  bool queueRender(int index) {
    if (!m_state) {
      return false;
    }
    {
      QMutexLocker locker(&m_state->mutex);
      if (!m_state->alive) {
        return false;
      }
      if (m_state->cached.contains(index) || m_state->inFlight.contains(index)) {
        return false;
      }
      m_state->inFlight.insert(index);
    }

    std::shared_ptr<DjvuRenderState> state = m_state;
    runDjvuTask([state, index]() {
      if (!state) {
        return;
      }
      const bool ok = renderDjvuPage(*state, index);
      std::function<void(int)> callback;
      {
        QMutexLocker locker(&state->mutex);
        state->inFlight.remove(index);
        if (!state->alive) {
          return;
        }
        if (ok) {
          touchCache(*state, index);
        }
        callback = state->onImageReady;
      }
      if (ok && callback) {
        callback(index);
      }
    });

    return true;
  }

  QString m_title;
  QString m_text;
  std::shared_ptr<DjvuRenderState> m_state;
};
} // namespace

QString DjvuProvider::name() const { return "DJVU"; }

QStringList DjvuProvider::supportedExtensions() const { return {"djvu", "djv"}; }

std::unique_ptr<FormatDocument> DjvuProvider::open(const QString &path, QString *error) {
  const QString djvusedPath = findTool("djvused");
  const QString ddjvuPath = findTool("ddjvu");
  if (djvusedPath.isEmpty() || ddjvuPath.isEmpty()) {
    if (error) {
      *error = "DjVu support requires djvulibre tools (djvused, ddjvu)";
    }
    qWarning() << "DjvuProvider: missing djvulibre tools";
    return nullptr;
  }

  const int pages = djvuPageCount(djvusedPath, path);
  if (pages <= 0) {
    if (error) {
      *error = "Failed to read DjVu page count";
    }
    qWarning() << "DjvuProvider: could not read page count" << path;
    return nullptr;
  }

  const QFileInfo info(path);
  const DjvuSettings settings = loadDjvuSettings();

  const QString outDir = tempDirFor(info);
  QDir().mkpath(outDir);

  const QString ext = formatExtension(settings.format);
  QStringList images;
  images.reserve(pages);
  for (int i = 0; i < pages; ++i) {
    images.append(QDir(outDir).filePath(QString("page_%1.%2")
                                            .arg(i + 1, 4, 10, QChar('0'))
                                            .arg(ext)));
  }

  auto state = std::make_shared<DjvuRenderState>();
  state->sourcePath = info.absoluteFilePath();
  state->images = images;
  state->tempDir = outDir;
  state->ddjvuPath = ddjvuPath;
  state->dpi = settings.dpi;
  state->cacheLimit = settings.cacheLimit;
  state->prefetchDistance = settings.prefetchDistance;
  state->cachePolicy = settings.cachePolicy;
  state->format = settings.format;
  state->rotation = settings.rotation;

  QString text;
  if (settings.extractText) {
    const QString djvutxtPath = findTool("djvutxt");
    if (djvutxtPath.isEmpty()) {
      qWarning() << "DjvuProvider: djvutxt not available";
    } else {
      text = djvuText(djvutxtPath, path);
    }
  }
  const QString title = info.completeBaseName();

  qInfo() << "DjvuProvider: pages" << pages << "dpi" << settings.dpi
          << "format" << settings.format << "prefetch" << settings.prefetchDistance
          << "cache" << settings.cacheLimit;

  return std::make_unique<DjvuDocument>(title, text, state);
}
