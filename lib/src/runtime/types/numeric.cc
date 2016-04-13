#include <puppet/runtime/values/value.hpp>
#include <boost/functional/hash.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    numeric const numeric::instance{};

    char const* numeric::name()
    {
        return "Numeric";
    }

    bool numeric::is_instance(values::value const& value, recursion_guard& guard) const
    {
        return integer::instance.is_instance(value, guard) || floating::instance.is_instance(value, guard);
    }

    bool numeric::is_assignable(values::type const& other, recursion_guard& guard) const
    {
        if (boost::get<numeric>(&other)) {
            return true;
        }
        return integer::instance.is_assignable(other, guard) || floating::instance.is_assignable(other, guard);
    }

    void numeric::write(ostream& stream, bool expand) const
    {
        stream << numeric::name();
    }

    ostream& operator<<(ostream& os, numeric const& type)
    {
        type.write(os);
        return os;
    }

    bool operator==(numeric const&, numeric const&)
    {
        return true;
    }

    bool operator!=(numeric const& left, numeric const& right)
    {
        return !(left == right);
    }

    size_t hash_value(numeric const& type)
    {
        static const size_t name_hash = boost::hash_value(numeric::name());

        size_t seed = 0;
        boost::hash_combine(seed, name_hash);
        return seed;
    }

}}}  // namespace puppet::runtime::types
