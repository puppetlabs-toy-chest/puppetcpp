#include <puppet/runtime/yielder.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime {

    yielder::yielder(expression_evaluator& evaluator, token_position const& position, boost::optional<ast::lambda> const& lambda) :
        _evaluator(evaluator),
        _position(position),
        _lambda(lambda)
    {
    }

    token_position const& yielder::position() const
    {
        if (!_lambda) {
            return _position;
        }
        return _lambda->position();
    }

    token_position const& yielder::position(size_t index) const
    {
        if (!_lambda || !_lambda->parameters()) {
            return position();
        }
        if (index >= _lambda->parameters()->size()) {
            throw runtime_error("parameter index out of range.");
        }
        return (*_lambda->parameters())[index].position();
    }

    bool yielder::lambda_given() const
    {
        return static_cast<bool>(_lambda);
    }

    size_t yielder::parameter_count() const
    {
        if (!_lambda || !_lambda->parameters()) {
            return 0;
        }
        return _lambda->parameters()->size();
    }

    value yielder::yield()
    {
        values::array arguments;
        return yield(arguments);
    }

    value yielder::yield(values::array& arguments)
    {
        if (!lambda_given()) {
            throw evaluation_exception(_position, "function call requires a lambda but one was not given.");
        }

        unordered_map<string, value> variables;
        bool has_optional_parameters = false;
        if (_lambda->parameters()) {
            for (size_t i = 0; i < _lambda->parameters()->size(); ++i) {
                auto& parameter = (*_lambda->parameters())[i];
                auto& variable = parameter.variable();
                values::value value;

                // Check for capture
                if (parameter.captures()) {
                    if (i != _lambda->parameters()->size() - 1) {
                        throw evaluation_exception(parameter.position(), (boost::format("parameter $%1% \"captures rest\" but is not the last parameter.") % variable.name()).str());
                    }
                    // Last parameter captures the remainder
                    if (parameter.default_value()) {
                        // Check the default value cache
                        auto it = _default_cache.find(variable.name());
                        if (it == _default_cache.end()) {
                            values::array captured;
                            captured.emplace_back(_evaluator.evaluate(*parameter.default_value()));
                            it = _default_cache.emplace(variable.name(), std::move(captured)).first;
                        }
                        // Copy the value from the cache
                        value = it->second;
                    } else {
                        values::array captured;
                        if (i < arguments.size()) {
                            captured.reserve(arguments.size() - i);
                            captured.insert(captured.end(), std::make_move_iterator(arguments.begin() + i), std::make_move_iterator(arguments.end()));
                        }
                        value = std::move(captured);
                    }
                } else {
                    if (has_optional_parameters && !parameter.default_value()) {
                        throw evaluation_exception(parameter.position(), (boost::format("parameter $%1% is required but appears after optional parameters.") % variable.name()).str());
                    }

                    has_optional_parameters = static_cast<bool>(parameter.default_value());

                    if (i >= arguments.size()) {
                        // Check for not present and without a default value
                        if (!parameter.default_value()) {
                            throw evaluation_exception(parameter.position(), (boost::format("parameter $%1% is required but no value was given.") % variable.name()).str());
                        }

                        // First check the default value cache
                        auto it = _default_cache.find(variable.name());
                        if (it == _default_cache.end()) {
                            it = _default_cache.emplace(variable.name(), _evaluator.evaluate(*parameter.default_value())).first;
                        }

                        // Copy the value from the cache
                        value = it->second;
                    } else {
                        // Use the supplied argument
                        value = std::move(arguments[i]);
                    }
                }

                // Verify the value matches the parameter type
                if (parameter.type()) {
                    auto result = _evaluator.evaluate(*parameter.type());
                    auto type = boost::get<values::type>(&result);
                    if (!type) {
                        throw evaluation_exception(parameter.position(), (boost::format("expected %1% for parameter type but found %2%.") % types::type::name() % get_type(type)).str());
                    }
                    if (!is_instance(dereference(value), *type)) {
                        throw evaluation_exception(parameter.position(), (boost::format("parameter $%1% has expected type %2% but was given %3%.") % variable.name() % *type % get_type(value)).str());
                    }
                }

                if (!variables.emplace(variable.name(), std::move(value)).second) {
                    throw evaluation_exception(parameter.position(), (boost::format("parameter $%1% already exists in the parameter list.") % variable.name()).str());
                }
            }
        }

        // If there's a lambda body, evaluate
        value result;
        if (_lambda->body()) {
            // Create an ephemeral scope
            ephemeral_scope ephemeral(_evaluator.context());
            auto& current_scope = _evaluator.context().current();

            // Set the variables
            for (auto& variable : variables) {
                current_scope.set(variable.first, std::move(variable.second));
            }

            // Evaluate the lambda body
            for (size_t i = 0; i < _lambda->body()->size(); ++i) {
                auto& expression = (*_lambda->body())[i];
                // The last expression in the block is allowed to be unproductive (i.e. the return value)
                result = _evaluator.evaluate(expression, i < ( _lambda->body()->size() - 1));
            }

            // If the result is a variable in the current scope (or a match variable), it's ephemeral and needs to be copied
            if (auto ptr = boost::get<values::variable>(&result)) {
                if (ptr->match() || _evaluator.context().current().get(ptr->name())) {
                    result = dereference(result);
                }
            }
        }

        return result;
    }

}}  // namespace puppet::runtime
