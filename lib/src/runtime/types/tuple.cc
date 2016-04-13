#include <puppet/runtime/values/value.hpp>
#include <puppet/cast.hpp>
#include <boost/functional/hash.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    tuple const tuple::instance;

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

    bool tuple::is_instance(values::value const& value, recursion_guard& guard) const
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
                if (!_types[i]->is_instance(*element, guard)) {
                    return false;
                }
            } else if (!last_type->is_instance(*element, guard)) {
                return false;
            }
        }
        return true;
    }

    bool tuple::is_assignable(values::type const& other, recursion_guard& guard) const
    {
        int64_t from, to;
        if (auto array = boost::get<types::array>(&other)) {
            if (!_types.empty()) {
                auto count = std::min(static_cast<int64_t>(_types.size()), _to);
                for (int64_t i = 0; i < count; ++i) {
                    if (!_types[i]->is_assignable(array->element_type(), guard)) {
                        return false;
                    }
                }
            }
            from = array->from();
            to = array->to();
        } else if (auto tuple = boost::get<types::tuple>(&other)) {
            if (!_types.empty()) {
                auto& other_types = tuple->types();
                auto& last_type = _types.back();
                for (size_t i = 0; i < other_types.size(); ++i) {
                    if (i < _types.size()) {
                        if (!_types[i]->is_assignable(*other_types[i], guard)) {
                            return false;
                        }
                    } else {
                        if (!last_type->is_assignable(*other_types[i], guard)) {
                            return false;
                        }
                    }
                }
            }
            from = tuple->from();
            to = tuple->to();
        } else {
            return false;
        }
        return std::min(from, to) >= std::min(_from, _to) && std::max(from, to) <= std::max(_from, _to);
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
        for (auto& t : type.types()) {
            boost::hash_combine(seed, *t);
        }
        boost::hash_combine(seed, type.from());
        boost::hash_combine(seed, type.to());
        return seed;
    }

}}}  // namespace puppet::runtime::types
