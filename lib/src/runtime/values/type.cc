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
            // Use an empty evaluation context that has no top scope
            // This will prevent evaluation of expressions that require access to the node, catalog, or scope
            evaluation::context context{ false };
            evaluation::evaluator evaluator { context };
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

}}}  // namespace puppet::runtime::values
