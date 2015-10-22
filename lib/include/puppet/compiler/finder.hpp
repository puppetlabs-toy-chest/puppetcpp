/**
 * @file
 * Declares the file finder.
 */
#pragma once

#include <string>
#include <memory>

namespace puppet { namespace compiler {

    /**
     * Represents the type of file to find.
     */
    enum class find_type
    {
        /**
         * Finds a manifest.
         */
        manifest
    };

    /**
     * Responsible for finding files.
     */
    struct finder
    {
        /**
         * Constructs a new finder.
         * @param directory The directory for the finder to use.
         */
        explicit finder(std::string directory);

        /**
         * Gets the directory used by the finder.
         * @return Returns the directory used by the finder.
         */
        std::string const& directory() const;

        /**
         * Finds a file by qualified name.
         * @param type The type of file to find.
         * @param name The qualified name (e.g. foo::bar).
         * @return Returns the path to the file if it exists or an empty string if the file does not.
         */
        std::string find(find_type type, std::string const& name) const;

     private:
        std::string _directory;
    };

}}  // puppet::compiler
