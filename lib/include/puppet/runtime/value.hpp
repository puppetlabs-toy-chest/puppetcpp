/**
 * @file
 * Declares the runtime value.
 */
#pragma once

#include <boost/functional/hash.hpp>
#include <boost/variant.hpp>
#include <string>
#include <unordered_map>
#include <regex>
#include <cstddef>
#include <functional>

namespace puppet { namespace runtime {

    /**
     * Represents the undefined value.
     */
    struct undef
    {
    };

    /**
     * Stream insertion operator for runtime undef.
     * @param os The output stream to write the runtime undef to.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, undef const&);

    /**
     * Represents a runtime regex.
     */
    struct regex
    {
        /**
         * Default constructor for regex.
         */
        regex();

        /**
         * Constructs a regex with the given pattern.
         * @param pattern The pattern for the regex.
         */
        regex(std::string pattern);

        /**
         * Gets the pattern for the regex.
         * @return Returns the pattern for the regex.
         */
        std::string const& pattern() const;

        /**
         * Gets the pattern for the regex.
         * @return Returns the pattern for the regex.
         */
        std::string& pattern();

        /**
         * Gets the value of the regex.
         * @return Returns the value of the regex.
         */
        std::regex const& value() const;

        /**
         * Gets the value of the regex.
         * @return Returns the value of the regex.
         */
        std::regex& value();

     private:
        std::string _pattern;
        std::regex _regex;
    };

    /**
     * Stream insertion operator for runtime regex.
     * @param os The output stream to write the runtime regex to.
     * @param regx The runtime regex to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, regex const& regx);

    /**
     * The kinds of runtime types supported.
     */
    enum class type_kind
    {
        /**
         * Unknown type.
         */
        unknown,
        /**
         * Represents the Any type.
         */
        any,
        /**
         * Represents the Scalar type.
         */
        scalar,
        /**
         * Represents the Numeric type.
         */
        numeric,
        /**
         * Represents the Integer[from, to] type.
         */
        integer,
        /**
         * Represents the Float[from, to] type.
         */
        floating,
        /**
         * Represents the String[from, to] type.
         */
        string,
        /**
         * Represents the Enum[*strings] type.
         */
        enumeration,
        /**
         * Represents the Pattern[*patterns] type.
         */
        pattern,
        /**
         * Represents the Boolean type.
         */
        boolean,
        /**
         * Represents the Regexp type.
         */
        regexp,
        /**
         * Represents the Collection type.
         */
        collection,
        /**
         * Represents the Array[T] type.
         */
        array,
        /**
         * Represents the Hash[K, V] type.
         */
        hash,
        /**
         * Represents the Variant[*T] type.
         */
        variant,
        /**
         * Represents the Optional[T] type.
         */
        optional,
        /**
         * Represents the CatalogEntry type.
         */
        catalog_entry,
        /**
         * Represents the Resource[type_name, title] type.
         */
        resource,
        /**
         * Represents the Class[name] type.
         */
        klass,
        /**
         * Represents the Undef type
         */
        undef,
        /**
         * Represents the Data type.
         */
        data,
        /**
         * Represents the Callable type.
         */
        callable,
        /**
         * Represents the Type[T] type.
         */
        type,
        /**
         * Represents the Runtime[runtime_name, type_name] type.
         */
        runtime,
        /**
         * Represents the Default type.
         */
        default_
    };

    /**
     * Gets the type kind for the given type name.
     * @param name The name of the type.
     * @return Returns the type kind or type_kind::unknown.
     */
    type_kind get_type_kind(std::string const& name);

    /**
     * Stream insertion operator for type kind.
     * @param os The output stream to write the type kind to.
     * @param kind The type kind to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, type_kind kind);

    /**
     * Represents a runtime type (instance of a Type).
     * @tparam ValueType The type of value.
     */
    template <typename ValueType>
    struct runtime_type
    {
        /**
         * Constructs a runtime type with the given type kind.
         * @param kind The type kind.
         */
        explicit runtime_type(type_kind kind) :
            _kind(kind)
        {
        }

        /**
         * Gets the kind of runtime type.
         * @return Returns the kind of the runtime type.
         */
        type_kind kind() const
        {
            return _kind;
        }

        /**
         * Gets the runtime type's parameters.
         * @return Returns the runtime type's parameters.
         */
        std::vector<ValueType> const& parameters() const
        {
            return _parameters;
        }

        /**
         * Adds a parameter to the runtime type.
         * @param parameter The parameter to add.
         */
        void add_parameter(ValueType parameter)
        {
            // TODO: implement for use with access operator
        }

    private:
        type_kind _kind;
        std::vector<ValueType> _parameters;
    };

    /**
     * Represents a reference to a variable.
     * Having this as a runtime value prevents unnecessary copying of a variable's value.
     * Thus, '$a = $b' simply points $a's value at what $b was set to.
     * @tparam ValueType The runtime value type.
     */
    template <typename ValueType>
    struct variable_reference
    {
        /**
         * The runtime value type.
         */
        typedef ValueType value_type;

        /**
         * Constructs a variable reference.
         * @param name The name of the variable.
         * @param val The current value of the variable.
         */
        variable_reference(std::string name, value_type const* val) :
            _name(std::move(name)),
            _value(val)
        {
        }

        /**
         * Gets the name of the variable.
         * @return Returns the name of the variable.
         */
        std::string const& name() const
        {
            return _name;
        }

        /**
         * Gets the value of the variable.
         * @return Returns the value of the variable.
         */
        value_type const& value() const
        {
            static value_type undefined;
            return _value ? *_value : undefined;
        }

        /**
         * Updates the value of the variable.
         * @param ptr The pointer to the variable's value.
         */
        void update(value_type const* ptr)
        {
            _value = ptr;
        }

     private:
        std::string _name;
        value_type const* _value;
    };

    namespace hack {
        /**
         * Compares two values for equality by untyped pointers.
         * This exists to support equals_to.  Do not use with anything but pointers to values.
         * @param left The left value to compare.
         * @param right The right value to compare.
         * @return Returns true if the two values are equal or false if they are not equal.
         */
        bool unsafe_equals(void const* left, void const* right);

        /**
         * This exists to break a circular declaration dependency with the recursive variant.
         * We do not want to use std::equals because it will define equality in terms of variant's operator==.
         * That operator will prevent correct equality when comparing variables.
         */
        template<typename T>
        struct equals_to
        {
            /**
             * Compares two values for equality.
             * @param left The left value to compare.
             * @param right The right value to compare.
             * @return Returns true if the two values are equal or false if they are not.
             */
            bool operator()(T const& left, T const& right) const
            {
                return unsafe_equals(&left, &right);
            }
        };
    }  // namespace details

    /**
     * Represents the possible runtime value types.
     * Note: undef should always come first (default value).
     */
    typedef boost::make_recursive_variant<
        undef,
        std::int64_t,
        long double,
        bool,
        std::string,
        regex,
        runtime_type<boost::recursive_variant_>,
        variable_reference<boost::recursive_variant_>,
        std::vector<boost::recursive_variant_>,
        std::unordered_map<
            boost::recursive_variant_,
            boost::recursive_variant_,
            boost::hash<boost::recursive_variant_>,
            hack::equals_to<boost::recursive_variant_>
        >
    >::type value;

    /**
     * Stream insertion operator for runtime value.
     * @param os The output stream to write the runtime value to.
     * @param val The runtime value to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, value const& val);

    /**
     * Type definition for runtime type.
     */
    typedef runtime_type<value> type;

    /**
     * Stream insertion operator for runtime type.
     * @param os The output stream to write the runtime type to.
     * @param t The type to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, type const& t);

    /**
     * Type definition for runtime variable.
     */
    typedef variable_reference<value> variable;

    /**
     * Stream insertion operator for runtime variable.
     * @param os The output stream to write the runtime variable to.
     * @param var The runtime variable to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, variable const& var);

    /**
     * Type definition for runtime array.
     */
    typedef std::vector<value> array;

    /**
     * Stream insertion operator for runtime array.
     * @param os The output stream to write the runtime array to.
     * @param arr The runtime array to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, array const& arr);

    /**
     * Type definition for runtime hash.
     */
    typedef std::unordered_map<value, value, boost::hash<value>, hack::equals_to<value>> hash;

    /**
     * Stream insertion operator for runtime hash.
     * @param os The output stream to write the runtime hash to.
     * @param h The runtime hash to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, hash const& h);

    /**
     * Dereferences a value.
     * @param val The value to dereference.
     * @return Returns the value of a variable or the original value if not a variable.
     */
    value const& dereference(value const& val);

    /**
     * Determines if a value is "truthy".
     * @param val The value to test for "truthiness".
     * @return Returns true if the value is "truthy" or false if it is not.
     */
    bool is_truthy(value const& val);

    /**
     * Gets the type of the given value.
     * @param val The runtime value to get the type of.
     * @return Returns the runtime type of the value.
     */
    type get_type(value const& val);

    /**
     * Equality operator for undef.
     * @return Always returns true.
     */
    bool operator==(undef const&, undef const&);

    /**
     * Equality operator for regex.
     * @param left The left regex to compare.
     * @param right The right regex to compare.
     * @return Returns true if both regexes have the same pattern or false if not.
     */
    bool operator==(regex const& left, regex const& right);

    /**
     * Equality operator for type.
     * @param left The left type to compare.
     * @param right The right type to compare.
     * @return Returns true if the runtime types are equal or false if not.
     */
    bool operator==(type const& left, type const& right);

    /**
     * Equality operator for variable.
     * @param left The left variable to compare.
     * @param right The right variable to compare.
     * @return Returns true if the values referenced by the variables are equal or false if not; does not compare variable names.
     */
    bool operator==(variable const& left, variable const& right);

    /**
     * Equality operator for array.
     * @param left The left array to compare.
     * @param right The right array to compare.
     * @return Returns true if all elements of the array are equal or false if not.
     */
    bool operator==(array const& left, array const& right);

    /**
     * Equality operator for hash.
     * @param left The left hash to compare.
     * @param right The right hash to compare.
     * @return Returns true if all keys and values of the hash are equal or false if not.
     */
    bool operator==(hash const& left, hash const& right);

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

}}  // namespace puppet::runtime

namespace boost
{
    /**
     * Hash specialization for undef.
     */
    template <>
    struct hash<puppet::runtime::undef>
    {
        /**
         * Hashes the undef value.
         * Note: all undef values hash the same.
         * @return Returns a constant hash value.
         */
        size_t operator()(puppet::runtime::undef const&) const
        {
            return 0;
        }
    };

    /**
     * Hash specialization for runtime type kind.
     */
    template <>
    struct hash<puppet::runtime::type_kind>
    {
        /**
         * Hashes the runtime type kind.
         * @param kind The kind to hash.
         * @return Returns the hash value for the type kind.
         */
        size_t operator()(puppet::runtime::type_kind kind) const
        {
            return hash_value(static_cast<size_t>(kind));
        }
    };

    /**
     * Hash specialization for runtime type.
     */
    template <>
    struct hash<puppet::runtime::type>
    {
        /**
         * Hashes the runtime type.
         * @param t The type to hash.
         * @return Returns the hash value for the type.
         */
        size_t operator()(puppet::runtime::type const& t) const
        {
            std::size_t seed = 0;
            boost::hash_combine(seed, static_cast<size_t>(t.kind()));
            boost::hash_combine(seed, t.parameters());
            return seed;
        }
    };

    /**
     * Hash specialization for regex.
     */
    template <>
    struct hash<puppet::runtime::regex>
    {
        /**
         * Hashes the regex value.
         * @param regex The regex to hash.
         * @return Returns the hash value for the regex.
         */
        size_t operator()(puppet::runtime::regex const& regex) const
        {
            return hash_value(regex.pattern());
        }
    };

    /**
     * Hash specialization for variable.
     */
    template <>
    struct hash<puppet::runtime::variable>
    {
        /**
         * Hashes the variable.
         * @param var The variable to hash.
         * @return Returns the hash value for the variable.
         */
        size_t operator()(puppet::runtime::variable const& var) const
        {
            return hash_value(var.value());
        }
    };

    /**
     * Hash specialization for hash type.
     */
    template <>
    struct hash<puppet::runtime::hash>
    {
        /**
         * Hashes the given hash type.
         * @param hash The hash type to hash.
         * @return Returns the hash value for the hash type.
         */
        size_t operator()(puppet::runtime::hash const& hash) const
        {
            return hash_range(hash.begin(), hash.end());
        }
    };

}  // namespace boost
