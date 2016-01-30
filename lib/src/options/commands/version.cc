#include <puppet/options/commands/version.hpp>
#include <iostream>

using namespace std;
namespace po = boost::program_options;

namespace puppet { namespace options { namespace commands {

    version::version(options::parser const& parser, ostream& stream) :
        command(parser),
        _stream(stream)
    {
    }

    char const* version::name() const
    {
        return "version";
    }

    char const* version::description() const
    {
        return "Show the version information.";
    }

    char const* version::summary() const
    {
        return "This command shows the version information and exits.";
    }

    executor version::create_executor(po::variables_map const& options) const
    {
        return {
            *this,
            [this]() {
                // TODO: use a real version number
                _stream << "0.1.0" << endl;
                return EXIT_SUCCESS;
            }
        };
    }

}}}  // namespace puppet::options::commands
