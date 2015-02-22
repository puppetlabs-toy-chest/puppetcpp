/**
 * @file
 * Declares the error reporter used in the compiler.
 */
#pragma once

#include <boost/format.hpp>
#include <string>
#include <fstream>

namespace puppet { namespace utility {

    /**
     * Implements the error reporter.
     */
    struct error_reporter
    {
        /**
         * Constructs an error reporter with the given stream to output to.
         * @param output The output stream to write errors and warnings to.
         */
        explicit error_reporter(std::ostream& output);

        /**
         * Logs a compilation warning with the given message.
         * @param message The warning message.
         */
        void warning(std::string const& message);

        /**
         * Logs a compilation warning with the given path, line, line number, column, and message.
         * @param path The path of the file that generated the warning.
         * @param line The line of text that generated the warning.
         * @param line_number The line number of the warning.
         * @param column The column number of the warning.
         * @param message The warning message.
         */
        void warning_with_location(std::string const& path, std::string const& line, size_t line_number, size_t column, std::string const& message);

        /**
         * Logs a formatted warning message.
         * @tparam TArgs The types of the arguments to format.
         * @param format The warning message format.
         * @param args The arguments to the format message.
         */
        template <typename... TArgs>
        void warning(std::string const& format, TArgs... args)
        {
            boost::format message(format);
            log(true, std::string(), std::string(), 0, 0, message, std::forward<TArgs>(args)...);
        }

        /**
         * Logs a formatted warning message with the given path, line, line number, column.
         * @tparam TArgs The types of the arguments to format.
         * @param path The path of the file that generated the warning.
         * @param line The line of text that generated the warning.
         * @param line_number The line number of the warning.
         * @param column The column number of the warning.
         * @param format The warning message format.
         * @param args The arguments to the format message.
         */
        template <typename... TArgs>
        void warning_with_location(std::string const& path, std::string const& line, size_t line_number, size_t column, std::string const& format, TArgs... args)
        {
            boost::format message(format);
            log(true, path, line, line_number, column, message, std::forward<TArgs>(args)...);
        }

        /**
         * Logs a compilation error with the given message.
         * @param message The error message.
         */
        void error(std::string const& message);

        /**
         * Logs a compilation error with the given path, line, line number, column, and message.
         * @param path The path of the file that generated the error.
         * @param line The line of text that generated the error.
         * @param line_number The line number of the error.
         * @param column The column number of the error.
         * @param message The error message.
         */
        void error_with_location(std::string const& path, std::string const& line, size_t line_number, size_t column, std::string const& message);

        /**
         * Logs a formatted error message.
         * @tparam TArgs The types of the arguments to format.
         * @param format The error message format.
         * @param args The arguments to the format message.
         */
        template <typename... TArgs>
        void error(std::string const& format, TArgs... args)
        {
            boost::format message(format);
            log(false, std::string(), std::string(), 0, 0, message, std::forward<TArgs>(args)...);
        }

        /**
         * Logs a formatted error message with the given path, line, line number, column.
         * @tparam TArgs The types of the arguments to format.
         * @param path The path of the file that generated the error.
         * @param line The line of text that generated the error.
         * @param line_number The line number of the error.
         * @param column The column number of the error.
         * @param format The error message format.
         * @param args The arguments to the format message.
         */
        template <typename... TArgs>
        void error_with_location(std::string const& path, std::string const& line, size_t line_number, size_t column, std::string const& format, TArgs... args)
        {
            boost::format message(format);
            log(false, path, line, line_number, column, message, std::forward<TArgs>(args)...);
        }

        /**
         * Gets the number of warnings reported by the reporter.
         * @return Returns the number of warnings reported.
         */
        size_t warnings() const;

        /**
         * Gets the number of errors reported by the reporter.
         * @return Returns the number of errors reported.
         */
        size_t errors() const;

    private:
        void log(bool warning, std::string const& path, std::string const& line, size_t line_number, size_t column, std::string const& message);
        void log(bool warning, std::string const& path, std::string const& line, size_t line_number, size_t column, boost::format& message);

        template <typename T, typename... TArgs>
        void log(bool warning, std::string const& path, std::string const& line, size_t line_number, size_t column, boost::format& message, T arg, TArgs... args)
        {
            message % arg;
            log(warning, path, line, line_number, column, message, std::forward<TArgs>(args)...);
        }

        std::ostream& _output;
        size_t _errors;
        size_t _warnings;
    };

}}  // namespace puppet::parser