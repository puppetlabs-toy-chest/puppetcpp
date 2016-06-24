#include <puppet/runtime/values/value.hpp>
#include <puppet/compiler/evaluation/context.hpp>
#include <puppet/cast.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace values {

    regex::regex(string pattern) :
        utility::regex(pattern.c_str()),
        _pattern(rvalue_cast(pattern))
    {
    }

    string const& regex::pattern() const
    {
        return _pattern;
    }

    bool regex::match(compiler::evaluation::context& context, std::string const& value) const
    {
        utility::regex::regions regions;
        bool result = _pattern.empty() || search(value, &regions);
        if (result) {
            context.set(regions.substrings(value));
        }
        return result;
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
        static const size_t name_hash = boost::hash_value("regex");

        std::size_t seed = 0;
        boost::hash_combine(seed, name_hash);
        boost::hash_combine(seed, regex.pattern());
        return seed;
    }

}}}  // namespace puppet::runtime::values
