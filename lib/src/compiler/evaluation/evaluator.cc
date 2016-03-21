#include <puppet/compiler/evaluation/evaluator.hpp>
#include <puppet/compiler/evaluation/collectors/query_collector.hpp>
#include <puppet/compiler/evaluation/postfix_evaluator.hpp>
#include <puppet/compiler/evaluation/functions/call_context.hpp>
#include <puppet/compiler/evaluation/operators/binary/call_context.hpp>
#include <puppet/compiler/evaluation/operators/unary/call_context.hpp>
#include <puppet/compiler/exceptions.hpp>

using namespace std;
using namespace puppet::compiler::ast;
using namespace puppet::compiler::evaluation::operators;
using namespace puppet::runtime;
using namespace puppet::runtime::values;
namespace x3 = boost::spirit::x3;

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
            function_evaluator evaluator{ _context, "<epp-eval>", *tree.parameters, tree.statements };

            values::hash empty;
            evaluator.evaluate(arguments ? *arguments : empty);
            return;
        }

        // Evaluate the statements
        for (auto& statement : tree.statements) {
            evaluate(statement, true /* all top-level statements must be productive */);
        }
    }

    value evaluator::evaluate(expression const& expression, bool productive)
    {
        _context.current_context(expression.context());

        if (productive && !expression.is_productive()) {
            throw evaluation_exception("unproductive expressions may only appear last in a block.", expression.context(), _context.backtrace());
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
                // Dispatch a match operator
                binary::call_context context{
                    _context,
                    ast::binary_operator::match,
                    ast::context{
                        actual_context.begin,
                        lexer::position{ actual_context.begin.offset() + 1, actual_context.begin.line() },
                        actual_context.tree
                    },
                    actual,
                    actual_context,
                    expected,
                    expected_context
                };
                if (_context.dispatcher().dispatch(context).is_truthy()) {
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
        if (expression.margin == 0) {
            return expression.value;
        }

        auto current_margin = expression.margin;
        std::string text;
        align_text(expression.value, expression.margin, current_margin, [&](char const* ptr, size_t size) {
            text.append(ptr, size);
        });
        return text;
    }

    value evaluator::operator()(ast::regex const& expression)
    {
        try {
            return values::regex(expression.value);
        } catch (regex_error const& ex) {
            throw evaluation_exception(
                (boost::format("invalid regular expression: %1%") %
                 ex.what()
                ).str(),
                expression,
                _context.backtrace()
            );
        }
    }

    value evaluator::operator()(ast::variable const& expression)
    {
        if (expression.name.empty()) {
            throw evaluation_exception("variable name cannot be empty.", expression, _context.backtrace());
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
            { types::iterable::name(),      types::iterable() },
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

    value evaluator::operator()(interpolated_string const& expression)
    {
        ostringstream os;
        scoped_output_stream output{ _context, os };

        size_t current_margin = expression.margin;

        for (auto const& part : expression.parts) {
            if (auto ptr = boost::get<literal_string_text>(&part)) {
                align_text(ptr->text, expression.margin, current_margin, [this](char const* ptr, size_t size) {
                    _context.write(ptr, size);
                });
            } else if (auto ptr = boost::get<ast::variable>(&part)) {
                current_margin = 0;
                _context.write(operator()(*ptr));
            } else if (auto ptr = boost::get<x3::forward_ast<ast::expression>>(&part)) {
                current_margin = 0;
                _context.write(evaluate(*ptr));
            } else {
                throw evaluation_exception("unsupported interpolation part.", part.context(), _context.backtrace());
            }
        }
        return os.str();
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
        match_scope scope{ _context };

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
        match_scope scope{ _context };

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
        match_scope scope{ _context };

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
        // Find the function before executing the call to ensure it is imported
        _context.find_function(expression.function.value);

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
                ).str(),
                expression.type.context(),
                _context.backtrace()
            );
        }

        if (is_class && expression.status == ast::resource_status::virtualized) {
            throw evaluation_exception("classes cannot be virtual resources.", expression.type.context(), _context.backtrace());
        } else if (is_class && expression.status == ast::resource_status::exported) {
            throw evaluation_exception("classes cannot be exported resources.", expression.type.context(), _context.backtrace());
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
        static const auto to_resource_type = [](evaluation::context& context, values::type const& type, ast::context const& resource_context) {
            // Check for Class types
            if (boost::get<types::klass>(&type)) {
                throw evaluation_exception("cannot override attributes of a class resource.", resource_context, context.backtrace());
            }

            // Make sure the type is a resource type
            auto resource = boost::get<types::resource>(&type);
            if (!resource) {
                throw evaluation_exception(
                    (boost::format("expected qualified %1% but found %2%.") %
                     types::resource::name() %
                     value(type).get_type()
                    ).str(),
                    resource_context,
                    context.backtrace()
                );
            }

            // Classes cannot be overridden
            if (resource->is_class()) {
                throw evaluation_exception("cannot override attributes of a class resource.", resource_context, context.backtrace());
            }
            return resource;
        };

        // Evaluate the resource reference
        auto reference = evaluate(expression.reference);

        // Evaluate the attributes
        compiler::attributes attributes = evaluate_attributes(false, expression.operations);
        auto context = expression.reference.context();
        auto& scope = _context.current_scope();

        if (auto array = reference.as<values::array>()) {
            for (auto const& element : *array) {
                if (auto type = element->as<values::type>()) {
                    auto resource = to_resource_type(_context, *type, context);
                    if (!resource->fully_qualified()) {
                        // Treat as a defaults expression
                        scope->add_defaults(_context, *resource, attributes);
                    } else {
                        _context.add(resource_override{ *resource, expression.reference.context(), attributes, scope });
                    }
                    _context.add(resource_override(*resource, expression.reference.context(), attributes, _context.current_scope()));
                } else {
                    throw evaluation_exception(
                        (boost::format("expected qualified %1% for array element but found %2%.") %
                         types::resource::name() %
                         element->get_type()
                        ).str(),
                        context,
                        _context.backtrace()
                    );
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

            auto resource = to_resource_type(_context, *type, context);
            if (!resource->fully_qualified()) {
                scope->add_defaults(_context, *resource, rvalue_cast(attributes));
            } else {
                _context.add(resource_override{ *resource, expression.reference.context(), rvalue_cast(attributes), scope });
            }
            _context.add(resource_override(*resource, expression.reference.context(), rvalue_cast(attributes), _context.current_scope()));
        } else {
            throw evaluation_exception(
                (boost::format("expected qualified %1% for resource reference but found %2%.") %
                 types::resource::name() %
                 reference.get_type()
                ).str(),
                context,
                _context.backtrace()
            );
        }
        return reference;
    }

    value evaluator::operator()(resource_defaults_expression const& expression)
    {
        // Evaluate the type
        auto value = evaluate(ast::primary_expression{expression.type});
        auto type = value.as<values::type>();
        if (!type) {
            throw evaluation_exception(
                (boost::format("expected %1% type but found %2%.") %
                 types::resource::name() %
                 value.get_type()
                ).str(),
                expression.type,
                _context.backtrace()
            );
        }

        // Make sure the type is a resource type
        auto resource = boost::get<types::resource>(type);
        if (!resource) {
            throw evaluation_exception(
                (boost::format("expected %1% type but found %2%.") %
                 types::resource::name() %
                 value.get_type()
                ).str(),
                expression.type,
                _context.backtrace()
            );
        }

        // Evaluate the attributes
        auto attributes = evaluate_attributes(false, expression.operations);
        for (auto& attribute : attributes) {
            if (attribute.first == ast::attribute_operator::append) {
                throw evaluation_exception(
                    (boost::format("expected %1% type but found %2%.") %
                     types::resource::name() %
                     value.get_type()
                    ).str(),
                    expression.type,
                    _context.backtrace()
                );
            }
        }

        _context.current_scope()->add_defaults(_context, *resource, rvalue_cast(attributes));
        return value;
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
        // Function expressions are handled by the scanner
        // TODO: it sure would be nice if functions and lambdas are represented in the type system so we can return
        // references to them here, allowing for functional programming
        return values::undef();
    }

    value evaluator::operator()(unary_expression const& expression)
    {
        auto operand = evaluate(expression.operand);
        auto operand_context = expression.operand.context();

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

    value evaluator::operator()(epp_render_expression const& expression)
    {
        if (!_context.write(evaluate(expression.expression))) {
            throw evaluation_exception("EPP expressions are not supported.", expression, _context.backtrace());
        }
        return values::undef();
    }

    value evaluator::operator()(epp_render_block const& expression)
    {
        if (!_context.write(evaluate_body(expression.block))) {
            throw evaluation_exception("EPP expressions are not supported.", expression, _context.backtrace());
        }
        return values::undef();
    }

    value evaluator::operator()(epp_render_string const& expression)
    {
        if (!_context.write(expression.string)) {
            throw evaluation_exception("EPP expressions are not supported.", expression, _context.backtrace());
        }
        return values::undef();
    }

    value evaluator::operator()(produces_expression const& expression)
    {
        // TODO: implement
        throw evaluation_exception("produces expressions are not yet implemented.", expression.context(), _context.backtrace());
    }

    value evaluator::operator()(consumes_expression const& expression)
    {
        // TODO: implement
        throw evaluation_exception("consumes expressions are not yet implemented.", expression.context(), _context.backtrace());
    }

    value evaluator::operator()(application_expression const& expression)
    {
        // TODO: implement
        throw evaluation_exception("application expressions are not yet implemented.", expression, _context.backtrace());
    }

    value evaluator::operator()(site_expression const& expression)
    {
        // TODO: implement
        throw evaluation_exception("site expressions are not yet implemented.", expression, _context.backtrace());
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
                throw evaluation_exception("only one default body is supported in a resource expression.", body.context(), _context.backtrace());
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
                throw evaluation_exception("title is not a valid parameter name.", operation.name, _context.backtrace());
            }

            // Splat the attribute if named '*'
            if (name == "*") {
                splat_attribute(attributes, names, operation);
                continue;
            }

            // Check for the "stage" attribute for non-classes
            if (!is_class && name == "stage") {
                throw evaluation_exception("attribute 'stage' is only valid for classes.", operation.name, _context.backtrace());
            }

            if (!names.insert(name).second) {
                throw evaluation_exception(
                    (boost::format("attribute '%1%' already exists in the list.") %
                     name
                    ).str(),
                    operation.name,
                    _context.backtrace()
                );
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
            throw evaluation_exception(
                (boost::format("expected a %1% but found %2%.") %
                 types::hash::name() %
                 value.get_type()
                ).str(),
                context,
                _context.backtrace()
            );
        }

        // Set each element of the hash as an attribute
        auto hash = value.move_as<values::hash>();
        for (auto& kvp : hash) {
            auto name = kvp.key().as<std::string>();
            if (!name) {
                throw evaluation_exception(
                    (boost::format("expected all keys in hash to be %1% but found %2%.") %
                     types::string::name() %
                     kvp.key().get_type()
                    ).str(),
                    context,
                    _context.backtrace()
                );
            }
            if (!names.insert(*name).second) {
                throw evaluation_exception(
                    (boost::format("attribute '%1%' already exists in the list.") %
                     name
                    ).str(),
                    context,
                    _context.backtrace()
                );
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
            throw evaluation_exception(
                (boost::format("expected %1% for attribute '%2%' but found %3%.") %
                 *type %
                 name %
                 (original ? original->get_type() : value.get_type())
                ).str(),
                context,
                _context.backtrace()
            );
        }
    }

    vector<resource*> evaluator::create_resources(bool is_class, std::string const& type_name, ast::resource_expression const& expression, attributes const& defaults)
    {
        auto& catalog = _context.catalog();
        auto& scope = _context.current_scope();

        // Lookup a defined type if not a built-in or class
        defined_type const* definition = nullptr;
        if (!is_class && !types::resource(type_name).is_builtin()) {
            definition = _context.find_defined_type(type_name);
            if (!definition) {
                throw evaluation_exception(
                    (boost::format("type '%1%' has not been defined.") %
                     type_name
                    ).str(),
                    expression.type.context(),
                    _context.backtrace()
                );
            }
        }

        // If a class, don't set a container; one will be associated when the class is declared
        // Stages never have a container
        resource const* container = is_class || type_name == "stage" ? nullptr : scope->resource();

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
                    throw evaluation_exception("resource title cannot be empty.", body.context(), _context.backtrace());
                }

                if (is_class) {
                    // Format the title based on the Class type.
                    types::klass::normalize(resource_title);
                }

                // Add the resource to the catalog
                types::resource type{ type_name, rvalue_cast(resource_title) };
                auto resource = catalog.add(
                    type,
                    container,
                    scope,
                    body.context(),
                    is_virtual,
                    is_exported);
                if (!resource) {
                    resource = catalog.find(type);
                    if (!resource) {
                        throw runtime_error("expected previous resource.");
                    }
                    throw evaluation_exception(
                        (boost::format("resource %1% was previously declared at %2%:%3%.") %
                         type %
                         resource->path() %
                         resource->line()
                        ).str(),
                        body.title.context(),
                        _context.backtrace()
                    );
                }

                // Set the default attributes
                for (auto& attribute : defaults) {
                    resource->set(attribute.second);
                }

                // Set the resource's attributes
                for (auto& attribute : attributes) {
                    resource->set(attribute.second);
                }

                // Add the declared defined type
                if (definition) {
                    _context.add(declared_defined_type(*resource, *definition));
                }

                // Evaluate any existing overrides for this resource now
                _context.evaluate_overrides(type);

                // Add the resource to the list
                resources.emplace_back(resource);
            })) {
                throw evaluation_exception(
                    (boost::format("expected %1% or an array of %1% for resource title.") %
                     types::string::name()
                    ).str(),
                    body.context(),
                    _context.backtrace()
                );
            }
        }
        return resources;
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
    }

    void evaluator::align_text(std::string const& text, size_t margin, size_t& current_margin, function<void(char const*, size_t)> const& callback)
    {
        auto begin = text.begin();
        auto end = text.end();

        while (begin != end) {
            if (current_margin > 0) {
                // Eat any leading whitespace, up-to the margin (spaces and tabs are treated the same)
                for (; current_margin > 0 && begin != end && (*begin == ' ' || *begin == '\t'); ++begin, --current_margin);
                current_margin = 0;
                if (begin == end) {
                    continue;
                }
            }

            // Seek to the end of the line
            auto line_end = begin;
            for (; line_end != end && *line_end != '\n'; ++line_end);
            if (line_end == end) {
                // Write the rest of the text
                callback(&*begin, distance(begin, end));
                break;
            }

            // Write the current line only and reset the margin
            ++line_end;
            callback(&*begin, distance(begin, line_end));
            begin = line_end;
            current_margin = margin;
        }
    }

    static void validate_parameter_type(evaluation::context& context, ast::parameter const& parameter, values::value const& value, function<void(std::string)> const& error)
    {
        if (!parameter.type) {
            return;
        }

        // Create a new match scope in case the type expression assigns match variables
        match_scope scope{ context };
        evaluation::evaluator evaluator{ context };

        // Verify the value matches the parameter type
        auto result = evaluator.evaluate(*parameter.type);
        auto type = result.as<values::type>();
        if (!type) {
            throw evaluation_exception(
                (boost::format("expected %1% for parameter type but found %2%.") %
                 types::type::name() %
                 result.get_type()
                ).str(),
                parameter.type->context(),
                context.backtrace()
            );
        }
        if (!type->is_instance(value)) {
            error((boost::format("parameter $%1% has expected type %2% but was given %3%.") % parameter.variable.name % *type % value.get_type()).str());
        }
    }

    static values::value evaluate_default_value(evaluation::context& context, ast::expression const& expression)
    {
        // Create a new match scope in case the default value assigns match variables
        match_scope scope{ context };
        evaluation::evaluator evaluator { context };
        return evaluator.evaluate(expression);
    }

    static values::value evaluate_body(evaluation::context& context, vector<ast::expression> const& body)
    {
        evaluation::evaluator evaluator { context };

        // Evaluate the body
        values::value result;
        for (size_t i = 0; i < body.size(); ++i) {
            auto& expression = body[i];

            // The last expression in the block is allowed to be unproductive (i.e. the return value)
            result = evaluator.evaluate(expression, i < (body.size() - 1));
        }
        return result;
    }

    function_evaluator::function_evaluator(evaluation::context& context, ast::function_expression const& expression) :
        _context(context),
        _name(nullptr),
        _expression(&expression),
        _parameters(expression.parameters),
        _body(expression.body)
    {
    }

    function_evaluator::function_evaluator(evaluation::context& context, char const* name, vector<ast::parameter> const& parameters, vector<ast::expression> const& body) :
        _context(context),
        _name(name),
        _expression(nullptr),
        _parameters(parameters),
        _body(body)
    {
    }

    values::value function_evaluator::evaluate(values::array& arguments, shared_ptr<scope> parent, ast::context const& call_context, bool allow_excessive) const
    {
        // Check to make sure there aren't too many arguments
        if (!allow_excessive && (_parameters.empty() || !_parameters.back().captures)) {
            if (arguments.size() > _parameters.size()) {
                throw evaluation_exception(
                    (boost::format("function '%1%' has %2% parameter%3% but %4% argument%5% were given.") %
                     (_name ? _name : _expression->name.value.c_str()) %
                     _parameters.size() %
                     (_parameters.size() != 1 ? "s" : "") %
                     arguments.size() %
                     (arguments.size() != 1 ? "s" : "")
                    ).str(),
                    call_context,
                    _context.backtrace()
                );
            }
        }

        // Create a new scope and stack frame
        auto scope = make_shared<evaluation::scope>(parent ? parent : _context.top_scope());
        scoped_stack_frame frame = _name ?
            scoped_stack_frame{ _context, stack_frame{ _name, scope } } :
            scoped_stack_frame{ _context, stack_frame{ _expression, scope } };

        bool has_optional_parameters = false;
        for (size_t i = 0; i < _parameters.size(); ++i) {
            auto const& parameter = _parameters[i];
            auto const& name = parameter.variable.name;

            // Check for "captures rest"
            values::value value;
            if (parameter.captures) {
                if (i != (_parameters.size() - 1)) {
                    throw evaluation_exception(
                        (boost::format("parameter $%1% \"captures rest\" but is not the last parameter.") %
                         name
                        ).str(),
                        parameter.context(),
                        _context.backtrace()
                    );
                }
                if (parameter.type) {
                    throw evaluation_exception(
                        (boost::format("parameter $%1% \"captures rest\" and cannot have a type specifier.") %
                         name
                        ).str(),
                        parameter.type->context(),
                        _context.backtrace()
                    );
                }
                values::array captured;
                if (i < arguments.size()) {
                    captured.reserve(arguments.size() - i);
                    captured.insert(captured.end(), std::make_move_iterator(arguments.begin() + i), std::make_move_iterator(arguments.end()));
                } else if (parameter.default_value) {
                    captured.emplace_back(evaluate_default_value(_context, *parameter.default_value));
                }
                value = rvalue_cast(captured);
            } else {
                // Check for a required parameter after an optional parameter
                if (has_optional_parameters && !parameter.default_value) {
                    throw evaluation_exception(
                        (boost::format("parameter $%1% is required but appears after optional parameters.") %
                         name
                        ).str(),
                        parameter.context(),
                        _context.backtrace()
                    );
                }

                has_optional_parameters = static_cast<bool>(parameter.default_value);

                // Check if the argument was given
                if (i < arguments.size()) {
                    value = rvalue_cast(arguments[i]);

                    // Verify the value matches the parameter type
                    validate_parameter_type(_context, parameter, value, [&](std::string message) {
                        throw argument_exception(rvalue_cast(message), i);
                    });
                } else {
                    // Check for not present and without a default value
                    if (!parameter.default_value) {
                        throw evaluation_exception(
                            (boost::format("parameter $%1% is required but no value was given.") %
                             name
                            ).str(),
                            parameter.variable,
                            _context.backtrace()
                        );
                    }

                    value = evaluate_default_value(_context, *parameter.default_value);

                    // Verify the value matches the parameter type
                    validate_parameter_type(_context, parameter, value, [&](std::string message) {
                        throw evaluation_exception(rvalue_cast(message), parameter.default_value->context(), _context.backtrace());
                    });
                }
            }

            if (scope->set(name, std::make_shared<values::value>(rvalue_cast(value)), parameter.variable)) {
                throw evaluation_exception(
                    (boost::format("parameter $%1% already exists in the parameter list.") %
                     name
                    ).str(),
                    parameter.context(),
                    _context.backtrace()
                );
            }
        }

        return evaluate_body(_context, _body);
    }

    values::value function_evaluator::evaluate(values::hash& arguments, shared_ptr<scope> parent) const
    {
        // Create a new scope and stack frame
        auto scope = make_shared<evaluation::scope>(parent ? parent : _context.top_scope());
        scoped_stack_frame frame = _name ?
           scoped_stack_frame{ _context, stack_frame{ _name, scope } } :
           scoped_stack_frame{ _context, stack_frame{ _expression, scope } };

        // Set any default parameters that do not have arguments
        for (auto const& parameter : _parameters) {
            auto const& name = parameter.variable.name;

            // Check if the attribute exists
            if (arguments.get(name)) {
                continue;
            }

            // If there's no default value, the parameter is required
            if (!parameter.default_value) {
                throw evaluation_exception(
                    (boost::format("parameter $%1% is required but no value was given.") %
                     name
                    ).str(),
                    parameter.variable,
                    _context.backtrace()
                );
            }

            // Evaluate the default value expression
            auto value = evaluate_default_value(_context, *parameter.default_value);

            // Verify the value matches the parameter type
            validate_parameter_type(_context, parameter, value, [&](std::string message) {
                throw evaluation_exception(rvalue_cast(message), parameter.default_value->context(), _context.backtrace());
            });

            if (scope->set(name, std::make_shared<values::value>(rvalue_cast(value)), parameter.variable)) {
                throw evaluation_exception(
                    (boost::format("parameter $%1% already exists in the parameter list.") %
                     name
                    ).str(),
                    parameter.variable,
                    _context.backtrace()
                );
            }
        }

        // Set the arguments
        size_t index = 0;
        for (auto& kvp : arguments) {
            auto name = kvp.key().as<std::string>();
            if (!name) {
                throw argument_exception(
                    (boost::format("expected %1% for argument key but found %2%.") %
                     types::string::name() %
                     kvp.key().get_type()
                    ).str(),
                    index
                );
            }

            auto parameter = find_if(_parameters.begin(), _parameters.end(), [&](auto const& parameter) { return *name == parameter.variable.name; });
            if (parameter == _parameters.end()) {
                throw argument_exception(
                    (boost::format("'%1%' is not a valid parameter.") %
                     *name
                    ).str(),
                    index
                );
            }

            // Check for illegal "captures rest"
            if (parameter->captures) {
                throw evaluation_exception(
                    (boost::format("parameter $%1% cannot \"captures rest\".") %
                     *name
                    ).str(),
                    parameter->variable,
                    _context.backtrace()
                );
            }

            // Verify the value matches the parameter type
            validate_parameter_type(_context, *parameter, kvp.value(), [&](std::string message) {
                throw argument_exception(rvalue_cast(message), index);
            });

            if (scope->set(*name, std::make_shared<values::value>(rvalue_cast(kvp.value())), parameter->variable)) {
                throw evaluation_exception(
                    (boost::format("parameter $%1% already exists in the parameter list.") %
                     *name
                    ).str(),
                    parameter->variable,
                    _context.backtrace()
                );
            }
        }
        return evaluate_body(_context, _body);
    }

    resource_evaluator::resource_evaluator(evaluation::context& context, vector<ast::parameter> const& parameters, vector<ast::expression> const& body) :
        _context(context),
        _parameters(parameters),
        _body(body)
    {
    }

    void resource_evaluator::prepare_scope(evaluation::scope& scope, compiler::resource& resource) const
    {
        // Set the title in the scope
        shared_ptr<values::value const> title = make_shared<values::value const>(resource.type().title());
        scope.set("title", title, resource.context());

        // Set the name in the scope
        shared_ptr<values::value const> name = rvalue_cast(title);
        if (auto attribute = resource.get("name")) {
            name = attribute->shared_value();
        }
        scope.set("name", rvalue_cast(name), resource.context());

        // Verify the resource's attributes
        resource.each_attribute([&](compiler::attribute const& attribute) {
            // Ignore the name attribute as it was already handled above
            if (attribute.name() == "name") {
                return true;
            }

            // If the attribute is a parameter, it will be validated below
            if (find_if(_parameters.begin(), _parameters.end(), [&](auto const& parameter) { return parameter.variable.name == attribute.name(); }) != _parameters.end()) {
                return true;
            }

            // If the attribute is a metaparameter, set it in the scope now
            if (resource::is_metaparameter(attribute.name())) {
                scope.set(attribute.name(), attribute.shared_value(), attribute.value_context());
                return true;
            }

            // Not a parameter or metaparameter, therefore not a valid parameter
            if (resource.type().is_class()) {
                throw evaluation_exception(
                    (boost::format("'%1%' is not a valid parameter for class '%2%'.") %
                     attribute.name() %
                     resource.type().title()
                    ).str(),
                    attribute.name_context(),
                    _context.backtrace()
                );
            }
            throw evaluation_exception(
                (boost::format("'%1%' is not a valid parameter for defined type %2%.") %
                 attribute.name() %
                 resource.type()
                ).str(),
                attribute.name_context(),
                _context.backtrace()
            );
        });

        // Go through the parameters and set them into the scope
        for (auto const& parameter : _parameters) {
            auto const& name = parameter.variable.name;

            ast::context context;
            shared_ptr<values::value> value;

            // Check if the attribute exists
            if (auto attribute = resource.get(name)) {
                value = attribute->shared_value();
                context = attribute->value_context();
            } else {
                // If there's no default value, the parameter is required
                if (!parameter.default_value) {
                    throw evaluation_exception(
                        (boost::format("parameter $%1% is required but no value was given.") %
                         name
                        ).str(),
                        parameter.variable,
                        _context.backtrace()
                    );
                }

                // Evaluate the default value expression
                value = std::make_shared<values::value>(evaluate_default_value(_context, *parameter.default_value));
                context = parameter.default_value->context();

                // Set the parameter as an attribute on the resource
                resource.set(std::make_shared<compiler::attribute>(
                    name,
                    parameter.variable,
                    value,
                    context
                ));
            }

            // Verify the value matches the parameter type
            validate_parameter_type(_context, parameter, *value, [&](std::string message) {
                throw evaluation_exception(rvalue_cast(message), parameter.default_value->context(), _context.backtrace());
            });

            // Set the parameter in the scope
            if (scope.set(name, rvalue_cast(value), context)) {
                throw evaluation_exception(
                    (boost::format("parameter $%1% already exists in the parameter list.") %
                     name
                    ).str(),
                    parameter.variable,
                    _context.backtrace()
                );
            }
        }
    }

    class_evaluator::class_evaluator(evaluation::context& context, ast::class_expression const& expression) :
        resource_evaluator(context, expression.parameters, expression.body),
        _expression(expression)
    {
    }

    void class_evaluator::evaluate(compiler::resource& resource) const
    {
        bool created = false;
        auto scope = _context.find_scope(resource.type().title());
        if (!scope) {
            // Create a temporary stack frame to show the child calling into the parent if parent evaluation fails
            scoped_stack_frame frame{ _context, stack_frame{ &_expression, nullptr } };
            scope = make_shared<evaluation::scope>(evaluate_parent(), &resource);
            _context.add_scope(scope);
            created = true;
        }

        scoped_stack_frame frame{ _context, stack_frame{ &_expression, scope } };

        if (created) {
            prepare_scope(*scope, resource);
        }
        evaluate_body(_context, _body);
    }

    shared_ptr<scope> class_evaluator::evaluate_parent() const
    {
        // If no parent, return the node or top scope
        auto& parent = _expression.parent;
        if (!parent) {
            return _context.node_or_top();
        }

        auto scope = _context.find_scope(_context.declare_class(parent->value, *parent)->type().title());
        if (!scope) {
            // Because classes are added to the catalog before the scope is added to the context, a failure to find the scope means there is a circular inheritence
            throw evaluation_exception(
                (boost::format("cannot evaluate parent class '%1%' because it causes a circular inheritence.") %
                 *parent
                ).str(),
                *parent,
                _context.backtrace()
            );
        }
        return scope;
    }

    defined_type_evaluator::defined_type_evaluator(evaluation::context& context, ast::defined_type_expression const& expression) :
        resource_evaluator(context, expression.parameters, expression.body),
        _expression(expression)
    {
    }

    void defined_type_evaluator::evaluate(compiler::resource& resource) const
    {
        // Create a scope for evaluating the defined type
        auto scope = make_shared<evaluation::scope>(_context.node_or_top(), &resource);
        scoped_stack_frame frame{ _context, stack_frame{ &_expression, scope } };

        prepare_scope(*scope, resource);
        evaluate_body(_context, _body);
    }

    node_evaluator::node_evaluator(evaluation::context& context, ast::node_expression const& expression) :
        resource_evaluator(context, _none, expression.body),
        _expression(expression)
    {
    }

    void node_evaluator::evaluate(compiler::resource& resource) const
    {
        // Set the node scope for the remainder of the evaluation
        // Nodes don't have parameters, so nothing to prepare in scope
        node_scope scope{ _context, resource };

        scoped_stack_frame frame{ _context, stack_frame{ &_expression, _context.node_scope() } };
        evaluate_body(_context, _body);
    }

    vector<ast::parameter> node_evaluator::_none;

}}}  // namespace puppet::compiler::evaluation
