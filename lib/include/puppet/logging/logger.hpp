/**
 * @file
 * Declares the logger used by the compiler.
 */
#pragma once

#include <boost/format.hpp>
#include <string>
#include <ostream>

namespace puppet { namespace logging {

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
         * The warning log level.
         */
        warning,
        /**
         * The error log level.
         */
        error,
        /**
         * The fatal error log level.
         */
        fatal
    };

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
         * @param lvl The log level.
         * @param message The message to log.
         */
        void log(level lvl, std::string const& message);

        /**
         * Logs a message with source context.
         * @param lvl The log level.
         * @param line The line of the source context.
         * @param column The column of the source context.
         * @param text The context text.
         * @param path The path of the source file.
         * @param message The message to log.
         */
        void log(level lvl, size_t line, size_t column, std::string const& text, std::string const& path, std::string const& message);

        /**
         * Logs a message.
         * @tparam TArgs The type of the message formatting arguments.
         * @param lvl The log level.
         * @param format The format of the message to log.
         * @param args The format arguments.
         */
        template <typename... TArgs>
        void log(level lvl, std::string const& format, TArgs... args)
        {
            boost::format message(format);
            log(lvl, 0, 0, {}, {}, message, std::forward<TArgs>(args)...);
        }

        /**
         * Logs a message.
         * @tparam TArgs The type of the message formatting arguments.
         * @param lvl The log level.
         * @param line The line of the source context.
         * @param column The column of the source context.
         * @param text The context text.
         * @param path The path of the source file.
         * @param format The format of the message to log.
         * @param args The format arguments.
         */
        template <typename... TArgs>
        void log(level lvl, size_t line, size_t column, std::string const& text, std::string const& path, std::string const& format, TArgs... args)
        {
            boost::format message(format);
            log(lvl, line, column, text, path, message, std::forward<TArgs>(args)...);
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
         * Resets the error and warning counts.
         */
        void reset();

     protected:
        /**
         * Logs a message.
         * @param lvl The log level.
         * @param line The line of the source context.
         * @param column The column of the source context.
         * @param text The context text.
         * @param path The path of the source file.
         * @param message The message to log.
         */
        virtual void log_message(level lvl, size_t line, size_t column, std::string const& text, std::string const& path, std::string const& message) = 0;

     private:
        void log(level lvl, size_t line, size_t column, std::string const& text, std::string const& path, boost::format& message);

        template <typename T, typename... TArgs>
        void log(level lvl, size_t line, size_t column, std::string const& text, std::string const& path, boost::format& message, T arg, TArgs... args)
        {
            message % std::forward<T>(arg);
            log(lvl, line, column, text, path, message, std::forward<TArgs>(args)...);
        }

        size_t _warnings;
        size_t _errors;
    };

    /**
     * Implements a logger that logs to the a stream.
     */
    struct stream_logger : logger
    {
        /**
         * Constructs a stream logger.
         * @param out The output stream to log to.
         */
        explicit stream_logger(std::ostream& out);

     protected:
        /**
         * Logs a message.
         * @param lvl The log level.
         * @param line The line of the source context.
         * @param column The column of the source context.
         * @param text The context text.
         * @param path The path of the source file.
         * @param message The message to log.
         */
        virtual void log_message(level lvl, size_t line, size_t column, std::string const& text, std::string const& path, std::string const& message) override;

     private:
        std::ostream& _out;
    };

}}  // namespace puppet::logger