#include <puppet/compiler/environment.hpp>
#include <puppet/cast.hpp>
#include <boost/filesystem.hpp>

using namespace std;
namespace fs = boost::filesystem;
namespace sys = boost::system;

namespace puppet { namespace compiler {

    environment::environment(logging::logger& logger, string name, string base) :
        _logger(logger),
        _name(rvalue_cast(name)),
        _base(rvalue_cast(base))
    {
        // TODO: the modules path can come from an environment configuration file
        vector<string> module_directories = {
            (fs::path{_base} / "modules").string()
        };

        _modules = load_modules(_logger, module_directories);
    }

    logging::logger& environment::logger()
    {
        return _logger;
    }

    string const& environment::name() const
    {
        return _name;
    }

    string const& environment::base() const
    {
        return _base;
    }

    vector<string> environment::find_manifests() const
    {
        // Needed for LOG macro
        auto& logger = _logger;

        // TODO: the "manifests" directory is a configuration setting; should not be hard coded
        auto manifests_directory = fs::path{_base} / "manifests";

        LOG(debug, "searching '%1%' for manifest files.", manifests_directory.string());

        vector<string> paths;
        sys::error_code ec;
        if (!fs::is_directory(manifests_directory, ec) || ec) {
            return paths;
        }

        fs::directory_iterator it{manifests_directory};
        fs::directory_iterator end{};

        // Add all files to the
        for (; it != end; ++it) {
            if (fs::is_regular_file(it->status()) && it->path().extension() == ".pp") {
                paths.emplace_back(it->path().string());
            }
        }

        // Sort and return the paths
        sort(paths.begin(), paths.end());
        return paths;
    }

    module const* environment::find_module(string const& name) const
    {
        auto it = _modules.find(name);
        if (it == _modules.end()) {
            return nullptr;
        }
        return &it->second;
    }

}}  // namespace puppet::compiler
