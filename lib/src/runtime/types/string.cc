#include <puppet/runtime/values/value.hpp>
#include <boost/functional/hash.hpp>
#include <algorithm>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    string const string::instance;

    string::string(int64_t from, int64_t to) :
        _from(from),
        _to(to)
    {
    }

    string::string(integer const& range) :
        _from(range.from()),
        _to(range.to())
    {
    }

    int64_t string::from() const
    {
        return _from;
    }

    int64_t string::to() const
    {
        return _to;
    }

    char const* string::name()
    {
        return "String";
    }

    values::type string::generalize() const
    {
        return types::string{};
    }

    bool string::is_instance(values::value const& value, recursion_guard& guard) const
    {
        auto ptr = value.as<std::string>();
        if (!ptr) {
            return false;
        }
        auto size = static_cast<int64_t>(ptr->size());
        return _to < _from ? (size >= _to && size <= _from) : (size >= _from && size <= _to);
    }

    bool string::is_assignable(values::type const& other, recursion_guard& guard) const
    {
        int64_t from, to;
        if (auto string = boost::get<types::string>(&other)) {
            from = string->from();
            to = string->to();
        } else if (boost::get<types::pattern>(&other)) {
            return _from >= 0 && _to >= 0;
        } else if (auto enumeration = boost::get<types::enumeration>(&other)) {
            if (enumeration->strings().empty()) {
                return _from >= 0 && _to >= 0;
            }
            auto minmax = minmax_element(enumeration->strings().begin(), enumeration->strings().end(), [&](auto const& left, auto const& right) {
                return left.size() < right.size();
            });
            from = minmax.first->size();
            to = minmax.second->size();
        } else {
            return false;
        }
        return std::min(from, to) >= std::min(_from, _to) && std::max(from, to) <= std::max(_from, _to);
    }

    void string::write(ostream& stream, bool expand) const
    {
        stream << string::name();
        bool from_default = _from == 0;
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

    ostream& operator<<(ostream& os, string const& type)
    {
        type.write(os);
        return os;
    }

    bool operator==(string const& left, string const& right)
    {
        return left.from() == right.from() && left.to() == right.to();
    }

    bool operator!=(string const& left, string const& right)
    {
        return !(left == right);
    }

    size_t hash_value(string const& type)
    {
        static const size_t name_hash = boost::hash_value(string::name());

        size_t seed = 0;
        boost::hash_combine(seed, name_hash);
        boost::hash_combine(seed, type.from());
        boost::hash_combine(seed, type.to());
        return seed;
    }

}}}  // namespace puppet::runtime::types
