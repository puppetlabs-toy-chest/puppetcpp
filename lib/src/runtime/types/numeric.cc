#include <puppet/runtime/values/value.hpp>
#include <boost/functional/hash.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    char const* numeric::name()
    {
        return "Numeric";
    }

    bool numeric::is_instance(values::value const& value) const
    {
        return value.as<int64_t>() || value.as<double>();
    }

    bool numeric::is_specialization(values::type const& other) const
    {
        // Integer or Float is a specialization
        return boost::get<integer>(&other) || boost::get<floating>(&other);
    }

    bool numeric::is_real(unordered_map<values::type const*, bool>& map) const
    {
        // Numeric is a real type
        return true;
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
