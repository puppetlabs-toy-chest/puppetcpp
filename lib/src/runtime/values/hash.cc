#include <puppet/runtime/values/value.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace values {

    ostream& operator<<(ostream& os, values::hash const& hash)
    {
        os << '{';
        bool first = true;
        for (auto const& element : hash) {
            if (first) {
                first = false;
            } else {
                os << ", ";
            }
            os << element.first << " => " << element.second;
        }
        os << '}';
        return os;
    }

    bool operator==(hash const& left, hash const& right)
    {
        if (left.size() != right.size()) {
            return false;
        }
        for (auto const& element : left) {
            // Other hash must have the same key
            auto other = right.find(element.first);
            if (other == right.end()) {
                return false;
            }
            // Values must be equal
            if (element.second != other->second) {
                return false;
            }
        }
        return true;
    }

    bool operator!=(hash const& left, hash const& right)
    {
        return !(left == right);
    }

    size_t hash_value(values::hash const& hash)
    {
        std::size_t seed = 0;

        for (auto const& element : hash) {
            boost::hash_combine(seed, element.first);
            boost::hash_combine(seed, element.second);
        }
        return seed;
    }

}}}  // namespace puppet::runtime::values
