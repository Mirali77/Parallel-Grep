#include "search.hpp"
#include "scanner.hpp"

#include <lib/semaphore/semaphore.hpp>
#include <lib/pending_manager/pending_manager.hpp>

#include <filesystem>\

namespace fs = std::filesystem;

namespace {
    using namespace NParallelGrep;

    bool AnyGlobMatch(const std::vector<std::string>& globs, const std::string& s) {
        for (const auto& g : globs) {
            if (fnmatch(g.c_str(), s.c_str(), FNM_PATHNAME) == 0) {
                return true;
            }
        }
        return false;
    }

    bool IsHidden(const fs::path& path) {
        auto name = path.filename().string();
        return !name.empty() && name[0] == '.';
    }

    bool IsExcluded(const NCli::TCliOptions& opts, const std::string& fileName) {
        return AnyGlobMatch(opts.ExcludeGlobs, fileName);
    }

    bool IsIncluded(const NCli::TCliOptions& opts, const std::string& fileName) {
        if (opts.IncludeGlobs.empty()) {
            return true;
        }
        return AnyGlobMatch(opts.IncludeGlobs, fileName);
    }
}

namespace NParallelGrep::NSearch {
    void RunSearchSequentialWalk(
        NCli::TCliOptions& opts,
        NLib::NThreadPool::TThreadPool& threadPool,
        const NMatcher::IMatcher& matcher,
        NLib::NThreadPool::TBlockingQueue<NResult::TResult>& out,
        std::atomic<bool>& cancelFlg
    ) {
        fs::path root(opts.Root);

        const uint32_t jobs = opts.JobsCount > 0 ? opts.JobsCount :  std::thread::hardware_concurrency();
        const uint32_t tasksLimit = std::max(2000u, jobs * 256);

        NLib::NSemaphore::TSemaphore semaphore(tasksLimit);
        NLib::NPendingManager::TPendingManager pendingManager;

        std::error_code errorCode;
        if (fs::is_regular_file(root, errorCode)) {
            std::string fileName = root.filename().string();

            pendingManager.Increment();
            semaphore.Acquire();
            
            threadPool.Post([&]{
                NScanner::ScanFile(opts, root, fileName, matcher, out, cancelFlg);
                semaphore.Release();
                pendingManager.Decrement();
            });

            pendingManager.Wait();
            out.Close();
            return;
        }

        if (errorCode.clear(); !fs::is_directory(root, errorCode)) {
            out.Close();
            return;
        }

        fs::directory_options dirOpts = fs::directory_options::skip_permission_denied;
        if (opts.FollowSymlinks) {
            dirOpts |= fs::directory_options::follow_directory_symlink;
        }

        for (
            fs::recursive_directory_iterator iter(root, dirOpts, errorCode), end;
            !errorCode && iter != end;
            iter.increment(errorCode)
        ) {
            if (cancelFlg.load(std::memory_order_relaxed)) {
                break;
            }

            auto disableDiving = [&iter]{
                std::error_code errorCode;
                if (iter->is_directory(errorCode) && !errorCode) {
                    iter.disable_recursion_pending();
                }
            };

            fs::path path = iter->path();
            if (!opts.ProcessHiddenFiles && IsHidden(path)) {
                disableDiving();
                continue;
            }
            
            std::string fileName;
            {
                std::error_code errorCode;
                fileName = fs::relative(path, root, errorCode).generic_string();
                if (errorCode) {
                    fileName = path.generic_string();
                }
            }

            if (IsExcluded(opts, fileName)) {
                disableDiving();
                continue;
            }

            if (std::error_code errorCode; iter->is_regular_file(errorCode) && !errorCode) {
                if (!IsIncluded(opts, fileName)) {
                    continue;
                }

                pendingManager.Increment();
                semaphore.Acquire();
                
                if (
                    !threadPool.Post([&]{
                        NScanner::ScanFile(opts, path, fileName, matcher, out, cancelFlg);
                        semaphore.Release();
                        pendingManager.Decrement();
                    })
                ) {
                    semaphore.Release();
                    pendingManager.Decrement();
                    break;
                }
            }
        }

        pendingManager.Wait();
        out.Close();
    }
}
