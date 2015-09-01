/**
 * @file
 * Declares the runtime value.
 */
#pragma once

#include "array.hpp"
#include "defaulted.hpp"
#include "hash.hpp"
#include "regex.hpp"
#include "type.hpp"
#include "undef.hpp"
#include "variable.hpp"
#include "../../cast.hpp"
#include "../../lexer/position.hpp"
#include <boost/variant.hpp>
#include <boost/optional.hpp>
#include <string>
#include <cstddef>
#include <functional>

// Forward declare needed RapidJSON types.
namespace rapidjson {
    class CrtAllocator;
    template <typename BaseAllocator> class MemoryPoolAllocator;
    template <typename Encoding, typename Allocator> class GenericValue;
    template<typename CharType> struct UTF8;
    /**
     * Represents a JSON value.
     */
    typedef GenericValue<UTF8<char>, MemoryPoolAllocator<CrtAllocator>> Value;
    /**
     * Represents the allocator used by RapidJSON.
     */
    typedef MemoryPoolAllocator<CrtAllocator> Allocator;
}

namespace puppet { namespace runtime { namespace values {

    /**
     * Represents a runtime value.
     * Note: undef should always come first (default value).
     */
    typedef boost::make_recursive_variant<
        undef,
        defaulted,
        std::int64_t,
        long double,
        bool,
        std::string,
        regex,
        type,
        basic_variable<boost::recursive_variant_>,
        basic_array<boost::recursive_variant_>,
        basic_hash<boost::recursive_variant_>
    >::type value;

    /**
     * Stream insertion operator for runtime value.
     * @param os The output stream to write the runtime value to.
     * @param val The runtime value to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, value const& val);

    /**
     * Type definition for runtime variable.
     */
    typedef basic_variable<value> variable;

    /**
     * Type definition for runtime array.
     */
    typedef basic_array<value> array;

    /**
     * Type definition for runtime hash.
     */
    typedef basic_hash<value> hash;

    /**
     * Prepares the value for mutation.
     * For non-variables, this simply moves the value into the return value.
     * For variables, this creates a copy of the variable's value so it can be mutated.
     * @param v The value to mutate.
     * @return Returns the original value if not a variable or a copy of the value if a variable.
     */
    value mutate(value& v);

    /**
     * Dereferences a value.
     * @param val The value to dereference.
     * @return Returns the value of a variable or the original value if not a variable.
     */
    value const& dereference(value const& val);

    /**
     * This exists to prevent unintentional invocations of the "dereference" function.
     * See the documentation for as<T>(value const*) for an explanation.
     */
    value const& dereference(value const*);

    /**
     * Casts the value to a pointer of the given type.
     * Use this over boost::get for values to properly dereference variables.
     * @tparam T The type to cast the value to.
     * @param v The value to cast.
     * @return Returns a pointer to the given type or nullptr if the value is not of that type.
     */
    template <typename T>
    T const* as(value const& v)
    {
        return boost::get<T>(&dereference(v));
    }

    /**
     * This exists to prevent unintentional invocations of the "as<T>" function.
     * Because a value can be a bool and pointers are implicitly convertable to bool,
     * calling as<T>(&val) results in constructing a temporary value that is set to "true".
     * This function is declared without a definition to induce build errors if invoked.
     */
    template <typename T>
    T const* as(value const*);

    /**
     * Prepares the value for mutation as the given type.
     * Note: throws boost::bad_get if the value is not of the given type.
     * @tparam T The resulting type.
     * @param v The value to mutate.
     * @return Returns the value moved into the given type or a copy if the value is a variable.
     */
    template <typename T>
    T mutate_as(value& v)
    {
        // Check for variable first
        if (boost::get<variable>(&v)) {
            return boost::get<T>(dereference(v));
        }
        // Move the value
        return rvalue_cast(boost::get<T>(v));
    }

    /**
     * Determines if the given value is undefined.
     * @return Returns true for undef values or false if not.
     */
    bool is_undef(value const& val);

    /**
     * This exists to prevent unintentional invocations of the "is_undef" function.
     * See the documentation for as<T>(value const*) for an explanation.
     */
    bool is_undef(value const*);

    /**
     * Determines if the given value is default.
     * @return Returns true for default values or false if not.
     */
    bool is_default(value const& val);

    /**
     * This exists to prevent unintentional invocations of the "is_default" function.
     * See the documentation for as<T>(value const*) for an explanation.
     */
    bool is_default(value const*);

    /**
     * Determines if a value is the "true" value.
     * @param val The value to test.
     * @return Returns true if the value is exactly "true", or false if not.
     */
    bool is_true(value const& val);

    /**
     * This exists to prevent unintentional invocations of the "is_true" function.
     * See the documentation for as<T>(value const*) for an explanation.
     */
    bool is_true(value const*);

    /**
     * Determines if a value is the "false" value.
     * @param val The value to test.
     * @return Returns true if the value is exactly "false", or false if not.
     */
    bool is_false(value const& val);

    /**
     * This exists to prevent unintentional invocations of the "is_false" function.
     * See the documentation for as<T>(value const*) for an explanation.
     */
    bool is_false(value const*);

    /**
     * Determines if a value is "truthy".
     * @param val The value to test for "truthiness".
     * @return Returns true if the value is "truthy" or false if it is not.
     */
    bool is_truthy(value const& val);

    /**
     * This exists to prevent unintentional invocations of the "is_truthy" function.
     * See the documentation for as<T>(value const*) for an explanation.
     */
    bool is_truthy(value const*);

    /**
     * Gets the type of the given value.
     * @param val The runtime value to get the type of.
     * @return Returns the runtime type of the value.
     */
    values::type get_type(value const& val);

    /**
     * This exists to prevent unintentional invocations of the "get_type" function.
     * See the documentation for as<T>(value const*) for an explanation.
     */
    values::type get_type(value const*);

    /**
     * Determines if the given value is an instance of the given type.
     * @param val The value to check.
     * @param t The type to check.
     * @return Returns true if the value is an instance of the given type or false if not.
     */
    bool is_instance(value const& val, type const& t);

    /**
     * This exists to prevent unintentional invocations of the "is_instance" function.
     * See the documentation for as<T>(value const*) for an explanation.
     */
    bool is_instance(value const* val, type const& t);

    /**
     * Determines if the second type is a specialization of the first.
     * @param first The first type.
     * @param second The second type.
     * @return Returns true if the second type is a specialization of the first or false if not.
     */
    bool is_specialization(type const& first, type const& second);

    /**
     * Converts the given value to an array; the value is returned as an array if already an array.
     * @param val The value to convert to an array.
     * @param convert_hash True if hashes should be converted to an array of name-value arrays or false if not.
     * @return Returns the converted array.
     */
    array to_array(value& val, bool convert_hash = true);

    /**
     * Joins the array by converting each element to a string.
     * @param os The output stream to write to.
     * @param arr The array to join.
     * @param separator The separator to write between array elements.
     */
    void join(std::ostream& os, array const& arr, std::string const& separator = " ");

    /**
     * Declaration of operator== for values.
     * This exists to intentionally cause an ambiguity if == is used on a value.
     * Always use equals() on values and not ==.
     * @return No return defined.
     */
    bool operator==(value const&, value const&);

    /**
     * Equality visitor for values that handles variable comparison.
     */
    struct equality_visitor : boost::static_visitor<bool>
    {
        /**
         * Compares a variable against another value type.
         * @tparam The type of the other value.
         * @param left The variable to compare.
         * @param right The other value type to compare.
         * @return Returns true if the variable's value equals the value on the right-hand side.
         */
        template <
            typename T,
            typename = typename std::enable_if<!std::is_same<T, variable>::value>::type
        >
        result_type operator()(variable const& left, T const& right) const
        {
            // Dereference and "cast" to T
            auto ptr = boost::get<T>(&dereference(left));
            if (!ptr) {
                // Not the same type
                return false;
            }
            return operator()(*ptr, right);
        }

        /**
         * Compares a variable against another value type.
         * @tparam The type of the other value.
         * @param left The other value type to compare.
         * @param right The variable to compare.
         * @return Returns true if the variable's value equals the value on the left-hand side.
         */
        template <
            typename T,
            typename = typename std::enable_if<!std::is_same<T, variable>::value>::type
        >
        result_type operator()(T const& left, variable const& right) const
        {
            auto ptr = boost::get<T>(&dereference(right));
            if (!ptr) {
                // Not the same type
                return false;
            }
            return operator()(left, *ptr);
        }

        /**
         * Compares two strings.
         * @param left The left operand.
         * @param right The right operand.
         * @return Returns true if the strings are equal (case insensitive) or false if not.
         */
        result_type operator()(std::string const& left, std::string const& right) const;

        /**
         * Compares two different value types.
         * @tparam T The left hand type.
         * @tparam U The right hand type.
         * @return Always returns false since two values of different types cannot be equal.
         */
        template <
            typename T,
            typename U,
            typename = typename std::enable_if<!std::is_same<T, variable>::value>::type,
            typename = typename std::enable_if<!std::is_same<U, variable>::value>::type
        >
        result_type operator()(T const&, U const&) const
        {
            // Not the same type
            return false;
        }

        /**
         * Compares two different value types.
         * @tparam T The type of the values being compared.
         * @param left The left hand value.
         * @param right The right hand value.
         * @return Returns true if both values are equal or false if not.
         */
        template <typename T>
        result_type operator()(T const& left, T const& right) const
        {
            // Same type
            return left == right;
        }
    };

    /**
     * Compares two values for equality.
     * Use this instead of operator== (which boost::variant unfortunately defines).
     * @param left The left value to compare.
     * @param right The right value to compare.
     * @return Returns true if both values are equal or false if they are not equal.
     */
    bool equals(value const& left, value const& right);

    /**
     * This exists to prevent unintentional invocations of the "equals" function.
     * See the documentation for as<T>(value const*) for an explanation.
     */
    bool equals(value const* left, value const* right);

    /**
     * Compares a value with a value type.
     * @tparam Left The type of the right-hand side.
     * @param left The value type to compare.
     * @param right The value to compare.
     * @return Returns true if the type stored in the value is equal to the value type.
     */
    template <typename Left>
    bool equals(Left const& left, value const& right)
    {
        return boost::apply_visitor(std::bind(equality_visitor(), std::ref(left), std::placeholders::_1), right);
    }

    /**
     * Compares a value with a value type.
     * @tparam Right The type of the right-hand side.
     * @param left The value to compare.
     * @param right The value type to compare.
     * @return Returns true if the type stored in the value is equal to the value type.
     */
    template <typename Right>
    bool equals(value const& left, Right const& right)
    {
        return boost::apply_visitor(std::bind(equality_visitor(), std::placeholders::_1, std::ref(right)), left);
    }

    /**
     * Enumerates each "resource" type in the given value.
     * This expects the value to be an Variant[String, Resource, Array[Variant[String, Resource]].
     * @param value The value to enumerate the resources for.
     * @param callback The callback to invoke for each resource.
     * @param error The callback to invoke upon error.
     */
    void each_resource(value const& value, std::function<void(types::resource const&)> const& callback, std::function<void(std::string const&)> const& error);

    /**
     * Creates a RapidJSON value for the given value.
     * @param allocator The current RapidJSON allocator.
     * @return Returns the runtime value as a RapidJSON value.
     */
    rapidjson::Value to_json(values::value const& value, rapidjson::Allocator& allocator);

    /**
     * This exists to prevent unintentional invocations of the "to_json" function.
     * See the documentation for as<T>(value const*) for an explanation.
     */
    rapidjson::Value to_json(values::value const* value, rapidjson::Allocator& allocator);

}}}  // namespace puppet::runtime::values
