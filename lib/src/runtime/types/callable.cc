#include <puppet/runtime/values/value.hpp>
#include <puppet/compiler/evaluation/functions/call_context.hpp>
#include <puppet/cast.hpp>
#include <boost/functional/hash.hpp>

using namespace std;
using namespace puppet::compiler::ast;
using namespace puppet::compiler::evaluation::functions;

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

    std::pair<callable const*, bool> callable::block() const
    {
        callable const* block = nullptr;
        bool required = static_cast<bool>(_block_type);

        if (_block_type) {
            // Check to see if the block is required
            block = boost::get<callable>(_block_type.get());
            if (!block) {
                // Check to see if the block is optional
                auto optional = boost::get<types::optional>(_block_type.get());
                if (optional) {
                    block = boost::get<callable>(optional->type().get());
                }
                if (block) {
                    required = false;
                }
            }
        }
        return std::make_pair(block, required);
    }

    char const* callable::name()
    {
        return "Callable";
    }

    bool callable::is_instance(values::value const& value, recursion_guard& guard) const
    {
        // Currently functions cannot be represented as a value.
        return false;
    }

    bool callable::is_assignable(values::type const& other, recursion_guard& guard) const
    {
        auto ptr = boost::get<callable>(&other);
        if (!ptr) {
            return false;
        }

        // Check to see if this type is simply Callable; if so, the other is always assignable
        if (_types.empty() && _min == 0 && _max == numeric_limits<int64_t>::max() && !_block_type) {
            return true;
        }

        if (std::min(ptr->min(), ptr->max()) < std::min(_min, _max) ||
            std::max(ptr->min(), ptr->max()) > std::max(_min, _max)) {
            return false;
        }

        // Ensure that the other Callable's types are assignable to this one's.
        // This is intentionally backwards because Callable only accepts types that it can be invoked with.
        // Thus, Callable[Numeric] cannot be assigned to Callable[Any] (Any is not assignable to Numeric), but
        // Callable[Numeric] can be assigned to Callable[Integer] (Integer is assignable to Numeric).
        size_t i = 0;
        for (auto& type : ptr->_types) {
            if (i >= _types.size()) {
                break;
            }

            if (!type->is_assignable(*_types[i++], guard)) {
                return false;
            }
        }

        if (_block_type || ptr->_block_type) {
            if (!_block_type || !ptr->_block_type) {
                return false;
            }
            if (!ptr->_block_type->is_assignable(*_block_type, guard)) {
                return false;
            }
        }
        return true;
    }

    void callable::write(ostream& stream, bool expand) const
    {
        // If no types, and the remaining parameters are all their default values, only output the type name
        stream << callable::name();
        if (_types.empty() && _min == 0 && _max == numeric_limits<int64_t>::max() && !_block_type) {
            return;
        }

        stream << "[";
        bool first = true;
        for (auto const& element : _types) {
            if (first) {
                first = false;
            } else {
                stream << ", ";
            }
            element->write(stream, false);
        }
        // If the min and max are equal to the number of types and there is no block, don't output the remaining parameters
        int64_t size = static_cast<int64_t>(_types.size());
        if (_min == size && _max == size && !_block_type) {
            stream << ']';
            return;
        }
        if (size > 0) {
            stream << ", ";
        }
        if (_min == 0) {
            stream << "default";
        } else {
            stream << _min;
        }
        stream << ", ";
        if (_max == numeric_limits<int64_t>::max()) {
            stream << "default";
        } else {
            stream << _max;
        }
        if (_block_type) {
            stream << ", ";
            _block_type->write(stream, false);
        }
        stream << ']';
    }

    bool callable::can_dispatch(call_context const& context) const
    {
        auto& arguments = context.arguments();
        auto argument_count = static_cast<int64_t>(arguments.size());

        // Check for too many or too few arguments
        if (argument_count < _min || argument_count > _max) {
            return false;
        }

        bool passed_block = static_cast<bool>(context.block());

        // Check for block mismatch
        types::callable const* block = nullptr;
        bool required = false;
        tie(block, required) = this->block();
        if ((!block && passed_block) || (block && required && !passed_block)) {
            return false;
        }

        // If a block was passed, check the number of parameters
        if (block && passed_block) {
            auto parameter_count = static_cast<int64_t>(context.block()->parameters.size());
            if (parameter_count < block->min() || parameter_count > block->max()) {
                return false;
            }
        }

        // Ensure no mismatched parameter types
        return find_mismatch(arguments) < 0;
    }

    int64_t callable::find_mismatch(values::array const& arguments) const
    {
        // If there are no argument types, there is no mismatch
        if (_types.empty()) {
            return -1;
        }

        // Check the argument types
        types::recursion_guard guard;
        auto argument_count = static_cast<int64_t>(arguments.size());
        for (int64_t i = 0; i < argument_count; ++i) {
            // Ensure the argument is of the specified type
            if (!parameter_type(i)->is_instance(arguments[i], guard)) {
                return i;
            }
        }
        return -1;
    }

    values::type const* callable::parameter_type(int64_t index) const
    {
        if (index < 0 || _types.empty()) {
            return nullptr;
        }
        // If the index is in bounds, use the specified type
        if (index < static_cast<int64_t>(_types.size())) {
            return _types[index].get();
        }
        // If the index is beyond the specified types, use the last one
        return _types.back().get();
    }

    ostream& operator<<(ostream& os, callable const& type)
    {
        type.write(os);
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
        if (left.block_type() || right.block_type()) {
            if (!left.block_type() || !right.block_type()) {
                return false;
            }
            return *left.block_type() == *right.block_type();
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
        for (auto& t : type.types()) {
            boost::hash_combine(seed, *t);
        }
        boost::hash_combine(seed, type.min());
        boost::hash_combine(seed, type.max());
        if (type.block_type()) {
            boost::hash_combine(seed, *type.block_type());
        }
        return seed;
    }

}}}  // namespace puppet::runtime::types
