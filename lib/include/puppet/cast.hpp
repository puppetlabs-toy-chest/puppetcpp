/**
 * @file
 * Declares the cast helpers.
 */
#pragma once

#include <type_traits>

namespace puppet {

    /**
     * Casts the given type to an rvalue reference.
     * Use this over std::move to check that move semantics are actually achieved.
     * Use of this function on a const reference will result in a compilation error.
     * @tparam T The underlying type.
     * @param t The reference to cast.
     * @return Returns the rvalue reference.
     */
    template <typename T>
    typename std::remove_reference<T>::type&& rvalue_cast(T&& t)
    {
        static_assert(
            !std::is_const<typename std::remove_reference<T>::type>::value,
            "Cannot rvalue-cast from a const reference as this will not invoke move semantics."
        );
        return std::move(t);
    }

}  // puppet
