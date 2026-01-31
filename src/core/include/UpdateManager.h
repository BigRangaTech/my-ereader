#pragma once

#include <QObject>
#include <QString>
#include <QProcess>

class UpdateManager : public QObject {
  Q_OBJECT
  Q_PROPERTY(State state READ state NOTIFY stateChanged)
  Q_PROPERTY(QString status READ status NOTIFY statusChanged)
  Q_PROPERTY(QString summary READ summary NOTIFY summaryChanged)
  Q_PROPERTY(QString details READ details NOTIFY detailsChanged)

public:
  enum class State {
    Idle,
    Checking,
    UpToDate,
    UpdateAvailable,
    Unavailable,
    Error
  };
  Q_ENUM(State)

  explicit UpdateManager(QObject *parent = nullptr);

  State state() const;
  QString status() const;
  QString summary() const;
  QString details() const;

  Q_INVOKABLE void checkForUpdates();

signals:
  void stateChanged();
  void statusChanged();
  void summaryChanged();
  void detailsChanged();

private:
  void setState(State state);
  void setStatus(const QString &status);
  void setSummary(const QString &summary);
  void setDetails(const QString &details);

  QString findGitRoot() const;
  void runGit(const QStringList &args);
  void handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

  enum class Step {
    None,
    Fetch,
    Upstream,
    Ahead,
    Log
  };

  State m_state = State::Idle;
  QString m_status = "Idle";
  QString m_summary;
  QString m_details;
  QString m_gitRoot;
  QString m_upstreamRef;
  int m_aheadCount = 0;
  Step m_step = Step::None;
  class QProcess *m_process = nullptr;
};
