#include <puppet/options/parser.hpp>
#include <puppet/options/commands/help.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
namespace po = boost::program_options;

namespace puppet { namespace options {

    option_exception::option_exception(string const& message, options::command const* command) :
        runtime_error(message),
        _command(command)
    {
    }

    options::command const* option_exception::command() const
    {
        return _command;
    }

    command const* parser::find(std::string const& name) const
    {
        auto it = _commands.find(name);
        if (it == _commands.end()) {
            return nullptr;
        }
        return it->second.get();
    }

    void parser::each(function<bool(command const&)> const& callback) const
    {
        for (auto const& kvp : _commands) {
            if (!callback(*kvp.second)) {
                return;
            }
        }
    }

    executor parser::parse(vector<string> const& arguments) const
    {
        string command_name;
        vector<string> command_arguments;
        if (arguments.empty()) {
            // If a command wasn't supplied, treat as a help command if supported
            command_name = commands::help(*this).name();
            if (!find(command_name)) {
                throw option_exception("a command is required.");
            }
        } else {
            if (boost::starts_with(arguments.front(), "-")) {
                // Strip and treat as the command if it was found
                command_name = boost::trim_left_copy_if(arguments.front(), boost::is_any_of("-"));
                if (!find(command_name)) {
                    throw option_exception((boost::format("unrecognized option '%1%'.") % arguments.front()).str());
                }
            } else {
                command_name = arguments[0];
            }
            command_arguments.reserve(arguments.size() - 1);
            command_arguments.assign(arguments.begin() + 1, arguments.end());
        }

        auto command = find(command_name);
        if (!command) {
            throw option_exception((boost::format("unrecognized command '%1%'.") % command_name).str());
        }
        return command->parse(command_arguments);
    }

}}  // namespace puppet::options
