/**
 * @file
 * Declares the compilation environment.
 */
#pragma once

#include "ast/ast.hpp"
#include "settings.hpp"
#include "registry.hpp"
#include "module.hpp"
#include "finder.hpp"
#include "evaluation/dispatcher.hpp"
#include "../logging/logger.hpp"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace puppet { namespace compiler {

    namespace evaluation {

        // Forward declaration of context.
        struct context;

    }  // namespace puppet::compiler::evaluation

    /**
     * Represents a compilation environment.
     */
    struct environment : finder
    {
        /**
         * Constructs a new environment.
         * @param logger The logger to use while loading modules.
         * @param settings The compiler settings.
         * @param name The name of the environment (e.g. 'production').
         * @param directory The directory for the environment.
         */
        environment(logging::logger& logger, compiler::settings const& settings, std::string name, std::string directory);

        /**
         * Gets the associated compiler settings.
         * @return Returns the associated compiler settings.
         */
        compiler::settings const& settings() const;

        /**
         * Gets the name of the environment.
         * @return Returns the name of the environment.
         */
        std::string const& name() const;

        /**
         * Gets the environment's registry.
         * @return Returns the environment's registry.
         */
        compiler::registry const& registry() const;

        /**
         * Gets the environment's function dispatcher.
         * @return Returns the environment's function dispatcher.
         */
        evaluation::dispatcher& dispatcher();

        /**
         * Gets the environment's function dispatcher.
         * @return Returns the environment's function dispatcher.
         */
        evaluation::dispatcher const& dispatcher() const;

        /**
         * Compiles the environment's manifests for the given evaluation context.
         * @param context The current evaluation context.
         */
        void compile(evaluation::context& context);

        /**
         * Finds a module by name.
         * @param name The module name to find.
         * @return Returns a pointer to the module or nullptr if the module does not exist.
         */
        module* find_module(std::string const& name);

        /**
         * Imports a file into the environment's registry.
         * @param logger The logger to use to log messages.
         * @param type The type of file to find.
         * @param name The qualified name to translated into a file (e.g. 'foo::bar::baz' => '.../modules/foo/bar/baz').
         */
        void import(logging::logger& logger, find_type type, std::string const& name);

     private:
        void load_modules(logging::logger& logger, std::string const& directory);
        std::shared_ptr<ast::syntax_tree> import(logging::logger& logger, std::string const& path, compiler::module const* module = nullptr);

        compiler::settings const& _settings;
        std::string _name;
        compiler::registry _registry;
        evaluation::dispatcher _dispatcher;
        std::unordered_map<std::string, std::shared_ptr<ast::syntax_tree>> _parsed;
        std::unordered_map<std::string, module> _modules;
    };

}}  // puppet::compiler
