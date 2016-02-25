/**
 * @file
 * Declares the auxillary parsers used in the Boost Spirit rules.
 */
#pragma once

#include "../ast/ast.hpp"
#include "../lexer/tokens.hpp"
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/nonterminal/simple_trace.hpp>
#include <tuple>

namespace puppet { namespace compiler { namespace parser {

    /**
     * Tag used for associating a syntax tree with a parser.
     */
    struct tree_context_tag;

    /**
     * Base type responsible for parsing tokens.
     * @tparam Parser The derived parser type.
     * @tparam Attribute The output attribute type.
     */
    template <typename Parser, typename Attribute>
    struct token_parser : boost::spirit::x3::parser<token_parser<Parser, Attribute>>
    {
        /**
         * The parser base type.
         */
        using base_type = token_parser<Parser, Attribute>;

        /**
         * The parser attribute type.
         */
        using attribute_type = Attribute;

        /**
         * Constructs a token parser that will match any token.
         * @param consume True to consume the token or false if not.
         */
        explicit token_parser(bool consume = true) :
            token_parser(static_cast<lexer::token_id>(0), consume)
        {
        }

        /**
         * Constructs a token parser that will match the given token identifier.
         * @param id The token identifier to match, as a character.
         * @param consume True to consume the token or false if not.
         */
        explicit token_parser(char id, bool consume = true) :
            token_parser(static_cast<lexer::token_id>(id), consume)
        {
        }

        /**
         * Constructs a token parser that will match the given token identifier.
         * @param id The token identifier to match.
         * @param consume True to consume the token or false if not.
         */
        explicit token_parser(lexer::token_id id, bool consume = true) :
            _id(id),
            _consume(consume)
        {
        }

        /**
         * Gets the token identifier for the parser.
         * @return Returns the token identifier for the parser.
         */
        lexer::token_id id() const
        {
            return _id;
        }

        /**
         * Parses the input.
         * @tparam Iterator The iterator type.
         * @tparam Context The context type.
         * @tparam Parsed The parsed attribute type.
         * @param first The first iterator.
         * @param last The last iterator.
         * @param context The parse context.
         * @param attr The output attribute resulting from the parse.
         * @return Returns true if the parse is successful or false if it is not.
         */
        template <typename Iterator, typename Context, typename Parsed>
        bool parse(Iterator& first, Iterator const& last, Context const& context, boost::spirit::x3::unused_type, Parsed& attr) const
        {
            namespace x3 = boost::spirit::x3;

            x3::skip_over(first, last, context);
            if (static_cast<size_t>(_id) == 0 || (first != last && static_cast<lexer::token_id>(first->id()) == _id)) {
                static_cast<Parser const*>(this)->assign(first, context, attr);
                if (_consume) {
                    ++first;
                }
                return true;
            }
            return false;
        }

     private:
        lexer::token_id _id;
        bool _consume;
    };

    /**
     * Responsible for parsing the beginning position of a token.
     * Does not consume the token by default.
     */
    struct begin_parser : token_parser<begin_parser, lexer::position>
    {
        // Use the base constructors.
        using base_type::base_type;

     private:
        friend struct token_parser<begin_parser, lexer::position>;

        template <typename Iterator, typename Context, typename Attribute>
        void assign(Iterator const& iterator, Context& context, Attribute& attr) const
        {
            attr = boost::apply_visitor(lexer::token_range_visitor(), iterator->value()).first;
        }
    };

    /**
     * Alias for begin parser.
     */
    using begin = begin_parser;

    /**
     * Responsible for parsing the ending position (not inclusive) of a token.
     */
    struct end_parser : token_parser<end_parser, lexer::position>
    {
        // Use the base constructors.
        using base_type::base_type;

     private:
        friend struct token_parser<end_parser, lexer::position>;

        template <typename Iterator, typename Context, typename Attribute>
        void assign(Iterator const& iterator, Context& context, Attribute& attr) const
        {
            attr = boost::apply_visitor(lexer::token_range_visitor(), iterator->value()).second;
        }
    };

    /**
     * Alias for end parser.
     */
    using end = end_parser;

    /**
     * Responsible for extracting the base syntax tree pointer from the parse context.
     * This parser never consumes a token.
     */
    struct tree_parser : boost::spirit::x3::parser<tree_parser>
    {
        /**
         * The parser attribute type.
         */
        using attribute_type = ast::syntax_tree*;

        /**
         * Parses the input.
         * @tparam Iterator The iterator type.
         * @tparam Context The context type.
         * @tparam Parsed The parsed attribute type.
         * @param first The first iterator.
         * @param last The last iterator.
         * @param context The parse context.
         * @param attr The output attribute resulting from the parse.
         * @return Returns true if the parse is successful or false if it is not.
         */
        template <typename Iterator, typename Context, typename Parsed>
        bool parse(Iterator& first, Iterator const& last, Context const& context, boost::spirit::x3::unused_type, Parsed& attr) const
        {
            attr = boost::spirit::x3::get<tree_context_tag>(context);
            return true;
        }
    };

    /**
     * Represents a parser that returns the base syntax tree from the parser context.
     */
    auto const tree = tree_parser();

    /**
     * Responsible for consuming a token without outputting an attribute.
     */
    struct raw_parser : token_parser<raw_parser, boost::spirit::x3::unused_type>
    {
        // Use the base constructors.
        using base_type::base_type;

     private:
        friend struct token_parser<raw_parser, boost::spirit::x3::unused_type>;

        template <typename Iterator, typename Context, typename Attribute>
        void assign(Iterator const& iterator, Context& context, Attribute& attr) const
        {
            // Do nothing
        }
    };

    /**
     * Alias for raw parser.
     */
    using raw = raw_parser;

    /**
     * Responsible for parsing the current token's value.
     * This parser does not consume the token.
     */
    struct value_parser : token_parser<value_parser, std::string>
    {
        /**
         * Constructs a value parser that will match the given token identifier.
         * @param front The character to trim from the front.
         * @param back The character to trim from the front.
         */
        explicit value_parser(char front = 0, char back = 0) :
            base_type(false),
            _front(front),
            _back(back)
        {
        }

     private:
        friend struct token_parser<value_parser, std::string>;

        template <typename Iterator, typename Context, typename Attribute>
        void assign(Iterator const& iterator, Context& context, Attribute& attr) const
        {
            // Get the range and copy the entire thing if not removing any characters
            auto& range = boost::get<boost::iterator_range<typename Iterator::value_type::iterator_type>>(iterator->value());

            // Move past the front character if it matches
            auto start = range.begin();
            if (_front && start != range.end() && *start == _front) {
                ++start;
            }

            // Assign the string
            attr.assign(start, range.end());

            // Pop the back if it matches
            if (_back && !attr.empty() && attr.back() == _back) {
                attr.pop_back();
            }
        }

        char _front;
        char _back;
    };

    /**
     * Responsible for parsing the regex values.
     */
    auto const regex_value = value_parser('/', '/');

    /**
     * Responsible for parsing variable values.
     */
    auto const variable_value = value_parser('$');

    /**
     * Responsible for parsing the current token's value.
     */
    auto const value = value_parser(false, false);

    /**
     * Responsible for parsing a number token.
     */
    struct number_parser : token_parser<number_parser, ast::number>
    {
        /**
         * Default constructor for number parser.
         */
        number_parser() :
            base_type(lexer::token_id::number)
        {
        }

     private:
        friend struct token_parser<number_parser, ast::number>;

        template <typename Iterator, typename Context, typename Attribute>
        void assign(Iterator& iterator, Context& context, Attribute& attr) const
        {
            namespace x3 = boost::spirit::x3;

            auto& token = boost::get<lexer::number_token>(iterator->value());
            attr.begin = token.begin;
            attr.end = token.end;
            attr.tree = x3::get<tree_context_tag>(context);
            attr.base = token.base;
            attr.value = token.value;
        }
    };

    /**
     * Responsible for parsing number tokens.
     */
    auto const number_token = number_parser();

    /**
     * Responsible for parsing literal strings.
     */
    struct string_parser : boost::spirit::x3::parser<string_parser>
    {
        /**
         * The parser attribute type.
         */
        using attribute_type = ast::string;

        /**
         * Parses the input.
         * @tparam Iterator The iterator type.
         * @tparam Context The context type.
         * @tparam Parsed The parsed attribute type.
         * @param first The first iterator.
         * @param last The last iterator.
         * @param context The parse context.
         * @param attr The output attribute resulting from the parse.
         * @return Returns true if the parse is successful or false if it is not.
         */
        template <typename Iterator, typename Context, typename Parsed>
        bool parse(Iterator& first, Iterator const& last, Context const& context, boost::spirit::x3::unused_type, Parsed& attr) const
        {
            namespace x3 = boost::spirit::x3;

            x3::skip_over(first, last, context);
            if (first == last) {
                return false;
            }
            // Check for simple string token
            if (static_cast<lexer::token_id>(first->id()) == lexer::token_id::string) {
                auto& token = boost::get<lexer::string_token>(first->value());
                attr.begin = token.begin;
                attr.end = token.end;
                attr.tree = x3::get<tree_context_tag>(context);
                attr.format = token.format;
                attr.value = token.value;
                attr.margin = token.margin;
                ++first;
                return true;
            }
            // Check for interpolating strings containing no interpolations (only one literal text token)
            if (static_cast<lexer::token_id>(first->id()) == lexer::token_id::string_start) {
                auto current = first;
                auto start_token = current;
                ++current;
                if (current == last || static_cast<lexer::token_id>(current->id()) != lexer::token_id::string_text) {
                    return false;
                }
                auto text_token = current;
                ++current;
                if (current == last || static_cast<lexer::token_id>(current->id()) != lexer::token_id::string_end) {
                    return false;
                }
                auto end_token = current;
                ++current;
                {
                    auto& token = boost::get<lexer::string_start_token>(start_token->value());
                    attr.begin = token.begin;
                    attr.tree = x3::get<tree_context_tag>(context);
                    attr.format = token.format;
                }
                {
                    auto& token = boost::get<lexer::string_text_token>(text_token->value());
                    attr.value = token.text;
                }
                {
                    auto& token = boost::get<lexer::string_end_token>(end_token->value());
                    attr.end = token.end;
                    attr.margin = token.margin;
                }
                first = current;
                return true;
            }
            return false;
        }
    };

    /**
     * Responsible for parsing string tokens.
     */
    auto const string_token = string_parser();

    /**
     * Responsible for parsing string format information from string start tokens.
     */
    struct string_format_parser : token_parser<string_format_parser, std::string>
    {
        /**
         * Default constructor for string format parser.
         */
        string_format_parser() :
            base_type(lexer::token_id::string_start)
        {
        }

     private:
        friend struct token_parser<string_format_parser, std::string>;

        template <typename Iterator, typename Context, typename Attribute>
        void assign(Iterator const& iterator, Context& context, Attribute& attr) const
        {
            namespace x3 = boost::spirit::x3;

            auto& token = boost::get<lexer::string_start_token>(iterator->value());
            attr = token.format;
        }
    };

    /**
     * Responsible for parsing string format information from string start tokens.
     */
    auto const string_format = string_format_parser();

    /**
     * Responsible for parsing interpolated string text.
     */
    struct string_text_parser : token_parser<string_text_parser, ast::literal_string_text>
    {
        /**
         * Default constructor for string text parser.
         */
        string_text_parser() :
            base_type(lexer::token_id::string_text)
        {
        }

     private:
        friend struct token_parser<string_text_parser, ast::literal_string_text>;

        template <typename Iterator, typename Context, typename Attribute>
        void assign(Iterator const& iterator, Context& context, Attribute& attr) const
        {
            namespace x3 = boost::spirit::x3;

            auto& token = boost::get<lexer::string_text_token>(iterator->value());
            attr.begin = token.begin;
            attr.end = token.end;
            attr.tree = x3::get<tree_context_tag>(context);
            attr.text = token.text;
        }
    };

    /**
     * Responsible for parsing string text tokens.
     */
    auto const string_text = string_text_parser();

    /**
     * Responsible for parsing string margin information from string end tokens.
     */
    struct string_margin_parser : token_parser<string_margin_parser, size_t>
    {
        /**
         * Default constructor for string margin parser.
         */
        string_margin_parser() :
            base_type(lexer::token_id::string_end, false)
        {
        }

     private:
        friend struct token_parser<string_margin_parser, size_t>;

        template <typename Iterator, typename Context, typename Attribute>
        void assign(Iterator const& iterator, Context& context, Attribute& attr) const
        {
            namespace x3 = boost::spirit::x3;

            auto& token = boost::get<lexer::string_end_token>(iterator->value());
            attr = token.margin;
        }
    };

    /**
     * Responsible for parsing string margin information from string end tokens.
     */
    auto const string_margin = string_margin_parser();

}}} // namespace puppet::compiler::parser

namespace boost { namespace spirit { namespace x3 {

    /**
     * Responsible for getting the info of a begin parser.
     */
    template <>
    struct get_info<puppet::compiler::parser::begin_parser>
    {
        /**
         * The parser type.
         */
        using parser_type = puppet::compiler::parser::begin_parser;
        /**
         * The result type.
         */
        using result_type = std::string;

        /**
         * Gets the info for a parser.
         * @param parser The parser to get the info for.
         * @return Returns the info for the parser.
         */
        result_type operator()(parser_type const& parser) const
        {
            return boost::lexical_cast<result_type>(parser.id());
        }
    };

    /**
     * Responsible for getting the info of an end parser.
     */
    template <>
    struct get_info<puppet::compiler::parser::end_parser>
    {
        /**
         * The parser type.
         */
        using parser_type = puppet::compiler::parser::end_parser;
        /**
         * The result type.
         */
        using result_type = std::string;

        /**
         * Gets the info for a parser.
         * @param parser The parser to get the info for.
         * @return Returns the info for the parser.
         */
        result_type operator()(parser_type const& parser) const
        {
            return boost::lexical_cast<result_type>(parser.id());
        }
    };

    /**
     * Responsible for getting the info of a tree parser.
     */
    template <>
    struct get_info<puppet::compiler::parser::tree_parser>
    {
        /**
         * The parser type.
         */
        using parser_type = puppet::compiler::parser::tree_parser;
        /**
         * The result type.
         */
        using result_type = std::string;

        /**
         * Gets the info for a parser.
         * @param parser The parser to get the info for.
         * @return Returns the info for the parser.
         */
        result_type operator()(parser_type const& parser) const
        {
            return "tree";
        }
    };

    /**
     * Responsible for getting the info of a raw parser.
     */
    template <>
    struct get_info<puppet::compiler::parser::raw_parser>
    {
        /**
         * The parser type.
         */
        using parser_type = puppet::compiler::parser::raw_parser;
        /**
         * The result type.
         */
        using result_type = std::string;

        /**
         * Gets the info for a parser.
         * @param parser The parser to get the info for.
         * @return Returns the info for the parser.
         */
        result_type operator()(parser_type const& parser) const
        {
            return boost::lexical_cast<result_type>(parser.id());
        }
    };

    /**
     * Responsible for getting the info of a value parser.
     */
    template <>
    struct get_info<puppet::compiler::parser::value_parser>
    {
        /**
         * The parser type.
         */
        using parser_type = puppet::compiler::parser::value_parser;
        /**
         * The result type.
         */
        using result_type = std::string;

        /**
         * Gets the info for a parser.
         * @param parser The parser to get the info for.
         * @return Returns the info for the parser.
         */
        result_type operator()(parser_type const& parser) const
        {
            return boost::lexical_cast<result_type>(parser.id());
        }
    };

    /**
     * Responsible for getting the info of a number parser.
     */
    template <>
    struct get_info<puppet::compiler::parser::number_parser>
    {
        /**
         * The parser type.
         */
        using parser_type = puppet::compiler::parser::number_parser;
        /**
         * The result type.
         */
        using result_type = std::string;

        /**
         * Gets the info for a parser.
         * @param parser The parser to get the info for.
         * @return Returns the info for the parser.
         */
        result_type operator()(parser_type const& parser) const
        {
            return "number";
        }
    };

    /**
     * Responsible for getting the info of a string parser.
     */
    template <>
    struct get_info<puppet::compiler::parser::string_parser>
    {
        /**
         * The parser type.
         */
        using parser_type = puppet::compiler::parser::string_parser;
        /**
         * The result type.
         */
        using result_type = std::string;

        /**
         * Gets the info for a parser.
         * @param parser The parser to get the info for.
         * @return Returns the info for the parser.
         */
        result_type operator()(parser_type const& parser) const
        {
            return "string";
        }
    };

    /**
     * Responsible for getting the info of an alternative parser.
     * @tparam Left The left type of the binary parser.
     * @tparam Right The right type of the binary parser.
     */
    template <typename Left, typename Right>
    struct get_info<boost::spirit::x3::alternative<Left, Right>>
    {
        /**
         * The parser type.
         */
        using parser_type = boost::spirit::x3::alternative<Left, Right>;
        /**
         * The result type.
         */
        using result_type = std::string;

        /**
         * Gets the info for a parser.
         * @param parser The parser to get the info for.
         * @return Returns the info for the parser.
         */
        result_type operator()(parser_type const& parser) const
        {
            std::ostringstream ss;
            ss << get_info<Left>()(parser.left)
               << " or "
               << get_info<Right>()(parser.right);
            return ss.str();
        }
    };

    /**
     * Responsible for getting the info of a sequence parser.
     * @tparam Left The left type of the binary parser.
     * @tparam Right The right type of the binary parser.
     */
    template <typename Left, typename Right>
    struct get_info<boost::spirit::x3::sequence<Left, Right>>
    {
        /**
         * The parser type.
         */
        using parser_type = boost::spirit::x3::sequence<Left, Right>;
        /**
         * The result type.
         */
        using result_type = std::string;

        /**
         * Gets the info for a parser.
         * @param parser The parser to get the info for.
         * @return Returns the info for the parser.
         */
        result_type operator()(parser_type const& parser) const
        {
            std::ostringstream ss;
            ss << get_info<Left>()(parser.left)
               << " followed by "
               << get_info<Right>()(parser.right);
            return ss.str();
        }
    };

    /**
     * Responsible for getting the info of a list parser.
     * @tparam Left The left type of the binary parser.
     * @tparam Right The right type of the binary parser.
     */
    template <typename Left, typename Right>
    struct get_info<boost::spirit::x3::list<Left, Right>>
    {
        /**
         * The parser type.
         */
        using parser_type = boost::spirit::x3::list<Left, Right>;
        /**
         * The result type.
         */
        using result_type = std::string;

        /**
         * Gets the info for a parser.
         * @param parser The parser to get the info for.
         * @return Returns the info for the parser.
         */
        result_type operator()(parser_type const& parser) const
        {
            std::ostringstream ss;
            ss << "list of " << get_info<Left>()(parser.left)
               << " separated by "
               << get_info<Right>()(parser.right);
            return ss.str();
        }
    };

    /**
     * Responsible for getting the info of an expectation directive.
     * @tparam Subject The parser's subject type.
     */
    template <typename Subject>
    struct get_info<boost::spirit::x3::expect_directive<Subject>>
    {
        /**
         * The parser type.
         */
        using parser_type = boost::spirit::x3::expect_directive<Subject>;
        /**
         * The result type.
         */
        using result_type = std::string;

        /**
         * Gets the info for a parser.
         * @param parser The parser to get the info for.
         * @return Returns the info for the parser.
         */
        result_type operator()(parser_type const& parser) const
        {
            return get_info<Subject>()(parser.subject);
        }
    };

    /**
     * Responsible for getting the info of an optional parser.
     * @tparam Subject The parser's subject type.
     */
    template <typename Subject>
    struct get_info<boost::spirit::x3::optional<Subject>>
    {
        /**
         * The parser type.
         */
        using parser_type = boost::spirit::x3::optional<Subject>;
        /**
         * The result type.
         */
        using result_type = std::string;

        /**
         * Gets the info for a parser.
         * @param parser The parser to get the info for.
         * @return Returns the info for the parser.
         */
        result_type operator()(parser_type const& parser) const
        {
            std::ostringstream ss;
            ss << "an optional " << get_info<Subject>()(parser.subject);
            return ss.str();
        }
    };

}}}  // namespace boost::spirit::x3
