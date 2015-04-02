/**
 * @file
 * Declares the Puppet language parser.
 */
#pragma once

#include "grammar.hpp"
#include "../lexer/static_lexer.hpp"
#include "../lexer/lexer.hpp"
#include <boost/optional.hpp>
#include <sstream>
#include <iomanip>

namespace puppet { namespace parser {

    /**
     * Exception for parse errors.
     */
    struct parse_exception : std::runtime_error
    {
        /**
         * Constructs a parse exception.
         * @param position The token position where parsing failed.
         * @param message The exception message.
         */
        parse_exception(lexer::token_position position, std::string const& message);

        /**
         * Gets the token position where parsing failed.
         * @return Returns the token     position where parsing failed.
         */
        lexer::token_position const& position() const;

    private:
        lexer::token_position _position;
    };

    /**
     * Implements the Puppet language parser.
     */
    struct parser
    {
        /**
         * Parses the given file into an AST manifest.
         * @param input The input file to parse.
         * @param interpolation True if parsing for string interpolation or false if not.
         * @return Returns the AST manifest if parsing succeeds or nullptr if not.
         */
        static ast::manifest parse(std::ifstream& input, bool interpolation = false);
        /**
         * Parses the given string into an AST manifest.
         * @param input The input string to parse.
         * @param interpolation True if parsing for string interpolation or false if not.
         * @return Returns the AST manifest if parsing succeeds or nullptr if not.
         */
        static ast::manifest parse(std::string const& input, bool interpolation = false);

        /**
         * Parses the given iterator range into an AST manifest.
         * @param begin The beginning of the input.
         * @param end The end of the input.  If interpolating, the parsing may terminate before the end (stops at non-matching '}' token).
         * @param interpolation True if parsing for string interpolation or false if not.
         * @return Returns the AST manifest if parsing succeeds or nullptr if not.
         */
        static ast::manifest parse(lexer::lexer_string_iterator& begin, lexer::lexer_string_iterator const& end, bool interpolation = false);

     private:
        struct expectation_info_printer
        {
            expectation_info_printer(std::ostream& os);
            void element(boost::spirit::utf8_string const& tag, boost::spirit::utf8_string const& value, int depth);

         private:
            std::ostream& _os;
            bool _next;
            std::stack<int> _depths;
        };

        template <typename Iterator>
        static std::string to_string(boost::spirit::qi::expectation_failure<Iterator> const& ex)
        {
            using namespace std;
            using namespace puppet::lexer;
            using namespace boost::spirit;

            // Expectation failure
            ostringstream ss;
            ss << "expected ";

            expectation_info_printer printer(ss);
            boost::apply_visitor(basic_info_walker<expectation_info_printer>(printer, ex.what_.tag, 0), ex.what_.value);

            ss << " but found " << static_cast<token_id>(ex.first->id()) << ".";
            return ss.str();
        }

        template <typename Lexer, typename Input, typename Iterator>
        static ast::manifest parse(Lexer& lexer, Input& input, Iterator& begin, Iterator const& end, bool interpolation)
        {
            using namespace std;
            using namespace puppet::lexer;
            namespace qi = boost::spirit::qi;

            try {
                // Construct a lexer of the given type with a callback that reports warnings
                Lexer lexer;
                auto token_begin = lexer.begin(begin, end);
                auto token_end = lexer.end();

                // Parse the input into an AST manifest
                ast::manifest manifest;
                if (qi::parse(token_begin, token_end, grammar<Lexer>(lexer, interpolation), manifest) &&
                    (token_begin == token_end || token_begin->id() == boost::lexer::npos || interpolation)) {
                    return manifest;
                }

                // If not all tokens were processed and the iterator points at a valid token, handle unexpected token
                if (token_begin != token_end && token_is_valid(*token_begin)) {
                    throw parse_exception(get_position(input, *token_begin), (boost::format("unexpected %1%.") % static_cast<token_id>(token_begin->id())).str());
                }
            } catch (lexer_exception<Iterator> const& ex) {
                throw parse_exception(ex.location().position(), ex.what());
            } catch (qi::expectation_failure<typename Lexer::iterator_type> const& ex) {
                throw parse_exception(get_position(input, *ex.first), to_string(ex));
            }

            // Unexpected character in the input
            ostringstream message;
            if (begin != end) {
                message << "unexpected character ";
                if (isprint(*begin)) {
                    message << '\'' << *begin << '\'';
                } else {
                    message << "0x" << setw(2) << setfill('0') << static_cast<int>(*begin);
                }
                message << ".";
            } else {
                message << "unexpected end of input.";
            }
            throw parse_exception(begin.position(), message.str());
        }
    };

}}  // namespace puppet::parser