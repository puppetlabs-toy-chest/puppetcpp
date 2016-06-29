#include <puppet/compiler/parser/parser.hpp>
#include <puppet/compiler/parser/rules.hpp>
#include <puppet/compiler/lexer/static_lexer.hpp>
#include <puppet/compiler/lexer/lexer.hpp>
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
                iterator.position(),
                position{ iterator.position().offset() + 1, iterator.position().line() }
            );
        }
    }

    template <typename Lexer, typename Input>
    void parse(Lexer const& lexer, Input& input, ast::syntax_tree& tree, bool epp = false)
    {
        namespace x3 = boost::spirit::x3;

        try {
            // Get lexer iterators from the input
            auto begin = lex_begin(input);
            auto end = lex_end(input);

            // Get the token iterators from the lexer
            auto token_begin = lexer.begin(begin, end, epp ? EPP_STATE : nullptr);
            auto token_end = lexer.end();

            // Parse the input
            bool success = false;
            if (epp) {
                auto parser = x3::with<tree_context_tag>(&tree)[epp_syntax_tree];
                success = x3::parse(token_begin, token_end, parser, tree);
            } else {
                auto parser = x3::with<tree_context_tag>(&tree)[parser::syntax_tree];
                success = x3::parse(token_begin, token_end, parser, tree);
            }

            // Check for success; for interpolation, it is not required to exhaust all tokens
            if (success && (token_begin == token_end || token_begin->id() == boost::lexer::npos)) {
                if (epp) {
                    check_missing_epp_end(begin);
                }
                return;
            }

            // If not all tokens were processed and the iterator points at a valid token, handle unexpected token
            if (token_begin != token_end && token_is_valid(*token_begin)) {
                throw parse_exception{ *token_begin };
            }
            throw parse_exception{ begin, end };
        } catch (lexer_exception<typename Lexer::input_iterator_type> const& ex) {
            throw parse_exception{ ex };
        } catch (x3::expectation_failure<typename Lexer::iterator_type> const& ex) {
            position begin;
            position end;
            if (ex.where()->id() == boost::lexer::npos) {
                begin = lexer::get_last_position(input);
                end = lexer::position{ begin.offset() + 1, begin.line() };
            } else {
                tie(begin, end) = boost::apply_visitor(lexer::token_range_visitor(), ex.where()->value());
            }
            throw parse_exception{ ex, rvalue_cast(begin), rvalue_cast(end) };
        }
    }

    shared_ptr<ast::syntax_tree> parse_file(logging::logger& logger, std::string path, compiler::module const* module, bool epp)
    {
        auto tree = ast::syntax_tree::create(rvalue_cast(path), module);
        ifstream input(tree->path());
        if (!input) {
            throw compilation_exception((boost::format("file '%1%' does not exist or cannot be read.") % tree->path()).str());
        }

        file_static_lexer lexer{ [&](logging::level level, std::string const& message, lexer::position const& position, size_t length) {
            if (!logger.would_log(level)) {
                return;
            }

            auto info = lexer::get_line_info(input, position.offset(), length);
            logger.log(level, position.line(), info.column, info.length, info.text, tree->path(), message);
        }};

        parse(lexer, input, *tree, epp);
        return tree;
    }

    shared_ptr<ast::syntax_tree> parse_string(logging::logger& logger, std::string source, std::string path, compiler::module const* module, bool epp)
    {
        auto tree = ast::syntax_tree::create(rvalue_cast(path), module);

        string_static_lexer lexer{ [&](logging::level level, std::string const& message, lexer::position const& position, size_t length) {
            if (!logger.would_log(level)) {
                return;
            }

            auto info = lexer::get_line_info(source, position.offset(), length);
            logger.log(level, position.line(), info.column, info.length, info.text, tree->path(), message);
        }};

        parse(lexer, source, *tree, epp);
        tree->source(rvalue_cast(source));
        return tree;
    }

    boost::optional<ast::postfix_expression> parse_postfix(std::string const& source)
    {
        string_static_lexer lexer;

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
