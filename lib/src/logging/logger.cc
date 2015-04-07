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


    stream_logger::stream_logger(ostream& out) :
        _out(out)
    {
    }

    void stream_logger::log_message(level lvl, size_t line, size_t column, string const& text, string const& path, string const& message)
    {
        static const string cyan = "\33[0;36m";
        static const string green = "\33[0;32m";
        static const string hyellow = "\33[1;33m";
        static const string hred = "\33[1;31m";
        static const string reset = "\33[0m";

        // TODO: don't output color codes for platforms that don't support them (also non-TTY)
        if (lvl == level::debug) {
            _out << cyan;
        } else if (lvl == level::info) {
            _out << green;
        } else if (lvl == level::warning) {
            _out << hyellow;
        } else if (lvl >= level::error) {
            _out << hred;
        }

       // Output the level
        if (lvl == level::debug) {
            _out << "Debug: ";
        } else if (lvl == level::info) {
            _out << "Info: ";
        } else if (lvl == level::notice) {
            _out << "Notice: ";
        } else if (lvl == level::warning) {
            _out << "Warning: ";
        } else if (lvl == level::error) {
            _out << "Error: ";
        } else if (lvl == level::alert) {
            _out << "Alert: ";
        } else if (lvl == level::emergency) {
            _out << "Emergency: ";
        } else if (lvl == level::critical) {
            _out << "Critical: ";
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

        // Reset unless the level was notice (no color)
        if (lvl != level::notice) {
            _out << reset;
        }
    }

}}  // namespace puppet::logging
