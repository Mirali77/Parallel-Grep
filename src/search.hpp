#pragma once

#include "cli_options.hpp"
#include "matcher.hpp"
#include "result.hpp"
#include <lib/thread_pool/thread_pool.hpp>
#include <lib/thread_pool/blocking_queue.hpp>

namespace NParallelGrep::NSearch {
    void RunSearchSequentialWalk(
        NCli::TCliOptions& opts,
        NLib::NThreadPool::TThreadPool& threadPool,
        const NMatcher::IMatcher& matcher,
        NLib::NThreadPool::TBlockingQueue<NResult::TResult>& out,
        std::atomic<bool>& cancelFlg
    );
}