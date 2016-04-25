#include <puppet/runtime/values/value.hpp>
#include <boost/functional/hash.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    char const* defaulted::name()
    {
        return "Default";
    }

    values::type defaulted::generalize() const
    {
        return *this;
    }

    bool defaulted::is_instance(values::value const& value, recursion_guard& guard) const
    {
        return value.is_default();
    }

    bool defaulted::is_assignable(values::type const& other, recursion_guard& guard) const
    {
        return boost::get<defaulted>(&other);
    }

    void defaulted::write(ostream& stream, bool expand) const
    {
        stream << defaulted::name();
    }

    ostream& operator<<(ostream& os, defaulted const& type)
    {
        type.write(os);
        return os;
    }

    bool operator==(defaulted const&, defaulted const&)
    {
        return true;
    }

    bool operator!=(defaulted const& left, defaulted const& right)
    {
        return !(left == right);
    }

    size_t hash_value(defaulted const& type)
    {
        static const size_t name_hash = boost::hash_value(defaulted::name());

        size_t seed = 0;
        boost::hash_combine(seed, name_hash);
        return seed;
    }

}}}  // namespace puppet::runtime::types
