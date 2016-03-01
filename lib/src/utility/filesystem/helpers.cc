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
        if (absolute_path.size() > 2 && (boost::ends_with(absolute_path, "/.") || boost::ends_with(absolute_path, "\\."))) {
            return absolute_path.substr(0, absolute_path.size() - 2);
        }
        return absolute_path;
    }

    bool normalize_relative_path(string& path)
    {
        auto normalized = fs::path{ path }.lexically_normal();
        auto& normalized_path = normalized.string();
        if (!normalized.is_relative() || boost::starts_with(normalized_path, "../") || boost::starts_with(normalized_path, "..\\")) {
            return false;
        }
        // If the path ends with a redundant /., remove it
        if (normalized_path.size() > 2 && (boost::ends_with(normalized_path, "/.") || boost::ends_with(normalized_path, "\\."))) {
            path = normalized_path.substr(0, normalized_path.size() - 2);
        } else {
            path = normalized_path;
        }
        return true;
    }

}}}  // namespace puppet::utility::filesystem
