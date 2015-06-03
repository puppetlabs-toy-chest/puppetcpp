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

    value executor::execute() const
    {
        values::array arguments;
        return execute(arguments);
    }

    value executor::execute(values::array& arguments) const
    {
        // Create an ephemeral scope
        ephemeral_scope ephemeral(_evaluator.context());

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
                if (parameter.type()) {
                    auto result = _evaluator.evaluate(*parameter.type());
                    auto type = as<values::type>(result);
                    if (!type) {
                        throw evaluation_exception(parameter.position(), (boost::format("expected %1% for parameter type but found %2%.") % types::type::name() % get_type(type)).str());
                    }
                    if (!is_instance(value, *type)) {
                        throw evaluation_exception(parameter.position(), (boost::format("parameter $%1% has expected type %2% but was given %3%.") % name % *type % get_type(value)).str());
                    }
                }

                if (!_evaluator.context().scope().set(name, rvalue_cast(value), parameter.position().line())) {
                    throw evaluation_exception(parameter.position(), (boost::format("parameter $%1% already exists in the parameter list.") % name).str());
                }
            }
        }

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
