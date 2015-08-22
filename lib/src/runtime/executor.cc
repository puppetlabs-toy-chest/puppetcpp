#include <puppet/runtime/executor.hpp>
#include <puppet/cast.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;
using boost::optional;

namespace puppet { namespace runtime {

    argument_exception::argument_exception(string const& message, size_t index) :
        runtime_error(message),
        _index(index)
    {
    }

    size_t argument_exception::index() const
    {
        return _index;
    }

    attribute_name_exception::attribute_name_exception(string const& message, string name) :
        runtime_error(message),
        _name(rvalue_cast(name))
    {
    }

    string const& attribute_name_exception::name() const
    {
        return _name;
    }

    attribute_value_exception::attribute_value_exception(string const& message, string name) :
        runtime_error(message),
        _name(rvalue_cast(name))
    {
    }

    string const& attribute_value_exception::name() const
    {
        return _name;
    }

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
        auto local_scope = _evaluator.evaluation_context().create_local_scope(scope);
        auto& current_scope = _evaluator.evaluation_context().current_scope();

        bool has_optional_parameters = false;
        if (_parameters) {
            for (size_t i = 0; i < _parameters->size(); ++i) {
                auto const& parameter = (*_parameters)[i];
                auto const& name = parameter.variable().name();
                auto position = parameter.position();

                // Check for capture
                values::value value;
                if (parameter.captures()) {
                    if (i != _parameters->size() - 1) {
                        throw _evaluator.create_exception(parameter.position(), (boost::format("parameter $%1% \"captures rest\" but is not the last parameter.") % name).str());
                    }
                    values::array captured;
                    if (i < arguments.size()) {
                        captured.reserve(arguments.size() - i);
                        captured.insert(captured.end(), std::make_move_iterator(arguments.begin() + i), std::make_move_iterator(arguments.end()));
                    } else if (parameter.default_value()) {
                        captured.emplace_back(_evaluator.evaluate(*parameter.default_value()));
                        position = parameter.default_value()->position();
                    }
                    value = rvalue_cast(captured);
                } else {
                    if (has_optional_parameters && !parameter.default_value()) {
                        throw _evaluator.create_exception(parameter.position(), (boost::format("parameter $%1% is required but appears after optional parameters.") % name).str());
                    }

                    has_optional_parameters = static_cast<bool>(parameter.default_value());

                    if (i < arguments.size()) {
                        // Use the supplied argument
                        value = rvalue_cast(arguments[i]);
                    } else {
                        // Check for not present and without a default value
                        if (!parameter.default_value()) {
                            throw _evaluator.create_exception(parameter.position(), (boost::format("parameter $%1% is required but no value was given.") % name).str());
                        }

                        value = _evaluator.evaluate(*parameter.default_value());
                        position = parameter.default_value()->position();
                    }
                }

                // Verify the value matches the parameter type
                validate_type(parameter, value, [&](string message) {
                    throw argument_exception(rvalue_cast(message), i);
                });

                if (current_scope->set(name, std::make_shared<values::value>(rvalue_cast(value)), _evaluator.compilation_context()->path(), parameter.position().line())) {
                    throw _evaluator.create_exception(parameter.position(), (boost::format("parameter $%1% already exists in the parameter list.") % name).str());
                }
            }
        }

        return evaluate_body();
    }

    values::value executor::execute(runtime::resource const& resource, shared_ptr<runtime::scope> const& scope) const
    {
        // Create the execution scope
        auto const& path = _evaluator.compilation_context()->path();
        auto local_scope = _evaluator.evaluation_context().create_local_scope(scope);
        auto& current_scope = _evaluator.evaluation_context().current_scope();
        auto const& attributes = resource.attributes();

        // Set any default parameters without attributes
        if (_parameters) {
            for (auto const& parameter : *_parameters) {
                auto const& name = parameter.variable().name();

                // Check if the attribute exists
                if (attributes.get(name)) {
                    continue;
                }

                // If there's no default value, the parameter is required
                if (!parameter.default_value()) {
                    throw _evaluator.create_exception(parameter.position(), (boost::format("parameter $%1% is required but no value was given.") % name).str());
                }

                // Evaluate the default value
                auto value = _evaluator.evaluate(*parameter.default_value());

                // Verify the value matches the parameter type
                validate_type(parameter, value, [&](string message) {
                    throw _evaluator.create_exception(parameter.default_value()->position(), rvalue_cast(message));
                });

                // Set the default value into the scope
                if (current_scope->set(name, std::make_shared<values::value>(rvalue_cast(value)), path, parameter.position().line())) {
                    throw _evaluator.create_exception(parameter.position(), (boost::format("parameter $%1% already exists in the parameter list.") % name).str());
                }
            }
        }

        shared_ptr<values::value const> title = make_shared<values::value const>(resource.type().title());
        shared_ptr<values::value const> name = title;

        // Set each attribute in the scope
        attributes.each([&](string const& attribute_name, shared_ptr<values::value const> const& attribute_value) {
            // Check for resource name
            if (attribute_name == "name") {
                name = attribute_value;
                return true;
            }

            // Find the attribute as a class parameter
            ast::parameter const* parameter = nullptr;
            if (_parameters) {
                for (auto const& p : *_parameters) {
                    if (p.variable().name() == attribute_name) {
                        parameter = &p;
                        break;
                    }
                }
            }

            // If the attribute is a parameter, validate it
            if (parameter) {
                // Verify the value matches the parameter type
                validate_type(*parameter, *attribute_value, [&](string message) {
                    throw attribute_value_exception(rvalue_cast(message), attribute_name);
                });
            } else if (!resource::is_metaparameter(attribute_name)) {
                // Not a parameter or metaparameter for the class or defined type
                if (resource.type().is_class()) {
                    throw attribute_name_exception(
                        (boost::format("'%1%' is not a valid parameter for class '%2%'.") % attribute_name % resource.type().title()).str(),
                        attribute_name);
                }
                throw attribute_name_exception(
                    (boost::format("'%1%' is not a valid parameter for defined type '%2%'.") % attribute_name % resource.type().type_name()).str(),
                    attribute_name);
            }

            current_scope->set(attribute_name, attribute_value, resource.path(), resource.line());
            return true;
        });

        scope->set("title", rvalue_cast(title), path, resource.line());
        scope->set("name", rvalue_cast(name), path, resource.line());

        return evaluate_body();
    }

    void executor::validate_type(ast::parameter const& parameter, values::value const& value, function<void(string)> const& type_error) const
    {
        if (!parameter.type()) {
            return;
        }

        auto const& type_expression = *parameter.type();

        // Verify the value matches the parameter type
        auto result = _evaluator.evaluate(type_expression);
        auto type = as<values::type>(result);
        if (!type) {
            throw _evaluator.create_exception(
                get_position(type_expression),
                (boost::format("expected %1% for parameter type but found %2%.") % types::type::name() % get_type(type)).str());
        }
        if (!is_instance(value, *type)) {
            type_error((boost::format("parameter $%1% has expected type %2% but was given %3%.") % parameter.variable().name() % *type % get_type(value)).str());
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
        return result;
    }

}}  // namespace puppet::runtime
