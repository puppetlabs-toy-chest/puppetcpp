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

    bool collection::is_instance(values::value const& value, recursion_guard& guard) const
    {
        int64_t size = 0;
        if (auto array = value.as<values::array>()) {
            size = static_cast<int64_t>(array->size());
        } else if (auto hash = value.as<values::hash>()) {
            size = static_cast<int64_t>(hash->size());
        } else {
            return false;
        }
        return _to < _from ? (size >= _to && size <= _from) : (size >= _from && size <= _to);
    }

    bool collection::is_assignable(values::type const& other, recursion_guard& guard) const
    {
        int64_t from, to;
        if (auto array = boost::get<types::array>(&other)) {
            from = array->from();
            to = array->to();
        } else if (auto tuple = boost::get<types::tuple>(&other)) {
            from = tuple->from();
            to = tuple->to();
        } else if (auto hash = boost::get<types::hash>(&other)) {
            from = hash->from();
            to = hash->to();
        } else if (auto collection = boost::get<types::collection>(&other)) {
            from = collection->from();
            to = collection->to();
        } else {
            return false;
        }
        return std::min(from, to) >= std::min(_from, _to) &&
               std::max(from, to) <= std::max(_from, _to);
    }

    void collection::write(ostream& stream, bool expand) const
    {
        stream << collection::name();
        bool from_default = _from == 0;
        bool to_default = _to == numeric_limits<int64_t>::max();
        if (from_default && to_default) {
            // Only output the type name
            return;
        }
        stream << '[';
        if (from_default) {
            stream << "default";
        } else {
            stream << _from;
        }
        stream << ", ";
        if (to_default) {
            stream << "default";
        } else {
            stream << _to;
        }
        stream << ']';
    }

    ostream& operator<<(ostream& os, collection const& type)
    {
        type.write(os);
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
