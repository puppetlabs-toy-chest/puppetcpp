/**
 * @file
 * Declares the AST array.
 */
#pragma once

#include "../lexer/position.hpp"
#include "expression.hpp"
#include <boost/optional.hpp>
#include <iostream>
#include <vector>

namespace puppet { namespace ast {

    /**
     * Represents an AST array.
     */
    struct array
    {
        /**
         * Default constructor for array.
         */
        array() = default;

        /**
         * Constructs an array with the given optional elements.
         * @param position The position of the array.
         * @param elements The optional expressions that make up the elements of the array.
         */
        array(lexer::position position, boost::optional<std::vector<expression>> elements);

        /**
         * Gets the optional elements of the array.
         * @return Returns the optional elements of the array.
         */
        boost::optional<std::vector<expression>> const& elements() const;

        /**
         * Gets the position of the array.
         * @return Returns the position of the array.
         */
        lexer::position const& position() const;

    private:
        lexer::position _position;
        boost::optional<std::vector<expression>> _elements;
    };

    /**
     * Stream insertion operator for AST array.
     * @param os The output stream to write the array to.
     * @param array The array to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, array const& array);

}}  // namespace puppet::ast
