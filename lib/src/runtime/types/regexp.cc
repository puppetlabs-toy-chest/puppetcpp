#include <puppet/runtime/values/value.hpp>
#include <puppet/cast.hpp>
#include <boost/functional/hash.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    regexp const regexp::instance;

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

    values::type regexp::generalize() const
    {
        return *this;
    }

    bool regexp::is_instance(values::value const& value, recursion_guard& guard) const
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

    bool regexp::is_assignable(values::type const& other, recursion_guard& guard) const
    {
        auto regexp = boost::get<types::regexp>(&other);
        if (!regexp) {
            return false;
        }

        return _pattern.empty() || _pattern == regexp->_pattern;
    }

    void regexp::write(ostream& stream, bool expand) const
    {
        stream << regexp::name();
        if (_pattern.empty()) {
            return;
        }
        stream << "[/" << _pattern << "/]";
    }

    ostream& operator<<(ostream& os, regexp const& type)
    {
        type.write(os);
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
