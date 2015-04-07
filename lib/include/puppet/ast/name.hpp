/**
 * @file
 * Declares the AST name.
 */
#pragma once

#include "../lexer/token_position.hpp"
#include <boost/range.hpp>
#include <iostream>
#include <string>

namespace puppet { namespace ast {

    /**
     * Represents an AST name.
     */
    struct name
    {
        /**
         * Default constructor for name.
         */
        name();

        /**
         * Constructs a name from a token.
         * @tparam Iterator The underlying iterator type for the token.
         * @param token The token representing the name.
         */
        template <typename Iterator>
        explicit name(boost::iterator_range<Iterator> const& token) :
            _position(token.begin().position()),
            _value(token.begin(), token.end())
        {
        }

        /**
         * Gets the value of the name.
         * @return Returns the value of the name.
         */
        std::string const& value() const;

        /**
         * Gets the position of the name.
         * @return Returns the position of the name.
         */
        lexer::token_position const& position() const;

    private:
        lexer::token_position _position;
        std::string _value;
    };

    /**
     * Stream insertion operator for AST name.
     * @param os The output stream to write the name to.
     * @param name The name to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, name const& name);

}}  // namespace puppet::ast
