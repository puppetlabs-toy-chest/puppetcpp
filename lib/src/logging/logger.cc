#include <puppet/logging/logger.hpp>
#include <sstream>
#include <iomanip>

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
        } else if (lvl == level::error || lvl == level::fatal) {
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


    stream_logger::stream_logger(ostream& out) :
        _out(out)
    {
    }

    void stream_logger::log_message(level lvl, size_t line, size_t column, string const& text, string const& path, string const& message)
    {
        static const string cyan = "\33[0;36m";
        static const string green = "\33[0;32m";
        static const string yellow = "\33[0;33m";
        static const string red = "\33[0;31m";
        static const string reset = "\33[0m";

        // TODO: don't output color codes for platforms that don't support them (also non-TTY)
        if (lvl == level::debug) {
            _out << cyan;
        } else if (lvl == level::info) {
            _out << green;
        } else if (lvl == level::warning) {
            _out << yellow;
        } else if (lvl == level::error || lvl == level::fatal) {
            _out << red;
        }

        // If a location was given, write it out
        if (!path.empty()) {
            _out << path;
            if (line > 0) {
                _out << ":" << line;
            }
            if (column > 0) {
                _out << ":" << column;
            }
            _out << ": ";
        }

        // Output the level
        if (lvl == level::debug) {
            _out << "debug: ";
        } else if (lvl == level::info) {
            _out << "info: ";
        } else if (lvl == level::warning) {
            _out << "warning: ";
        } else if (lvl == level::error) {
            _out << "error: ";
        } else if (lvl == level::fatal) {
            _out << "fatal: ";
        }

        // Output the message
        if (!message.empty()) {
            _out << message;
        }
        _out << "\n";

        // Output the offending line's text
        if (!text.empty() && column > 0) {
            _out << "    " << text << '\n';
            _out << setfill(' ') << setw(column + 5) << "^\n";
        }

        _out << reset;
    }

}}  // namespace puppet::logging