#include <puppet/runtime/values/value.hpp>

using namespace std;

namespace puppet { namespace runtime { namespace values {

    hash::pair::pair(values::value key, values::value value) :
        _key(rvalue_cast(key)),
        _value(rvalue_cast(value))
    {
    }

    values::value const& hash::pair::key() const
    {
        return _key;
    }

    values::value& hash::pair::value()
    {
        return _value;
    }

    values::value const& hash::pair::value() const
    {
        return _value;
    }

    hash::hash(hash const& other)
    {
        operator=(other);
    }

    hash& hash::operator=(hash const& other)
    {
        // Copy the elements
        _elements = other._elements;

        // Rebuild the index as it is reference-based
        _index.clear();
        _index.reserve(_elements.size());
        for (auto it = _elements.begin(); it != _elements.end(); ++it) {
            _index.emplace(&it->key(), it);
        }
        return *this;
    }

    hash::iterator hash::begin()
    {
        return _elements.begin();
    }

    hash::const_iterator hash::begin() const
    {
        return _elements.begin();
    }

    hash::iterator hash::end()
    {
        return _elements.end();
    }

    hash::const_iterator hash::end() const
    {
        return _elements.end();
    }

    hash::const_iterator hash::cbegin() const
    {
        return _elements.cbegin();
    }

    hash::const_iterator hash::cend() const
    {
        return _elements.cend();
    }

    hash::reverse_iterator hash::rbegin()
    {
        return _elements.rbegin();
    }

    hash::const_reverse_iterator hash::rbegin() const
    {
        return _elements.rbegin();
    }

    hash::reverse_iterator hash::rend()
    {
        return _elements.rend();
    }

    hash::const_reverse_iterator hash::rend() const
    {
        return _elements.rend();
    }

    hash::const_reverse_iterator hash::crbegin() const
    {
        return _elements.crbegin();
    }

    hash::const_reverse_iterator hash::crend() const
    {
        return _elements.crend();
    }

    size_t hash::size() const
    {
        return _elements.size();
    }

    bool hash::empty() const
    {
        return _elements.empty();
    }

    void hash::set(value key, values::value value)
    {
        auto it = _index.find(&key);
        if (it != _index.end()) {
            it->second->value() = rvalue_cast(value);
            return;
        }
        auto element = _elements.emplace(_elements.end(), rvalue_cast(key), rvalue_cast(value));
        _index[&element->key()] = element;
    }

    void hash::set(const_iterator begin, const_iterator end)
    {
        while (begin != end) {
            set(begin->key(), begin->value());
            ++begin;
        }
    }

    value* hash::get(value const& key)
    {
        return const_cast<value*>(static_cast<hash const*>(this)->get(key));
    }

    value const* hash::get(value const& key) const
    {
        auto it = _index.find(&key);
        if (it == _index.end()) {
            return nullptr;
        }
        return &it->second->value();
    }

    bool hash::erase(value const& key)
    {
        auto it = _index.find(&key);
        if (it == _index.end()) {
            return false;
        }
        _elements.erase(it->second);
        _index.erase(it);
        return true;
    }

    size_t hash::indirect_hasher::operator()(values::value const* value) const
    {
        return hash_value(*value);
    }

    bool hash::indirect_comparer::operator()(values::value const* right, values::value const* left) const
    {
        return *right == *left;
    }

    ostream& operator<<(ostream& os, values::hash const& hash)
    {
        os << '{';
        bool first = true;
        for (auto const& kvp : hash) {
            if (first) {
                first = false;
            } else {
                os << ", ";
            }
            os << kvp.key() << " => " << kvp.value();
        }
        os << '}';
        return os;
    }

    bool operator==(hash const& left, hash const& right)
    {
        if (left.size() != right.size()) {
            return false;
        }
        for (auto const& kvp : left) {
            // Other hash must have the same key and the values must be equal
            auto other = right.get(kvp.key());
            if (!other || *other != kvp.value()) {
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
        static const size_t name_hash = boost::hash_value("hash");

        std::size_t seed = 0;
        boost::hash_combine(seed, name_hash);
        for (auto const& element : hash) {
            boost::hash_combine(seed, element.key());
            boost::hash_combine(seed, element.value());
        }
        return seed;
    }

}}}  // namespace puppet::runtime::values
