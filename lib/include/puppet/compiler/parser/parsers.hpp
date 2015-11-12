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

namespace puppet { namespace compiler { namespace parser {

    /**
     * Tag used for associating a syntax tree with a parser.
     */
    struct tree_context_tag;

    /**
     * Responsible for parsing AST context.
     */
    struct context : boost::spirit::x3::parser<context>
    {
        /**
         * The resulting attribute type.
         */
        using attribute_type = ast::context;

        /**
         * Constructs a context parser that will match any token.
         * @param consume True to consume the token or false if not.
         */
        explicit context(bool consume = true) :
            context(static_cast<lexer::token_id>(0), consume)
        {
        }

        /**
         * Constructs a context parser that will match the given character token.
         * @param id The token identifier, as a character.
         * @param consume True to consume the token or false if not.
         */
        explicit context(char id, bool consume = true) :
            context(static_cast<lexer::token_id>(id), consume)
        {
        }

        /**
         * Constructs a context parser that will match the given token identifier.
         * @param id The token identifier.
         * @param consume True to consume the token or false if not.
         */
        explicit context(lexer::token_id id, bool consume = true) :
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
         * @param first The first iterator.
         * @param last The last iterator.
         * @param context The parse context.
         * @param attr The output attribute resulting from the parse.
         * @return Returns true if the parse is successful or false if it is not.
         */
        template <typename Iterator, typename Context>
        bool parse(Iterator& first, Iterator const& last, Context const& context, boost::spirit::x3::unused_type, attribute_type& attr) const
        {
            namespace x3 = boost::spirit::x3;

            x3::skip_over(first, last, context);
            if (first != last && (static_cast<size_t>(_id) == 0 || static_cast<lexer::token_id>(first->id()) == _id)) {
                attr.tree = x3::get<tree_context_tag>(context);
                attr.position = boost::apply_visitor(lexer::token_position_visitor(), first->value());
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
     * Represents a parser that returns the current context without consuming a token.
     */
    auto const current_context = context(false);

    /**
     * Responsible for parsing lexer positions (used in string interpolation).
     */
    struct position : boost::spirit::x3::parser<position>
    {
        /**
         * The resulting attribute type.
         */
        using attribute_type = lexer::position;

        /**
         * Constructs a position parser that will match the given character token.
         * @param id The token identifier, as a character.
         */
        explicit position(char id) :
            position(static_cast<lexer::token_id>(id))
        {
        }

        /**
         * Constructs a position parser that will match the given token identifier.
         * @param id The token identifier.
         */
        explicit position(lexer::token_id id) :
            _id(id)
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
         * @param first The first iterator.
         * @param last The last iterator.
         * @param context The parse context.
         * @param attr The output attribute resulting from the parse.
         * @return Returns true if the parse is successful or false if it is not.
         */
        template <typename Iterator, typename Context>
        bool parse(Iterator& first, Iterator const& last, Context const& context, boost::spirit::x3::unused_type, attribute_type& attr) const
        {
            namespace x3 = boost::spirit::x3;

            x3::skip_over(first, last, context);
            if (first != last && (static_cast<size_t>(_id) == 0 || static_cast<lexer::token_id>(first->id()) == _id)) {
                attr = boost::apply_visitor(lexer::token_position_visitor(), first->value());
                ++first;
                return true;
            }
            return false;
        }

     private:
        lexer::token_id _id;
    };

    /**
     * Responsible for parsing a token without ouputting an attribute.
     */
    struct raw_token : boost::spirit::x3::parser<raw_token>
    {
        /**
         * The resulting attribute type.
         */
        using attribute_type = boost::spirit::x3::unused_type;

        /**
         * Constructs a raw token parser that will match the given token identifier.
         * @param id The token identifier to match, as a character.
         */
        explicit raw_token(char id) :
            raw_token(static_cast<lexer::token_id>(id))
        {
        }

         /**
         * Constructs a raw token parser that will match the given token identifier.
         * @param id The token identifier to match.
         */
        explicit raw_token(lexer::token_id id) :
            _id(id)
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
         * @param first The first iterator.
         * @param last The last iterator.
         * @param context The parse context.
         * @return Returns true if the parse is successful or false if it is not.
         */
        template <typename Iterator, typename Context>
        bool parse(Iterator& first, Iterator const& last, Context const& context, boost::spirit::x3::unused_type, boost::spirit::x3::unused_type) const
        {
            namespace x3 = boost::spirit::x3;

            x3::skip_over(first, last, context);
            if (first != last && static_cast<lexer::token_id>(first->id()) == _id) {
                ++first;
                return true;
            }
            return false;
        }

     private:
        lexer::token_id _id;
    };

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
         * Constructs a token parser that will match the given token identifier.
         * @param id The token identifier to match, as a character.
         */
        explicit token_parser(char id) :
            token_parser(static_cast<lexer::token_id>(id))
        {
        }

        /**
         * Constructs a token parser that will match the given token identifier.
         * @param id The token identifier to match.
         */
        explicit token_parser(lexer::token_id id) :
            _id(id)
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
         * @param first The first iterator.
         * @param last The last iterator.
         * @param context The parse context.
         * @param attr The output attribute resulting from the parse.
         * @return Returns true if the parse is successful or false if it is not.
         */
        template <typename Iterator, typename Context>
        bool parse(Iterator& first, Iterator const& last, Context const& context, boost::spirit::x3::unused_type, attribute_type& attr) const
        {
            namespace x3 = boost::spirit::x3;

            x3::skip_over(first, last, context);
            if (first != last && static_cast<lexer::token_id>(first->id()) == _id) {
                static_cast<Parser const*>(this)->assign(first, context, attr);
                ++first;
                return true;
            }
            return false;
        }

     private:
        lexer::token_id _id;
    };

    /**
     * Responsible for parsing a token and outputting a string.
     */
    struct token : token_parser<token, std::string>
    {
        /**
         * Constructs a token parser that will match the given token identifier.
         * @param id The token identifier to match, as a character.
         * @param remove_front True to remove the first character of the string or false if not.
         * @param remove_back True to remove the last character of the string or false if not.
         */
        explicit token(char id, bool remove_front = false, bool remove_back = false) :
            token(static_cast<lexer::token_id>(id), remove_front, remove_back)
        {
        }

        /**
         * Constructs a token parser that will match the given token identifier.
         * @param id The token identifier to match.
         * @param remove_front True to remove the first character of the string or false if not.
         * @param remove_back True to remove the last character of the string or false if not.
         */
        explicit token(lexer::token_id id, bool remove_front = false, bool remove_back = false) :
            base_type(id),
            _remove_front(remove_front),
            _remove_back(remove_back)
        {
        }

        /**
         * Assigns an attribute from a matched input.
         * @tparam Iterator The iterator type.
         * @tparam Context The context type.
         * @param iterator The iterator that matched.
         * @param context The parser context.
         * @param attr The output attribute.
         */
        template <typename Iterator, typename Context>
        void assign(Iterator const& iterator, Context& context, attribute_type& attr) const
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

     private:
        bool _remove_front;
        bool _remove_back;
    };

    /**
     * Responsible for parsing a number token.
     */
    struct number_token : token_parser<number_token, ast::number>
    {
        /**
         * Default constructor for number token.
         */
        number_token() :
            base_type(lexer::token_id::number)
        {
        }

        /**
         * Assigns an attribute from a matched input.
         * @tparam Iterator The iterator type.
         * @tparam Context The context type.
         * @param iterator The iterator that matched.
         * @param context The parser context.
         * @param attr The output attribute.
         */
        template <typename Iterator, typename Context>
        void assign(Iterator const& iterator, Context& context, attribute_type& attr) const
        {
            namespace x3 = boost::spirit::x3;

            attr.context.tree = x3::get<tree_context_tag>(context);
            auto& number_token = boost::get<lexer::number_token>(iterator->value());
            attr.context.position = number_token.position();
            attr.value = number_token.value();
        }
    };

    /**
     * Responsible for parsing a string token.
     */
    struct string_token : token_parser<string_token, ast::string>
    {
        // Use base constructors
        using base_type::base_type;

        /**
         * Assigns an attribute from a matched input.
         * @tparam Iterator The iterator type.
         * @tparam Context The context type.
         * @param iterator The iterator that matched.
         * @param context The parser context.
         * @param attr The output attribute.
         */
        template <typename Iterator, typename Context>
        void assign(Iterator const& iterator, Context& context, attribute_type& attr) const
        {
            namespace x3 = boost::spirit::x3;

            attr.context.tree = x3::get<tree_context_tag>(context);
            auto& string_token = boost::get<lexer::string_token<typename Iterator::value_type::iterator_type>>(iterator->value());
            attr.context.position = string_token.position();
            attr.value.assign(string_token.begin(), string_token.end());
            attr.escapes = string_token.escapes();
            attr.quote = string_token.quote();
            attr.interpolated = string_token.interpolated();
            attr.format = string_token.format();
            attr.margin = string_token.margin();
            attr.remove_break = string_token.remove_break();
        }
    };

}}} // namespace puppet::compiler::parser

namespace boost { namespace spirit { namespace x3 {

    /**
     * Responsible for getting the info of a context parser.
     */
    template <>
    struct get_info<puppet::compiler::parser::context>
    {
        /**
         * The parser type.
         */
        using parser_type = puppet::compiler::parser::context;
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
     * Responsible for getting the info of a raw token parser.
     */
    template <>
    struct get_info<puppet::compiler::parser::raw_token>
    {
        /**
         * The parser type.
         */
        using parser_type = puppet::compiler::parser::raw_token;
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
     * Responsible for getting the info of a token parser.
     */
    template <>
    struct get_info<puppet::compiler::parser::token>
    {
        /**
         * The parser type.
         */
        using parser_type = puppet::compiler::parser::token;
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
     * Responsible for getting the info of a number token parser.
     */
    template <>
    struct get_info<puppet::compiler::parser::number_token>
    {
        /**
         * The parser type.
         */
        using parser_type = puppet::compiler::parser::number_token;
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
     * Responsible for getting the info of a string token parser.
     */
    template <>
    struct get_info<puppet::compiler::parser::string_token>
    {
        /**
         * The parser type.
         */
        using parser_type = puppet::compiler::parser::string_token;
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
