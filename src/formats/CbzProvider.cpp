#include "CbzProvider.h"

#include <QDir>
#include <QCryptographicHash>
#include <QFileInfo>
#include <QStandardPaths>
#include <QCollator>
#include <QProcess>
#include <QDebug>

#ifdef HAVE_LIBARCHIVE
#include <archive.h>
#include <archive_entry.h>
#endif

#include "miniz.h"

namespace {
class CbzDocument final : public FormatDocument {
public:
  CbzDocument(QString title, QStringList images)
      : m_title(std::move(title)), m_images(std::move(images)) {}

  QString title() const override { return m_title; }
  QStringList chapterTitles() const override { return {}; }
  QString readAllText() const override { return {}; }
  QStringList imagePaths() const override { return m_images; }

private:
  QString m_title;
  QStringList m_images;
};

struct ZipReader {
  mz_zip_archive archive{};
  bool ok = false;

  explicit ZipReader(const QString &path) {
    memset(&archive, 0, sizeof(archive));
    ok = mz_zip_reader_init_file(&archive, path.toUtf8().constData(), 0) != 0;
  }

  ~ZipReader() {
    if (ok) {
      mz_zip_reader_end(&archive);
    }
  }
};

bool isImageFile(const QString &name) {
  const QString ext = QFileInfo(name).suffix().toLower();
  return ext == "jpg" || ext == "jpeg" || ext == "png" || ext == "webp" || ext == "bmp";
}

QString tempDirFor(const QString &path) {
  const QByteArray hash = QCryptographicHash::hash(path.toUtf8(), QCryptographicHash::Sha1).toHex();
  return QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation))
      .filePath(QString("ereader_cbz_%1").arg(QString::fromUtf8(hash)));
}

void naturalSort(QStringList &paths) {
  QCollator collator;
  collator.setNumericMode(true);
  collator.setCaseSensitivity(Qt::CaseInsensitive);
  std::sort(paths.begin(), paths.end(), [&collator](const QString &a, const QString &b) {
    return collator.compare(a, b) < 0;
  });
}

bool extractCbrWithTool(const QString &archivePath, const QString &outDir) {
  // Try common tools in order: bsdtar, unrar, unar
  struct Tool {
    QString program;
    QStringList args;
  };
  const QList<Tool> tools = {
      {"bsdtar", {"-xf", archivePath, "-C", outDir}},
      {"unrar", {"x", "-o+", archivePath, outDir}},
      {"unar", {"-o", outDir, archivePath}},
  };

  for (const auto &tool : tools) {
    QProcess process;
    process.start(tool.program, tool.args);
    if (!process.waitForFinished(30000)) {
      continue;
    }
    if (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0) {
      return true;
    }
  }
  return false;
}

#ifdef HAVE_LIBARCHIVE
bool extractWithLibarchive(const QString &archivePath, const QString &outDir) {
  struct archive *ar = archive_read_new();
  archive_read_support_format_all(ar);
  archive_read_support_filter_all(ar);

  if (archive_read_open_filename(ar, archivePath.toUtf8().constData(), 10240) != ARCHIVE_OK) {
    archive_read_free(ar);
    return false;
  }

  struct archive_entry *entry = nullptr;
  while (archive_read_next_header(ar, &entry) == ARCHIVE_OK) {
    const char *path = archive_entry_pathname(entry);
    if (!path) {
      archive_read_data_skip(ar);
      continue;
    }
    const QString name = QString::fromUtf8(path);
    if (!isImageFile(name)) {
      archive_read_data_skip(ar);
      continue;
    }
    const QString outPath = QDir(outDir).filePath(name);
    QDir().mkpath(QFileInfo(outPath).absolutePath());
    archive_entry_set_pathname(entry, outPath.toUtf8().constData());
    archive_read_extract(ar, entry, ARCHIVE_EXTRACT_TIME);
  }

  archive_read_close(ar);
  archive_read_free(ar);
  return true;
}
#endif
} // namespace

QString CbzProvider::name() const { return "CBZ"; }

QStringList CbzProvider::supportedExtensions() const { return {"cbz", "cbr"}; }

std::unique_ptr<FormatDocument> CbzProvider::open(const QString &path, QString *error) {
  const QString ext = QFileInfo(path).suffix().toLower();
  if (ext == "cbr") {
    const QString outDir = tempDirFor(path);
    QDir().mkpath(outDir);
    bool extracted = false;
#ifdef HAVE_LIBARCHIVE
    extracted = extractWithLibarchive(path, outDir);
#endif
    if (!extracted) {
      extracted = extractCbrWithTool(path, outDir);
    }
    if (!extracted) {
      if (error) {
        *error = "CBR extraction failed (install libarchive/bsdtar/unrar/unar)";
      }
      qWarning() << "CbzProvider: CBR extraction failed";
      return nullptr;
    }
    QStringList images;
    const QDir dir(outDir);
    const auto files = dir.entryList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
    for (const auto &name : files) {
      if (isImageFile(name)) {
        images.append(dir.filePath(name));
      }
    }
    naturalSort(images);
    if (images.isEmpty()) {
      if (error) {
        *error = "No images found in CBR";
      }
      return nullptr;
    }
    const QString title = QFileInfo(path).completeBaseName();
    return std::make_unique<CbzDocument>(title, images);
  }

  ZipReader zip(path);
  if (!zip.ok) {
    if (error) {
      *error = "Failed to open CBZ";
    }
    return nullptr;
  }

  const QString outDir = tempDirFor(path);
  QDir().mkpath(outDir);

  const int count = mz_zip_reader_get_num_files(&zip.archive);
  QStringList extracted;
  extracted.reserve(count);
  for (int i = 0; i < count; ++i) {
    mz_zip_archive_file_stat stat{};
    if (!mz_zip_reader_file_stat(&zip.archive, i, &stat)) {
      continue;
    }
    const QString name = QString::fromUtf8(stat.m_filename);
    if (!isImageFile(name)) {
      continue;
    }
    const QString outPath = QDir(outDir).filePath(name);
    QDir().mkpath(QFileInfo(outPath).absolutePath());
    if (!mz_zip_reader_extract_to_file(&zip.archive, i, outPath.toUtf8().constData(), 0)) {
      continue;
    }
    extracted.append(outPath);
  }

  extracted.sort();
  naturalSort(extracted);
  if (extracted.isEmpty()) {
    if (error) {
      *error = "No images found in CBZ";
    }
    return nullptr;
  }

  const QString title = QFileInfo(path).completeBaseName();
  return std::make_unique<CbzDocument>(title, extracted);
}
