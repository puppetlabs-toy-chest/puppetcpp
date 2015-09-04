/**
 * @file
 * Declares the compilation context.
 */
#pragma once

#include "exceptions.hpp"
#include "../logging/logger.hpp"
#include "../lexer/position.hpp"
#include "../ast/syntax_tree.hpp"
#include <string>
#include <fstream>
#include <memory>

namespace puppet { namespace compiler {

    // Forward declaration of node
    struct node;

    /**
     * Represents a compilation context.
     */
    struct context
    {
        /**
         * Constructs a compilation context.
         * @param path The path to the file being compiled.
         * @param node The compilation node.
         * @param parse True if the file should be parsed or false if not.
         */
        context(std::shared_ptr<std::string> path, compiler::node& node, bool parse = true);

        /**
         * Gets the path of the file being compiled.
         * @return Returns the path of the file being compiled.
         */
        std::shared_ptr<std::string> const& path() const;

        /**
         * Gets the syntax tree that was parsed.
         * @return Returns the syntax tree that was parsed.
         */
        ast::syntax_tree const& tree() const;

        /**
         * Gets the current compilation node.
         * @return Returns the current compilation node.
         */
        compiler::node& node();

        /**
         * Writes a message to the log with the given position.
         * @param level The logging level.
         * @param position The position of the message.
         * @param message The message.
         */
        void log(logging::level level, lexer::position const& position, std::string const& message);

        /**
         * Creates a compilation exception for the given position and message.
         * @param position The position of the error.
         * @param message The error message.
         * @return Returns a compilation exception for the given position and message.
         */
        compilation_exception create_exception(lexer::position const& position, std::string const& message);

     private:
        std::ifstream _stream;
        std::shared_ptr<std::string> _path;
        ast::syntax_tree _tree;
        compiler::node& _node;
    };

}}  // puppet::compiler
