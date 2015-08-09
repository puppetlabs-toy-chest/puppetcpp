/**
 * @file
 * Declares the YAML fact provider.
 */
#pragma once

#include "provider.hpp"
#include <unordered_map>
#include <string>
#include <memory>
#include <functional>
#include <exception>

namespace YAML {

    // Forward declaration of Node
    class Node;

}  // namespace YAML

namespace puppet { namespace facts {

    /**
     * Exception for yaml parse errors.
     */
    struct yaml_parse_exception : std::runtime_error
    {
        /**
         * Constructs a yaml parse exception.
         * @param message The exception message.
         * @param path The path to the input file.
         * @param line The line containing the parsing error.
         * @param column The column containing the parsing error.
         * @param text The line of text containing the parsing error.
         */
        explicit yaml_parse_exception(std::string const& message, std::string path = std::string(), size_t line = 0, size_t column = 0, std::string text = std::string());

        /**
         * Gets the path of the input file.
         * @return Returns the path of the input file.
         */
        std::string const& path() const;

        /**
         * Gets the line of the parsing error.
         * @return Returns the line of the parsing error.
         */
        size_t line() const;

        /**
         * Gets the column of the parsing error.
         * @return Returns the column of the parsing error.
         */
        size_t column() const;

        /**
         * Gets the line of text containing the parsing error.
         * @return Returns the line of text containing the parsing error.
         */
        std::string const& text() const;

    private:
        std::string _path;
        size_t _line;
        size_t _column;
        std::string _text;
    };

    /**
     * Represents the YAML fact provider.
     */
    struct yaml : provider
    {
        /**
         * Constructs a YAML fact provider with the given path.
         * @param path The path to the YAML file to load.
         */
        yaml(std::string const& path);

        /**
         * Looks up a fact value by name.
         * @param name The name of the fact to look up.
         * @return Returns the fact's value or nullptr if the fact is not found.
         */
        std::shared_ptr<runtime::values::value const> lookup(std::string const& name) override;

        /**
         * Enumerates the facts in the provider.
         * @param accessed True to enumerate only the facts which have already been accessed or false to enumerate all facts.
         * @param callback The callback to call for each fact.
         */
        void each(bool accessed, std::function<bool(std::string const&, std::shared_ptr<runtime::values::value const> const&)> const& callback) override;

    private:
        void store(std::string const& name, YAML::Node const& node, runtime::values::value* parent = nullptr);

        std::unordered_map<std::string, std::shared_ptr<runtime::values::value const>> _cache;
        std::unordered_map<std::string, std::shared_ptr<runtime::values::value const>> _accessed;
    };

}}  // puppet::facts
