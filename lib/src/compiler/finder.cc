#include <puppet/compiler/finder.hpp>
#include <puppet/utility/filesystem/helpers.hpp>
#include <puppet/cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

using namespace std;
using namespace puppet::utility::filesystem;
namespace fs = boost::filesystem;
namespace sys = boost::system;

namespace puppet { namespace compiler {

    static char const* extension(find_type type)
    {
        switch (type) {
            case find_type::manifest:
                return ".pp";

            default:
                throw runtime_error("unexpected file type.");
        }
    }

    static bool each_file(find_type type, fs::path const& directory, std::locale const& locale, function<bool(string const&)> const& callback)
    {
        vector<fs::path> entries;
        copy_if(fs::directory_iterator{ directory }, fs::directory_iterator{}, back_inserter(entries),
            [&](auto const& entry) {
                // Add entries that are manifest files or directories that are not symlinks
                return (fs::is_regular_file(entry.status()) && entry.path().extension() == extension(type)) ||
                       (!fs::is_symlink(entry.symlink_status()) && fs::is_directory(entry.status()));
            }
        );

        // Sort the entries into a deterministic order, based on the given locale
        sort(entries.begin(), entries.end(), [&](auto const& left, auto const& right) {
            return locale(left.string(), right.string());
        });

        // Go through the entires; recurse into any directories
        for (auto& entry : entries) {
            sys::error_code ec;
            if (is_directory(entry, ec)) {
                if (!each_file(type, entry, locale, callback)) {
                    return false;
                }
                continue;
            }
            if (!callback(entry.string())) {
                return false;
            }
        }
        return true;
    }

    finder::finder(string directory, compiler::settings const* settings) :
        _directory(rvalue_cast(directory))
    {
        if (settings) {
            auto manifest = settings->get(settings::manifest);
            if (manifest.as<string>()) {
                _manifests_path = make_absolute(*manifest.as<string>(), _directory);
            }
        }
        // Fall back to default search locations if not specified
        if (_manifests_path.empty()) {
            _manifests_path = make_absolute("manifests", _directory);
        }
    }

    string const& finder::directory() const
    {
        return _directory;
    }

    string finder::find_file(find_type type, string const& name) const
    {
        if (name.empty()) {
            return string();
        }

        // Start with the base
        fs::path path = base_path(type);

        // Split the name on '::' and treat all but the last component as a subdirectory
        boost::split_iterator<string::const_iterator> end;
        for (auto it = boost::make_split_iterator(name, boost::first_finder("::", boost::is_equal())); it != end; ++it) {
            path.append(it->begin(), it->end());
        }
        path.replace_extension(extension(type));

        // Otherwise, check that the file exists
        sys::error_code ec;
        if (!fs::is_regular_file(path, ec)) {
            return string();
        }
        return path.string();
    }

    void finder::each_file(find_type type, function<bool(string const&)> const& callback) const
    {
        auto& base = base_path(type);
        sys::error_code ec;
        if (fs::is_regular_file(base, ec)) {
            // If the base itself is to a file, treat it like a manifest
            callback(base);
            return;
        }
        if (!fs::is_directory(base, ec)) {
            return;
        }

        // The locale to sort the filenames with
        // This defaults to the C locale to ensure a consistent sort regardless of the current user's locale
        std::locale locale{};
        compiler::each_file(type, base, locale, callback);
    }

    string const& finder::base_path(find_type type) const
    {
        switch (type) {
            case find_type::manifest:
                return _manifests_path;

            default:
                throw runtime_error("unexpected file type.");
        }
    }

}}  // namespace puppet::compiler
