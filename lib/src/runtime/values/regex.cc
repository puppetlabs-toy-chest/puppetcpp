#include <puppet/runtime/values/regex.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace values {

    regex::regex()
    {
    }

    regex::regex(string pattern) :
        _regex(pattern)
    {
        _pattern = std::move(pattern);
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

}}}  // namespace puppet::runtime::values
