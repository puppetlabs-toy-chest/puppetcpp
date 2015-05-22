#include <puppet/runtime/operators/plus.hpp>
#include <puppet/runtime/expression_evaluator.hpp>
#include <puppet/cast.hpp>
#include <boost/format.hpp>
#include <limits>
#include <cfenv>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace operators {

    struct plus_visitor : boost::static_visitor<value>
    {
        plus_visitor(lexer::position const& left_position, lexer::position const& right_position) :
            _left_position(left_position),
            _right_position(right_position)
        {
        }

        result_type operator()(int64_t left, int64_t right) const
        {
            if (right > 0) {
                if (left > numeric_limits<int64_t>::max() - right) {
                    throw evaluation_exception(_left_position, (boost::format("addition of %1% and %2% results in an arithmetic overflow.") % left % right).str());
                }
            } else {
                if (left < numeric_limits<int64_t>::min() - right) {
                    throw evaluation_exception(_left_position, (boost::format("addition of %1% and %2% results in an arithmetic underflow.") % left % right).str());
                }
            }
            return left + right;
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
            feclearexcept(FE_OVERFLOW | FE_UNDERFLOW);
            long double result = left + right;
            if (fetestexcept(FE_OVERFLOW)) {
                throw evaluation_exception(_left_position, (boost::format("addition of %1% and %2% results in an arithmetic overflow.") % left % right).str());
            } else if (fetestexcept(FE_UNDERFLOW)) {
                throw evaluation_exception(_left_position, (boost::format("addition of %1% and %2% results in an arithmetic underflow.") % left % right).str());
            }
            return result;
        }

        result_type operator()(values::array& left, values::array& right) const
        {
            // Move everything from the right into the left
            left.reserve(left.size() + right.size());
            for (auto& element : right) {
                left.emplace_back(rvalue_cast(element));
            }
            return rvalue_cast(left);
        }

        result_type operator()(values::array& left, values::hash& right) const
        {
            // Convert each entry in the hash into an array of [key, value]
            for (auto& element : right) {
                values::array subarray;
                subarray.reserve(2);
                subarray.push_back(element.first);
                subarray.push_back(rvalue_cast(element.second));
                left.emplace_back(rvalue_cast(subarray));
            }
            return rvalue_cast(left);
        }

        template <typename Right>
        result_type operator()(values::array& left, Right& right) const
        {
            left.emplace_back(rvalue_cast(right));
            return rvalue_cast(left);
        }

        result_type operator()(values::hash& left, values::hash& right) const
        {
            // Move everything from the right into the left
            left.reserve(left.size() + right.size());
            for (auto& element : right) {
                left.emplace(make_pair(element.first, rvalue_cast(element.second)));
            }
            return rvalue_cast(left);
        }

        result_type operator()(values::hash& left, values::array& right) const
        {
            // Check to see if the array is a "hash" (made up of two-element arrays only)
            bool hash = true;
            for (auto const& element : right) {
                auto subarray = as<values::array>(element);
                if (!subarray || subarray->size() != 2) {
                    hash = false;
                    break;
                }
            }

            if (hash) {
                for (auto& element : right) {
                    auto subarray = mutate_as<values::array>(element);
                    left[rvalue_cast(subarray[0])] = rvalue_cast(subarray[1]);
                }
            } else {
                // Otherwise, there should be an even number of elements
                if (right.size() & 1) {
                    throw evaluation_exception(_right_position, (boost::format("expected an even number of elements in %1% for concatenation but found %2%.") % types::array::name() % right.size()).str());
                }

                for (size_t i = 0; i < right.size(); i += 2) {
                    left[rvalue_cast(right[i])] = rvalue_cast(right[i + 1]);
                }
            }
            return rvalue_cast(left);
        }

        template <typename Right>
        result_type operator()(values::hash const&, Right const& right) const
        {
            throw evaluation_exception(_right_position, (boost::format("expected %1% or %2% for concatenation but found %3%.") % types::array::name() % types::hash::name() % get_type(right)).str());
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, int64_t>::value>::type,
            typename = typename enable_if<!is_same<Right, long double>::value>::type
        >
        result_type operator()(int64_t, Right const& right) const
        {
            throw evaluation_exception(_right_position, (boost::format("expected %1% for arithmetic addition but found %2%.") % types::numeric::name() % get_type(right)).str());
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, int64_t>::value>::type,
            typename = typename enable_if<!is_same<Right, long double>::value>::type
        >
        result_type operator()(long double, Right const& right) const
        {
            throw evaluation_exception(_right_position, (boost::format("expected %1% for arithmetic addition but found %2%.") % types::numeric::name() % get_type(right)).str());
        }

        template <typename Left, typename Right>
        result_type operator()(Left const& left, Right const&) const
        {
            throw evaluation_exception(_left_position, (boost::format("expected %1% for arithmetic addition but found %2%.") % types::numeric::name() % get_type(left)).str());
        }

     private:
        lexer::position const& _left_position;
        lexer::position const& _right_position;
    };

    value plus::operator()(binary_context& context) const
    {
        // Prepare the left and right for mutation
        auto left = mutate(context.left());
        auto right = mutate(context.right());

        return boost::apply_visitor(plus_visitor(context.left_position(), context.right_position()), left, right);
    }

}}}  // namespace puppet::runtime::operators
