#include "include/UpdateManager.h"
#include "include/AppPaths.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QProcessEnvironment>
#include <QStandardPaths>

namespace {
QString trimNewlines(const QString &value) {
  QString out = value;
  while (out.endsWith('\n') || out.endsWith('\r')) {
    out.chop(1);
  }
  return out.trimmed();
}
} // namespace

UpdateManager::UpdateManager(QObject *parent) : QObject(parent) {
  m_process = new QProcess(this);
  connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
          this, &UpdateManager::handleProcessFinished);
}

UpdateManager::State UpdateManager::state() const { return m_state; }
QString UpdateManager::status() const { return m_status; }
QString UpdateManager::summary() const { return m_summary; }
QString UpdateManager::details() const { return m_details; }

void UpdateManager::checkForUpdates() {
  if (m_state == State::Checking) {
    return;
  }
  const QString flatpakId = QProcessEnvironment::systemEnvironment().value("FLATPAK_ID").trimmed();
  if (!flatpakId.isEmpty()) {
    setState(State::Unavailable);
    setStatus("Updates are handled by Flatpak");
    setSummary({});
    setDetails({});
    return;
  }
  const QString git = QStandardPaths::findExecutable("git");
  if (git.isEmpty()) {
    setState(State::Unavailable);
    setStatus("Updates require git (not installed)");
    setSummary({});
    setDetails({});
    return;
  }

  m_gitRoot = findGitRoot();
  if (m_gitRoot.isEmpty()) {
    setState(State::Unavailable);
    setStatus("Updates require a git checkout");
    setSummary({});
    setDetails({});
    return;
  }

  setState(State::Checking);
  setStatus("Looking for update...");
  setSummary({});
  setDetails({});
  m_step = Step::Fetch;
  runGit({"fetch", "--tags"});
}

void UpdateManager::setState(State state) {
  if (m_state == state) {
    return;
  }
  m_state = state;
  emit stateChanged();
}

void UpdateManager::setStatus(const QString &status) {
  if (m_status == status) {
    return;
  }
  m_status = status;
  emit statusChanged();
}

void UpdateManager::setSummary(const QString &summary) {
  if (m_summary == summary) {
    return;
  }
  m_summary = summary;
  emit summaryChanged();
}

void UpdateManager::setDetails(const QString &details) {
  if (m_details == details) {
    return;
  }
  m_details = details;
  emit detailsChanged();
}

QString UpdateManager::findGitRoot() const {
  QDir dir(QCoreApplication::applicationDirPath());
  for (int i = 0; i < 8; ++i) {
    if (dir.exists(".git")) {
      return dir.absolutePath();
    }
    if (!dir.cdUp()) {
      break;
    }
  }
  return {};
}

void UpdateManager::runGit(const QStringList &args) {
  if (!m_process) {
    return;
  }
  m_process->setWorkingDirectory(m_gitRoot);
  m_process->start("git", args);
}

void UpdateManager::handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
  if (exitStatus != QProcess::NormalExit || exitCode != 0) {
    setState(State::Error);
    setStatus("Update check failed");
    setSummary({});
    setDetails(trimNewlines(QString::fromUtf8(m_process->readAllStandardError())));
    m_step = Step::None;
    return;
  }

  const QString output = trimNewlines(QString::fromUtf8(m_process->readAllStandardOutput()));

  if (m_step == Step::Fetch) {
    m_step = Step::Upstream;
    runGit({"rev-parse", "--abbrev-ref", "--symbolic-full-name", "@{u}"});
    return;
  }

  if (m_step == Step::Upstream) {
    if (output.isEmpty()) {
      setState(State::Unavailable);
      setStatus("No upstream configured");
      setSummary({});
      setDetails({});
      m_step = Step::None;
      return;
    }
    m_upstreamRef = output;
    m_step = Step::Ahead;
    runGit({"rev-list", "--count", "HEAD..@{u}"});
    return;
  }

  if (m_step == Step::Ahead) {
    bool ok = false;
    m_aheadCount = output.toInt(&ok);
    if (!ok) {
      setState(State::Error);
      setStatus("Update check failed");
      setSummary({});
      setDetails(output);
      m_step = Step::None;
      return;
    }
    if (m_aheadCount <= 0) {
      setState(State::UpToDate);
      setStatus("Up to date");
      setSummary({});
      setDetails({});
      m_step = Step::None;
      return;
    }
    m_step = Step::Log;
    runGit({"log", "--oneline", "-n", "5", "HEAD..@{u}"});
    return;
  }

  if (m_step == Step::Log) {
    setState(State::UpdateAvailable);
    setStatus("Update found");
    setSummary(QString("%1 update(s) available").arg(m_aheadCount));
    setDetails(output);
    m_step = Step::None;
    return;
  }
}
