#include <puppet/runtime/values/value.hpp>
#include <boost/functional/hash.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace values {

    void array::join(ostream& os, string const& separator) const
    {
        bool first = true;
        for (auto const& element : *this) {
            if (first) {
                first = false;
            } else {
                os << separator;
            }
            os << *element;
        }
    }

    ostream& operator<<(ostream& os, values::array const& array)
    {
        os << '[';
        bool first = true;
        for (auto const& element : array) {
            if (first) {
                first = false;
            } else {
                os << ", ";
            }
            os << element;
        }
        os << ']';
        return os;
    }

    bool operator==(array const& left, array const& right)
    {
        if (left.size() != right.size()) {
            return false;
        }
        for (size_t i = 0; i < left.size(); ++i) {
            if (left[i] != right[i]) {
                return false;
            }
        }
        return true;
    }

    bool operator!=(array const& left, array const& right)
    {
        return !(left == right);
    }

    size_t hash_value(values::array const& array)
    {
        std::size_t seed = 0;

        for (auto const& element : array) {
            boost::hash_combine(seed, element);
        }
        return seed;
    }

}}}  // namespace puppet::runtime::values
