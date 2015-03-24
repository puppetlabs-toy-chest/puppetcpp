#include <puppet/runtime/expression_evaluator.hpp>
#include <puppet/runtime/operators.hpp>
#include <puppet/ast/expression_def.hpp>
#include <boost/format.hpp>

using namespace std;

namespace puppet { namespace runtime {

    evaluation_exception::evaluation_exception(lexer::token_position position, string const& message) :
        runtime_error(message),
        _position(std::move(position))
    {
    }

    lexer::token_position const& evaluation_exception::position() const
    {
        return _position;
    }

    struct basic_expression_visitor : boost::static_visitor<value>
    {
        explicit basic_expression_visitor(expression_evaluator& evaluator) :
            _evaluator(evaluator)
        {
        }

        result_type operator()(ast::undef&) const
        {
            return value();
        }

        result_type operator()(ast::boolean& boolean) const
        {
            return boolean.value();
        }

        result_type operator()(int64_t integer) const
        {
            return integer;
        }

        result_type operator()(long double floating) const
        {
            return floating;
        }

        result_type operator()(ast::number& number) const
        {
            return boost::apply_visitor(*this, number.value());
        }

        result_type operator()(ast::string& str) const
        {
            return move(str.value());
        }

        result_type operator()(ast::regex& regx) const
        {
            try {
                return runtime::regex(move(regx.value()));
            } catch (std::regex_error const& ex) {
                throw evaluation_exception(regx.position(), ex.what());
            }
        }

        result_type operator()(ast::variable& var) const
        {
            auto val = _evaluator.context().lookup(var.name());
            return variable(move(var.name()), val);
        }

        result_type operator()(ast::name& name) const
        {
            // Treat as a string
            return move(name.value());
        }

        result_type operator()(ast::type& type) const
        {
            auto kind = get_type_kind(type.name());
            if (kind == type_kind::unknown) {
                throw evaluation_exception(type.position(), (boost::format("unknown type %1%.") % type.name()).str());
            }
            return runtime::type(kind);
        }

        result_type operator()(ast::array& arr) const
        {
            array new_array;

            if (arr.elements()) {
                for (auto& element : *arr.elements()) {
                    new_array.emplace_back(_evaluator.evaluate(element));
                }
            }
            return new_array;
        }

        result_type operator()(ast::hash& h) const
        {
            hash new_hash;

            if (h.elements()) {
                for (auto& element : *h.elements()) {
                    new_hash.emplace(_evaluator.evaluate(element.first), _evaluator.evaluate(element.second));
                }
            }
            return new_hash;
        }

     private:
        expression_evaluator& _evaluator;
    };

    struct control_flow_expression_visitor : boost::static_visitor<value>
    {
        explicit control_flow_expression_visitor(expression_evaluator& evaluator) :
            _evaluator(evaluator)
        {
        }

        result_type operator()(ast::selector_expression& expr) const
        {
            // TODO: implement
            throw evaluation_exception(expr.position(), "selector expression not yet implemented");
        }

        result_type operator()(ast::case_expression& expr) const
        {
            // TODO: implement
            throw evaluation_exception(expr.position(), "case expression not yet implemented");
        }

        result_type operator()(ast::if_expression& expr) const
        {
            if (is_truthy(_evaluator.evaluate(expr.conditional()))) {
                return execute(expr.body());
            }
            if (expr.elsifs()) {
                for (auto& elsif : *expr.elsifs()) {
                    if (is_truthy(_evaluator.evaluate(elsif.conditional()))) {
                        return execute(elsif.body());
                    }
                }
            }
            if (expr.else_()) {
                return execute(expr.else_()->body());
            }
            return value();
        }

        result_type operator()(ast::unless_expression& expr) const
        {
            if (!is_truthy(_evaluator.evaluate(expr.conditional()))) {
                return execute(expr.body());
            }
            if (expr.else_()) {
                return execute(expr.else_()->body());
            }
            return value();
        }

        result_type operator()(ast::method_call_expression& expr) const
        {
            // TODO: implement
            throw evaluation_exception(expr.position(), "method call expression not yet implemented");
        }

        result_type operator()(ast::function_call_expression& expr) const
        {
            // Evaluate the arguments first
            // TODO: support splat
            array arguments;
            arguments.reserve(expr.arguments() ? expr.arguments()->size() : 0);
            if (expr.arguments()) {
                for (auto& argument : *expr.arguments()) {
                    arguments.emplace_back(_evaluator.evaluate(argument));
                }
            }
            // TODO: this is just a HACK implementation of notice
            // TODO: implement real function dispatch
            if (expr.function().value() == "notice") {
                cout << "Notice: " << _evaluator.context().current() << ": ";
                bool first = true;
                for (auto const& argument : arguments) {
                    if (first) {
                        first = false;
                    } else {
                        cout << ' ';
                    }
                    cout << argument;
                }
                cout << endl;
                return value();
            } else {
                throw evaluation_exception(expr.position(), (boost::format("unknown function \"%1%\".") % expr.function().value()).str());
            }
        }

     private:
        result_type execute(boost::optional<vector<ast::expression>>& expressions) const
        {
            value result;
            if (expressions) {
                for (auto &expr : *expressions) {
                    result = _evaluator.evaluate(expr);
                }
            }
            return result;
        }

        expression_evaluator& _evaluator;
    };

    struct expression_visitor : boost::static_visitor<value>
    {
        explicit expression_visitor(expression_evaluator& evaluator) :
            _evaluator(evaluator)
        {
        }

        result_type operator()(boost::blank&) const
        {
            throw runtime_error("unexpected blank expression.");
        }

        result_type operator()(ast::basic_expression& expr) const
        {
            return boost::apply_visitor(basic_expression_visitor(_evaluator), expr);
        }

        result_type operator()(ast::control_flow_expression& expr) const
        {
            return boost::apply_visitor(control_flow_expression_visitor(_evaluator), expr);
        }

        result_type operator()(ast::catalog_expression& expr) const
        {
            // TODO: implement
            throw evaluation_exception(get_position(expr), "catalog expression not yet implemented");
        }

        result_type operator()(ast::unary_expression& expr) const
        {
            switch (expr.op()) {
                case ast::unary_operator::negate:
                    return negate(boost::apply_visitor(*this, expr.operand()), expr.position());

                case ast::unary_operator::logical_not:
                    return logical_not(boost::apply_visitor(*this, expr.operand()));

                case ast::unary_operator::splat:
                    // TODO: implement
                    throw evaluation_exception(expr.position(), "unary splat expression not yet implemented");

                default:
                    throw evaluation_exception(expr.position(), "unexpected unary expression.");
            }
        }

        result_type operator()(ast::access_expression& expr) const
        {
            // TODO: implement
            throw evaluation_exception(expr.position(), "access expression not yet implemented");
        }

        result_type operator()(ast::expression& expr) const
        {
            return _evaluator.evaluate(expr);
        }

     private:
        expression_evaluator& _evaluator;
    };

    expression_evaluator::expression_evaluator(runtime::context& ctx) :
        _context(ctx)
    {
    }

    value expression_evaluator::evaluate(ast::expression& expr)
    {
        if (expr.blank()) {
            return value();
        }

        // Evaluate the first sub-expression
        auto result = boost::apply_visitor(expression_visitor(*this), expr.first());
        auto position = expr.position();

        // Climb the remainder of the expression
        auto& remainder = expr.remainder();
        auto begin = remainder.begin();
        climb_expression(result, position, 0, begin, remainder.end());

        return result;
    }

    runtime::context const& expression_evaluator::context() const
    {
        return _context;
    }

    runtime::context& expression_evaluator::context()
    {
        return _context;
    }

    void expression_evaluator::climb_expression(
        value& left,
        lexer::token_position& left_position,
        uint8_t min_precedence,
        vector<ast::binary_expression>::iterator& begin,
        vector<ast::binary_expression>::iterator const& end)
    {
        // This member implements precedence climbing for binary expressions
        uint8_t precedence;
        while (begin != end && (precedence = get_precedence(begin->op())) >= min_precedence)
        {
            auto op = begin->op();
            auto& operand = begin->operand();
            auto right_position = begin->position();
            ++begin;

            // Evaluate the right side
            value right = boost::apply_visitor(expression_visitor(*this), operand);

            // Recurse and climb the expression
            uint8_t next_precdence = precedence + (is_right_associative(op) ? static_cast<uint8_t>(0) : static_cast<uint8_t>(1));
            climb_expression(right, right_position, next_precdence, begin, end);

            // Evaluate this part of the expression
            evaluate(left, left_position, op, right, right_position);
        }
    }

    void expression_evaluator::evaluate(
        value& left,
        lexer::token_position const& left_position,
        ast::binary_operator op,
        value& right,
        lexer::token_position& right_position)
    {
        switch (op) {
            case ast::binary_operator::in:
                left = in(left, right, _context);
                return;

            case ast::binary_operator::assignment:
                assign(left, right, _context, left_position);
                return;

            case ast::binary_operator::plus:
                left = plus(left, right, left_position, right_position);
                return;

            case ast::binary_operator::minus:
                left = minus(left, right, left_position, right_position);
                return;

            case ast::binary_operator::multiply:
                left = multiply(left, right, left_position, right_position);
                return;

            case ast::binary_operator::divide:
                left = divide(left, right, left_position, right_position);
                return;

            case ast::binary_operator::modulo:
                left = modulo(left, right, left_position, right_position);
                return;

            case ast::binary_operator::left_shift:
                left = left_shift(left, right, left_position, right_position);
                return;

            case ast::binary_operator::right_shift:
                left = right_shift(left, right, left_position, right_position);
                return;

            case ast::binary_operator::logical_and:
                left = logical_and(left, right);
                return;

            case ast::binary_operator::logical_or:
                left = logical_or(left, right);
                return;

            case ast::binary_operator::equals:
                left = equals(left, right);
                return;

            case ast::binary_operator::not_equals:
                left = !equals(left, right);
                return;

            case ast::binary_operator::less_than:
                left = less(left, right, left_position, right_position);
                return;

            case ast::binary_operator::less_equals:
                left = less_equal(left, right, left_position, right_position);
                return;

            case ast::binary_operator::greater_than:
                left = greater(left, right, left_position, right_position);
                return;

            case ast::binary_operator::greater_equals:
                left = greater_equal(left, right, left_position, right_position);
                return;

            case ast::binary_operator::match:
                left = match(left, right, left_position, right_position, _context);
                return;

            case ast::binary_operator::not_match:
                left = !is_truthy(match(left, right, left_position, right_position, _context));
                return;

            default:
                throw evaluation_exception(left_position, "unexpected binary expression.");
        }
    }

    uint8_t expression_evaluator::get_precedence(ast::binary_operator op)
    {
        // Return the precedence (low to high)
        switch (op) {
            case ast::binary_operator::in_edge:
            case ast::binary_operator::in_edge_subscribe:
            case ast::binary_operator::out_edge:
            case ast::binary_operator::out_edge_subscribe:
                return 1;

            case ast::binary_operator::assignment:
                return 2;

            case ast::binary_operator::logical_or:
                return 3;

            case ast::binary_operator::logical_and:
                return 4;

            case ast::binary_operator::greater_than:
            case ast::binary_operator::greater_equals:
            case ast::binary_operator::less_than:
            case ast::binary_operator::less_equals:
                return 5;

            case ast::binary_operator::equals:
            case ast::binary_operator::not_equals:
                return 6;

            case ast::binary_operator::left_shift:
            case ast::binary_operator::right_shift:
                return 7;

            case ast::binary_operator::plus:
            case ast::binary_operator::minus:
                return 8;

            case ast::binary_operator::multiply:
            case ast::binary_operator::divide:
            case ast::binary_operator::modulo:
                return 9;

            case ast::binary_operator::match:
            case ast::binary_operator::not_match:
                return 10;

            case ast::binary_operator::in:
                return 11;

            default:
                break;
        }

        throw runtime_error("invalid binary operator");
    }

    bool expression_evaluator::is_right_associative(ast::binary_operator op)
    {
        return op == ast::binary_operator::assignment;
    }

}}  // namespace puppet::runtime
