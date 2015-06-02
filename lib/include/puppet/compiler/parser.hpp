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
#include <memory>

namespace puppet { namespace compiler {

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
        parse_exception(lexer::position position, std::string const& message);

        /**
         * Gets the token position where parsing failed.
         * @return Returns the token position where parsing failed.
         */
        lexer::position const& position() const;

     private:
        lexer::position _position;
    };

    /**
     * Implements the Puppet language parser.
     */
    struct parser
    {
        /**
         * Parses the given file into a syntax tree.
         * @param path The path to the file to parse.
         * @param input The input file to parse.
         * @param interpolation True if parsing for string interpolation or false if not.
         * @return Returns the syntax tree if parsing succeeds throws parse_exception if not.
         */
        static std::shared_ptr<ast::syntax_tree> parse(std::string const& path, std::ifstream& input, bool interpolation = false);
        /**
         * Parses the given string into a syntax tree.
         * @param input The input string to parse.
         * @param interpolation True if parsing for string interpolation or false if not.
         * @return Returns the syntax tree if parsing succeeds throws parse_exception if not.
         */
        static std::shared_ptr<ast::syntax_tree> parse(std::string const& input, bool interpolation = false);

        /**
         * Parses the given iterator range into a syntax tree.
         * @param begin The beginning of the input.
         * @param end The end of the input.  If interpolating, the parsing may terminate before the end (stops at non-matching '}' token).
         * @param interpolation True if parsing for string interpolation or false if not.
         * @return Returns the syntax tree if parsing succeeds throws parse_exception if not.
         */
        static std::shared_ptr<ast::syntax_tree> parse(lexer::lexer_string_iterator& begin, lexer::lexer_string_iterator const& end, bool interpolation = false);

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
        static std::string to_string(boost::spirit::qi::expectation_failure<Iterator> const& ex, bool interpolation)
        {
            using namespace std;
            using namespace puppet::lexer;
            using namespace boost::spirit;

            // Expectation failure
            ostringstream ss;
            ss << "expected ";

            expectation_info_printer printer(ss);
            boost::apply_visitor(basic_info_walker<expectation_info_printer>(printer, ex.what_.tag, 0), ex.what_.value);

            ss << " but found " << static_cast<token_id>(ex.first->id());

            if (interpolation) {
                ss << " during string interpolation";
            }
            ss << ".";
            return ss.str();
        }

        template <typename Lexer, typename Input, typename Iterator>
        static std::shared_ptr<ast::syntax_tree> parse(Lexer& lexer, std::string const& path, Input& input, Iterator& begin, Iterator const& end, bool interpolation)
        {
            using namespace std;
            using namespace puppet::lexer;
            namespace qi = boost::spirit::qi;

            try {
                // Construct a lexer of the given type with a callback that reports warnings
                Lexer lexer;
                auto token_begin = lexer.begin(begin, end);
                auto token_end = lexer.end();

                // Parse the input into a syntax tree
                auto tree = make_shared<ast::syntax_tree>();
                if (qi::parse(token_begin, token_end, grammar<Lexer>(lexer, path, interpolation), *tree) &&
                    (token_begin == token_end || token_begin->id() == boost::lexer::npos || interpolation)) {
                    return tree;
                }

                // If not all tokens were processed and the iterator points at a valid token, handle unexpected token
                if (token_begin != token_end && token_is_valid(*token_begin)) {
                    throw parse_exception(get_position(input, *token_begin), (boost::format("unexpected %1%.") % static_cast<token_id>(token_begin->id())).str());
                }
            } catch (lexer_exception<Iterator> const& ex) {
                throw parse_exception(ex.location().position(), ex.what());
            } catch (qi::expectation_failure<typename Lexer::iterator_type> const& ex) {
                throw parse_exception(get_position(input, *ex.first), to_string(ex, interpolation));
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

}}  // namespace puppet::compiler