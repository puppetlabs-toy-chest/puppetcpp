#include <puppet/runtime/operators/minus.hpp>
#include <puppet/runtime/expression_evaluator.hpp>
#include <puppet/cast.hpp>
#include <boost/format.hpp>
#include <limits>
#include <cfenv>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace operators {

    struct minus_visitor : boost::static_visitor<value>
    {
        explicit minus_visitor(binary_context& context) :
            _context(context)
        {
        }

        result_type operator()(int64_t left, int64_t right) const
        {
            auto& evaluator = _context.evaluator();
            if (right > 0) {
                if (left < numeric_limits<int64_t>::min() + right) {
                    throw evaluator.create_exception(_context.left_position(), (boost::format("subtraction of %1% and %2% results in an arithmetic underflow.") % left % right).str());
                }
            } else {
                if (left > numeric_limits<int64_t>::max() + right) {
                    throw evaluator.create_exception(_context.left_position(), (boost::format("subtraction of %1% and %2% results in an arithmetic overflow.") % left % right).str());
                }
            }
            return left - right;
        }

        result_type operator()(int64_t left, long double right) const
        {
            return operator()(static_cast<long double>(left), right);
        }

        result_type operator()(long double left, int64_t right) const
        {
            return operator()(left, static_cast<long double>(right));
        }

        result_type operator()(long double left, long double right) const
        {
            auto& evaluator = _context.evaluator();
            feclearexcept(FE_OVERFLOW | FE_UNDERFLOW);
            long double result = left - right;
            if (fetestexcept(FE_OVERFLOW)) {
                throw evaluator.create_exception(_context.left_position(), (boost::format("subtraction of %1% and %2% results in an arithmetic overflow.") % left % right).str());
            } else if (fetestexcept(FE_UNDERFLOW)) {
                throw evaluator.create_exception(_context.left_position(), (boost::format("subtraction of %1% and %2% results in an arithmetic underflow.") % left % right).str());
            }
            return result;
        }

        result_type operator()(values::array& left, values::array const& right) const
        {
            // Remove any elements in left that are also in right
            left.erase(remove_if(left.begin(), left.end(), [&](value const& v) {
                return find_if(right.begin(), right.end(), [&](value const& v2) { return equals(v, v2); }) != right.end();
            }), left.end());
            return rvalue_cast(left);
        }

        result_type operator()(values::array& left, values::hash const& right) const
        {
            // Remove any [K,V] elements in left that are key and value in right
            left.erase(remove_if(left.begin(), left.end(), [&](value const& v) {
                auto ptr = as<values::array>(v);
                if (!ptr || ptr->size() != 2) {
                    return false;
                }
                auto it = right.find((*ptr)[0]);
                if (it == right.end()) {
                    return false;
                }
                return equals((*ptr)[1], it->second);
            }), left.end());
            return rvalue_cast(left);
        }

        template <typename Right>
        result_type operator()(values::array& left, Right const& right) const
        {
            // Remove any elements in left that are equal to right
            left.erase(remove_if(left.begin(), left.end(), [&](value const& v) { return equals(v, right); }), left.end());
            return rvalue_cast(left);
        }

        result_type operator()(values::hash& left, values::hash const& right) const
        {
            // Remove any elements in left that have keys in right
            for (auto const& element : right) {
                left.erase(element.first);
            }
            return rvalue_cast(left);
        }

        result_type operator()(values::hash& left, values::array const& right) const
        {
            // Remove any elements in left with keys in right
            for (auto const& element : right) {
                left.erase(element);
            }
            return rvalue_cast(left);
        }

        template <typename Right>
        result_type operator()(values::hash const&, Right const& right) const
        {
            throw _context.evaluator().create_exception(_context.right_position(), (boost::format("expected %1% or %2% for deletion but found %3%.") % types::array::name() % types::hash::name() % get_type(right)).str());
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, int64_t>::value>::type,
            typename = typename enable_if<!is_same<Right, long double>::value>::type
        >
        result_type operator()(int64_t const&, Right const& right) const
        {
            throw _context.evaluator().create_exception(_context.right_position(), (boost::format("expected %1% for arithmetic subtraction but found %2%.") % types::numeric::name() % get_type(right)).str());
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, int64_t>::value>::type,
            typename = typename enable_if<!is_same<Right, long double>::value>::type
        >
        result_type operator()(long double const&, Right const& right) const
        {
            throw _context.evaluator().create_exception(_context.right_position(), (boost::format("expected %1% for arithmetic subtraction but found %2%.") % types::numeric::name() % get_type(right)).str());
        }

        template <typename Left, typename Right>
        result_type operator()(Left const& left, Right const&) const
        {
            throw _context.evaluator().create_exception(_context.left_position(), (boost::format("expected %1% for arithmetic subtraction but found %2%.") % types::numeric::name() % get_type(left)).str());
        }

     private:
        binary_context& _context;
    };

    value minus::operator()(binary_context& context) const
    {
        // Prepare the left and right for mutation
        auto left = mutate(context.left());
        auto right = mutate(context.right());

        minus_visitor visitor(context);
        return boost::apply_visitor(visitor, left, right);
    }

}}}  // namespace puppet::runtime::operators
