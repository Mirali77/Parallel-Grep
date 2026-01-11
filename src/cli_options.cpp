#include "cli_options.h"

#include <getopt.h>
#include <iostream>
#include <limits>

namespace {
    static void PrintHelp(const char* prog) {
        std::cerr <<
            "Usage: " << prog << " [OPTIONS] PATTERN ROOT\n\n"
            "OPTIONS:\n"
            "  -j, --jobs N           number of worker threads (default: auto)\n"
            "  -r, --regex            treat PATTERN as regex (default: literal substring)\n"
            "      --max-size BYTES   skip files larger than BYTES (default: 64 * 1024 * 1024 (64MiB))\n"
            "      --include GLOB     include only paths matching glob (repeatable)\n"
            "      --exclude GLOB     exclude paths matching glob (repeatable)\n"
            "      --hidden           include hidden files/directories\n"
            "      --follow-symlinks  follow symlinks\n"
            "      --binary MODE      skip|text (default: skip)\n"
            "  -h, --help             show this help\n";
    }

    static std::uint64_t ParseUInt64(const char* s, const char* what) {
        if (!s || !*s) throw std::runtime_error(std::string("missing value for ") + what);
        char* end = nullptr;
        errno = 0;
        unsigned long long v = std::strtoull(s, &end, 10);
        if (errno != 0 || end == s || *end != '\0')
            throw std::runtime_error(std::string("invalid number for ") + what + ": " + s);
        return static_cast<std::uint64_t>(v);
    }

    static int ParseIntPos(const char* s, const char* what) {
        auto v = ParseUInt64(s, what);
        if (v == 0 || v > static_cast<std::uint64_t>(std::numeric_limits<int>::max()))
        throw std::runtime_error(std::string(what) + " must be in int range");
        return static_cast<int>(v);
    }

    static NParallelGrep::NCli::EBinaryPolicy ParseBinaryPolicy(const char* s) {
        if (s == "skip") return NParallelGrep::NCli::EBinaryPolicy::Skip;
        if (s == "text") return NParallelGrep::NCli::EBinaryPolicy::Text;
        throw std::runtime_error(std::string("invalid --binary mode: ") + s + ". Use skip|text");
    }
}

namespace NParallelGrep::NCli {
    TCliOptions ParseCli(int argc, char** argv) {
        TCliOptions opts;

        const char* prog = (argc > 0 ? argv[0] : "pgrep");

        enum {
            OPT_MAX_SIZE = 1000,
            OPT_INCLUDE,
            OPT_EXCLUDE,
            OPT_HIDDEN,
            OPT_FOLLOW_SYMLINKS,
            OPT_BINARY
        };

        static option optsToParse [] = {
            {"jobs", required_argument, nullptr, 'j'},
            {"regex", no_argument, nullptr, 'r'},
            {"help", no_argument, nullptr, 'h'},
            {"max-size", required_argument, nullptr, OPT_MAX_SIZE},
            {"include", required_argument, nullptr, OPT_INCLUDE},
            {"exclude", required_argument, nullptr, OPT_EXCLUDE},
            {"hidden", no_argument, nullptr, OPT_HIDDEN},
            {"follow-symlinks", no_argument, nullptr, OPT_FOLLOW_SYMLINKS},
            {"binary", required_argument, nullptr, OPT_BINARY},
            {nullptr, 0, nullptr, 0}
        };

        optind = 1;

        while (true) {
            int idx = 0;
            int c = getopt_long(argc, argv, "j:rnh", optsToParse, &idx);
            if (c == -1) break;

            switch (c) {
                case 'j':
                    opts.JobsCount = ParseIntPos(optarg, "--jobs");
                    break;
                case 'r':
                    opts.UseRegex = true;
                    break;
                case 'h':
                    PrintHelp(prog);
                    std::exit(0);
                case OPT_MAX_SIZE:
                    opts.MaxFileSize = ParseUInt64(optarg, "--max-size");
                    break;
                case OPT_INCLUDE:
                    opts.IncludeGlobs.emplace_back(optarg);
                    break;
                case OPT_EXCLUDE:
                    opts.ExcludeGlobs.emplace_back(optarg);
                    break;
                case OPT_HIDDEN:
                    opts.ProcessHiddenFiles = true;
                    break;
                case OPT_FOLLOW_SYMLINKS:
                    opts.FollowSymlinks = true;
                    break;
                case OPT_BINARY:
                    opts.BinaryFilesPolicy = ParseBinaryPolicy(optarg);
                    break;
                case '?':
                default:
                    PrintHelp(prog);
                    throw std::runtime_error("invalid arguments");
            }
        }

        int remaining = argc - optind;
        if (remaining != 2) {
            PrintHelp(prog);
            throw std::runtime_error("expected PATTERN and ROOT");
        }
        opts.Pattern = argv[optind];
        opts.Root = argv[optind + 1];

        return opts;
    }
}