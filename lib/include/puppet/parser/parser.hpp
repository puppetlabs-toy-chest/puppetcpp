/**
 * @file
 * Declares the Puppet language parser.
 */
#pragma once

#include "grammar.hpp"
#include "../lexer/lexer.hpp"
#include <boost/optional.hpp>

namespace puppet { namespace parser {

    /**
     * Implements the Puppet language parser.
     */
    struct parser
    {
        /**
         * Parses the given manifest file into an AST manifest.
         * @tparam ErrorReporter The error reporter type.
         * @param reporter The error reporter to use for warnings and errors.
         * @param input The input file to parse.
         * @param path The path to the manifest file begin parsed.
         * @return Returns the AST manifest if parsing succeeds or nullptr if not.
         */
        template <typename ErrorReporter>
        static boost::optional<ast::manifest> parse_manifest_file(ErrorReporter& reporter, std::ifstream& input, std::string const& path)
        {
            return parse_manifest<lexer::file_static_lexer>(reporter, input, path);
        }

        /**
         * Parses the given manifest string into an AST manifest.
         * @tparam ErrorReporter The error reporter type.
         * @param reporter The error reporter to use for warnings and errors.
         * @param contents The manifest contents to parse.
         * @return Returns the AST manifest if parsing succeeds or nullptr if not.
         */
        template <typename ErrorReporter>
        static boost::optional<ast::manifest> parse_manifest_string(ErrorReporter& reporter, std::string const& contents)
        {
            return parse_manifest<lexer::string_static_lexer>(reporter, contents, "<string>");
        }

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

        template <typename Lexer, typename ErrorReporter, typename Input>
        static boost::optional<ast::manifest> parse_manifest(ErrorReporter& reporter, Input& input, std::string const& path)
        {
            using namespace std;
            using namespace puppet::lexer;
            using namespace boost::spirit::qi;

            auto input_begin = lex_begin(input);
            auto input_end = lex_end(input);

            // Used in error handling
            token_position error_position;
            boost::optional<token_id> unexpected_token;
            std::string error_message;

            try {
                // Construct a lexer of the given type with a callback that reports warnings
                Lexer lexer([&](token_position const& position, std::string const& message) {
                    std::string line;
                    size_t column;
                    tie(line, column) = get_line_and_column(input, get<0>(position));
                    reporter.warning_with_location(path, line, get<1>(position), column, message);
                });
                auto token_begin = lexer.begin(input_begin, input_end);
                auto token_end = lexer.end();

                // Parse the input into an AST manifest
                ast::manifest manifest;
                if (parse(token_begin, token_end, grammar<Lexer>(lexer), manifest) && (token_begin == token_end || token_begin->id() == boost::lexer::npos)) {
                    return manifest;
                }

                // If not all tokens were processed and the iterator points at a valid token, handle unexpected token
                if (token_begin != token_end && token_is_valid(*token_begin)) {
                    error_position = get_position(input, *token_begin);
                    unexpected_token = static_cast<token_id>(token_begin->id());
                }
                else {
                    error_position = input_begin.position();
                }
            } catch (lexer_exception<decltype(input_begin)> const& ex) {
                error_position = ex.location().position();
                error_message = ex.what();
            } catch (expectation_failure<typename Lexer::iterator_type> const& ex) {
                error_position = get_position(input, *ex.first);
                error_message = to_string(ex);
            }

            std::string line;
            size_t column;
            tie(line, column) = get_line_and_column(input, get<0>(error_position));

            if (unexpected_token) {
                reporter.error_with_location(path, line, get<1>(error_position), column, "unexpected %1%.", *unexpected_token);
            }
            else {
                reporter.error_with_location(path, line, get<1>(error_position), column, error_message);
            }
            return nullptr;
        }
    };

}}  // namespace puppet::parser