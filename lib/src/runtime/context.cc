#include <puppet/runtime/context.hpp>
#include <puppet/runtime/executor.hpp>
#include <puppet/cast.hpp>
#include <boost/format.hpp>

using namespace std;

namespace puppet { namespace runtime {

    struct node_scope_helper
    {
        node_scope_helper(runtime::context& context, string scope_name) :
            _context(context)
        {
            _context._node_scope.reset(new runtime::scope("node", rvalue_cast(scope_name), &_context.top_scope()));
        }

        ~node_scope_helper()
        {
            _context._node_scope.reset();
        }

     private:
        runtime::context& _context;
    };

    class_definition::class_definition(types::klass klass, shared_ptr<compiler::context> context, ast::class_definition_expression const* expression) :
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
        auto& scope = context.add_scope(_klass.title(), display_name.str(), parent_scope);

        // Set the title and name variables in the scope
        scope.set("title", _klass.title(), _context->path(), _expression->position().line());
        scope.set("name", rvalue_cast(name), _context->path(), _expression->position().line());

        try {
            expression_evaluator evaluator{ _context, context };
            runtime::executor executor(evaluator, _expression->position(), _expression->parameters(), _expression->body());
            executor.execute(keywords, &scope);
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

    runtime::scope* class_definition::evaluate_parent(runtime::context& context)
    {
        // If no parent, return the node or top scope
        auto parent = this->parent();
        if (!parent) {
            return &context.node_or_top();
        }

        if (!context.is_class_defined(*parent)) {
            _context->log(logging::level::error, _expression->parent()->position(), (boost::format("base class '%2%' has not been defined.") % parent->title()).str());
            return nullptr;
        }
        if (!context.declare_class(*parent, _context->path(), _expression->parent()->position())) {
            return nullptr;
        }
        return context.find_scope(parent->title());
    }

    defined_type::defined_type(string type, shared_ptr<compiler::context> context, ast::defined_type_expression const& expression) :
        _type(rvalue_cast(type)),
        _context(rvalue_cast(context)),
        _expression(expression)
    {
        if (!_context) {
            throw runtime_error("expected compilation context.");
        }
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
        runtime::scope scope{ _type, display_name.str(), &context.node_or_top() };

        // Set the title and name variables in the scope
        scope.set("title", resource.type().title(), _context->path(), _expression.position().line());
        scope.set("name", rvalue_cast(name), _context->path(), _expression.position().line());

        try {
            expression_evaluator evaluator{ _context, context };
            runtime::executor executor(evaluator, _expression.position(), _expression.parameters(), _expression.body());
            executor.execute(keywords, &scope);
        } catch (evaluation_exception const& ex) {
            // The compilation context is probably not the current one, so do not let evaluation exception propagate
            _context->log(logging::level::error, ex.position(), ex.what());
            return false;
        }
        return true;
    }

    node_definition::node_definition(shared_ptr<compiler::context> context, ast::node_definition_expression const& expression) :
        _context(rvalue_cast(context)),
        _expression(expression)
    {
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
            expression_evaluator evaluator{ _context, context };
            runtime::executor executor(evaluator, _expression.position(), boost::none, _expression.body());
            executor.execute(context.node_scope());
        } catch (evaluation_exception const& ex) {
            // The compilation context is probably not the current one, so do not let evaluation exception propagate
            _context->log(logging::level::error, ex.position(), ex.what());
            return false;
        }
        return true;
    }

    context::context(runtime::catalog& catalog) :
        _catalog(catalog),
        _default_node_index(-1)
    {
        // Add the top scope
        push_scope(add_scope("", "Class[main]"));
    }

    runtime::catalog& context::catalog()
    {
        return _catalog;
    }

    runtime::scope& context::scope()
    {
        return *_scope_stack.back();
    }

    runtime::scope& context::top_scope()
    {
        return *_scope_stack.front();
    }

    runtime::scope* context::node_scope()
    {
        return _node_scope.get();
    }

    runtime::scope& context::node_or_top()
    {
        auto node = node_scope();
        if (node) {
            return *node;
        }
        return top_scope();
    }

    runtime::scope& context::add_scope(string name, string display_name, runtime::scope* parent)
    {
        runtime::scope scope{name, rvalue_cast(display_name), parent};
        return _scopes.emplace(make_pair(rvalue_cast(name), rvalue_cast(scope))).first->second;
    }

    runtime::scope* context::find_scope(string const& name)
    {
        auto it = _scopes.find(name);
        if (it == _scopes.end()) {
            return nullptr;
        }
        return &it->second;
    }

    void context::push_scope(runtime::scope& current)
    {
        _scope_stack.push_back(&current);
    }

    bool context::pop_scope()
    {
        // Don't pop the top scope
        if (_scope_stack.size() == 1) {
            return false;
        }
        _scope_stack.pop_back();
        return true;
    }

    void context::define_class(types::klass klass, shared_ptr<compiler::context> context, ast::class_definition_expression const& expression)
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
        definitions.push_back(class_definition(rvalue_cast(klass), context, &expression));
    }

    runtime::resource* context::declare_class(types::klass const& klass, shared_ptr<string> path, lexer::position const& position, unordered_map<ast::name, values::value> const* arguments)
    {
        // Attempt to find the existing resource
        types::resource resource_type("class", klass.title());
        runtime::resource* resource = _catalog.find_resource(resource_type);
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
        resource = _catalog.add_resource(resource_type, rvalue_cast(path), position.line(), false);
        if (!resource) {
            return nullptr;
        }

        // Evaluate all definitions of the class
        for (auto& definition : it->second) {
            if (!definition.evaluate(*this, arguments)) {
                return nullptr;
            }
        }
        return resource;
    }

    bool context::is_class_defined(types::klass const& klass) const
    {
        auto it = _classes.find(klass);
        return it != _classes.end() && !it->second.empty();
    }

    bool context::is_class_declared(types::klass const& klass) const
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

    void context::define_type(string type, shared_ptr<compiler::context> context, ast::defined_type_expression const& expression)
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
        defined_type defined(type, rvalue_cast(context), expression);
        _defined_types.emplace(make_pair(rvalue_cast(type), rvalue_cast(defined)));
    }

    bool context::is_defined_type(std::string const& type) const
    {
        return _defined_types.count(type) > 0;
    }

    runtime::resource* context::declare_defined_type(string const& type, string const& title, shared_ptr<string> path, lexer::position const& position, unordered_map<ast::name, values::value> const* arguments)
    {
        // Lookup the defined type
        auto it = _defined_types.find(type);
        if (it == _defined_types.end()) {
            // TODO: search node for defined type
            return nullptr;
        }

        // Add a resource to the catalog
        auto added = _catalog.add_resource(types::resource(type, title), rvalue_cast(path), position.line(), false);
        if (!added) {
            return nullptr;
        }

        // Evaluate the defined type
        if (!it->second.evaluate(*this, *added, arguments)) {
            return nullptr;
        }
        return added;
    }

    void context::define_node(shared_ptr<compiler::context> context, ast::node_definition_expression const& expression)
    {
        // Emplace the node
        _nodes.emplace_back(rvalue_cast(context), expression);
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

    bool context::evaluate_node(compiler::node const& node)
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
        node_scope_helper helper(*this, rvalue_cast(scope_name));

        // Evaluate the node definition
        return definition->evaluate(*this);
    }

    void context::validate_parameters(bool klass, vector<ast::parameter> const& parameters)
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

    local_scope::local_scope(runtime::context& context, runtime::scope* scope) :
        _context(context),
        _scope(&context.scope())
    {
        if (scope) {
            _context.push_scope(*scope);
        } else {
            _context.push_scope(_scope);
        }
    }

    local_scope::~local_scope()
    {
        _context.pop_scope();
    }

}}  // namespace puppet::runtime
