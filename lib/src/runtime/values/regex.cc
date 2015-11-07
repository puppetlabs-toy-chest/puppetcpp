#include <puppet/runtime/values/value.hpp>
#include <puppet/cast.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace values {

    regex::regex()
    {
    }

    regex::regex(string pattern) :
        _regex(pattern)
    {
        _pattern = rvalue_cast(pattern);
    }

    string const& regex::pattern() const
    {
        return _pattern;
    }

    std::regex const& regex::value() const
    {
        return _regex;
    }

    ostream& operator<<(ostream& os, regex const& regx)
    {
        os << '/' << regx.pattern() << '/';
        return os;
    }

    bool operator==(regex const& left, regex const& right)
    {
        return left.pattern() == right.pattern();
    }

    bool operator!=(regex const& left, regex const& right)
    {
        return !(left == right);
    }

    size_t hash_value(values::regex const& regex)
    {
        return boost::hash_value(regex.pattern());
    }

}}}  // namespace puppet::runtime::values
