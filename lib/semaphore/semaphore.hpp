#pragma once

#include <cstddef>
#include <mutex>
#include <condition_variable>

namespace NSemaphore {
    class TSemaphore {
    public:
        explicit TSemaphore(std::ptrdiff_t initial = 0);

        void Release(std::ptrdiff_t n = 1);
        void Acquire();
        bool TryAcquire();

    private:
        std::mutex Mutex_;
        std::condition_variable CondVar_;
        std::ptrdiff_t Count_;
    };
}