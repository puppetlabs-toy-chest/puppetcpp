/**
 * @file
 * Declares the runtime string interpolator.
 */
#pragma once

#include "../lexer/static_lexer.hpp"
#include "../lexer/lexer.hpp"
#include "expression_evaluator.hpp"

namespace puppet { namespace runtime {

    /**
     * Represents the runtime string interpolator.
     */
    struct string_interpolator
    {
        /**
         * Constructs an string interpolator.
         * @param evaluator The expression evaluator to use.
         */
        explicit string_interpolator(expression_evaluator& evaluator);

        /**
         * Interpolates a string in the current evaluation context.
         * @param position The position where the string token starts.
         * @param text The text to interpolate.
         * @param escapes The list of valid escape characters.
         * @param quote The quote character for the string (null character for heredocs).
         * @param full True if full interpolation is performed or false if only escape character interpolation is performed.
         * @param margin The whitespace margin for the interpolated string.
         * @param remove_break True if a trailing line break should be removed or false if not.
         * @return Returns the interpolated string.
         */
        std::string interpolate(lexer::token_position const& position, std::string const& text, std::string const& escapes, char quote, bool full = true, int margin = 0, bool remove_break = false);

     private:
        bool write_unicode_escape_sequence(lexer::token_position const& position, lexer::lexer_string_iterator& begin, lexer::lexer_string_iterator const& end, std::string& result, bool four_characters = true);

        expression_evaluator& _evaluator;
    };

}}  // puppet::runtime