#pragma once

#include <lib/thread_pool/blocking_queue.hpp>
#include <atomic>


namespace NLib::NThreadPool {
    class TThreadPool {
    public:
        explicit TThreadPool(std::size_t threads = std::thread::hardware_concurrency());
    private:
        
    };
}
