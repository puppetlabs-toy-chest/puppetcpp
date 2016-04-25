#include <puppet/runtime/values/value.hpp>
#include <boost/functional/hash.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    scalar const scalar::instance{};

    char const* scalar::name()
    {
        return "Scalar";
    }

    values::type scalar::generalize() const
    {
        return *this;
    }

    bool scalar::is_instance(values::value const& value, recursion_guard& guard) const
    {
        return numeric::instance.is_instance(value, guard) || string::instance.is_instance(value, guard) ||
               boolean::instance.is_instance(value, guard) || regexp::instance.is_instance(value, guard);
    }

    bool scalar::is_assignable(values::type const& other, recursion_guard& guard) const
    {
        if (boost::get<scalar>(&other)) {
            return true;
        }
        return numeric::instance.is_assignable(other, guard) || string::instance.is_assignable(other, guard) ||
               boolean::instance.is_assignable(other, guard) || regexp::instance.is_assignable(other, guard);
    }

    void scalar::write(ostream& stream, bool expand) const
    {
        stream << scalar::name();
    }

    ostream& operator<<(ostream& os, scalar const& type)
    {
        type.write(os);
        return os;
    }

    bool operator==(scalar const&, scalar const&)
    {
        return true;
    }

    bool operator!=(scalar const& left, scalar const& right)
    {
        return !(left == right);
    }

    size_t hash_value(scalar const& type)
    {
        static const size_t name_hash = boost::hash_value(puppet::runtime::types::scalar::name());

        size_t seed = 0;
        boost::hash_combine(seed, name_hash);
        return seed;
    }

}}}  // namespace puppet::runtime::types
