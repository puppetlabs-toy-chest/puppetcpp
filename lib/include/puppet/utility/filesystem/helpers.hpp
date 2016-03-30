/**
 * @file
 * Declares the filesystem helper functions.
 */
#pragma once

#include <string>
#include <boost/filesystem.hpp>

namespace puppet { namespace utility { namespace filesystem {

    /**
     * Gets the default path separator.
     * This will be ':' on POSIX systems and ';' on Windows.
     * @return Returns the default path separator.
     */
    char const* path_separator();

    /**
     * Gets the home directory of the current user.
     * @return Returns the home directory of the current user or an empty string if the home directory can't be determined.
     */
    std::string home_directory();

    /**
     * Makes a path absolute.
     * This will also make the path lexically normal.
     * @param path The path to make absolute.
     * @param base The base path to resolve a relative path to; defaults to the current directory.
     * @return Returns the normalized absolute path.
     */
    std::string make_absolute(std::string const& path, std::string const& base = {});

    /**
     * Normalizes a relative path.
     * @param path The relative path to normalize.
     * @return Returns true if the path is relative and was normallized; returns false if the path was not relative.
     */
    bool normalize_relative_path(std::string& path);


}}}  // namespace puppet::utility::filesystem
