#include <puppet/runtime/values/value.hpp>
#include <puppet/cast.hpp>
#include <boost/functional/hash.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    array const array::instance;

    array::array(unique_ptr<values::type> type, int64_t from, int64_t to) :
        _element_type(rvalue_cast(type)),
        _from(from),
        _to(to)
    {
        if (!_element_type) {
            _element_type.reset(new values::type(data()));
        }
    }

    array::array(array const& other) :
        _element_type(new values::type(other.element_type())),
        _from(other.from()),
        _to(other.to())
    {
    }

    array& array::operator=(array const& other)
    {
        _element_type.reset(new values::type(other.element_type()));
        _from = other.from();
        _to = other.to();
        return *this;
    }

    values::type const& array::element_type() const
    {
        return *_element_type;
    }

    int64_t array::from() const
    {
        return _from;
    }

    int64_t array::to() const
    {
        return _to;
    }

    char const* array::name()
    {
        return "Array";
    }

    bool array::is_instance(values::value const& value, recursion_guard& guard) const
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

        // Check that each element is of the type
        for (auto const& element : *ptr) {
            if (!_element_type->is_instance(*element, guard)) {
                return false;
            }
        }
        return true;
    }

    bool array::is_assignable(values::type const& other, recursion_guard& guard) const
    {
        int64_t from, to;
        if (auto array = boost::get<types::array>(&other)) {
            // Ensure element type is assignable
            if (!_element_type->is_assignable(*array->_element_type, guard)) {
                return false;
            }
            from = array->_from;
            to = array->_to;
        } else if (auto tuple = boost::get<types::tuple>(&other)) {
            if (tuple->types().empty()) {
                // Not assignable from an empty tuple
                return false;
            }
            // Ensure the element type is assignable from all of the tuple's types
            for (auto& type : tuple->types()) {
                if (!_element_type->is_assignable(*type, guard)) {
                    return false;
                }
            }
            from = tuple->from();
            to = tuple->to();
        } else {
            // Not a Tuple or Array
            return false;
        }
        return std::min(from, to) >= std::min(_from, _to) &&
               std::max(from, to) <= std::max(_from, _to);
    }

    void array::write(ostream& stream, bool expand) const
    {
        stream << array::name() << '[';
        _element_type->write(stream, false);
        bool from_default = _from == 0;
        bool to_default = _to == numeric_limits<int64_t>::max();
        if (from_default && to_default) {
            // Only output the type
            stream << ']';
            return;
        }
        stream << ", ";
        if (from_default) {
            stream << "default";
        } else {
            stream << _from;
        }
        stream << ", ";
        if (to_default) {
            stream << "default";
        } else {
            stream << _to;
        }
        stream << ']';
    }

    ostream& operator<<(ostream& os, array const& type)
    {
        type.write(os);
        return os;
    }

    bool operator==(array const& left, array const& right)
    {
        return left.from() == right.from() && left.to() == right.to() && left.element_type() == right.element_type();
    }

    bool operator!=(array const& left, array const& right)
    {
        return !(left == right);
    }

    size_t hash_value(array const& type)
    {
        static const size_t name_hash = boost::hash_value(array::name());

        size_t seed = 0;
        boost::hash_combine(seed, name_hash);
        boost::hash_combine(seed, type.element_type());
        boost::hash_combine(seed, type.from());
        boost::hash_combine(seed, type.to());
        return seed;
    }

}}}  // namespace puppet::runtime::types
