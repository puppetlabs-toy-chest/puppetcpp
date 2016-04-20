/**
 * @file
 * Declares the runtime value.
 */
#pragma once

#include "forward.hpp"
#include "array.hpp"
#include "defaulted.hpp"
#include "hash.hpp"
#include "iterator.hpp"
#include "regex.hpp"
#include "type.hpp"
#include "undef.hpp"
#include "type.hpp"
#include "undef.hpp"
#include "variable.hpp"
#include "../../cast.hpp"
#include <boost/variant.hpp>
#include <boost/mpl/contains.hpp>
#include <string>
#include <cstddef>
#include <functional>

// Forward declare needed RapidJSON types.
namespace rapidjson {
    class CrtAllocator;
    template <typename Encoding, typename Allocator> class GenericValue;
    template <typename Encoding, typename Allocator, typename StackAllocator> class GenericDocument;
    template<typename CharType> struct UTF8;
}

namespace puppet { namespace runtime { namespace values {

    /**
     * The RapidJSON allocator.
     */
    using json_allocator = rapidjson::CrtAllocator;
    /**
     * The RapidJSON value.
     */
    using json_value = rapidjson::GenericValue<rapidjson::UTF8<char>, json_allocator>;
    /**
     * The RapidJSON document.
     */
    using json_document = rapidjson::GenericDocument<rapidjson::UTF8<char>, json_allocator, json_allocator>;

    /**
     * Represents all possible value types.
     */
    using value_base = boost::variant<
        undef,
        defaulted,
        std::int64_t,
        double,
        bool,
        std::string,
        regex,
        type,
        variable,
        array,
        hash,
        iterator
    >;

    /**
     * Represents a runtime value.
     */
    struct value : value_base
    {
        /**
         * Default constructor for value.
         */
        value() = default;

        /**
         * Constructs a value based on a variant type.
         * @tparam T The variant type to construct with.
         * @param value The value to initialize the variant with.
         */
        template <
            typename T,
            typename = typename std::enable_if<
                !std::is_pointer<T>::value &&
                !std::is_same<typename std::remove_reference<T>::type, value>::value &&
                !std::is_same<typename std::remove_reference<T>::type, wrapper<value>>::value
            >::type
        >
        value(T const& value) :
            value_base(value)
        {
        }

        /**
         * Constructs a value based on a variant type.
         * @tparam T The variant type to construct with.
         * @param value The value to initialize the variant with.
         */
        template <
            typename T,
            typename = typename std::enable_if<
                !std::is_pointer<T>::value &&
                !std::is_const<T>::value &&
                std::is_rvalue_reference<T&&>::value &&
                !std::is_same<typename std::remove_reference<T>::type, value>::value &&
                !std::is_same<typename std::remove_reference<T>::type, wrapper<value>>::value
            >::type
        >
        value(T&& value) noexcept :
            value_base(rvalue_cast(value))
        {
        }

        /**
         * Constructs a value by moving the value contained in a wrapper.
         * @param wrapper The wrapper containing the value to move.
         */
        value(values::wrapper<value>&& wrapper);

        /**
         * Constructs a value given a C string.
         * @param string The string to construct the value with.
         */
        value(char const* string);

        /**
         * Intentionally deleted constructor to prevent implicit conversion to a boolean value.
         */
        value(void const*) = delete;

        /**
         * Move assigns the value given a wrapper containing the value to move.
         * @param wrapper The wrapper containing the value to move.
         * @return Returns this value.
         */
        value& operator=(values::wrapper<value>&& wrapper);

        /**
         * Copy assignment operator given a C-string to set as a string value.
         * @param string The string to assign to the value.
         * @return Returns this value.
         */
        value& operator=(char const* string);

        /**
         * Copy assignment operator for value.
         * @tparam T The variant type to assign with.
         * @param value The value to assign.
         * @return Returns this value.
         */
        template <
            typename T,
            typename = typename std::enable_if<
                !std::is_pointer<T>::value &&
                !std::is_same<typename std::remove_reference<T>::type, value>::value &&
                !std::is_same<typename std::remove_reference<T>::type, wrapper<value>>::value
            >::type
        >
        value& operator=(T const& value)
        {
            value_base::operator=(value);
            return *this;
        }

        /**
         * Move assignment operator for value.
         * @tparam T The variant type to assign with.
         * @param value The value to assign.
         * @return Returns this value.
         */
        template <
            typename T,
            typename = typename std::enable_if<
                !std::is_pointer<T>::value &&
                !std::is_const<T>::value &&
                std::is_rvalue_reference<T&&>::value &&
                !std::is_same<typename std::remove_reference<T>::type, value>::value &&
                !std::is_same<typename std::remove_reference<T>::type, wrapper<value>>::value
            >::type
        >
        value& operator=(T&& value)
        {
            value_base::operator=(rvalue_cast(value));
            return *this;
        }

        /**
         * Gets a pointer to the requested value type.
         * Use this over boost::get for values to properly dereference variables.
         * @tparam T The requested value type.
         * @return Returns a pointer to the given type or nullptr if this value is not of that type.
         */
        template <typename T>
        T const* as() const
        {
            if (auto var = boost::get<variable>(this)) {
                return var->value().as<T>();
            }
            return boost::get<T>(this);
        }

        /**
         * Requires that the value hold the given type and returns a reference to it.
         * @tparam T The requested value type.
         * @return Returns a reference to the given type or throws an exception if the value is not of that type.
         */
        template <typename T>
        T const& require() const
        {
            auto ptr = as<T>();
            if (!ptr) {
                throw std::runtime_error("invalid cast requested.");
            }
            return *ptr;
        }

        /**
         * Moves the value as the given value type.
         * Note: throws boost::bad_get if the value is not of the given type.
         * @tparam T The requested value type.
         * @return Returns the value moved into the given type or a copy if the value is a variable.
         */
        template <typename T>
        T move_as()
        {
            if (auto var = boost::get<variable>(this)) {
                value copy = var->value();
                return copy.move_as<T>();
            }
            // Move this value
            return rvalue_cast(boost::get<T>(*this));
        }

        /**
         * Moves a value or the elements of the value if the value is an array (recursively).
         * @tparam T The expected type of the value or the elements of the value if the value is an array.
         * @param callback The callback to call with each moved value.
         * @return Returns true if all values moved successfully or false if not.
         */
        template <typename T>
        bool move_as(std::function<void(T)> const& callback)
        {
            if (as<T>()) {
                callback(move_as<T>());
                return true;
            }
            if (as<values::array>()) {
                // For arrays, recurse on each element
                auto array = move_as<values::array>();
                for (auto& element : array) {
                    if (!element->move_as<T>(callback)) {
                        return false;
                    }
                }
                return true;
            }
            return false;
        }

        /**
         * Determines if the given value is undefined.
         * @return Returns true for undef values or false if not.
         */
        bool is_undef() const;

        /**
         * Determines if the given value is default.
         * @return Returns true for default values or false if not.
         */
        bool is_default() const;

        /**
         * Determines if a value is the "true" value.
         * @return Returns true if the value is exactly "true", or false if not.
         */
        bool is_true() const;

        /**
         * Determines if a value is the "false" value.
         * @return Returns true if the value is exactly "false", or false if not.
         */
        bool is_false() const;

        /**
         * Determines if a value is "truthy".
         * @return Returns true if the value is "truthy" or false if it is not.
         */
        bool is_truthy() const;

        /**
         * Infers the type of the value.
         * @param detailed True to do a detailed inference or false to do a reduced inference.
         * @return Returns the runtime type of the value.
         */
        values::type infer_type(bool detailed = false) const;

        /**
         * Converts the value to an array; the value is returned as an array if already an array.
         * Note: after calling this function, using the original value results in undefined behavior as it may have moved.
         * @param convert_hash True if hashes should be converted to an array of name-value arrays or false if not.
         * @return Returns the converted array.
         */
        array to_array(bool convert_hash = true);

        /**
         * Enumerates each "resource" type in the value.
         * This expects the value to be a Variant[String, Resource, Array[Variant[String, Resource]] or a collector.
         * @param callback The callback to invoke for each resource.
         * @param error The callback to invoke upon error.
         */
        void each_resource(std::function<void(runtime::types::resource const&)> const& callback, std::function<void(std::string const&)> const& error) const;

        /**
         * Creates a RapidJSON value for the value.
         * @param allocator The current RapidJSON allocator.
         * @return Returns the runtime value as a RapidJSON value.
         */
        json_value to_json(json_allocator& allocator) const;

        /**
         * Called to apply a visitor to the value.
         * This is responsible for automatically dereferencing a variable value.
         * @tparam Visitor The visitor type.
         * @param visitor The visitor to apply.
         * @return Returns the result from the visitor.
         */
        template <typename Visitor>
        typename Visitor::result_type apply_visitor(Visitor& visitor) const
        {
            if (auto ptr = boost::get<variable>(this)) {
                return ptr->value().apply_visitor(visitor);
            }
            return value_base::apply_visitor(visitor);
        }
    };

    /**
     * Stream insertion operator for runtime value.
     * @param os The output stream to write the runtime value to.
     * @param val The runtime value to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, value const& val);

    /**
     * Equality operator for value.
     * @param left The left value to compare.
     * @param right The right value to compare.
     * @return Returns true if the two values are equal or false if not.
     */
    bool operator==(value const& left, value const& right);

    /**
     * Inequality operator for value.
     * @param left The left value to compare.
     * @param right The right value to compare.
     * @return Returns true if the two values are not equal or false if they are equal.
     */
    bool operator!=(value const& left, value const& right);

    /**
     * Equality visitor for values that handles variable comparison.
     */
    struct equality_visitor : boost::static_visitor<bool>
    {
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
            typename U
        >
        result_type operator()(T const&, U const&) const
        {
            // Not the same type
            return false;
        }

        /**
         * Compares two value types.
         * @tparam T The type of the values being compared.
         * @param left The left hand value.
         * @param right The right hand value.
         * @return Returns true if both values are equal or false if not.
         */
        template <typename T>
        result_type operator()(T const& left, T const& right) const
        {
            // Same type, so use equality operator
            return left == right;
        }
    };

    /**
     * Compares a value with a value type.
     * @tparam T The type of the left-hand side.
     * @param left The value type to compare.
     * @param right The value to compare.
     * @return Returns true if the type stored in the value is equal to the value type.
     */
    template <
        typename T,
        typename = typename std::enable_if<boost::mpl::contains<typename value_base::types, T>::type::value>::type
    >
    bool operator==(T const& left, value const& right)
    {
        return boost::apply_visitor(std::bind(equality_visitor(), std::ref(left), std::placeholders::_1), right);
    }

    /**
     * Compares a value with a value type.
     * @tparam T The type of the right-hand side.
     * @param left The value to compare.
     * @param right The value type to compare.
     * @return Returns true if the type stored in the value is equal to the value type.
     */
    template <
        typename T,
        typename = typename std::enable_if<boost::mpl::contains<typename value_base::types, T>::type::value>::type
    >
    bool operator==(value const& left, T const& right)
    {
        return boost::apply_visitor(std::bind(equality_visitor(), std::placeholders::_1, std::ref(right)), left);
    }

    /**
     * Iterates each Unicode code point in a string.
     * @param str The string to iterate.
     * @param callback The callback to call for each Unicode code point, passed as a UTF-8 string.
     * @param reverse True to reverse the iteration or false if not.
     */
    void each_code_point(std::string const& str, std::function<bool(std::string)> const& callback, bool reverse = false);

}}}  // namespace puppet::runtime::values
