/**
 * @file
 * Declares the filesystem helper functions.
 */
#pragma once

#include <string>

namespace puppet { namespace utility { namespace filesystem {

    /**
     * Gets the default path separator.
     * This will be ':' on POSIX systems and ';' on Windows.
     * @return Returns the default path separator.
     */
    char const* path_separator();

    /**
     * Makes a path absolute.
     * This will also make the path lexically normal.
     * @param path The path to make absolute.
     * @param base The base path to resolve a relative path to; defaults to the current directory.
     * @return Returns the normalized absolute path.
     */
    std::string make_absolute(std::string const& path, std::string const& base = {});

}}}  // namespace puppet::utility::filesystem
