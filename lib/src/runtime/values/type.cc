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

    type type::parse(evaluation::context& context, string const& expression)
    {
        try {
            auto ast = parser::parse_string(expression);
            if (ast->statements.size() == 1) {
                evaluation::evaluator evaluator { context };
                auto result = evaluator.evaluate(ast->statements.front());
                if (result.as<type>()) {
                    return result.move_as<type>();
                }
            }
        } catch (parse_exception const&) {
        } catch (evaluation_exception const&) {
        }
        throw parse_exception((boost::format("the expression '%1%' is not a valid type specification.") % expression).str(), compiler::lexer::position(0, 1));
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
