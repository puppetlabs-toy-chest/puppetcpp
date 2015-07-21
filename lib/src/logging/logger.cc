#include <puppet/logging/logger.hpp>
#include <sstream>
#include <iomanip>
// TODO: fix istty support for windows
#include <unistd.h>

using namespace std;
using boost::format;

namespace puppet { namespace logging {

    logger::logger() :
        _warnings(0),
        _errors(0)
    {
    }

    void logger::log(level lvl, string const& message)
    {
        log(lvl, 0, 0, {}, {}, message);
    }

    void logger::log(level lvl, size_t line, size_t column, string const& text, string const& path, string const& message)
    {
        if (lvl == level::warning) {
            ++_warnings;
        } else if (lvl >= level::error) {
            ++_errors;
        }
        log_message(lvl, line, column, text, path, message);
    }

    size_t logger::warnings() const
    {
        return _warnings;
    }

    size_t logger::errors() const
    {
        return _errors;
    }

    void logger::reset()
    {
        _warnings = _errors = 0;
    }

    void logger::log(level lvl, size_t line, size_t column, string const& text, string const& path, boost::format& message)
    {
        log(lvl, line, column, text, path, message.str());
    }

    void stream_logger::log_message(level lvl, size_t line, size_t column, string const& text, string const& path, string const& message)
    {
        ostream& stream = get_stream(lvl);

        // Colorize the output
        colorize(lvl);

        // Output the level
        if (lvl == level::debug) {
            stream << "Debug: ";
        } else if (lvl == level::info) {
            stream << "Info: ";
        } else if (lvl == level::notice) {
            stream << "Notice: ";
        } else if (lvl == level::warning) {
            stream << "Warning: ";
        } else if (lvl == level::error) {
            stream << "Error: ";
        } else if (lvl == level::alert) {
            stream << "Alert: ";
        } else if (lvl == level::emergency) {
            stream << "Emergency: ";
        } else if (lvl == level::critical) {
            stream << "Critical: ";
        }

        // If a location was given, write it out
        if (!path.empty()) {
            stream << path;
            if (line > 0) {
                stream << ":" << line;
            }
            if (column > 0) {
                stream << ":" << column;
            }
            stream << ": ";
        }

        // Output the message
        if (!message.empty()) {
            stream << message;
        }

        stream << "\n";

        // Output the offending line's text
        if (!text.empty() && column > 0) {
            stream << "    ";

            // Ignore leading whitespace in the line
            size_t offset = 0;
            for (; offset < text.size() && isspace(text[offset]); ++offset);
            stream.write(text.c_str() + offset, text.size() - offset);

            stream << '\n' << setfill(' ') << setw(column + 5 - offset) << "^\n";
        }

        // Reset the colorization
        reset(lvl);
    }

    void stream_logger::colorize(level) const
    {
        // Stream loggers do not colorize
    }

    void stream_logger::reset(level) const
    {
        // Stream loggers do not colorize
    }

    ostream& console_logger::get_stream(level lvl) const
    {
        return lvl >= level::warning ? cerr : cout;
    }

    console_logger::console_logger() :
        _colorize_stdout(static_cast<bool>(isatty(fileno(stdout)))),
        _colorize_stderr(static_cast<bool>(isatty(fileno(stderr))))
    {
    }

    void console_logger::colorize(level lvl) const
    {
        static const string cyan = "\33[0;36m";
        static const string green = "\33[0;32m";
        static const string hyellow = "\33[1;33m";
        static const string hred = "\33[1;31m";

        if (!should_colorize(lvl)) {
            return;
        }

        auto& stream = get_stream(lvl);

        if (lvl == level::debug) {
            stream << cyan;
        } else if (lvl == level::info) {
            stream << green;
        } else if (lvl == level::warning) {
            stream << hyellow;
        } else if (lvl >= level::error) {
            stream << hred;
        }
    }

    void console_logger::reset(level lvl) const
    {
        static const string reset = "\33[0m";

        if (!should_colorize(lvl)) {
            return;
        }

        auto& stream = get_stream(lvl);

        if (lvl != level::notice) {
            stream << reset;
        }
    }

    bool console_logger::should_colorize(level lvl) const
    {
        return lvl >= level::warning ? _colorize_stderr : _colorize_stdout;
    }

}}  // namespace puppet::logging
