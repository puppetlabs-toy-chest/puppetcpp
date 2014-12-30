/**
 * @file
 * Declares the AST type.
 */
#pragma once

#include "../lexer/lexer.hpp"
#include <boost/range.hpp>
#include <iostream>
#include <string>

namespace puppet { namespace ast {

    /**
     * Represents an AST type.
     */
    struct type
    {
        /**
         * Default constructor for type.
         */
        type();

        /**
         * Constructs a type from a token.
         * @tparam Iterator The underlying iterator type for the token.
         * @param token The token representing the type.
         */
        template <typename Iterator>
        explicit type(boost::iterator_range<Iterator> const& token) :
            _position(token.begin().position()),
            _name(token.begin(), token.end())
        {
        }

        /**
         * Gets the name of the type.
         * @return Returns the name of the type.
         */
        std::string const& name() const;

        /**
         * Gets the name of the type.
         * @return Returns the name of the type.
         */
        std::string& name();

        /**
         * Gets the position of the type.
         * @return Returns the position of the type.
         */
        lexer::token_position const& position() const;

     private:
        lexer::token_position _position;
        std::string _name;
    };

    /**
     * Stream insertion operator for AST type.
     * @param os The output stream to write the type to.
     * @param type The type to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, ast::type const& type);

}}  // namespace puppet::ast
