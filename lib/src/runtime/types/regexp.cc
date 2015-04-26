#include <puppet/runtime/types/regexp.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    regexp::regexp(string pattern) :
        _pattern(std::move(pattern))
    {
    }

    string const& regexp::pattern() const
    {
        return _pattern;
    }

    const char* regexp::name()
    {
        return "Regexp";
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

}}}  // namespace puppet::runtime::types
