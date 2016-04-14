#include <puppet/runtime/values/value.hpp>
#include <puppet/cast.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace values {

    iterator::iterator(wrapper<values::value> value, int64_t step, bool reverse) :
        _value(rvalue_cast(value)),
        _step(step),
        _reverse(reverse)
    {
        if (_step == 0) {
            throw runtime_error("invalid step value.");
        }
    }

    values::value const& iterator::value() const
    {
        if (auto iterator = _value->as<values::iterator>()) {
            return iterator->value();
        }
        return _value;
    }

    int64_t iterator::step() const
    {
        return _step;
    }

    bool iterator::reverse() const
    {
        return _reverse;
    }

    void iterator::each(callback_type const& callback, bool reverse) const
    {
        if (!callback) {
            return;
        }

        boost::apply_visitor(iteration_visitor{ callback, _step, reverse ? !_reverse : _reverse }, _value.get());
    }

    ostream& operator<<(ostream& os, values::iterator const& iterator)
    {
        bool is_hash = iterator.value().as<values::hash>();

        if (is_hash) {
            os << '{';
        } else {
            os << '[';
        }

        bool first = true;
        iterator.each([&](auto const* key, auto const& value) {
            if (first) {
                first = false;
            } else {
                os << ", ";
            }
            if (key) {
                os << *key << " => ";
            }
            os << value;
            return true;
        });

        if (is_hash) {
            os << '}';
        } else {
            os << ']';
        }
        return os;
    }

    bool operator==(iterator const& left, iterator const& right)
    {
        return left.step()    == right.step()    &&
               left.reverse() == right.reverse() &&
               left.value()   == right.value();
    }

    bool operator!=(iterator const& left, iterator const& right)
    {
        return !(left == right);
    }

    size_t hash_value(values::iterator const& iterator)
    {
        static const size_t name_hash = boost::hash_value("iterator");

        std::size_t seed = 0;
        boost::hash_combine(seed, name_hash);
        boost::hash_combine(seed, iterator.value());
        boost::hash_combine(seed, iterator.step());
        boost::hash_combine(seed, iterator.reverse());
        return seed;
    }

    iteration_visitor::iteration_visitor(iterator::callback_type const& callback, int64_t step, bool reverse) :
        _callback(callback),
        _step(step),
        _reverse(reverse)
    {
    }

    void iteration_visitor::operator()(undef const&) const
    {
        throw runtime_error("undef is not iterable.");
    }

    void iteration_visitor::operator()(defaulted const&) const
    {
        throw runtime_error("defaulted is not iterable.");
    }

    void iteration_visitor::operator()(int64_t value) const
    {
        if (value <= 0) {
            return;
        }

        for (int64_t i = (_reverse ? value - 1 : 0); i >= 0 && (_reverse ? true : i < value); i += (_reverse ? -_step : _step)) {
            if (!_callback(nullptr, i)) {
                break;
            }
            // Check for potential underflow/overflow
            if ((_reverse && i < static_cast<int64_t>(_step)) || (!_reverse && i > (i + static_cast<int64_t>(_step)))) {
                break;
            }
        }
    }

    void iteration_visitor::operator()(double) const
    {
        throw runtime_error("double is not iterable.");
    }

    void iteration_visitor::operator()(bool) const
    {
        throw runtime_error("boolean is not iterable.");
    }

    void iteration_visitor::operator()(std::string const& value) const
    {
        int64_t step = 0;
        each_code_point(
            value,
            [&](auto character) {
                if (step > 0) {
                    --step;
                    return true;
                }
                step = _step - 1;
                return _callback(nullptr, character);
            },
            _reverse
        );
    }

    void iteration_visitor::operator()(values::regex const& value) const
    {
        throw runtime_error("regex is not iterable.");
    }

    void iteration_visitor::operator()(values::type const& value) const
    {
        if (auto integer = boost::get<types::integer>(&value)) {
            operator()(*integer);
            return;
        }
        if (auto enumeration = boost::get<types::enumeration>(&value)) {
            operator()(*enumeration);
            return;
        }
        throw runtime_error("type is not iterable.");
    }

    void iteration_visitor::operator()(types::integer const& range) const
    {
        if (!range.iterable()) {
            return;
        }

        for (int64_t i = (_reverse ? range.to() : range.from()); i >= range.from() && (_reverse ? true : i <= range.to()); i += (_reverse ? -_step : _step)) {
            if (!_callback(nullptr, i)) {
                break;
            }
            // Check for potential underflow/overflow
            if ((_reverse && i < (i - static_cast<int64_t>(_step))) || (!_reverse && i > (i + static_cast<int64_t>(_step)))) {
                break;
            }
        }
    }

    void iteration_visitor::operator()(types::enumeration const& enumeration) const
    {
        for (auto& string : enumeration.strings()) {
            if (!_callback(nullptr, string)) {
                break;
            }
        }
    }

    void iteration_visitor::operator()(variable const& value) const
    {
        boost::apply_visitor(*this, value.value());
    }

    void iteration_visitor::operator()(values::array const& value) const
    {
        if (value.empty()) {
            return;
        }
        int64_t size = static_cast<int64_t>(value.size());
        for (int64_t i = (_reverse ? size - 1 : 0); _reverse ? true : i < size; i += (_reverse ? -_step : _step)) {
            if (!_callback(nullptr, value[i])) {
                break;
            }
            // Check for potential underflow/overflow
            if ((_reverse && i < _step) || (!_reverse && i > (i + _step))) {
                break;
            }
        }
    }

    void iteration_visitor::operator()(values::hash const& value) const
    {
        if (value.empty()) {
            return;
        }
        if (_reverse) {
            int64_t step = 0;
            for (auto it = value.rbegin(); it != value.rend(); ++it) {
                if (step > 0) {
                    --step;
                    continue;
                }
                step = _step - 1;
                if (!_callback(&it->key(), it->value())) {
                    return;
                }
            }
            return;
        }
        int64_t step = 0;
        for (auto& kvp : value) {
            if (step > 0) {
                --step;
                continue;
            }
            step = _step - 1;
            if (!_callback(&kvp.key(), kvp.value())) {
                return;
            }
        }
    }

    void iteration_visitor::operator()(values::iterator const& value) const
    {
        int64_t step = 0;
        value.each(
            [&](auto const* key, auto const& value) {
                if (step > 0) {
                    --step;
                    return true;
                }
                step = _step - 1;
                return _callback(key, value);
            },
            _reverse
        );
    }

}}}  // namespace puppet::runtime::values
