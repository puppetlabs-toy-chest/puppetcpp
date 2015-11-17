#include <puppet/runtime/values/value.hpp>
#include <boost/functional/hash.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    floating::floating(double from, double to) :
        _from(from),
        _to(to)
    {
    }

    double floating::from() const
    {
        return _from;
    }

    double floating::to() const
    {
        return _to;
    }

    char const* floating::name()
    {
        return "Float";
    }

    bool floating::is_instance(values::value const& value) const
    {
        auto ptr = value.as<double>();
        if (!ptr) {
            return false;
        }
        return _to < _from ? (*ptr >= _to && *ptr <= _from) : (*ptr >= _from && *ptr <= _to);
    }

    bool floating::is_specialization(values::type const& other) const
    {
        // Check for an Float with a range inside of this type's range
        auto ptr = boost::get<floating>(&other);
        if (!ptr) {
            return false;
        }
        // Check for equality
        if (ptr->from() == _from && ptr->to() == _to) {
            return false;
        }
        return std::min(ptr->from(), ptr->to()) >= std::min(_from, _to) &&
               std::max(ptr->from(), ptr->to()) <= std::max(_from, _to);
    }

    ostream& operator<<(ostream& os, floating const& type)
    {
        os << floating::name();
        // BUG: fix direct floating point comparison
        bool from_default = type.from() == numeric_limits<double>::min();
        bool to_default = type.to() == numeric_limits<double>::max();
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

    bool operator==(floating const& left, floating const& right)
    {
        return left.from() == right.from() && left.to() == right.to();
    }

    bool operator!=(floating const& left, floating const& right)
    {
        return !(left == right);
    }

    size_t hash_value(floating const& type)
    {
        static const size_t name_hash = boost::hash_value(floating::name());

        size_t seed = 0;
        boost::hash_combine(seed, name_hash);
        boost::hash_combine(seed, type.from());
        boost::hash_combine(seed, type.to());
        return seed;
    }

}}}  // namespace puppet::runtime::types
