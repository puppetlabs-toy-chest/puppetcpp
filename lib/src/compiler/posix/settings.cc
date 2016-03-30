#include <puppet/compiler/settings.hpp>
#include <puppet/utility/filesystem/helpers.hpp>
#include <boost/filesystem.hpp>
#include <unistd.h>

using namespace std;
using namespace puppet::utility::filesystem;
namespace fs = boost::filesystem;
namespace sys = boost::system;

namespace puppet { namespace compiler {

    static string default_code_directory()
    {
        // For root or users without a HOME directory, use the global location
        auto home = home_directory();
        if (home.empty() || geteuid() == 0) {
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
