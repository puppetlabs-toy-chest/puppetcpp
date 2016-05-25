#include <puppet/runtime/values/value.hpp>
#include <boost/functional/hash.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    floating const floating::instance;

    floating::floating(double from, double to) :
        _from(from),
        _to(to)
    {
    }

    double floating::from() const
    {
        return _from;
    }

    double floating::to() const
    {
        return _to;
    }

    char const* floating::name()
    {
        return "Float";
    }

    values::type floating::generalize() const
    {
        return types::floating{};
    }

    bool floating::is_instance(values::value const& value, recursion_guard& guard) const
    {
        auto ptr = value.as<double>();
        if (!ptr) {
            return false;
        }
        return _to < _from ? (*ptr >= _to && *ptr <= _from) : (*ptr >= _from && *ptr <= _to);
    }

    bool floating::is_assignable(values::type const& other, recursion_guard& guard) const
    {
        auto ptr = boost::get<floating>(&other);
        if (!ptr) {
            return false;
        }
        return std::min(ptr->_from, ptr->_to) >= std::min(_from, _to) &&
               std::max(ptr->_from, ptr->_to) <= std::max(_from, _to);
    }

    void floating::write(ostream& stream, bool expand) const
    {
        stream << floating::name();
        // BUG: fix direct floating point comparison
        bool from_default = _from == numeric_limits<double>::lowest();
        bool to_default = _to == numeric_limits<double>::max();
        if (from_default && to_default) {
            // Only output the type name
            return;
        }
        stream << '[';
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

    values::value floating::instantiate(values::value from)
    {
        if (auto integer = from.as<int64_t>()) {
            return static_cast<double>(*integer);
        }
        if (auto floating = from.as<double>()) {
            return *floating;
        }
        if (auto boolean = from.as<bool>()) {
            return *boolean ? 1.0 : 0.0;
        }
        if (from.as<std::string>()) {
            auto string = from.move_as<std::string>();

            // Puppet supports whitespace between the sign and first digit and stod does not
            // Thus, we need to handle that by parsing the sign ourselves (sigh)

            // Skip leading whitespace
            size_t start = 0;
            bool negate = false;
            for(; start < string.size() && isspace(string[start]); ++start);

            // Check for the sign
            if (start < string.size()) {
                if (string[start] == '+') {
                    ++start;
                } else if (string[start] == '-') {
                    ++start;
                    negate = true;
                }
            }

            // Skip whitespace between sign and digit
            for(; start < string.size() && isspace(string[start]); ++start);

            // Check for duplicate sign characters
            if (start < string.size() && string[start] != '+' && string[start] != '-') {
                try {
                    size_t pos = 0;
                    int radix = 0;

                    // stod does not support integer prefixes, so check for them here (binary and hex only; octal is illegal)
                    if (start + 2 < string.size() && string[start] == '0') {
                        if (string[start + 1] == 'b' || string[start + 1] == 'B') {
                            start += 2;
                            radix = 2;
                        } else if (string[start + 1] == 'x' || string[start + 1] == 'X') {
                            start += 2;
                            radix = 16;
                        }

                        if (radix != 0) {
                            // Can't have whitespace following the prefix
                            if (start < string.size() && isspace(string[start])) {
                                // Set to less than 0 to indicate an invalid value
                                radix = -1;
                            }
                        }
                    }

                    if (radix >= 0) {
                        values::value value;
                        if (radix != 0) {
                            value = stoll(string.substr(start), &pos, radix) * (negate ? -1.0 : 1.0);
                        } else {
                            value = stod(start == 0 ? string : string.substr(start), &pos);
                        }

                        // The conversion must consume the entire string, but allow trailing whitespace
                        if (std::all_of(string.begin() + pos + start, string.end(), boost::is_space())) {
                            return value;
                        }
                    }
                } catch (invalid_argument const&) {
                } catch (out_of_range const&) {
                    throw values::type_conversion_exception(
                        (boost::format("string '%1%' is out of range for %2%.") %
                         string %
                         name()
                        ).str()
                    );
                }
            }
            throw values::type_conversion_exception(
                (boost::format("string '%1%' cannot be converted to %2%.") %
                 string %
                 name()
                ).str()
            );
        }
        throw values::type_conversion_exception(
            (boost::format("cannot convert %1% to %2%.") %
             from.infer_type() %
             name()
            ).str()
        );
    }

    ostream& operator<<(ostream& os, floating const& type)
    {
        type.write(os);
        return os;
    }

    bool operator==(floating const& left, floating const& right)
    {
        return left.from() == right.from() && left.to() == right.to();
    }

    bool operator!=(floating const& left, floating const& right)
    {
        return !(left == right);
    }

    size_t hash_value(floating const& type)
    {
        static const size_t name_hash = boost::hash_value(floating::name());

        size_t seed = 0;
        boost::hash_combine(seed, name_hash);
        boost::hash_combine(seed, type.from());
        boost::hash_combine(seed, type.to());
        return seed;
    }

}}}  // namespace puppet::runtime::types
