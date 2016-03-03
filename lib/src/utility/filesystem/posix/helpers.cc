#include <puppet/utility/filesystem/helpers.hpp>

using namespace std;

namespace puppet { namespace utility { namespace filesystem {

    char const* path_separator()
    {
        return ":";
    }

    string home_directory()
    {
        return getenv("HOME");
    }

}}}  // namespace puppet::utility::filesystem
