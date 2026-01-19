#pragma once

#include <lib/thread_pool/thread_pool.hpp>

#include <mutex>

namespace NLib::NTaskTracker {
    class TTaskTracker {
    // Usefull for recursive task creation chains
    public:
        explicit TTaskTracker(NThreadPool::TThreadPool& threadPool);

        template <class TFunction>
        bool Submit(TFunction&& function) {
            {
                std::lock_guard lock(Mutex_);
                if (DoneFlg_) {
                    return false;
                }
                PendingCount_++;
            }

            if (!ThreadPool_.Post([this, func = std::forward<TFunction>(function)]() mutable {
                func();
                FinishOne();
            })) { // thread pool is closed
                FinishOne();
            }

            return ok;
        }

        void Wait();

    private:
        void FinishOne();

    private:        
        NThreadPool::TThreadPool& ThreadPool_;
        std::mutex Mutex_;
        std::condition_variable CondVar_;
        size_t PendingCount_ = 0;
        bool DoneFlg_ = false;
    };
}