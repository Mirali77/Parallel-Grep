#include "thread_pool.hpp"

namespace NLib::NThreadPool {
    TThreadPool::TThreadPool(std::size_t threads)
        : StopFlg_(false)
    {
        if (threads == 0) {
            threads = 1;
        }
        Workers_.reserve(threads);
        while (threads--) {
            Workers_.emplace_back([this] { GetWorkerLoop(); });
        }
    }

    TThreadPool::~TThreadPool() {
        Shutdown();
    }

    void TThreadPool::Shutdown() {
        if (bool expected = false; !StopFlg_.compare_exchange_strong(expected, true)) {
            std::cerr << "Thread pool is already shut down" << std::endl;
            return;
        }

        Tasks_.Close();

        for (auto& w : Workers_) {
            if (w.joinable()) {
                w.join();
            }
        }
        Workers_.clear();
    }

    void TThreadPool::GetWorkerLoop() {
        std::function<void()> task;
        while (Tasks_.Pop(task)) {
            task();
        }
    }
}