#include <puppet/compiler/finder.hpp>
#include <puppet/cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

using namespace std;
namespace fs = boost::filesystem;
namespace sys = boost::system;

namespace puppet { namespace compiler {

    finder::finder(string directory) :
        _directory(rvalue_cast(directory))
    {
    }

    string const& finder::directory() const
    {
        return _directory;
    }

    string finder::find(find_type type, string const& name) const
    {
        if (name.empty()) {
            return string();
        }

        fs::path path = _directory;

        switch (type) {
            case find_type::manifest:
                path /= "manifests";
                break;

            default:
                throw runtime_error("unexpected file type.");
        }

       // Split the name on '::' and treat all but the last component as a subdirectory
        boost::split_iterator<string::const_iterator> end;
        for (auto it = boost::make_split_iterator(name, boost::first_finder("::", boost::is_equal())); it != end; ++it) {
            path.append(it->begin(), it->end());
        }
        path.replace_extension(".pp");

        // Otherwise, check that the file exists
        sys::error_code ec;
        if (!fs::is_regular_file(path, ec)) {
            return string();
        }
        return path.string();
    }

}}  // namespace puppet::compiler
