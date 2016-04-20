#include <puppet/runtime/values/value.hpp>
#include <puppet/utility/indirect_collection.hpp>
#include <puppet/cast.hpp>
#include <boost/functional/hash.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    variant::variant(vector<unique_ptr<values::type>> types)
    {
        utility::indirect_set<values::type> set;
        for (auto& type : types) {
            if (!type) {
                throw runtime_error("a non-null type was expected.");
            }

            if (set.insert(type.get()).second) {
                _types.emplace_back(rvalue_cast(type));
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

    values::type variant::unwrap()
    {
        if (_types.size() == 1) {
            return values::type{ rvalue_cast(*_types.front()) };
        }
        return values::type{ rvalue_cast(*this) };
    }

    values::type variant::generalize() const
    {
        vector<unique_ptr<values::type>> types;
        for (auto& type : _types) {
            types.emplace_back(make_unique<values::type>(type->generalize()));
        }
        return types::variant{ rvalue_cast(types) };
    }

    bool variant::is_instance(values::value const& value, recursion_guard& guard) const
    {
        // Go through each type and ensure one matches
        for (auto const& type : _types) {
            if (type->is_instance(value, guard)) {
                return true;
            }
        }
        return false;
    }

    bool variant::is_assignable(values::type const& other, recursion_guard& guard) const
    {
        // Check for another Variant
        auto ptr = boost::get<variant>(&other);
        if (ptr) {
            // A Variant with no types is assignable
            for (auto& other_type : ptr->_types) {
                bool found = false;
                for (auto& type : _types) {
                    found = type->is_assignable(*other_type, guard);
                    if (found) {
                        break;
                    }
                }
                if (!found) {
                    return false;
                }
            }
            return true;
        }

        // Check that the type is assignable to something in this Variant
        for (auto& type : _types) {
            if (type->is_assignable(other, guard)) {
                return true;
            }
        }
        return false;
    }

    void variant::write(ostream& stream, bool expand) const
    {
        stream << variant::name();
        if (_types.empty()) {
            return;
        }
        stream << '[';
        bool first = true;
        for (auto const& element : _types) {
            if (first) {
                first = false;
            } else {
                stream << ", ";
            }
            element->write(stream, false);
        }
        stream << ']';
    }

    ostream& operator<<(ostream& os, variant const& type)
    {
        type.write(os);
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
        for (auto& t : type.types()) {
            boost::hash_combine(seed, *t);
        }
        return seed;
    }

}}}  // namespace puppet::runtime::types
