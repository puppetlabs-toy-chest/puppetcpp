#include <puppet/compiler/evaluation/call_evaluator.hpp>
#include <puppet/compiler/evaluation/evaluator.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation {

    argument_exception::argument_exception(string const& message, size_t index) :
        runtime_error(message),
        _index(index)
    {
    }

    size_t argument_exception::index() const
    {
        return _index;
    }

    call_evaluator::call_evaluator(evaluation::context& context, vector<ast::parameter> const& parameters, vector<ast::expression> const& body) :
        _context(context),
        _parameters(parameters),
        _body(body)
    {
    }

    values::value call_evaluator::evaluate(shared_ptr<scope> const& scope) const
    {
        values::array arguments;
        return evaluate(arguments, scope);
    }

    values::value call_evaluator::evaluate(values::array& arguments, shared_ptr<scope> const& scope) const
    {
        // Create the local scope
        auto local_scope = _context.create_local_scope(scope);
        auto& current_scope = _context.current_scope();

        evaluation::evaluator evaluator { _context };

        bool has_optional_parameters = false;
        for (size_t i = 0; i < _parameters.size(); ++i) {
            auto const& parameter = _parameters[i];
            auto const& name = parameter.variable.name;

            // Check for "captures rest"
            values::value value;
            if (parameter.captures) {
                if (i != (_parameters.size() - 1)) {
                    throw evaluation_exception((boost::format("parameter $%1% \"captures rest\" but is not the last parameter.") % name).str(), parameter.context());
                }
                if (parameter.type) {
                    throw evaluation_exception((boost::format("parameter $%1% \"captures rest\" and cannot have a type specifier.") % name).str(), parameter.context());
                }
                values::array captured;
                if (i < arguments.size()) {
                    captured.reserve(arguments.size() - i);
                    captured.insert(captured.end(), std::make_move_iterator(arguments.begin() + i), std::make_move_iterator(arguments.end()));
                } else if (parameter.default_value) {
                    captured.emplace_back(evaluator.evaluate(*parameter.default_value));
                }
                value = rvalue_cast(captured);
            } else {
                // Check for a required parameter after an optional parameter
                if (has_optional_parameters && !parameter.default_value) {
                    throw evaluation_exception((boost::format("parameter $%1% is required but appears after optional parameters.") % name).str(), parameter.context());
                }

                has_optional_parameters = static_cast<bool>(parameter.default_value);

                // Check if the argument was given
                if (i < arguments.size()) {
                    value = rvalue_cast(arguments[i]);

                    // Verify the value matches the parameter type
                    validate_parameter_type(parameter, value, [&](string message) {
                        throw argument_exception(rvalue_cast(message), i);
                    });
                } else {
                    // Check for not present and without a default value
                    if (!parameter.default_value) {
                        throw evaluation_exception((boost::format("parameter $%1% is required but no value was given.") % name).str(), parameter.context());
                    }

                    value = evaluator.evaluate(*parameter.default_value);

                    // Verify the value matches the parameter type
                    validate_parameter_type(parameter, value, [&](string message) {
                        throw evaluation_exception(rvalue_cast(message), parameter.default_value->context());
                    });
                }
            }

            if (current_scope->set(name, std::make_shared<values::value>(rvalue_cast(value)), &parameter.context())) {
                throw evaluation_exception((boost::format("parameter $%1% already exists in the parameter list.") % name).str(), parameter.context());
            }
        }
        return evaluate_body();
    }

    values::value call_evaluator::evaluate(values::hash& arguments, shared_ptr<scope> const& scope) const
    {
        // Create the local scope
        auto local_scope = _context.create_local_scope(scope);
        auto& current_scope = _context.current_scope();

        evaluation::evaluator evaluator { _context };

        // Set any default parameters that do not have arguments
        for (auto const& parameter : _parameters) {
            auto const& name = parameter.variable.name;

            // Check if the attribute exists
            if (arguments.get(name)) {
                continue;
            }

            // If there's no default value, the parameter is required
            if (!parameter.default_value) {
                throw evaluation_exception((boost::format("parameter $%1% is required but no value was given.") % name).str(), parameter.context());
            }

            // Evaluate the default value
            auto value = evaluator.evaluate(*parameter.default_value);

            // Verify the value matches the parameter type
            validate_parameter_type(parameter, value, [&](string message) {
                throw evaluation_exception(rvalue_cast(message), parameter.default_value->context());
            });

            if (current_scope->set(name, std::make_shared<values::value>(rvalue_cast(value)), &parameter.context())) {
                throw evaluation_exception((boost::format("parameter $%1% already exists in the parameter list.") % name).str(), parameter.context());
            }
        }

        // Set the arguments
        size_t index = 0;
        for (auto& kvp : arguments) {
            auto name = kvp.key().as<string>();
            if (!name) {
                throw argument_exception((boost::format("expected %1% for argument key but found %2%.") % types::string::name() % kvp.key().get_type()).str(), index);
            }
            auto parameter = find_if(_parameters.begin(), _parameters.end(), [&](auto const& parameter) { return *name == parameter.variable.name; });
            if (parameter == _parameters.end()) {
                throw argument_exception((boost::format("'%1%' is not a valid parameter.") % *name).str(), index);
            }

            // Check for illegal "captures rest"
            if (parameter->captures) {
                throw evaluation_exception((boost::format("parameter $%1% cannot \"captures rest\".") % *name).str(), parameter->context());
            }

            // Verify the value matches the parameter type
            validate_parameter_type(*parameter, kvp.value(), [&](string message) {
                throw argument_exception(rvalue_cast(message), index);
            });

            if (current_scope->set(*name, std::make_shared<values::value>(rvalue_cast(kvp.value())), &parameter->context())) {
                throw evaluation_exception((boost::format("parameter $%1% already exists in the parameter list.") % *name).str(), parameter->context());
            }
        }
        return evaluate_body();
    }

    values::value call_evaluator::evaluate(compiler::resource& resource, shared_ptr<scope> const& scope) const
    {
        // Create the local scope
        auto local_scope = _context.create_local_scope(scope);
        auto& current_scope = _context.current_scope();

        // Set the title in the scope
        shared_ptr<values::value const> title = make_shared<values::value const>(resource.type().title());
        scope->set("title", title, &resource.context());

        // Set the name in the scope
        shared_ptr<values::value const> name = rvalue_cast(title);
        if (auto attribute = resource.get("name")) {
            name = attribute->shared_value();
        }
        scope->set("name", rvalue_cast(name), &resource.context());

        evaluation::evaluator evaluator { _context };

        // Set any default parameters without attributes
        for (auto const& parameter : _parameters) {
            auto const& name = parameter.variable.name;

            // Check if the attribute exists
            if (resource.get(name)) {
                continue;
            }

            // If there's no default value, the parameter is required
            if (!parameter.default_value) {
                throw evaluation_exception((boost::format("parameter $%1% is required but no value was given.") % name).str(), parameter.context());
            }

            // Evaluate the default value
            auto value = evaluator.evaluate(*parameter.default_value);

            // Verify the value matches the parameter type
            validate_parameter_type(parameter, value, [&](string message) {
                throw evaluation_exception(rvalue_cast(message), parameter.default_value->context());
            });

            // Set the parameter as an attribute on the resource
            resource.set(std::make_shared<attribute>(
                parameter.variable.name,
                parameter.context(),
                std::make_shared<values::value>(rvalue_cast(value)),
                parameter.default_value->context()
            ));
        }

        // Set each attribute in the scope
        resource.each_attribute([&](compiler::attribute const& attribute) {
            // Ignore the name attribute as it was already set
            if (attribute.name() == "name") {
                return true;
            }

            // Find the attribute as a class parameter
            auto it = find_if(_parameters.begin(), _parameters.end(), [&](auto const& parameter) { return parameter.variable.name == attribute.name(); });

            // If the attribute is a parameter, validate it
            if (it != _parameters.end()) {
                // Verify the value matches the parameter type
                validate_parameter_type(*it, attribute.value(), [&](string message) {
                    throw evaluation_exception(rvalue_cast(message), attribute.value_context());
                });
            } else if (!resource::is_metaparameter(attribute.name())) {
                // Not a parameter or metaparameter for the class or defined type
                if (resource.type().is_class()) {
                    throw evaluation_exception(
                        (boost::format("'%1%' is not a valid parameter for class '%2%'.") %
                         attribute.name() %
                         resource.type().title()
                        ).str(),
                        attribute.name_context());
                } else {
                    throw evaluation_exception(
                        (boost::format("'%1%' is not a valid parameter for defined type %2%.") %
                         attribute.name() %
                         resource.type()
                        ).str(),
                        attribute.name_context());
                }
            }
            current_scope->set(attribute.name(), attribute.shared_value(), &attribute.value_context());
            return true;
        });
        return evaluate_body();
    }

    void call_evaluator::validate_parameter_type(ast::parameter const& parameter, values::value const& value, function<void(string)> const& error) const
    {
        if (!parameter.type) {
            return;
        }

        evaluation::evaluator evaluator { _context };

        // Verify the value matches the parameter type
        auto result = evaluator.evaluate(*parameter.type);
        auto type = result.as<values::type>();
        if (!type) {
            throw evaluation_exception((boost::format("expected %1% for parameter type but found %2%.") % types::type::name() % result.get_type()).str(), parameter.type->context());
        }
        if (!type->is_instance(value)) {
            error((boost::format("parameter $%1% has expected type %2% but was given %3%.") % parameter.variable.name % *type % value.get_type()).str());
        }
    }

    values::value call_evaluator::evaluate_body() const
    {
        evaluation::evaluator evaluator { _context };

        // Evaluate the body
        values::value result;
        for (size_t i = 0; i < _body.size(); ++i) {
            auto& expression = _body[i];
            // The last expression in the block is allowed to be unproductive (i.e. the return value)
            result = evaluator.evaluate(expression, i < (_body.size() - 1));
        }
        return result;
    }

}}}  // puppet::compiler::evaluation
