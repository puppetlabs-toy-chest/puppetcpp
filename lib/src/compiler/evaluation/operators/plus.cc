#include <puppet/compiler/evaluation/operators/plus.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>
#include <limits>
#include <cfenv>

using namespace std;
using namespace puppet::runtime;
using namespace puppet::runtime::values;

namespace puppet { namespace compiler { namespace evaluation { namespace operators {

    struct plus_visitor : boost::static_visitor<value>
    {
        explicit plus_visitor(binary_operator_context const& context) :
            _context(context)
        {
        }

        result_type operator()(int64_t left, int64_t right) const
        {
            if (right > 0) {
                if (left > numeric_limits<int64_t>::max() - right) {
                    throw evaluation_exception((boost::format("addition of %1% and %2% results in an arithmetic overflow.") % left % right).str(), _context.right_context());
                }
            } else {
                if (left < numeric_limits<int64_t>::min() - right) {
                    throw evaluation_exception((boost::format("addition of %1% and %2% results in an arithmetic underflow.") % left % right).str(), _context.right_context());
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
                throw evaluation_exception((boost::format("addition of %1% and %2% results in an arithmetic overflow.") % left % right).str(), _context.right_context());
            } else if (fetestexcept(FE_UNDERFLOW)) {
                throw evaluation_exception((boost::format("addition of %1% and %2% results in an arithmetic underflow.") % left % right).str(), _context.right_context());
            }
            return result;
        }

        result_type operator()(values::array const& left, values::array const& right) const
        {
            values::array result;
            result.reserve(left.size() + right.size());
            result.insert(result.end(), left.begin(), left.end());
            result.insert(result.end(), right.begin(), right.end());
            return result;
        }

        result_type operator()(values::array const& left, values::hash const& right) const
        {
            values::array result;
            result.reserve(left.size() + right.size());

            // Copy everything from the left
            result.insert(result.end(), left.begin(), left.end());

            // Convert each entry in the hash into an array of [key, value]
            for (auto& element : right) {
                values::array subarray;
                subarray.reserve(2);
                subarray.emplace_back(element.key());
                subarray.emplace_back(element.value());
                result.emplace_back(rvalue_cast(subarray));
            }
            return result;
        }

        template <typename Right>
        result_type operator()(values::array const& left, Right const& right) const
        {
            values::array result;
            result.reserve(left.size() + 1);
            result.insert(result.end(), left.begin(), left.end());
            result.emplace_back(right);
            return result;
        }

        result_type operator()(values::hash const& left, values::hash const& right) const
        {
            // Create a copy of the left and add key-value pairs
            auto result = left;
            result.set(right.begin(), right.end());
            return result;
        }

        result_type operator()(values::hash const& left, values::array const& right) const
        {
            // Check to see if the array is a "hash" (made up of two-element arrays only)
            bool hash = true;
            for (auto const& element : right) {
                auto subarray = element->as<values::array>();
                if (!subarray || subarray->size() != 2) {
                    hash = false;
                    break;
                }
            }

            auto result = left;
            if (hash) {
                for (auto& element : right) {
                    if (auto ptr = element->as<values::array>()) {
                        result.set((*ptr)[0], (*ptr)[1]);
                    }
                }
            } else {
                // Otherwise, there should be an even number of elements
                if (right.size() & 1) {
                    throw evaluation_exception((boost::format("expected an even number of elements in %1% for concatenation but found %2%.") % types::array::name() % right.size()).str(), _context.right_context());
                }

                for (size_t i = 0; i < right.size(); i += 2) {
                    result.set(right[i], right[i + 1]);
                }
            }
            return result;
        }

        template <typename Right>
        result_type operator()(values::hash const&, Right const& right) const
        {
            throw evaluation_exception((boost::format("expected %1% or %2% for concatenation but found %3%.") % types::array::name() % types::hash::name() % value(right).get_type()).str(), _context.right_context());
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, int64_t>::value>::type,
            typename = typename enable_if<!is_same<Right, long double>::value>::type
        >
        result_type operator()(int64_t, Right const& right) const
        {
            throw evaluation_exception((boost::format("expected %1% for arithmetic addition but found %2%.") % types::numeric::name() % value(right).get_type()).str(), _context.right_context());
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, int64_t>::value>::type,
            typename = typename enable_if<!is_same<Right, long double>::value>::type
        >
        result_type operator()(long double, Right const& right) const
        {
            throw evaluation_exception((boost::format("expected %1% for arithmetic addition but found %2%.") % types::numeric::name() % value(right).get_type()).str(), _context.right_context());
        }

        template <typename Left, typename Right>
        result_type operator()(Left const& left, Right const&) const
        {
            throw evaluation_exception((boost::format("expected %1% for arithmetic addition but found %2%.") % types::numeric::name() % value(left).get_type()).str(), _context.left_context());
        }

     private:
        binary_operator_context const& _context;
    };

    value plus::operator()(binary_operator_context const& context) const
    {
        return boost::apply_visitor(plus_visitor(context), context.left(), context.right());
    }

}}}}  // namespace puppet::compiler::evaluation::operators
