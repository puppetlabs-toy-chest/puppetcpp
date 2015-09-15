/**
 * @file
 * Declares the Puppet lexer token id.
 */
#pragma once

#include <boost/spirit/include/lex_lexer.hpp>
#include <boost/spirit/include/lex_lexertl.hpp>
#include <boost/spirit/include/lex_lexertl_token.hpp>

namespace puppet { namespace lexer {

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
        last_keyword,       // Add new keywords before this id
        statement_call,
        single_quoted_string,
        double_quoted_string,
        bare_word,
        variable,
        type,
        name,
        regex,
        heredoc,
        number,
        array_start,          // Same as '[', but whitespace delimited to force array expression
        comment,              // Not in token stream
        whitespace,           // Not in token stream
        unclosed_quote,       // Error token that will not match any grammars
        unclosed_comment,     // Error token that will not match any grammars
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

}}  // namespace puppet::lexer

namespace boost { namespace spirit { namespace qi {

    /** @cond NOT_DOCUMENTED */
    /**
     * Specialization for information for tokens with id type token_id.
     * @tparam Context The type of context.
     * @return Returns The boost info for the token.
     */
    template <>
    template <typename Context>
    info plain_token<puppet::lexer::token_id>::what(Context&) const
    {
        return boost::spirit::info("token", boost::lexical_cast<std::string>(id));
    }

    /**
     * Specialization for information for tokens with id type char.
     * @tparam Context The type of context.
     * @return Returns The boost info for the token.
     */
    template <>
    template <typename Context>
    info plain_token<char>::what(Context&) const
    {
        return boost::spirit::info("token", boost::lexical_cast<std::string>(static_cast<puppet::lexer::token_id>(id)));
    }
    /**
     * Specialization for information for tokens with id type token_id.
     * @tparam Context The type of context.
     * @return Returns The boost info for the token.
     */
    template <>
    template <typename Context>
    info plain_raw_token<puppet::lexer::token_id>::what(Context&) const
    {
        return boost::spirit::info("raw_token", boost::lexical_cast<std::string>(id));
    }

    /**
     * Specialization for information for tokens with id type char.
     * @tparam Context The type of context.
     * @return Returns The boost info for the token.
     */
    template <>
    template <typename Context>
    info plain_raw_token<char>::what(Context&) const
    {
        return boost::spirit::info("raw_token", boost::lexical_cast<std::string>(static_cast<puppet::lexer::token_id>(id)));
    }
    /** @endcond */

}}}  // namespace boost::spirit::qi
