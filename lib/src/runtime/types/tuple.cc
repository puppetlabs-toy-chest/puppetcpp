#include <puppet/runtime/values/value.hpp>
#include <puppet/cast.hpp>
#include <boost/functional/hash.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    tuple::tuple(vector<unique_ptr<values::type>> types, int64_t from, int64_t to) :
        _types(rvalue_cast(types)),
        _from(from),
        _to(to)
    {
        for (auto const& type : _types) {
            if (!type) {
                throw runtime_error("a non-null type was expected.");
            }
        }
    }

    tuple::tuple(tuple const& other) :
        _from(other._from),
        _to(other._to)
    {
        _types.reserve(other._types.size());
        for (auto const& element : other._types) {
            _types.emplace_back(new values::type(*element));
        }
    }

    tuple& tuple::operator=(tuple const& other)
    {
        _types.reserve(other._types.size());
        for (auto const& element : other._types) {
            _types.emplace_back(new values::type(*element));
        }
        _from = other._from;
        _to = other._to;
        return *this;
    }

    std::vector<std::unique_ptr<values::type>> const& tuple::types() const
    {
        return _types;
    }

    int64_t tuple::from() const
    {
        return _from;
    }

    int64_t tuple::to() const
    {
        return _to;
    }

    char const* tuple::name()
    {
        return "Tuple";
    }

    bool tuple::is_instance(values::value const& value) const
    {
        // Check for array
        auto ptr = value.as<values::array>();
        if (!ptr) {
            return false;
        }

        // Check for size is range
        int64_t size = static_cast<int64_t>(ptr->size());
        if (!(_to < _from ? (size >= _to && size <= _from) : (size >= _from && size <= _to))) {
            return false;
        }

        // If no types, only empty arrays match
        if (_types.empty()) {
            return size == 0;
        }

        // Get the last type in this tuple
        auto& last_type = _types.back();

        // For each element, check that its type is in the tuple
        for (size_t i = 0; i < ptr->size(); ++i) {
            auto const& element = (*ptr)[i];

            // If this element's position is in the tuple, match the type
            // If not, match the last type
            if (i < _types.size()) {
                if (!_types[i]->is_instance(*element)) {
                    return false;
                }
            } else if (!last_type->is_instance(*element)) {
                return false;
            }
        }
        return true;
    }

    bool tuple::is_specialization(values::type const& other) const
    {
        // Check for another Tuple
        auto ptr = boost::get<types::tuple>(&other);
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
            if (_types[i] != other_types[i]) {
                return false;
            }
        }
        // If the other type has more types, it is more specialized
        if (other_types.size() > _types.size()) {
            return true;
        }
        // Check for equality
        if (ptr->from() == _from && ptr->to() == _to) {
            return false;
        }
        return std::min(ptr->from(), ptr->to()) >= std::min(_from, _to) &&
               std::max(ptr->from(), ptr->to()) <= std::max(_from, _to);
    }

    bool tuple::is_real(unordered_map<values::type const*, bool>& map) const
    {
        // Tuple is a real type
        return true;
    }

    void tuple::write(ostream& stream, bool expand) const
    {
        // Only output the tuple name if everything is default
        stream << tuple::name();
        if (_types.empty() && _from == 0 && _to == std::numeric_limits<int64_t>::max()) {
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
        // If the from and to are equal to the size of the types, only output the types
        int64_t size = static_cast<int64_t>(_types.size());
        if (_from == size && _to == size) {
            stream << ']';
            return;
        }
        if (size > 0) {
            stream << ", ";
        }
        if (_from == 0) {
            stream << "default";
        } else {
            stream << _from;
        }
        stream << ", ";
        if (_to == std::numeric_limits<int64_t>::max()) {
            stream << "default";
        } else {
            stream << _to;
        }
        stream << ']';
    }

    ostream& operator<<(ostream& os, tuple const& type)
    {
        type.write(os);
        return os;
    }

    bool operator==(tuple const& left, tuple const& right)
    {
        auto const& left_types = left.types();
        auto const& right_types = right.types();

        // Check the range and number of types
        if (left.from() != right.from() || left.to() != right.to() || left_types.size() != right_types.size()) {
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

    bool operator!=(tuple const& left, tuple const& right)
    {
        return !(left == right);
    }

    size_t hash_value(tuple const& type)
    {
        static const size_t name_hash = boost::hash_value(tuple::name());

        size_t seed = 0;
        boost::hash_combine(seed, name_hash);
        boost::hash_range(seed, type.types().begin(), type.types().end());
        boost::hash_combine(seed, type.from());
        boost::hash_combine(seed, type.to());
        return seed;
    }

}}}  // namespace puppet::runtime::types
