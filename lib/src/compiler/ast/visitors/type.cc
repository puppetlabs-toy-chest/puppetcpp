#include <puppet/compiler/ast/visitors/type.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>

using namespace std;

namespace puppet { namespace compiler { namespace ast { namespace visitors {

    void type::visit(postfix_expression const& expression)
    {
        operator()(expression);
    }

    void type::operator()(ast::undef const&)
    {
        // OK
    }

    void type::operator()(ast::defaulted const&)
    {
        // OK
    }

    void type::operator()(ast::boolean const& expression)
    {
        // OK
    }

    void type::operator()(ast::number const& expression)
    {
        // OK
    }

    void type::operator()(ast::string const& expression)
    {
        // OK
    }

    void type::operator()(ast::regex const& expression)
    {
        // OK
    }

    void type::operator()(ast::variable const& expression)
    {
        throw parse_exception("variables cannot be used in type specifications.", expression.begin, expression.end);
    }

    void type::operator()(ast::name const& expression)
    {
        // OK
    }

    void type::operator()(ast::bare_word const& expression)
    {
        // OK
    }

    void type::operator()(ast::type const& expression)
    {
        // OK
    }

    void type::operator()(ast::literal_string_text const& expression)
    {
        // OK
    }

    void type::operator()(ast::interpolated_string const& expression)
    {
        // Ensure the interpolation parts contain nothing illegal
        for (auto& part : expression.parts) {
            boost::apply_visitor(*this, part);
        }
    }

    void type::operator()(ast::array const& expression)
    {
        // OK if all elements are okay
        for (auto& element : expression.elements) {
            operator()(element);
        }
    }

    void type::operator()(ast::hash const& expression)
    {
        // OK if all elements are okay
        for (auto& element : expression.elements) {
            operator()(element.first);
            operator()(element.second);
        }
    }

    void type::operator()(ast::case_expression const& expression)
    {
        throw parse_exception("case expressions cannot be used in type specifications.", expression.begin, expression.end);
    }

    void type::operator()(ast::if_expression const& expression)
    {
        auto context = expression.context();
        throw parse_exception("if expressions cannot be used in type specifications.", context.begin, context.end);
    }

    void type::operator()(ast::unless_expression const& expression)
    {
        auto context = expression.context();
        throw parse_exception("unless expressions cannot be used in type specifications.", context.begin, context.end);
    }

    void type::operator()(ast::function_call_expression const& expression)
    {
        auto context = expression.context();
        throw parse_exception("function call expressions cannot be used in type specifications.", context.begin, context.end);
    }

    void type::operator()(ast::new_expression const& expression)
    {
        auto context = expression.context();
        throw parse_exception("new expressions cannot be used in type specifications.", context.begin, context.end);
    }

    void type::operator()(ast::epp_render_expression const& expression)
    {
        throw parse_exception("EPP expressions cannot be used in type specifications.", expression.begin, expression.end);
    }

    void type::operator()(ast::epp_render_block const& expression)
    {
        throw parse_exception("EPP expressions cannot be used in type specifications.", expression.begin, expression.end);
    }

    void type::operator()(ast::epp_render_string const& expression)
    {
        throw parse_exception("EPP expressions cannot be used in type specifications.", expression.begin, expression.end);
    }

    void type::operator()(ast::unary_expression const& expression)
    {
        operator()(expression.operand);
    }

    void type::operator()(ast::nested_expression const& expression)
    {
        operator()(expression.expression);
    }

    void type::operator()(ast::basic_expression const& expression)
    {
        boost::apply_visitor(*this, expression);
    }

    void type::operator()(ast::expression const& expression)
    {
        operator()(expression.operand);

        for (auto const& operation : expression.operations) {
            operator()(operation.operand);
        }
    }

    void type::operator()(ast::postfix_expression const& expression)
    {
        operator()(expression.operand);

        for (auto const& operation : expression.operations)
        {
            boost::apply_visitor(*this, operation);
        }
    }

    void type::operator()(ast::selector_expression const& expression)
    {
        throw parse_exception("selector expressions cannot be used in type specifications.", expression.begin, expression.end);
    }

    void type::operator()(ast::access_expression const& expression)
    {
        for (auto const& argument : expression.arguments) {
            operator()(argument);
        }
    }

    void type::operator()(ast::method_call_expression const& expression)
    {
        auto context = expression.context();
        throw parse_exception("method call expressions cannot be used in type specifications.", context.begin, context.end);
    }

}}}}  // namespace puppet::compiler::visitors
