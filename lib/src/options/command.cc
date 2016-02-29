#include <puppet/options/command.hpp>
#include <puppet/options/parser.hpp>
#include <boost/format.hpp>

using namespace std;
namespace po = boost::program_options;

namespace puppet { namespace options {

    command::command(options::parser const& parser) :
        _parser(parser)
    {
    }

    char const* command::arguments() const
    {
        return "";
    }

    options::parser const& command::parser() const
    {
        return _parser;
    }

    executor command::parse(vector<string> const& arguments) const
    {
        po::variables_map variables;

        auto options = create_options();
        auto hidden = create_hidden_options();
        auto positional = create_positional_options();

        po::options_description all_options;
        all_options.add(options).add(hidden);

        try {
            // Store the options
            po::store(
                po::command_line_parser(arguments).
                    style(po::command_line_style::unix_style & ~po::command_line_style::allow_guessing).
                    options(all_options).
                    positional(positional).
                    run(),
                variables
            );

            // Notify the callbacks
            po::notify(variables);
        } catch (po::too_many_positional_options_error const&) {
            if (positional.max_total_count() == 0) {
                throw option_exception((boost::format("the '%1%' command does not accept arguments.") % name()).str(), this);
            }
            throw option_exception((boost::format("the '%1%' command expects at most %2% arguments.") % name() % positional.max_total_count()).str(), this);
        } catch (po::unknown_option const& ex) {
            throw option_exception((boost::format("unrecognized option '%1%' for command '%2%'.") % ex.get_option_name() % name()).str(), this);
        } catch (po::error const& ex) {
            throw option_exception(ex.what(), this);
        } catch (runtime_error const& ex) {
            throw option_exception(ex.what(), this);
        }
        return create_executor(variables);
    }

    po::options_description command::create_options() const
    {
        return { "" };
    }

    po::options_description command::create_hidden_options() const
    {
        return { "" };
    }

    po::positional_options_description command::create_positional_options() const
    {
        return {};
    }

    logging::level command::get_level(po::variables_map const& options) const
    {
        // Check for conflicting options
        if ((options.count(DEBUG_OPTION) + options.count(VERBOSE_OPTION) + (options[LOG_LEVEL_OPTION].defaulted() ? 0 : 1)) > 1) {
            throw option_exception((boost::format("%1%, %2%, and %3% options conflict: please specify only one.") % DEBUG_OPTION % VERBOSE_OPTION % LOG_LEVEL_OPTION).str(), this);
        }

        // Override the log level for debug/verbose
        if (options.count(DEBUG_OPTION)) {
            return logging::level::debug;
        }
        if (options.count(VERBOSE_OPTION)) {
            return logging::level::info;
        }
        return options[LOG_LEVEL_OPTION].as<logging::level>();
    }

    boost::optional<bool> command::get_colorization(po::variables_map const& options) const
    {
        if (options.count(COLOR_OPTION) && options.count(NO_COLOR_OPTION)) {
            throw option_exception((boost::format("%1% and %2% options conflict: please specify only one.") % COLOR_OPTION % NO_COLOR_OPTION).str(), this);
        }
        if (!options.count(COLOR_OPTION) && !options.count(NO_COLOR_OPTION)) {
            return boost::none;
        }
        return options.count(COLOR_OPTION) > 0;
    }

    char const* const command::COLOR_OPTION          = "color";
    char const* const command::COLOR_DESCRIPTION     = "Force color output on platforms that support colorized output.";
    char const* const command::DEBUG_OPTION          = "debug";
    char const* const command::DEBUG_OPTION_FULL     = "debug,d";
    char const* const command::DEBUG_DESCRIPTION     = "Enable debug output.";
    char const* const command::HELP_OPTION           = "help";
    char const* const command::HELP_DESCRIPTION      = "Display command help.";
    char const* const command::LOG_LEVEL_OPTION      = "log-level";
    char const* const command::LOG_LEVEL_OPTION_FULL = "log-level,l";
    char const* const command::LOG_LEVEL_DESCRIPTION = "Set logging level.\nSupported levels: debug, info, notice, warning, error, alert, emergency, critical.";
    char const* const command::NO_COLOR_OPTION       = "no-color";
    char const* const command::NO_COLOR_DESCRIPTION  = "Disable color output.";
    char const* const command::VERBOSE_OPTION        = "verbose";
    char const* const command::VERBOSE_DESCRIPTION   = "Enable verbose output (info level).";

}}  // namespace puppet::options
