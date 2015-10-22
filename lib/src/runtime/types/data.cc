#include <puppet/runtime/values/value.hpp>
#include <boost/functional/hash.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    bool data::is_instance(values::value const& value) const
    {
        // Data is scalar, array, hash, or undef
        return scalar().is_instance(value) ||
               array().is_instance(value)  ||
               hash().is_instance(value)   ||
               undef().is_instance(value);
    }

    bool data::is_specialization(values::type const& other) const
    {
        // Scalar, Array, Hash, and Undef are specializations of Data
        // Also specializations of Scalar, Array, and Hash
        return boost::get<scalar>(&other) ||
               boost::get<array>(&other) ||
               boost::get<hash>(&other) ||
               boost::get<undef>(&other) ||
               scalar().is_specialization(other) ||
               array().is_specialization(other) ||
               hash().is_specialization(other);
    }

    char const* data::name()
    {
        return "Data";
    }

    ostream& operator<<(ostream& os, data const&)
    {
        os << data::name();
        return os;
    }

    bool operator==(data const&, data const&)
    {
        return true;
    }

    bool operator!=(data const& left, data const& right)
    {
        return !(left == right);
    }

    size_t hash_value(data const& type)
    {
        static const size_t name_hash = boost::hash_value(data::name());

        size_t seed = 0;
        boost::hash_combine(seed, name_hash);
        return seed;
    }

}}}  // namespace puppet::runtime::types
