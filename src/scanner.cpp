#include "scanner.hpp"

#include <fstream>

namespace {
    bool IsBinary(std::ifstream& input) {
        char buffer[1 << 12];
        input.read(buffer, sizeof(buffer));
        std::streamsize n = input.gcount();
        for (std::streamsize i = 0; i < n; ++i) {
            if (buffer[i] == '\0') {
                return true;
            }
        }
        input.clear();
        input.seekg(0, std::ios::beg);
        return false;
    }
}

namespace NParallelGrep::NScanner {
    void ScanFile(
        const NCli::TCliOptions& opts,
        const fs::path& path,
        const std::string& fileName,
        const NMatcher::IMatcher& matcher,
        NLib::NThreadPool::TBlockingQueue<NResult::TResult>& out,
        std::atomic<bool>& cancelFlg
    ) {
        if (cancelFlg.load(std::memory_order_relaxed)) {
            return;
        }

        std::error_code errorCode;
        auto sz = fs::file_size(path, errorCode);
        if (!errorCode && sz > opts.MaxFileSize) {
            return;
        }

        std::ifstream input(path);
        if (!input) {
            return;
        }

        std::vector<char> buffer(1 << 20);
        input.rdbuf()->pubsetbuf(buffer.data(), buffer.size());

        if (opts.BinaryFilesPolicy == NCli::EBinaryPolicy::Skip) {
            if (IsBinary(input)) {
                return;
            }
        }

        std::string line;
        size_t lineNumber = 0;
        while (!cancelFlg.load(std::memory_order_relaxed) && std::getline(input, line)) {
            ++lineNumber;
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            if (matcher.Match(line)) {
                NResult::TResult result{
                    fileName,
                    lineNumber,
                    line,
                };
                out.Push(std::move(result));
            }
        }
    }
}