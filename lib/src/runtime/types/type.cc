#include <puppet/runtime/values/value.hpp>
#include <puppet/cast.hpp>
#include <boost/functional/hash.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    type::type(unique_ptr<values::type const> parameter) :
        _parameter(rvalue_cast(parameter))
    {
    }

    type::type(type const& other) :
        _parameter(other.parameter() ? new values::type(*other.parameter()) : nullptr)
    {
    }

    type& type::operator=(type const& other)
    {
        _parameter.reset(other.parameter() ? new values::type(*other.parameter()) : nullptr);
        return *this;
    }

    unique_ptr<values::type const> const& type::parameter() const
    {
        return _parameter;
    }

    char const* type::name()
    {
        return "Type";
    }

    bool type::is_instance(values::value const& value) const
    {
        // Check for type
        auto ptr = value.as<values::type>();
        if (!ptr) {
            return false;
        }
        // Unparameterized Type matches all types
        if (!_parameter) {
            return true;
        }
        // Compare the types
        return *ptr == *_parameter || _parameter->is_specialization(*ptr);
    }

    bool type::is_specialization(values::type const& other) const
    {
        // If this Type has a specialization, the other type cannot be a specialization
        if (_parameter) {
            return false;
        }
        // Check that the other Type is specialized
        auto type = boost::get<types::type>(&other);
        return type && type->parameter();
    }

    bool type::is_real(unordered_map<values::type const*, bool>& map) const
    {
        // Type is a real type
        return true;
    }

    void type::write(ostream& stream, bool expand) const
    {
        stream << types::type::name();
        if (!_parameter) {
            return;
        }
        stream << '[';
        _parameter->write(stream, false);
        stream << ']';
    }

    ostream& operator<<(ostream& os, types::type const& type)
    {
        type.write(os);
        return os;
    }

    bool operator==(type const& left, type const& right)
    {
        if (!left.parameter() && !right.parameter()) {
            return true;
        } else if (left.parameter() || right.parameter()) {
            return false;
        }
        return *left.parameter() == *right.parameter();
    }

    bool operator!=(type const& left, type const& right)
    {
        return !(left == right);
    }

    size_t hash_value(types::type const& type)
    {
        static const size_t name_hash = boost::hash_value(types::type::name());

        size_t seed = 0;
        boost::hash_combine(seed, name_hash);
        if (type.parameter()) {
            boost::hash_combine(seed, *type.parameter());
        }
        return seed;
    }

}}}  // namespace puppet::runtime::types
