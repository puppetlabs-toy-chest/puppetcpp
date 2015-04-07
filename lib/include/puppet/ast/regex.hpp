/**
 * @file
 * Declares the AST regex.
 */
#pragma once

#include "../lexer/token_position.hpp"
#include <boost/range.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <string>

namespace puppet { namespace ast {

    /**
     * Represents an AST regex.
     */
    struct regex
    {
        /**
         * Default constructor for regex.
         */
        regex();

        /**
         * Constructs a regex from a token.
         * @tparam Iterator The underlying iterator type for the token.
         * @param token The token representing the regex.
         */
        template <typename Iterator>
        explicit regex(boost::iterator_range<Iterator> const& token) :
            _position(token.begin().position())
        {
            auto start = token.begin();
            auto end = token.end();

            if (start != end) {
                ++start;
            }

            // Move the end to the position before the closing / character
            auto last = start;
            for (auto current = start; current != end; ++current) {
                last = current;
            }
            _value.assign(start, last);
        }

        /**
         * Gets the value of the regex.
         * @return Returns the value of the regex.
         */
        std::string const& value() const;

        /**
         * Gets the position of the regex.
         * @return Returns the position of the regex.
         */
        lexer::token_position const& position() const;

    private:
        lexer::token_position _position;
        std::string _value;
    };

    /**
     * Stream insertion operator for AST regex.
     * @param os The output stream to write the regex to.
     * @param regex The regex to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, regex const& regex);

}}  // namespace puppet::ast
