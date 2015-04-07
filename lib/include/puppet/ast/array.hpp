/**
 * @file
 * Declares the AST array.
 */
#pragma once

#include "../lexer/token_position.hpp"
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
         * Default contructor for array.
         */
        array();

        /**
         * Constructs an array with the given optional elements.
         * @param bracket_position Tracks the position of the array's opening bracket.
         * @param elements The optional expressions that make up the elements of the array.
         */
        array(lexer::token_position bracket_position, boost::optional<std::vector<expression>> elements);

        /**
         * Gets the optional elements of the array.
         * @return Returns the optional elements of the array.
         */
        boost::optional<std::vector<expression>> const& elements() const;

        /**
         * Gets the position of the array.
         * @return Returns the position of the array.
         */
        lexer::token_position const& position() const;

    private:
        lexer::token_position _position;
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
