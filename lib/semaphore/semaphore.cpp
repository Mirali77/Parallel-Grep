#include "semaphore.hpp"

namespace NSemaphore {
    TSemaphore::TSemaphore(std::ptrdiff_t initial)
        : Count_(initial)
    {
    }

    void TSemaphore::Release(std::ptrdiff_t n) {
        {
            std::lock_guard lock(Mutex_);
            Count_ += n;
        }
        CondVar_.notify_all();
    }

    void TSemaphore::Acquire() {
        std::unique_lock lock(Mutex_);
        CondVar_.wait(lock, [&] { return Count_ > 0; });
        --Count_;
    }

    bool TSemaphore::TryAcquire() {
        std::lock_guard lock(Mutex_);
        if (Count_ <= 0) {
            return false;
        }
        --Count_;
        return true;
    }
}