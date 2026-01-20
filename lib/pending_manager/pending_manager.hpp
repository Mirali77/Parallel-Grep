#pragma once

#include <mutex>
#include <condition_variable>

namespace NLib::NPendingManager {
    class TPendingManager {
    public:
        void Increment();
        void Decrement();
        void Wait();

    private:
        std::mutex Mutex_;
        std::condition_variable CondVar_;
        size_t Counter_ = 0;
    };
}
