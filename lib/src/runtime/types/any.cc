#include <puppet/runtime/values/value.hpp>
#include <boost/functional/hash.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    char const* any::name()
    {
        return "Any";
    }

    values::type any::generalize() const
    {
        return *this;
    }

    bool any::is_instance(values::value const& value, recursion_guard& guard) const
    {
        // All values are an instance of Any
        return true;
    }

    bool any::is_assignable(values::type const& other, recursion_guard& guard) const
    {
        // All types are assignable to Any
        return true;
    }

    void any::write(ostream& stream, bool expand) const
    {
        stream << any::name();
    }

    ostream& operator<<(ostream& os, any const& type)
    {
        type.write(os);
        return os;
    }

    bool operator==(any const&, any const&)
    {
        return true;
    }

    bool operator!=(any const& left, any const& right)
    {
        return !(left == right);
    }

    size_t hash_value(any const&)
    {
        static const size_t name_hash = boost::hash_value(any::name());

        size_t seed = 0;
        boost::hash_combine(seed, name_hash);
        return seed;
    }

}}}  // namespace puppet::runtime::types
