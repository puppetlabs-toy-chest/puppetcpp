#include <puppet/runtime/values/value.hpp>
#include <puppet/cast.hpp>
#include <boost/functional/hash.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    callable::callable(vector<unique_ptr<values::type>> types, int64_t min, int64_t max, unique_ptr<values::type> block_type) :
        _types(rvalue_cast(types)),
        _min(min),
        _max(max),
        _block_type(rvalue_cast(block_type))
    {
    }

    callable::callable(callable const& other) :
        _min(other._min),
        _max(other._max)
    {
        _types.reserve(other._types.size());
        for (auto const& element : other._types) {
            _types.emplace_back(new values::type(*element));
        }
        _block_type.reset(other._block_type ? new values::type{ *other._block_type } : nullptr);
    }

    callable& callable::operator=(callable const& other)
    {
        _types.reserve(other._types.size());
        for (auto const& element : other._types) {
            _types.emplace_back(new values::type(*element));
        }
        _min = other._min;
        _max = other._max;
        _block_type.reset(other._block_type ? new values::type{ *other._block_type } : nullptr);
        return *this;
    }

    vector<unique_ptr<values::type>> const& callable::types() const
    {
        return _types;
    }

    int64_t callable::min() const
    {
        return _min;
    }

    int64_t callable::max() const
    {
        return _max;
    }

    unique_ptr<values::type> const& callable::block_type() const
    {
        return _block_type;
    }

    char const* callable::name()
    {
        return "Callable";
    }

    bool callable::is_instance(values::value const& value) const
    {
        // Currently functions cannot be represented as a value,
        // This will need to change once functions can be properly treated as a value.
        return false;
    }

    bool callable::is_specialization(values::type const& other) const
    {
        // Check for another Callable
        auto ptr = boost::get<callable>(&other);
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

        // The blocks must match
        if ((_block_type && !ptr->block_type()) ||
            (!_block_type && ptr->block_type()) ||
            *_block_type != *ptr->block_type()) {
            return false;
        }

        // Check for equality
        if (_min == ptr->min() && _max == ptr->max()) {
            return false;
        }
        // Check for a more specialized min/max
        return std::min(ptr->min(), ptr->max()) >= std::min(_min, _max) &&
               std::max(ptr->min(), ptr->max()) <= std::max(_min, _max);
    }

    ostream& operator<<(ostream& os, callable const& type)
    {
        // If no types, and the remaining parameters are all their default values, only output the type name
        os << callable::name();
        if (type.types().empty() && type.min() == 0 && type.max() == numeric_limits<int64_t>::max() && !type.block_type()) {
            return os ;
        }

        os << "[";
        bool first = true;
        for (auto const& element : type.types()) {
            if (first) {
                first = false;
            } else {
                os << ", ";
            }
            os << *element;
        }
        // If the min and max are equal to the number of types and there is no block, don't output the remaining parameters
        int64_t size = static_cast<int64_t>(type.types().size());
        if (type.min() == size && type.max() == size && !type.block_type()) {
            os << ']';
            return os;
        }
        if (size > 0) {
            os << ", ";
        }
        if (type.min() == 0) {
            os << "default";
        } else {
            os << type.min();
        }
        os << ", ";
        if (type.max() == numeric_limits<int64_t>::max()) {
            os << "default";
        } else {
            os << type.max();
        }
        if (type.block_type()) {
            os << ", ";
            os << *type.block_type();
        }
        os << ']';
        return os;
    }

    bool operator==(callable const& left, callable const& right)
    {
        auto const& left_types = left.types();
        auto const& right_types = right.types();

        // Check for mismatch min, max, or type sizes
        if (left.min() != right.min() ||
            left.max() != right.max() ||
            left_types.size() != right_types.size()) {
            return false;
        }
        // Check the types
        for (size_t i = 0; i < left_types.size(); ++i) {
            if (*left_types[i] != *right_types[i]) {
                return false;
            }
        }
        // Check the block
        if ((left.block_type() && !right.block_type()) ||
            (!left.block_type() && right.block_type()) ||
            *left.block_type() != *right.block_type()) {
            return false;
        }
        return true;
    }

    bool operator!=(callable const& left, callable const& right)
    {
        return !(left == right);
    }

    size_t hash_value(callable const& type)
    {
        static const size_t name_hash = boost::hash_value(callable::name());

        size_t seed = 0;
        boost::hash_combine(seed, name_hash);
        boost::hash_range(seed, type.types().begin(), type.types().end());
        boost::hash_combine(seed, type.min());
        boost::hash_combine(seed, type.max());
        if (type.block_type()) {
            boost::hash_combine(seed, *type.block_type());
        }
        return seed;
    }

}}}  // namespace puppet::runtime::types
