#include "include/AsyncUtil.h"

#include <QThreadPool>
#include <QRunnable>

namespace {
class LambdaRunnable final : public QRunnable {
public:
  explicit LambdaRunnable(std::function<void()> task) : m_task(std::move(task)) {}
  void run() override {
    if (m_task) {
      m_task();
    }
  }

private:
  std::function<void()> m_task;
};
} // namespace

void runInBackground(std::function<void()> task) {
  auto *runnable = new LambdaRunnable(std::move(task));
  runnable->setAutoDelete(true);
  QThreadPool::globalInstance()->start(runnable);
}
