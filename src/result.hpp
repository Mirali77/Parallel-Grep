#pragma once

#include <string>

namespace NParallelGrep::NResult {
    struct TResult {
        std::string FileName;
        uint64_t LineNumber;
        std::string Line;
    };
}
