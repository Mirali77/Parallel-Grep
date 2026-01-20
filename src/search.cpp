#include "search.hpp"

#include <lib/semaphore/semaphore.hpp>

#include <filesystem>

namespace NParallelGrep::NSearch {
    void RunSearchSequentialWalk(
        NCli::TCliOptions& opts,
        NLib::NThreadPool::TThreadPool& threadPool,
        const NMatcher::IMatcher& matcher,
        NLib::NThreadPool::TBlockingQueue<NResult::TResult>& out,
        std::atomic<bool>& cancelFlg
    ) {
        std::filesystem::path root(opts.Root);

        const uint32_t jobs = opts.JobsCount > 0 ? opts.JobsCount :  std::thread::hardware_concurrency();
        const uint32_t tasksLimit = std::max(2000u, jobs * 256);

        NLib::NSemaphore::TSemaphore semaphore(tasksLimit);
    }
}