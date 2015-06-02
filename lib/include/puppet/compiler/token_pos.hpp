#pragma once

#include "../lexer/position.hpp"
#include "../lexer/lexer.hpp"
#include <boost/spirit/include/qi_parse.hpp>

namespace puppet { namespace compiler {

    /**
     * Declare a new terminal for Spirit.
     */
    BOOST_SPIRIT_TERMINAL_EX(token_pos);

}}  // namespace puppet::compiler

namespace boost { namespace spirit {

    /**
     * Enables token_pos as a terminal in parser expressions.
     * @tparam A0 The type of the first argument to the terminal.
     */
    template <typename A0>
    struct use_terminal<qi::domain, terminal_ex<puppet::compiler::tag::token_pos, fusion::vector1<A0>>> : mpl::or_<is_integral<A0>, is_enum<A0>>
    {
    };

}}  // namespace boost::spirit

namespace puppet { namespace compiler {

    /**
     * Implements a primitive parser for token positions.
     * @tparam TokenId The type of token id.
     */
    template <typename TokenId>
    struct token_pos_parser : boost::spirit::qi::primitive_parser<token_pos_parser<TokenId>>
    {
        /**
         * Defines the attribute for the parser.
         * @tparam Context The context type.
         * @tparam Iterator The iterator type.
         */
        template <typename Context, typename Iterator>
        struct attribute
        {
            /**
             * The type of the attribute for the parser.
             */
            typedef lexer::position type;
        };

        /**
         * Constructs a token position parser with the given id.
         * @param id The token id to match on.
         */
        token_pos_parser(TokenId const& id) :
            _id(id)
        {
        }

        /**
         * Parses the input into the attribute.
         * @tparam Iterator The iterator type.
         * @tparam Context The context type.
         * @tparam Skipper The skipper type.
         * @tparam Attribute The attribute type.
         * @param first The first iterator.
         * @param last The last iterator.
         * @param skipper The skipper instance.
         * @param attr The attribute instance.
         * @return Returns true if successfully parsed or false if not.
         */
        template <typename Iterator, typename Context, typename Skipper, typename Attribute>
        bool parse(Iterator& first, Iterator const& last, Context&, Skipper const& skipper, Attribute& attr) const
        {
            boost::spirit::qi::skip_over(first, last, skipper);
            if (first != last) {
                if (static_cast<typename Iterator::value_type::id_type>(_id) == first->id()) {
                    attr = boost::apply_visitor(lexer::token_position_visitor(), first->value());
                    ++first;
                    return true;
                }
            }
            return false;
        }

        /**
         * Describes the parser.
         * @return Returns the boost::spirit::info about the parser.s
         */
        template <typename Context>
        boost::spirit::info what(Context&) const
        {
            return boost::spirit::info("token", boost::lexical_cast<std::string>(_id));
        }

     private:
        TokenId _id;
    };

}}  // namespace puppet::compiler

namespace boost { namespace spirit { namespace qi {

    /**
     * Instantiates a primitive parser.
     * @tparam Modifiers The modifiers type.
     * @tparam TokenId The type of token id.
     */
    template <typename Modifiers, typename TokenId>
    struct make_primitive<terminal_ex<puppet::compiler::tag::token_pos, fusion::vector1<TokenId>>, Modifiers>
    {
        /**
         * Represents the resulting parser type.
         */
        typedef puppet::compiler::token_pos_parser<TokenId> result_type;

        /**
         * Constructs the primitive parser.
         * @tparam Terminal The terminal type.
         * @param term The terminal.
         * @return Returns the primitive parser.
         */
        template <typename Terminal>
        result_type operator()(Terminal const& term, unused_type) const
        {
            return result_type(fusion::at_c<0>(term.args));
        }
    };

}}}  // namesoace boost::spirit::qi