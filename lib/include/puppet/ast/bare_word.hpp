/**
 * @file
 * Declares the AST bare word.
 */
#pragma once

#include "../lexer/token_position.hpp"
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
         * Default constructor for bare_word.
         */
        bare_word();

        /**
         * Constructs a bare word from a token.
         * @tparam Iterator The underlying iterator type for the token.
         * @param token The token representing the bare word.
         */
        template <typename Iterator>
        explicit bare_word(boost::iterator_range<Iterator> const& token) :
            _position(token.begin().position()),
            _value(token.begin(), token.end())
        {
        }

        /**
         * Gets the value of the bare word.
         * @return Returns the value of the bare word.
         */
        std::string const& value() const;

        /**
         * Gets the position of the bare word.
         * @return Returns the position of the bare word.
         */
        lexer::token_position const& position() const;

    private:
        lexer::token_position _position;
        std::string _value;
    };

    /**
     * Stream insertion operator for AST bare word.
     * @param os The output stream to write the bare word to.
     * @param word The bare word to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, bare_word const& word);

}}  // namespace puppet::ast
