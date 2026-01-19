#pragma once

#include <string>
#include <string_view>
#include <regex>

namespace NParallelGrep::NMatcher {
    class IMatcher {
    public:
        virtual ~IMatcher() = default;
        virtual bool Match(std::string_view s) const = 0;
    };

    class TLiteralMatcher : public IMatcher {
    public:
        explicit TLiteralMatcher(const std::string& pattern);
        bool Match(std::string_view s) const override;
    private:
        std::string Pattern;
    };

    class TRegexMatcher : public IMatcher {
    public:
        explicit TRegexMatcher(const std::string& pattern);
        bool Match(std::string_view s) const override;
    private:
        std::regex Re;
    };
}