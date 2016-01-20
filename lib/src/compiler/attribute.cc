#include <puppet/compiler/attribute.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler {

    attribute::attribute(string name, ast::context name_context, shared_ptr<values::value> value, ast::context value_context) :
        _name(rvalue_cast(name)),
        _name_context(rvalue_cast(name_context)),
        _value(rvalue_cast(value)),
        _value_context(rvalue_cast(value_context))
    {
        if (!_value) {
            throw runtime_error("expected an attribute value.");
        }
        if (_name_context.tree) {
            _tree = _name_context.tree->shared_from_this();
        } else if (_value_context.tree) {
            _tree = _value_context.tree->shared_from_this();
        }
    }

    string const& attribute::name() const
    {
        return _name;
    }

    ast::context const& attribute::name_context() const
    {
        return _name_context;
    }

    values::value& attribute::value()
    {
        return *_value;
    }

    values::value const& attribute::value() const
    {
        return *_value;
    }

    shared_ptr<values::value> attribute::shared_value()
    {
        return _value;
    }

    shared_ptr<values::value const> attribute::shared_value() const
    {
        return _value;
    }

    ast::context const& attribute::value_context() const
    {
        return _value_context;
    }

    bool attribute::unique() const
    {
        return _value.unique();
    }

}}  // namespace puppet::compiler
