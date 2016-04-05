#include <puppet/runtime/values/value.hpp>
#include <puppet/cast.hpp>
#include <boost/functional/hash.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    not_undef::not_undef(unique_ptr<values::type> type) :
        _type(rvalue_cast(type))
    {
    }

    not_undef::not_undef(not_undef const& other) :
        _type(other.type() ? new values::type(*other.type()) : nullptr)
    {
    }

    not_undef& not_undef::operator=(not_undef const& other)
    {
        _type.reset(other.type() ? new values::type(*other.type()) : nullptr);
        return *this;
    }

    unique_ptr<values::type> const& not_undef::type() const
    {
        return _type;
    }

    char const* not_undef::name()
    {
        return "NotUndef";
    }

    bool not_undef::is_instance(values::value const& value) const
    {
        // Undef never matches
        if (value.is_undef()) {
            return false;
        }

        // Unparameterized means "anything that isn't undef"
        if (!_type) {
            return true;
        }

        // Check that the instance is of the given type
        return _type->is_instance(value);
    }

    bool not_undef::is_specialization(values::type const& other) const
    {
        // If this type has a specialization, the other type cannot be a specialization
        if (_type) {
            return false;
        }
        // Check that the other type is specialized
        auto not_undef = boost::get<types::not_undef>(&other);
        return not_undef && not_undef->type();
    }

    bool not_undef::is_real(unordered_map<values::type const*, bool>& map) const
    {
        // NotUndef is a real type
        return true;
    }

    void not_undef::write(ostream& stream, bool expand) const
    {
        stream << not_undef::name();
        if (!_type) {
            return;
        }
        stream << '[';
        _type->write(stream, false);
        stream << ']';
    }

    ostream& operator<<(ostream& os, not_undef const& type)
    {
        type.write(os);
        return os;
    }

    bool operator==(not_undef const& left, not_undef const& right)
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

    bool operator!=(not_undef const& left, not_undef const& right)
    {
        return !(left == right);
    }

    size_t hash_value(not_undef const& type)
    {
        static const size_t name_hash = boost::hash_value(not_undef::name());

        size_t seed = 0;
        boost::hash_combine(seed, name_hash);
        if (type.type()) {
            boost::hash_combine(seed, *type.type());
        }
        return seed;
    }

}}}  // namespace puppet::runtime::types
