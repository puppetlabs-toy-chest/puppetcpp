/**
 * @file
 * Declares the number token type.
 */
#pragma once

#include "position.hpp"
#include <boost/spirit/include/lex_lexer.hpp>
#include <cstdint>
#include <string>
#include <ostream>

namespace puppet { namespace compiler { namespace lexer {

    /**
     * Represents the numeric base of a number token.
     */
    enum class numeric_base
    {
        /**
         * Decimal (base 10).
         */
        decimal,
        /**
         * Octal (base 8).
         */
        octal,
        /**
         * Hexadecimal (base 16).
         */
        hexadecimal
    };

    /**
     * Represents a number token.
     */
    struct number_token
    {
        /**
         * The type of the numeric value.
         */
        using value_type = boost::variant<std::int64_t, double>;

        /**
         * The default constructor for number token.
         */
        number_token();

        /**
         * Constructs a number token with the given position and integral value.
         * @param position The position of the token.
         * @param value The integral value of the token.
         * @param base The numeric base of the token.
         */
        number_token(lexer::position position, std::int64_t value, numeric_base base);

        /**
         * Constructs a number token with the given position and floating point value.
         * @param position The position of the token.
         * @param value The floating point value of the token.
         */
        number_token(lexer::position position, double value);

        /**
         * Gets the position of the token.
         * @return Returns the position of the token.
         */
        lexer::position const& position() const;

        /**
         * Gets the value of the token.
         * @return Returns the value of the token.
         */
        value_type const& value() const;

        /**
         * Gets the numeric base of the token.
         * @return Returns the numeric base of the token.
         */
        numeric_base base() const;

     private:
        lexer::position _position;
        value_type _value;
        numeric_base _base;
    };

    /**
     * Stream insertion operator for number token.
     * @param os The output stream to write the token.
     * @param token The token to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, number_token const& token);

}}}  // namespace puppet::compiler::lexer

namespace boost { namespace spirit { namespace traits
{
    /**
     * Utility for converting iterator ranges into number tokens.
     */
    template <typename Iterator>
    struct assign_to_attribute_from_iterators<puppet::compiler::lexer::number_token, Iterator>
    {
        /**
         * Assigns a number token based on an iterator range.
         * Number tokens cannot be assigned from iterator range, only from values.
         * Calling this function will result in a runtime error.
         * @param first The first iterator.
         * @param last The last iterator.
         * @param attr The resulting number token attribute.
         */
        static void call(Iterator const& first, Iterator const& last, puppet::compiler::lexer::number_token& attr)
        {
            // This should not get called and only exists for the code to compile
            // Tokens that have number data associated with them should be assigned by value, not iterators
            throw std::runtime_error("attempt to assign number token from iterators.");
        }
    };

}}}  // namespace boost::spirit::traits
