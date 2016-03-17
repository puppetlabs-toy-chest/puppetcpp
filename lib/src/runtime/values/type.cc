#include <puppet/runtime/values/value.hpp>
#include <puppet/compiler/parser/parser.hpp>
#include <puppet/compiler/evaluation/evaluator.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::compiler;

namespace puppet { namespace runtime { namespace values {

    type_variant& type::get()
    {
        return _value;
    }

    type_variant const& type::get() const
    {
        return _value;
    }

    struct is_instance_visitor : boost::static_visitor<bool>
    {
        explicit is_instance_visitor(values::value const& value) :
            _value(value)
        {
        }

        template <typename T>
        bool operator()(T const& type) const
        {
            return type.is_instance(_value);
        }

     private:
        values::value const& _value;
    };

    bool type::is_instance(values::value const& value) const
    {
        return boost::apply_visitor(is_instance_visitor(value), _value);
    }

    struct is_specialization_visitor : boost::static_visitor<bool>
    {
        explicit is_specialization_visitor(values::type const& type) :
            _type(type)
        {
        }

        template <typename T>
        bool operator()(T const& type) const
        {
            return type.is_specialization(_type);
        }

     private:
        values::type const& _type;
    };

    bool type::is_specialization(values::type const& type) const
    {
        return boost::apply_visitor(is_specialization_visitor(type), _value);
    }

    boost::optional<type> type::parse(string const& expression)
    {
        // Type specifications are postfix access expressions
        auto postfix = parser::parse_postfix(expression);
        if (!postfix) {
            return boost::none;
        }

        try {
            // Use an empty evaluation context
            // This will prevent evaluation of expressions that require access to the node, catalog, or scope
            evaluation::context context;
            evaluation::evaluator evaluator{ context };
            auto result = evaluator.evaluate(*postfix);
            if (result.as<type>()) {
                return result.move_as<type>();
            }
        } catch (evaluation_exception const&) {
        }
        return boost::none;
    }

    bool operator==(type const& left, type const& right)
    {
        return left.get() == right.get();
    }

    bool operator!=(type const& left, type const& right)
    {
        return !(left == right);
    }

    size_t hash_value(values::type const& type)
    {
        return hash_value(type.get());
    }

    void type_set::add(values::type const& type)
    {
        if (_set.emplace(&type).second) {
            _types.emplace_back(&type);
        }
    }

    void type_set::clear()
    {
        _types.clear();
        _set.clear();
    }

    bool type_set::empty() const
    {
        return _types.empty();
    }

    size_t type_set::size() const
    {
        return _types.size();
    }

    type const& type_set::operator[](size_t index) const
    {
        return *_types[index];
    }

    size_t type_set::indirect_hasher::operator()(values::type const* type) const
    {
        return hash_value(*type);
    }

    bool type_set::indirect_comparer::operator()(type const* right, type const* left) const
    {
        return *right == *left;
    }

    ostream& operator<<(ostream& os, type_set const& set)
    {
        auto count = set.size();
        for (size_t i = 0; i < count; ++i) {
            if (i > 0) {
                if (count > 2) {
                    os << ",";
                }
                os << " ";
                if (i == (count - 1)) {
                    os << "or ";
                }
            }
            os << set[i];
        }
        return os;
    }

}}}  // namespace puppet::runtime::values
