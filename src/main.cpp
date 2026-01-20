#include "cli_options.hpp"
#include "matcher.hpp"
#include "result.hpp"
#include "printer.hpp"
#include "search.hpp"

#include <lib/thread_pool/blocking_queue.hpp>
#include <lib/thread_pool/thread_pool.hpp>

#include <iostream>
#include <csignal>

static std::atomic<bool>* cancelFlgPtr = nullptr;

static void OnSigint(int) {
  if (cancelFlgPtr) cancelFlgPtr->store(true, std::memory_order_relaxed);
}

int main(int argc, char* argv[]) {
    using namespace NParallelGrep;

    try {
        auto opts = NParallelGrep::NCli::ParseCli(argc, argv);

        NLib::NThreadPool::TThreadPool threadPool(
            opts.JobsCount > 0
                ? opts.JobsCount
                : std::thread::hardware_concurrency()
        );
        NLib::NThreadPool::TBlockingQueue<NResult::TResult> out;

        std::atomic<bool> cancelFlg{false};
        cancelFlgPtr = &cancelFlg;
        std::signal(SIGINT, OnSigint);

        std::unique_ptr<NMatcher::IMatcher> matcher;
        if (opts.UseRegex) {
            matcher = std::make_unique<NMatcher::TRegexMatcher>(opts.Pattern);
        } else {
            matcher = std::make_unique<NMatcher::TLiteralMatcher>(opts.Pattern);
        }

        NPrinter::TPrinter printer(out, opts);

        NSearch::RunSearchSequentialWalk(opts, threadPool, *matcher, out, *cancelFlgPtr);

        printer.Close();
        threadPool.Shutdown();

        return cancelFlg.load(std::memory_order_relaxed) ? 130 : 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 2;
    }
    return 0;
}
