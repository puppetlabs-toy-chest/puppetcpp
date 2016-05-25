/**
 * @file
 * Declares the iterator runtime value.
 */
#pragma once

#include "wrapper.hpp"
#include "array.hpp"
#include "hash.hpp"
#include "regex.hpp"
#include "type.hpp"
#include <boost/variant.hpp>
#include <ostream>
#include <functional>
#include <exception>

namespace puppet { namespace runtime { namespace values {

    /**
     * Exception for type not iterable errors.
     */
    struct type_not_iterable_exception : std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

    /**
     * Represents the iterator value.
     */
    struct iterator
    {
        /**
         * The callback type for iterating over the iterator.
         * The parameters to the callback are the optional key (nullptr if not iterating a hash) and value.
         */
        using callback_type = std::function<bool(values::value const*, values::value const&)>;

        /**
         * Constructs an iterator based off an iterable value.
         * @param value The value being iterated.
         * @param step The iterator step size.
         * @param reverse True to reverse the iteration or false if not.
         */
        explicit iterator(wrapper<values::value> value, int64_t step = 1, bool reverse = false);

        /**
         * Gets the underlying iterable value.
         * This never returns an iterator, only an underlying value.
         * @return Returns the underlying iterable value.
         */
        values::value const& value() const;

        /**
         * Infers the type for the produced values of the iterator.
         * For example, Integer produces Integer, String produces String, Array[T] produces T, etc.
         * @return Returns the inferred type.
         */
        values::type infer_produced_type() const;

        /**
         * Gets the iterator's step count.
         * @return Returns the iterator's step count.
         */
        int64_t step() const;

        /**
         * Gets whether or not the iterator traverses in a reverse direction.
         * @return Returns true if the direction is reverse or false if not.
         */
        bool reverse() const;

        /**
         * Iterates over the iterator.
         * @param callback The callback to invoke for each iteration.
         * @param reverse True to reverse the iteration or false otherwise.
         */
        void each(callback_type const& callback, bool reverse = false) const;

     private:
        wrapper<values::value> _value;
        int64_t _step;
        bool _reverse;
    };

    /**
     * Stream insertion operator for runtime iterator.
     * @param os The output stream to write the runtime iterator to.
     * @param iterator The runtime iterator to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, values::iterator const& iterator);

    /**
     * Equality operator for iterator.
     * @param left The left iterator to compare.
     * @param right The right iterator to compare.
     * @return Returns true if all elements of the iterator are equal or false if not.
     */
    bool operator==(iterator const& left, iterator const& right);

    /**
     * Inequality operator for iterator.
     * @param left The left iterator to compare.
     * @param right The right iterator to compare.
     * @return Returns true if any elements of the iterators are not equal or false if all elements are equal.
     */
    bool operator!=(iterator const& left, iterator const& right);

    /**
     * Hashes the iterator value.
     * @param iterator The iterator value to hash.
     * @return Returns the hash value for the value.
     */
    size_t hash_value(values::iterator const& iterator);

    /**
     * Utility visitor for iteration.
     */
    struct iteration_visitor : boost::static_visitor<void>
    {
        /**
         * Constructs an iteration visitor.
         * @param callback The callback to invoke for each iteration.
         * @param step The iteration step count.
         * @param reverse True to reverse the iteration or false if not.
         */
        iteration_visitor(iterator::callback_type const& callback, int64_t step = 1, bool reverse = false);

     private:
        template<class> friend class ::boost::detail::variant::invoke_visitor;

        void operator()(undef const&) const;
        void operator()(defaulted const&) const;
        void operator()(int64_t value) const;
        void operator()(double) const;
        void operator()(bool) const;
        void operator()(std::string const& value) const;
        void operator()(values::regex const& value) const;
        void operator()(values::type const& value) const;
        void operator()(types::integer const& range) const;
        void operator()(types::enumeration const& enumeration) const;
        void operator()(variable const& value) const;
        void operator()(values::array const& value) const;
        void operator()(values::hash const& value) const;
        void operator()(values::iterator const& value) const;

        iterator::callback_type const& _callback;
        int64_t _step;
        bool _reverse;
    };

}}}  // namespace puppet::runtime::values
