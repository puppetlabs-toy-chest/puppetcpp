/**
 * @file
 * Declares the AST bare word.
 */
#pragma once

#include "../lexer/position.hpp"
#include <boost/range.hpp>
#include <iostream>
#include <string>

namespace puppet { namespace ast {

    /**
     * Represents an AST bare word.
     */
    struct bare_word
    {
        /**
         * Default constructor for bare word.
         */
        bare_word();

        /**
         * Constructs a bare word from a token.
         * @tparam Iterator The underlying iterator type for the token.
         * @param token The token representing the bare word.
         */
        template <typename Iterator>
        explicit bare_word(boost::iterator_range<Iterator> const& token) :
            position(token.begin().position()),
            value(token.begin(), token.end())
        {
        }

        /**
         * The position of the bare word.
         */
        lexer::position position;

        /**
         * The string value of the bare word.
         */
        std::string value;
    };

    /**
     * Stream insertion operator for AST bare word.
     * @param os The output stream to write the bare word to.
     * @param word The bare word to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, bare_word const& word);

}}  // namespace puppet::ast
