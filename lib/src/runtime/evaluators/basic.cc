#include <puppet/runtime/evaluators/basic.hpp>
#include <puppet/runtime/string_interpolator.hpp>
#include <puppet/ast/expression_def.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime { namespace evaluators {

    basic_expression_evaluator::basic_expression_evaluator(expression_evaluator& evaluator, ast::basic_expression const& expression) :
        _evaluator(evaluator),
        _expression(expression)
    {
    }

    basic_expression_evaluator::result_type basic_expression_evaluator::evaluate()
    {
        return boost::apply_visitor(*this, _expression);
    }

    basic_expression_evaluator::result_type basic_expression_evaluator::operator()(ast::undef const&)
    {
        return value();
    }

    basic_expression_evaluator::result_type basic_expression_evaluator::operator()(ast::defaulted const&)
    {
        return defaulted();
    }

    basic_expression_evaluator::result_type basic_expression_evaluator::operator()(ast::boolean const& boolean)
    {
        return boolean.value();
    }

    basic_expression_evaluator::result_type basic_expression_evaluator::operator()(int64_t integer)
    {
        return integer;
    }

    basic_expression_evaluator::result_type basic_expression_evaluator::operator()(long double floating)
    {
        return floating;
    }

    basic_expression_evaluator::result_type basic_expression_evaluator::operator()(ast::number const& number)
    {
        return boost::apply_visitor(*this, number.value());
    }

    basic_expression_evaluator::result_type basic_expression_evaluator::operator()(ast::string const& str)
    {
        string_interpolator interpolator(_evaluator);
        return interpolator.interpolate(str.position(), str.value(), str.escapes(), str.quote(), str.interpolated(), str.margin(), str.remove_break());
    }

    basic_expression_evaluator::result_type basic_expression_evaluator::operator()(ast::regex const& regx)
    {
        try {
            return values::regex(regx.value());
        } catch (std::regex_error const& ex) {
            throw evaluation_exception(regx.position(), ex.what());
        }
    }

    basic_expression_evaluator::result_type basic_expression_evaluator::operator()(ast::variable const& var)
    {
        static const std::regex match_variable_patterh("^\\d+$");

        auto& name = var.name();
        auto& scope = _evaluator.context().scope();

        bool match = false;
        value const* val = nullptr;
        if (regex_match(name, match_variable_patterh)) {
            // Check for invalid match name
            if (name.size() > 1 && name[0] == '0') {
                throw evaluation_exception(var.position(), (boost::format("variable name $%1% is not a valid match variable name.") % var.name()).str());
            }
            // Look up the match
            val = scope.get(stoi(name));
            match = true;
        } else {
            val = scope.get(name);
        }
        return variable(name, val, match);
    }

    basic_expression_evaluator::result_type basic_expression_evaluator::operator()(ast::name const& name)
    {
        // Treat as a string
        return name.value();
    }

    basic_expression_evaluator::result_type basic_expression_evaluator::operator()(ast::bare_word const& word)
    {
        // Treat as a string
        return word.value();
    }

    basic_expression_evaluator::result_type basic_expression_evaluator::operator()(ast::type const& type)
    {
        static const unordered_map<string, values::type> names = {
            { types::any::name(),           types::any() },
            { types::array::name(),         types::array() },
            { types::boolean::name(),       types::boolean() },
            { types::callable::name(),      types::callable() },
            { types::catalog_entry::name(), types::catalog_entry() },
            { types::klass::name(),         types::klass() },
            { types::collection::name(),    types::collection() },
            { types::data::name(),          types::data() },
            { types::defaulted::name(),     types::defaulted() },
            { types::enumeration::name(),   types::enumeration() },
            { types::floating::name(),      types::floating() },
            { types::hash::name(),          types::hash() },
            { types::integer::name(),       types::integer() },
            { types::klass::name(),         types::klass() },
            { types::numeric::name(),       types::numeric() },
            { types::optional::name(),      types::optional(boost::none) },
            { types::pattern::name(),       types::pattern() },
            { types::regexp::name(),        types::regexp() },
            { types::resource::name(),      types::resource() },
            { types::runtime::name(),       types::runtime() },
            { types::scalar::name(),        types::scalar() },
            { types::string::name(),        types::string() },
            { types::structure::name(),     types::structure() },
            { types::tuple::name(),         types::tuple() },
            { types::type::name(),          types::type(boost::none) },
            { types::undef::name(),         types::undef() },
            { types::variant::name(),       types::variant() },
        };

        auto it = names.find(type.name());
        if (it == names.end()) {
            // Assume the unknown type is a resource
            return types::resource(type.name());
        }
        return it->second;
    }

    basic_expression_evaluator::result_type basic_expression_evaluator::operator()(ast::array const& array)
    {
        values::array new_array;

        if (array.elements()) {
            for (auto& element : *array.elements()) {
                auto result = _evaluator.evaluate(element);

                // If unfolding, append the array's elements
                auto unfold_array = _evaluator.unfold(element, result);
                if (unfold_array) {
                    new_array.reserve(new_array.size() + unfold_array->size());
                    new_array.insert(new_array.end(), std::make_move_iterator(unfold_array->begin()), std::make_move_iterator(unfold_array->end()));
                    continue;
                }
                new_array.emplace_back(rvalue_cast(result));
            }
        }
        return new_array;
    }

    basic_expression_evaluator::result_type basic_expression_evaluator::operator()(ast::hash const& hash)
    {
        values::hash new_hash;

        if (hash.elements()) {
            for (auto& element : *hash.elements()) {
                new_hash.emplace(_evaluator.evaluate(element.first), _evaluator.evaluate(element.second));
            }
        }
        return new_hash;
    }

}}}  // namespace puppet::runtime::evaluators
