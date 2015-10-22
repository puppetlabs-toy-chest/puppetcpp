#include <puppet/runtime/values/value.hpp>
#include <boost/functional/hash.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    collection::collection(int64_t from, int64_t to) :
        _from(from),
        _to(to)
    {
    }

    int64_t collection::from() const
    {
        return _from;
    }

    int64_t collection::to() const
    {
        return _to;
    }

    char const* collection::name()
    {
        return "Collection";
    }

    bool collection::is_instance(values::value const& value) const
    {
        // Check for array first
        int64_t size = 0;
        auto array = value.as<values::array>();
        if (array) {
            size = static_cast<int64_t>(array->size());
        } else {
            // Check for hash
            auto hash = value.as<values::hash>();
            if (hash) {
                size = static_cast<int64_t>(hash->size());
            } else {
                // Not a collection
                return false;
            }
        }
        // Check for size is range
        return _to < _from ? (size >= _to && size <= _from) : (size >= _from && size <= _to);
    }

    bool collection::is_specialization(values::type const& other) const
    {
        // Array and Hash are specializations
        // So are any specializations of Array and Hash
        return boost::get<array>(&other)        ||
               boost::get<hash>(&other)         ||
               array().is_specialization(other) ||
               hash().is_specialization(other);
    }

    ostream& operator<<(ostream& os, collection const& type)
    {
        os << collection::name();
        bool from_default = type.from() == numeric_limits<int64_t>::min();
        bool to_default = type.to() == numeric_limits<int64_t>::max();
        if (from_default && to_default) {
            // Only output the type name
            return os;
        }
        os << '[';
        if (from_default) {
            os << "default";
        } else {
            os << type.from();
        }
        os << ", ";
        if (to_default) {
            os << "default";
        } else {
            os << type.to();
        }
        os << ']';
        return os;
    }

    bool operator==(collection const& left, collection const& right)
    {
        return left.from() == right.from() && left.to() == right.to();
    }

    bool operator!=(collection const& left, collection const& right)
    {
        return !(left == right);
    }

    size_t hash_value(collection const& type)
    {
        static const size_t name_hash = boost::hash_value(collection::name());

        size_t seed = 0;
        boost::hash_combine(seed, name_hash);
        boost::hash_combine(seed, type.from());
        boost::hash_combine(seed, type.to());
        return seed;
    }

}}}  // namespace puppet::runtime::types
