#include <puppet/runtime/expression_evaluator.hpp>
#include <puppet/runtime/operators.hpp>
#include <puppet/runtime/string_interpolator.hpp>
#include <puppet/ast/expression_def.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::lexer;

namespace puppet { namespace runtime {

    struct match_variable_scope
    {
        explicit match_variable_scope(scope& s) :
            _scope(s)
        {
            _scope.push_matches();
        }

        ~match_variable_scope()
        {
            _scope.pop_matches();
        }

     private:
        scope& _scope;
    };

    evaluation_exception::evaluation_exception(token_position position, string const& message) :
        runtime_error(message),
        _position(std::move(position))
    {
    }

    // Forward declaration for evaluating primarye expressions
    static value evaluate_primary_expression(expression_evaluator& evaluator, ast::primary_expression& expr);

    token_position const& evaluation_exception::position() const
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
            string_interpolator interpolator(_evaluator);
            return interpolator.interpolate(str.position(), str.value(), str.escapes(), str.quote(), str.interpolated(), str.margin(), str.remove_break());
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
            static const std::regex match_variable_patterh("^\\d+$");

            string& name = var.name();

            bool match = false;
            value const* val = nullptr;
            if (regex_match(name, match_variable_patterh)) {
                // Check for invalid match name
                if (name.size() > 1 && name[0] == '0') {
                    throw evaluation_exception(var.position(), (boost::format("variable name $%1% is not a valid match variable name.") % var.name()).str());
                }
                // Look up the match
                val = _evaluator.context().current().get(stoi(name));
                match = true;
            } else {
                val = _evaluator.context().lookup(name);
            }
            return variable(move(name), val, match);
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
            // Selector expressions create a new match scope
            match_variable_scope match_scope(_evaluator.context().current());

            // Evaluate the selector's value
            value result = evaluate_primary_expression(_evaluator, expr.value());

            boost::optional<size_t> default_index;

            auto& cases = expr.cases();
            for (size_t i = 0; i < cases.size(); ++i) {
                auto& selector_case = cases[i];
                if (selector_case.is_default()) {
                    // Remember where the default case is and keep going
                    default_index = i;
                    continue;
                }

                // Evaluate the case selector
                value selector = _evaluator.evaluate(selector_case.selector());

                // If the selector is a regex, use match
                auto regex = boost::get<runtime::regex>(&dereference(selector));
                if (regex) {
                    // Only match against strings
                    if (boost::get<string>(&dereference(result))) {
                        if (is_truthy(match(result, selector, expr.position(), selector_case.position(), _evaluator.context()))) {
                            return _evaluator.evaluate(selector_case.result());
                        }
                    }
                    continue;
                }

                // Otherwise, use equals
                if (equals(result, selector)) {
                    return _evaluator.evaluate(selector_case.result());
                }
            }

            // Handle no matching case
            if (!default_index) {
                throw evaluation_exception(expr.position(), (boost::format("no matching selector case for value '%1%'.") % result).str());
            }

            // Evaluate the default case
            return _evaluator.evaluate(cases[*default_index].result());
        }

        result_type operator()(ast::case_expression& expr) const
        {
            // Case expressions create a new match scope
            match_variable_scope match_scope(_evaluator.context().current());

            // Evaluate the case's expression
            value result = _evaluator.evaluate(expr.expression());

            boost::optional<size_t> default_index;

            auto& propositions = expr.propositions();
            for (size_t i = 0; i < propositions.size(); ++i) {
                auto& proposition = propositions[i];
                if (proposition.is_default()) {
                    // Remember where the default is and keep going
                    default_index = i;
                    continue;
                }

                // Look for a match in the options
                for (auto& option : proposition.options()) {
                    // Evaluate the option
                    value option_value = _evaluator.evaluate(option);

                    // If the option is a regex, use match
                    auto regex = boost::get<runtime::regex>(&dereference(option_value));
                    if (regex) {
                        // Only match against strings
                        if (boost::get<string>(&dereference(result))) {
                            if (is_truthy(match(result, option_value, expr.position(), proposition.position(), _evaluator.context()))) {
                                return execute_block(proposition.body());
                            }
                        }
                        continue;
                    }

                    // Otherwise, use equals
                    if (equals(result, option_value)) {
                        return execute_block(proposition.body());
                    }
                }
            }

            // Handle no matching case
            if (default_index) {
                return execute_block(propositions[*default_index].body());
            }

            // Nothing matched, return undef
            return value();
        }

        result_type operator()(ast::if_expression& expr) const
        {
            // If expressions create a new match scope
            match_variable_scope match_scope(_evaluator.context().current());

            if (is_truthy(_evaluator.evaluate(expr.conditional()))) {
                return execute_block(expr.body());
            }
            if (expr.elsifs()) {
                for (auto& elsif : *expr.elsifs()) {
                    if (is_truthy(_evaluator.evaluate(elsif.conditional()))) {
                        return execute_block(elsif.body());
                    }
                }
            }
            if (expr.else_()) {
                return execute_block(expr.else_()->body());
            }
            return value();
        }

        result_type operator()(ast::unless_expression& expr) const
        {
            // Unless expressions create a new match scope
            match_variable_scope match_scope(_evaluator.context().current());

            if (!is_truthy(_evaluator.evaluate(expr.conditional()))) {
                return execute_block(expr.body());
            }
            if (expr.else_()) {
                return execute_block(expr.else_()->body());
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
                ostringstream ss;
                bool first = true;
                for (auto const& argument : arguments) {
                    if (first) {
                        first = false;
                    } else {
                        ss << ' ';
                    }
                    ss << argument;
                }
                string message = ss.str();
                cout << message << endl;
                return arguments.empty() ? value() : message;
            } else {
                throw evaluation_exception(expr.position(), (boost::format("unknown function \"%1%\".") % expr.function().value()).str());
            }
        }

     private:
        result_type execute_block(boost::optional<vector<ast::expression>>& expressions) const
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

    struct access_expression_evaluator : boost::static_visitor<value>
    {
        access_expression_evaluator(expression_evaluator& evaluator, vector<ast::expression>& expressions, token_position const& position) :
            _evaluator(evaluator),
            _expressions(expressions),
            _position(position)
        {
        }

        result_type operator()(string const& target) const
        {
            if (_expressions.size() > 2) {
                throw evaluation_exception(_expressions[2].position(), "expected at most two expressions when accessing a String.");
            }

            // Get the index
            value result = _evaluator.evaluate(_expressions[0]);
            auto ptr = boost::get<int64_t>(&result);
            if (!ptr) {
                throw evaluation_exception(_expressions[0].position(), (boost::format("expected Integer for start index but found %1%.") % get_type(result)).str());
            }

            // If the index is negative, it's from the end of the string
            int64_t index = *ptr;
            if (index < 0) {
                index += static_cast<int64_t>(target.size());
            }

            // Get the count
            int64_t count = 1;
            if (_expressions.size() == 2) {
                result = _evaluator.evaluate(_expressions[1]);
                ptr = boost::get<int64_t>(&result);
                if (!ptr) {
                    throw evaluation_exception(_expressions[1].position(), (boost::format("expected Integer for count but found %1%.") % get_type(result)).str());
                }
                count = *ptr;

                // A negative count denotes an end index (inclusive)
                if (count < 0) {
                    count += (target.size() + 1 - index);
                }
            }

            // If the index is still to the "left" of the start of the string, adjust the count and start at index 0
            if (index < 0) {
                count += index;
                index = 0;
            }
            if (count <= 0) {
                return string();
            }
            return target.substr(static_cast<size_t>(index), static_cast<size_t>(count));
        }

        result_type operator()(array const& target) const
        {
            if (_expressions.size() > 2) {
                throw evaluation_exception(_expressions[2].position(), "expected at most two expressions when accessing an Array.");
            }

            // Get the index
            value result = _evaluator.evaluate(_expressions[0]);
            auto ptr = boost::get<int64_t>(&result);
            if (!ptr) {
                throw evaluation_exception(_expressions[0].position(), (boost::format("expected Integer for start index but found %1%.") % get_type(result)).str());
            }

            // If the index is negative, it's from the end of the array
            int64_t index = *ptr;
            if (index < 0) {
                index += static_cast<int64_t>(target.size());
            }

            // Get the count
            int64_t count = 1;
            if (_expressions.size() == 2) {
                result = _evaluator.evaluate(_expressions[1]);
                ptr = boost::get<int64_t>(&result);
                if (!ptr) {
                    throw evaluation_exception(_expressions[1].position(), (boost::format("expected Integer for count but found %1%.") % get_type(result)).str());
                }
                count = *ptr;

                // A negative count denotes an end index (inclusive)
                if (count < 0) {
                    count += (target.size() + 1 - index);
                }
            } else {
                // Only the index given; return the element itself if in range
                if (index < 0 || static_cast<size_t>(index) >= target.size()) {
                    return value();
                }
                return target[index];
            }

            // If the index is still to the "left" of the start of the array, adjust the count and start at index 0
            if (index < 0) {
                count += index;
                index = 0;
            }
            if (count <= 0) {
                return array();
            }

            // Create the subarray
            array subarray;
            subarray.reserve(count);
            for (int64_t i = 0; i < count && static_cast<size_t>(i + index) < target.size(); ++i) {
                subarray.emplace_back(target[i + index]);
            }
            return subarray;
        }

        result_type operator()(hash const& target) const
        {
            if (_expressions.size() == 1) {
                // Lookup by key
                auto it = target.find(_evaluator.evaluate(_expressions[0]));
                if (it == target.end()) {
                    return value();
                }
                return it->second;
            }

            // Otherwise, build an array of values
            array result;
            for (auto& expr : _expressions) {
                // Lookup by key
                auto it = target.find(_evaluator.evaluate(expr));
                if (it == target.end()) {
                    continue;
                }
                result.emplace_back(it->second);
            }
            return result;
        }

        result_type operator()(type const& target) const
        {
            // TODO: implement
            throw evaluation_exception(_position, "access operator on Type not yet implemented.");
        }

        template <typename T>
        result_type operator()(T const& target) const
        {
            throw evaluation_exception(_position, (boost::format("access operator cannot be applied to a %1%.") % get_type(target)).str());
        }

     private:
        expression_evaluator& _evaluator;
        vector<ast::expression>& _expressions;
        token_position const& _position;
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
            value target = boost::apply_visitor(*this, expr.target());
            for (auto& access : expr.accesses()) {
                target = boost::apply_visitor(access_expression_evaluator(_evaluator, access.arguments(), ast::get_position(expr.target())), dereference(target));
            }
            return target;
        }

        result_type operator()(ast::expression& expr) const
        {
            return _evaluator.evaluate(expr);
        }

     private:
        expression_evaluator& _evaluator;
    };

    static value evaluate_primary_expression(expression_evaluator& evaluator, ast::primary_expression& expr)
    {
        return boost::apply_visitor(expression_visitor(evaluator), expr);
    }

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
        auto result = evaluate_primary_expression(*this, expr.first());
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
        token_position& left_position,
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
            value right = evaluate_primary_expression(*this, operand);

            // Recurse and climb the expression
            uint8_t next_precdence = precedence + (is_right_associative(op) ? static_cast<uint8_t>(0) : static_cast<uint8_t>(1));
            climb_expression(right, right_position, next_precdence, begin, end);

            // Evaluate this part of the expression
            evaluate(left, left_position, op, right, right_position);
        }
    }

    void expression_evaluator::evaluate(
        value& left,
        token_position const& left_position,
        ast::binary_operator op,
        value& right,
        token_position& right_position)
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
