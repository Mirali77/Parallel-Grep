#include "task_tracker.hpp"

namespace NLib::NTaskTracker {

    TTaskTracker::TTaskTracker(NThreadPool::TThreadPool& threadPool)
        : ThreadPool_(threadPool)
        , PendingCount_(1)
    {
    }

    void TTaskTracker::Wait() {
        FinishOne();

        std::unique_lock lock(Mutex_);
        CondVar_.wait(lock, [&] { return DoneFlg_; });
    }

    void TTaskTracker::FinishOne() {
        std::unique_lock lock(Mutex_);

        if (--PendingCount_ == 0) {
            DoneFlg_ = true;
            lock.unlock();
            CondVar_.notify_all();
        }
    }
}
