#include <puppet/compiler/parser/parser.hpp>
#include <puppet/compiler/parser/rules.hpp>
#include <puppet/compiler/lexer/static_lexer.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <puppet/cast.hpp>
#include <sstream>
#include <fstream>
#include <iomanip>

using namespace std;
using namespace puppet::compiler::lexer;
using namespace boost::spirit;

namespace puppet { namespace compiler { namespace parser {

    template <typename Iterator>
    void check_missing_epp_end(Iterator const& iterator)
    {
        if (!iterator.epp_end()) {
            throw parse_exception(
                "expected '%>' or '-%>' but found end of input.",
                lexer::range{
                    iterator.position(),
                    lexer::position{
                        iterator.position().offset() + 1,
                        iterator.position().line()
                    }
                }
            );
        }
    }

    template <typename Lexer, typename Input>
    void parse(Lexer const& lexer, Input& input, ast::syntax_tree& tree, bool epp = false, bool interpolation = false)
    {
        namespace x3 = boost::spirit::x3;

        try {
            // Get lexer iterators from the input
            auto begin = lex_begin(input);
            auto end = lex_end(input);

            // Get the token iterators from the lexer
            auto token_begin = lexer.begin(begin, end, epp ? Lexer::EPP_STATE : nullptr);
            auto token_end = lexer.end();

            // Check for "semantically empty" input
            if (token_begin != token_end && token_begin->id() == boost::lexer::npos) {
                if (epp) {
                    check_missing_epp_end(begin);
                }
                return;
            }

            // Parse the input
            bool success = false;
            if (interpolation) {
                auto parser = x3::with<tree_context_tag>(&tree)[interpolated_syntax_tree];
                success = x3::parse(token_begin, token_end, parser, tree);
            } else if (epp) {
                auto parser = x3::with<tree_context_tag>(&tree)[epp_syntax_tree];
                success = x3::parse(token_begin, token_end, parser, tree);
            } else {
                auto parser = x3::with<tree_context_tag>(&tree)[parser::syntax_tree];
                success = x3::parse(token_begin, token_end, parser, tree);
            }

            // Check for success; for interpolation, it is not required to exhaust all tokens
            if (success && (token_begin == token_end || token_begin->id() == boost::lexer::npos || interpolation)) {
                if (epp) {
                    check_missing_epp_end(begin);
                }
                return;
            }

            // If not all tokens were processed and the iterator points at a valid token, handle unexpected token
            if (token_begin != token_end && token_is_valid(*token_begin)) {
                throw parse_exception((boost::format("syntax error: unexpected %1%.") % static_cast<token_id>(token_begin->id())).str(), get_range(input, *token_begin));
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
            throw parse_exception(
                message.str(),
                lexer::range{
                    begin.position(),
                    lexer::position{
                        begin.position().offset() + 1,
                        begin.position().line()
                    }
                }
            );
        } catch (lexer_exception<typename Lexer::input_iterator_type> const& ex) {
            auto& location = ex.location().position();
            throw parse_exception(
                ex.what(),
                lexer::range{
                    location,
                    lexer::position{
                        location.offset() + 1,
                        location.line()
                    }
                }
            );
        } catch (x3::expectation_failure<typename Lexer::iterator_type> const& ex) {
            throw parse_exception(
                (boost::format("expected %1% but found %2%.") %
                 ex.which() %
                 static_cast<token_id>(ex.where()->id())
                ).str(),
                get_range(input, *ex.where()));
        }
    }

    shared_ptr<ast::syntax_tree> parse_file(std::string path, compiler::module const* module, bool epp)
    {
        static file_static_lexer lexer;

        ifstream input(path);
        if (!input) {
            throw compilation_exception((boost::format("file '%1%' does not exist or cannot be read.") % path).str());
        }

        auto tree = ast::syntax_tree::create(rvalue_cast(path), module);
        parse(lexer, input, *tree, epp);
        return tree;
    }

    shared_ptr<ast::syntax_tree> parse_string(std::string source, std::string path, compiler::module const* module, bool epp)
    {
        static string_static_lexer lexer;

        auto tree = ast::syntax_tree::create(rvalue_cast(path), module);
        parse(lexer, source, *tree, epp);
        tree->source(rvalue_cast(source));
        return tree;
    }

    shared_ptr<ast::syntax_tree> interpolate(boost::iterator_range<lexer_string_iterator> range, compiler::module const* module)
    {
        static string_static_lexer lexer;

        auto tree = ast::syntax_tree::create("<string>", module);
        parse(lexer, range, *tree, false, true);
        return tree;
    }

    boost::optional<ast::postfix_expression> parse_postfix(std::string const& source)
    {
        static string_static_lexer lexer;

        try {
            // Get lexer iterators
            auto begin = lex_begin(source);
            auto end = lex_end(source);

            // Get the token iterators from the lexer
            auto token_begin = lexer.begin(begin, end);
            auto token_end = lexer.end();

            // Parse the entire source into a single postfix expression
            ast::postfix_expression result;
            if (x3::parse(token_begin, token_end, x3::with<tree_context_tag>(nullptr)[parser::postfix_expression], result) &&
                (token_begin == token_end || token_begin->id() == boost::lexer::npos)) {
                return result;
            }
        } catch (lexer_exception<typename string_static_lexer::input_iterator_type> const& ex) {
        } catch (x3::expectation_failure<typename string_static_lexer::iterator_type> const& ex) {
        }
        return boost::none;
    }

}}}  // namespace puppet::compiler::parser
