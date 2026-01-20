#pragma once

#include "cli_options.hpp"
#include "matcher.hpp"
#include "result.hpp"
#include <lib/thread_pool/blocking_queue.hpp>

#include <filesystem>

namespace fs = std::filesystem;

namespace NParallelGrep::NScanner {
    void ScanFile(
        const NCli::TCliOptions& opts,
        const fs::path& path,
        const std::string& fileName,
        const NMatcher::IMatcher& matcher,
        NLib::NThreadPool::TBlockingQueue<NResult::TResult>& out,
        std::atomic<bool>& cancelFlg
    );
}