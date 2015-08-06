/**
 * @file
 * Declares the compiler-related exceptions.
 */
#pragma once

#include "../lexer/position.hpp"
#include <string>
#include <exception>

namespace puppet { namespace compiler {

    /**
     * Exception for parse errors.
     */
    struct parse_exception : std::runtime_error
    {
        /**
         * Constructs a parse exception.
         * @param position The token position where parsing failed.
         * @param message The exception message.
         */
        parse_exception(lexer::position position, std::string const& message);

        /**
         * Gets the token position where parsing failed.
         * @return Returns the token position where parsing failed.
         */
        lexer::position const& position() const;

    private:
        lexer::position _position;
    };

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
