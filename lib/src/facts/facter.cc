#include <puppet/facts/facter.hpp>
#include <puppet/cast.hpp>
#include <facter/facts/collection.hpp>
#include <facter/facts/scalar_value.hpp>
#include <facter/facts/map_value.hpp>
#include <facter/facts/array_value.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace puppet::runtime;
using namespace facter::facts;

namespace puppet { namespace facts {

    facter::facter()
    {
        // Add default facts
        _collection.add_default_facts(true);
        _collection.add_external_facts();

        // TODO: support additional locations for external facts?
        _collection.add_environment_facts();

        // TODO: add custom facts?  Need to initialize the Ruby VM in main
    }

    shared_ptr<values::value const> facter::lookup(string const& name)
    {
        shared_ptr<values::value const> value;

        // First check the cache
        auto it = _cache.find(name);
        if (it == _cache.end()) {
            // Not in cache, check the fact collection
            store(name, _collection[name]);
            it = _cache.find(name);
        }
        if (it != _cache.end()) {
            value = it->second;
        }
        return value;
    }

    void facter::each(bool accessed, function<bool(string const&, shared_ptr<values::value const> const&)> const& callback)
    {
        // If all facts, enumerate all the facts and store in the cache
        if (!accessed) {
            _collection.each([this](string const& name, ::facter::facts::value const* value) {
                if (_cache.count(name)) {
                    return true;
                }
                store(name, value);
                return true;
            });
        }

        // Enumerate what's in the cache
        for (auto& kvp : _cache) {
            if (!callback(kvp.first, kvp.second)) {
                break;
            }
        }
    }

    void facter::store(string const& name, ::facter::facts::value const* value, values::value* parent)
    {
        if (!value) {
            return;
        }

        values::value converted;

        if (auto ptr = dynamic_cast<string_value const*>(value)) {
            converted = ptr->value();
        } else if (auto ptr = dynamic_cast<integer_value const*>(value)) {
            converted = ptr->value();
        } else if (auto ptr = dynamic_cast<boolean_value const*>(value)) {
            converted = ptr->value();
        } else if (auto ptr = dynamic_cast<double_value const*>(value)) {
            converted = static_cast<long double>(ptr->value());
        } else if (auto ptr = dynamic_cast<array_value const*>(value)) {
            converted = values::array();
            ptr->each([&](::facter::facts::value const* element) {
                store(string(), element, &converted);
                return true;
            });
        } else if (auto ptr = dynamic_cast<map_value const*>(value)) {
            converted = values::hash();
            ptr->each([&](string const& name, ::facter::facts::value const* element) {
                store(name, element, &converted);
                return true;
            });
        }

        // If parent, add to array or map
        if (parent) {
            // boost::get is used here because we know the parent is an array or hash and not a variable
            if (auto ptr = boost::get<values::array>(parent)) {
                ptr->emplace_back(rvalue_cast(converted));
            } else if (auto ptr = boost::get<values::hash>(parent)) {
                ptr->set(name, rvalue_cast(converted));
            }
        } else {
            _cache.emplace(boost::to_lower_copy(name), std::make_shared<values::value>(rvalue_cast(converted)));
        }
    }

}}  // namespace puppet::facts
