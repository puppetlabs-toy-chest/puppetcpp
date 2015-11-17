#include <puppet/compiler/evaluation/operators/minus.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>
#include <limits>
#include <cfenv>

using namespace std;
using namespace puppet::runtime;
using namespace puppet::runtime::values;

namespace puppet { namespace compiler { namespace evaluation { namespace operators {

    struct minus_visitor : boost::static_visitor<value>
    {
        explicit minus_visitor(binary_operator_context const& context) :
            _context(context)
        {
        }

        result_type operator()(int64_t left, int64_t right) const
        {
            if (right > 0) {
                if (left < numeric_limits<int64_t>::min() + right) {
                    throw evaluation_exception((boost::format("subtraction of %1% and %2% results in an arithmetic underflow.") % left % right).str(), _context.right_context());
                }
            } else {
                if (left > numeric_limits<int64_t>::max() + right) {
                    throw evaluation_exception((boost::format("subtraction of %1% and %2% results in an arithmetic overflow.") % left % right).str(), _context.right_context());
                }
            }
            return left - right;
        }

        result_type operator()(int64_t left, double right) const
        {
            return operator()(static_cast<double>(left), right);
        }

        result_type operator()(double left, int64_t right) const
        {
            return operator()(left, static_cast<double>(right));
        }

        result_type operator()(double left, double right) const
        {
            feclearexcept(FE_OVERFLOW | FE_UNDERFLOW);
            double result = left - right;
            if (fetestexcept(FE_OVERFLOW)) {
                throw evaluation_exception((boost::format("subtraction of %1% and %2% results in an arithmetic overflow.") % left % right).str(), _context.right_context());
            } else if (fetestexcept(FE_UNDERFLOW)) {
                throw evaluation_exception((boost::format("subtraction of %1% and %2% results in an arithmetic underflow.") % left % right).str(), _context.right_context());
            }
            return result;
        }

        result_type operator()(values::array const& left, values::array const& right) const
        {
            auto result = left;

            // Remove any elements in left that are also in right
            result.erase(remove_if(result.begin(), result.end(), [&](value const& v) {
                return find_if(right.begin(), right.end(), [&](value const& v2) { return v == v2; }) != right.end();
            }), result.end());
            return result;
        }

        result_type operator()(values::array const& left, values::hash const& right) const
        {
            auto result = left;

            // Remove any [K,V] elements in left that are key and value in right
            result.erase(remove_if(result.begin(), result.end(), [&](value const& v) {
                auto ptr = v.as<values::array>();
                if (!ptr || ptr->size() != 2) {
                    return false;
                }
                auto value = right.get((*ptr)[0]);
                return value && (*ptr)[1] == *value;
            }), result.end());
            return result;
        }

        template <typename Right>
        result_type operator()(values::array const& left, Right const& right) const
        {
            auto result = left;

            // Remove any elements in left that are equal to right
            result.erase(remove_if(result.begin(), result.end(), [&](value const& v) { return v == right; }), result.end());
            return result;
        }

        result_type operator()(values::hash const& left, values::hash const& right) const
        {
            auto result = left;

            // Remove any elements in left that have keys in right
            for (auto const& kvp : right) {
                result.erase(kvp.key());
            }
            return result;
        }

        result_type operator()(values::hash const& left, values::array const& right) const
        {
            auto result = left;

            // Remove any elements in left with keys in right
            for (auto const& element : right) {
                result.erase(element);
            }
            return result;
        }

        template <typename Right>
        result_type operator()(values::hash const&, Right const& right) const
        {
            throw evaluation_exception((boost::format("expected %1% or %2% for deletion but found %3%.") % types::array::name() % types::hash::name() % value(right).get_type()).str(), _context.right_context());
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, int64_t>::value>::type,
            typename = typename enable_if<!is_same<Right, double>::value>::type
        >
        result_type operator()(int64_t const&, Right const& right) const
        {
            throw evaluation_exception((boost::format("expected %1% for arithmetic subtraction but found %2%.") % types::numeric::name() % value(right).get_type()).str(), _context.right_context());
        }

        template <
            typename Right,
            typename = typename enable_if<!is_same<Right, int64_t>::value>::type,
            typename = typename enable_if<!is_same<Right, double>::value>::type
        >
        result_type operator()(double const&, Right const& right) const
        {
            throw evaluation_exception((boost::format("expected %1% for arithmetic subtraction but found %2%.") % types::numeric::name() % value(right).get_type()).str(), _context.right_context());
        }

        template <typename Left, typename Right>
        result_type operator()(Left const& left, Right const&) const
        {
            throw evaluation_exception((boost::format("expected %1% for arithmetic subtraction but found %2%.") % types::numeric::name() % value(left).get_type()).str(), _context.left_context());
        }

     private:
        binary_operator_context const& _context;
    };

    value minus::operator()(binary_operator_context const& context) const
    {
        return boost::apply_visitor(minus_visitor(context), context.left(), context.right());
    }

}}}}  // namespace puppet::compiler::evaluation::operators
