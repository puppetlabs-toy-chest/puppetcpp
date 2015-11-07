#include <puppet/compiler/exceptions.hpp>
#include <puppet/compiler/lexer/lexer.hpp>
#include <puppet/compiler/ast/ast.hpp>
#include <puppet/cast.hpp>

using namespace std;

namespace puppet { namespace compiler {

    parse_exception::parse_exception(string const& message, lexer::position position) :
        runtime_error(message),
        _position(rvalue_cast(position))
    {
    }

    lexer::position const& parse_exception::position() const
    {
        return _position;
    }

    evaluation_exception::evaluation_exception(string const& message) :
        runtime_error(message)
    {
    }

    evaluation_exception::evaluation_exception(string const& message, ast::context const& context) :
        runtime_error(message),
        _context(context)
    {
        if (_context.tree) {
            // Take a shared pointer on the tree
            // This ensures the AST continues to exist even if the exception propagates beyond the tree's original scope
            _tree = context.tree->shared_from_this();
        }
    }

    ast::context const& evaluation_exception::context() const
    {
        return _context;
    }

    compilation_exception::compilation_exception(string const& message, string path, size_t line, size_t column, string text) :
        runtime_error(message),
        _path(rvalue_cast(path)),
        _line(line),
        _column(column),
        _text(rvalue_cast(text))
    {
    }

    compilation_exception::compilation_exception(parse_exception const& ex, string const& path) :
        runtime_error(ex.what()),
        _path(path),
        _line(ex.position().line())
    {
        ifstream input{path};
        if (input) {
            tie(_text, _column) = lexer::get_text_and_column(input, ex.position().offset());
        }
    }

    compilation_exception::compilation_exception(evaluation_exception const& ex) :
        runtime_error(ex.what())
    {
        auto& context = ex.context();
        if (context.tree) {
            _path = context.tree->path();
            _line = context.position.line();
            ifstream input{_path};
            if (input) {
                tie(_text, _column) = lexer::get_text_and_column(input, context.position.offset());
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

    string const& compilation_exception::text() const
    {
        return _text;
    }

    settings_exception::settings_exception(string const& message) :
        runtime_error(message)
    {
    }

}}  // namespace puppet::compiler