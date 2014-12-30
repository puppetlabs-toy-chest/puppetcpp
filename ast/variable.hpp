/**
 * @file
 * Declares the AST variable.
 */
#pragma once

#include "../lexer/lexer.hpp"
#include <boost/range.hpp>
#include <iostream>
#include <string>

namespace puppet { namespace ast {

    /**
     * Represents an AST variable.
     */
    struct variable
    {
        /**
         * Default constructor for variable.
         */
        variable();

        /**
         * Constructs a variable from a token.
         * @tparam Iterator The underlying iterator type for the token.
         * @param token The token representing the variable.
         */
        template <typename Iterator>
        explicit variable(boost::iterator_range<Iterator> const& token) :
            _position(token.begin().position()),
            _name(token.begin(), token.end())
        {
        }

        /**
         * Gets the name of the variable.
         * @return Returns the name of the variable.
         */
        std::string const& name() const;

        /**
         * Gets the name of the variable.
         * @return Returns the name of the variable.
         */
        std::string& name();

        /**
         * Gets the position of the variable.
         * @return Returns the position of the variable.
         */
        lexer::token_position const& position() const;

     private:
        lexer::token_position _position;
        std::string _name;
    };

    /**
     * Stream insertion operator for AST variable.
     * @param os The output stream to write the variable to.
     * @param var The variable to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, variable const& var);

}}  // namespace puppet::ast
