#include <puppet/runtime/catalog.hpp>
#include <puppet/runtime/context.hpp>
#include <puppet/runtime/executor.hpp>
#include <puppet/runtime/expression_evaluator.hpp>
#include <puppet/compiler/node.hpp>
#include <puppet/cast.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime {

    class_definition::class_definition(runtime::catalog& catalog, types::klass klass, shared_ptr<compiler::context> context, ast::class_definition_expression const* expression) :
        _catalog(catalog),
        _klass(rvalue_cast(klass)),
        _context(rvalue_cast(context)),
        _expression(expression)
    {
        if (!_context) {
            throw runtime_error("expected compilation context.");
        }

        _path = _context->path();

        if (_expression) {
            _line = _expression->position().line();
            if (_expression->parent()) {
                _parent = types::klass(_expression->parent()->value());
            }
        }
    }

    runtime::catalog const& class_definition::catalog() const
    {
        return _catalog;
    }

    types::klass const& class_definition::klass() const
    {
        return _klass;
    }

    types::klass const* class_definition::parent() const
    {
        return _parent.title().empty() ? nullptr : &_parent;
    }

    string const& class_definition::path() const
    {
        return *_path;
    }

    size_t class_definition::line() const
    {
        return _line;
    }

    bool class_definition::evaluated() const
    {
        return !_context || !_expression;
    }

    bool class_definition::evaluate(runtime::context& context, unordered_map<ast::name, values::value> const* arguments)
    {
        if (evaluated()) {
            return true;
        }

        values::hash keywords;
        values::value name = _klass.title();

        // Ensure there is a parameter for each argument
        if (arguments) {
            for (auto const& argument : *arguments) {
                // Check for the name argument
                if (argument.first.value() == "name") {
                    name = argument.second;
                    continue;
                }
                bool found = false;
                for (auto const& parameter : *_expression->parameters()) {
                    if (parameter.variable().name() == argument.first.value()) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    throw evaluation_exception(argument.first.position(), (boost::format("'%1%' is not a valid parameter for class '%2%'.") % argument.first % _klass.title()).str());
                }
                keywords[argument.first.value()] = argument.second;
            }
        }

        // Evaluate the parent
        auto parent_scope = evaluate_parent(context);
        if (!parent_scope) {
            return false;
        }

        // Create a scope for the class
        ostringstream display_name;
        display_name << _klass;
        auto scope = make_shared<runtime::scope>(parent_scope, _klass.title(), display_name.str());
        context.add_scope(scope);

        // Set the title and name variables in the scope
        scope->set("title", _klass.title(), _context->path(), _expression->position().line());
        scope->set("name", rvalue_cast(name), _context->path(), _expression->position().line());

        try {
            // Create a new expression evaluator based on the class' compilation context
            expression_evaluator evaluator{ _context, context };

            // Execute the body of the class
            runtime::executor executor{ evaluator, _expression->position(), _expression->parameters(), _expression->body() };
            executor.execute(keywords, scope);
        } catch (evaluation_exception const& ex) {
            // The compilation context is probably not the current one, so do not let evaluation exception propagate
            _context->log(logging::level::error, ex.position(), ex.what());
            return false;
        }

        // Copy the path for later use
        _context.reset();
        _expression = nullptr;
        return true;
    }

    shared_ptr<runtime::scope> class_definition::evaluate_parent(runtime::context& context)
    {
        // If no parent, return the node or top scope
        auto parent = this->parent();
        if (!parent) {
            return context.node_or_top();
        }

        // Check that the parent is defined
        if (!_catalog.is_class_defined(*parent)) {
            _context->log(logging::level::error, _expression->parent()->position(), (boost::format("base class '%2%' has not been defined.") % parent->title()).str());
            return nullptr;
        }

        // Declare the parent class
        if (!_catalog.declare_class(context, *parent, _context->path(), _expression->parent()->position())) {
            return nullptr;
        }
        return context.find_scope(parent->title());
    }

    defined_type::defined_type(runtime::catalog& catalog, string type, shared_ptr<compiler::context> context, ast::defined_type_expression const& expression) :
        _catalog(catalog),
        _type(rvalue_cast(type)),
        _context(rvalue_cast(context)),
        _expression(expression)
    {
        if (!_context) {
            throw runtime_error("expected compilation context.");
        }
    }

    runtime::catalog const& defined_type::catalog() const
    {
        return _catalog;
    }

    string const& defined_type::type() const
    {
        return _type;
    }

    string const& defined_type::path() const
    {
        return *_context->path();
    }

    size_t defined_type::line() const
    {
        return _expression.position().line();
    }

    bool defined_type::evaluate(runtime::context& context, runtime::resource const& resource, unordered_map<ast::name, values::value> const* arguments)
    {
        values::hash keywords;
        values::value name = resource.type().title();

        // Ensure there is a parameter for each argument
        if (arguments) {
            for (auto const& argument : *arguments) {
                // Check for the name argument
                if (argument.first.value() == "name") {
                    name = argument.second;
                    continue;
                }
                bool found = false;
                for (auto const& parameter : *_expression.parameters()) {
                    if (parameter.variable().name() == argument.first.value()) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    throw evaluation_exception(argument.first.position(), (boost::format("'%1%' is not a valid parameter for defined type '%2%'.") % argument.first % _type).str());
                }
                keywords[argument.first.value()] = argument.second;
            }
        }

        // Create a temporary scope
        ostringstream display_name;
        display_name << resource.type();
        auto scope = make_shared<runtime::scope>(context.node_or_top(), _type, display_name.str());

        // Set the title and name variables in the scope
        scope->set("title", resource.type().title(), _context->path(), _expression.position().line());
        scope->set("name", rvalue_cast(name), _context->path(), _expression.position().line());

        try {
            // Create a new expression evaluator based on the defined type's compilation context
            expression_evaluator evaluator{ _context, context };

            // Execute hte body of the defined type
            runtime::executor executor{ evaluator, _expression.position(), _expression.parameters(), _expression.body() };
            executor.execute(keywords, scope);
        } catch (evaluation_exception const& ex) {
            // The compilation context is probably not the current one, so do not let evaluation exception propagate
            _context->log(logging::level::error, ex.position(), ex.what());
            return false;
        }
        return true;
    }

    node_definition::node_definition(runtime::catalog& catalog, shared_ptr<compiler::context> context, ast::node_definition_expression const& expression) :
        _catalog(catalog),
        _context(rvalue_cast(context)),
        _expression(expression)
    {
    }

    runtime::catalog const& node_definition::catalog() const
    {
        return _catalog;
    }

    string const& node_definition::path() const
    {
        return *_context->path();
    }

    size_t node_definition::line() const
    {
        return _expression.position().line();
    }

    bool node_definition::evaluate(runtime::context& context)
    {
        try {
            // Create a new expression evaluator based on the node's compilation context
            expression_evaluator evaluator{ _context, context };

            // Execute the node's body
            runtime::executor executor(evaluator, _expression.position(), boost::none, _expression.body());
            executor.execute(context.node_scope());
        } catch (evaluation_exception const& ex) {
            // The compilation context is probably not the current one, so do not let evaluation exception propagate
            _context->log(logging::level::error, ex.position(), ex.what());
            return false;
        }
        return true;
    }

    resource::resource(runtime::catalog& catalog, types::resource type, shared_ptr<string> path, size_t line, bool exported) :
        _catalog(catalog),
        _type(rvalue_cast(type)),
        _path(rvalue_cast(path)),
        _line(line),
        _exported(exported)
    {
        if (!_path) {
            throw runtime_error("expected path.");
        }
    }

    runtime::catalog const& resource::catalog() const
    {
        return _catalog;
    }

    types::resource const& resource::type() const
    {
        return _type;
    }

    string const& resource::path() const
    {
        return *_path;
    }

    size_t resource::line() const
    {
        return _line;
    }

    unordered_set<string> const& resource::tags() const
    {
        return _tags;
    }

    unordered_map<string, value> const& resource::parameters() const
    {
        return _parameters;
    }

    bool resource::exported() const
    {
        return _exported;
    }

    void resource::add_tag(string tag)
    {
        _tags.emplace(rvalue_cast(tag));
    }

    void resource::set_parameter(string const& name, lexer::position const& name_position, values::value value, lexer::position const& value_position, bool override)
    {
        // Handle metaparameters
        if (handle_metaparameter(name, name_position, value, value_position)) {
            return;
        }

        store_parameter(name, name_position, rvalue_cast(value), override);
    }

    bool resource::remove_parameter(string const& name)
    {
        return _parameters.erase(name) > 0;
    }

    void resource::store_parameter(string const& name, lexer::position const& name_position, values::value value, bool override)
    {
        auto it = _parameters.find(name);
        if (it != _parameters.end()) {
            if (!override) {
                throw evaluation_exception(name_position, (boost::format("attribute '%1%' has already been set for resource %2%.") % name % _type).str());
            }
            it->second = rvalue_cast(value);
            return;
        }
        _parameters.emplace(make_pair(name, rvalue_cast(value)));
    }

    bool resource::handle_metaparameter(string const& name, lexer::position const& name_position, values::value& value, lexer::position const& value_position)
    {
        if (name == "alias") {
            create_alias(value, value_position);
            store_parameter(name, name_position, rvalue_cast(value), false);
            return true;
        } else if (name == "audit") {
            throw evaluation_exception(name_position, "the resource metaparameter 'audit' is not supported.");
        }
        return false;
    }

    void resource::create_alias(values::value const& name, lexer::position const& position)
    {
        if (auto alias = as<string>(name)) {
            if (alias->empty()) {
                throw evaluation_exception(position, "alias name cannot be empty.");
            }
            // Alias this resource
            if (!_catalog.alias_resource(_type, *alias)) {
                throw evaluation_exception(position, (boost::format("a %1% resource with name or alias '%2%' already exists in the catalog.") % _type % *alias).str());
            }
            return;
        }
        if (auto aliases = as<values::array>(name)) {
            // For arrays, recurse on each element
            for (auto& element : *aliases) {
                create_alias(element, position);
            }
            return;
        }
        throw evaluation_exception(position, (boost::format("expected %1% for alias name but found %2%.") % types::string::name() % get_type(name)).str());
    }

    catalog::catalog() :
        _default_node_index(-1)
    {
    }

    resource* catalog::find_resource(types::resource const& resource)
    {
        if (resource.type_name().empty() || resource.title().empty()) {
            return nullptr;
        }

        // Check for an alias first
        auto aliases = _aliases.find(resource.type_name());
        if (aliases != _aliases.end()) {
            auto alias = aliases->second.find(resource.title());
            if (alias != aliases->second.end()) {
                return alias->second;
            }
        }

        // Otherwise find the resource type and title
        auto resources = _resources.find(resource.type_name());
        if (resources == _resources.end()) {
            return nullptr;
        }
        auto it = resources->second.find(resource.title());
        if (it == resources->second.end()) {
            return nullptr;
        }
        return &it->second;
    }

    bool catalog::alias_resource(types::resource const& resource, string const& alias)
    {
        // Check if a resource with the alias name already exists
        if (find_resource(types::resource(resource.type_name(), alias))) {
            return false;
        }

        // Find the resource being aliased
        auto existing = find_resource(resource);
        if (!existing) {
            return false;
        }

        _aliases[resource.type_name()].emplace(make_pair(alias, existing));
        return true;
    }

    runtime::resource* catalog::add_resource(types::resource resource, shared_ptr<string> path, size_t line, bool exported)
    {
        if (resource.type_name().empty() || resource.title().empty()) {
            return nullptr;
        }

        string type = resource.type_name();
        string title = resource.title();
        auto result = _resources[rvalue_cast(type)].emplace(make_pair(rvalue_cast(title), runtime::resource(*this, rvalue_cast(resource), rvalue_cast(path), line, exported)));
        if (!result.second) {
            return nullptr;
        }
        return &result.first->second;
    }

    void catalog::define_class(types::klass klass, shared_ptr<compiler::context> context, ast::class_definition_expression const& expression)
    {
        // Validate the class parameters
        if (expression.parameters()) {
            validate_parameters(true, *expression.parameters());
        }

        auto& definitions = _classes[klass];

        // Check to make sure all existing definitions of the class have the same base or do not inherit
        if (expression.parent()) {
            types::klass parent(expression.parent()->value());
            for (auto const& definition : definitions) {
                auto existing = definition.parent();
                if (!existing) {
                    continue;
                }
                if (parent == *existing) {
                    continue;
                }
                throw evaluation_exception(expression.parent()->position(),
                                           (boost::format("class '%1%' cannot inherit from '%2%' because the class already inherits from '%3%' at %4%:%5%.") %
                                            klass.title() %
                                            expression.parent()->value() %
                                            existing->title() %
                                            definition.path() %
                                            definition.line()
                                           ).str());
            }
        }
        definitions.push_back(class_definition(*this, rvalue_cast(klass), context, &expression));
    }

    runtime::resource* catalog::declare_class(runtime::context& context, types::klass const& klass, shared_ptr<string> path, lexer::position const& position, unordered_map<ast::name, values::value> const* arguments)
    {
        // Attempt to find the existing resource
        types::resource resource_type("class", klass.title());
        runtime::resource* resource = find_resource(resource_type);
        if (resource) {
            return resource;
        }

        // Lookup the class
        auto it = _classes.find(klass);
        if (it == _classes.end() || it->second.empty()) {
            // TODO: search node for class
            return nullptr;
        }

        // Declare the class in the catalog
        resource = add_resource(resource_type, rvalue_cast(path), position.line(), false);
        if (!resource) {
            return nullptr;
        }

        // Evaluate all definitions of the class
        for (auto& definition : it->second) {
            if (!definition.evaluate(context, arguments)) {
                return nullptr;
            }
        }
        return resource;
    }

    bool catalog::is_class_defined(types::klass const& klass) const
    {
        auto it = _classes.find(klass);
        return it != _classes.end() && !it->second.empty();
    }

    bool catalog::is_class_declared(types::klass const& klass) const
    {
        auto it = _classes.find(klass);
        if (it == _classes.end()) {
            return false;
        }

        for (auto& definition : it->second) {
            if (definition.evaluated()) {
                return true;
            }
        }
        return false;
    }

    void catalog::define_type(string type, shared_ptr<compiler::context> context, ast::defined_type_expression const& expression)
    {
        // Validate the defined type's parameters
        if (expression.parameters()) {
            validate_parameters(false, *expression.parameters());
        }

        // Look for the existing type
        auto it = _defined_types.find(type);
        if (it != _defined_types.end()) {
            throw evaluation_exception(expression.name().position(),
                                       (boost::format("defined type '%1%' was previously defined at %2%:%3%.") %
                                        it->second.type() %
                                        it->second.path() %
                                        it->second.line()
                                       ).str());
        }

        // Add the defined type
        defined_type defined(*this, type, rvalue_cast(context), expression);
        _defined_types.emplace(make_pair(rvalue_cast(type), rvalue_cast(defined)));
    }

    bool catalog::is_defined_type(std::string const& type) const
    {
        return _defined_types.count(type) > 0;
    }

    runtime::resource* catalog::declare_defined_type(runtime::context& context, string const& type, string const& title, shared_ptr<string> path, lexer::position const& position, unordered_map<ast::name, values::value> const* arguments)
    {
        // Lookup the defined type
        auto it = _defined_types.find(type);
        if (it == _defined_types.end()) {
            // TODO: search node for defined type
            return nullptr;
        }

        // Add a resource to the catalog
        auto added = add_resource(types::resource(type, title), rvalue_cast(path), position.line(), false);
        if (!added) {
            return nullptr;
        }

        // Evaluate the defined type
        if (!it->second.evaluate(context, *added, arguments)) {
            return nullptr;
        }
        return added;
    }

    void catalog::define_node(shared_ptr<compiler::context> context, ast::node_definition_expression const& expression)
    {
        // Emplace the node
        _nodes.emplace_back(*this, rvalue_cast(context), expression);
        size_t node_index = _nodes.size() -1;

        for (auto const& name : expression.names()) {
            // Check for default node
            if (name.is_default()) {
                if (_default_node_index < 0) {
                    _default_node_index = static_cast<ssize_t>(node_index);
                    continue;
                }
                auto const& previous = _nodes[_default_node_index];
                throw evaluation_exception(name.position(), (boost::format("a default node was previously defined at %1%:%2%.") % previous.path() % previous.line()).str());
            }
            // Check for regular expression names
            if (name.regex()) {
                auto it = find_if(_regex_node_definitions.begin(), _regex_node_definitions.end(), [&](decltype(_regex_node_definitions.front()) existing) { return existing.first.pattern() == name.value(); });
                if (it != _regex_node_definitions.end()) {
                    auto const& previous = _nodes[it->second];
                    throw evaluation_exception(name.position(), (boost::format("node /%1%/ was previously defined at %2%:%3%.") % name.value() % previous.path() % previous.line()).str());
                }

                try {
                    _regex_node_definitions.emplace_back(values::regex(name.value()), node_index);
                } catch (regex_error const& ex) {
                    throw evaluation_exception(name.position(), (boost::format("invalid regular expression: %1%") % ex.what()).str());
                }
                continue;
            }
            // Otherwise, this is a qualified node name
            auto it = _named_nodes.find(name.value());
            if (it != _named_nodes.end()) {
                auto const& previous = _nodes[it->second];
                throw evaluation_exception(name.position(), (boost::format("node '%1%' was previously defined at %2%:%3%.") % name.value() % previous.path() % previous.line()).str());
            }
            _named_nodes.emplace(make_pair(boost::to_lower_copy(name.value()), node_index));
        }
    }

    bool catalog::evaluate_node(runtime::context& context, compiler::node const& node)
    {
        // If there are no node definitions, do nothing
        if (_nodes.empty()) {
            return true;
        }

        // Find a node definition
        string scope_name;
        node_definition* definition = nullptr;
        node.each_name([&](string const& name) {
            // First check by name
            auto it = _named_nodes.find(name);
            if (it != _named_nodes.end()) {
                scope_name = "Node[" + it->first + "]";
                definition = &_nodes[it->second];
                return false;
            }
            // Next, check by looking at every regex
            for (auto const& kvp : _regex_node_definitions) {
                if (regex_search(name, kvp.first.value())) {
                    scope_name = "Node[/" + kvp.first.pattern() + "/]";
                    definition = &_nodes[kvp.second];
                    return false;
                }
            }
            return true;
        });

        if (!definition) {
            if (_default_node_index < 0) {
                ostringstream message;
                message << "could not find a default node or a node with the following names: ";
                bool first = true;
                node.each_name([&](string const& name) {
                    if (first) {
                        first = false;
                    } else {
                        message << ", ";
                    }
                    message << name;
                    return true;
                });
                message << ".";
                throw compiler::compilation_exception(message.str());
            }
            scope_name = "Node[default]";
            definition = &_nodes[_default_node_index];
        }

        // Set the node scope for the remainder of the evaluation
        node_scope scope(context, rvalue_cast(scope_name));

        // Evaluate the node definition
        return definition->evaluate(context);
    }

    void catalog::validate_parameters(bool klass, vector<ast::parameter> const& parameters)
    {
        for (auto const& parameter : parameters) {
            auto const& name = parameter.variable().name();

            if (name == "title" || name == "name") {
                throw evaluation_exception(parameter.variable().position(), (boost::format("parameter $%1% is reserved and cannot be used.") % name).str());
            }
            if (parameter.captures()) {
                throw evaluation_exception(parameter.variable().position(), (boost::format("%1% parameter $%2% cannot \"captures rest\".") % (klass ? "class" : "defined type") % name).str());
            }

            // TODO: warn or error about metaparameters
        }
    }

}}  // namespace puppet::runtime
