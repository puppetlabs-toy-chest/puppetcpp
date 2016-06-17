#include <puppet/compiler/evaluation/repl.hpp>
#include <puppet/compiler/parser/parser.hpp>
#include <puppet/compiler/parser/rules.hpp>
#include <puppet/compiler/scanner.hpp>
#include <puppet/compiler/evaluation/evaluator.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::compiler::lexer;
namespace x3 = boost::spirit::x3;

namespace puppet { namespace compiler { namespace evaluation {

    static string REPL_PATH = "<repl>";

    struct evaluation_helper
    {
        evaluation_helper(evaluation::repl& repl, bool& multiline) :
            _repl(repl),
            _multiline(multiline)
        {
        }

        ~evaluation_helper()
        {
            _repl.complete(_multiline);
        }

     private:
        evaluation::repl& _repl;
        bool& _multiline;
    };

    repl::repl(evaluation::context& context, function<void(compilation_exception const&)> error_handler) :
        _error_handler(rvalue_cast(error_handler)),
        _lexer(create_lexer_callback(context)),
        _scanner(context.node().environment().registry(), context.node().environment().dispatcher()),
        _evaluator(context)
    {
        _prompt = _evaluator.context().node().name() + ":001:1> ";
    }

    string const& repl::prompt() const
    {
        return _prompt;
    }

    size_t repl::count() const
    {
        return _count;
    }

    size_t repl::line() const
    {
        return _line;
    }

    boost::optional<repl::result> repl::evaluate(string const& source)
    {
        return evaluate(source.c_str());
    }

    boost::optional<repl::result> repl::evaluate(char const* source)
    {
        bool multiline = false;
        evaluation_helper helper{ *this, multiline };

        if (!source || !*source) {
            multiline = !_buffer.empty();
            return boost::none;
        }

        _buffer += source;

        try {
            // Get the input iterators
            auto begin = lex_begin(_buffer);
            auto end = lex_end(_buffer);

            // Get the token iterators
            auto token_begin = _lexer.begin(begin, end);
            auto token_end = _lexer.end();

            // Check for "semantically empty" input
            if (token_begin != token_end && token_begin->id() == boost::lexer::npos) {
                return boost::none;
            }

            auto tree = ast::syntax_tree::create(REPL_PATH);
            tree->statements.emplace_back();
            auto& statement = tree->statements.front();

            bool success = x3::parse(
                token_begin,
                token_end,
                x3::with<compiler::parser::tree_context_tag>(tree.get())[compiler::parser::statement],
                statement
            );
            if (success && (token_begin == token_end || token_begin->id() == boost::lexer::npos)) {
                // Copy the source into the tree
                tree->source(_buffer);

                // Validate the AST
                tree->validate();

                // If the expression contains a definition, we need to keep the tree around
                if (_scanner.scan(*tree)) {
                    _trees.emplace_back(tree);
                }

                repl::result result;
                result.source = _buffer;
                result.value = _evaluator.evaluate(statement);
                return result;
            }

            if (_error_handler) {
                // If not all tokens were processed and the iterator points at a valid token, handle unexpected token
                if (token_begin != token_end && token_is_valid(*token_begin)) {
                    parse_exception ex{ *token_begin  };
                    _error_handler(compilation_exception{ ex, REPL_PATH, _buffer });
                } else {
                    parse_exception ex{ begin, end  };
                    _error_handler(compilation_exception{ ex, REPL_PATH, _buffer });
                }
            }
        } catch (lexer_exception<typename lexer_type::input_iterator_type> const& ex) {
            if (_error_handler) {
                _error_handler(compilation_exception{ parse_exception{ ex }, REPL_PATH, _buffer });
            }
        } catch (x3::expectation_failure<typename lexer_type::iterator_type> const& ex) {
            if (ex.where()->id() == boost::lexer::npos) {
                multiline = true;
            } else if (_error_handler) {
                position begin;
                position end;
                tie(begin, end) = boost::apply_visitor(lexer::token_range_visitor(), ex.where()->value());
                _error_handler(compilation_exception{ parse_exception{ ex, begin, end }, REPL_PATH, _buffer });
            }
        } catch (parse_exception const& ex) {
            if (_error_handler) {
                _error_handler(compilation_exception{ ex , REPL_PATH, _buffer });
            }
        } catch (evaluation_exception const& ex) {
            if (_error_handler) {
                _error_handler(compilation_exception{ ex });
            }
        } catch (compilation_exception const& ex) {
            if (_error_handler) {
                _error_handler(ex);
            }
        }
        return boost::none;
    }

    function<void(logging::level, string const&, position const&, size_t)> repl::create_lexer_callback(evaluation::context& context)
    {
        return [&](logging::level level, string const& message, lexer::position const& position, size_t length) {
            if (!context.node().logger().would_log(level)) {
                return;
            }

            auto info = lexer::get_line_info(_buffer, position.offset(), length);
            context.node().logger().log(level, position.line(), info.column, info.length, info.text, REPL_PATH, message);
        };
    }

    void repl::complete(bool multiline)
    {
        // If the evaluation didn't complete because it spans lines, append a newline and increment the line count
        if (multiline) {
            _buffer += '\n';
            ++_line;
        } else {
            _buffer.clear();
            _line = 1;
            ++_count;
        }
        _prompt = (boost::format("%1%:%2$03d:%3%> ") % _evaluator.context().node().name() % _count % _line).str();
    }

}}}  // puppet::compiler::evaluation
