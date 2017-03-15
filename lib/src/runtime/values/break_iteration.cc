#include <puppet/runtime/values/break_iteration.hpp>
#include <puppet/compiler/exceptions.hpp>

using namespace std;
using namespace puppet::compiler::evaluation;
using namespace puppet::compiler::ast;
using namespace puppet::compiler;

namespace puppet { namespace runtime { namespace values {

    break_iteration::break_iteration(break_statement const& statement, vector<stack_frame> frames) :
        _context(statement),
        _frames(rvalue_cast(frames))
    {
        if (_context.tree) {
            _tree = _context.tree->shared_from_this();
        }
    }

    evaluation_exception break_iteration::create_exception() const
    {
        return evaluation_exception{
            "break statement is illegal in this context.",
            _context,
            _frames
        };
    }

    ostream& operator<<(ostream& os, break_iteration const& value)
    {
        // Value cannot be output
        throw value.create_exception();
    }

    bool operator==(break_iteration const& left, break_iteration const& right)
    {
        // Value cannot be compared
        throw left.create_exception();
    }

    bool operator!=(break_iteration const& left, break_iteration const& right)
    {
        // Value cannot be compared
        throw left.create_exception();
    }

    size_t hash_value(break_iteration const& value)
    {
        // Value cannot be hashed
        throw value.create_exception();
    }

}}}  // namespace puppet::runtime::values
