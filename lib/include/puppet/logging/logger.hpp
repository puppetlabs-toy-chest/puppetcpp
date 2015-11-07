/**
 * @file
 * Declares the logger used by the compiler.
 */
#pragma once

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
     * Reads a log level from an input stream.
     * This is used in boost::lexical_cast<level>.
     * @param in The input stream.
     * @param level The returned log level.
     * @returns Returns the input stream.
     */
    std::istream& operator>>(std::istream& in, logging::level& level);

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
         * @param text The context text.
         * @param path The path of the source file.
         * @param message The message to log.
         */
        void log(logging::level level, size_t line, size_t column, std::string const& text, std::string const& path, std::string const& message);

        /**
         * Logs a message.
         * @tparam TArgs The type of the message formatting arguments.
         * @param level The log level.
         * @param format The format of the message to log.
         * @param args The format arguments.
         */
        template <typename... TArgs>
        void log(logging::level level, std::string const& format, TArgs... args)
        {
            if (!would_log(level)) {
                return;
            }
            boost::format message(format);
            log(level, 0, 0, {}, {}, message, std::forward<TArgs>(args)...);
        }

        /**
         * Logs a message.
         * @tparam TArgs The type of the message formatting arguments.
         * @param level The log level.
         * @param line The line of the source context.
         * @param column The column of the source context.
         * @param text The context text.
         * @param path The path of the source file.
         * @param format The format of the message to log.
         * @param args The format arguments.
         */
        template <typename... TArgs>
        void log(logging::level level, size_t line, size_t column, std::string const& text, std::string const& path, std::string const& format, TArgs... args)
        {
            if (!would_log(level)) {
                return;
            }
            boost::format message(format);
            log(level, line, column, text, path, message, std::forward<TArgs>(args)...);
        }

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
         * @param text The context text.
         * @param path The path of the source file.
         * @param message The message to log.
         */
        virtual void log_message(logging::level level, size_t line, size_t column, std::string const& text, std::string const& path, std::string const& message) = 0;

     private:
        void log(logging::level level, size_t line, size_t column, std::string const& text, std::string const& path, boost::format& message);

        template <typename T, typename... TArgs>
        void log(logging::level level, size_t line, size_t column, std::string const& text, std::string const& path, boost::format& message, T arg, TArgs... args)
        {
            message % std::forward<T>(arg);
            log(level, line, column, text, path, message, std::forward<TArgs>(args)...);
        }

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
         * @param text The context text.
         * @param path The path of the source file.
         * @param message The message to log.
         */
        virtual void log_message(logging::level level, size_t line, size_t column, std::string const& text, std::string const& path, std::string const& message) override;

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
     public:
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