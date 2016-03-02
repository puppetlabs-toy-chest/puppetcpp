#include <puppet/compiler/finder.hpp>
#include <puppet/cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

using namespace std;
namespace fs = boost::filesystem;
namespace sys = boost::system;

namespace puppet { namespace compiler {

    static fs::path base_path(find_type type, compiler::finder const& finder)
    {
        fs::path path = finder.directory();

        switch (type) {
            case find_type::manifest:
                path /= "manifests";
                break;

            default:
                throw runtime_error("unexpected file type.");
        }
        return path;
    }

    static char const* extension(find_type type)
    {
        switch (type) {
            case find_type::manifest:
                return ".pp";

            default:
                throw runtime_error("unexpected file type.");
        }
    }

    static bool each_file(fs::path const& directory, find_type type, function<bool(string const&)> const& callback)
    {
        static std::locale locale{ "" };

        // Build a list of paths contained in the given directory and sort based on the locale
        vector<fs::path> entries;
        copy(fs::directory_iterator(directory), fs::directory_iterator(), back_inserter(entries));
        sort(entries.begin(), entries.end(), [&](auto const& left, auto const& right) {
            return locale(left.string(), right.string());
        });

        // Go through the entires; recurse into any directories and check for matching files
        vector<fs::path const*> matches;
        for (auto& entry : entries) {
            sys::error_code ec;
            if (is_directory(entry, ec)) {
                if (!each_file(entry, type, callback)) {
                    return false;
                }
                continue;
            }
            ec.clear();
            if (is_regular_file(entry, ec) && entry.extension().string() == extension(type)) {
                matches.emplace_back(&entry);
                continue;
            }
        }
        // Issue the callback for any matching files
        for (auto& match : matches) {
            if (!callback(match->string())) {
                return false;
            }
        }
        return true;
    }

    finder::finder(string directory) :
        _directory(rvalue_cast(directory))
    {
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

        auto path = base_path(type, *this);

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
        auto directory = base_path(type, *this);
        sys::error_code ec;
        if (!is_directory(directory, ec)) {
            return;
        }
        compiler::each_file(directory, type, callback);
    }

}}  // namespace puppet::compiler
