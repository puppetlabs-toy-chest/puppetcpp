#include <puppet/runtime/values/value.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <puppet/unicode/string.hpp>
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

    values::type iterator::infer_produced_type() const
    {
        auto& value = this->value();

        if (value.as<int64_t>()) {
            return types::integer{};
        }
        if (value.as<std::string>()) {
            return types::string{};
        }
        if (auto type = value.as<values::type>()) {
            if (boost::get<types::integer>(type)) {
                return types::integer{};
            }
            if (boost::get<types::enumeration>(type)) {
                return types::string{};
            }
            throw type_not_iterable_exception("value is not iterable.");
        }
        if (value.as<values::array>()) {
            auto type = value.infer_type();
            auto& array_type = boost::get<types::array>(type);
            return array_type.element_type();
        }
        if (value.as<values::hash>()) {
            auto type = value.infer_type();
            auto& hash_type = boost::get<types::hash>(type);
            vector<unique_ptr<values::type>> types;
            types.emplace_back(make_unique<values::type>(hash_type.key_type()));
            types.emplace_back(make_unique<values::type>(hash_type.value_type()));
            return types::tuple{ rvalue_cast(types), 2, 2 };
        }
        throw type_not_iterable_exception("value is not iterable.");
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
        throw type_not_iterable_exception("undef is not iterable.");
    }

    void iteration_visitor::operator()(defaulted const&) const
    {
        throw type_not_iterable_exception("defaulted is not iterable.");
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
        throw type_not_iterable_exception("double is not iterable.");
    }

    void iteration_visitor::operator()(bool) const
    {
        throw type_not_iterable_exception("boolean is not iterable.");
    }

    void iteration_visitor::operator()(std::string const& value) const
    {
        unicode::string string{ value };

        auto it = _reverse ? string.rbegin() : string.begin();
        auto end = _reverse ? string.rend() : string.end();

        int64_t step = 0;
        for (; it != end; ++it) {
            if (step > 0) {
                --step;
                continue;
            }
            step = _step - 1;
            if (!_callback(nullptr, std::string{ it->begin(), it->end() })) {
                break;
            }
        }
    }

    void iteration_visitor::operator()(values::regex const& value) const
    {
        throw type_not_iterable_exception("regex is not iterable.");
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
        throw type_not_iterable_exception("type is not iterable.");
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
        auto& strings = enumeration.strings();
        if (_reverse) {
            for (auto it = strings.rbegin(); it != strings.rend(); ++it) {
                if (!_callback(nullptr, *it)) {
                    break;
                }
            }
            return;
        }
        for (auto& string : strings) {
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

    void iteration_visitor::operator()(values::break_iteration const& value) const
    {
        throw value.create_exception();
    }

}}}  // namespace puppet::runtime::values
