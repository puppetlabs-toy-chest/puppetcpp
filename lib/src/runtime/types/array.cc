#include <puppet/runtime/values/value.hpp>
#include <puppet/cast.hpp>
#include <boost/functional/hash.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

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

    bool array::is_instance(values::value const& value) const
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
            if (!_element_type->is_instance(*element)) {
                return false;
            }
        }
        return true;
    }

    bool array::is_specialization(values::type const& other) const
    {
        // For the other type to be a specialization, it must be an Array or Tuple
        // For Tuple, the number of types must be 1
        // The element types must match
        // And the range of other needs to be inside of this type's range
        int64_t from, to;
        auto array = boost::get<types::array>(&other);
        if (!array) {
            // Check for Array[ElementType]
            if (array->element_type() != *_element_type) {
                return false;
            }
            from = array->from();
            to = array->to();
        } else {
            // Check for a Tuple with a single type
            auto tuple = boost::get<types::tuple>(&other);
            if (!tuple || tuple->types().size() != 1) {
                return false;
            }
            // Check that the Tuple's type matches the element type
            auto& type = tuple->types().front();
            if (*type != *_element_type) {
                return false;
            }
            from = tuple->from();
            to = tuple->to();
        }
        // Check for equality
        if (from == _from && to == _to) {
            return false;
        }
        return std::min(from, to) >= std::min(_from, _to) &&
               std::max(from, to) <= std::max(_from, _to);
    }

    ostream& operator<<(ostream& os, array const& type)
    {
        os << array::name() << '[' << type.element_type();
        bool from_default = type.from() == numeric_limits<int64_t>::min();
        bool to_default = type.to() == numeric_limits<int64_t>::max();
        if (from_default && to_default) {
            // Only output the type
            os << ']';
            return os;
        }
        os << ", ";
        if (from_default) {
            os << "default";
        } else {
            os << type.from();
        }
        os << ", ";
        if (to_default) {
            os << "default";
        } else {
            os << type.to();
        }
        os << ']';
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
