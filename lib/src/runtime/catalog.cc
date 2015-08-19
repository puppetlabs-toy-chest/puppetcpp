#include <puppet/runtime/catalog.hpp>
#include <puppet/runtime/expression_evaluator.hpp>
#include <puppet/runtime/executor.hpp>
#include <puppet/compiler/node.hpp>
#include <puppet/ast/expression_def.hpp>
#include <boost/graph/tiernan_all_cycles.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;

namespace boost {
    // Helper needed for dependency cycle detection
    void renumber_vertex_indices(puppet::runtime::dependency_graph const&)
    {
    }
}

namespace puppet { namespace runtime {

    attributes::attributes(shared_ptr<runtime::attributes const> parent) :
        _parent(rvalue_cast(parent))
    {
    }

    shared_ptr<values::value const> attributes::get(string const& name, bool check_parent) const
    {
        // Check the values first
        auto it = _values.find(name);
        if (it != _values.end()) {
            return is_undef(*it->second) ? nullptr : it->second;
        }
        // Check the parent if there is one
        return (check_parent && _parent) ? _parent->get(name) : nullptr;
    }

    void attributes::set(string const& name, values::value value)
    {
        // Set the value
        _values[name] = make_shared<values::value>(rvalue_cast(value));
    }

    bool attributes::append(string const& name, values::value value, bool append_duplicates)
    {
        values::array new_value = to_array(value);

        // Check to see if the attribute already exists
        auto existing = get(name);
        if (!existing) {
            set(name, rvalue_cast(new_value));
            return true;
        }

        // Ensure the attribute is an array
        if (!as<values::array>(*existing)) {
            return false;
        }

        values::array existing_value;
        auto it = _values.find(name);
        if (it != _values.end()) {
            // The value is stored locally, so mutate the existing one
            existing_value = mutate_as<values::array>(*it->second);
        } else {
            // Otherwise, it's stored in a parent and cannot be changed; create a copy
            existing_value = *as<values::array>(*existing);
        }

        // Move the elements from the value into the existing array
        existing_value.reserve(existing_value.size() + new_value.size());
        for (auto& element : new_value) {
            // Check to see if the value already exists, if requested to do so
            if (append_duplicates && find_if(existing_value.begin(), existing_value.end(), [&](values::value const& v) { return values::equals(v, element); }) != existing_value.end()) {
                continue;
            }
            existing_value.emplace_back(rvalue_cast(element));
        }

        // Set the appended value
        set(name, rvalue_cast(existing_value));
        return true;
    }

    void attributes::each(function<bool(string const& name, shared_ptr<values::value const> const& value)> const& callback) const
    {
        // Enumerate this collection's values first
        for (auto const& element : _values) {
            // Skip undefined elements as they aren't set
            if (is_undef(*element.second)) {
                continue;
            }
            if (!callback(element.first, element.second)) {
                return;
            }
        }

        // Enumerate the parent and call the callback for any attribute not in this collection
        if (_parent) {
            _parent->each([&](string const& name, shared_ptr<values::value const> const& value) {
                return _values.count(name) != 0 || callback(name, value);
            });
        }
    }

    resource::resource(
        runtime::catalog& catalog,
        types::resource type,
        std::shared_ptr<std::string> path,
        size_t line,
        std::shared_ptr<runtime::attributes> attributes,
        bool exported) :
            _catalog(catalog),
            _type(rvalue_cast(type)),
            _path(rvalue_cast(path)),
            _line(line),
            _attributes(rvalue_cast(attributes)),
            _exported(exported)
    {
        if (!_path) {
            throw evaluation_exception("a declared resource must have a path.");
        }
        if (!_attributes) {
            _attributes = make_shared<runtime::attributes>();
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

    shared_ptr<string> const& resource::path() const
    {
        return _path;
    }

    size_t resource::line() const
    {
        return _line;
    }

    bool resource::exported() const
    {
        return _exported;
    }

    runtime::attributes& resource::attributes()
    {
        return *_attributes;
    }

    runtime::attributes const& resource::attributes() const
    {
        return *_attributes;
    }

    void resource::make_attributes_unique()
    {
        if (_attributes.unique()) {
            return;
        }

        // Create a new set of attributes inherited from the current one
        _attributes = make_shared<runtime::attributes>(_attributes);
    }

    bool resource::is_metaparameter(string const& name)
    {
        static const unordered_set<string> metaparameters = {
            "alias",
            "audit",
            "before",
            "loglevel",
            "noop",
            "notify",
            "require",
            "schedule",
            "stage",
            "subscribe",
            "tag"
        };
        return metaparameters.count(name) > 0;
    }

    size_t resource::vertex_id() const
    {
        return _vertex_id;
    }

    void resource::vertex_id(size_t id)
    {
        _vertex_id = id;
    }

    class_definition::class_definition(types::klass klass, shared_ptr<compiler::context> context, ast::class_definition_expression const& expression) :
        _klass(rvalue_cast(klass)),
        _context(rvalue_cast(context)),
        _expression(&expression)
    {
        if (!_context) {
            throw evaluation_exception("a class definition must have a compilation context.");
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

    boost::optional<types::klass> const& class_definition::parent() const
    {
        return _parent;
    }

    shared_ptr<string> const& class_definition::path() const
    {
        return _path;
    }

    size_t class_definition::line() const
    {
        return _line;
    }

    void class_definition::evaluate(runtime::context& context, runtime::resource& resource)
    {
        if (!_context || !_expression) {
            return;
        }

        // Create a scope for the class
        auto scope = make_shared<runtime::scope>(evaluate_parent(context), &resource);

        // Add the class' scope
        context.add_scope(scope);

        // Create a new expression evaluator based on the class' compilation context
        expression_evaluator evaluator{ _context, context };

        // Execute the body of the class
        runtime::executor executor{ evaluator, _expression->position(), _expression->parameters(), _expression->body() };
        executor.execute(resource, scope);

        // Reset the context; classes cannot be evaluated more than once
        _context.reset();
        _expression = nullptr;
    }

    shared_ptr<runtime::scope> class_definition::evaluate_parent(runtime::context& context)
    {
        // If no parent, return the node or top scope
        auto const& parent = this->parent();
        if (!parent) {
            return context.node_or_top();
        }

        // If the parent isn't declared, declare it now
        auto catalog = context.catalog();
        types::resource type("class", parent->title());
        if (!catalog->find_resource(type)) {
            // Declare the parent class
            catalog->declare_class(context, rvalue_cast(type), _context, _expression->parent()->position());
        }
        return context.find_scope(parent->title());
    }

    defined_type::defined_type(string type, shared_ptr<compiler::context> context, ast::defined_type_expression const& expression) :
        _type(rvalue_cast(type)),
        _context(rvalue_cast(context)),
        _expression(expression)
    {
        if (!_context) {
            throw evaluation_exception("a defined type definition must have a compilation context.");
        }
    }

    string const& defined_type::type() const
    {
        return _type;
    }

    shared_ptr<string> const& defined_type::path() const
    {
        return _context->path();
    }

    size_t defined_type::line() const
    {
        return _expression.position().line();
    }

    void defined_type::evaluate(runtime::context& context, runtime::resource& resource)
    {
        // Create a temporary scope for evaluating the defined type
        auto scope = make_shared<runtime::scope>(context.node_or_top(), &resource);

        // Create a new expression evaluator based on the defined type's compilation context
        expression_evaluator evaluator{ _context, context };

        // Execute hte body of the defined type
        runtime::executor executor{ evaluator, _expression.position(), _expression.parameters(), _expression.body() };
        executor.execute(resource, scope);
    }

    node_definition::node_definition(shared_ptr<compiler::context> context, ast::node_definition_expression const& expression) :
        _context(rvalue_cast(context)),
        _expression(expression)
    {
        if (!_context) {
            throw evaluation_exception("a node definition must have a compilation context.");
        }
    }

    shared_ptr<compiler::context> const& node_definition::context() const
    {
        return _context;
    }

    lexer::position const& node_definition::position() const
    {
        return _expression.position();
    }

    void node_definition::evaluate(runtime::context& context)
    {
        // Create a new expression evaluator based on the node's compilation context
        expression_evaluator evaluator{ _context, context };

        // Execute the node's body
        runtime::executor executor(evaluator, _expression.position(), boost::none, _expression.body());
        executor.execute(context.node_scope());
    }

    dependency_graph const& catalog::graph() const
    {
        return _graph;
    }

    resource* catalog::find_resource(types::resource const& resource)
    {
        if (!resource.fully_qualified()) {
            return nullptr;
        }

        // Find the resource type and title
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

    runtime::resource& catalog::add_resource(types::resource type, shared_ptr<string> path, size_t line, shared_ptr<runtime::attributes> attributes, bool exported)
    {
        if (!type.fully_qualified()) {
            throw evaluation_exception("resource name is not fully qualified.");
        }

        string resource_type = type.type_name();
        string title = type.title();

        auto& resources = _resources[rvalue_cast(resource_type)];

        auto result = resources.emplace(make_pair(rvalue_cast(title), runtime::resource(*this, rvalue_cast(type), rvalue_cast(path), line, rvalue_cast(attributes), exported)));
        auto& resource = result.first->second;
        if (!result.second) {
            throw evaluation_exception((boost::format("resource %1% was previously declared at %2%:%3%.") % type % *resource.path() % resource.line()).str());
        }

        // Add a graph vertex for the resource
        resource.vertex_id(boost::add_vertex(&resource, _graph));
        return resource;
    }

    vector<class_definition> const* catalog::find_class(types::klass const& klass)
    {
        auto it = _classes.find(klass);
        if (it == _classes.end() || it->second.empty()) {
            return nullptr;
        }
        return &it->second;
    }

    void catalog::define_class(
        types::klass klass,
        shared_ptr<compiler::context> const& context,
        ast::class_definition_expression const& expression)
    {
        auto& definitions = _classes[klass];
        definitions.push_back(class_definition(rvalue_cast(klass), context, expression));
    }

    runtime::resource& catalog::declare_class(
        runtime::context& evaluation_context,
        types::resource type,
        shared_ptr<compiler::context> const& compilation_context,
        lexer::position const& position,
        shared_ptr<runtime::attributes> attributes)
    {
        if (!compilation_context) {
            throw evaluation_exception("expected a compilation context.");
        }
        if (!type.is_class()) {
            throw evaluation_exception("expected a class resource.", compilation_context, position);
        }
        if (!type.fully_qualified()) {
            throw evaluation_exception("class name is not fully qualified.", compilation_context, position);
        }

        // Attempt to find the existing resource
        if (auto existing = find_resource(type)) {
            throw evaluation_exception((boost::format("class '%1%' was previously declared at %2%:%3%.") % type.title() % *existing->path() % existing->line()).str(), compilation_context, position);
        }

        // Lookup the class
        auto it = _classes.find(types::klass(type.title()));
        if (it == _classes.end() || it->second.empty()) {
            // TODO: search node for class
            throw evaluation_exception((boost::format("cannot declare class '%1%' because it has not been defined.") % type.title()).str(), compilation_context, position);
        }

        // Add the resource
        auto& resource = add_resource(rvalue_cast(type), compilation_context->path(), position.line(), attributes);

        try {
            // Evaluate all definitions of the class
            for (auto &definition : it->second) {
                definition.evaluate(evaluation_context, resource);
            }
        } catch (evaluation_exception const& ex) {
            // If the original exception has context, log an error with full context
            if (ex.context()) {
                ex.context()->log(logging::level::error, ex.position(), ex.what());
                throw evaluation_exception((boost::format("failed to evaluate class '%1%'.") % resource.type().title()).str(), compilation_context, position);
            }
            throw evaluation_exception((boost::format("failed to evaluate class '%1%': %2%") % resource.type().title() % ex.what()).str(), compilation_context, position);
        }
        return resource;
    }

    defined_type const* catalog::find_defined_type(string const& type)
    {
        auto it = _defined_types.find(type);
        if (it == _defined_types.end()) {
            return nullptr;
        }
        return &it->second;
    }

    void catalog::define_type(
        string type,
        shared_ptr<compiler::context> const& context,
        ast::defined_type_expression const& expression)
    {
        // Add the defined type
        defined_type defined(type, context, expression);
        auto result = _defined_types.emplace(make_pair(rvalue_cast(type), rvalue_cast(defined)));
        if (!result.second) {
            auto const& existing = result.first->second;
            throw evaluation_exception(
                (boost::format("defined type '%1%' was previously defined at %2%:%3%.") %
                 existing.type() %
                 *existing.path() %
                 existing.line()
                ).str(),
                context,
                expression.name().position());
        }
    }

    runtime::resource& catalog::declare_defined_type(
        runtime::context& evaluation_context,
        types::resource type,
        shared_ptr<compiler::context> const& compilation_context,
        lexer::position const& position,
        shared_ptr<runtime::attributes> attributes)
    {
        if (!compilation_context) {
            throw evaluation_exception("expected a compilation context.");
        }
        if (!type.fully_qualified()) {
            throw evaluation_exception("defined type name is not fully qualified.", compilation_context, position);
        }

        // Attempt to find the existing resource
        if (auto existing = find_resource(type)) {
            throw evaluation_exception((boost::format("defined type '%1%' was previously declared at %2%:%3%.") % type % *existing->path() % existing->line()).str(), compilation_context, position);
        }

        // Lookup the defined type
        auto it = _defined_types.find(type.title());
        if (it == _defined_types.end()) {
            // TODO: search node for defined type
            throw evaluation_exception((boost::format("cannot declare defined type %1% because it has not been defined.") % type).str(), compilation_context, position);
        }

        // Add the resource
        auto& resource = add_resource(rvalue_cast(type), compilation_context->path(), position.line(), attributes);

        try {
            // Evaluate the defined type
            it->second.evaluate(evaluation_context, resource);
        } catch (evaluation_exception const& ex) {
            // If the original exception has context, log an error with full context
            if (ex.context()) {
                ex.context()->log(logging::level::error, ex.position(), ex.what());
                throw evaluation_exception((boost::format("failed to evaluate defined type '%1%'.") % resource.type()).str(), compilation_context, position);
            }
            throw evaluation_exception((boost::format("failed to evaluate defined type '%1%': %2%") % resource.type() % ex.what()).str(), compilation_context, position);
        }
        return resource;
    }

    void catalog::define_node(shared_ptr<compiler::context> const& context, ast::node_definition_expression const& expression)
    {
        // Emplace the node
        _nodes.emplace_back(context, expression);
        size_t node_index = _nodes.size() - 1;

        for (auto const& name : expression.names()) {
            // Check for default node
            if (name.is_default()) {
                if (!_default_node_index) {
                    _default_node_index = node_index;
                    continue;
                }
                auto const& previous = _nodes[*_default_node_index];
                throw evaluation_exception((boost::format("a default node was previously defined at %1%:%2%.") % *previous.context()->path() % previous.position().line()).str(), context, name.position());
            }
            // Check for regular expression names
            if (name.regex()) {
                auto it = find_if(_regex_nodes.begin(), _regex_nodes.end(), [&](std::pair<values::regex, size_t> const& existing) { return existing.first.pattern() == name.value(); });
                if (it != _regex_nodes.end()) {
                    auto const& previous = _nodes[it->second];
                    throw evaluation_exception((boost::format("node /%1%/ was previously defined at %2%:%3%.") % name.value() % *previous.context()->path() % previous.position().line()).str(), context, name.position());
                }

                try {
                    _regex_nodes.emplace_back(values::regex(name.value()), node_index);
                } catch (regex_error const& ex) {
                    throw evaluation_exception((boost::format("invalid regular expression: %1%") % ex.what()).str(), context, name.position());
                }
                continue;
            }
            // Otherwise, this is a qualified node name
            auto it = _named_nodes.find(name.value());
            if (it != _named_nodes.end()) {
                auto const& previous = _nodes[it->second];
                throw evaluation_exception((boost::format("node '%1%' was previously defined at %2%:%3%.") % name.value() % *previous.context()->path() % previous.position().line()).str(), context, name.position());
            }
            _named_nodes.emplace(make_pair(boost::to_lower_copy(name.value()), node_index));
        }
    }

    runtime::resource* catalog::declare_node(runtime::context& evaluation_context, compiler::node const& node)
    {
        // If there are no node definitions, do nothing
        if (_nodes.empty()) {
            return nullptr;
        }

        // Find a node definition
        string node_name;
        node_definition* definition = nullptr;
        node.each_name([&](string const& name) {
            // First check by name
            auto it = _named_nodes.find(name);
            if (it != _named_nodes.end()) {
                node_name = it->first;
                definition = &_nodes[it->second];
                return false;
            }
            // Next, check by looking at every regex
            for (auto const& kvp : _regex_nodes) {
                if (regex_search(name, kvp.first.value())) {
                    node_name = "/" + kvp.first.pattern() + "/";
                    definition = &_nodes[kvp.second];
                    return false;
                }
            }
            return true;
        });

        if (!definition) {
            if (!_default_node_index) {
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
                throw evaluation_exception(message.str());
            }
            node_name = "default";
            definition = &_nodes[*_default_node_index];
        }

        // Add a Node resource to the catalog
        auto& context = definition->context();
        auto& position = definition->position();
        auto& resource = add_resource(types::resource("node", node_name), context->path(), position.line());

        // Set the node scope for the remainder of the evaluation
        node_scope scope{evaluation_context, &resource};

        // Evaluate the node definition
        try {
            definition->evaluate(evaluation_context);
        } catch (evaluation_exception const& ex) {
            // If the original exception has context, log an error with full context
            if (ex.context()) {
                ex.context()->log(logging::level::error, ex.position(), ex.what());
                throw evaluation_exception("failed to evaluate node.", context, position);
            }
            throw evaluation_exception((boost::format("failed to evaluate node: %2%.") % ex.what()).str(), context, position);
        }
        return &resource;
    }

    void catalog::finalize()
    {
        // Populate the dependency graph
        populate_graph();
    }

    void catalog::populate_graph()
    {
        string before_parameter = "before";
        string notify_parameter = "notify";
        string require_parameter = "require";
        string subscribe_parameter = "subscribe";

        // Loop through each resource and add relationships
        for (auto const& types_pair : _resources) {
            for (auto const& resource_pair : types_pair.second) {
                auto const& source = resource_pair.second;

                // Proccess the relationship metaparameters for the resource
                process_relationship_parameter(source, before_parameter, runtime::relationship::before);
                process_relationship_parameter(source, notify_parameter, runtime::relationship::notify);
                process_relationship_parameter(source, require_parameter, runtime::relationship::require);
                process_relationship_parameter(source, subscribe_parameter, runtime::relationship::subscribe);

                // TODO: handle containment
            }
        }

        // Finally, detect any cycles
        detect_cycles();
    }

    void catalog::process_relationship_parameter(resource const& source, string const& name, runtime::relationship relationship)
    {
        auto parameter = source.attributes().get(name);
        if (!parameter) {
            return;
        }
        each_resource(*parameter, [&](types::resource const& target_resource) {
            // Locate the target in the catalog
            auto target = find_resource(target_resource);
            if (!target) {
                throw evaluation_exception(
                    (boost::format("resource %1% (declared at %2%:%3%) cannot form a '%4%' relationship with resource %5%: the resource does not exist in the catalog.") %
                     source.type() %
                     *source.path() %
                     source.line() %
                     name %
                     target_resource).str());
            }

            if (&source == target) {
                throw evaluation_exception(
                    (boost::format("resource %1% (declared at %2%:%3%) cannot form a '%4%' relationship with resource %5%: the relationship is self-referencing.") %
                     source.type() %
                     *source.path() %
                     source.line() %
                     name %
                     target_resource).str());
            }

            // Add the relationship
            add_relationship(relationship, source, *target);
        }, [&](string const& message) {
            throw evaluation_exception(
                (boost::format("resource %1% (declared at %2%:%3%) cannot form a '%4%' relationship: %5%") %
                 source.type() %
                 *source.path() %
                 source.line() %
                 name %
                 message).str());
        });
    }

    void catalog::add_relationship(runtime::relationship relationship, runtime::resource const& source, runtime::resource const& target)
    {
        auto source_ptr = &source;
        auto target_ptr = &target;

        // For "reverse" relationships (require and subscribe), swap the target and source vertices
        if (relationship == runtime::relationship::require || relationship == runtime::relationship::subscribe) {
            source_ptr = &target;
            target_ptr = &source;
        }

        // Add the edge to the graph if it doesn't already exist
        if (!boost::edge(source_ptr->vertex_id(), target_ptr->vertex_id(), _graph).second) {
            boost::add_edge(source_ptr->vertex_id(), target_ptr->vertex_id(), relationship, _graph);
        }
    }

    struct cycle_visitor
    {
        explicit cycle_visitor(vector<string>& cycles) :
            _cycles(cycles)
        {
        }

        template <typename Path, typename Graph>
        void cycle(Path const& path, Graph const& graph)
        {
            ostringstream cycle;
            bool first = true;
            for (auto const& id : path) {
                if (first) {
                    first = false;
                } else {
                    cycle << " => ";
                }
                auto resource = graph[id];
                cycle << resource->type() << " declared at " << *resource->path() << ":" << resource->line();
            }
            // Append on the first vertex again to complete the cycle
            auto resource = graph[path.front()];
            cycle << " => " << resource->type();
            _cycles.push_back(cycle.str());
        }

        vector<string>& _cycles;
    };

    void catalog::detect_cycles()
    {
        // Check for cycles in the graph
        vector<string> cycles;
        boost::tiernan_all_cycles(_graph, cycle_visitor(cycles));
        if (cycles.empty()) {
            return;
        }

        // At least one cycle found, so throw an exception
        ostringstream message;
        message << "found " << cycles.size() << " resource dependency cycle" << (cycles.size() == 1 ? ":\n" : "s:\n");
        for (size_t i = 0; i < cycles.size(); ++i) {
            if (i > 0) {
                message << "\n";
            }
            message << "  " << i + 1 << ". " << cycles[i];
        }
        throw compiler::compilation_exception(message.str());
    }

}}  // namespace puppet::runtime
