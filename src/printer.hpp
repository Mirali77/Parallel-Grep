#pragma once

#include "result.hpp"
#include "cli_options.hpp"
#include "lib/thread_pool/blocking_queue.hpp"

#include <thread>

namespace NParallelGrep::NPrinter {
    class TPrinter {
    public:
        TPrinter(
            NLib::NThreadPool::TBlockingQueue<NParallelGrep::NResult::TResult>& out,
            NParallelGrep::NCli::TCliOptions& opts
        );

    private:
        std::thread Worker;
    };
}