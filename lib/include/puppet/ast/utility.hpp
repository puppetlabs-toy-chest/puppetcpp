/**
 * @file
 * Defines some utility functions.
 */
#pragma once

#include <boost/optional.hpp>
#include <string>
#include <iostream>
#include <vector>

namespace puppet { namespace ast {

    /**
     * Pretty prints a container.
    * @tparam Container The container type.
     * @param os The output stream to write to.
     * @param container The container to print.
     * @param delimiter The delimiter to write between each element.
     */
    template <typename Container>
    void pretty_print(std::ostream& os, Container const& container, std::string const& delimiter = ", ")
    {
        bool first = true;
        for (auto const& element : container) {
            if (first) {
                first = false;
            } else {
                os << delimiter;
            }
            os << element;
        }
    }

    /**
     * Pretty prints an optional container.
     * @tparam Container The container type.
     * @param os The output stream to write to.
     * @param container The optional container to print.
     * @param delimiter The delimiter to write between each element.
     */
    template <typename Container>
    void pretty_print(std::ostream& os, boost::optional<Container> const& container, std::string const& delimiter = ", ")
    {
        if (container) {
            pretty_print(os, *container, delimiter);
        }
    }
}}  // namespace puppet::ast
