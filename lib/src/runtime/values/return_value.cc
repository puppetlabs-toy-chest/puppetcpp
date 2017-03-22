#include <puppet/runtime/values/return_value.hpp>
#include <puppet/runtime/values/value.hpp>
#include <puppet/compiler/exceptions.hpp>

using namespace std;
using namespace puppet::compiler::evaluation;
using namespace puppet::compiler::ast;
using namespace puppet::compiler;

namespace puppet { namespace runtime { namespace values {

    return_value::return_value(return_statement const& statement, wrapper<value> value, vector<stack_frame> frames) :
        _context(statement.context()),
        _value(rvalue_cast(value)),
        _frames(rvalue_cast(frames))
    {
        if (_context.tree) {
            _tree = _context.tree->shared_from_this();
        }
    }

    evaluation_exception return_value::create_exception() const
    {
        return evaluation_exception{
            "return statement is illegal in this context.",
            _context,
            _frames
        };
    }

    value return_value::unwrap()
    {
        value v{ rvalue_cast(_value.get()) };
        _value = wrapper<value>{};
        return v;
    }

    ostream& operator<<(ostream& os, return_value const& value)
    {
        // Value cannot be output
        throw value.create_exception();
    }

    bool operator==(return_value const& left, return_value const& right)
    {
        // Value cannot be compared
        throw left.create_exception();
    }

    bool operator!=(return_value const& left, return_value const& right)
    {
        // Value cannot be compared
        throw left.create_exception();
    }

    size_t hash_value(return_value const& value)
    {
        // Value cannot be hashed
        throw value.create_exception();
    }

}}}  // namespace puppet::runtime::values
