#include <puppet/compiler/evaluation/operators/binary/plus.hpp>
#include <puppet/compiler/evaluation/operators/binary/call_context.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>
#include <limits>
#include <cfenv>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation { namespace operators { namespace binary {

    static values::value add(call_context& context, int64_t left, int64_t right)
    {
        auto& evaluation_context = context.context();

        if (right > 0) {
            if (left > numeric_limits<int64_t>::max() - right) {
                throw evaluation_exception(
                    (boost::format("addition of %1% and %2% results in an arithmetic overflow.") %
                     left %
                     right
                    ).str(),
                    context.right_context(),
                    evaluation_context.backtrace()
                );
            }
        } else {
            if (left < numeric_limits<int64_t>::min() - right) {
                throw evaluation_exception(
                    (boost::format("addition of %1% and %2% results in an arithmetic underflow.") %
                     left %
                     right
                    ).str(),
                    context.right_context(),
                    evaluation_context.backtrace()
                );
            }
        }
        return left + right;
    }

    static values::value add(call_context& context, double left, double right)
    {
        auto& evaluation_context = context.context();

        feclearexcept(FE_OVERFLOW | FE_UNDERFLOW);
        double result = left + right;
        if (fetestexcept(FE_OVERFLOW)) {
            throw evaluation_exception(
                (boost::format("addition of %1% and %2% results in an arithmetic overflow.") %
                 left %
                 right
                ).str(),
                context.right_context(),
                evaluation_context.backtrace()
            );
        } else if (fetestexcept(FE_UNDERFLOW)) {
            throw evaluation_exception(
                (boost::format("addition of %1% and %2% results in an arithmetic underflow.") %
                 left %
                 right
                ).str(),
                context.right_context(),
                evaluation_context.backtrace()
            );
        }
        return result;
    }

    static values::value add(call_context& context, values::value const& left, values::value const& right)
    {
        if (left.as<int64_t>() && right.as<int64_t>()) {
            return add(context, left.require<int64_t>(), right.require<int64_t>());
        }
        if (left.as<int64_t>()) {
            return add(context, static_cast<double>(left.require<int64_t>()), right.require<double>());
        }
        if (right.as<int64_t>()) {
            return add(context, left.require<double>(), static_cast<double>(right.require<int64_t>()));
        }
        return add(context, left.require<double>(), right.require<double>());
    }

    // Declared in divide.cc
    values::value arithmetic_string_conversion(call_context& context, bool left = true);

    descriptor plus::create_descriptor()
    {
        binary::descriptor descriptor{ ast::binary_operator::plus };

        // The order of these overloads is important (most specific to least specific)
        descriptor.add("Numeric", "Numeric", [](call_context& context) {
            return add(context, context.left(), context.right());
        });
        descriptor.add("String", "Numeric", [](call_context& context) {
            return add(context, arithmetic_string_conversion(context), context.right());
        });
        descriptor.add("Numeric", "String", [](call_context& context) {
            return add(context, context.left(), arithmetic_string_conversion(context, false));
        });
        descriptor.add("String", "String", [](call_context& context) {
            return add(context, arithmetic_string_conversion(context), arithmetic_string_conversion(context, false));
        });
        descriptor.add("Array[Any]", "Array[Any]", [](call_context& context) {
            auto& left = context.left().require<values::array>();
            auto& right = context.right().require<values::array>();

            values::array result;
            result.reserve(left.size() + right.size());
            result.insert(result.end(), left.begin(), left.end());
            result.insert(result.end(), right.begin(), right.end());
            return result;
        });
        descriptor.add("Array[Any]", "Hash[Any, Any]", [](call_context& context) {
            auto& left = context.left().require<values::array>();
            auto& right = context.right().require<values::hash>();

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
        });
        descriptor.add("Array[Any]", "Any", [](call_context& context) {
            auto& left = context.left().require<values::array>();
            auto& right = context.right();

            values::array result;
            result.reserve(left.size() + 1);
            result.insert(result.end(), left.begin(), left.end());
            result.emplace_back(right);
            return result;
        });
        descriptor.add("Hash[Any, Any]", "Hash[Any, Any]", [](call_context& context) {
            // Create a copy of the left and add key-value pairs
            auto result = context.left().require<values::hash>();
            auto& right = context.right().require<values::hash>();
            result.set(right.begin(), right.end());
            return result;
        });
        descriptor.add("Hash[Any, Any]", "Array[Any]", [](call_context& context) {
            auto& left = context.left().require<values::hash>();
            auto& right = context.right().require<values::array>();

            // Check to see if the array is a "hash" (made up of one or two element arrays only)
            bool hash = true;
            for (auto const& element : right) {
                auto subarray = element->as<values::array>();
                if (!subarray || subarray->empty() || subarray->size() > 2) {
                    hash = false;
                    break;
                }
            }

            auto result = left;
            if (hash) {
                for (auto& element : right) {
                    if (auto ptr = element->as<values::array>()) {
                        result.set((*ptr)[0], ptr->size() == 1 ? values::undef() : (*ptr)[1]);
                    }
                }
            } else {
                // Otherwise, there should be an even number of elements
                if (right.size() & 1) {
                    throw evaluation_exception(
                        (boost::format("expected an even number of elements in %1% for concatenation but found %2%.") %
                         types::array::name() %
                         right.size()
                        ).str(),
                        context.right_context(),
                        context.context().backtrace()
                    );
                }

                for (size_t i = 0; i < right.size(); i += 2) {
                    result.set(right[i], right[i + 1]);
                }
            }
            return result;
        });
        return descriptor;
    }

}}}}}  // namespace puppet::compiler::evaluation::operators::binary
