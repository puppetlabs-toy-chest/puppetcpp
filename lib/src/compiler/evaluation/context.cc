#include <puppet/compiler/evaluation/context.hpp>
#include <puppet/compiler/lexer/lexer.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <puppet/cast.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation {

    match_scope::match_scope(evaluation::context& context) :
        _context(context)
    {
        _context._match_stack.emplace_back();
    }

    match_scope::~match_scope()
    {
        _context._match_stack.pop_back();
    }

    local_scope::local_scope(evaluation::context& context, shared_ptr<evaluation::scope> scope) :
        match_scope(context),
        _context(context)
    {
        if (!scope) {
            scope = make_shared<evaluation::scope>(_context.current_scope());
        }

        // Push the named scope onto the stack
        _context._scope_stack.emplace_back(rvalue_cast(scope));
    }

    local_scope::~local_scope()
    {
        _context._scope_stack.pop_back();
    }

    node_scope::node_scope(evaluation::context& context, compiler::resource& resource) :
        _context(context)
    {
        // Create a node scope that inherits from the top scope
        _context._node_scope = make_shared<scope>(_context.top_scope(), &resource);
        _context._scope_stack.push_back(_context._node_scope);
    }

    node_scope::~node_scope()
    {
        _context._scope_stack.pop_back();
        _context._node_scope.reset();
    }

    local_output_stream::local_output_stream(evaluation::context& context, ostream& stream) :
        _context(context)
    {
        _context._stream_stack.push_back(&stream);
    }

    local_output_stream::~local_output_stream()
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

    void resource_relationship::evaluate(compiler::catalog& catalog) const
    {
        // Build a list of targets
        vector<resource*> targets;
        _target.each_resource([&](types::resource const& target_resource) {
            // Locate the target in the catalog
            auto target = catalog.find(target_resource);
            if (!target || target->virtualized()) {
                throw evaluation_exception((boost::format("cannot create relationship: resource %1% does not exist in the catalog.") % target_resource).str(), _target_context);
            }
            targets.push_back(target);
        }, [&](string const& message) {
            throw evaluation_exception(message, _target_context);
        });

        // Now add a relationship from each source
        _source.each_resource([&](types::resource const& source_resource) {
            // Locate the source in the catalog
            auto source = catalog.find(source_resource);
            if (!source || source->virtualized()) {
                throw evaluation_exception((boost::format("cannot create relationship: resource %1% does not exist in the catalog.") % source_resource).str(), _source_context);
            }

            // Add a relationship to each target
            for (auto target : targets) {
                if (source == target) {
                    throw evaluation_exception(
                        (boost::format("resource %1% cannot form a relationship with itself.") %
                         source->type()
                        ).str(),
                        _source_context);
                }

                catalog.relate(_relationship, *source, *target);
            }
        }, [&](string const& message) {
            throw evaluation_exception(message, _source_context);
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

    void resource_override::evaluate(compiler::catalog& catalog) const
    {
        auto resource = catalog.find(_type);
        if (!resource) {
            throw evaluation_exception(
                (boost::format("resource %1% does not exist in the catalog.") %
                 _type
                ).str(),
                _context
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

        // Override the attributes
        resource->apply(_attributes, override);
    }

    declared_defined_type::declared_defined_type(compiler::resource& resource, defined_type const& definition) :
        _resource(resource),
        _definition(definition)
    {
    }

    compiler::resource const& declared_defined_type::resource() const
    {
        return _resource;
    }

    defined_type const& declared_defined_type::definition() const
    {
        return _definition;
    }

    void declared_defined_type::evaluate(evaluation::context& context) const
    {
        _definition.evaluate(context, _resource);
    }

    context::context(bool create_scope) :
        _node(nullptr),
        _catalog(nullptr),
        _registry(nullptr),
        _dispatcher(nullptr)
    {
        if (create_scope) {
            create_top_scope();
        }
    }

    context::context(compiler::node& node, compiler::catalog& catalog) :
        _node(&node),
        _catalog(&catalog),
        _registry(&node.environment().registry()),
        _dispatcher(&node.environment().dispatcher())
    {
        create_top_scope(node.facts());
    }

    compiler::node& context::node() const
    {
        if (!_node) {
            throw evaluation_exception("operation not permitted: node is not available.");
        }
        return *_node;
    }

    compiler::catalog& context::catalog() const
    {
        if (!_catalog) {
            throw evaluation_exception("operation not permitted: catalog is not available.");
        }
        return *_catalog;
    }

    compiler::registry const& context::registry() const
    {
        if (!_registry) {
            throw evaluation_exception("operation not permitted: registry is not available.");
        }
        return *_registry;
    }

    evaluation::dispatcher const& context::dispatcher() const
    {
        if (!_dispatcher) {
            throw evaluation_exception("operation not permitted: function dispatcher is not available.");
        }
        return *_dispatcher;
    }

    shared_ptr<scope> const& context::current_scope()
    {
        if (_scope_stack.empty()) {
            throw evaluation_exception("operation not permitted: the current scope is not available.");
        }
        return _scope_stack.back();
    }

    shared_ptr<scope> const& context::top_scope()
    {
        if (_scope_stack.empty()) {
            throw evaluation_exception("operation not permitted: the top scope is not available.");
        }
        return _scope_stack.front();
    }

    shared_ptr<scope> const& context::node_scope()
    {
        return _node_scope;
    }

    shared_ptr<scope> const& context::node_or_top()
    {
        if (_node_scope) {
            return _node_scope;
        }
        return top_scope();
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
        return _scopes.emplace(rvalue_cast(name), rvalue_cast(scope)).second;
    }

    std::shared_ptr<scope> context::find_scope(string const& name) const
    {
        auto it = _scopes.find(name);
        if (it == _scopes.end()) {
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

        // An empty namespace is the top scope
        if (ns.empty()) {
            return top_scope()->get(var);
        }

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

    match_scope context::create_match_scope()
    {
        return match_scope { *this };
    }

    local_scope context::create_local_scope(shared_ptr<evaluation::scope> scope)
    {
        return local_scope { *this, rvalue_cast(scope) };
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

        // Find the class definitions
        auto definitions = registry().find_class(name);
        if (!definitions) {
            auto& node = this->node();

            // Attempt to import the class
            node.environment().import(node.logger(), find_type::manifest, name);

            // Search again
            definitions = registry().find_class(name);
            if (!definitions) {
                throw evaluation_exception((boost::format("cannot evaluate class '%1%' because it has not been defined.") % name).str(), context);
            }
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
                     attribute->value().get_type()
                    ).str(),
                    attribute->value_context());
            }
            stage = catalog.find(types::resource("stage", *ptr));
            if (!stage) {
                throw evaluation_exception(
                    (boost::format("stage '%1%' does not exist in the catalog.") %
                     *ptr
                    ).str(),
                    attribute->value_context());
            }
        } else {
            stage = catalog.find(types::resource("stage", "main"));
            if (!stage) {
                throw evaluation_exception("stage 'main' does not exist in the catalog.");
            }
        }

        // Contain the class in the stage
        catalog.relate(relationship::contains, *stage, *resource);

        try {
            // Evaluate all definitions of the class
            for (auto& definition : *definitions) {
                definition.evaluate(*this, *resource);
            }
        } catch (evaluation_exception const& ex) {
            // Log the original exception and throw that evaluation failed
            log(logging::level::error, ex.what(), &ex.context());
            throw evaluation_exception(
                (boost::format("failed to evaluate class '%1%'.") %
                 resource->type().title()
                ).str(),
                context);
        }
        return resource;
    }

    vector<klass> const* context::find_class(string name, bool import)
    {
        auto& registry = this->registry();

        // Ensure the name is in the expected format
        types::klass::normalize(name);

        auto definitions = registry.find_class(name);
        if (!definitions && import) {
            auto& node = this->node();

            // Attempt to import the class
            node.environment().import(node.logger(), find_type::manifest, name);

            // Find it again
            definitions = registry.find_class(name);
        }
        return definitions;
    }

    compiler::defined_type const* context::find_defined_type(string name, bool import)
    {
        auto& registry = this->registry();

        // Ensure the name is in the expected format
        types::klass::normalize(name);

        auto definition = registry.find_defined_type(name);
        if (!definition && import) {
            auto& node = this->node();

            // Attempt to import the defined type
            node.environment().import(node.logger(), find_type::manifest, name);

            // Find it again
            definition = registry.find_defined_type(name);
        }
        return definition;
    }

    bool context::is_defined(string name, bool klass, bool defined_type)
    {
        auto& registry = this->registry();

        if (!klass && !defined_type) {
            return false;
        }

        // Ensure the name is in the expected format
        types::klass::normalize(name);

        // Check for class or defined type
        if ((klass && registry.find_class(name)) || (defined_type && registry.find_defined_type(name))) {
            return true;
        }

        auto& node = this->node();

        // Try to import a manifest with the name
        node.environment().import(node.logger(), find_type::manifest, name);

        // Check again for class or defined type
        return (klass && registry.find_class(name)) || (defined_type && registry.find_defined_type(name));
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
        override.evaluate(catalog);
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
            it->second.evaluate(catalog);
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
                throw evaluation_exception("maximum defined type evaluations exceeded: a defined type may be infinitely recursive.");
            }

            // Loop one more time so that collectors are run again
        }

        // Ensure there are no uncollected resources
        for (auto const& collector : _collectors) {
            collector->detect_uncollected();
        }

        // Evaluate all resource relationships
        for (auto const& relationship : _relationships) {
            relationship.evaluate(catalog);
        }

        // Evaluate any remaining overrides
        for (auto& kvp : _overrides) {
            kvp.second.evaluate(catalog);
        }

        // Clear the data
        _classes.clear();
        _collectors.clear();
        _defined_types.clear();
        _relationships.clear();
        _overrides.clear();
    }

    void context::create_top_scope(std::shared_ptr<facts::provider> facts)
    {
        if (!_scopes.empty()) {
            return;
        }

        // Add the top scope
        auto top = make_shared<scope>(rvalue_cast(facts));
        _scopes.emplace("", top);
        _scope_stack.emplace_back(rvalue_cast(top));

        // Add an empty top match scope
        _match_stack.emplace_back();
    }

    void context::evaluate_defined_types(size_t& index, vector<declared_defined_type*>& virtualized)
    {
        resource const* current = nullptr;

        try {
            // Evaluate any previously virtual defined type
            virtualized.erase(remove_if(virtualized.begin(), virtualized.end(), [&](auto const& declared) {
                // Check to see if the resource is still virtual
                if (declared->resource().virtualized()) {
                    return false;
                }
                // Evaluate the defined type
                current = &declared->resource();
                declared->evaluate(*this);
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
                declared.evaluate(*this);
            }
        } catch (evaluation_exception const& ex) {
            // Log the original exception and throw that evaluation failed
            log(logging::level::error, ex.what(), &ex.context());
            throw evaluation_exception(
                (boost::format("failed to evaluate defined type '%1%'.") %
                 current->type()
                ).str(),
                current->context());
        }
    }

}}}  // namespace puppet::compiler::evaluation
