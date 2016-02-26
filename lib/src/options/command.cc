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

}}  // namespace puppet::options
