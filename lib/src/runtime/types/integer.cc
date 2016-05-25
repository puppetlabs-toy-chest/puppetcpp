#include <puppet/runtime/values/value.hpp>
#include <boost/functional/hash.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    integer const integer::instance;

    integer::integer(int64_t from, int64_t to) :
        _from(from),
        _to(to)
    {
        if (from > to) {
            throw runtime_error("from cannot be greater than to.");
        }
    }

    int64_t integer::from() const
    {
        return _from;
    }

    int64_t integer::to() const
    {
        return _to;
    }

    char const* integer::name()
    {
        return "Integer";
    }

    bool integer::iterable() const
    {
        return _from != numeric_limits<int64_t>::min() && _to != numeric_limits<int64_t>::max();
    }

    void integer::each(function<bool(int64_t, int64_t)> const& callback) const
    {
        if (!callback || !iterable()) {
            return;
        }

        // Check if we should go downwards
        bool backwards = _to < _from;
        for (int64_t index = 0, start = _from; (backwards && (start >= _to)) || (!backwards && (start <= _to)); ++index, start += (backwards ? -1 : 1)) {
            if (!callback(index, start)) {
                break;
            }
        }
    }

    values::type integer::generalize() const
    {
        return types::integer{};
    }

    bool integer::is_instance(values::value const& value, recursion_guard& guard) const
    {
        auto ptr = value.as<int64_t>();
        if (!ptr) {
            return false;
        }
        return _to < _from ? (*ptr >= _to && *ptr <= _from) : (*ptr >= _from && *ptr <= _to);
    }

    bool integer::is_assignable(values::type const& other, recursion_guard& guard) const
    {
        // Check for an Integer with a range inside of this type's range
        auto ptr = boost::get<integer>(&other);
        if (!ptr) {
            return false;
        }
        return std::min(ptr->from(), ptr->to()) >= std::min(_from, _to) &&
               std::max(ptr->from(), ptr->to()) <= std::max(_from, _to);
    }

    void integer::write(ostream& stream, bool expand) const
    {
        stream << integer::name();
        bool from_default = _from == numeric_limits<int64_t>::min();
        bool to_default = _to == numeric_limits<int64_t>::max();
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

    values::value integer::instantiate(values::value from, int radix)
    {
        values::value value;
        if (auto integer = from.as<int64_t>()) {
            value = *integer;
        } else if (auto floating = from.as<double>()) {
            value = static_cast<int64_t>(*floating);
        } else if (auto boolean = from.as<bool>()) {
            value = static_cast<int64_t>(*boolean ? 1 : 0);
        } else if (from.as<std::string>()) {
            auto string = from.move_as<std::string>();

            // Puppet supports whitespace between the sign and first digit and stoll does not
            // Thus, we need to handle that by parsing the sign ourselves (sigh)

            // Skip leading whitespace
            size_t start = 0;
            bool negate = false;
            bool invalid = false;
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

            // Check for now invalid sign characters
            if (start < string.size() && (string[start] == '+' || string[start] == '-')) {
                invalid = true;
            }

            if (!invalid) {
                try {
                    size_t pos = 0;

                     if (radix == 0) {
                        // stroll does not support a binary prefix, so check for one here
                        if (start + 2 < string.size() && string[start] == '0' && (string[start + 1] == 'b' || string[start + 1] == 'B')) {
                            start += 2;
                            radix = 2;

                            // Can't have whitespace following the prefix
                            if (start < string.size() && isspace(string[start])) {
                                invalid = true;
                            }
                        }
                    }

                    if (!invalid) {
                        value = static_cast<int64_t>(stoll(start == 0 ? string : string.substr(start), &pos, radix)) * (negate ? -1 : 1);

                        // The conversion must consume the entire string, but allow trailing whitespace
                        if (!std::all_of(string.begin() + pos + start, string.end(), boost::is_space())) {
                            invalid = true;
                        }
                   }
                } catch (invalid_argument const&) {
                    invalid = true;
                } catch (out_of_range const&) {
                    throw values::type_conversion_exception(
                        (boost::format("string '%1%' is out of range for %2%.") %
                         string %
                         name()
                        ).str()
                    );
                }
            }
            if (invalid) {
                throw values::type_conversion_exception(
                    (boost::format("string '%1%' cannot be converted to %2%.") %
                     string %
                     name()
                    ).str()
                );
            }
        } else {
            throw values::type_conversion_exception(
                (boost::format("cannot convert %1% to %2%.") %
                 from.infer_type() %
                 name()
                ).str()
            );
        }
        return value;
    }

    ostream& operator<<(ostream& os, integer const& type)
    {
        type.write(os);
        return os;
    }

    bool operator==(integer const& left, integer const& right)
    {
        return left.from() == right.from() && left.to() == right.to();
    }

    bool operator!=(integer const& left, integer const& right)
    {
        return !(left == right);
    }

    size_t hash_value(integer const& type)
    {
        static const size_t name_hash = boost::hash_value(integer::name());

        size_t seed = 0;
        boost::hash_combine(seed, name_hash);
        boost::hash_combine(seed, type.from());
        boost::hash_combine(seed, type.to());
        return seed;
    }

}}}  // namespace puppet::runtime::types
