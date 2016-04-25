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

    values::type optional::generalize() const
    {
        return types::optional{ _type ? make_unique<values::type>(_type->generalize()) : nullptr };
    }

    bool optional::is_instance(values::value const& value, recursion_guard& guard) const
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
        return _type->is_instance(value, guard);
    }

    bool optional::is_assignable(values::type const& other, recursion_guard& guard) const
    {
        if (boost::get<types::undef>(&other)) {
            return true;
        }
        if (auto optional = boost::get<types::optional>(&other)) {
            if (!_type) {
                return !optional->type();
            }
            if (!optional->type()) {
                return boost::get<types::undef>(_type.get());
            }
            return _type->is_assignable(*optional->type(), guard);
        }
        // Unparameterized Optional is only assignable from Undef, which was checked above
        if (!_type) {
            return false;
        }
        return _type->is_assignable(other, guard);
    }

    void optional::write(ostream& stream, bool expand) const
    {
        // If a type is not specified on an Optional, treat it like an Undef
        stream << (_type ? optional::name() : undef::name());
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
        if (!left.type()) {
            return !right.type() || boost::get<undef>(right.type().get());
        }
        if (!right.type()) {
            return !left.type() || boost::get<undef>(left.type().get());
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
