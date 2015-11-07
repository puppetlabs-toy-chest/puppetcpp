#include <puppet/runtime/values/value.hpp>
#include <puppet/cast.hpp>
#include <boost/functional/hash.hpp>

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

    char const* pattern::name()
    {
        return "Pattern";
    }

    bool pattern::is_instance(values::value const& value) const
    {
        // Check for string
        auto ptr = value.as<std::string>();
        if (!ptr) {
            return false;
        }

        // Check for no patterns (accept any string)
        if (_patterns.empty()) {
            return true;
        }

        // Check for a matching pattern
        for (auto const& regex : _patterns) {
            if (regex.pattern().empty() || std::regex_search(*ptr, regex.value())) {
                return true;
            }
        }
        return false;
    }

    bool pattern::is_specialization(values::type const& other) const
    {
        // Specializations of Pattern have *fewer* patterns (i.e. are more restrictive)
        auto ptr = boost::get<pattern>(&other);
        if (!ptr) {
            return false;
        }
        return ptr->patterns().size() < _patterns.size();
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
            if (left_pattern != right_pattern) {
                return false;
            }
        }
        return true;
    }

    bool operator!=(pattern const& left, pattern const& right)
    {
        return !(left == right);
    }

    size_t hash_value(pattern const& type)
    {
        static const size_t name_hash = boost::hash_value(pattern::name());

        size_t seed = 0;
        boost::hash_combine(seed, name_hash);
        boost::hash_range(seed, type.patterns().begin(), type.patterns().end());
        return seed;
    }

}}}  // namespace puppet::runtime::types
