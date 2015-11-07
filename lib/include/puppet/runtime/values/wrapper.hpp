/**
 * @file
 * Declares the wrapper utility type used in the value variant.
 */
#pragma once

#include <memory>
#include <string>
#include <cstdint>
#include "../../cast.hpp"

namespace puppet { namespace runtime { namespace values {

    /**
     * Implements the value wrapper.
     * This is used over boost variant's wrapper because the wrapper does not implement a noexcept move constructor
     * and therefore makes vector<value> not movable (and, by extension, also makes value not movable).
     */
    template <typename T>
    struct wrapper
    {
        /**
         * The t
         */
        using value_type = T;

        /**
         * Constructs an empty wrapper.
         * The value represented by this wrapper will be undef.
         */
        wrapper() = default;

        /**
         * Copy constructor for wrapper.
         * @param other The other wrapper to copy.
         */
        wrapper(wrapper const& other) :
            _value(new value_type(*other))
        {
        }

        /**
         * Move constructor for wrapper.
         * Uses the default implementation.
         */
        wrapper(wrapper&&) noexcept = default;

        /**
         * Copy assignment operator for wrapper.
         * @param other The other wrapper to copy from.
         * @return Returns this wrapper.
         */
        wrapper& operator=(wrapper const& other)
        {
            _value.reset(new value_type(other.get()));
            return *this;
        }

        /**
         * Move assignment operator for wrapper.
         * Uses the default implementation.
         * @return Returns this wrapper.
         */
        wrapper& operator=(wrapper&&) noexcept = default;

        /**
         * Constructs a wrapper based on a value variant type.
         * @tparam U The variant type to construct with.
         * @param value The value to initialize the variant with.
         */
        template <
            typename U,
            typename = typename std::enable_if<!std::is_same<typename std::remove_reference<U>::type, wrapper>::value>::type
        >
        wrapper(U const& value) :
            _value(new value_type(value))
        {
        }

        /**
         * Constructs a wrapper based on a value variant type.
         * @tparam U The variant type to construct with.
         * @param value The value to initialize the variant with.
         */
        template <
            typename U,
            typename = typename std::enable_if<
                !std::is_const<U>::value &&
                std::is_rvalue_reference<U&&>::value &&
                !std::is_same<typename std::remove_reference<U>::type, wrapper>::value
            >::type
        >
        wrapper(U&& value) :
            _value(new value_type(rvalue_cast(value)))
        {
        }

        /**
         * Copy assignment operator for wrapper.
         * @tparam T The variant type to assign with.
         * @param value The value to assign.
         * @return Returns this wrapper.
         */
        template <
            typename U,
            typename = typename std::enable_if<!std::is_same<typename std::remove_reference<U>::type, wrapper>::value>::type
        >
        wrapper& operator=(T const& value)
        {
            if (!_value) {
                _value.reset(new value_type(value));
            } else {
                *_value = value;
            }
            return *this;
        }

        /**
         * Move assignment operator for wrapper.
         * @tparam T The variant type to assign with.
         * @param value The value to assign.
         * @return Returns this wrapper.
         */
        template <
            typename U,
            typename = typename std::enable_if<!std::is_same<typename std::remove_reference<U>::type, wrapper>::value>::type
        >
        wrapper& operator=(U&& value)
        {
            if (!_value) {
                _value.reset(new value_type(rvalue_cast(value)));
            } else {
                *_value = rvalue_cast(value);
            }
            return *this;
        }

        /**
         * Gets a reference to the wrapped value.
         * @return Returns a reference to the wrapped value.
         */
        value_type const& get() const
        {
            return _value ? *_value.get() : _undef;
        }

        /**
         * Gets a reference to the wrapped value.
         * @return Returns a reference to the wrapped value.
         */
        value_type& get()
        {
            return _value ? *_value.get() : _undef;
        }

        /**
         * Gets a pointer to the wrapped value.
         * @return Returns a pointer to the wrapped value.
         */
        value_type const* get_ptr() const
        {
            return _value ? _value.get() : &_undef;
        }

        /**
         * Gets a pointer to the wrapped value.
         * @return Returns a pointer to the wrapped value.
         */
        value_type* get_ptr()
        {
            return _value ? _value.get() : &_undef;
        }

        /**
         * Member access operator for the wrapper.
         * @return Returns a pointer to the wrapped value.
         */
        value_type const* operator->() const
        {
            return get_ptr();
        }

        /**
         * Member access operator for the wrapper.
         * @return Returns a pointer to the wrapped value.
         */
        value_type* operator->()
        {
            return get_ptr();
        }

        /**
         * Dereference operator for the wrapper.
         * @return Returns a reference to the wrapped value.
         */
        value_type const& operator*() const
        {
            return get();
        }

        /**
         * Dereference operator for the wrapper.
         * @return Returns a reference to the wrapped value.
         */
        value_type& operator*()
        {
            return get();
        }

        /**
         * Converts the wrapper to a value reference. a pointer to the wrapped value.
         * @return Returns a pointer to the wrapped value.
         */
        operator value_type const&() const
        {
            return get();
        }

        /**
         * Converts the wrapper to a value reference. a pointer to the wrapped value.
         * @return Returns a pointer to the wrapped value.
         */
        operator value_type&()
        {
            return get();
        }

        /**
         * Called to apply a visitor to the wrapped value.
         * @tparam Visitor The visitor type.
         * @param visitor The visitor to apply.
         * @return Returns the result from the visitor.
         */
        template <typename Visitor>
        typename Visitor::result_type apply_visitor(Visitor& visitor) const
        {
            if (!_value) {
                return _undef.apply_visitor(visitor);
            } else {
                return _value->apply_visitor(visitor);
            }
        }

    private:
        static value_type _undef;
        std::unique_ptr<value_type> _value;
    };

    /**
     * Stream insertion operator for wrapper.
     * @param os The output stream to write the wrapper to.
     * @param wrapper The wrapper to write.
     * @return Returns the given output stream.
     */
    template <typename T>
    std::ostream& operator<<(std::ostream& os, values::wrapper<T> const& wrapper)
    {
        os << *wrapper;
        return os;
    }

    /**
     * Equality operator for wrapper.
     * @param left The left wrapper to compare.
     * @param right The right value to compare.
     * @return Returns true if the two values are equal or false if not.
     */
    template <typename T>
    bool operator==(wrapper<T> const& left, T const& right)
    {
        return left.get() == right;
    }

    /**
     * Equality operator for wrapper.
     * @param left The left value to compare.
     * @param right The right wrapper to compare.
     * @return Returns true if the two values are equal or false if not.
     */
    template <typename T>
    bool operator==(T const& left, wrapper<T> const& right)
    {
        return left == right.get();
    }

    /**
     * Equality operator for wrapper.
     * @param left The left wrapper to compare.
     * @param right The right wrapper to compare.
     * @return Returns true if the two values are equal or false if not.
     */
    template <typename T>
    bool operator==(wrapper<T> const& left, wrapper<T> const& right)
    {
        return left.get() == right.get();
    }

    /**
     * Inequality operator for wrapper.
     * @param left The left wrapper to compare.
     * @param right The right value to compare.
     * @return Returns true if the two values are not equal or false if they are equal.
     */
    template <typename T>
    bool operator!=(wrapper<T> const& left, T const& right)
    {
        return left.get() != right;
    }

    /**
     * Inequality operator for wrapper.
     * @param left The left value to compare.
     * @param right The right wrapper to compare.
     * @return Returns true if the two values are not equal or false if they are equal.
     */
    template <typename T>
    bool operator!=(T const& left, wrapper<T> const& right)
    {
        return left != right.get();
    }

    /**
     * Inequality operator for wrapper.
     * @param left The left wrapper to compare.
     * @param right The right wrapper to compare.
     * @return Returns true if the two values are not equal or false if they are equal.
     */
    template <typename T>
    bool operator!=(wrapper<T> const& left, wrapper<T> const& right)
    {
        return left.get() != right.get();
    }

    /**
     * Hashes the wrapper.
     * @param wrapper The wrapper to hash.
     * @return Returns the hash value for the wrapper.
     */
    template <typename T>
    size_t hash_value(values::wrapper<T> const& wrapper)
    {
        return hash_value(*wrapper);
    }

}}}  // namespace puppet::runtime::values