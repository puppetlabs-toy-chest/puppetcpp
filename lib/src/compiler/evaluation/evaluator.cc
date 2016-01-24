#include <puppet/compiler/evaluation/evaluator.hpp>
#include <puppet/compiler/evaluation/collectors/query_collector.hpp>
#include <puppet/compiler/evaluation/postfix_evaluator.hpp>
#include <puppet/compiler/evaluation/interpolator.hpp>
#include <puppet/compiler/evaluation/call_evaluator.hpp>
#include <puppet/compiler/evaluation/functions/call_context.hpp>
#include <puppet/compiler/evaluation/operators/binary/call_context.hpp>
#include <puppet/compiler/evaluation/operators/unary/call_context.hpp>
#include <puppet/compiler/evaluation/operators/less_equal.hpp>
#include <puppet/compiler/evaluation/operators/logical_and.hpp>
#include <puppet/compiler/evaluation/operators/logical_not.hpp>
#include <puppet/compiler/evaluation/operators/logical_or.hpp>
#include <puppet/compiler/evaluation/operators/match.hpp>
#include <puppet/compiler/evaluation/operators/minus.hpp>
#include <puppet/compiler/evaluation/operators/modulo.hpp>
#include <puppet/compiler/evaluation/operators/multiply.hpp>
#include <puppet/compiler/evaluation/operators/negate.hpp>
#include <puppet/compiler/evaluation/operators/not_equals.hpp>
#include <puppet/compiler/evaluation/operators/not_match.hpp>
#include <puppet/compiler/evaluation/operators/plus.hpp>
#include <puppet/compiler/evaluation/operators/relationship.hpp>
#include <puppet/compiler/evaluation/operators/right_shift.hpp>
#include <puppet/compiler/evaluation/operators/splat.hpp>
#include <puppet/compiler/exceptions.hpp>

using namespace std;
using namespace puppet::compiler::ast;
using namespace puppet::compiler::evaluation::operators;
using namespace puppet::runtime;
using namespace puppet::runtime::values;

namespace puppet { namespace compiler { namespace evaluation {

    evaluator::evaluator(evaluation::context& context) :
        _context(context)
    {
    }

    evaluation::context& evaluator::context()
    {
        return _context;
    }

    void evaluator::evaluate(syntax_tree const& tree, values::hash* arguments)
    {
        if (tree.parameters) {
            // Create an ephemeral scope for evaluation
            auto local_scope = _context.create_local_scope();

            // "call" an empty function to populate the arguments into the scope
            vector<expression> body;
            values::hash empty;
            call_evaluator evaluator{ _context, *tree.parameters, body };
            evaluator.evaluate(arguments ? *arguments : empty, _context.current_scope());

            // Evaluate the statements
            for (auto& statement : tree.statements) {
                evaluate(statement, true /* all top-level statements must be productive */);
            }
            return;
        }

        // Evaluate the statements
        for (auto& statement : tree.statements) {
            evaluate(statement, true /* all top-level statements must be productive */);
        }
    }

    value evaluator::evaluate(expression const& expression, bool productive)
    {
        if (productive && !expression.is_productive()) {
            throw evaluation_exception("unproductive expressions may only appear last in a block.", expression.context());
        }

        // Climb the expression
        auto begin = expression.operations.begin();
        return climb_expression(expression.first, 0, begin, expression.operations.end());
    }

    value evaluator::evaluate(postfix_expression const& expression)
    {
        postfix_evaluator evaluator{ _context };
        return evaluator.evaluate(expression);
    }

    value evaluator::evaluate(primary_expression const& expression)
    {
        return boost::apply_visitor(*this, expression);
    }

    bool evaluator::is_match(value& actual, ast::context const& actual_context, value& expected, ast::context const& expected_context)
    {
        // If the expected value is a regex, use match
        auto regex = expected.as<values::regex>();
        if (regex) {
            // Only match against strings
            if (actual.as<std::string>()) {
                operators::binary_operator_context context{ _context, actual, actual_context, expected, expected_context };
                if (operators::match()(context).is_truthy()) {
                    return true;
                }
            }
            return false;
        }

        // Otherwise, use equals
        return actual == expected;
    }

    value evaluator::evaluator::operator()(ast::undef const&)
    {
        return values::undef();
    }

    value evaluator::operator()(ast::defaulted const&)
    {
        return values::defaulted();
    }

    value evaluator::operator()(ast::boolean const& expression)
    {
        return expression.value;
    }

    value evaluator::operator()(int64_t value)
    {
        return value;
    }

    value evaluator::operator()(double value)
    {
        return value;
    }

    value evaluator::operator()(number const& expression)
    {
        return boost::apply_visitor(*this, expression.value);
    }

    value evaluator::operator()(ast::string const& expression)
    {
        evaluation::interpolator interpolator{_context};
        return interpolator.interpolate(expression);
    }

    value evaluator::operator()(ast::regex const& expression)
    {
        try {
            return values::regex(expression.value);
        } catch (regex_error const& ex) {
            throw evaluation_exception((boost::format("invalid regular expression: %1%") % ex.what()).str(), expression);
        }
    }

    value evaluator::operator()(ast::variable const& expression)
    {
        if (expression.name.empty()) {
            throw evaluation_exception("variable name cannot be empty.", expression);
        }

        shared_ptr<values::value const> value;
        if (isdigit(expression.name[0])) {
            value = _context.lookup(stoi(expression.name));
        } else {
            value = _context.lookup(expression);
        }
        return values::variable(expression.name, rvalue_cast(value));
    }

    value evaluator::operator()(name const& expression)
    {
        // Treat as a string
        return expression.value;
    }

    value evaluator::operator()(bare_word const& expression)
    {
        // Treat as a string
        return expression.value;
    }

    value evaluator::operator()(ast::type const& expression)
    {
        static const unordered_map<std::string, values::type> names = {
            { types::any::name(),           types::any() },
            { types::array::name(),         types::array() },
            { types::boolean::name(),       types::boolean() },
            { types::callable::name(),      types::callable() },
            { types::catalog_entry::name(), types::catalog_entry() },
            { types::collection::name(),    types::collection() },
            { types::data::name(),          types::data() },
            { types::defaulted::name(),     types::defaulted() },
            { types::enumeration::name(),   types::enumeration() },
            { types::floating::name(),      types::floating() },
            { types::hash::name(),          types::hash() },
            { types::integer::name(),       types::integer() },
            { types::klass::name(),         types::klass() },
            { types::not_undef::name(),     types::not_undef() },
            { types::numeric::name(),       types::numeric() },
            { types::optional::name(),      types::optional() },
            { types::pattern::name(),       types::pattern() },
            { types::regexp::name(),        types::regexp() },
            { types::resource::name(),      types::resource() },
            { types::runtime::name(),       types::runtime() },
            { types::scalar::name(),        types::scalar() },
            { types::string::name(),        types::string() },
            { types::structure::name(),     types::structure() },
            { types::tuple::name(),         types::tuple() },
            { types::type::name(),          types::type() },
            { types::undef::name(),         types::undef() },
            { types::variant::name(),       types::variant() },
        };

        auto it = names.find(expression.name);
        if (it == names.end()) {
            // Assume the unknown type is a resource
            // TODO: this needs to check registered types
            return types::resource(expression.name);
        }
        return it->second;
    }

    value evaluator::operator()(ast::array const& expression)
    {
        values::array array;

        for (auto& element : expression.elements) {
            auto result = evaluate(element);

            // If the element is being splatted, move its elements
            if (element.is_splat() && result.as<values::array>()) {
                auto unfolded = result.move_as<values::array>();
                array.reserve(array.size() + unfolded.size());
                array.insert(array.end(), std::make_move_iterator(unfolded.begin()), std::make_move_iterator(unfolded.end()));
                continue;
            }
            array.emplace_back(rvalue_cast(result));
        }
        return array;
    }

    value evaluator::operator()(ast::hash const& expression)
    {
        values::hash hash;
        for (auto& element : expression.elements) {
            hash.set(evaluate(element.first), evaluate(element.second));
        }
        return hash;
    }

    value evaluator::operator()(ast::nested_expression const& expression)
    {
        return evaluate(expression.expression);
    }

    value evaluator::operator()(case_expression const& expression)
    {
        // Case expressions create a new match scope
        auto match_scope = _context.create_match_scope();

        // Evaluate the case's expression
        value result = evaluate(expression.conditional);

        // Search for a matching proposition
        auto& propositions = expression.propositions;
        boost::optional<size_t> default_index;
        for (size_t i = 0; i < propositions.size(); ++i) {
            auto& proposition = propositions[i];

            // Look for a match in the options
            for (auto& option : proposition.options) {
                // Evaluate the option
                value option_value = evaluate(option);
                if (option_value.is_default()) {
                    // Remember where the default is and keep going
                    default_index = i;
                    continue;
                }

                // If splatted, unfold the array and match against each element
                if (option_value.as<values::array>() && option.is_splat()) {
                    auto array = option_value.move_as<values::array>();
                    for (auto& element : array) {
                        if (is_match(result, expression, element, option.context())) {
                            return evaluate_body(proposition.body);
                        }
                    }
                }

                // Otherwise, match against the value
                if (is_match(result, expression, option_value, option.context())) {
                    return evaluate_body(proposition.body);
                }
            }
        }

        // Handle no matching case
        if (default_index) {
            return evaluate_body(propositions[*default_index].body);
        }

        // Nothing matched, return undef
        return values::undef();
    }

    value evaluator::operator()(if_expression const& expression)
    {
        // If expressions create a new match scope
        auto match_scope = _context.create_match_scope();

        if (evaluate(expression.conditional).is_truthy()) {
            return evaluate_body(expression.body);
        }
        for (auto& elsif : expression.elsifs) {
            if (evaluate(elsif.conditional).is_truthy()) {
                return evaluate_body(elsif.body);
            }
        }
        if (expression.else_) {
            return evaluate_body(expression.else_->body);
        }
        return values::undef();
    }

    value evaluator::operator()(unless_expression const& expression)
    {
        // Unless expressions create a new match scope
        auto match_scope = _context.create_match_scope();

        if (!evaluate(expression.conditional).is_truthy()) {
            return evaluate_body(expression.body);
        }
        if (expression.else_) {
            return evaluate_body(expression.else_->body);
        }
        return values::undef();
    }

    value evaluator::operator()(function_call_expression const& expression)
    {
        functions::call_context context{ _context, expression };
        return _context.dispatcher().dispatch(context);
    }

    value evaluator::operator()(resource_expression const& expression)
    {
        // Evaluate the type name
        std::string type_name;
        auto type_value = evaluate(expression.type);

        // Resource expressions support either strings or Resource[Type] for the type name
        bool is_class = false;
        if (type_value.as<std::string>()) {
            type_name = type_value.move_as<std::string>();
            is_class = type_name == "class";
        } else if (auto type = type_value.as<values::type>()) {
            if (auto resource = boost::get<types::resource>(type)) {
                if (resource->title().empty()) {
                    type_name = resource->type_name();
                    is_class = resource->is_class();
                }
            }
        }

        // Ensure there was a valid type name
        if (type_name.empty()) {
            throw evaluation_exception(
                (boost::format("expected %1% or qualified %2% for resource type but found %3%.") %
                    types::string::name() %
                    types::resource::name() %
                    type_value.get_type()
                ).str(), expression.type.context());
        }

        if (is_class && expression.status == ast::resource_status::virtualized) {
            throw evaluation_exception("classes cannot be virtual resources.", expression.type.context());
        } else if (is_class && expression.status == ast::resource_status::exported) {
            throw evaluation_exception("classes cannot be exported resources.", expression.type.context());
        }

        // Get the default body attributes
        attributes default_attributes;
        if (auto default_body = find_default_body(expression)) {
            default_attributes = evaluate_attributes(is_class, default_body->operations);
        }

        // Create the resources in the expression
        vector<resource*> resources = create_resources(is_class, type_name, expression, default_attributes);

        // Declare classes now; defined types are declared when the evaluation context is finalized
        if (is_class) {
            for (auto resource : resources) {
                _context.declare_class(resource->type().title(), resource->context());
            }
        }

        // Return an array of the resource types
        values::array types;
        types.reserve(resources.size());
        for (auto resource : resources) {
            types.push_back(resource->type());
        }
        return types;
    }

    value evaluator::operator()(resource_override_expression const& expression)
    {
        static const auto to_resource_type = [](values::type const& type, ast::context const& context) {
            // Check for Class types
            if (boost::get<types::klass>(&type)) {
                throw evaluation_exception("cannot override attributes of a class resource.", context);
            }

            // Make sure the type is a resource type
            auto resource = boost::get<types::resource>(&type);
            if (!resource) {
                throw evaluation_exception((boost::format("expected qualified %1% but found %2%.") % types::resource::name() % value(type).get_type()).str(), context);
            }

            // Classes cannot be overridden
            if (resource->is_class()) {
                throw evaluation_exception("cannot override attributes of a class resource.", context);
            }
            return resource;
        };

        // Evaluate the resource reference
        auto reference = evaluate(expression.reference);

        // Evaluate the attributes
        compiler::attributes attributes = evaluate_attributes(false, expression.operations);
        auto context = expression.reference.context();

        if (auto array = reference.as<values::array>()) {
            for (auto const& element : *array) {
                if (auto type = element->as<values::type>()) {
                    auto resource = to_resource_type(*type, context);
                    if (!resource->fully_qualified()) {
                        // TODO: support resource defaults expression
                        throw evaluation_exception("resource defaults expressions are not yet implemented.", context);
                    }
                    _context.add(resource_override(*resource, expression, attributes, _context.current_scope()));
                } else {
                    throw evaluation_exception((boost::format("expected qualified %1% for array element but found %2%.") % types::resource::name() % element->get_type()).str(), context);
                }
            }
        } else if (auto type = reference.as<values::type>()) {
            // Check for a collector
            if (auto runtime = boost::get<types::runtime>(type)) {
                if (runtime->object()) {
                    if (auto collector = boost::get<shared_ptr<collectors::collector>>(*runtime->object())) {
                        // The value is a collector; set the attributes
                        collector->attributes(rvalue_cast(attributes));
                        return reference;
                    }
                }
            }

            auto resource = to_resource_type(*type, context);
            if (!resource->fully_qualified()) {
                // TODO: support resource defaults expression
                throw evaluation_exception("resource defaults expressions are not yet implemented.", context);
            }
            _context.add(resource_override(*resource, expression, rvalue_cast(attributes), _context.current_scope()));
        } else {
            throw evaluation_exception((boost::format("expected qualified %1% for resource reference but found %2%.") % types::resource::name() % reference.get_type()).str(), context);
        }
        return reference;
    }

    value evaluator::operator()(resource_defaults_expression const& expression)
    {
        // TODO: implement
        throw evaluation_exception("resource defaults expressions are not yet implemented.", expression);
    }

    value evaluator::operator()(class_expression const& expression)
    {
        // Class expressions are handled by the scanner; just return a reference to the class
        return types::klass(_context.current_scope()->qualify(expression.name.value));
    }

    value evaluator::operator()(defined_type_expression const& expression)
    {
        // Defined type expressions are handled by the scanner; just return a reference to the resource
        return types::resource(_context.current_scope()->qualify(expression.name.value));
    }

    value evaluator::operator()(node_expression const& expression)
    {
        // Node definition expressions are handled by the scanner; just return undef
        // TODO: should this perhaps return [Resource[Node, hostname], Resource[Node, hostname], ...] for consistency?
        return values::undef();
    }

    value evaluator::operator()(collector_expression const& expression)
    {
        // Create and add a collector to the catalog
        auto collector = make_shared<collectors::query_collector>(expression, _context.current_scope());
        _context.add(collector);
        return types::runtime(types::runtime::object_type(rvalue_cast(collector)));
    }

    value evaluator::operator()(function_expression const& expression)
    {
        // Not yet implemented for evaluation (TODO: uncomment the subsequence lines)
        throw evaluation_exception("function expressions are not yet implemented.", expression);
        // Function expressions are handled by the scanner
        // TODO: it sure would be nice if functions and lambdas are represented in the type system so we can return
        // references to them here, allowing for functional programming
        // return values::undef();
    }

    value evaluator::operator()(unary_expression const& expression)
    {
        static const unordered_map<ast::unary_operator, function<value(operators::unary_operator_context const&)>, boost::hash<unary_operator>> unary_operators = {
            { ast::unary_operator::negate,      operators::negate() },
            { ast::unary_operator::logical_not, operators::logical_not() },
            { ast::unary_operator::splat,       operators::splat() }
        };

        auto operand = evaluate(expression.operand);

        auto it = unary_operators.find(expression.operator_);
        if (it == unary_operators.end()) {
            auto operand_context = expression.operand.context();

            // TODO: use only the dispatcher once all operators have been ported
            unary::call_context context{
                _context,
                expression.operator_,
                ast::context{
                    expression.operator_position,
                    lexer::position{ expression.operator_position.offset() + 1, expression.operator_position.line() },
                    operand_context.tree
                },
                operand,
                operand_context
            };
            return _context.dispatcher().dispatch(context);
        }

        operators::unary_operator_context context{ _context, operand, expression.operand.context() };
        return it->second(context);
    }

    value evaluator::operator()(epp_render_expression const& expression)
    {
        if (!_context.epp_write(evaluate(expression.expression))) {
            throw evaluation_exception("EPP expressions are not supported.", expression);
        }
        return values::undef();
    }

    value evaluator::operator()(epp_render_block const& expression)
    {
        if (!_context.epp_write(evaluate_body(expression.block))) {
            throw evaluation_exception("EPP expressions are not supported.", expression);
        }
        return values::undef();
    }

    value evaluator::operator()(epp_render_string const& expression)
    {
        if (!_context.epp_write(expression.string)) {
            throw evaluation_exception("EPP expressions are not supported.", expression);
        }
        return values::undef();
    }

    value evaluator::operator()(produces_expression const& expression)
    {
        // TODO: implement
        throw evaluation_exception("produces expressions are not yet implemented.", expression.context());
    }

    value evaluator::operator()(consumes_expression const& expression)
    {
        // TODO: implement
        throw evaluation_exception("consumes expressions are not yet implemented.", expression.context());
    }

    value evaluator::operator()(application_expression const& expression)
    {
        // TODO: implement
        throw evaluation_exception("application expressions are not yet implemented.", expression);
    }

    value evaluator::operator()(site_expression const& expression)
    {
        // TODO: implement
        throw evaluation_exception("site expressions are not yet implemented.", expression);
    }

    value evaluator::evaluate_body(vector<ast::expression> const& body)
    {
        value result;
        for (size_t i = 0; i < body.size(); ++i) {
            auto& expression = body[i];
            // The last expression in the block is allowed to be unproductive (i.e. the return value)
            result = evaluate(expression, i < (body.size() - 1));
        }
        return result;
    }

    ast::resource_body const* evaluator::find_default_body(ast::resource_expression const& expression)
    {
        ast::resource_body const* default_body = nullptr;
        for (auto const& body : expression.bodies) {
            if (!body.title.is_default()) {
                continue;
            }
            if (default_body) {
                throw evaluation_exception("only one default body is supported in a resource expression.", body.context());
            }
            default_body = &body;
        }
        return default_body;
    }

    attributes evaluator::evaluate_attributes(bool is_class, vector<ast::attribute_operation> const& operations)
    {
        compiler::attributes attributes;

        unordered_set<std::string> names;
        for (auto& operation : operations) {
            auto& name = operation.name.value;

            // Check for setting the title via an attribute
            if (name == "title") {
                throw evaluation_exception("title is not a valid parameter name.", operation.name);
            }

            // Splat the attribute if named '*'
            if (name == "*") {
                splat_attribute(attributes, names, operation);
                continue;
            }

            // Check for the "stage" attribute for non-classes
            if (!is_class && name == "stage") {
                throw evaluation_exception("attribute 'stage' is only valid for classes.", operation.name);
            }

            if (!names.insert(name).second) {
                throw evaluation_exception((boost::format("attribute '%1%' already exists in the list.") % name).str(), operation.name);
            }

            // Evaluate and validate the attribute value
            auto value = evaluate(operation.value);
            validate_attribute(name, value, operation.value.context());

            // Add an attribute to the list
            attributes.emplace_back(make_pair(operation.operator_, std::make_shared<attribute>(
                name,
                operation.name,
                std::make_shared<values::value>(rvalue_cast(value)),
                operation.value.context()
            )));
        }
        return attributes;
    }

    void evaluator::splat_attribute(compiler::attributes& attributes, unordered_set<std::string>& names, ast::attribute_operation const& operation)
    {
        auto context = operation.value.context();

        // Evaluate what must be a hash
        auto value = evaluate(operation.value);
        if (!value.as<values::hash>()) {
            throw evaluation_exception((boost::format("expected a %1% but found %2%.") % types::hash::name() % value.get_type()).str(), context);
        }

        // Set each element of the hash as an attribute
        auto hash = value.move_as<values::hash>();
        for (auto& kvp : hash) {
            auto name = kvp.key().as<std::string>();
            if (!name) {
                throw evaluation_exception((boost::format("expected all keys in hash to be %1% but found %2%.") % types::string::name() % kvp.key().get_type()).str(), context);
            }
            if (!names.insert(*name).second) {
                throw evaluation_exception((boost::format("attribute '%1%' already exists in the list.") % name).str(), context);
            }

            // Validate the attribute value
            auto value = kvp.value();
            validate_attribute(*name, value, context);

            // Add the attribute to the list
            attributes.emplace_back(make_pair(operation.operator_, std::make_shared<compiler::attribute>(
                *name,
                operation.name,
                std::make_shared<values::value>(rvalue_cast(value)),
                rvalue_cast(context)
            )));
        }
    }

    void evaluator::validate_attribute(std::string const& name, values::value& value, ast::context const& context)
    {
        // Type information for metaparameters
        static const values::type string_array_type = types::array(make_unique<values::type>(types::string()));
        static const values::type relationship_type = create_relationship_type();
        static const values::type string_type = types::string();
        static const values::type boolean_type = types::boolean();
        static const values::type loglevel_type = types::enumeration({ "debug", "info", "notice", "warning", "err", "alert", "emerg", "crit", "verbose" });
        static const values::type audit_type = create_audit_type();

        // Ignore undef attributes
        if (value.is_undef()) {
            return;
        }

        // Perform metaparameter checks
        values::type const* type = nullptr;
        values::value const* original = nullptr;
        if (name == "alias") {
            type = &string_array_type;
            if (!value.as<values::array>()) {
                value = value.to_array(false);
                original = &*value.as<values::array>()->at(0);
            }
        } else if (name == "audit") {
            type = &audit_type;
        } else if (name == "before" || name == "notify" || name == "require" || name == "subscribe") {
            type = &relationship_type;
            if (!value.as<values::array>()) {
                value = value.to_array(false);
                original = &*value.as<values::array>()->at(0);
            }
        } else if (name == "loglevel") {
            type = &loglevel_type;
        } else if (name == "noop") {
            type = &boolean_type;
        } else if (name == "schedule") {
            type = &string_type;
        } else if (name == "stage") {
            type = &string_type;
        } else if (name == "tag") {
            type = &string_array_type;
            if (!value.as<values::array>()) {
                value = value.to_array(false);
                original = &*value.as<values::array>()->at(0);
            }
        }

        if (!type) {
            // Not a metaparameter
            // TODO: get the attribute type from the type's definition and validate
        }

        // Validate the type of the parameter
        if (type && !type->is_instance(value)) {
            throw evaluation_exception((boost::format("expected %1% for attribute '%2%' but found %3%.") % *type % name % (original ? original->get_type() : value.get_type())).str(), context);
        }
    }

    vector<resource*> evaluator::create_resources(bool is_class, std::string const& type_name, ast::resource_expression const& expression, attributes const& defaults)
    {
        auto& catalog = _context.catalog();

        // Lookup a defined type if not a built-in or class
        defined_type const* definition = nullptr;
        if (!is_class && !types::resource(type_name).is_builtin()) {
            definition = _context.find_defined_type(type_name);
            if (!definition) {
                throw evaluation_exception((boost::format("type '%1%' has not been defined.") % type_name).str(), expression.type.context());
            }
        }

        // If a class, don't set a container; one will be associated when the class is declared
        resource const* container = is_class ? nullptr : _context.current_scope()->resource();

        bool is_exported = expression.status == ast::resource_status::exported;
        bool is_virtual = is_exported || expression.status == ast::resource_status::virtualized;

        vector<resource*> resources;
        for (auto const& body : expression.bodies) {
            auto title = evaluate(body.title);

            // If the default title, ignore (we've already evaluated the default attributes)
            if (title.is_default()) {
                continue;
            }

            // Evaluate the attributes
            auto attributes = evaluate_attributes(is_class, body.operations);

            // Add each resource to the catalog
            if (!title.move_as<std::string>([&](std::string resource_title) {
                if (resource_title.empty()) {
                    throw evaluation_exception("resource title cannot be empty.", body.context());
                }

                if (is_class) {
                    // Format the title based on the Class type.
                    types::klass::normalize(resource_title);
                }

                // Add the resource to the catalog
                auto resource = catalog.add(
                    types::resource(type_name, rvalue_cast(resource_title)),
                    container,
                    body.context(),
                    is_virtual,
                    is_exported);
                if (!resource) {
                    resource = catalog.find(types::resource(type_name, rvalue_cast(resource_title)));
                    if (!resource) {
                        throw runtime_error("expected previous resource.");
                    }
                    throw evaluation_exception((boost::format("resource %1% was previously declared at %2%:%3%.") % resource->type() % resource->path() % resource->line()).str(), body.context());
                }

                // Set the default attributes
                set_attributes(*resource, defaults);

                // Set the resource's attributes
                set_attributes(*resource, attributes);

                // Add the declared defined type
                if (definition) {
                    _context.add(declared_defined_type(*resource, *definition));
                }

                // Evaluate any existing overrides for this resource now
                _context.evaluate_overrides(resource->type());

                // Add the resource to the list
                resources.emplace_back(resource);
            })) {
                throw evaluation_exception((boost::format("expected %1% or an array of %1% for resource title.") % types::string::name()).str(), body.context());
            }
        }
        return resources;
    }

    void evaluator::set_attributes(compiler::resource& resource, compiler::attributes const& attributes)
    {
        // Set the default attributes
        for (auto& kvp : attributes) {
            auto oper = kvp.first;
            auto& attribute = kvp.second;

            // Only support assignment
            if (oper != ast::attribute_operator::assignment) {
                throw evaluation_exception(
                    (boost::format("illegal attribute operation '%1%': only '%2%' is supported in a resource expression.") %
                     oper %
                     ast::attribute_operator::assignment
                    ).str(),
                    attribute->name_context());
            }
            resource.set(attribute);
        }
    }

    values::type evaluator::create_relationship_type()
    {
        vector<unique_ptr<values::type>> types;
        types.emplace_back(make_unique<values::type>(types::string()));
        types.emplace_back(make_unique<values::type>(types::catalog_entry()));
        return types::array(make_unique<values::type>(types::variant(rvalue_cast(types))));
    }

    values::type evaluator::create_audit_type()
    {
        vector<unique_ptr<values::type>> types;
        types.emplace_back(make_unique<values::type>(types::string()));
        types.emplace_back(make_unique<values::type>(types::array(make_unique<values::type>(types::string()))));
        return types::variant(rvalue_cast(types));
    }

    values::value evaluator::climb_expression(
        ast::postfix_expression const& expression,
        unsigned int min_precedence,
        vector<binary_operation>::const_iterator& begin,
        vector<binary_operation>::const_iterator const& end)
    {
        // Evaluate the left-hand side
        auto left = evaluate(expression);

        // Climb the binary operations based on operator precedence
        unsigned int current = 0;
        while (begin != end && (current = precedence(begin->operator_)) >= min_precedence)
        {
            auto const& operation = *begin;
            ++begin;

            // If the operator is a logical and/or operator, attempt short circuiting
            if ((operation.operator_ == binary_operator::logical_and && !left.is_truthy()) ||
                (operation.operator_ == binary_operator::logical_or && left.is_truthy())) {
                left = operation.operator_ == binary_operator::logical_or;
                begin = end;
                return left;
            }

            // Recurse and climb the expression
            unsigned int next = current + (is_right_associative(operation.operator_) ? 0 : 1);
            auto right = climb_expression(operation.operand, next, begin, end);

            // Evaluate the binary expression
            evaluate(left, expression.context(), right, operation);
        }
        return left;
    }

    void evaluator::evaluate(
        value& left,
        ast::context const& left_context,
        value& right,
        ast::binary_operation const& operation)
    {
        static const unordered_map<binary_operator, function<value(operators::binary_operator_context const&)>, boost::hash<binary_operator>> binary_operators = {
            { ast::binary_operator::in_edge,            operators::in_edge() },
            { ast::binary_operator::in_edge_subscribe,  operators::in_edge_subscribe() },
            { ast::binary_operator::less_equals,        operators::less_equal() },
            { ast::binary_operator::logical_and,        operators::logical_and() },
            { ast::binary_operator::logical_or,         operators::logical_or() },
            { ast::binary_operator::match,              operators::match() },
            { ast::binary_operator::minus,              operators::minus() },
            { ast::binary_operator::modulo,             operators::modulo() },
            { ast::binary_operator::multiply,           operators::multiply() },
            { ast::binary_operator::not_equals,         operators::not_equals() },
            { ast::binary_operator::not_match,          operators::not_match() },
            { ast::binary_operator::out_edge,           operators::out_edge() },
            { ast::binary_operator::out_edge_subscribe, operators::out_edge_subscribe() },
            { ast::binary_operator::plus,               operators::plus() },
            { ast::binary_operator::right_shift,        operators::right_shift() }
        };

        auto it = binary_operators.find(operation.operator_);
        if (it == binary_operators.end()) {
            // TODO: use only the dispatcher once all operators have been ported
            binary::call_context context{
                _context,
                operation.operator_,
                ast::context{
                    operation.operator_position,
                    lexer::position{ operation.operator_position.offset() + 1, operation.operator_position.line() },
                    left_context.tree
                },
                left,
                left_context,
                right,
                operation.operand.context()
            };
            left = _context.dispatcher().dispatch(context);
            return;
        }

        operators::binary_operator_context context{ _context, left, left_context, right, operation.operand.context() };
        left = it->second(context);
    }

}}}  // namespace puppet::compiler::evaluation
