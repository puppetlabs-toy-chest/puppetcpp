#include <puppet/runtime/operators.hpp>
#include <puppet/runtime/expression_evaluator.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <limits>

using namespace std;
using namespace puppet::lexer;

namespace puppet { namespace runtime {

    struct in_visitor : boost::static_visitor<bool>
    {
        explicit in_visitor(context& ctx) :
            _context(ctx)
        {
        }

        result_type operator()(string const& left, string const& right) const
        {
            return boost::algorithm::icontains(right, left);
        }

        result_type operator()(regex const& left, string const& right) const
        {
            smatch matches;
            bool result = left.pattern().empty() || regex_match(right, matches, left.value());
            _context.current().set(matches);
            return result;
        }

        result_type operator()(type const& left, array const& right) const
        {
            // TODO: implement
            throw runtime_error("in operator not yet implemented for type");
        }

        result_type operator()(regex const& left, array const& right) const
        {
            for (auto const& element : right) {
                auto ptr = boost::get<string>(&element);
                if (ptr && operator()(left, *ptr)) {
                    return true;
                }
            }
            return false;
        }

        template <typename Left>
        result_type operator()(Left const& left, array const& right) const
        {
            for (auto const& element : right) {
                if (equals(left, element)) {
                    return true;
                }
            }
            return false;
        }

        result_type operator()(type const& left, hash const& right) const
        {
            // TODO: implement
            throw runtime_error("in operator not yet implemented for type");
        }

        template <typename Left>
        result_type operator()(Left const& left, hash const& right) const
        {
            for (auto const& element : right) {
                if (equals(left, element.first)) {
                    return true;
                }
            }
            return false;
        }

        template <typename Left, typename Right>
        result_type operator()(Left const&, Right const& right) const
        {
            return false;
        }

     private:
        context& _context;
    };

    bool in(value const& left, value const& right, context& ctx)
    {
        return boost::apply_visitor(in_visitor(ctx), dereference(left), dereference(right));
    }

    void assign(value& left, value& right, context& ctx, token_position const& position)
    {
        auto var = boost::get<variable>(&left);
        if (!var) {
            throw evaluation_exception(position, (boost::format("cannot assign to %1%: assignment can only be performed on variables.") % get_type(left)).str());
        }
        // Can't assign to match variables
        if (var->match()) {
            throw evaluation_exception(position, (boost::format("cannot assign to $%1%: variable name is reserved for match variables.") % var->name()).str());
        }
        if (var->name().find(':') != string::npos) {
            throw evaluation_exception(position, (boost::format("cannot assign to $%1%: assignment can only be performed on variables local to the current scope.") % var->name()).str());
        }

        // If the RHS is a match variable, we need to copy the value because it is temporary
        auto var_right = boost::get<variable>(&right);
        if (var_right && var_right->match()) {
            right = var_right->value();
        }

        // Set the value in the current scope
        auto value = ctx.current().set(var->name(), std::move(right));
        if (!value) {
            throw evaluation_exception(position, (boost::format("cannot assign to $%1%: variable already exists in the current scope.") % var->name()).str());
        }
        var->update(value);
    }

    template <typename T>
    bool overflow(T left, T right)
    {
        return left > 0 && right > numeric_limits<T>::max() - left;
    }

    template <typename T>
    bool underflow(T left, T right)
    {
        return left < 0 && right < numeric_limits<T>::min() - left;
    }

    struct plus_visitor : boost::static_visitor<value>
    {
        plus_visitor(token_position const& left_position, token_position const& right_position) :
            _left_position(left_position),
            _right_position(right_position)
        {
        }

        result_type operator()(int64_t left, int64_t right) const
        {
            check_operands(left, right);
            return left + right;
        }

        result_type operator()(int64_t left, long double right) const
        {
            return static_cast<long double>(left) + right;
        }

        result_type operator()(long double left, int64_t right) const
        {
            return left + static_cast<long double>(right);
        }

        result_type operator()(long double left, long double right) const
        {
            return left + right;
        }

        result_type operator()(array const& left, array const& right) const
        {
            array copy = left;
            for (auto const& element : right) {
                copy.emplace_back(element);
            }
            return copy;
        }

        result_type operator()(array const& left, hash const& right) const
        {
            array copy = left;
            for (auto const& element : right) {
                array subarray;
                subarray.push_back(element.first);
                subarray.push_back(element.second);
                copy.emplace_back(subarray);
            }
            return copy;
        }

        template <typename Right>
        result_type operator()(array const& left, Right const& right) const
        {
            array copy = left;
            copy.emplace_back(right);
            return copy;
        }

        result_type operator()(hash const& left, hash const& right) const
        {
            hash copy = left;
            for (auto const& element : right) {
                copy.emplace(make_pair(element.first, element.second));
            }
            return copy;
        }

        result_type operator()(hash const& left, array const& right) const
        {
            hash copy = left;

            // Check to see if the array is a "hash" (made up of two-element arrays only)
            bool hash = true;
            for (auto const& element : right) {
                auto subarray = boost::get<array>(&element);
                if (!subarray || subarray->size() != 2) {
                    hash = false;
                    break;
                }
            }

            if (hash) {
                for (auto const& element : right) {
                    auto subarray = boost::get<array>(element);
                    copy[subarray[0]] = subarray[1];
                }
                return copy;
            }

            // Otherwise, there should be an even number of elements
            // If not valid, the
            if (right.size() & 1) {
                throw evaluation_exception(_right_position, (boost::format("expected an even number of elements in Array for concatenation but found %1%.") % right.size()).str());
            }

            for (size_t i = 0; i < right.size(); i += 2) {
                copy[right[i]] = right[i + 1];
            }
            return copy;
        }

        template <typename Right>
        result_type operator()(hash const&, Right const& right) const
        {
            throw evaluation_exception(_right_position, (boost::format("expected Numeric for arithmetic addition but found %1%.") % get_type(right)).str());
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, int64_t>::value>::type,
            typename = typename enable_if<!is_same<Right, long double>::value>::type
        >
        result_type operator()(int64_t const&, Right const& right) const
        {
            throw evaluation_exception(_right_position, (boost::format("expected Numeric for arithmetic addition but found %1%.") % get_type(right)).str());
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, int64_t>::value>::type,
            typename = typename enable_if<!is_same<Right, long double>::value>::type
        >
        result_type operator()(long double const&, Right const& right) const
        {
            throw evaluation_exception(_right_position, (boost::format("expected Numeric for arithmetic addition but found %1%.") % get_type(right)).str());
        }

        template <typename Left, typename Right>
        result_type operator()(Left const& left, Right const&) const
        {
            throw evaluation_exception(_left_position, (boost::format("expected Numeric for arithmetic addition but found %1%.") % get_type(left)).str());
        }

     private:
        template <typename T>
        void check_operands(T left, T right) const
        {
            if (overflow(left, right)) {
                throw evaluation_exception(_left_position, (boost::format("addition of %1% and %2% results in an arithmetic overflow.") % left % right).str());
            }
            if (underflow(left, right)) {
                throw evaluation_exception(_left_position, (boost::format("addition of %1% and %2% results in an arithmetic underflow.") % left % right).str());
            }
        }

        token_position const& _left_position;
        token_position const& _right_position;
    };

    value plus(value const& left, value const& right, token_position const& left_position, token_position const& right_position)
    {
        return boost::apply_visitor(plus_visitor(left_position, right_position), dereference(left), dereference(right));
    }

    struct minus_visitor : boost::static_visitor<value>
    {
        minus_visitor(token_position const& left_position, token_position const& right_position) :
            _left_position(left_position),
            _right_position(right_position)
        {
        }

        result_type operator()(int64_t left, int64_t right) const
        {
            check_operands(left, right);
            return left - right;
        }

        result_type operator()(int64_t left, long double right) const
        {
            auto operand = static_cast<long double>(left);
            check_operands(operand, right);
            return operand - right;
        }

        result_type operator()(long double left, int64_t right) const
        {
            auto operand = static_cast<long double>(right);
            check_operands(left, operand);
            return left - operand;
        }

        result_type operator()(long double left, long double right) const
        {
            check_operands(left, right);
            return left - right;
        }

        result_type operator()(array const& left, array const& right) const
        {
            array copy = left;
            copy.erase(remove_if(copy.begin(), copy.end(), [&](value const& v) {
                for (auto const& element : right) {
                    if (equals(v, element)) {
                        return true;
                    }
                }
                return false;
            }), copy.end());
            return copy;
        }

        result_type operator()(array const& left, hash const& right) const
        {
            array copy = left;
            copy.erase(remove_if(copy.begin(), copy.end(), [&](value const& v) {
                // The element should be an array of [K, V]
                auto ptr = boost::get<array>(&v);
                if (!ptr || ptr->size() != 2) {
                    return false;
                }
                auto it = right.find((*ptr)[0]);
                if (it == right.end()) {
                    return false;
                }
                return equals((*ptr)[1], it->second);
            }), copy.end());
            return copy;
        }

        template <typename Right>
        result_type operator()(array const& left, Right& right) const
        {
            array copy = left;
            copy.erase(remove_if(copy.begin(), copy.end(), [&](value const& v) {
                return equals(v, right);
            }), copy.end());
            return copy;
        }

        result_type operator()(hash const& left, hash const& right) const
        {
            hash copy = left;
            for (auto it = copy.begin(); it != copy.end();) {
                if (right.count(it->first)) {
                    it = copy.erase(static_cast<hash::const_iterator>(it));
                } else {
                    ++it;
                }
            }
            return copy;
        }

        result_type operator()(hash const& left, array const& right) const
        {
            hash copy = left;
            for (auto const& element : right) {
                copy.erase(element);
            }
            return copy;
        }

        template <typename Right>
        result_type operator()(hash const&, Right const& right) const
        {
            throw evaluation_exception(_right_position, (boost::format("expected Hash or Array for deletion but found %1%.") % get_type(right)).str());
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, int64_t>::value>::type,
            typename = typename enable_if<!is_same<Right, long double>::value>::type
        >
        result_type operator()(int64_t const&, Right const& right) const
        {
            throw evaluation_exception(_right_position, (boost::format("expected Numeric for arithmetic subtraction but found %1%.") % get_type(right)).str());
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, int64_t>::value>::type,
            typename = typename enable_if<!is_same<Right, long double>::value>::type
        >
        result_type operator()(long double const&, Right const& right) const
        {
            throw evaluation_exception(_right_position, (boost::format("expected Numeric for arithmetic subtraction but found %1%.") % get_type(right)).str());
        }

        template <typename Left, typename Right>
        result_type operator()(Left const& left, Right const&) const
        {
            throw evaluation_exception(_left_position, (boost::format("expected Numeric for arithmetic subtraction but found %1%.") % get_type(left)).str());
        }

     private:
        template <typename T>
        void check_operands(T left, T right) const
        {
            if (overflow(left, right)) {
                throw evaluation_exception(_left_position, (boost::format("subtraction of %1% and %2% results in an arithmetic overflow.") % left % right).str());
            }
            if (underflow(left, right)) {
                throw evaluation_exception(_left_position, (boost::format("subtraction of %1% and %2% results in an arithmetic underflow.") % left % right).str());
            }
        }

        token_position const& _left_position;
        token_position const& _right_position;
    };

    value minus(value const& left, value const& right, token_position const& left_position, token_position const& right_position)
    {
        return boost::apply_visitor(minus_visitor(left_position, right_position), dereference(left), dereference(right));
    }

    struct multiply_visitor : boost::static_visitor<value>
    {
        multiply_visitor(token_position const& left_position, token_position const& right_position) :
            _left_position(left_position),
            _right_position(right_position)
        {
        }

        result_type operator()(int64_t left, int64_t right) const
        {
            return multiply(left, right);
        }

        result_type operator()(int64_t left, long double right) const
        {
            return static_cast<long double>(left) * right;
        }

        result_type operator()(long double left, int64_t right) const
        {
            return left * static_cast<long double>(right);
        }

        result_type operator()(long double left, long double right) const
        {
            return left * right;
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, int64_t>::value>::type,
            typename = typename enable_if<!is_same<Right, long double>::value>::type
        >
        result_type operator()(int64_t const&, Right const& right) const
        {
            throw evaluation_exception(_right_position, (boost::format("expected Numeric for arithmetic multiplication but found %1%.") % get_type(right)).str());
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, int64_t>::value>::type,
            typename = typename enable_if<!is_same<Right, long double>::value>::type
        >
        result_type operator()(long double const&, Right const& right) const
        {
            throw evaluation_exception(_right_position, (boost::format("expected Numeric for arithmetic multiplication but found %1%.") % get_type(right)).str());
        }

        template <typename Left, typename Right>
        result_type operator()(Left const& left, Right const&) const
        {
            throw evaluation_exception(_left_position, (boost::format("expected Numeric for arithmetic multiplication but found %1%.") % get_type(left)).str());
        }

    private:
        int64_t multiply(int64_t left, int64_t right) const
        {
            int64_t result = left * right;
            if (left != 0 && (result / left) != right) {
                throw evaluation_exception(_left_position, (boost::format("multiplication of %1% and %2% results in an arithmetic overflow.") % left % right).str());
            }
            return result;
        }

        token_position const& _left_position;
        token_position const& _right_position;
    };

    value multiply(value const& left, value const& right, token_position const& left_position, token_position const& right_position)
    {
        return boost::apply_visitor(multiply_visitor(left_position, right_position), dereference(left), dereference(right));
    }

    struct divide_visitor : boost::static_visitor<value>
    {
        divide_visitor(token_position const& left_position, token_position const& right_position) :
            _left_position(left_position),
            _right_position(right_position)
        {
        }

        result_type operator()(int64_t left, int64_t right) const
        {
            return divide(left, right);
        }

        result_type operator()(int64_t left, long double right) const
        {
            return static_cast<long double>(left) / right;
        }

        result_type operator()(long double left, int64_t right) const
        {
            return left / static_cast<long double>(right);
        }

        result_type operator()(long double left, long double right) const
        {
            return left / right;
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, int64_t>::value>::type,
            typename = typename enable_if<!is_same<Right, long double>::value>::type
        >
        result_type operator()(int64_t const&, Right const& right) const
        {
            throw evaluation_exception(_right_position, (boost::format("expected Numeric for arithmetic division but found %1%.") % get_type(right)).str());
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, int64_t>::value>::type,
            typename = typename enable_if<!is_same<Right, long double>::value>::type
        >
        result_type operator()(long double const&, Right const& right) const
        {
            throw evaluation_exception(_right_position, (boost::format("expected Numeric for arithmetic division but found %1%.") % get_type(right)).str());
        }

        template <typename Left, typename Right>
        result_type operator()(Left const& left, Right const&) const
        {
            throw evaluation_exception(_left_position, (boost::format("expected Numeric for arithmetic division but found %1%.") % get_type(left)).str());
        }

    private:
        int64_t divide(int64_t left, int64_t right) const
        {
            if (right == 0) {
                throw evaluation_exception(_right_position, "cannot divide by zero.");
            }
            int64_t result = left / right;
            if (left != 0 && (result * left) != right) {
                throw evaluation_exception(_left_position, (boost::format("division of %1% by %2% results in an arithmetic overflow.") % left % right).str());
            }
            return left / right;
        }

        token_position const& _left_position;
        token_position const& _right_position;
    };

    value divide(value const& left, value const& right, token_position const& left_position, token_position const& right_position)
    {
        return boost::apply_visitor(divide_visitor(left_position, right_position), dereference(left), dereference(right));
    }

    struct modulo_visitor : boost::static_visitor<value>
    {
        modulo_visitor(token_position const& left_position, token_position const& right_position) :
            _left_position(left_position),
            _right_position(right_position)
        {
        }

        result_type operator()(int64_t left, int64_t right) const
        {
            if (right == 0) {
                throw evaluation_exception(_right_position, (boost::format("cannot divide by zero.") % right).str());
            }
            return left % right;
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, int64_t>::value>::type
        >
        result_type operator()(int64_t const&, Right const& right) const
        {
            throw evaluation_exception(_right_position, (boost::format("expected Integer for arithmetic modulo but found %1%.") % get_type(right)).str());
        }

        template <typename Left, typename Right>
        result_type operator()(Left const& left, Right const&) const
        {
            throw evaluation_exception(_left_position, (boost::format("expected Integer for arithmetic modulo but found %1%.") % get_type(left)).str());
        }

    private:
        token_position const& _left_position;
        token_position const& _right_position;
    };

    value modulo(value const& left, value const& right, token_position const& left_position, token_position const& right_position)
    {
        return boost::apply_visitor(modulo_visitor(left_position, right_position), dereference(left), dereference(right));
    }

    struct negate_visitor : boost::static_visitor<value>
    {
        negate_visitor(token_position const& position) :
                _position(position)
        {
        }

        result_type operator()(int64_t operand) const
        {
            return -operand;
        }

        result_type operator()(long double operand) const
        {
            return -operand;
        }

        template <typename T>
        result_type operator()(T const& operand) const
        {
            throw evaluation_exception(_position, (boost::format("expected Numeric for unary negation operator but found %1%.") % get_type(operand)).str());
        }

    private:
        token_position const& _position;
    };

    value negate(value const& operand, token_position const& position)
    {
        return boost::apply_visitor(negate_visitor(position), operand);
    }

    struct left_shift_visitor : boost::static_visitor<value>
    {
        left_shift_visitor(token_position const& left_position, token_position const& right_position) :
            _left_position(left_position),
            _right_position(right_position)
        {
        }

        result_type operator()(int64_t left, int64_t right) const
        {
            // If right < 0. reverse direction
            // If left is negative, keep the sign bit
            if (right < 0 && left < 0) {
                return -(-left >> -right);
            }
            if (right < 0) {
                return left >> -right;
            }
            if (left < 0) {
                return -(-left << right);
            }
            return left << right;
        }

        template <typename Right>
        result_type operator()(array const& left, Right const& right) const
        {
            array copy = left;
            copy.emplace_back(right);
            return copy;
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, int64_t>::value>::type
        >
        result_type operator()(int64_t const&, Right const& right) const
        {
            throw evaluation_exception(_right_position, (boost::format("expected Integer for bitwise left shift but found %1%.") % get_type(right)).str());
        }

        template <typename Left, typename Right>
        result_type operator()(Left const& left, Right const&) const
        {
            throw evaluation_exception(_left_position, (boost::format("expected Integer for bitwise left shift but found %1%.") % get_type(left)).str());
        }

    private:
        token_position const& _left_position;
        token_position const& _right_position;
    };

    value left_shift(value const& left, value const& right, token_position const& left_position, token_position const& right_position)
    {
        return boost::apply_visitor(left_shift_visitor(left_position, right_position), dereference(left), dereference(right));
    }

    struct right_shift_visitor : boost::static_visitor<value>
    {
        right_shift_visitor(token_position const& left_position, token_position const& right_position) :
            _left_position(left_position),
            _right_position(right_position)
        {
        }

        result_type operator()(int64_t left, int64_t right) const
        {
            // If right < 0. reverse direction
            // If left is negative, keep the sign bit
            if (right < 0 && left < 0) {
                return -(-left << -right);
            }
            if (right < 0) {
                return left << -right;
            }
            if (left < 0) {
                return -(-left >> right);
            }
            return left >> right;
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, int64_t>::value>::type
        >
        result_type operator()(int64_t const&, Right const& right) const
        {
            throw evaluation_exception(_right_position, (boost::format("expected Integer for bitwise right shift but found %1%.") % get_type(right)).str());
        }

        template <typename Left, typename Right>
        result_type operator()(Left const& left, Right const&) const
        {
            throw evaluation_exception(_left_position, (boost::format("expected Integer for bitwise right shift but found %1%.") % get_type(left)).str());
        }

    private:
        token_position const& _left_position;
        token_position const& _right_position;
    };

    value right_shift(value const& left, value const& right, token_position const& left_position, token_position const& right_position)
    {
        return boost::apply_visitor(right_shift_visitor(left_position, right_position), dereference(left), dereference(right));
    }

    value logical_and(value const& left, value const& right)
    {
        return is_truthy(left) && is_truthy(right);
    }

    value logical_or(value const& left, value const& right)
    {
        return is_truthy(left) || is_truthy(right);
    }

    value logical_not(value const& operand)
    {
        return !is_truthy(operand);
    }

    struct less_visitor : boost::static_visitor<value>
    {
        less_visitor(token_position const& left_position, token_position const& right_position) :
            _left_position(left_position),
            _right_position(right_position)
        {
        }

        result_type operator()(int64_t left, int64_t right) const
        {
            return left < right;
        }

        result_type operator()(int64_t left, long double right) const
        {
            return static_cast<long double>(left) < right;
        }

        result_type operator()(long double left, int64_t right) const
        {
            return left < static_cast<long double>(right);
        }

        result_type operator()(long double left, long double right) const
        {
            return left < right;
        }

        result_type operator()(string const& left, string const& right) const
        {
            return boost::ilexicographical_compare(left, right);
        }

        result_type operator()(type const& left, type const& right) const
        {
            // TODO: implement
            throw evaluation_exception(_left_position, "operator < not yet implemented for type.");
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, int64_t>::value>::type,
            typename = typename enable_if<!is_same<Right, long double>::value>::type
        >
        result_type operator()(int64_t const&, Right const& right) const
        {
            throw evaluation_exception(_right_position, (boost::format("expected Numeric for comparison but found %1%.") % get_type(right)).str());
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, int64_t>::value>::type,
            typename = typename enable_if<!is_same<Right, long double>::value>::type
        >
        result_type operator()(long double const&, Right const& right) const
        {
            throw evaluation_exception(_right_position, (boost::format("expected Numeric for comparison but found %1%.") % get_type(right)).str());
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, string>::value>::type
        >
        result_type operator()(string const&, Right const& right) const
        {
            throw evaluation_exception(_right_position, (boost::format("expected String for comparison but found %1%.") % get_type(right)).str());
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, type>::value>::type
        >
        result_type operator()(type const&, Right const& right) const
        {
            throw evaluation_exception(_right_position, (boost::format("expected Type for comparison but found %1%.") % get_type(right)).str());
        }

        template <typename Left, typename Right>
        result_type operator()(Left const& left, Right const&) const
        {
            throw evaluation_exception(_left_position, (boost::format("expected Numeric, String, or Type for comparison but found %1%.") % get_type(left)).str());
        }

     private:
        token_position const& _left_position;
        token_position const& _right_position;
    };

    value less(value const& left, value const& right, token_position const& left_position, token_position const& right_position)
    {
        return boost::apply_visitor(less_visitor(left_position, right_position), dereference(left), dereference(right));
    }

    struct less_equal_visitor : boost::static_visitor<value>
    {
        less_equal_visitor(token_position const& left_position, token_position const& right_position) :
            _left_position(left_position),
            _right_position(right_position)
        {
        }

        result_type operator()(int64_t left, int64_t right) const
        {
            return left <= right;
        }

        result_type operator()(int64_t left, long double right) const
        {
            return static_cast<long double>(left) <= right;
        }

        result_type operator()(long double left, int64_t right) const
        {
            return left <= static_cast<long double>(right);
        }

        result_type operator()(long double left, long double right) const
        {
            return left <= right;
        }

        result_type operator()(string const& left, string const& right) const
        {
            // TODO: revisit performance
            return boost::ilexicographical_compare(left, right) || boost::iequals(left, right);
        }

        result_type operator()(type const& left, type const& right) const
        {
            // TODO: implement
            throw evaluation_exception(_left_position, "operator <= not yet implemented for type.");
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, int64_t>::value>::type,
            typename = typename enable_if<!is_same<Right, long double>::value>::type
        >
        result_type operator()(int64_t const&, Right const& right) const
        {
            throw evaluation_exception(_right_position, (boost::format("expected Numeric for comparison but found %1%.") % get_type(right)).str());
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, int64_t>::value>::type,
            typename = typename enable_if<!is_same<Right, long double>::value>::type
        >
        result_type operator()(long double const&, Right const& right) const
        {
            throw evaluation_exception(_right_position, (boost::format("expected Numeric for comparison but found %1%.") % get_type(right)).str());
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, string>::value>::type
        >
        result_type operator()(string const&, Right const& right) const
        {
            throw evaluation_exception(_right_position, (boost::format("expected String for comparison but found %1%.") % get_type(right)).str());
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, type>::value>::type
        >
        result_type operator()(type const&, Right const& right) const
        {
            throw evaluation_exception(_right_position, (boost::format("expected Type for comparison but found %1%.") % get_type(right)).str());
        }

        template <typename Left, typename Right>
        result_type operator()(Left const& left, Right const&) const
        {
            throw evaluation_exception(_left_position, (boost::format("expected Numeric, String, or Type for comparison but found %1%.") % get_type(left)).str());
        }

    private:
        token_position const& _left_position;
        token_position const& _right_position;
    };

    value less_equal(value const& left, value const& right, token_position const& left_position, token_position const& right_position)
    {
        return boost::apply_visitor(less_equal_visitor(left_position, right_position), dereference(left), dereference(right));
    }

    struct greater_visitor : boost::static_visitor<value>
    {
        greater_visitor(token_position const& left_position, token_position const& right_position) :
            _left_position(left_position),
            _right_position(right_position)
        {
        }

        result_type operator()(int64_t left, int64_t right) const
        {
            return left > right;
        }

        result_type operator()(int64_t left, long double right) const
        {
            return static_cast<long double>(left) > right;
        }

        result_type operator()(long double left, int64_t right) const
        {
            return left > static_cast<long double>(right);
        }

        result_type operator()(long double left, long double right) const
        {
            return left > right;
        }

        result_type operator()(string const& left, string const& right) const
        {
            // TODO: revisit performance
            return !boost::ilexicographical_compare(left, right) && !boost::iequals(left, right);
        }

        result_type operator()(type const& left, type const& right) const
        {
            // TODO: implement
            throw evaluation_exception(_left_position, "operator > not yet implemented for type.");
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, int64_t>::value>::type,
            typename = typename enable_if<!is_same<Right, long double>::value>::type
        >
        result_type operator()(int64_t const&, Right const& right) const
        {
            throw evaluation_exception(_right_position, (boost::format("expected Numeric for comparison but found %1%.") % get_type(right)).str());
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, int64_t>::value>::type,
            typename = typename enable_if<!is_same<Right, long double>::value>::type
        >
        result_type operator()(long double const&, Right const& right) const
        {
            throw evaluation_exception(_right_position, (boost::format("expected Numeric for comparison but found %1%.") % get_type(right)).str());
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, string>::value>::type
        >
        result_type operator()(string const&, Right const& right) const
        {
            throw evaluation_exception(_right_position, (boost::format("expected String for comparison but found %1%.") % get_type(right)).str());
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, type>::value>::type
        >
        result_type operator()(type const&, Right const& right) const
        {
            throw evaluation_exception(_right_position, (boost::format("expected Type for comparison but found %1%.") % get_type(right)).str());
        }

        template <typename Left, typename Right>
        result_type operator()(Left const& left, Right const&) const
        {
            throw evaluation_exception(_left_position, (boost::format("expected Numeric, String, or Type for comparison but found %1%.") % get_type(left)).str());
        }

    private:
        token_position const& _left_position;
        token_position const& _right_position;
    };

    value greater(value const& left, value const& right, token_position const& left_position, token_position const& right_position)
    {
        return boost::apply_visitor(greater_visitor(left_position, right_position), dereference(left), dereference(right));
    }

    struct greater_equal_visitor : boost::static_visitor<value>
    {
        greater_equal_visitor(token_position const& left_position, token_position const& right_position) :
            _left_position(left_position),
            _right_position(right_position)
        {
        }

        result_type operator()(int64_t left, int64_t right) const
        {
            return left >= right;
        }

        result_type operator()(int64_t left, long double right) const
        {
            return static_cast<long double>(left) >= right;
        }

        result_type operator()(long double left, int64_t right) const
        {
            return left >= static_cast<long double>(right);
        }

        result_type operator()(long double left, long double right) const
        {
            return left >= right;
        }

        result_type operator()(string const& left, string const& right) const
        {
            return !boost::ilexicographical_compare(left, right);
        }

        result_type operator()(type const& left, type const& right) const
        {
            // TODO: implement
            throw evaluation_exception(_left_position, "operator <= not yet implemented for type.");
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, int64_t>::value>::type,
            typename = typename enable_if<!is_same<Right, long double>::value>::type
        >
        result_type operator()(int64_t const&, Right const& right) const
        {
            throw evaluation_exception(_right_position, (boost::format("expected Numeric for comparison but found %1%.") % get_type(right)).str());
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, int64_t>::value>::type,
            typename = typename enable_if<!is_same<Right, long double>::value>::type
        >
        result_type operator()(long double const&, Right const& right) const
        {
            throw evaluation_exception(_right_position, (boost::format("expected Numeric for comparison but found %1%.") % get_type(right)).str());
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, string>::value>::type
        >
        result_type operator()(string const&, Right const& right) const
        {
            throw evaluation_exception(_right_position, (boost::format("expected String for comparison but found %1%.") % get_type(right)).str());
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, type>::value>::type
        >
        result_type operator()(type const&, Right const& right) const
        {
            throw evaluation_exception(_right_position, (boost::format("expected Type for comparison but found %1%.") % get_type(right)).str());
        }

        template <typename Left, typename Right>
        result_type operator()(Left const& left, Right const&) const
        {
            throw evaluation_exception(_left_position, (boost::format("expected Numeric, String, or Type for comparison but found %1%.") % get_type(left)).str());
        }

    private:
        token_position const& _left_position;
        token_position const& _right_position;
    };

    value greater_equal(value const& left, value const& right, token_position const& left_position, token_position const& right_position)
    {
        return boost::apply_visitor(greater_equal_visitor(left_position, right_position), dereference(left), dereference(right));
    }

    struct match_visitor : boost::static_visitor<value>
    {
        match_visitor(token_position const& left_position, token_position const& right_position, context& ctx) :
            _left_position(left_position),
            _right_position(right_position),
            _context(ctx)
        {
        }

        result_type operator()(string const& left, string const& right) const
        {
            smatch matches;
            bool result = right.empty() || regex_match(left, matches, std::regex(right));
            _context.current().set(matches);
            return result;
        }

        result_type operator()(string const& left, runtime::regex const& right) const
        {
            smatch matches;
            bool result = right.pattern().empty() || regex_match(left, matches, right.value());
            _context.current().set(matches);
            return result;
        }

        template <typename Left>
        result_type operator()(Left const& left, type const& right) const
        {
            // TODO: implement
            throw evaluation_exception(_right_position, "operator =~ not yet implemented for type.");
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, string>::value>::type,
            typename = typename enable_if<!is_same<Right, type>::value>::type
        >
        result_type operator()(string const&, Right const& right) const
        {
            throw evaluation_exception(_right_position, (boost::format("expected String or Regexp for match but found %1%.") % get_type(right)).str());
        }

        template <
            typename Left,
            typename Right,
            typename = typename enable_if<!is_same<Right, type>::value>::type
        >
        result_type operator()(Left const& left, Right const&) const
        {
            throw evaluation_exception(_left_position, (boost::format("expected String for match but found %1%.") % get_type(left)).str());
        }

    private:
        token_position const& _left_position;
        token_position const& _right_position;
        context& _context;
    };

    value match(value const& left, value const& right, token_position const& left_position, token_position const& right_position, context& ctx)
    {
        return boost::apply_visitor(match_visitor(left_position, right_position, ctx), dereference(left), dereference(right));
    }

    value splat(value operand)
    {
        // If an array, reuse it; otherwise, to_array it (copy if variable)
        array result;
        auto ptr = get<array>(&operand);
        if (ptr) {
            return std::move(*ptr);
        }
        return to_array(dereference(operand));
    }

}}  // namespace puppet::runtime
