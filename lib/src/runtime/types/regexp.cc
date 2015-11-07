#include <puppet/runtime/values/value.hpp>
#include <puppet/cast.hpp>
#include <boost/functional/hash.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    regexp::regexp(std::string pattern) :
        _pattern(rvalue_cast(pattern))
    {
    }

    std::string const& regexp::pattern() const
    {
        return _pattern;
    }

    char const* regexp::name()
    {
        return "Regexp";
    }

    bool regexp::is_instance(values::value const& value) const
    {
        auto ptr = value.as<values::regex>();
        if (!ptr) {
            return false;
        }
        if (_pattern.empty()) {
            return true;
        }
        return ptr->pattern() == _pattern;
    }

    bool regexp::is_specialization(values::type const& other) const
    {
        // If this Regexp has a pattern, the other type cannot be a specialization
        if (!_pattern.empty()) {
            return false;
        }
        // Check that the other type has a pattern
        auto rgx = boost::get<regexp>(&other);
        return rgx && !rgx->pattern().empty();
    }

    ostream& operator<<(ostream& os, regexp const& type)
    {
        os << regexp::name();
        if (type.pattern().empty()) {
            return os;
        }
        os << "[/" << type.pattern() << "/]";
        return os;
    }

    bool operator==(regexp const& left, regexp const& right)
    {
        return left.pattern() == right.pattern();
    }

    bool operator!=(regexp const& left, regexp const& right)
    {
        return !(left == right);
    }

    size_t hash_value(regexp const& type)
    {
        static const size_t name_hash = boost::hash_value(regexp::name());

        size_t seed = 0;
        boost::hash_combine(seed, name_hash);
        boost::hash_combine(seed, type.pattern());
        return seed;
    }

}}}  // namespace puppet::runtime::types
