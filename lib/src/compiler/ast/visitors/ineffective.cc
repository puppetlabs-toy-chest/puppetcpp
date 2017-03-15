#include <puppet/compiler/ast/visitors/ineffective.hpp>

using namespace std;

namespace puppet { namespace compiler { namespace ast { namespace visitors {

    static void climb(
        bool& ineffective,
        unsigned int min_precedence,
        vector<binary_operation>::const_iterator& begin,
        vector<binary_operation>::const_iterator const& end)
    {
        // Climb the binary operations based on operator precedence
        unsigned int current = 0;
        while (begin != end && (current = precedence(begin->operator_)) >= min_precedence)
        {
            auto const& operation = *begin;
            ++begin;

            // Recurse and climb the expression
            unsigned int next = current + (is_right_associative(operation.operator_) ? 0 : 1);
            climb(ineffective, next, begin, end);

            // Ineffective if not assignment
            ineffective = operation.operator_ != binary_operator::assignment;
        }
    }

    bool ineffective::visit(statement const& statement) const
    {
        return operator()(statement);
    }

    bool ineffective::operator()(basic_expression const& expression) const
    {
        return boost::apply_visitor(*this, expression);
    }

    bool ineffective::operator()(undef const&) const
    {
        return true;
    }

    bool ineffective::operator()(defaulted const&) const
    {
        return true;
    }

    bool ineffective::operator()(boolean const& expression) const
    {
        return true;
    }

    bool ineffective::operator()(number const& expression) const
    {
        return true;
    }

    bool ineffective::operator()(ast::string const& expression) const
    {
        return true;
    }

    bool ineffective::operator()(regex const& expression) const
    {
        return true;
    }

    bool ineffective::operator()(variable const& expression) const
    {
        return true;
    }

    bool ineffective::operator()(name const& expression) const
    {
        return true;
    }

    bool ineffective::operator()(bare_word const& expression) const
    {
        return true;
    }

    bool ineffective::operator()(type const& expression) const
    {
        return true;
    }

    bool ineffective::operator()(interpolated_string const& expression) const
    {
        return true;
    }

    bool ineffective::operator()(literal_string_text const&) const
    {
        return true;
    }

    bool ineffective::operator()(ast::array const& expression) const
    {
        return true;
    }

    bool ineffective::operator()(hash const& expression) const
    {
        return true;
    }

    bool ineffective::operator()(case_expression const& expression) const
    {
        if (!operator()(expression.conditional)) {
            return false;
        }

        for (auto const& proposition : expression.propositions) {
            for (auto const& option : proposition.options) {
                if (!operator()(option)) {
                    return false;
                }
            }
            for (auto const& statement : proposition.body) {
                if (!operator()(statement)) {
                    return false;
                }
            }
        }
        return true;
    }

    bool ineffective::operator()(if_expression const& expression) const
    {
        if (!operator()(expression.conditional)) {
            return false;
        }

        for (auto const& statement : expression.body) {
            if (!operator()(statement)) {
                return false;
            }
        }

        for (auto const& elsif : expression.elsifs) {
            if (!operator()(elsif.conditional)) {
                return false;
            }
            for (auto const& statement : elsif.body) {
                if (!operator()(statement)) {
                    return false;
                }
            }
        }

        if (expression.else_) {
            for (auto const& statement : expression.else_->body) {
                if (!operator()(statement)) {
                    return false;
                }
            }
        }
        return true;
    }

    bool ineffective::operator()(unless_expression const& expression) const
    {
        if (!operator()(expression.conditional)) {
            return false;
        }

        for (auto const& statement : expression.body) {
            if (!operator()(statement)) {
                return false;
            }
        }

        if (expression.else_) {
            for (auto const& statement : expression.else_->body) {
                if (!operator()(statement)) {
                    return false;
                }
            }
        }
        return true;
    }

    bool ineffective::operator()(function_call_expression const& expression) const
    {
        return false;
    }

    bool ineffective::operator()(new_expression const& expression) const
    {
        return false;
    }

    bool ineffective::operator()(epp_render_expression const& expression) const
    {
        return false;
    }

    bool ineffective::operator()(epp_render_block const& expression) const
    {
        return false;
    }

    bool ineffective::operator()(epp_render_string const& expression) const
    {
        return false;
    }

    bool ineffective::operator()(unary_expression const& expression) const
    {
        return true;
    }

    bool ineffective::operator()(nested_expression const& expression) const
    {
        return operator()(expression.expression);
    }

    bool ineffective::operator()(postfix_expression const& expression) const
    {
        // If there are postfix operations, then the postfix expression is only effecitve if the last operation is effective
        if (!expression.operations.empty()) {
            return operator()(expression.operations.back());
        }
        return operator()(expression.operand);
    }

    bool ineffective::operator()(postfix_operation const& operation) const
    {
        return boost::apply_visitor(*this, operation);
    }

    bool ineffective::operator()(selector_expression const& expression) const
    {
        return true;
    }

    bool ineffective::operator()(access_expression const& expression) const
    {
        return true;
    }

    bool ineffective::operator()(method_call_expression const& expression) const
    {
        return false;
    }

    bool ineffective::operator()(ast::expression const& expression) const
    {
        if (expression.operations.empty()) {
            return operator()(expression.operand);
        }

        // Climb the binary operations
        bool ineffective = false;
        auto begin = expression.operations.begin();
        climb(ineffective, 0, begin, expression.operations.end());
        return ineffective;
    }

    bool ineffective::operator()(ast::statement const& statement) const
    {
        return boost::apply_visitor(*this, statement);
    }

    bool ineffective::operator()(class_statement const& statement) const
    {
        return false;
    }

    bool ineffective::operator()(defined_type_statement const& statement) const
    {
        return false;
    }

    bool ineffective::operator()(node_statement const& statement) const
    {
        return false;
    }

    bool ineffective::operator()(function_statement const& statement) const
    {
        return false;
    }

    bool ineffective::operator()(produces_statement const& statement) const
    {
        return false;
    }

    bool ineffective::operator()(consumes_statement const& statement) const
    {
        return false;
    }

    bool ineffective::operator()(application_statement const& statement) const
    {
        return false;
    }

    bool ineffective::operator()(site_statement const& statement) const
    {
        return false;
    }

    bool ineffective::operator()(type_alias_statement const& statement) const
    {
        return false;
    }

    bool ineffective::operator()(function_call_statement const& statement) const
    {
        return false;
    }

    bool ineffective::operator()(relationship_statement const& statement) const
    {
        // If an actual relationship statement, it has effect
        if (!statement.operations.empty()) {
            return false;
        }

        return operator()(statement.operand);
    }

    bool ineffective::operator()(relationship_expression const& expression) const
    {
        return boost::apply_visitor(*this, expression);
    }

    bool ineffective::operator()(resource_declaration_expression const& expression) const
    {
        return false;
    }

    bool ineffective::operator()(resource_override_expression const& expression) const
    {
        return false;
    }

    bool ineffective::operator()(resource_defaults_expression const& expression) const
    {
        return false;
    }

    bool ineffective::operator()(collector_expression const& expression) const
    {
        return false;
    }

    bool ineffective::operator()(break_statement const& statement) const
    {
        return false;
    }

}}}}  // namespace puppet::compiler::visitors
