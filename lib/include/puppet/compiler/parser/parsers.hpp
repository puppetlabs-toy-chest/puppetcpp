/**
 * @file
 * Declares the auxillary parsers used in the Boost Spirit rules.
 */
#pragma once

#include "../ast/ast.hpp"
#include "../lexer/lexer.hpp"
#include "../lexer/number_token.hpp"
#include "../lexer/string_token.hpp"
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
            attr = boost::apply_visitor(lexer::token_range_visitor(), iterator->value()).begin();
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
            attr = boost::apply_visitor(lexer::token_range_visitor(), iterator->value()).end();
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
         * @param remove_front True to remove the first character of the string or false if not.
         * @param remove_back True to remove the last character of the string or false if not.
         */
        explicit value_parser(bool remove_front, bool remove_back) :
            base_type(false),
            _remove_front(remove_front),
            _remove_back(remove_back)
        {
        }

     private:
        friend struct token_parser<value_parser, std::string>;

        template <typename Iterator, typename Context, typename Attribute>
        void assign(Iterator const& iterator, Context& context, Attribute& attr) const
        {
            // Get the range and copy the entire thing if not removing any characters
            auto& range = boost::get<boost::iterator_range<typename Iterator::value_type::iterator_type>>(iterator->value());
            if (!_remove_front && !_remove_back) {
                attr.assign(range.begin(), range.end());
                return;
            }

            // Move past the front
            auto start = range.begin();
            if (start != range.end()) {
                ++start;
            }

            // If not removing back, copy from the new front to the end
            if (!_remove_back) {
                attr.assign(start, range.end());
                return;
            }

            // Iterate to the input before the range's end
            auto end = start;
            for (auto current = start; current != range.end(); ++current) {
                end = current;
            }
            attr.assign(start, end);
        }

        bool _remove_front;
        bool _remove_back;
    };

    /**
     * Responsible for parsing the current token's value and trimming the first and last characters.
     */
    auto const trim_value = value_parser(true, true);

    /**
     * Responsible for parsing the current token's value and trimming the first character.
     */
    auto const ltrim_value = value_parser(true, false);

    /**
     * Responsible for parsing the current token's value and trimming the first character.
     */
    auto const rtrim_value = value_parser(false, true);

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
        void assign(Iterator const& iterator, Context& context, Attribute& attr) const
        {
            namespace x3 = boost::spirit::x3;

            auto& token = boost::get<lexer::number_token>(iterator->value());
            attr.begin = token.range().begin();
            attr.end = token.range().end();
            attr.tree = x3::get<tree_context_tag>(context);
            attr.value = token.value();
        }
    };

    /**
     * Responsible for parsing number tokens.
     */
    auto const number_token = number_parser();

    /**
     * Responsible for parsing a string token.
     */
    struct string_parser : token_parser<string_parser, ast::string>
    {
        // Use base constructors
        using base_type::base_type;

     private:
        friend struct token_parser<string_parser, ast::string>;

        template <typename Iterator, typename Context, typename Attribute>
        void assign(Iterator const& iterator, Context& context, Attribute& attr) const
        {
            namespace x3 = boost::spirit::x3;

            auto& token = boost::get<lexer::string_token<typename Iterator::value_type::iterator_type>>(iterator->value());
            auto& value = token.value();
            attr.tree = x3::get<tree_context_tag>(context);
            attr.begin = token.range().begin();
            attr.end = token.range().end();
            attr.value_range = lexer::range(value.begin().position(), value.end().position());
            attr.value = std::string(value.begin(), value.end());
            attr.escapes = token.escapes();
            attr.quote = token.quote();
            attr.interpolated = token.interpolated();
            attr.format = token.format();
            attr.margin = token.margin();
            attr.remove_break = token.remove_break();
        }
    };

    /**
     * Responsible for parsing string tokens.
     */
    auto const string_token = string_parser(lexer::token_id::single_quoted_string) |
                              string_parser(lexer::token_id::double_quoted_string) |
                              string_parser(lexer::token_id::heredoc);

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
