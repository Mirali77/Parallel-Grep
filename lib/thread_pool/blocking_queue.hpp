#pragma once

#include <condition_variable>
#include <queue>
#include <mutex>

namespace NLib::NThreadPool {
    template <class T>
    class TBlockingQueue {
    public:
        bool Push(T v) {
            {
                std::lock_guard lock(Mutex_);
                if (ClosedFlg_) {
                    return false;
                }
                Queue_.push(std::move(v));
            }
            CondVar_.notify_one();
            return true;
        }

        bool Pop(T& v) {
            std::unique_lock lock(Mutex_);
            CondVar_.wait(lock, [&]{ return ClosedFlg_ || Queue_.empty(); });
            if (Queue_.empty()) { // Means that queue is closed
                return false;
            }
            v = std::move(Queue_.front());
            Queue_.pop();
            return true;
        }

        void Close() {
            {
                std::lock_guard lock(Mutex_);
                ClosedFlg_ = true;
            }
            CondVar_.notify_all();
        }

        bool IsClosed() const {
            std::lock_guard lock(Mutex_);
            return ClosedFlg_;
        }

        std::size_t Size() const {
            std::lock_guard lock(Mutex_);
            return Queue_.size();
        }

    private:
        std::queue<T> Queue_;
        mutable std::mutex Mutex_;
        std::condition_variable CondVar_;
        bool ClosedFlg_ = false;
    };
}
