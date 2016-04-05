#include <puppet/runtime/values/value.hpp>
#include <puppet/cast.hpp>
#include <boost/functional/hash.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    enumeration::enumeration(vector<std::string> strings) :
        _strings(rvalue_cast(strings))
    {
    }

    vector<std::string> const& enumeration::strings() const
    {
        return _strings;
    }

    char const* enumeration::name()
    {
        return "Enum";
    }

    bool enumeration::is_instance(values::value const& value) const
    {
        auto ptr = value.as<std::string>();
        if (!ptr) {
            return false;
        }
        if (_strings.empty()) {
            return true;
        }
        for (auto const& string : _strings) {
            // Enumerations are case sensative, so just use operator==
            if (string == *ptr) {
                return true;
            }
        }
        return false;
    }

    bool enumeration::is_specialization(values::type const& other) const
    {
        // Specializations of Enum have *fewer* strings (i.e. are more restrictive)
        auto ptr = boost::get<enumeration>(&other);
        if (!ptr) {
            return false;
        }
        return ptr->strings().size() < _strings.size();
    }

    bool enumeration::is_real(unordered_map<values::type const*, bool>& map) const
    {
        // Enum is a real type
        return true;
    }

    void enumeration::write(ostream& stream, bool expand) const
    {
        stream << enumeration::name();
        if (_strings.empty()) {
            return;
        }
        stream << '[';
        bool first = true;
        for (auto const& string : _strings) {
            if (first) {
                first = false;
            } else {
                stream << ", ";
            }
            stream << string;
        }
        stream << ']';
    }

    ostream& operator<<(ostream& os, enumeration const& type)
    {
        type.write(os);
        return os;
    }

    bool operator==(enumeration const& left, enumeration const& right)
    {
        auto& lstrings = left.strings();
        auto& rstrings = right.strings();

        if (lstrings.size() != rstrings.size()) {
            return false;
        }
        for (size_t i = 0; i < lstrings.size(); ++i) {
            if (lstrings[i] != rstrings[i]) {
                return false;
            }
        }
        return true;
    }

    bool operator!=(enumeration const& left, enumeration const& right)
    {
        return !(left == right);
    }

    size_t hash_value(enumeration const& type)
    {
        static const size_t name_hash = boost::hash_value(enumeration::name());

        size_t seed = 0;
        boost::hash_combine(seed, name_hash);
        boost::hash_range(seed, type.strings().begin(), type.strings().end());
        return seed;
    }

}}}  // namespace puppet::runtime::types
