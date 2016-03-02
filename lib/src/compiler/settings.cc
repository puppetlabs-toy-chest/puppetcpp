#include <puppet/compiler/settings.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <puppet/cast.hpp>
#include <boost/format.hpp>
#include <sstream>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler {

    string const settings::base_module_path = "basemodulepath";
    string const settings::code_directory = "codedir";
    string const settings::environment = "environment";
    string const settings::environment_path = "environmentpath";
    string const settings::manifest = "manifest";
    string const settings::module_path = "modulepath";

    void settings::set(string const& name, values::value value)
    {
        auto it = find_if(_settings.begin(), _settings.end(), [&](auto const& setting) { return setting.name == name; });
        if (it != _settings.end()) {
            it->value = rvalue_cast(value);
            return;
        }
        _settings.emplace_back(setting{ name, rvalue_cast(value) });
    }

    values::value settings::get(string const& name, bool interpolate) const
    {
        auto it = find_if(_settings.begin(), _settings.end(), [&](auto const& setting) { return setting.name == name; });
        if (it == _settings.end()) {
            return values::undef{};
        }
        if (interpolate && it->value.as<string>()) {
            vector<setting const*> evaluating;
            string result;
            this->interpolate(*it, evaluating, result);
            return result;
        }
        return it->value;
    }

    void settings::each(function<bool(string const&, values::value)> const& callback, bool interpolate) const
    {
        for (auto& setting : _settings) {
            if (interpolate && setting.value.as<string>()) {
                vector<settings::setting const*> evaluating;
                string result;
                this->interpolate(setting, evaluating, result);
                if (!callback(setting.name, result)) {
                    break;
                }
            } else {
                if (!callback(setting.name, setting.value)) {
                    break;
                }
            }
        }
    }

    void settings::interpolate(settings::setting const& setting, vector<settings::setting const*>& evaluating, string& result) const
    {
        // If not a string, just lexically cast it
        if (!setting.value.as<string>()) {
            result += boost::lexical_cast<string>(setting.value);
            return;
        }

        // Ensure the interpolation itself isn't recursive
        if (find(evaluating.begin(), evaluating.end(), &setting) != evaluating.end()) {
            throw compilation_exception((boost::format("setting '%1%' with value '%2%' contains a recursive interpolation.") % setting.name % setting.value).str());
        }

        evaluating.push_back(&setting);

        bool first = true;
        boost::split_iterator<string::const_iterator> end;
        for (auto part = boost::make_split_iterator(*setting.value.as<string>(), boost::first_finder("$", boost::is_equal())); part != end; ++part) {
            // The first part is never a variable, so write it as is (if the value starts with $, an empty part will be the first)
            if (first) {
                first = false;
                result.append(part->begin(), part->end());
                continue;
            }

            // All setting names match [a-z0-9_]+ (note: lowercase letters only)
            // This looks for the ending of the variable name
            auto name_end = part->begin();
            for (; name_end != part->end() && ((*name_end >= 'a' && *name_end <= 'z') || (*name_end >= '0' && *name_end <= '9') || *name_end == '_'); ++name_end);

            string name{ part->begin(), name_end };
            auto it = find_if(_settings.begin(), _settings.end(), [&](auto const& setting) { return setting.name == name; });
            if (it == _settings.end()) {
                // Not a setting, so write it with the $ delimiter
                result += "$";
                result.append(part->begin(), part->end());
                continue;
            }

            // Interpolate where the variable was and append on the remainder of the part
            interpolate(*it, evaluating, result);
            result.append(name_end, part->end());
        }

        evaluating.pop_back();
    }

}}  // namespace puppet::compiler
