#include "pending_manager.hpp"

namespace NLib::NPendingManager {
    void TPendingManager::Increment() {
        std::lock_guard lock(Mutex_);
        ++Counter_;
    }

    void TPendingManager::Decrement() {
        std::unique_lock lock(Mutex_);
        if (--Counter_ == 0) {
            lock.unlock();
            CondVar_.notify_all();
        }
    }

    void TPendingManager::Wait() {
        std::unique_lock lock(Mutex_);
        CondVar_.wait(lock, [&] { return Counter_ == 0; });
    }
}