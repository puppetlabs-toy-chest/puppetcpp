/**
 * @file
 * Declares lexer tokens.
 */
#pragma once

#include "position.hpp"
#include "../../cast.hpp"
#include <boost/spirit/include/lex_lexer.hpp>
#include <string>
#include <ostream>

namespace puppet { namespace compiler { namespace lexer {

    /**
     * Represents the kinds of tokens returned by the lexer.
     * Every token returned from the lexer will either have one of these id values or be less than 128.
     * If the id is less than 128, the token represents a literal character token.
     */
    enum class token_id
    {
        unknown = boost::spirit::lex::min_token_id,
        append, // Not supported in grammar, but kept for legacy parsing
        remove, // Not supported in grammer, but kept for legacy parsing
        equals,
        not_equals,
        match,
        not_match,
        greater_equals,
        less_equals,
        fat_arrow,
        plus_arrow,
        left_shift,
        left_double_collect,
        left_collect,
        right_shift,
        right_double_collect,
        right_collect,
        atat,
        in_edge,
        in_edge_sub,
        out_edge,
        out_edge_sub,
        first_keyword,      // Add keywords after this id
        keyword_case,
        keyword_class,
        keyword_default,
        keyword_define,
        keyword_if,
        keyword_elsif,
        keyword_else,
        keyword_inherits,
        keyword_node,
        keyword_and,
        keyword_or,
        keyword_undef,
        keyword_false,
        keyword_true,
        keyword_in,
        keyword_unless,
        keyword_function,
        keyword_type,
        keyword_attr,
        keyword_private,
        keyword_produces,
        keyword_consumes,
        keyword_application,
        keyword_site,
        keyword_break,
        last_keyword,       // Add new keywords before this id
        statement_call,
        string,
        string_start,
        string_end,
        string_text,
        interpolation_start,
        interpolation_end,
        bare_word,
        variable,
        type,
        name,
        regex,
        number,
        array_start,          // Same as '[', but whitespace delimited to force array expression
        epp_start,
        epp_end,
        epp_start_trim,
        epp_end_trim,
        epp_render_string,
        epp_render_expression,
        comment,              // Not in token stream
        whitespace,           // Not in token stream
        unclosed_comment,     // Error token that will not match the grammar
    };

    /**
     * Determines if the token id is for a keyword.
     * @param id The token id.
     * @return Returns true if the id is for a keyword or false if not.
     */
    bool is_keyword(token_id id);

    /**
     * Stream insertion operator for token id.
     * @param os The output stream to write the token id.
     * @param id The token id to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, token_id const& id);

    /**
     * Utility visitor for getting the range of a token.
     */
    struct token_range_visitor : boost::static_visitor<std::pair<position, position>>
    {
        /**
         * Gets the range of a token.
         * @tparam Token The type of token.
         * @param token The token to get the range of.
         * @return Returns the token range.
         */
        template <typename Iterator>
        result_type operator()(boost::iterator_range<Iterator> const& token) const
        {
            return std::make_pair(token.begin().position(), token.end().position());
        }

        /**
         * Gets the range of a token.
         * @tparam Token The type of token.
         * @param token The token to get the range of.
         * @return Returns the token range.
         */
        template <typename Token>
        result_type operator()(Token const& token) const
        {
            return std::make_pair(token.begin, token.end);

        }
    };

    /**
     * Represents data about a string token.
     * Used for non-interpolated heredocs and single-quoted strings.
     */
    struct string_token
    {
        /**
         * Stores the beginning position for the token.
         */
        lexer::position begin;

        /**
         * Stores the ending position for the token.
         */
        lexer::position end;

        /**
         * Stores the value for the token.
         */
        std::string value;

        /**
         * Stores the string format (heredoc only).
         */
        std::string format = {};

        /**
         * Stores the string margin (heredoc only).
         */
        size_t margin = 0;
    };

    /**
     * Stream insertion operator for string token.
     * @param os The output stream to write the token.
     * @param token The token to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, string_token const& token);

    /**
     * Represents data about the start of an interpolated string.
     * Used for double quoted strings and interpolated heredocs.
     */
    struct string_start_token
    {
        /**
         * Stores the beginning position for the token.
         */
        lexer::position begin;

        /**
         * Stores the ending position for the token.
         */
        lexer::position end;

        /**
         * Stores the string format (heredoc only).
         */
        std::string format = {};
    };

    /**
     * Stream insertion operator for string start token.
     * @param os The output stream to write the token.
     * @param token The token to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, string_start_token const& token);

    /**
     * Represents literal text of an interpolated string.
     */
    struct string_text_token
    {
        /**
         * Stores the beginning position for the token.
         */
        lexer::position begin;

        /**
         * Stores the ending position for the token.
         */
        lexer::position end;

        /**
         * Stores the string text.
         */
        std::string text;
    };

    /**
     * Stream insertion operator for string text token.
     * @param os The output stream to write the token.
     * @param token The token to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, string_text_token const& token);

    /**
     * Represents data about the end of an interpolated string.
     * Used for double quoted strings and interpolated heredocs.
     */
    struct string_end_token
    {
        /**
         * Stores the beginning position for the token.
         */
        lexer::position begin;

        /**
         * Stores the ending position for the token.
         */
        lexer::position end;

        /**
         * Stores the string margin (heredoc only).
         */
        size_t margin = 0;
    };

    /**
     * Stream insertion operator for string end token.
     * @param os The output stream to write the token.
     * @param token The token to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, string_end_token const& token);

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
     * Stream insertion operator for numberic base.
     * @param os The output stream to write the token.
     * @param base The base to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, numeric_base base);

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
         * Stores the beginning position for the token.
         */
        lexer::position begin;

        /**
         * Stores the ending position for the token.
         */
        lexer::position end;

        /**
         * Stores the value of the number.
         */
        value_type value = static_cast<std::int64_t>(0);

        /**
         * Stores the numeric base.
         */
        numeric_base base = numeric_base::decimal;
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
     * Utility for converting iterator ranges into tokens.
     * @tparam Iterator The input iterator.
     */
    template <typename Token, typename Iterator>
    struct assign_to_attribute_from_iterators<Token, Iterator>
    {
        /**
         * Assigns a token based on an iterator range.
         * Custom tokens cannot be assigned from iterator range, only from values.
         * Calling this function will result in a runtime error.
         */
        static void call(Iterator const&, Iterator const&, Token&)
        {
            // This should not get called and only exists for the code to compile
            throw std::runtime_error("attempt to assign token from iterators.");
        }
    };

}}}  // namespace boost::spirit::traits
