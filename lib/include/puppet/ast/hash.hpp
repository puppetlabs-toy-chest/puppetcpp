/**
 * @file
 * Declares the AST hash.
 */
#pragma once

#include "../lexer/position.hpp"
#include "expression.hpp"
#include <boost/optional.hpp>
#include <iostream>
#include <vector>

namespace puppet { namespace ast {

    /**
     * Represents the key-value pair of a hash.
     */
    typedef std::pair<expression, expression> hash_pair;

    /**
     * Stream insertion operator for AST hash pair.
     * @param os The output stream to write the pair to.
     * @param pair The pair to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, hash_pair const& pair);

    /**
     * Represents an AST hash.
     */
    struct hash
    {
        /**
         * Default constructor for hash.
         */
        hash();

        /**
         * Constructs a hash with the given position and optional name-value pair elements.
         * @param position The position of the hash.
         * @param elements The optional name-value pair elements of the hash.
         */
        hash(lexer::position position, boost::optional<std::vector<hash_pair>> elements);

        /**
         * The position of the hash.
         */
        lexer::position position;

        /**
         * The elements of the hash.
         */
        boost::optional<std::vector<hash_pair>> elements;
    };

    /**
     * Stream insertion operator for AST hash.
     * @param os The output stream to write the hash to.
     * @param hash The hash to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, hash const& hash);

}}  // namespace puppet::ast
