/**
 * @file
 * Declares the logger used by the compiler.
 */
#pragma once

#include "../compiler/evaluation/stack_frame.hpp"
#include <boost/format.hpp>
#include <string>
#include <iostream>
#include <functional>

namespace puppet { namespace logging {

    /**
     * Macro for logging messages.
     * @param lvl The log level name.
     * @param ... The arguments to the logger.
     */
    #define LOG(lvl, ...) \
    if (logger.would_log(puppet::logging::level::lvl)) { \
        logger.log(puppet::logging::level::lvl, ##__VA_ARGS__); \
    }

    /**
     * Represents the log level.
     */
    enum class level
    {
        /**
         * The debug log level.
         */
        debug,
        /**
         * The info log level.
         */
        info,
        /**
         * The notice log level.
         */
        notice,
        /**
         * The warning log level.
         */
        warning,
        /**
         * The error log level.
         */
        error,
        /**
         * The alert log level.
         */
        alert,
        /**
         * The emergency log level.
         */
        emergency,
        /**
         * The critical error log level.
         */
        critical
    };

    /**
     * Produces the printed representation of the logging level.
     * @param out The stream to write.
     * @param level The logging level to print.
     * @return Returns the stream after writing to it.
     */
    std::ostream& operator<<(std::ostream& out, logging::level level);

    /**
     * Implements the base logger.
     */
    struct logger
    {
        /**
         * Constructs a logger.
         */
        logger();

        /**
         * Logs a message.
         * @param level The log level.
         * @param message The message to log.
         */
        void log(logging::level level, std::string const& message);

        /**
         * Logs a message with source context.
         * @param level The log level.
         * @param line The line of the source context.
         * @param column The column of the source context.
         * @param length The length of the source to highlight.
         * @param text The context text.
         * @param path The path of the source file.
         * @param message The message to log.
         */
        void log(logging::level level, size_t line, size_t column, size_t length, std::string const& text, std::string const& path, std::string const& message);

        /**
         * Logs a message.
         * @tparam Args The type of the message formatting arguments.
         * @param level The log level.
         * @param format The format of the message to log.
         * @param args The format arguments.
         */
        template <typename... Args>
        void log(logging::level level, std::string const& format, Args&&... args)
        {
            if (!would_log(level)) {
                return;
            }
            // This magic calls `boost::format::operator %` for every argument given without having to do recursion
            boost::format message{ format };
            static_cast<void>(std::initializer_list<int>{(message % std::forward<Args>(args), 0)...});
            log(level, 0, 0, 0, {}, {}, message.str());
        }

        /**
         * Logs a message.
         * @tparam Args The type of the message formatting arguments.
         * @param level The log level.
         * @param line The line of the source context.
         * @param column The column of the source context.
         * @param length The length of the source to highlight.
         * @param text The context text.
         * @param path The path of the source file.
         * @param format The format of the message to log.
         * @param args The format arguments.
         */
        template <typename... Args>
        void log(logging::level level, size_t line, size_t column, size_t length, std::string const& text, std::string const& path, std::string const& format, Args&&... args)
        {
            if (!would_log(level)) {
                return;
            }
            // This magic calls `boost::format::operator %` for every argument given without having to do recursion
            boost::format message{ format };
            static_cast<void>(std::initializer_list<int>{(message % std::forward<Args>(args), 0)...});
            log(level, line, column, length, text, path, message.str());
        }

        /**
         * Logs a backtrace at error level.
         * @param backtrace The backtrace to log.
         */
        void log(std::vector<compiler::evaluation::stack_frame> const& backtrace);

        /**
         * Gets the number of warnings logged.
         * @return Returns the number of warnings logged.
         */
        size_t warnings() const;

        /**
         * Gets the number of errors logged.
         * @return Returns the number of errors logged.
         */
        size_t errors() const;

        /**
         * Gets the current logging level.
         * @return Returns the current logging level.
         */
        logging::level level() const;

        /**
         * Sets the logging level.
         * @param level The new logging level.
         */
        void level(logging::level level);

        /**
         * Resets the error and warning counts.
         */
        void reset();

        /**
         * Determins if the given log level would be logged.
         * @param level The logging level to check.
         * @return Returns true if the logger would log for the given level or false if not.
         */
        bool would_log(logging::level level);

    protected:
        /**
         * Logs a message.
         * @param level The log level.
         * @param line The line of the source context.
         * @param column The column of the source context.
         * @param length The length of the source to highlight.
         * @param text The context text.
         * @param path The path of the source file.
         * @param message The message to log.
         */
        virtual void log_message(logging::level level, size_t line, size_t column, size_t length, std::string const& text, std::string const& path, std::string const& message) = 0;

        /**
         * Logs a backtrace.
         * @param backtrace The backtrace to log.
         */
        virtual void log_backtrace(std::vector<compiler::evaluation::stack_frame> const& backtrace) = 0;

     private:
        size_t _warnings;
        size_t _errors;
        logging::level _level;
    };

    /**
     * Base type for stream loggers.
     */
    struct stream_logger : logger
    {
     protected:
        /**
         * Logs a message.
         * @param level The log level.
         * @param line The line of the source context.
         * @param column The column of the source context.
         * @param length The length of the source to highlight.
         * @param text The context text.
         * @param path The path of the source file.
         * @param message The message to log.
         */
        void log_message(logging::level level, size_t line, size_t column, size_t length, std::string const& text, std::string const& path, std::string const& message) override;

        /**
         * Logs a backtrace.
         * @param backtrace The backtrace to log.
         */
        void log_backtrace(std::vector<compiler::evaluation::stack_frame> const& backtrace) override;

        /**
         * Gets the stream to log to based on the message's log level.
         * @param level The log level.
         * @return Returns the stream to log to.
         */
        virtual std::ostream& get_stream(logging::level level) const = 0;

        /**
         * Colorizes for the given log level.
         * @param level The log level to colorize for.
         */
        virtual void colorize(logging::level level) const;

        /**
         * Resets the colorization.
         * @param level The log level that was colorized.
         */
        virtual void reset(logging::level level) const;
    };

    /**
     * Implements a logger that logs to a console.
     */
    struct console_logger : stream_logger
    {
        /**
         * Constructs a console logger.
         */
        console_logger();

     protected:
        /**
         * Gets the stream to log to based on the message's log level.
         * @param level The log level.
         * @return Returns the stream to log to.
         */
        virtual std::ostream& get_stream(logging::level level) const override;

        /**
         * Colorizes for the given log level.
         * @param level The log level to colorize for.
         */
        virtual void colorize(logging::level level) const override;

        /**
         * Resets the colorization.
         * @param level The log level that was colorized.
         */
        virtual void reset(logging::level level) const override;

     private:
        bool should_colorize(logging::level level) const;
        bool _colorize_stdout;
        bool _colorize_stderr;
    };

}}  // namespace puppet::logger
