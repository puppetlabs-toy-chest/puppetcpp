/**
 * @file
 * Declares the compilation environment.
 */
#pragma once

#include "ast/ast.hpp"
#include "registry.hpp"
#include "module.hpp"
#include "finder.hpp"
#include "settings.hpp"
#include "evaluation/dispatcher.hpp"
#include "../logging/logger.hpp"
#include <string>
#include <vector>
#include <deque>
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
         * Creates a new environment given the compiler settings.
         * @param logger The logger to use for logging messages.
         * @param settings The settings to use for the environment.
         * @param manifests The main manifests to use instead of the environment's default.
         * @return Returns the new environment.
         */
        static std::shared_ptr<environment> create(logging::logger& logger, compiler::settings settings, std::vector<std::string> const& manifests = {});

        /**
         * Gets the name of the environment.
         * @return Returns the name of the environment.
         */
        std::string const& name() const;

        /**
         * Gets the compiler settings for the environment.
         * @return Returns the compiler settings for the environment.
         */
        compiler::settings const& settings() const;

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
         * Gets the environment's modules.
         * Note: the module list will be empty unless load is called.
         * @return Returns the environment's modules.
         */
        std::deque<module> const& modules() const;

        /**
         * Gets the environment's main manifests.
         * Note: the manifest list will be empty unless load is called.
         * @return Returns the environment's main manifests.
         */
        std::vector<std::string> const& manifests() const;

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
         * Finds a module by name.
         * @param name The module name to find.
         * @return Returns a pointer to the module or nullptr if the module does not exist.
         */
        module const* find_module(std::string const& name) const;

        /**
         * Imports a file into the environment's registry.
         * @param logger The logger to use to log messages.
         * @param type The type of file to find.
         * @param name The qualified name to translated into a file (e.g. 'foo::bar::baz' => '.../modules/foo/bar/baz').
         */
        void import(logging::logger& logger, find_type type, std::string const& name);

     private:
        environment(std::string name, std::string directory, compiler::settings settings);
        void populate(logging::logger& logger, std::vector<std::string> const& manifests);
        void load_environment_settings(logging::logger& logger);
        void add_modules(logging::logger& logger);
        void add_modules(logging::logger& logger, std::string const& directory);
        void add_manifests(logging::logger& logger);
        void add_manifests(logging::logger& logger, std::string const& directory, bool throw_if_missing = false);
        std::shared_ptr<ast::syntax_tree> import(logging::logger& logger, std::string const& path, compiler::module const* module = nullptr);

        std::string _name;
        compiler::settings _settings;
        compiler::registry _registry;
        evaluation::dispatcher _dispatcher;
        std::vector<std::string> _manifests;
        std::deque<module> _modules;
        std::unordered_map<std::string, module*> _module_map;
        std::unordered_map<std::string, std::shared_ptr<ast::syntax_tree>> _parsed;
    };

}}  // puppet::compiler
