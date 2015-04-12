#include <puppet/runtime/yielder.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::lexer;

namespace puppet { namespace runtime {

    yielder::yielder(expression_evaluator& evaluator, token_position const& position, boost::optional<ast::lambda> const& lambda) :
        _evaluator(evaluator),
        _position(position),
        _lambda(lambda)
    {
    }

    bool yielder::lambda_given() const
    {
        return static_cast<bool>(_lambda);
    }

    value yielder::yield()
    {
        array arguments;
        return yield(arguments);
    }

    value yielder::yield(array& arguments)
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
                runtime::value value;

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
                            array captured;
                            captured.emplace_back(_evaluator.evaluate(*parameter.default_value()));
                            it = _default_cache.emplace(variable.name(), std::move(captured)).first;
                        }
                        // Copy the value from the cache
                        value = it->second;
                    } else {
                        array captured;
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

                // TODO: verify the value is of the parameter's type

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
                auto& expression = (* _lambda->body())[i];
                // The last expression in the block is allowed to be unproductive (i.e. the return value)
                result = _evaluator.evaluate(expression, i < ( _lambda->body()->size() - 1));
            }
        }
        return result;
    }

}}  // namespace puppet::runtime
