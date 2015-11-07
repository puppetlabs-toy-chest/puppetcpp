#include <puppet/runtime/values/value.hpp>
#include <puppet/cast.hpp>
#include <boost/functional/hash.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    klass::klass(std::string title) :
        _title(rvalue_cast(title))
    {
        normalize(_title);
    }

    std::string const& klass::title() const
    {
        return _title;
    }

    bool klass::fully_qualified() const
    {
        return !_title.empty();
    }

    char const* klass::name()
    {
        return "Class";
    }

    bool klass::is_instance(values::value const& value) const
    {
        // Check for type
        auto ptr = value.as<values::type>();
        if (!ptr) {
            return false;
        }
        // Check for class type
        auto class_ptr = boost::get<klass>(ptr);
        if (!class_ptr) {
            return false;
        }
        return _title.empty() || _title == class_ptr->title();
    }

    bool klass::is_specialization(values::type const& other) const
    {
        // If the class has a specialization, the other type cannot be one
        if (!_title.empty()) {
            return false;
        }
        // Check that the other Class is specialized
        auto class_ptr = boost::get<klass>(&other);
        if (!class_ptr) {
            // Not the same type
            return false;
        }
        // Otherwise, the other one is a specialization if it has a title
        return !class_ptr->title().empty();
    }

    void klass::normalize(std::string& name)
    {
        if (boost::starts_with(name, "::")) {
            name = name.substr(2);
        }

        boost::to_lower(name);
    }

    ostream& operator<<(ostream& os, klass const& type)
    {
        os << klass::name();
        if (type.title().empty()) {
            return os;
        }
        os << "[" << type.title() << "]";
        return os;
    }

    bool operator==(klass const& left, klass const& right)
    {
        return left.title() == right.title();
    }

    bool operator!=(klass const& left, klass const& right)
    {
        return !(left == right);
    }

    size_t hash_value(klass const& type)
    {
        static const size_t name_hash = boost::hash_value(klass::name());

        size_t seed = 0;
        boost::hash_combine(seed, name_hash);
        boost::hash_combine(seed, type.title());
        return seed;
    }

}}}  // namespace puppet::runtime::types
