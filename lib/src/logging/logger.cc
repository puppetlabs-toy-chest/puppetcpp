#include <puppet/logging/logger.hpp>
#include <boost/algorithm/string.hpp>
#include <sstream>
#include <iomanip>
// TODO: fix istty support for windows
#include <unistd.h>

using namespace std;
using boost::format;

namespace puppet { namespace logging {

    istream& operator>>(istream& in, logging::level& level)
    {
        string value;
        if (in >> value) {
            boost::algorithm::to_lower(value);
            if (value == "debug") {
                level = logging::level::debug;
                return in;
            }
            if (value == "info") {
                level = logging::level::info;
                return in;
            }
            if (value == "notice") {
                level = logging::level::notice;
                return in;
            }
            if (value == "warning") {
                level = logging::level::warning;
                return in;
            }
            if (value == "err" || value == "error") {
                level = logging::level::error;
                return in;
            }
            if (value == "alert") {
                level = logging::level::alert;
                return in;
            }
            if (value == "emerg" || value == "emergency") {
                level = logging::level::emergency;
                return in;
            }
            if (value == "crit" || value == "critical") {
                level = logging::level::critical;
                return in;
            }
        }
        throw runtime_error((boost::format("invalid log level '%1%': expected debug, info, notice, warning, error, alert, emergency, or critical.") % value).str());
    }

    ostream& operator<<(ostream& out, logging::level level)
    {
        // Keep this in sync with the definition of logging::level
        static const vector<string> strings = {
            "Debug",
            "Info",
            "Notice",
            "Warning",
            "Error",
            "Alert",
            "Emergency",
            "Critical"
        };

        size_t index = static_cast<size_t>(level);
        if (index < strings.size()) {
            out << strings[index];
        }
        return out;
    }

    logger::logger() :
        _warnings(0),
        _errors(0),
        _level(logging::level::notice),
        _failed(false)
    {
    }

    void logger::log(logging::level level, string const& message, bool error_is_failure)
    {
        if (!would_log(level)) {
           return;
        }
        log(level, 0, 0, 0, {}, {}, message, error_is_failure);
    }

    void logger::log(logging::level level, size_t line, size_t column, size_t length, string const& text, string const& path, string const& message, bool error_is_failure)
    {
        if (!would_log(level)) {
            return;
        }
        if (level == logging::level::warning) {
            ++_warnings;
        } else if (level >= logging::level::error) {
            if (error_is_failure) {
                _failed = true;
            }
            ++_errors;
        }
        log_message(level, line, column, length, text, path, message);
    }

    size_t logger::warnings() const
    {
        return _warnings;
    }

    size_t logger::errors() const
    {
        return _errors;
    }

    bool logger::failed() const
    {
        return _failed;
    }

    logging::level logger::level() const
    {
        return _level;
    }

    void logger::level(logging::level level)
    {
        _level = level;
    }

    void logger::reset()
    {
        _warnings = _errors = 0;
        _failed = false;
    }

    bool logger::would_log(logging::level level)
    {
        return static_cast<size_t>(level) >= static_cast<size_t>(_level);
    }

    void stream_logger::log_message(logging::level level, size_t line, size_t column, size_t length, string const& text, string const& path, string const& message)
    {
        ostream& stream = get_stream(level);

        // Colorize the output
        colorize(level);

        // Output the level
        stream << level << ": ";

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
            reset(level);

            // Ignore leading whitespace in the line
            size_t offset = 0;
            size_t column_offset = 0;
            for (; offset < text.size(); ++offset, ++column_offset) {
                // If the current offset into the string is not a space, break out
                if (!isspace(text[offset])) {
                    break;
                }
                // If a tab, offset the column by a tab width (3 + 1)
                if (text[offset] == '\t') {
                    column_offset += 3;
                    continue;
                }
            }

            // Write the line
            stream << "    ";
            stream.write(text.c_str() + offset, text.size() - offset);
            stream << '\n';

            // Write the "pointer" pointing at the column
            fill_n(ostream_iterator<char>(stream), column - column_offset + 3, ' ');
            colorize(logging::level::info);
            stream << "^";
            if (length > 0) {
                length -= 1; // need to offset as the caret indicator counts as a length of 1
                if (length > (text.size() - column)) {
                    length = text.size() - column;
                }
                fill_n(ostream_iterator<char>(stream), length, '~');
            }
            stream << "\n";
        }

        // Reset the colorization
        reset(level);
    }

    void stream_logger::colorize(logging::level) const
    {
        // Stream loggers do not colorize
    }

    void stream_logger::reset(logging::level) const
    {
        // Stream loggers do not colorize
    }

    ostream& console_logger::get_stream(logging::level level) const
    {
        return level >= logging::level::warning ? cerr : cout;
    }

    console_logger::console_logger() :
        _colorize_stdout(static_cast<bool>(isatty(fileno(stdout)))),
        _colorize_stderr(static_cast<bool>(isatty(fileno(stderr))))
    {
    }

    void console_logger::colorize(logging::level level) const
    {
        static const string cyan = "\33[0;36m";
        static const string green = "\33[0;32m";
        static const string hyellow = "\33[1;33m";
        static const string hred = "\33[1;31m";

        if (!should_colorize(level)) {
            return;
        }

        auto& stream = get_stream(level);

        if (level == logging::level::debug) {
            stream << cyan;
        } else if (level == logging::level::info) {
            stream << green;
        } else if (level == logging::level::warning) {
            stream << hyellow;
        } else if (level >= logging::level::error) {
            stream << hred;
        }
    }

    void console_logger::reset(logging::level level) const
    {
        static const string reset = "\33[0m";

        if (!should_colorize(level)) {
            return;
        }

        auto& stream = get_stream(level);

        if (level != logging::level::notice) {
            stream << reset;
        }
    }

    bool console_logger::should_colorize(logging::level level) const
    {
        return level >= logging::level::warning ? _colorize_stderr : _colorize_stdout;
    }

}}  // namespace puppet::logging
