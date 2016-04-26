#include <puppet/runtime/values/value.hpp>
#include <puppet/cast.hpp>
#include <boost/functional/hash.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace values {

    variable::variable(string name, shared_ptr<values::value const> value) :
        _name(rvalue_cast(name)),
        _value(rvalue_cast(value))
    {
    }

    string const& variable::name() const
    {
        return _name;
    }

    values::value const& variable::value() const
    {
        static values::value undefined;
        return _value ? *_value : undefined;
    }

    shared_ptr<values::value const> const& variable::shared_value() const
    {
        return _value;
    }

    void variable::assign(shared_ptr<values::value const> value)
    {
        _value = rvalue_cast(value);
    }

    bool operator==(variable const& left, variable const& right)
    {
        // Optimization: if both variables point to the same value, they are equal
        if (left.shared_value() == right.shared_value()) {
            return true;
        }
        return left.value() == right.value();
    }

    bool operator!=(variable const& left, variable const& right)
    {
        return !(left == right);
    }

    ostream& operator<<(ostream& os, values::variable const& variable)
    {
        os << variable.value();
        return os;
    }

    size_t hash_value(values::variable const& variable)
    {
        // Hash the underlying value
       return boost::hash_value(variable.value());
    }

}}}  // namespace puppet::runtime::values
