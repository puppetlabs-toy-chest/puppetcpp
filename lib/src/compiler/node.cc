#include <puppet/compiler/node.hpp>
#include <puppet/runtime/expression_evaluator.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime;
using namespace puppet::logging;

namespace puppet { namespace compiler {

    compilation_exception::compilation_exception(string const& message, string path, size_t line, size_t column, string text) :
        runtime_error(message),
        _path(rvalue_cast(path)),
        _line(line),
        _column(column),
        _text(rvalue_cast(text))
    {
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

    node::node(string name, compiler::environment& environment) :
        _name(rvalue_cast(name)),
        _environment(environment)
    {
    }

    string const& node::name() const
    {
        return _name;
    }

    compiler::environment& node::environment()
    {
        return _environment;
    }

    catalog node::compile(logging::logger& logger, string const& path)
    {
        auto compilation_context = make_shared<compiler::context>(logger, make_shared<string>(path), *this);
        logger.log(level::debug, "parsed syntax tree:\n%1%", compilation_context->tree());

        try {
            runtime::catalog catalog;
            runtime::context evaluation_context{catalog};

            // TODO: set parameters and facts in the top scope

            // TODO: create settings scope in catalog

            // Evaluate the syntax tree
            expression_evaluator evaluator{compilation_context, evaluation_context};
            evaluator.evaluate();

            // TODO: evaluate node scope

            // TODO: evaluate node classes

            // TODO: evaluate generators

            // TODO: finalize catalog
            return catalog;
        } catch (evaluation_exception const& ex) {
            throw compilation_context->create_exception(ex.position(), ex.what());
        }
    }

}}  // namespace puppet::compiler
