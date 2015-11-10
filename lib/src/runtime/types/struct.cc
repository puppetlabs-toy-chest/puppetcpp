#include <puppet/runtime/values/value.hpp>
#include <puppet/cast.hpp>
#include <boost/functional/hash.hpp>

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

    bool structure::is_instance(values::value const& value) const
    {
        // Check for hash
        auto ptr = value.as<values::hash>();
        if (!ptr) {
            return false;
        }

        // If given an empty schema, only empty hashes match
        if (!_schema.empty()) {
            return ptr->empty();
        }

        // Go through the schema and ensure the hash conforms
        size_t count = 0;
        for (auto const& kvp : _schema) {
            auto value = ptr->get(*kvp.first);
            if (!value) {
                // Key not found, treat as undef
                if (!kvp.second->is_instance(values::undef())) {
                    return false;
                }
                continue;
            }

            // Check that the value is of the expected type
            if (!kvp.second->is_instance(*value)) {
                return false;
            }

            // Required key found
            ++count;
        }
        // Ensure that the hash doesn't contain more keys than what is present in the types
        return count == ptr->size();
    }

    bool structure::is_specialization(values::type const& other) const
    {
        // Check for another Struct
        auto ptr = boost::get<structure>(&other);
        if (!ptr) {
            return false;
        }

        // Check for less types (i.e. this type is more specialized)
        auto& other_schema = ptr->schema();
        if (other_schema.size() < _schema.size()) {
            return false;
        }

        // All values present in this schema must match the other
        for (auto const& kvp : _schema) {
            auto other_it = find_if(other_schema.begin(), other_schema.end(), [&](auto& other_kvp) { return *kvp.first == *other_kvp.first; });
            if (other_it == other_schema.end()) {
                return false;
            }
            if (*kvp.second != *other_it->second) {
                return false;
            }
        }
        // If the other type has more types, it is more specialized
        if (other_schema.size() > _schema.size()) {
            return true;
        }
        return false;
    }

    ostream& operator<<(std::ostream& os, structure const& type)
    {
        os << structure::name();
        if (type.schema().empty()) {
            return os;
        }
        os << "[";
        bool first = true;
        for (auto const& kvp : type.schema()) {
            if (first) {
                first = false;
            } else {
                os << ", ";
            }
            os << *kvp.first << " => " << *kvp.second;
        }
        os << "]";
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
        for (auto const& kvp : type.schema()) {
            boost::hash_combine(seed, *kvp.first);
            boost::hash_combine(seed, *kvp.second);
        }
        return seed;
    }

}}}  // namespace puppet::runtime::types
