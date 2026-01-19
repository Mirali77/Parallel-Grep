#pragma once

#include <string>

namespace NResult {
    struct TResult {
        std::string RelativePath;
        uint64_t LineNumber;
        std::string Line;
    };
}
