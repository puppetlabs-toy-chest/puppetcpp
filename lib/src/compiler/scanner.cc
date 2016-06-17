#include <puppet/compiler/scanner.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <puppet/compiler/ast/visitors/definition.hpp>
#include <puppet/cast.hpp>
#include <boost/format.hpp>

using namespace std;

namespace puppet { namespace compiler {

    scanner::scanner(compiler::registry& registry, evaluation::dispatcher& dispatcher) :
        _registry(registry),
        _dispatcher(dispatcher)
    {
    }

    bool scanner::scan(ast::syntax_tree const& tree)
    {
        bool registered = false;
        ast::visitors::definition visitor{
            [&](std::string name, ast::visitors::definition::statement const& definition) {
                if (auto statement = boost::get<ast::class_statement const*>(&definition)) {
                    register_class(rvalue_cast(name), **statement);
                } else if (auto statement = boost::get<ast::defined_type_statement const*>(&definition)) {
                    register_defined_type(rvalue_cast(name), **statement);
                } else if (auto statement = boost::get<ast::node_statement const*>(&definition)) {
                    register_node(**statement);
                }  else if (auto statement = boost::get<ast::function_statement const*>(&definition)) {
                    register_function(**statement);
                } else if (auto statement = boost::get<ast::type_alias_statement const*>(&definition)) {
                    register_type_alias(**statement);
                } else if (auto statement = boost::get<ast::produces_statement const*>(&definition)) {
                    register_produces(**statement);
                } else if (auto statement = boost::get<ast::consumes_statement const*>(&definition)) {
                    register_consumes(**statement);
                } else if (auto statement = boost::get<ast::application_statement const*>(&definition)) {
                    register_application(**statement);
                } else if (auto statement = boost::get<ast::site_statement const*>(&definition)) {
                    register_site(**statement);
                } else {
                    throw runtime_error("unsupported definition statement.");
                }

                registered = true;
            }
        };
        visitor.visit(tree);
        return registered;
    }

    void scanner::register_class(string name, ast::class_statement const& statement)
    {
        if (auto existing = _registry.find_class(name)) {
            throw parse_exception(
                (boost::format("class '%1%' was previously defined at %2%:%3%.") %
                 existing->name() %
                 existing->statement().tree->path() %
                 existing->statement().begin.line()
                ).str(),
                statement.name.begin,
                statement.name.end
            );
        }

        if (auto existing = _registry.find_defined_type(name)) {
            throw parse_exception(
                (boost::format("'%1%' was previously defined as a defined type at %2%:%3%.") %
                 existing->name() %
                 existing->statement().tree->path() %
                 existing->statement().begin.line()
                ).str(),
                statement.name.begin,
                statement.name.end
            );
        }

        _registry.register_class(klass{ rvalue_cast(name), statement });
    }

    void scanner::register_defined_type(string name, ast::defined_type_statement const& statement)
    {
        if (auto existing = _registry.find_defined_type(name)) {
            throw parse_exception(
                (boost::format("defined type '%1%' was previously defined at %2%:%3%.") %
                 existing->name() %
                 existing->statement().tree->path() %
                 existing->statement().begin.line()
                ).str(),
                statement.name.begin,
                statement.name.end
            );
        }

        if (auto existing = _registry.find_class(name)) {
            throw parse_exception(
                (boost::format("'%1%' was previously defined as a class at %2%:%3%.") %
                 existing->name() %
                 existing->statement().tree->path() %
                 existing->statement().begin.line()
                ).str(),
                statement.name.begin,
                statement.name.end
            );
        }

        _registry.register_defined_type(defined_type{ rvalue_cast(name), statement });
    }

    void scanner::register_node(ast::node_statement const& statement)
    {
        if (auto existing = _registry.find_node(statement)) {
            throw parse_exception(
                (boost::format("a conflicting node definition was previously defined at %1%:%2%.") %
                 existing->statement().tree->path() %
                 existing->statement().begin.line()
                ).str(),
                statement.begin,
                statement.end
            );
        }

        _registry.register_node(node_definition{ statement });
    }

    void scanner::register_function(ast::function_statement const& statement)
    {
        if (auto descriptor = _dispatcher.find(statement.name.value)) {
            if (auto existing = descriptor->statement()) {
                throw parse_exception(
                    (boost::format("cannot define function '%1%' because it conflicts with a previous definition at %2%:%3%.") %
                     statement.name %
                     existing->tree->path() %
                     existing->begin.line()
                    ).str(),
                    statement.name.begin,
                    statement.name.end
                );
            }
            throw parse_exception(
                (boost::format("cannot define function '%1%' because it conflicts with a built-in function of the same name.") %
                 statement.name
                ).str(),
                statement.name.begin,
                statement.name.end
            );
        }

        _dispatcher.add(evaluation::functions::descriptor{ statement.name.value, &statement });
    }

    void scanner::register_type_alias(ast::type_alias_statement const& statement)
    {
        auto alias = _registry.find_type_alias(statement.alias.name);
        if (alias) {
            auto context = alias->statement().context();
            throw parse_exception(
                (boost::format("type alias '%1%' was previously defined at %2%:%3%.") %
                 statement.alias %
                 context.tree->path() %
                 context.begin.line()
                ).str(),
                statement.alias.begin,
                statement.alias.end
            );
        }

        _registry.register_type_alias(type_alias{ statement });
    }

    void scanner::register_produces(ast::produces_statement const& statement)
    {
        // TODO: implement
    }

    void scanner::register_consumes(ast::consumes_statement const& statement)
    {
        // TODO: implement
    }

    void scanner::register_application(ast::application_statement const& statement)
    {
        // TODO: implement
    }

    void scanner::register_site(ast::site_statement const& statement)
    {
        // TODO: implement
    }

}}  // namespace puppet::compiler
