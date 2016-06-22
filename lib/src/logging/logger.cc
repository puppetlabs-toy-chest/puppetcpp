#include <puppet/logging/logger.hpp>
#include <boost/algorithm/string.hpp>
#include <sstream>
#include <iomanip>
// TODO: fix istty support for windows
#include <unistd.h>

using namespace std;
using boost::format;

namespace puppet { namespace logging {

    static const size_t MAX_BACKTRACE_COUNT = 25;

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
        _level(logging::level::notice)
    {
    }

    void logger::log(logging::level level, string const& message)
    {
        if (!would_log(level)) {
           return;
        }
        log(level, 0, 0, 0, {}, {}, message);
    }

    void logger::log(logging::level level, size_t line, size_t column, size_t length, string const& text, string const& path, string const& message)
    {
        if (!would_log(level)) {
            return;
        }
        if (level == logging::level::warning) {
            ++_warnings;
        } else if (level >= logging::level::error) {
            ++_errors;
        }
        log_message(level, line, column, length, text, path, message);
    }

    void logger::log(vector<compiler::evaluation::stack_frame> const& backtrace)
    {
        if (!would_log(logging::level::error) || backtrace.empty()) {
            return;
        }

        log_backtrace(backtrace);
    }

    size_t logger::warnings() const
    {
        return _warnings;
    }

    size_t logger::errors() const
    {
        return _errors;
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
    }

    bool logger::would_log(logging::level level)
    {
        return static_cast<size_t>(level) >= static_cast<size_t>(_level);
    }

    void stream_logger::log_message(logging::level level, size_t line, size_t column, size_t length, string const& text, string const& path, string const& message)
    {
        ostream& stream = get_stream(level);

        colorize(level);
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

        stream << message << "\n";

        if (!text.empty() && column > 0) {
            reset(level);

            // Ignore leading whitespace in the line
            // Note: this is only counting ASCII whitespace
            size_t offset = 0;
            size_t leading_spaces = 0;
            for (; offset < text.size(); ++offset, ++leading_spaces) {
                // If the current offset into the string is not a space, break out
                if (!isspace(text[offset])) {
                    break;
                }
                // If a tab, offset the column by a tab width (3 + 1)
                if (text[offset] == '\t') {
                    leading_spaces += 3;
                    continue;
                }
            }

            // Write the line starting at the whitespace offset
            stream << "  ";
            stream.write(text.c_str() + offset, text.size() - offset);
            stream << '\n';

            // Write the "pointer" pointing at the column
            fill_n(ostream_iterator<char>(stream), (leading_spaces > column ? 1 : (column - leading_spaces)) + 1, ' ');
            colorize(logging::level::info);
            stream << "^";
            if (length > 1) {
                fill_n(ostream_iterator<char>(stream), length - 1 /* caret counts as one */, '~');
            }
            stream << "\n";
        }

        reset(level);
    }

    void stream_logger::log_backtrace(vector<compiler::evaluation::stack_frame> const& backtrace)
    {
        colorize(logging::level::error);

        ostream& stream = get_stream(logging::level::error);

        stream << "  backtrace:\n";
        for (size_t i = 0; i < backtrace.size() && i < MAX_BACKTRACE_COUNT; ++i) {
            stream << "    " << backtrace[i] << '\n';
        }
        if (backtrace.size() > MAX_BACKTRACE_COUNT) {
            size_t remaining = backtrace.size() - MAX_BACKTRACE_COUNT;
            stream << "    and " << remaining << " more frame" << ((remaining != 1) ? "s\n" : "\n");
        }

        reset(logging::level::error);
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
