#include <puppet/runtime/types/enumeration.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    enumeration::enumeration(vector<string> strings) :
        _strings(std::move(strings))
    {
    }

    vector<string> const& enumeration::strings() const
    {
        return _strings;
    }

    const char* enumeration::name()
    {
        return "Enum";
    }

    ostream& operator<<(ostream& os, enumeration const& type)
    {
        os << enumeration::name();
        if (type.strings().empty()) {
            return os;
        }
        os << '[';
        bool first = true;
        for (auto const& string : type.strings()) {
            if (first) {
                first = false;
            } else {
                os << ", ";
            }
            os << string;
        }
        os << ']';
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

}}}  // namespace puppet::runtime::types
