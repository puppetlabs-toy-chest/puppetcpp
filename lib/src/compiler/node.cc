#include <puppet/compiler/node.hpp>
#include <puppet/compiler/parser.hpp>
#include <puppet/runtime/expression_evaluator.hpp>
#include <puppet/cast.hpp>
#include <fstream>

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
        ifstream file(path);
        if (!file) {
            throw compilation_exception("file does not exist or cannot be read.", path);
        }

        try {
            // Parse the manifest file
            auto manifest = parser::parse(file);
            logger.log(level::debug, "parsed syntax tree:\n%1%", manifest);

            // Create a helper warning function
            auto warning = [&](lexer::position const& position, string const& message) {
                string text;
                size_t column;
                tie(text, column) = get_text_and_column(file, position.offset());
                logger.log(level::warning, position.line(), column, text, path, message);
            };

            runtime::catalog catalog;

            // TODO: set parameters and facts in the top scope

            // TODO: create settings scope in catalog

            // Evaluate all the top level expressions in the manifest
            if (manifest.body) {
                expression_evaluator evaluator(logger, catalog, path, *this, warning);
                for (auto& expression : *manifest.body) {
                    // Top level expressions must be productive
                    evaluator.evaluate(expression, true);
                }
            }

            // TODO: evaluate node scope

            // TODO: evaluate node classes

            // TODO: evaluate generators

            // TODO: finalize catalog

            return catalog;
        } catch (parse_exception const& ex) {
            string text;
            size_t column;
            tie(text, column) = get_text_and_column(file, ex.position().offset());
            throw compilation_exception(ex.what(), path, ex.position().line(), column, rvalue_cast(text));
        } catch (evaluation_exception const& ex) {
            string text;
            size_t column;
            tie(text, column) = get_text_and_column(file, ex.position().offset());
            throw compilation_exception(ex.what(), path, ex.position().line(), column, rvalue_cast(text));
        }
    }

}}  // namespace puppet::compiler
