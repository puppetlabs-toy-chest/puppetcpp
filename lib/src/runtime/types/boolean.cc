#include <puppet/runtime/values/value.hpp>
#include <boost/functional/hash.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    boolean const boolean::instance{};

    char const* boolean::name()
    {
        return "Boolean";
    }

    values::type boolean::generalize() const
    {
        return *this;
    }

    bool boolean::is_instance(values::value const& value, recursion_guard& guard) const
    {
        return value.as<bool>();
    }

    bool boolean::is_assignable(values::type const& other, recursion_guard& guard) const
    {
        return boost::get<boolean>(&other);
    }

    void boolean::write(ostream& stream, bool expand) const
    {
        stream << boolean::name();
    }

    values::value boolean::instantiate(values::value from)
    {
        if (auto integer = from.as<int64_t>()) {
            return *integer != 0;
        }
        if (auto floating = from.as<double>()) {
            return *floating != 0;
        }
        if (auto boolean = from.as<bool>()) {
            return *boolean;
        }
        if (from.as<std::string>()) {
            auto string = from.move_as<std::string>();
            boost::to_lower(string);

            if (string == "true" || string == "yes" || string == "y") {
                return true;
            }
            if (string == "false" || string == "no" || string == "n") {
                return false;
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

    ostream& operator<<(ostream& os, boolean const& type)
    {
        type.write(os);
        return os;
    }

    bool operator==(boolean const&, boolean const&)
    {
        return true;
    }

    bool operator!=(boolean const& left, boolean const& right)
    {
        return !(left == right);
    }

    size_t hash_value(boolean const&)
    {
        static const size_t name_hash = boost::hash_value(boolean::name());

        size_t seed = 0;
        boost::hash_combine(seed, name_hash);
        return seed;
    }

}}}  // namespace puppet::runtime::types
