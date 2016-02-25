#include <puppet/utility/filesystem/helpers.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
namespace fs = boost::filesystem;
namespace sys = boost::system;

namespace puppet { namespace utility { namespace filesystem {

    string make_absolute(string const& path, string const& base)
    {
        auto base_path = base.empty() ? fs::current_path() : fs::path{ base };

        // Make the path absolute and lexically normal
        auto absolute_path = fs::absolute(path, base_path).lexically_normal().string();

        // If the path ends with a redundant /., remove it
        if (absolute_path.size() > 2 && boost::ends_with(absolute_path, "/.")) {
            return absolute_path.substr(0, absolute_path.size() - 2);
        }
        return absolute_path;
    }

}}}  // namespace puppet::utility::filesystem
