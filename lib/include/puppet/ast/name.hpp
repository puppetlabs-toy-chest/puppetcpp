/**
 * @file
 * Declares the AST name.
 */
#pragma once

#include "../lexer/position.hpp"
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
         * Constructs an AST name with the given position and value.
         * @param position The position of the name.
         * @param value The value of the name.
         */
        name(lexer::position position, std::string value);

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
        lexer::position const& position() const;

    private:
        lexer::position _position;
        std::string _value;
    };

    /**
     * Equality operator for AST name.
     * @param left The left operand.
     * @param right The right operand.
     * @return Returns true if the two names are equal or false if they are not equal.
     */
    bool operator==(ast::name const& left, ast::name const& right);

    /**
     * Stream insertion operator for AST name.
     * @param os The output stream to write the name to.
     * @param name The name to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, name const& name);

}}  // namespace puppet::ast

namespace std
{
    /**
     * Responsible for hashing AST name.
     */
    template<>
    struct hash<puppet::ast::name>
    {
        /**
         * The argument type.
         */
        typedef puppet::ast::name argument_type;
        /**
         * The result type of the hash.
         */
        typedef std::hash<std::string>::result_type result_type;

        /**
         * Responsible for hashing the argument.
         * @param arg The argument to hash.
         * @return Returns the hash value.
         */
        result_type operator()(argument_type const& arg) const
        {
            std::hash<std::string> hasher;
            return hasher(arg.value());
        }
    };
}
