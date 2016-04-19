#include <puppet/runtime/values/value.hpp>
#include <boost/functional/hash.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    data const data::instance{};

    bool data::is_instance(values::value const& value, recursion_guard& guard) const
    {
        return scalar::instance.is_instance(value, guard) || array::instance.is_instance(value, guard) ||
               hash::instance.is_instance(value, guard)   || undef::instance.is_instance(value, guard);
    }

    bool data::is_assignable(values::type const& other, recursion_guard& guard) const
    {
        if (boost::get<data>(&other)) {
            return true;
        }
        if (scalar::instance.is_assignable(other, guard) || array::instance.is_assignable(other, guard)  ||
            hash::instance.is_assignable(other, guard)   || undef::instance.is_assignable(other, guard)  ||
            tuple::instance.is_assignable(other, guard)) {
            return true;
        }
        return false;
    }

    void data::write(ostream& stream, bool expand) const
    {
        stream << data::name();
    }

    char const* data::name()
    {
        return "Data";
    }

    ostream& operator<<(ostream& os, data const& type)
    {
        type.write(os);
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
