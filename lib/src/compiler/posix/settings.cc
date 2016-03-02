#include <puppet/compiler/settings.hpp>
#include <boost/filesystem.hpp>
#include <unistd.h>

using namespace std;
namespace fs = boost::filesystem;
namespace sys = boost::system;

namespace puppet { namespace compiler {

    static string default_code_directory()
    {
        auto home = getenv("HOME");

        // For root or users without a HOME directory, use the global location
        if (!home || geteuid() == 0) {
            return "/etc/puppetlabs/code";
        }

        // Otherwise, use the local directory
        return (fs::path(home) / ".puppetlabs" / "etc" / "code").string();
    }

    settings::settings()
    {
        set(base_module_path, "$codedir/modules:/opt/puppetlabs/puppet/modules");
        set(code_directory, default_code_directory());
        set(environment, "production");
        set(environment_path, "$codedir/environments");
        set(manifest, "manifests");
        set(module_path, "modules:$basemodulepath");
    }

}}  // namespace puppet::options
