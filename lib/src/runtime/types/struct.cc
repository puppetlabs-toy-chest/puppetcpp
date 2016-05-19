#include <puppet/runtime/values/value.hpp>
#include <puppet/cast.hpp>
#include <boost/functional/hash.hpp>
#include <puppet/runtime/types/not_undef.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    structure::structure(schema_type schema) :
        _schema(rvalue_cast(schema))
    {
        for (auto const& kvp : _schema) {
            if (!kvp.first) {
                throw runtime_error("a non-null key for schema was expected.");
            }
            if (!kvp.second) {
                throw runtime_error("a non-null value for schema was expected.");
            }
        }
    }

    structure::structure(structure const& other)
    {
        _schema.reserve(other._schema.size());
        for (auto const& kvp : other._schema) {
            auto key = make_unique<values::type>(*kvp.first);
            auto value = make_unique<values::type>(*kvp.second);
            _schema.emplace_back(make_pair(rvalue_cast(key), rvalue_cast(value)));
        }
    }

    structure& structure::operator=(structure const& other)
    {
        _schema.reserve(other._schema.size());
        for (auto const& kvp : other._schema) {
            auto key = make_unique<values::type>(*kvp.first);
            auto value = make_unique<values::type>(*kvp.second);
            _schema.emplace_back(make_pair(rvalue_cast(key), rvalue_cast(value)));
        }
        return *this;
    }

    structure::schema_type const& structure::schema() const
    {
        return _schema;
    }

    char const* structure::name()
    {
        return "Struct";
    }

    values::type structure::generalize() const
    {
        schema_type schema;
        for (auto& kvp : _schema) {
            schema.emplace_back(make_unique<values::type>(kvp.first->generalize()), make_unique<values::type>(kvp.second->generalize()));
        }
        return types::structure{ rvalue_cast(schema) };
    }

    bool structure::is_instance(values::value const& value, recursion_guard& guard) const
    {
        // Check for hash
        auto ptr = value.as<values::hash>();
        if (!ptr) {
            return false;
        }

        // If given an empty schema, only empty hashes match
        if (_schema.empty()) {
            return ptr->empty();
        }

        // Go through the schema and ensure the hash conforms
        size_t count = 0;
        for (auto const& kvp : _schema) {
            auto value = ptr->get(to_key(*kvp.first));
            if (!value) {
                // Check to see if the key is entirely optional
                if (boost::get<types::optional>(kvp.first.get())) {
                    continue;
                }
                // Check to see if the key is required
                if (boost::get<types::not_undef>(kvp.first.get())) {
                    return false;
                }
                // Key not found, treat as undef
                if (!kvp.second->is_instance(values::undef(), guard)) {
                    return false;
                }
                continue;
            }

            // Check that the value is of the expected type
            if (!kvp.second->is_instance(*value, guard)) {
                return false;
            }

            // Required key found
            ++count;
        }
        // Ensure that the hash doesn't contain more keys than what is present in the types
        return count == ptr->size();
    }

    bool structure::is_assignable(values::type const& other, recursion_guard& guard) const
    {
        if (auto structure = boost::get<types::structure>(&other)) {
            auto& other_schema = structure->schema();
            size_t matched = 0;
            for (auto& kvp : _schema) {
                // Find the key in the other structure
                auto it = find_if(other_schema.begin(), other_schema.end(), [&](auto const& okvp) {
                    return to_key(*kvp.first) == to_key(*okvp.first);
                });
                if (it == other_schema.end()) {
                    // Ensure the key is optional
                    if (!kvp.first->is_assignable(undef::instance, guard)) {
                        return false;
                    }
                } else {
                    ++matched;
                    if (!kvp.first->is_assignable(*it->first, guard) || !kvp.second->is_assignable(*it->second, guard)) {
                        return false;
                    }
                }
            }
            return matched == other_schema.size();
        } else if (auto hash = boost::get<types::hash>(&other)) {
            int64_t required = 0;
            for (auto& kvp : _schema) {
                if (kvp.first->is_assignable(undef::instance, guard)) {
                    continue;
                }
                if (!hash->key_type().is_instance(to_key(*kvp.first), guard)) {
                    return false;
                }
                ++required;
                if (!kvp.second->is_assignable(hash->value_type(), guard)) {
                    return false;
                }
            }
            // Ensure the number of required elements is in range of the hash
            return integer{required, static_cast<int64_t>(_schema.size())}.is_assignable(integer{ hash->from(), hash->to() }, guard);
        }
        return false;
    }

    void structure::write(ostream& stream, bool expand) const
    {
        stream << structure::name();
        if (_schema.empty()) {
            return;
        }
        stream << "[{";
        bool first = true;
        for (auto const& kvp : _schema) {
            if (first) {
                first = false;
            } else {
                stream << ", ";
            }
            if (boost::get<enumeration>(kvp.first.get())) {
                stream << to_key(*kvp.first);
            } else {
                kvp.first->write(stream, false);
            }
            stream << " => ";
            kvp.second->write(stream, false);
        }
        stream << "}]";
    }

    std::string const& structure::to_key(values::type const& type)
    {
        static std::string empty;

        // Check for Enum[string]
        types::enumeration const* enumeration = boost::get<types::enumeration>(&type);

        if (!enumeration) {
            // Check for Optional[Enum[string]]
            if (auto optional = boost::get<types::optional>(&type)) {
                if (optional->type()) {
                    enumeration = boost::get<types::enumeration>(optional->type().get());
                }
            }
        }

        if (!enumeration) {
            // Check for NotUndef[Enum[string]]
            if (auto not_undef = boost::get<types::not_undef>(&type)) {
                if (not_undef->type()) {
                    enumeration = boost::get<types::enumeration>(not_undef->type().get());
                }
            }
        }

        return enumeration && !enumeration->strings().empty() ? *enumeration->strings().begin() : empty;
    }

    ostream& operator<<(ostream& os, structure const& type)
    {
        type.write(os);
        return os;
    }

    bool operator==(structure const& left, structure const& right)
    {
        // Ensure the schemas are of the same size
        auto& left_schema = left.schema();
        auto& right_schema = right.schema();
        if (left_schema.size() != right_schema.size()) {
            return false;
        }

        // Check the types
        for (size_t i = 0; i < left_schema.size(); ++i) {
            auto& left_pair = left_schema[i];
            auto& right_pair = right_schema[i];
            if (*left_pair.first != *right_pair.first ||
                *left_pair.second != *right_pair.second) {
                return false;
            }
        }
        return true;
    }

    bool operator!=(structure const& left, structure const& right)
    {
        return !(left == right);
    }

    size_t hash_value(structure const& type)
    {
        static const size_t name_hash = boost::hash_value(structure::name());

        size_t seed = 0;
        boost::hash_combine(seed, name_hash);
        for (auto& kvp : type.schema()) {
            boost::hash_combine(seed, *kvp.first);
            boost::hash_combine(seed, *kvp.second);
        }
        return seed;
    }

}}}  // namespace puppet::runtime::types
