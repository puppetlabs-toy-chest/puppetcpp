#include <puppet/options/defaults.hpp>
#include <boost/filesystem.hpp>
#include <unistd.h>

using namespace std;
namespace fs = boost::filesystem;
namespace sys = boost::system;

namespace puppet { namespace options {

    string defaults::code_directory()
    {
        auto home = getenv("HOME");

        // For root or users without a HOME directory, use the global location
        if (!home || geteuid() == 0) {
            return "/etc/puppetlabs/code";
        }

        // Otherwise, use the local directory
        return (fs::path(home) / ".puppetlabs" / "etc" / "code").string();
    }

    vector<string> defaults::environment_directories()
    {
        vector<string> directories = {
            "$codedir/environments",
        };
        return directories;
    }

    vector<string> defaults::module_directories()
    {
        vector<string> directories = {
            "$codedir/modules",
            "/opt/puppetlabs/puppet/modules",
        };
        return directories;
    }

}}  // namespace puppet::options
