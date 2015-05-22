#include <puppet/runtime/types/pattern.hpp>
#include <puppet/cast.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    pattern::pattern(vector<values::regex> patterns) :
        _patterns(rvalue_cast(patterns))
    {
    }

    vector<values::regex> const& pattern::patterns() const
    {
        return _patterns;
    }

    const char* pattern::name()
    {
        return "Pattern";
    }

    ostream& operator<<(ostream& os, pattern const& type)
    {
        os << pattern::name();
        if (type.patterns().empty()) {
            return os;
        }
        os << '[';
        bool first = true;
        for (auto const& pattern : type.patterns()) {
            if (first) {
                first = false;
            } else {
                os << ", ";
            }
            os << pattern;
        }
        os << ']';
        return os;
    }

    bool operator==(pattern const& left, pattern const& right)
    {
        auto& left_patterns = left.patterns();
        auto& right_patterns = right.patterns();
        if (left_patterns.size() != right_patterns.size()) {
            return false;
        }
        for (size_t i = 0; i < left_patterns.size(); ++i) {
            auto& left_pattern = left_patterns[i];
            auto& right_pattern = right_patterns[i];
            if (!(left_pattern == right_pattern)) {
                return false;
            }
        }
        return true;
    }

}}}  // namespace puppet::runtime::types
