#include <puppet/runtime/values/value.hpp>
#include <boost/functional/hash.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    char const* scalar::name()
    {
        return "Scalar";
    }

    bool scalar::is_instance(values::value const& value) const
    {
        // A Scalar is a Numeric, String, Boolean, and Regexp
        return
            numeric().is_instance(value) ||
            string().is_instance(value) ||
            boolean().is_instance(value) ||
            regexp().is_instance(value);
    }

    bool scalar::is_specialization(values::type const& other) const
    {
        // Numeric, String, Boolean and Regexp are specializations
        // Also specializations of Numeric and String
        return boost::get<numeric>(&other) ||
               boost::get<string>(&other) ||
               boost::get<boolean>(&other) ||
               boost::get<regexp>(&other) ||
               numeric().is_specialization(other) ||
               string().is_specialization(other);
    }

    bool scalar::is_real(unordered_map<values::type const*, bool>& map) const
    {
        // Scalar is a real type
        return true;
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
