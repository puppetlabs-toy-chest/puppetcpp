#include <puppet/compiler/module.hpp>
#include <puppet/cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <regex>

using namespace std;
namespace fs = boost::filesystem;
namespace sys = boost::system;

namespace puppet { namespace compiler {

    module::module(logging::logger& logger, string name, string base) :
        _logger(logger),
        _name(rvalue_cast(name)),
        _base(rvalue_cast(base))
    {
    }

    logging::logger& module::logger()
    {
        return _logger;
    }

    string const& module::name() const
    {
        return _name;
    }

    string const& module::base() const
    {
        return _base;
    }

    string module::find_manifest(string const& name) const
    {
        fs::path path = _base;
        path /= "manifests";

        if (name.empty()) {
            path /= "init";
        } else {
            boost::split_iterator<string::const_iterator> end;
            for (auto it = boost::make_split_iterator(name, boost::first_finder("::", boost::is_equal())); it != end; ++it) {
                path.append(it->begin(), it->end());
            }
        }

        path.replace_extension(".pp");

        sys::error_code ec;
        if (fs::is_regular_file(path, ec)) {
            return path.string();
        }
        return string();
    }

    bool module::is_valid_name(string const& name)
    {
        static const regex valid_name{"^[a-z][a-z0-9_]*$"};
        static const vector<string> invalid_names = {
            "and",
            "attr",
            "case",
            "class",
            "default",
            "define",
            "else",
            "elsif",
            "false",
            "function",
            "if",
            "in",
            "inherits",
            "node",
            "or",
            "private",
            "true",
            "type",
            "undef",
            "unless"
        };

        if (!regex_match(name, valid_name)) {
            return false;
        }

        if (find(invalid_names.begin(), invalid_names.end(), name) != invalid_names.end()) {
            return false;
        }
        return true;
    }

    unordered_map<string, module> load_modules(logging::logger& logger, vector<string> const& directories)
    {
        unordered_map<string, module> modules;

        for (auto const& directory : directories) {

            sys::error_code ec;
            if (!fs::is_directory(directory, ec) || ec) {
                LOG(debug, "base module directory '%1%' does not exist.", directory);
                return modules;
            }

            fs::directory_iterator it{directory};
            fs::directory_iterator end{};

            // Search for modules
            LOG(debug, "searching '%1%' for modules.", directory);
            for (; it != end; ++it) {
                // If not a directory, ignore
                if (!fs::is_directory(it->status())) {
                    continue;
                }
                auto base = it->path().string();
                auto name = it->path().filename().string();
                if (name == "lib") {
                    // Warn that the module path may not be set correctly, but add a "lib" module
                    LOG(warning, "found module named 'lib' at '%1%': this may indicate the module search path is incorrect.", base);
                }  else if (!module::is_valid_name(name)) {
                    // Warn about an invalid name
                    LOG(warning, "found module with invalid name '%1%' at '%2%': module will be ignored.", name, base);
                    continue;
                }

                auto existing = modules.find(name);
                if (existing != modules.end()) {
                    LOG(warning, "module '%1%' at '%2%' conflicts with existing module at '%3%' and will be ignored.", name, base, existing->second.base());
                    continue;
                }

                LOG(debug, "found module '%1%' at '%2%'.", name, base);
                module mod{logger, name, rvalue_cast(base)};
                modules.emplace(make_pair(rvalue_cast(name), rvalue_cast(mod)));
            }
        }
        return modules;
    }

}}  // namespace puppet::compiler
