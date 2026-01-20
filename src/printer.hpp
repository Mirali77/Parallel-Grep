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

        void Close();

    private:
        bool CountOnlyFlg_;
        std::thread Worker_;
        size_t Counter_ = 0;
    };
}