/**
 * @file
 * Declares the compiler node.
 */
#pragma once

#include "module.hpp"
#include "environment.hpp"
#include "catalog.hpp"
#include "ast/ast.hpp"
#include "../facts/provider.hpp"
#include "../logging/logger.hpp"
#include <exception>
#include <functional>
#include <set>
#include <vector>
#include <memory>
#include <unordered_set>
#include <unordered_map>

namespace puppet { namespace compiler {

    namespace evaluation {
        // Forward declaration of evaluation context.
        struct context;
    }

    /**
     * Represents a compilation node.
     */
    struct node
    {
        /**
         * Constructs a compilation node.
         * @param logger The logger to use.
         * @param name The name of the node.
         * @param environment The environment for the node.
         * @param facts The facts provider for the node.
         */
        node(logging::logger& logger, std::string const& name, std::shared_ptr<compiler::environment> environment, std::shared_ptr<facts::provider> facts = nullptr);

        /**
         * Gets the logger used for logging messages.
         * @return Returns the logger used for logging messages.
         */
        logging::logger& logger();

        /**
         * Gets the display name of the node.
         * @return Returns the display name of the node.
         */
        std::string const& name() const;

        /**
         * Gets the node's environment.
         * @return Returns the node's environment.
         */
        compiler::environment& environment() const;

        /**
         * Gets the facts provider for the node.
         * @return Returns the facts provider for the node.
         */
        std::shared_ptr<facts::provider> const& facts() const;

        /**
         * Compiles manifests into a catalog for this node.
         * @return Returns the compiled catalog for the node.
         */
        catalog compile();

        /**
         * Calls the given callback for each name associated with the node.
         * @param callback The callback to call for each name.
         */
        void each_name(std::function<bool(std::string const&)> const& callback) const;

     private:
        void create_initial_resources(evaluation::context& context) const;

        logging::logger& _logger;
        std::set<std::string> _names;
        std::shared_ptr<compiler::environment> _environment;
        std::shared_ptr<facts::provider> _facts;
    };

}}  // puppet::compiler
