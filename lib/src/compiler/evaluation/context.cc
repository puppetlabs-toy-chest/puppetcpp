#include <puppet/compiler/evaluation/context.hpp>
#include <puppet/compiler/evaluation/evaluator.hpp>
#include <puppet/compiler/lexer/lexer.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <puppet/cast.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation {

    static const size_t MAX_STACK_DEPTH = 1000;

    match_scope::match_scope(evaluation::context& context) :
        _context(context)
    {
        _context._match_stack.emplace_back();
    }

    match_scope::~match_scope()
    {
        _context._match_stack.pop_back();
    }

    node_scope::node_scope(evaluation::context& context, compiler::resource& resource) :
        _context(context)
    {
        // Create a node scope that inherits from the top scope
        _context._node_scope = make_shared<scope>(_context.top_scope(), &resource);
    }

    node_scope::~node_scope()
    {
        _context._node_scope.reset();
    }

    scoped_output_stream::scoped_output_stream(evaluation::context& context, ostream& stream) :
        _context(context)
    {
        _context._stream_stack.push_back(&stream);
    }

    scoped_output_stream::~scoped_output_stream()
    {
        _context._stream_stack.pop_back();
    }

    resource_relationship::resource_relationship(
        compiler::relationship relationship,
        values::value source,
        ast::context source_context,
        values::value target,
        ast::context target_context) :
            _relationship(relationship),
            _source(rvalue_cast(source)),
            _source_context(rvalue_cast(source_context)),
            _target(rvalue_cast(target)),
            _target_context(rvalue_cast(target_context))
    {
        if (source_context.tree) {
            _tree = source_context.tree->shared_from_this();
        } else if (target_context.tree) {
            _tree = target_context.tree->shared_from_this();
        }
    }

    compiler::relationship resource_relationship::relationship() const
    {
        return _relationship;
    }

    values::value const& resource_relationship::source() const
    {
        return _source;
    }

    ast::context const& resource_relationship::source_context() const
    {
        return _source_context;
    }

    values::value const& resource_relationship::target() const
    {
        return _target;
    }

    ast::context const& resource_relationship::target_context() const
    {
        return _target_context;
    }

    void resource_relationship::evaluate(evaluation::context& context, compiler::catalog& catalog) const
    {
        // Build a list of targets
        vector<resource*> targets;
        _target.each_resource([&](types::resource const& target_resource) {
            // Locate the target in the catalog
            auto target = catalog.find(target_resource);
            if (!target || target->virtualized()) {
                throw evaluation_exception(
                    (boost::format("cannot create relationship: resource %1% does not exist in the catalog.") %
                     target_resource
                    ).str(),
                    _target_context,
                    context.backtrace()
                );
            }
            targets.push_back(target);
        }, [&](string const& message) {
            throw evaluation_exception(message, _target_context, context.backtrace());
        });

        // Now add a relationship from each source
        _source.each_resource([&](types::resource const& source_resource) {
            // Locate the source in the catalog
            auto source = catalog.find(source_resource);
            if (!source || source->virtualized()) {
                throw evaluation_exception(
                    (boost::format("cannot create relationship: resource %1% does not exist in the catalog.") %
                     source_resource
                    ).str(),
                    _source_context,
                    context.backtrace()
                );
            }

            // Add a relationship to each target
            for (auto target : targets) {
                if (source == target) {
                    throw evaluation_exception(
                        (boost::format("resource %1% cannot form a relationship with itself.") %
                         source->type()
                        ).str(),
                        _source_context,
                        context.backtrace()
                    );
                }

                catalog.relate(_relationship, *source, *target);
            }
        }, [&](string const& message) {
            throw evaluation_exception(message, _source_context, context.backtrace());
        });
    }

    resource_override::resource_override(
        types::resource type,
        ast::context context,
        compiler::attributes attributes,
        std::shared_ptr<evaluation::scope> scope) :
            _type(rvalue_cast(type)),
            _context(rvalue_cast(context)),
            _attributes(rvalue_cast(attributes)),
            _scope(rvalue_cast(scope))
    {
        if (_context.tree) {
            _tree = _context.tree->shared_from_this();
        }
    }

    types::resource const& resource_override::type() const
    {
        return _type;
    }

    ast::context const& resource_override::context() const
    {
        return _context;
    }

    compiler::attributes const& resource_override::attributes() const
    {
        return _attributes;
    }

    shared_ptr<evaluation::scope> const& resource_override::scope() const
    {
        return _scope;
    }

    void resource_override::evaluate(evaluation::context& context, compiler::catalog& catalog) const
    {
        auto resource = catalog.find(_type);
        if (!resource) {
            throw evaluation_exception(
                (boost::format("resource %1% does not exist in the catalog.") %
                 _type
                ).str(),
                _context,
                context.backtrace()
            );
        }

        // No attributes? Nothing to do once we've checked existence
        if (_attributes.empty()) {
            return;
        }

        // Walk the parent scope looking for an associated resource that contains this one
        bool override = true;
        if (_scope) {
            override = false;
            auto parent = _scope->parent().get();
            while (parent && parent->parent()) {
                if (parent->resource() && (resource->container() == parent->resource())) {
                    override = true;
                    break;
                }
                parent = parent->parent().get();
            }
        }

        // If not allowing overrides, check for conflicts
        if (!override) {
            for (auto& pair : _attributes) {
                auto oper = pair.first;
                auto& attribute = pair.second;

                auto previous = resource->get(attribute->name());
                if (!previous) {
                    continue;
                }

                char const* action = "set";
                if (oper == ast::attribute_operator::assignment) {
                    if (attribute->value().is_undef()) {
                        action = "remove";
                    }
                } else if (oper == ast::attribute_operator::append) {
                    action = "append";
                } else {
                    throw runtime_error("unexpected attribute operator");
                }

                auto const& name_context = previous->name_context();
                throw evaluation_exception(
                    (boost::format("cannot %1% attribute '%2%' from resource %3% that was previously set at %4%:%5%.") %
                     action %
                     attribute->name() %
                     resource->type() %
                     name_context.tree->path() %
                     name_context.begin.line()
                    ).str(),
                    attribute->name_context(),
                    context.backtrace()
                );
            }
        }

        // Set the attributes
        for (auto& pair : _attributes) {
            switch (pair.first) {
                case ast::attribute_operator::assignment:
                    resource->set(pair.second);
                    break;

                case ast::attribute_operator::append:
                    resource->append(pair.second);
                    break;

                default:
                    throw runtime_error("unexpected attribute operator");
            }
        }
    }

    declared_defined_type::declared_defined_type(compiler::resource& resource, defined_type const& definition) :
        _resource(resource),
        _definition(definition)
    {
    }

    compiler::resource& declared_defined_type::resource() const
    {
        return _resource;
    }

    defined_type const& declared_defined_type::definition() const
    {
        return _definition;
    }

    scoped_stack_frame::scoped_stack_frame(evaluation::context& context, stack_frame frame) :
        match_scope(context)
    {
        if (_context._call_stack.size() > MAX_STACK_DEPTH) {
            auto context = _context._call_stack.empty() ? ast::context{} : _context._call_stack.back().current();
            throw evaluation_exception(
                (boost::format("cannot call '%1%': maximum stack depth reached.") % frame.name()).str(),
                rvalue_cast(context),
                _context.backtrace()
            );
        }
        _context._call_stack.emplace_back(rvalue_cast(frame));
    }

    scoped_stack_frame::~scoped_stack_frame()
    {
        _context._call_stack.pop_back();
    }

    context::context() :
        _node(nullptr),
        _catalog(nullptr),
        _registry(nullptr),
        _dispatcher(nullptr)
    {
    }

    context::context(compiler::node& node, compiler::catalog& catalog) :
        _node(&node),
        _catalog(&catalog),
        _registry(&node.environment().registry()),
        _dispatcher(&node.environment().dispatcher()),
        _top_scope(make_shared<scope>(node.facts()))
    {
    }

    compiler::node& context::node() const
    {
        if (!_node) {
            throw evaluation_exception("operation not permitted: node is not available.", backtrace());
        }
        return *_node;
    }

    compiler::catalog& context::catalog() const
    {
        if (!_catalog) {
            throw evaluation_exception("operation not permitted: catalog is not available.", backtrace());
        }
        return *_catalog;
    }

    compiler::registry const& context::registry() const
    {
        if (!_registry) {
            throw evaluation_exception("operation not permitted: registry is not available.", backtrace());
        }
        return *_registry;
    }

    evaluation::dispatcher const& context::dispatcher() const
    {
        if (!_dispatcher) {
            throw evaluation_exception("operation not permitted: function dispatcher is not available.", backtrace());
        }
        return *_dispatcher;
    }

    shared_ptr<scope> const& context::current_scope() const
    {
        if (_call_stack.empty()) {
            throw evaluation_exception("operation not permitted: the current scope is not available.", backtrace());
        }
        return _call_stack.back().scope();
    }

    shared_ptr<scope> const& context::top_scope() const
    {
        if (!_top_scope) {
            throw evaluation_exception("operation not permitted: the top scope is not available.", backtrace());
        }
        return _top_scope;
    }

    shared_ptr<scope> const& context::node_scope() const
    {
        return _node_scope;
    }

    shared_ptr<scope> const& context::node_or_top() const
    {
        return _node_scope ? _node_scope : _top_scope;
    }

    shared_ptr<scope> const& context::calling_scope() const
    {
        if (_call_stack.size() < 2) {
            throw evaluation_exception("operation not permitted: there is no calling scope.", backtrace());
        }
        return _call_stack[_call_stack.size() - 2].scope();
    }

    bool context::add_scope(std::shared_ptr<evaluation::scope> scope)
    {
        if (!scope) {
            throw runtime_error("expected a non-null scope.");
        }
        if (!scope->resource()) {
            throw runtime_error("expected a scope with an associated resource.");
        }
        string name = scope->resource()->type().title();
        return _named_scopes.emplace(rvalue_cast(name), rvalue_cast(scope)).second;
    }

    std::shared_ptr<scope> context::find_scope(string const& name) const
    {
        if (name.empty()) {
            return top_scope();
        }

        auto it = _named_scopes.find(name);
        if (it == _named_scopes.end()) {
            return nullptr;
        }
        return it->second;
    }

    void context::set(smatch const& matches)
    {
        if (_match_stack.empty()) {
            return;
        }

        auto& scope = _match_stack.back();

        // If there is no scope or a closure has captured the matches, reset
        if (!scope || !scope.unique()) {
            scope = make_shared<vector<shared_ptr<values::value const>>>();
        }

        scope->clear();
        scope->reserve(matches.size());

        for (auto const& match : matches) {
            scope->emplace_back(make_shared<values::value const>(match.str()));
        }
    }

    shared_ptr<values::value const> context::lookup(ast::variable const& expression, bool warn)
    {
        // Look for the last :: delimiter; if not found, use the current scope
        auto pos = expression.name.rfind("::");
        if (pos == string::npos) {
            return current_scope()->get(expression.name);
        }

        // Split into namespace and variable name
        // For global names, remove the leading ::
        bool global = boost::starts_with(expression.name, "::");
        auto ns = expression.name.substr(global ? 2 : 0, global ? (pos > 2 ? pos - 2 : 0) : pos);
        auto var = expression.name.substr(pos + 2);

        // Lookup the namespace
        auto scope = find_scope(ns);
        if (scope) {
            return scope->get(var);
        }

        if (warn) {
            string message;
            if (_registry && !_registry->find_class(ns)) {
                message = (boost::format("could not look up variable $%1% because class '%2%' is not defined.") % expression.name % ns).str();
            } else if (_catalog && !_catalog->find(types::resource("class", ns))) {
                message = (boost::format("could not look up variable $%1% because class '%2%' has not been declared.") % expression.name % ns).str();
            }

            if (!message.empty()) {
                log(logging::level::warning, message, &expression);
            }
        }
        return nullptr;
    }

    shared_ptr<values::value const> context::lookup(size_t index) const
    {
        // Walk the match scope stack for a non-null set of matches
        for (auto it = _match_stack.rbegin(); it != _match_stack.rend(); ++it) {
            auto const& matches = *it;
            if (matches) {
                if (index >= matches->size()) {
                    return nullptr;
                }
                return (*matches)[index];
            }
        }
        return nullptr;
    }

    vector<stack_frame> context::backtrace() const
    {
        return vector<stack_frame>{ _call_stack.rbegin(), _call_stack.rend() };
    }

    void context::current_context(ast::context context)
    {
        if (!_call_stack.empty()) {
            _call_stack.back().current(rvalue_cast(context));
        }
    }

    bool context::write(values::value const& value)
    {
        if (_stream_stack.empty()) {
            return false;
        }
        *_stream_stack.back() << value;
        return true;
    }

    bool context::write(char const* ptr, size_t size)
    {
        if (_stream_stack.empty()) {
            return false;
        }
        _stream_stack.back()->write(ptr, size);
        return true;
    }

    void context::log(logging::level level, string const& message, ast::context const* context)
    {
        auto& logger = node().logger();

        // Do nothing if the logger would not log at this level
        if (!logger.would_log(level)) {
            return;
        }

        // If given no context, just log the message
        if (!context || !context->tree) {
            logger.log(level, message);
            return;
        }

        auto& path = context->tree->path();
        size_t offset = context->begin.offset();
        size_t line = context->begin.line();
        size_t column = 0;
        size_t length = context->end.offset() - context->begin.offset();
        string text;

        if (context->tree->source().empty()) {
            ifstream input{ path };
            if (input) {
                tie(text, column) = lexer::get_text_and_column(input, offset);
            }
        } else {
            tie(text, column) = lexer::get_text_and_column(context->tree->source(), offset);
        }
        logger.log(level, line, column, length, text, path, message);
    }

    resource* context::declare_class(string name, ast::context const& context)
    {
        auto& catalog = this->catalog();

        // Ensure the name is in the expected format
        types::klass::normalize(name);

        // Find the class definition
        auto klass = find_class(name);
        if (!klass) {
            throw evaluation_exception(
                (boost::format("cannot evaluate class '%1%' because it has not been defined.") %
                 name
                ).str(),
                context,
                backtrace()
            );
        }

        // Find the resource
        auto type = types::resource("class", rvalue_cast(name));
        auto resource = catalog.find(type);
        if (!resource) {
            // Create the class resource
            resource = catalog.add(rvalue_cast(type), nullptr, nullptr, context);
        }

        // If the class was already declared, return it without evaluating
        if (!_classes.insert(resource->type().title()).second) {
            return resource;
        }

        // Validate the stage metaparameter
        compiler::resource const* stage = nullptr;
        if (auto attribute = resource->get("stage")) {
            auto ptr = attribute->value().as<string>();
            if (!ptr) {
                throw evaluation_exception(
                    (boost::format("expected %1% for 'stage' metaparameter but found %2%.") %
                     types::string::name() %
                     attribute->value().infer_type()
                    ).str(),
                    attribute->value_context(),
                    backtrace()
                );
            }
            stage = catalog.find(types::resource("stage", *ptr));
            if (!stage) {
                throw evaluation_exception(
                    (boost::format("stage '%1%' does not exist in the catalog.") %
                     *ptr
                    ).str(),
                    attribute->value_context(),
                    backtrace()
                );
            }
        } else {
            stage = catalog.find(types::resource("stage", "main"));
            if (!stage) {
                throw evaluation_exception("stage 'main' does not exist in the catalog.", backtrace());
            }
        }

        // Contain the class in the stage
        catalog.relate(relationship::contains, *stage, *resource);

        // Evaluate the class
        evaluation::class_evaluator evaluator{ *this, klass->expression() };
        evaluator.evaluate(*resource);
        return resource;
    }

    klass const* context::find_class(string name, bool import)
    {
        auto& registry = this->registry();

        types::klass::normalize(name);

        auto klass = registry.find_class(name);
        if (!klass && import && _node) {
            // Attempt to import the class
            _node->environment().import(_node->logger(), find_type::manifest, name);
            klass = registry.find_class(name);
        }
        return klass;
    }

    compiler::defined_type const* context::find_defined_type(string name, bool import)
    {
        auto& registry = this->registry();

        // Ensure the name is in the expected format
        types::klass::normalize(name);

        auto definition = registry.find_defined_type(name);
        if (!definition && import && _node) {
            // Attempt to import the defined type
            _node->environment().import(_node->logger(), find_type::manifest, name);

            // Find it again
            definition = registry.find_defined_type(name);
        }
        return definition;
    }

    functions::descriptor* context::find_function(string const& name, bool import)
    {
        return const_cast<functions::descriptor*>(static_cast<context const*>(this)->find_function(name, import));
    }

    functions::descriptor const* context::find_function(string const& name, bool import) const
    {
        auto& dispatcher = this->dispatcher();
        auto descriptor = dispatcher.find(name);
        if (!descriptor && import && _node) {
            // Attempt to import the function and find it again
            _node->environment().import(_node->logger(), find_type::function, name);
            descriptor = dispatcher.find(name);
        }
        return descriptor;
    }

    compiler::type_alias* context::find_type_alias(string const& name, bool import)
    {
        return const_cast<compiler::type_alias*>(static_cast<context const*>(this)->find_type_alias(name, import));
    }

    compiler::type_alias const* context::find_type_alias(string const& name, bool import) const
    {
        auto& registry = this->registry();

        auto alias = registry.find_type_alias(name);
        if (!alias && import && _node) {
            // Attempt to import the alias using a lowercase name and find it again
            _node->environment().import(_node->logger(), find_type::type, name);
            alias = registry.find_type_alias(name);
        }
        return alias;
    }

    shared_ptr<values::type> context::resolve_type_alias(string const& name)
    {
        auto it = _resolved_type_aliases.find(name);
        if (it != _resolved_type_aliases.end()) {
            return it->second;
        }

        auto alias = find_type_alias(name);
        if (!alias) {
            return nullptr;
        }

        // Push a frame indicating an alias resolution
        scoped_stack_frame frame{ *this, stack_frame{ &alias->expression(), current_scope() }};

        // Initially map to an Any type
        auto resolved = make_shared<values::type>();
        _resolved_type_aliases[name] = resolved;

        auto type = values::type::create(alias->expression().type, this);
        if (!type) {
            throw evaluation_exception(
                (boost::format("expected type alias '%1%' to evaluate to a type.") %
                 name
                ).str(),
                alias->expression().alias,
                backtrace()
            );
        }

        *resolved = rvalue_cast(*type);

        types::recursion_guard guard;
        if (!resolved->is_real(guard)) {
            throw evaluation_exception(
                (boost::format("%1% does not resolve to a real type.") %
                 *resolved
                ).str(),
                alias->expression().type.context(),
                backtrace()
            );
        }
        return resolved;
    }

    bool context::is_defined(string name, bool check_classes, bool checked_defined_types)
    {
        auto& registry = this->registry();

        if (!check_classes && !checked_defined_types) {
            return false;
        }

        types::klass::normalize(name);

        // Check for class or defined type
        if ((check_classes && registry.find_class(name)) || (checked_defined_types && registry.find_defined_type(name))) {
            return true;
        }

        if (!_node) {
            return false;
        }

        // Try to import a manifest with the name and check again
        _node->environment().import(_node->logger(), find_type::manifest, name);
        return (check_classes && registry.find_class(name)) || (checked_defined_types && registry.find_defined_type(name));
    }

    void context::add(resource_relationship relationship)
    {
        // Ensure there is a catalog
        catalog();

        _relationships.emplace_back(rvalue_cast(relationship));
    }

    void context::add(resource_override override)
    {
        auto& catalog = this->catalog();

        // Find the resource first
        auto resource = catalog.find(override.type());
        if (!resource) {
            // Not yet declared, so store for later
            auto type = override.type();
            _overrides.insert(make_pair(rvalue_cast(type), rvalue_cast(override)));
            return;
        }

        // Evaluate any existing overrides first
        evaluate_overrides(override.type());

        // Now evaluate the given override
        override.evaluate(*this, catalog);
    }

    void context::add(declared_defined_type defined_type)
    {
        // Ensure there is a catalog
        catalog();

        _defined_types.emplace_back(rvalue_cast(defined_type));
    }

    void context::add(shared_ptr<collectors::collector> collector)
    {
        // Ensure there is a catalog
        catalog();

        _collectors.emplace_back(rvalue_cast(collector));
    }

    void context::evaluate_overrides(runtime::types::resource const& resource)
    {
        auto& catalog = this->catalog();

        // Evaluate the overrides for the given type
        auto range = _overrides.equal_range(resource);
        for (auto it = range.first; it != range.second; ++it) {
            it->second.evaluate(*this, catalog);
        }
        _overrides.erase(resource);
    }

    void context::finalize()
    {
        auto& catalog = this->catalog();

        const size_t max_iterations = 1000;
        size_t iteration = 0;
        size_t index = 0;

        // Keep track of a list of defined types that are virtual
        vector<declared_defined_type*> virtualized;
        while (true) {
            // Run all collectors
            for (auto& collector : _collectors) {
                collector->collect(*this);
            }

            // After collection, if all defined types have been evaluated and the elements of the virtualized list are
            // still virtual, then there is nothing left to do
            if (index >= _defined_types.size() && std::all_of(virtualized.begin(), virtualized.end(), [](auto const& element) {
                return element->resource().virtualized();
            })) {
                break;
            }

            // Evaluate the defined types
            evaluate_defined_types(index, virtualized);

            // Guard against infinite recursion by limiting the number of loop iterations
            if (iteration++ >= max_iterations) {
                throw evaluation_exception("maximum defined type evaluations exceeded: a defined type may be infinitely recursive.", backtrace());
            }

            // Loop one more time so that collectors are run again
        }

        // Ensure there are no uncollected resources
        for (auto const& collector : _collectors) {
            collector->detect_uncollected(*this);
        }

        // Evaluate all resource relationships
        for (auto const& relationship : _relationships) {
            relationship.evaluate(*this, catalog);
        }

        // Evaluate any remaining overrides
        for (auto& kvp : _overrides) {
            kvp.second.evaluate(*this, catalog);
        }

        // Clear the data
        _classes.clear();
        _collectors.clear();
        _defined_types.clear();
        _relationships.clear();
        _overrides.clear();
    }

    void context::evaluate_defined_types(size_t& index, vector<declared_defined_type*>& virtualized)
    {
        resource const* current = nullptr;

        // Evaluate any previously virtual defined type
        virtualized.erase(remove_if(virtualized.begin(), virtualized.end(), [&](auto const& declared) {
            // Check to see if the resource is still virtual
            if (declared->resource().virtualized()) {
                return false;
            }
            // Evaluate the defined type
            current = &declared->resource();
            defined_type_evaluator evaluator{ *this, declared->definition().expression() };
            evaluator.evaluate(declared->resource());
            return true;
        }), virtualized.end());

        // Evaluate all non-virtual define types from the current start to the current end *only*
        // Any defined types that are added to the list as a result of the evaluation will be themselves
        // evaluated on the next pass.
        auto size = _defined_types.size();
        for (; index < size; ++index) {
            auto& declared = _defined_types[index];

            if (declared.resource().virtualized()) {
                // Defined type is virtual, enqueue it for later evaluation
                virtualized.emplace_back(&declared);
                continue;
            }
            current = &declared.resource();
            defined_type_evaluator evaluator{ *this, declared.definition().expression() };
            evaluator.evaluate(declared.resource());
        }
    }

}}}  // namespace puppet::compiler::evaluation
