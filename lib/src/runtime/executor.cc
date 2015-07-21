#include <puppet/runtime/executor.hpp>
#include <puppet/cast.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;
using boost::optional;

namespace puppet { namespace runtime {

    executor::executor(expression_evaluator& evaluator, lexer::position const& position, optional<vector<ast::parameter>> const& parameters, optional<vector<ast::expression>> const& body) :
        _evaluator(evaluator),
        _position(position),
        _parameters(parameters),
        _body(body)
    {
    }

    lexer::position const& executor::position() const
    {
        return _position;
    }

    lexer::position const& executor::position(size_t index) const
    {
        if (index >= parameter_count()) {
            throw runtime_error("parameter index out of range.");
        }
        return (*_parameters)[index].position();
    }

    size_t executor::parameter_count() const
    {
        if (!_parameters) {
            return 0;
        }
        return _parameters->size();
    }

    value executor::execute(shared_ptr<runtime::scope> const& scope) const
    {
        values::array arguments;
        return execute(arguments, scope);
    }

    value executor::execute(values::array& arguments, shared_ptr<runtime::scope> const& scope) const
    {
        // Create the execution scope
        auto local_scope = _evaluator.context().create_local_scope(scope);
        auto& current_scope = _evaluator.context().current_scope();

        bool has_optional_parameters = false;
        if (_parameters) {
            for (size_t i = 0; i < _parameters->size(); ++i) {
                auto const& parameter = (*_parameters)[i];
                auto const& name = parameter.variable().name();
                values::value value;

                // Check for capture
                if (parameter.captures()) {
                    if (i != _parameters->size() - 1) {
                        throw evaluation_exception(parameter.position(), (boost::format("parameter $%1% \"captures rest\" but is not the last parameter.") % name).str());
                    }
                    values::array captured;
                    if (i < arguments.size()) {
                        captured.reserve(arguments.size() - i);
                        captured.insert(captured.end(), std::make_move_iterator(arguments.begin() + i), std::make_move_iterator(arguments.end()));
                    } else if (parameter.default_value()) {
                        captured.emplace_back(_evaluator.evaluate(*parameter.default_value()));
                    }
                    value = rvalue_cast(captured);
                } else {
                    if (has_optional_parameters && !parameter.default_value()) {
                        throw evaluation_exception(parameter.position(), (boost::format("parameter $%1% is required but appears after optional parameters.") % name).str());
                    }

                    has_optional_parameters = static_cast<bool>(parameter.default_value());

                    if (i < arguments.size()) {
                        // Use the supplied argument
                        value = rvalue_cast(arguments[i]);
                    } else {
                        // Check for not present and without a default value
                        if (!parameter.default_value()) {
                            throw evaluation_exception(parameter.position(), (boost::format("parameter $%1% is required but no value was given.") % name).str());
                        }

                        value = _evaluator.evaluate(*parameter.default_value());
                    }
                }

                // Verify the value matches the parameter type
                validate_parameter(parameter, value);

                if (!current_scope->set(name, rvalue_cast(value), _evaluator.path(), parameter.position().line())) {
                    throw evaluation_exception(parameter.position(), (boost::format("parameter $%1% already exists in the parameter list.") % name).str());
                }
            }
        }

        return evaluate_body();
    }

    values::value executor::execute(values::hash& arguments, shared_ptr<runtime::scope> const& scope) const
    {
        // Create the execution scope
        auto local_scope = _evaluator.context().create_local_scope(scope);
        auto& current_scope = _evaluator.context().current_scope();

        if (_parameters) {
            for (auto const& parameter : *_parameters) {
                auto const& name = parameter.variable().name();

                // The parameter must either have been given an argument or have a default value
                values::value value;
                auto it = arguments.find(name);
                if (it == arguments.end()) {
                    if (!parameter.default_value()) {
                        throw evaluation_exception(parameter.position(), (boost::format("parameter $%1% is required but no value was given.") % name).str());
                    }
                    value = _evaluator.evaluate(*parameter.default_value());
                } else {
                    value = rvalue_cast(it->second);
                }

                validate_parameter(parameter, value);

                if (!current_scope->set(name, rvalue_cast(value), _evaluator.path(), parameter.position().line())) {
                    throw evaluation_exception(parameter.position(), (boost::format("parameter $%1% already exists in the parameter list.") % name).str());
                }
            }
        }
        return evaluate_body();
    }

    void executor::validate_parameter(ast::parameter const& parameter, values::value const& value) const
    {
        if (!parameter.type()) {
            return;
        }

        // Verify the value matches the parameter type
        auto result = _evaluator.evaluate(*parameter.type());
        auto type = as<values::type>(result);
        if (!type) {
            throw evaluation_exception(parameter.position(), (boost::format("expected %1% for parameter type but found %2%.") % types::type::name() % get_type(type)).str());
        }
        if (!is_instance(value, *type)) {
            throw evaluation_exception(parameter.position(), (boost::format("parameter $%1% has expected type %2% but was given %3%.") % parameter.variable().name() % *type % get_type(value)).str());
        }
    }

    values::value executor::evaluate_body() const
    {
        // Evaluate the body
        value result;
        if (_body) {
            for (size_t i = 0; i < _body->size(); ++i) {
                auto& expression = (*_body)[i];
                // The last expression in the block is allowed to be unproductive (i.e. the return value)
                result = _evaluator.evaluate(expression, i < (_body->size() - 1));
            }
        }
        // Return a mutated result in case we're returning a local variable
        return mutate(result);
    }

}}  // namespace puppet::runtime
