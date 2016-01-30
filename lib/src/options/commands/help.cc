#include <puppet/options/commands/help.hpp>
#include <puppet/options/parser.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

using namespace std;
namespace po = boost::program_options;

namespace puppet { namespace options { namespace commands {

    static char const* const COMMAND_OPTION = "command";

    static void word_wrap(ostream& stream, string const& message, size_t width = 80)
    {
        // Split the string on any whitespace
        size_t remaining = width;
        boost::split_iterator<string::const_iterator> end;
        for (auto word = boost::make_split_iterator(message, boost::token_finder(boost::is_space(), boost::token_compress_on)); word != end; ++word) {
            size_t length = std::distance(word->begin(), word->end());

            // Check for a "new paragraph" indicator
            if (length == 3 && *word == boost::as_literal("<p>")) {
                stream << "\n\n";
                remaining = width;
                continue;
            }
            // If the word (and a space) goes over the line width, write it out on a new line
            if (remaining < length + 1) {
                stream << '\n';
                remaining = length > width ? 0 : (width - length);
            } else if (remaining < width) {
                // A word was written to this line already, so output a space before outputting the word
                stream << ' ';
                remaining -= length + 1;
            } else {
                // Otherwise, start of a line, don't write out a space
                remaining -= length;
            }
            stream.write(&*word->begin(), length);
        }
    }

    help::help(options::parser const& parser, ostream& stream) :
        command(parser),
        _stream(stream)
    {
    }

    char const* help::name() const
    {
        return "help";
    }

    char const* help::description() const
    {
        return "Display help information.";
    }

    char const* help::summary() const
    {
        return "If a command is given, the help for that command will be displayed.";
    }

    char const* help::arguments() const
    {
        return "[command]";
    }

    po::options_description help::create_hidden_options() const
    {
        po::options_description options;
        options.add_options()
            (COMMAND_OPTION, po::value<string>())
            ;
        return options;
    }

    po::positional_options_description help::create_positional_options() const
    {
        po::positional_options_description options;
        options.add(COMMAND_OPTION, 1);
        return options;
    }

    executor help::create_executor(po::variables_map const& options) const
    {
        options::parser const* parser = &this->parser();
        options::command const* command = nullptr;
        if (options.count(COMMAND_OPTION)) {
            auto& name = options[COMMAND_OPTION].as<string>();
            command = parser->find(name);
            if (!command) {
                throw option_exception((boost::format("'%1%' is not a recognized command.") % name).str());
            }
        }
        return {
            *this,
            [command, this]() {
                if (!command) {
                    print_help();
                } else {
                    print_command_help(*command);
                }
                return EXIT_SUCCESS;
            }
        };
    }

    void help::print_help() const
    {
        _stream <<
            "\n"
            "Usage: puppetcpp <command> [options]\n"
            "\n"
            "Compiles Puppet manifests into Puppet catalogs.\n"
            "\n"
            "Commands:\n"
            "\n";

        // Find the maximum width of all command names
        size_t max_width = 0;
        auto flags = _stream.setf(ios::left);
        _stream << left;
        parser().each([&](options::command const& command) {
            string name = command.name();
            if (name.size() > max_width) {
                max_width = name.size();
            }
            return true;
        });
        _stream.setf(flags);

        // Print the commands and their descriptions
        parser().each([&](options::command const& command) {
            _stream << "  ";
            auto width = _stream.width(max_width + 2);
            auto fill = _stream.fill(' ');
            _stream << command.name();
            _stream.width(width);
            _stream.fill(fill);
            _stream << command.description() << "\n";
            return true;
        });

        _stream << "\n";
        _stream << "Run 'puppetcpp help <command>' for more information on a command.\n";
    }

    void help::print_command_help(options::command const& command) const
    {
        auto options = command.create_options();

        _stream << "\nUsage: puppetcpp " << command.name();
        if (!options.options().empty()) {
            _stream << " [options]";
        }

        string arguments = command.arguments();
        if (!arguments.empty()) {
            _stream << " " << arguments;
        }
        _stream << "\n\n";
        word_wrap(_stream, command.description());
        _stream << "\n\n";
        if (!options.options().empty()) {
            _stream << "Options:\n\n" << options << "\n";
        }
        word_wrap(_stream, command.summary());
        _stream << "\n";
    }

}}}  // namespace puppet::options::commands
