#include <puppet/utility/error_reporter.hpp>
#include <iomanip>

using namespace std;
using boost::format;

namespace puppet { namespace utility {

    error_reporter::error_reporter(ostream& output) :
        _output(output),
        _errors(0),
        _warnings(0)
    {
    }

    void error_reporter::warning(std::string const& message)
    {
        log(true, std::string(), std::string(), 0, 0, message);
    }

    void error_reporter::warning_with_location(std::string const& path, std::string const& line, size_t line_number, size_t column, std::string const& message)
    {
        log(true, path, line, line_number, column, message);
    }

    void error_reporter::error(std::string const& message)
    {
        log(false, std::string(), std::string(), 0, 0, message);
    }

    void error_reporter::error_with_location(std::string const& path, std::string const& line, size_t line_number, size_t column, std::string const& message)
    {
        log(false, path, line, line_number, column, message);
    }

    void error_reporter::log(bool warning, string const& path, string const& line, size_t line_number, size_t column, boost::format& message)
    {
        log(warning, path, line, line_number, column, message.str());
    }

    void error_reporter::log(bool warning, string const& path, string const& line, size_t line_number, size_t column, string const& message)
    {
        static const string yellow = "\33[0;33m";
        static const string red = "\33[0;31m";
        static const string reset = "\33[0m";

        // TODO: don't output color codes for platforms that don't support them (also non-TTY)

        if (warning) {
            ++_warnings;
            _output << yellow;

        } else {
            ++_errors;
            _output << red;
        }

        if (!path.empty()) {
            _output << path << ":" << line_number << ":" << column << ": ";
        }
        _output << (warning ? "warning" : "error") << ": ";
        if (!message.empty()) {
            _output << message << "\n";
        } else if (!line.empty() && column > 0) {
            auto current = line[column - 1];
            if (current == '\'' || current == '\"') {
                _output << "unclosed quote";
            } else {
                _output << "unexpected character ";
                if (isprint(current)) {
                    _output << '\'' << current << '\'';
                } else {
                    _output << "0x" << setw(2) << setfill('0') << static_cast<int>(current);
                }
            }
            _output << ".\n";
        }

        if (!line.empty()) {
            _output << "    " << line << '\n';
            _output << setfill(' ') << setw(column + 5) << "^\n";
        }

        _output << reset;
    }

    size_t error_reporter::warnings() const
    {
        return _warnings;
    }

    size_t error_reporter::errors() const
    {
        return _errors;
    }

    void error_reporter::reset()
    {
        _errors = _warnings = 0;
    }

}}  // namespace puppet::parser