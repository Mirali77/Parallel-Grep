#pragma once

#include <string>
#include <vector>

namespace NParallelGrep::NCli {
    enum class EBinaryPolicy { Skip, Text };

    struct TCliOptions {
        int JobsCount = 0;
        bool UseRegex = false;
        bool ProcessHiddenFiles = false;
        bool FollowSymlinks = false;
        EBinaryPolicy BinaryFilesPolicy = EBinaryPolicy::Skip;

        std::uint64_t MaxFileSize = 64ull * 1024 * 1024; // 64MiB

        std::vector<std::string> IncludeGlobs;
        std::vector<std::string> ExcludeGlobs;

        std::string Pattern;
        std::string Root;
    };

    TCliOptions ParseCli(int argc, char** argv);
}
