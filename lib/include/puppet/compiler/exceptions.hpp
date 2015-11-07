/**
 * @file
 * Declares the compiler-related exceptions.
 */
#pragma once

#include "lexer/position.hpp"
#include "ast/ast.hpp"
#include "../cast.hpp"
#include <string>
#include <exception>
#include <memory>

namespace puppet { namespace compiler {

    namespace ast {
        // Forward declaration of context
        struct context;

        // Forward declaration of syntax tree
        struct syntax_tree;
    }

    /**
     * Exception for parse errors.
     */
    struct parse_exception : std::runtime_error
    {
        /**
         * Constructs a parse exception.
         * @param message The exception message.
         * @param position The token position where parsing failed.
         */
        parse_exception(std::string const& message, lexer::position position);

        /**
         * Gets the token position where parsing failed.
         * @return Returns the token position where parsing failed.
         */
        lexer::position const& position() const;

    private:
        lexer::position _position;
    };

    /**
     * Exception for evaluation errors.
     */
    struct evaluation_exception : std::runtime_error
    {
        /**
         * Constructs an evaluation exception.
         * @param message The exception message.
         */
        explicit evaluation_exception(std::string const& message);

        /**
         * Constructs an evaluation exception.
         * @param message The exception message.
         * @param context The AST context when evaluation failed.
         */
        evaluation_exception(std::string const& message, ast::context const& context);

        /**
         * Gets the context where the evaluation failed.
         * @return Returns the context where evaluation failed.
         */
        ast::context const& context() const;

     private:
        std::shared_ptr<ast::syntax_tree> _tree;
        ast::context _context;
    };

    /**
     * Exception for compilation errors.
     */
    struct compilation_exception : std::runtime_error
    {
        /**
         * Constructs a compilation exception.
         * @param message The exception message.
         * @param path The path to the input file.
         * @param line The line containing the compilation error.
         * @param column The column containing the compilation error.
         * @param text The line of text containing the compilation error.
         */
        explicit compilation_exception(std::string const& message, std::string path = std::string(), size_t line = 0, size_t column = 0, std::string text = std::string());

        /**
         * Constructs a compilation exception from a parse exception.
         * @param ex The parse exception.
         * @param path The path to the file that was parsed.
         */
        compilation_exception(parse_exception const& ex, std::string const& path);

        /**
         * Constructs a compilation exception from an evaluation exception.
         * @param ex The evaluation exception.
         */
        explicit compilation_exception(evaluation_exception const& ex);

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
     * Exception for compilation settings.
     */
    struct settings_exception : std::runtime_error
    {
        /**
         * Creates a settings exception.
         * @param message The exception message.
         */
        explicit settings_exception(std::string const& message);
    };

}}  // puppet::compiler
