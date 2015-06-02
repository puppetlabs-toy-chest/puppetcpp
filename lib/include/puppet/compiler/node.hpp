/**
 * @file
 * Declares the compiler node.
 */
#pragma once

#include "../logging/logger.hpp"
#include "../runtime/catalog.hpp"
#include "environment.hpp"
#include <exception>

namespace puppet { namespace compiler {

    /**
     * Exception for compilation errors.
     */
    struct compilation_exception : std::runtime_error
    {
        /**
         * Constructs a parse exception.
         * @param message The exception message.
         * @param path The path to the input file.
         * @param line The line containing the compilation error.
         * @param column The column containing the compilation error.
         * @param text The line of text containing the compilation error.
         */
        explicit compilation_exception(std::string const& message, std::string path = std::string(), size_t line = 0, size_t column = 0, std::string text = std::string());

        /**
         * Gets the path of the input file.
         * @return Returns the path of the input file.
         */
        std::string const& path() const;

        /**
         * Gets the line of the compilation error.
         * @return Returns the line of the compilation error.
         */
        size_t line() const;

        /**
         * Gets the column of the compilation error.
         * @return Returns the column of the compilation error.
         */
        size_t column() const;

        /**
         * Gets the line of text containing the compilation error.
         * @return Returns the line of text containing the compilation error.
         */
        std::string const& text() const;

     private:
        std::string _path;
        size_t _line;
        size_t _column;
        std::string _text;
    };

    /**
     * Represents a compilation node.
     */
    struct node
    {
        /**
         * Constructs a compilation node.
         * @param name The name of the node.
         * @param environment The environment for the node.
         */
        node(std::string name, compiler::environment& environment);

        /**
         * Gets the name of the node.
         * @return Returns the name of the node.
         */
        std::string const& name() const;

        /**
         * Gets the node's environment.
         * @return Returns the node's environment.
         */
        compiler::environment& environment();

        /**
         * Compiles a manifest into a catalog for this node.
         * @param logger The logger to use.
         * @param path The path of the manifest file being compiled.
         * @return Returns the compiled catalog for the node.
         */
        runtime::catalog compile(logging::logger& logger, std::string const& path);

     private:
        std::string _name;
        compiler::environment& _environment;
    };

}}  // puppet::compiler
