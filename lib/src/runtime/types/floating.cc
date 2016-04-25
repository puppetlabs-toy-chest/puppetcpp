#include <puppet/runtime/values/value.hpp>
#include <boost/functional/hash.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    floating const floating::instance;

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

    values::type floating::generalize() const
    {
        return types::floating{};
    }

    bool floating::is_instance(values::value const& value, recursion_guard& guard) const
    {
        auto ptr = value.as<double>();
        if (!ptr) {
            return false;
        }
        return _to < _from ? (*ptr >= _to && *ptr <= _from) : (*ptr >= _from && *ptr <= _to);
    }

    bool floating::is_assignable(values::type const& other, recursion_guard& guard) const
    {
        auto ptr = boost::get<floating>(&other);
        if (!ptr) {
            return false;
        }
        return std::min(ptr->_from, ptr->_to) >= std::min(_from, _to) &&
               std::max(ptr->_from, ptr->_to) <= std::max(_from, _to);
    }

    void floating::write(ostream& stream, bool expand) const
    {
        stream << floating::name();
        // BUG: fix direct floating point comparison
        bool from_default = _from == numeric_limits<double>::lowest();
        bool to_default = _to == numeric_limits<double>::max();
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

    ostream& operator<<(ostream& os, floating const& type)
    {
        type.write(os);
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
