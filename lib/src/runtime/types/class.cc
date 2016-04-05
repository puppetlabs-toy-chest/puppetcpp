#include <puppet/runtime/values/value.hpp>
#include <puppet/cast.hpp>
#include <boost/functional/hash.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace types {

    klass const klass::instance;

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

    bool klass::is_instance(values::value const& value, recursion_guard& guard) const
    {
        auto ptr = value.as<values::type>();
        if (!ptr) {
            return false;
        }
        auto class_ptr = boost::get<klass>(ptr);
        if (!class_ptr) {
            return false;
        }
        return _title.empty() || _title == class_ptr->title();
    }

    bool klass::is_assignable(values::type const& other, recursion_guard& guard) const
    {
        auto ptr = boost::get<klass>(&other);
        if (!ptr) {
            return false;
        }
        return _title.empty() || _title == ptr->title();
    }

    void klass::write(ostream& stream, bool expand) const
    {
        stream << klass::name();
        if (_title.empty()) {
            return;
        }
        stream << "[" << _title << "]";
    }

    void klass::normalize(std::string& name)
    {
        if (boost::starts_with(name, "::")) {
            name = name.substr(2);
        }

        // Now lowercase every start of a type name
        boost::split_iterator<std::string::iterator> end;
        for (auto it = boost::make_split_iterator(name, boost::first_finder("::", boost::is_equal())); it != end; ++it) {
            if (!*it) {
                continue;
            }
            auto range = boost::make_iterator_range(it->begin(), it->begin() + 1);
            boost::to_lower(range);
        }
    }

    ostream& operator<<(ostream& os, klass const& type)
    {
        type.write(os);
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
