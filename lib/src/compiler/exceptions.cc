#include <puppet/compiler/exceptions.hpp>
#include <puppet/compiler/lexer/lexer.hpp>
#include <puppet/compiler/ast/ast.hpp>
#include <puppet/cast.hpp>
#include <sstream>
#include <iomanip>

using namespace std;

namespace puppet { namespace compiler {

    argument_exception::argument_exception(std::string const& message, size_t index) :
        runtime_error(message),
        _index(index)
    {
    }

    size_t argument_exception::index() const
    {
        return _index;
    }

    parse_exception::parse_exception(string const& message, lexer::position begin, lexer::position end) :
        runtime_error(message),
        _begin(rvalue_cast(begin)),
        _end(rvalue_cast(end))
    {
    }

    lexer::position const& parse_exception::begin() const
    {
        return _begin;
    }

    lexer::position const& parse_exception::end() const
    {
        return _end;
    }

    string parse_exception::format_message(lexer::token_id id)
    {
        return (boost::format("syntax error: unexpected %1%.") % id).str();
    }

    string parse_exception::format_message(boost::optional<char> character)
    {
        ostringstream message;
        if (character) {
            message << "unexpected character ";
            if (isprint(*character)) {
                message << '\'' << *character << '\'';
            } else {
                message << "0x" << setw(2) << setfill('0') << static_cast<int>(*character);
            }
            message << ".";
        } else {
            message << "unexpected end of input.";
        }
        return message.str();
    }

    string parse_exception::format_message(string const& expected, lexer::token_id found)
    {
        return (boost::format("expected %1% but found %2%.") % expected % found).str();
    }

    evaluation_exception::evaluation_exception(string const& message, vector<evaluation::stack_frame> backtrace) :
        runtime_error(message),
        _backtrace(rvalue_cast(backtrace))
    {
    }

    evaluation_exception::evaluation_exception(string const& message, ast::context context, vector<evaluation::stack_frame> backtrace) :
        runtime_error(message),
        _context(rvalue_cast(context)),
        _backtrace(rvalue_cast(backtrace))
    {
        if (_context.tree) {
            // Take a shared pointer on the tree
            // This ensures the AST continues to exist even if the exception propagates beyond the tree's original scope
            _tree = _context.tree->shared_from_this();
        }
    }

    ast::context const& evaluation_exception::context() const
    {
        return _context;
    }

    vector<evaluation::stack_frame> const& evaluation_exception::backtrace() const
    {
        return _backtrace;
    }

    compilation_exception::compilation_exception(string const& message, string path, size_t line, size_t column, size_t length, string text) :
        runtime_error(message),
        _path(rvalue_cast(path)),
        _line(line),
        _column(column),
        _length(length),
        _text(rvalue_cast(text))
    {
    }

    compilation_exception::compilation_exception(parse_exception const& ex, string const& path, string const& source) :
        runtime_error(ex.what()),
        _path(path),
        _line(ex.begin().line()),
        _column(0),
        _length(ex.end().offset() - ex.begin().offset())
    {
        if (source.empty()) {
            ifstream input{path};
            if (input) {
                tie(_text, _column) = lexer::get_text_and_column(input, ex.begin().offset());
            }
        } else {
            tie(_text, _column) = lexer::get_text_and_column(source, ex.begin().offset());
        }
    }

    compilation_exception::compilation_exception(evaluation_exception const& ex) :
        runtime_error(ex.what()),
        _line(0),
        _column(0),
        _length(0),
        _backtrace(ex.backtrace())
    {
        auto& context = ex.context();
        if (context.tree) {
            _path = context.tree->path();
            _line = context.begin.line();
            _length = context.end.offset() - context.begin.offset();
            if (context.tree->source().empty()) {
                ifstream input{ _path };
                if (input) {
                    tie(_text, _column) = lexer::get_text_and_column(input, context.begin.offset());
                }
            } else {
                tie(_text, _column) = lexer::get_text_and_column(context.tree->source(), context.begin.offset());
            }
        }
    }

    string const& compilation_exception::path() const
    {
        return _path;
    }

    size_t compilation_exception::line() const
    {
        return _line;
    }

    size_t compilation_exception::column() const
    {
        return _column;
    }

    size_t compilation_exception::length() const
    {
        return _length;
    }

    string const& compilation_exception::text() const
    {
        return _text;
    }

    vector<evaluation::stack_frame> const& compilation_exception::backtrace() const
    {
        return _backtrace;
    }

}}  // namespace puppet::compiler
