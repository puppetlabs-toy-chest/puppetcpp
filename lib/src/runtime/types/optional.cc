#include <puppet/runtime/values/value.hpp>
#include <puppet/cast.hpp>
#include <boost/functional/hash.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    optional::optional(unique_ptr<values::type> type) :
        _type(rvalue_cast(type))
    {
    }

    optional::optional(optional const& other) :
        _type(other.type() ? new values::type(*other.type()) : nullptr)
    {
    }

    optional& optional::operator=(optional const& other)
    {
        _type.reset(other.type() ? new values::type(*other.type()) : nullptr);
        return *this;
    }

    unique_ptr<values::type> const& optional::type() const
    {
        return _type;
    }

    char const* optional::name()
    {
        return "Optional";
    }

    bool optional::is_instance(values::value const& value) const
    {
        // Undef always matches
        if (value.is_undef()) {
            return true;
        }

        // Unparameterized Optional matches only undef
        if (!_type) {
            return false;
        }

        // Check that the instance is of the given type
        return _type->is_instance(value);
    }

    bool optional::is_specialization(values::type const& other) const
    {
        // If this type has a specialization, the other type cannot be a specialization
        if (_type) {
            return false;
        }
        // Check that the other type is specialized
        auto optional = boost::get<types::optional>(&other);
        return optional && optional->type();
    }

    bool optional::is_real(unordered_map<values::type const*, bool>& map) const
    {
        // Optional is a real type
        return true;
    }

    void optional::write(ostream& stream, bool expand) const
    {
        stream << optional::name();
        if (!_type) {
            return;
        }
        stream << '[';
        _type->write(stream, false);
        stream << ']';
    }

    ostream& operator<<(ostream& os, optional const& type)
    {
        type.write(os);
        return os;
    }

    bool operator==(optional const& left, optional const& right)
    {
        if (!left.type() && !right.type()) {
            return true;
        } else if (!left.type()) {
            return false;
        } else if (!right.type()) {
            return false;
        }
        return *left.type() == *right.type();
    }

    bool operator!=(optional const& left, optional const& right)
    {
        return !(left == right);
    }

    size_t hash_value(optional const& type)
    {
        static const size_t name_hash = boost::hash_value(optional::name());

        size_t seed = 0;
        boost::hash_combine(seed, name_hash);
        if (type.type()) {
            boost::hash_combine(seed, *type.type());
        }
        return seed;
    }

}}}  // namespace puppet::runtime::types
