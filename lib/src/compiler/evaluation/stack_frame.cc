#include <puppet/compiler/evaluation/stack_frame.hpp>

using namespace std;

namespace puppet { namespace compiler { namespace evaluation {

    stack_frame::stack_frame(char const* name, shared_ptr<evaluation::scope> scope, bool external) :
        _name(name),
        _scope(rvalue_cast(scope)),
        _external(external)
    {
        if (!_scope) {
            throw runtime_error("expected a scope for stack frame.");
        }
    }

    stack_frame::stack_frame(expression_type expression, shared_ptr<evaluation::scope> scope) :
        _name(nullptr),
        _expression(rvalue_cast(expression)),
        _scope(rvalue_cast(scope)),
        _current(expression_context(_expression)),
        _external(false)
    {
    }

    string stack_frame::name() const
    {
        ostringstream buffer;
        if (auto expression = as<ast::function_expression>()) {
            buffer << expression->name;
        } else if (auto expression = as<ast::class_expression>()) {
            buffer << "<class " << expression->name << ">";
        } else if (auto expression = as<ast::defined_type_expression>()) {
            buffer << "<define " << expression->name << ">";
        } else if (as<ast::node_expression>()) {
            buffer << "<node>";
        } else if (as<ast::collector_expression>()) {
            buffer << "<collector>";
        } else if (auto expression = as<ast::type_alias_expression>()) {
            buffer << "<type alias " << expression->alias << ">";
        } else if (_name) {
            buffer << _name;
        }
        return buffer.str();
    }

    bool stack_frame::external() const
    {
        return _external;
    }

    shared_ptr<evaluation::scope> const& stack_frame::scope() const
    {
        return _scope;
    }

    ast::context const& stack_frame::current() const
    {
        return _current;
    }

    void stack_frame::current(ast::context value)
    {
        // Update the current source for Puppet frames only
        if (!_external) {
            _current = rvalue_cast(value);
        }
    }

    ast::context stack_frame::expression_context(expression_type const& expression)
    {
        struct context_visitor : boost::static_visitor<ast::context>
        {
            ast::context operator()(ast::function_expression const* expression) const
            {
                return *expression;
            }

            ast::context operator()(ast::class_expression const* expression) const
            {
                return *expression;
            }

            ast::context operator()(ast::defined_type_expression const* expression) const
            {
                return *expression;
            }

            ast::context operator()(ast::node_expression const* expression) const
            {
                return *expression;
            }

            ast::context operator()(ast::collector_expression const* expression) const
            {
                return expression->context();
            }

            ast::context operator()(ast::type_alias_expression const* expression) const
            {
                return expression->context();
            }
        };
        return boost::apply_visitor(context_visitor{}, expression);
    }

    ostream& operator<<(ostream& os, stack_frame const& frame)
    {
        os << "in '" << frame.name() << '\'';

        auto& current = frame.current();
        if (current.tree) {
            os << " at " << current.tree->path() << ":" << current.begin.line();
        } else {
            os << " (no source)";
        }
        return os;
    }

}}}  // namespace puppet::compiler::evaluation
