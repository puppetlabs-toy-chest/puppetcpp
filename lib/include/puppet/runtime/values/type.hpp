/**
 * @file
 * Declares the type runtime value.
 */
#pragma once

#include "../values/forward.hpp"
#include "../types/any.hpp"
#include "../types/array.hpp"
#include "../types/boolean.hpp"
#include "../types/callable.hpp"
#include "../types/catalog_entry.hpp"
#include "../types/class.hpp"
#include "../types/collection.hpp"
#include "../types/data.hpp"
#include "../types/defaulted.hpp"
#include "../types/enumeration.hpp"
#include "../types/floating.hpp"
#include "../types/hash.hpp"
#include "../types/integer.hpp"
#include "../types/not_undef.hpp"
#include "../types/numeric.hpp"
#include "../types/optional.hpp"
#include "../types/pattern.hpp"
#include "../types/regexp.hpp"
#include "../types/resource.hpp"
#include "../types/runtime.hpp"
#include "../types/scalar.hpp"
#include "../types/string.hpp"
#include "../types/struct.hpp"
#include "../types/tuple.hpp"
#include "../types/type.hpp"
#include "../types/undef.hpp"
#include "../types/variant.hpp"
#include "../../cast.hpp"
#include <boost/variant.hpp>
#include <boost/optional.hpp>

namespace puppet { namespace compiler { namespace evaluation {

    // Forward declare the evaluation context.
    struct context;

}}}  // namespace puppet::compiler::evaluation

namespace puppet { namespace runtime { namespace values {

    /**
     * The variant representing all possible types.
     * The type value does not derive from this because Boost.Variant does not support variants (like value) containing
     * types derived from boost::variant (ambiguous construction between the type variant and a variant<U...>).
     */
    using type_variant = boost::variant<
        types::any,
        types::array,
        types::boolean,
        types::callable,
        types::catalog_entry,
        types::klass,
        types::collection,
        types::data,
        types::defaulted,
        types::enumeration,
        types::floating,
        types::hash,
        types::integer,
        types::not_undef,
        types::numeric,
        types::optional,
        types::pattern,
        types::regexp,
        types::resource,
        types::runtime,
        types::scalar,
        types::string,
        types::structure,
        types::tuple,
        types::type,
        types::undef,
        types::variant
    >;

    /**
     * Represents the type runtime value.
     */
    struct type
    {
        /**
         * Default constructor for type.
         */
        type() = default;

        /**
         * Constructs a type based on a variant type.
         * @tparam T The variant type to construct with.
         * @param value The value to initialize the variant with.
         */
        template <
            typename T,
            typename = typename std::enable_if<!std::is_same<typename std::remove_reference<T>::type, type>::value>::type
        >
        type(T const& value) :
            _value(value)
        {
        }

        /**
         * Constructs a type based on a variant type.
         * @tparam T The variant type to construct with.
         * @param value The value to initialize the variant with.
         */
        template <
            typename T,
            typename = typename std::enable_if<!std::is_same<typename std::remove_reference<T>::type, type>::value>::type
        >
        type(T&& value) :
            _value(rvalue_cast(value))
        {
        }

        /**
         * Copy assignment operator for type.
         * @tparam T The variant type to assign with.
         * @param value The value to assign.
         * @return Returns this type.
         */
        template <
            typename T,
            typename = typename std::enable_if<!std::is_same<typename std::remove_reference<T>::type, type>::value>::type
        >
        type& operator=(T const& value)
        {
            _value = value;
            return *this;
        }

        /**
         * Move assignment operator for type.
         * @tparam T The variant type to assign with.
         * @param value The value to assign.
         * @return Returns this type.
         */
        template <
            typename T,
            typename = typename std::enable_if<!std::is_same<typename std::remove_reference<T>::type, type>::value>::type
        >
        type& operator=(T&& value)
        {
            _value = rvalue_cast(value);
            return *this;
        }

        /**
         * Applies a visitor to the type variant.
         * @tparam Visitor The visitor type.
         * @param visitor The visitor to apply.
         * @return Returns the result of the visitor.
         */
        template <typename Visitor>
        typename Visitor::result_type apply_visitor(Visitor& visitor)
        {
            return _value.apply_visitor(visitor);
        }

        /**
         * Applies a visitor to the type variant.
         * @tparam Visitor The visitor type.
         * @param visitor The visitor to apply.
         * @return Returns the result of the visitor.
         */
        template <typename Visitor>
        typename Visitor::result_type apply_visitor(Visitor& visitor) const
        {
            return _value.apply_visitor(visitor);
        }

        /**
         * Gets the type variant for the type.
         * @return Returns the type variant for the type.
         */
        type_variant& get();

        /**
         * Gets the type variant for the type.
         * @return Returns the type variant for the type.
         */
        type_variant const& get() const;

        /**
         * Determines if the value is an instance of this type.
         * @param value The value to check if being an instance of this type.
         * @return Returns true if the value is an instance of this type or false if not.
         */
        bool is_instance(values::value const& value) const;

        /**
         * Determines if the given type is a specialization of this type.
         * @param type The type to check for specialization.
         * @return Returns true if the given type is a specialization of this type or false if not.
         */
        bool is_specialization(values::type const& type) const;

        /**
         * Creates a type from a Puppet type expression.
         * @param context The current evaluation context.
         * @param expression The expression to parse for the type.
         * @return Returns the type if the parse was successful or boost::none if the parsing failed.
         */
        static boost::optional<type> parse(compiler::evaluation::context& context, std::string const& expression);

     private:
        type_variant _value;
    };

    /**
     * Equality operator for type.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if types are equal or false if not.
     */
    bool operator==(type const& left, type const& right);

    /**
     * Inequality operator for type.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the two types are not equal or false if they are equal.
     */
    bool operator!=(type const& left, type const& right);

    /**
     * Hashes the type value.
     * @param type The type value to hash.
     * @return Returns the hash value for the type.
     */
    size_t hash_value(values::type const& type);

}}}  // puppet::runtime::values

namespace boost {

    /**
     * Gets a value from a type.
     * @tparam T The type to get.
     * @param type The type to get a value from.
     * @return Returns the type or throws boost::bad_get if the type is not of the requested value.
     */
    template <typename T>
    inline T& get(puppet::runtime::values::type& type)
    {
        return boost::get<T>(type.get());
    }

    /**
     * Gets a value from a type.
     * @tparam T The type to get.
     * @param type The type to get a value from.
     * @return Returns the type or throws boost::bad_get if the type is not of the requested value.
     */
    template <typename T>
    inline T const& get(puppet::runtime::values::type const& type)
    {
        return boost::get<T>(type.get());
    }

    /**
     * Gets a value from a type.
     * @tparam T The type to get.
     * @param type The type to get a value from.
     * @return Returns the type or nullptr if the type is not of the requested value.
     */
    template <typename T>
    inline T* get(puppet::runtime::values::type* type)
    {
        return boost::get<T>(&type->get());
    }

    /**
     * Gets a value from a type.
     * @tparam T The type to get.
     * @param type The type to get a value from.
     * @return Returns the type or nullptr if the type is not of the requested value.
     */
    template <typename T>
    inline T const* get(puppet::runtime::values::type const* type)
    {
        return boost::get<T>(&type->get());
    }

}  // namespace boost