#include <puppet/runtime/values/value.hpp>
#include <puppet/cast.hpp>
#include <boost/functional/hash.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    variant::variant(vector<unique_ptr<values::type>> types) :
        _types(rvalue_cast(types))
    {
        for (auto const& type : _types) {
            if (!type) {
                throw runtime_error("a non-null type was expected.");
            }
        }
    }

    variant::variant(variant const& other)
    {
        _types.reserve(other._types.size());
        for (auto const& element : other._types) {
            _types.emplace_back(new values::type(*element));
        }
    }

    variant& variant::operator=(variant const& other)
    {
        _types.reserve(other._types.size());
        for (auto const& element : other._types) {
            _types.emplace_back(new values::type(*element));
        }
        return *this;
    }

    vector<unique_ptr<values::type>> const& variant::types() const
    {
        return _types;
    }

    char const* variant::name()
    {
        return "Variant";
    }

    bool variant::is_instance(values::value const& value) const
    {
        // Go through each type and ensure one matches
        for (auto const& type : _types) {
            if (type->is_instance(value)) {
                return true;
            }
        }
        return false;
    }

    bool variant::is_specialization(values::type const& other) const
    {
        // Check for another Variant
        auto ptr = boost::get<variant>(&other);
        if (!ptr) {
            return false;
        }

        // Check for less types (i.e. this type is more specialized)
        auto& other_types = ptr->types();
        if (other_types.size() < _types.size()) {
            return false;
        }
        // All types must match
        for (size_t i = 0; i < _types.size(); ++i) {
            if (*_types[i] != *other_types[i]) {
                return false;
            }
        }
        // If the other type has more types, it is more specialized
        return other_types.size() > _types.size();
    }

    ostream& operator<<(ostream& os, variant const& type)
    {
        os << variant::name();
        if (type.types().empty()) {
            return os;
        }
        os << '[';
        bool first = true;
        for (auto const& element : type.types()) {
            if (first) {
                first = false;
            } else {
                os << ", ";
            }
            os << *element;
        }
        os << ']';
        return os;
    }

    bool operator==(variant const& left, variant const& right)
    {
        auto const& left_types = left.types();
        auto const& right_types = right.types();

        // Check the number of types
        if (left_types.size() != right_types.size()) {
            return false;
        }

        // Check the types
        for (size_t i = 0; i < left_types.size(); ++i) {
            if (*left_types[i] != *right_types[i]) {
                return false;
            }
        }
        return true;
    }

    bool operator!=(variant const& left, variant const& right)
    {
        return !(left == right);
    }

    size_t hash_value(variant const& type)
    {
        static const size_t name_hash = boost::hash_value(variant::name());

        size_t seed = 0;
        boost::hash_combine(seed, name_hash);
        for (auto const& element : type.types()) {
            boost::hash_combine(seed, *element);
        }
        return seed;
    }

}}}  // namespace puppet::runtime::types
