#include <puppet/runtime/values/value.hpp>
#include <boost/functional/hash.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    char const* boolean::name()
    {
        return "Boolean";
    }

    bool boolean::is_instance(values::value const& value) const
    {
        return value.as<bool>();
    }

    bool boolean::is_specialization(values::type const& other) const
    {
        // No specializations for Boolean
        return false;
    }

    bool boolean::is_real(unordered_map<values::type const*, bool>& map) const
    {
        // Boolean is a real type
        return true;
    }

    void boolean::write(ostream& stream, bool expand) const
    {
        stream << boolean::name();
    }

    ostream& operator<<(ostream& os, boolean const& type)
    {
        type.write(os);
        return os;
    }

    bool operator==(boolean const&, boolean const&)
    {
        return true;
    }

    bool operator!=(boolean const& left, boolean const& right)
    {
        return !(left == right);
    }

    size_t hash_value(boolean const&)
    {
        static const size_t name_hash = boost::hash_value(boolean::name());

        size_t seed = 0;
        boost::hash_combine(seed, name_hash);
        return seed;
    }

}}}  // namespace puppet::runtime::types
