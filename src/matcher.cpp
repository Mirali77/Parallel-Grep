#include "matcher.hpp"

namespace NMatcher {
    TLiteralMatcher::TLiteralMatcher(const std::string& pattern)
        : Pattern(pattern)
    {
    }

    bool TLiteralMatcher::Match(std::string_view s) const {
        return s.find(Pattern) != std::string_view::npos;
    }

    TRegexMatcher::TRegexMatcher(const std::string& pattern)
        : Re(pattern, std::regex::ECMAScript)
    {
    }

    bool TRegexMatcher::Match(std::string_view s) const {
        return std::regex_search(s.begin(), s.end(), Re);
    }
}
