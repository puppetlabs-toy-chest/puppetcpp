/**
 * @file
 * Declares the compiler settings.
 */
#pragma once

#include "../runtime/values/value.hpp"
#include <string>
#include <memory>
#include <vector>
#include <functional>

namespace puppet { namespace compiler {

    /**
     * Represents various settings used in the compiler.
     */
    struct settings
    {
        /**
         * The base module path setting.
         */
        static std::string const base_module_path;
        /**
         * The code directory setting.
         */
        static std::string const code_directory;
        /**
         * The environment setting.
         */
        static std::string const environment;
        /**
         * The environment path setting.
         */
        static std::string const environment_path;
        /**
         * The main manifest setting.
         */
        static std::string const manifest;
        /**
         * The module path setting.
         */
        static std::string const module_path;

        /**
         * Constructs the settings using the platform's defaults.
         */
        settings();

        /**
         * Sets a setting's value.
         * @param name The setting's name.
         * @param value The setting's value.
         */
        void set(std::string const& name, runtime::values::value value);

        /**
         * Gets a setting's value.
         * @param name The setting name.
         * @param interpolate True to interpolate string values or false to return the uninterpolated value.
         * @return Returns the setting's value or nullptr if the setting is not set.
         */
        runtime::values::value get(std::string const& name, bool interpolate = true) const;

        /**
         * Enumerates each setting.
         * @param callback The callback to call for each setting.
         * @param interpolate True to interpolate string values or false to pass the uninterpolated value.
         */
        void each(std::function<bool(std::string const&, runtime::values::value)> const& callback, bool interpolate = true) const;

     private:
        struct setting
        {
            std::string name;
            runtime::values::value value;
        };

        void interpolate(setting const& value, std::vector<setting const*>& evaluating, std::string& result) const;

        // Using a vector instead of a map given the small number of settings
        std::vector<setting> _settings;
    };

}}  // puppet::compiler
