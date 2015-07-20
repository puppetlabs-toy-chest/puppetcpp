#include <puppet/compiler/context.hpp>
#include <puppet/compiler/parser.hpp>
#include <puppet/compiler/node.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::logging;

namespace puppet { namespace compiler {

    context::context(logging::logger& logger, shared_ptr<string> path, compiler::node& node) :
        _logger(logger),
        _path(rvalue_cast(path)),
        _node(node)
    {
        if (!_path) {
            throw runtime_error("expected path");
        }
        _stream.open(*_path);
        if (!_stream) {
            throw compilation_exception("file does not exist or cannot be read.", *_path);
        }

        // Parse the file into a syntax tree
        try {
            logger.log(level::debug, "parsing '%1%'.", *_path);
            _tree = parser::parse(_stream);
            logger.log(level::debug, "parsed syntax tree:\n%1%", _tree);
        } catch (parse_exception const& ex) {
            throw create_exception(ex.position(), ex.what());
        }
    }

    logging::logger& context::logger()
    {
        return _logger;
    }

    shared_ptr<string> const& context::path() const
    {
        return _path;
    }

    ast::syntax_tree const& context::tree() const
    {
        return _tree;
    }

    compiler::node& context::node()
    {
        return _node;
    }

    void context::log(logging::level level, lexer::position const& position, std::string const& message)
    {
        string text;
        size_t column;
        tie(text, column) = get_text_and_column(_stream, position.offset());
        _logger.log(level, position.line(), column, text, *_path, "node '%1%': %2%", _node.name(), message);
    }

    compilation_exception context::create_exception(lexer::position const& position, string const& message)
    {
        string text;
        size_t column;
        tie(text, column) = get_text_and_column(_stream, position.offset());
        return compilation_exception(message, *_path, position.line(), column, rvalue_cast(text));
    }

}}  // namespace puppet::compiler
