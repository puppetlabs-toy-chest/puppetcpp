#include <puppet/compiler/evaluation/operators/binary/minus.hpp>
#include <puppet/compiler/evaluation/operators/binary/call_context.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>
#include <limits>
#include <cfenv>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace operators { namespace binary {

    static values::value subtract(call_context& context, int64_t left, int64_t right)
    {
        auto& evaluation_context = context.context();

        if (right > 0) {
            if (left < numeric_limits<int64_t>::min() + right) {
                throw evaluation_exception(
                    (boost::format("subtraction of %1% and %2% results in an arithmetic underflow.") %
                     left %
                     right
                    ).str(),
                    context.right_context(),
                    evaluation_context.backtrace()
                );
            }
        } else {
            if (left > numeric_limits<int64_t>::max() + right) {
                throw evaluation_exception(
                    (boost::format("subtraction of %1% and %2% results in an arithmetic overflow.") %
                     left %
                     right
                    ).str(),
                    context.right_context(),
                    evaluation_context.backtrace()
                );
            }
        }
        return left - right;
    }

    static values::value subtract(call_context& context, double left, double right)
    {
        auto& evaluation_context = context.context();

        feclearexcept(FE_OVERFLOW | FE_UNDERFLOW);
        double result = left - right;
        if (fetestexcept(FE_OVERFLOW)) {
            throw evaluation_exception(
                (boost::format("subtraction of %1% and %2% results in an arithmetic overflow.") %
                 left %
                 right
                ).str(),
                context.right_context(),
                evaluation_context.backtrace()
            );
        } else if (fetestexcept(FE_UNDERFLOW)) {
            throw evaluation_exception(
                (boost::format("subtraction of %1% and %2% results in an arithmetic underflow.") %
                 left %
                 right
                ).str(),
                context.right_context(),
                evaluation_context.backtrace()
            );
        }
        return result;
    }

    static values::value subtract(call_context& context, values::value const& left, values::value const& right)
    {
        if (left.as<int64_t>() && right.as<int64_t>()) {
            return subtract(context, left.require<int64_t>(), right.require<int64_t>());
        }
        if (left.as<int64_t>()) {
            return subtract(context, static_cast<double>(left.require<int64_t>()), right.require<double>());
        }
        if (right.as<int64_t>()) {
            return subtract(context, left.require<double>(), static_cast<double>(right.require<int64_t>()));
        }
        return subtract(context, left.require<double>(), right.require<double>());
    }

    // Declared in divide.cc
    values::value arithmetic_string_conversion(call_context& context, bool left = true);

    descriptor minus::create_descriptor()
    {
        binary::descriptor descriptor{ ast::binary_operator::minus };

        // The order of these overloads is important (most specific to least specific)
        descriptor.add("Numeric", "Numeric", [](call_context& context) {
            return subtract(context, context.left(), context.right());
        });
        descriptor.add("String", "Numeric", [](call_context& context) {
            return subtract(context, arithmetic_string_conversion(context), context.right());
        });
        descriptor.add("Numeric", "String", [](call_context& context) {
            return subtract(context, context.left(), arithmetic_string_conversion(context, false));
        });
        descriptor.add("String", "String", [](call_context& context) {
            return subtract(context, arithmetic_string_conversion(context), arithmetic_string_conversion(context, false));
        });
        descriptor.add("Array[Any]", "Array[Any]", [](call_context& context) {
            auto result = context.left().require<values::array>();
            auto& right = context.right().require<values::array>();

            // Remove any elements in left that are also in right
            result.erase(remove_if(result.begin(), result.end(), [&](values::value const& v) {
                return find_if(right.begin(), right.end(), [&](values::value const& v2) { return v == v2; }) != right.end();
            }), result.end());
            return result;
        });
        descriptor.add("Array[Any]", "Hash[Any, Any]", [](call_context& context) {
            auto result = context.left().require<values::array>();
            auto& right = context.right().require<values::hash>();

            // Remove any [K,V] elements in left that are key and value in right
            result.erase(remove_if(result.begin(), result.end(), [&](values::value const& v) {
                auto ptr = v.as<values::array>();
                if (!ptr || ptr->size() != 2) {
                    return false;
                }
                auto value = right.get((*ptr)[0]);
                return value && (*ptr)[1] == *value;
            }), result.end());
            return result;
        });
        descriptor.add("Array[Any]", "Any", [](call_context& context) {
            auto result = context.left().require<values::array>();
            auto& right = context.right();

            // Remove any elements in left that are equal to right
            result.erase(remove_if(result.begin(), result.end(), [&](values::value const& v) { return v == right; }), result.end());
            return result;
        });
        descriptor.add("Hash[Any, Any]", "Hash[Any, Any]", [](call_context& context) {
            auto result = context.left().require<values::hash>();
            auto& right = context.right().require<values::hash>();

            // Remove any elements in left that have keys in right
            for (auto const& kvp : right) {
                result.erase(kvp.key());
            }
            return result;
        });
        descriptor.add("Hash[Any, Any]", "Array[Any]", [](call_context& context) {
            auto result = context.left().require<values::hash>();
            auto& right = context.right().require<values::array>();

            // Remove any elements in left with keys in right
            for (auto const& element : right) {
                result.erase(element);
            }
            return result;
        });
        return descriptor;
    }

}}}}}  // namespace puppet::compiler::evaluation::operators::binary
