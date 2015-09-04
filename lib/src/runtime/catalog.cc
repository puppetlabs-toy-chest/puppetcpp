#include <puppet/runtime/catalog.hpp>
#include <puppet/runtime/expression_evaluator.hpp>
#include <puppet/runtime/executor.hpp>
#include <puppet/compiler/node.hpp>
#include <puppet/ast/expression_def.hpp>
#include <boost/graph/tiernan_all_cycles.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/format.hpp>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <ctime>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::runtime::values;
using namespace rapidjson;

namespace boost {
    // Helper needed for dependency cycle detection
    void renumber_vertex_indices(puppet::runtime::dependency_graph const&)
    {
    }
}

namespace puppet { namespace runtime {

    attribute::attribute(
        shared_ptr<compiler::context> context,
        string name,
        lexer::position name_position,
        shared_ptr<values::value> value,
        lexer::position value_position) :
            _context(rvalue_cast(context)),
            _name(rvalue_cast(name)),
            _name_position(rvalue_cast(name_position)),
            _value(rvalue_cast(value)),
            _value_position(rvalue_cast(value_position))
    {
    }

    shared_ptr<compiler::context> const& attribute::context() const
    {
        return _context;
    }

    string const& attribute::name() const
    {
        return _name;
    }

    lexer::position const& attribute::name_position() const
    {
        return _name_position;
    }

    shared_ptr<values::value> const& attribute::value() const
    {
        return _value;
    }

    lexer::position const& attribute::value_position() const
    {
        return _value_position;
    }

    ostream& operator<<(ostream& out, runtime::relationship relationship)
    {
        // These are labels for edges in a dependency graph.
        // Thus, they should always read as "A <string> B", where the string explains why A depends on B.
        switch (relationship) {
            case runtime::relationship::contains:
                out << "contains";
                break;

            case runtime::relationship::before:
                out << "after";
                break;

            case runtime::relationship::require:
                out << "requires";
                break;

            case runtime::relationship::notify:
                out << "notified by";
                break;

            case runtime::relationship::subscribe:
                out << "subscribes to";
                break;

            default:
                throw runtime_error("unexpected relationship.");
        }
        return out;
    }

    resource::resource(
        types::resource type,
        shared_ptr<compiler::context> context,
        lexer::position position,
        bool virtualized,
        bool exported) :
            _type(rvalue_cast(type)),
            _context(rvalue_cast(context)),
            _position(rvalue_cast(position)),
            _vertex_id(static_cast<size_t>(-1)),
            _virtualized(virtualized),
            _exported(exported)
    {
        if (!_context) {
            throw evaluation_exception("a compilation context is required.");
        }
        _path = _context->path();
    }

    types::resource const& resource::type() const
    {
        return _type;
    }

    shared_ptr<compiler::context> const& resource::context() const
    {
        return _context;
    }

    lexer::position const& resource::position() const
    {
        return _position;
    }

    string const& resource::path() const
    {
        return *_path;
    }

    bool resource::virtualized() const
    {
        return _virtualized;
    }

    bool resource::exported() const
    {
        return _exported;
    }

    void resource::set(shared_ptr<runtime::attribute> attribute)
    {
        if (!attribute) {
            return;
        }

        _attributes[attribute->name()] = rvalue_cast(attribute);
    }

    void resource::append(shared_ptr<runtime::attribute> attribute)
    {
        if (!attribute) {
            return;
        }

        auto it = _attributes.find(attribute->name());
        if (it == _attributes.end()) {
            // Not present, just set
            set(rvalue_cast(attribute));
            return;
        }

        // Ensure the existing value is an array
        auto existing = as<values::array>(*it->second->value());
        if (!existing) {
            throw evaluation_exception(
                (boost::format("attribute '%1%' cannot be appended to because it is not an array.") %
                 attribute->name()
                ).str(),
                attribute->context(),
                attribute->name_position());
        }

        // Copy the existing value
        values::array new_value = *existing;

        // Append the value to the array
        auto value = to_array(*attribute->value());
        new_value.insert(new_value.end(), std::make_move_iterator(value.begin()), std::make_move_iterator(value.end()));

        // Update the attribute's value and set it
        *attribute->value() = rvalue_cast(new_value);
        set(rvalue_cast(attribute));
    }

    shared_ptr<attribute> resource::get(string const& name) const
    {
        auto it = _attributes.find(name);
        if (it == _attributes.end()) {
            return nullptr;
        }
        return it->second;
    }

    void resource::each_attribute(std::function<bool(runtime::attribute const&)> const& callback) const
    {
        if (!callback) {
            return;
        }

        for (auto const& kvp : _attributes) {
            if (!callback(*kvp.second)) {
                break;
            }
        }
    }

    size_t resource::vertex_id() const
    {
        return _vertex_id;
    }

    Value resource::to_json(Allocator& allocator) const
    {
        Value value;
        value.SetObject();

        auto& type_name = _type.type_name();
        auto& title = _type.title();
        auto& path = _context->path();

        // Write out the type and title
        value.AddMember("type", StringRef(type_name.c_str(), type_name.size()), allocator);
        value.AddMember("title", StringRef(title.c_str(), title.size()), allocator);

        // Write out the tags
        // TODO: auto populate the resource's tags
        Value tags;
        tags.SetArray();
        value.AddMember("tags", tags, allocator);

        // Write out the file and line if not from "main"
        if (*path != "main") {
            value.AddMember("file", StringRef(path->c_str(), path->size()), allocator);
            value.AddMember("line", static_cast<uint64_t>(_position.line()), allocator);
        }

        // Write out whether or not the resource is exported
        value.AddMember("exported", _exported, allocator);

        // Write out the parameters
        Value parameters;
        parameters.SetObject();
        for (auto& attribute : _attributes) {
            auto const& name = attribute.first;
            auto const& value = *attribute.second->value();

            // Do not write any values set to undef
            if (is_undef(value)) {
                continue;
            }

            parameters.AddMember(
                StringRef(name.c_str(), name.size()),
                ::to_json(value, allocator),
                allocator);
        }
        value.AddMember("parameters", parameters, allocator);

        return value;
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

    void resource::vertex_id(size_t id)
    {
        _vertex_id = id;
    }

    class_definition::class_definition(types::klass klass, shared_ptr<compiler::context> context, ast::class_definition_expression const& expression) :
        _klass(rvalue_cast(klass)),
        _context(rvalue_cast(context)),
        _expression(expression)
    {
        if (!_context) {
            throw evaluation_exception("a class definition must have a compilation context.");
        }

        if (_expression.parent()) {
            _parent = types::klass(_expression.parent()->value());
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

    string const& class_definition::path() const
    {
        return *_context->path();
    }

    lexer::position const& class_definition::position() const
    {
        return _expression.position();
    }

    void class_definition::evaluate(runtime::context& context, runtime::resource& resource) const
    {
        // Create a scope for the class
        auto scope = make_shared<runtime::scope>(evaluate_parent(context), &resource);

        // Add the class' scope
        context.add_scope(scope);

        // Create a new expression evaluator based on the class' compilation context
        expression_evaluator evaluator{ _context, context };

        // Execute the body of the class
        runtime::executor executor{ evaluator, _expression.position(), _expression.parameters(), _expression.body() };
        executor.execute(resource, scope);
    }

    shared_ptr<runtime::scope> class_definition::evaluate_parent(runtime::context& context) const
    {
        // If no parent, return the node or top scope
        auto const& parent = this->parent();
        if (!parent) {
            return context.node_or_top();
        }

        // Declare the parent class
        context.catalog()->declare_class(context, types::resource("class", parent->title()), _context, _expression.parent()->position());
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

    string const& defined_type::path() const
    {
        return *_context->path();
    }

    lexer::position const& defined_type::position() const
    {
        return _expression.position();
    }

    void defined_type::evaluate(runtime::context& context, runtime::resource& resource) const
    {
        // Create a temporary scope for evaluating the defined type
        auto scope = make_shared<runtime::scope>(context.node_or_top(), &resource);

        // Create a new expression evaluator based on the defined type's compilation context
        expression_evaluator evaluator{ _context, context };

        // Execute the body of the defined type
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

    void node_definition::evaluate(runtime::context& context, runtime::resource& resource) const
    {
        // Set the node scope for the remainder of the evaluation
        node_scope scope{context, &resource};

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

    void catalog::add_relationship(runtime::relationship relationship, runtime::resource const& source, runtime::resource const& target)
    {
        auto source_ptr = &source;
        auto target_ptr = &target;

        // For before and notify, swap the target and source vertices so that the target depends on the source
        if (relationship == runtime::relationship::before || relationship == runtime::relationship::notify) {
            source_ptr = &target;
            target_ptr = &source;
        }

        // Add the edge to the graph if it doesn't already exist
        auto range = boost::edge_range(source_ptr->vertex_id(), target_ptr->vertex_id(), _graph);
        for (auto it = range.first; it != range.second; ++it) {
            if (_graph[*it] == relationship) {
                return;
            }
        }
        boost::add_edge(source_ptr->vertex_id(), target_ptr->vertex_id(), relationship, _graph);
    }

    resource* catalog::find_resource(types::resource const& type)
    {
        if (!type.fully_qualified()) {
            return nullptr;
        }

        // Find the resource type and title
        auto it = _resource_map.find(type);
        if (it == _resource_map.end()) {
            return nullptr;
        }
        return it->second;
    }

    runtime::resource& catalog::add_resource(
        types::resource type,
        shared_ptr<compiler::context> const& compilation_context,
        lexer::position const& position,
        resource const* container,
        bool virtualized,
        bool exported,
        defined_type const* definition)
    {
        if (!compilation_context) {
            throw evaluation_exception("expected a compilation context.");
        }
        if (!type.fully_qualified()) {
            throw evaluation_exception("resource name is not fully qualified.", compilation_context, position);
        }

        // Ensure the resource doesn't already exist
        if (auto previous = find_resource(type)) {
            throw evaluation_exception((boost::format("resource %1% was previously declared at %2%:%3%.") % type % previous->path() % previous->position().line()).str(), compilation_context, position);
        }

        // Add the resource
        _resources.emplace_back(
            rvalue_cast(type),
            compilation_context,
            position,
            virtualized,
            exported
        );

        // Map the type to the resource
        auto resource = &_resources.back();
        _resource_map[resource->type()] = resource;

        // Append to the type list
        _resource_lists[resource->type().type_name()].emplace_back(resource);

        // Add a graph vertex for the resource
        resource->vertex_id(boost::add_vertex(resource, _graph));

        // Stages should never be contained
        if (!type.is_stage() && container) {
            // Add a relationship to the container
            add_relationship(relationship::contains, *container, *resource);
        }

        // If a defined type, add it to the list of declared defined types
        if (definition) {
            _defined_types.emplace_back(make_pair(definition, resource));
        }
        return *resource;
    }

    vector<class_definition> const* catalog::find_class(types::klass const& klass, runtime::context* context)
    {
        auto it = _class_definitions.find(klass);
        if (it == _class_definitions.end() || it->second.empty()) {
            if (!context) {
                return nullptr;
            }
            context->node().load_manifest(*context, klass.title());
            it = _class_definitions.find(klass);
            if (it == _class_definitions.end() || it->second.empty()) {
                return nullptr;
            }
        }
        return &it->second;
    }

    void catalog::define_class(
        types::klass klass,
        shared_ptr<compiler::context> const& context,
        ast::class_definition_expression const& expression)
    {
        auto& definitions = _class_definitions[klass];
        definitions.push_back(class_definition(rvalue_cast(klass), context, expression));
    }

    resource& catalog::declare_class(
        runtime::context& evaluation_context,
        types::resource const& type,
        shared_ptr<compiler::context> const& compilation_context,
        lexer::position const& position)
    {
        if (!compilation_context) {
            throw evaluation_exception("expected a compilation context.");
        }
        if (!type.is_class()) {
            throw evaluation_exception("expected a class resource.", compilation_context, position);
        }
        if (!type.fully_qualified()) {
            throw evaluation_exception("cannot declare a class with an unspecified title.", compilation_context, position);
        }

        // Find the class definition
        auto definitions = find_class(types::klass(type.title()), &evaluation_context);
        if (!definitions) {
            throw evaluation_exception((boost::format("cannot evaluate class '%1%' because it has not been defined.") % type.title()).str(), compilation_context, position);
        }

        // Find the resource
        auto klass = find_resource(type);
        if (!klass) {
            // Create the class resource
            klass = &add_resource(type, compilation_context, position);
        }

        // If the class was already declared, return it without evaluating
        if (!_classes.insert(type.title()).second) {
            return *klass;
        }

        // Validate the stage metaparameter
        resource const* stage = nullptr;
        if (auto attribute = klass->get("stage")) {
            auto ptr = as<string>(*attribute->value());
            if (!ptr) {
                throw evaluation_exception(
                    (boost::format("expected %1% for 'stage' metaparameter but found %2%.") %
                     types::string::name() %
                     get_type(*attribute->value())
                    ).str(),
                    attribute->context(),
                    attribute->value_position());
            }
            stage = find_resource(types::resource("stage", *ptr));
            if (!stage) {
                throw evaluation_exception(
                    (boost::format("stage '%1%' does not exist in the catalog.") %
                     *ptr
                    ).str(),
                    attribute->context(),
                    attribute->value_position());
            }
        } else {
            stage = find_resource(types::resource("stage", "main"));
            if (!stage) {
                throw evaluation_exception("stage 'main' does not exist in the catalog.");
            }
        }

        // Set the stage as the container of the class
        add_relationship(relationship::contains, *stage, *klass);

        try {
            // Evaluate all definitions of the class
            for (auto& definition : *definitions) {
                definition.evaluate(evaluation_context, *klass);
            }
        } catch (evaluation_exception const& ex) {
            // If the original exception has context, log an error with full context first
            if (ex.context()) {
                ex.context()->log(logging::level::error, ex.position(), ex.what());
                throw evaluation_exception(
                    (boost::format("failed to evaluate class '%1%'.") %
                     type.title()
                    ).str(),
                    compilation_context,
                    position);
            }
            throw evaluation_exception(
                (boost::format("failed to evaluate class '%1%': %2%") %
                 type.title() %
                 ex.what()
                ).str(),
                compilation_context,
                position);
        }
        return *klass;
    }

    defined_type const* catalog::find_defined_type(string const& type, runtime::context* context)
    {
        auto it = _defined_type_definitions.find(type);
        if (it == _defined_type_definitions.end()) {
            if (!context) {
                return nullptr;
            }
            context->node().load_manifest(*context, type);
            it = _defined_type_definitions.find(type);
            if (it == _defined_type_definitions.end()) {
                return nullptr;
            }
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
        auto result = _defined_type_definitions.emplace(make_pair(rvalue_cast(type), rvalue_cast(defined)));
        if (!result.second) {
            auto const& existing = result.first->second;
            throw evaluation_exception(
                (boost::format("defined type '%1%' was previously defined at %2%:%3%.") %
                 existing.type() %
                 existing.path() %
                 existing.position().line()
                ).str(),
                context,
                expression.name().position());
        }
    }

    void catalog::define_node(shared_ptr<compiler::context> const& context, ast::node_definition_expression const& expression)
    {
        // Emplace the node
        _node_definitions.emplace_back(context, expression);
        size_t node_index = _node_definitions.size() - 1;

        for (auto const& name : expression.names()) {
            // Check for default node
            if (name.is_default()) {
                if (!_default_node_index) {
                    _default_node_index = node_index;
                    continue;
                }
                auto const& previous = _node_definitions[*_default_node_index];
                throw evaluation_exception((boost::format("a default node was previously defined at %1%:%2%.") % *previous.context()->path() % previous.position().line()).str(), context, name.position());
            }
            // Check for regular expression names
            if (name.regex()) {
                auto it = find_if(_regex_nodes.begin(), _regex_nodes.end(), [&](std::pair<values::regex, size_t> const& existing) { return existing.first.pattern() == name.value(); });
                if (it != _regex_nodes.end()) {
                    auto const& previous = _node_definitions[it->second];
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
                auto const& previous = _node_definitions[it->second];
                throw evaluation_exception((boost::format("node '%1%' was previously defined at %2%:%3%.") % name.value() % *previous.context()->path() % previous.position().line()).str(), context, name.position());
            }
            _named_nodes.emplace(make_pair(boost::to_lower_copy(name.value()), node_index));
        }
    }

    runtime::resource* catalog::declare_node(runtime::context& evaluation_context, compiler::node const& node)
    {
        // If there are no node definitions, do nothing
        if (_node_definitions.empty()) {
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
                definition = &_node_definitions[it->second];
                return false;
            }
            // Next, check by looking at every regex
            for (auto const& kvp : _regex_nodes) {
                if (regex_search(name, kvp.first.value())) {
                    node_name = "/" + kvp.first.pattern() + "/";
                    definition = &_node_definitions[kvp.second];
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
            definition = &_node_definitions[*_default_node_index];
        }

        // Add a Node resource to the catalog
        auto& context = definition->context();
        auto& position = definition->position();
        auto& resource = add_resource(types::resource("node", node_name), context, position, evaluation_context.current_scope()->resource());

        // Evaluate the node definition
        try {
            definition->evaluate(evaluation_context, resource);
        } catch (evaluation_exception const& ex) {
            // If the original exception has context, log an error with full context
            if (ex.context()) {
                ex.context()->log(logging::level::error, ex.position(), ex.what());
                throw evaluation_exception("failed to evaluate node.", context, position);
            }
            throw evaluation_exception((boost::format("failed to evaluate node: %1%.") % ex.what()).str(), context, position);
        }
        return &resource;
    }

    bool catalog::is_contained(runtime::resource const& resource, runtime::resource const& container) const
    {
        // Search through the out edges of the container
        auto iterators = boost::out_edges(container.vertex_id(), _graph);
        for (auto it = iterators.first; it != iterators.second; ++it) {
            // If the edge is not a containment edge, ignore
            auto relationship = _graph[*it];
            if (relationship != runtime::relationship::contains) {
                continue;
            }
            // If the target is the given resource, the resource is contained
            auto target = _graph[boost::target(*it, _graph)];
            if (&resource == target) {
                return true;
            }
        }
        return false;
    }

    void catalog::finalize(runtime::context& context)
    {
        vector<pair<defined_type const*, resource*>> virtualized;
        while (true) {
            // TODO: collect resources

            // We've collected, so check to see if all remaining defined types are virtual
            bool all_virtual = std::all_of(
                virtualized.begin(),
                virtualized.end(),
                [](pair<defined_type const*, resource*> const& element) {
                    return element.second->virtualized();
                });

            // If there are some realized defined types, move the entire collection over to evaluate
            if (!all_virtual) {
                _defined_types.insert(
                    _defined_types.begin(),
                    std::make_move_iterator(virtualized.begin()),
                    std::make_move_iterator(virtualized.end()));

                virtualized.clear();
            }

            // If there are no more defined types to evaluate, we're done
            if (_defined_types.empty()) {
                break;
            }

            // Evaluate the defined types
            evaluate_defined_types(context, virtualized);
        }

        // Populate the dependency graph
        populate_graph();
    }

    void catalog::write(compiler::node const& node, ostream& out) const
    {
        // Declare an adapter for RapidJSON's pretty writter
        struct stream_adapter
        {
            explicit stream_adapter(ostream& stream) : _stream(stream)
            {
            }

            void Put(char c)
            {
                _stream.put(c);
            }

            void Flush()
            {
            }

         private:
            ostream& _stream;
        } adapter(out);

        // Create the document and treat it as an object
        Document document;
        document.SetObject();

        auto& allocator = document.GetAllocator();

        // Write out the tags
        // TODO: populate top-level tags
        Value tags;
        tags.SetArray();
        document.AddMember("tags", tags, allocator);

        // Write out the catalog attributes
        auto const& name = node.name();
        document.AddMember("name", StringRef(name.c_str(), name.size()), allocator);
        document.AddMember("version", static_cast<int64_t>(std::time(nullptr)), allocator);
        auto const& environment = node.environment().name();
        document.AddMember("environment", StringRef(environment.c_str(), environment.size()), allocator);

        // Create an array to store the edges
        Value edges;
        edges.SetArray();

        // Write out the resources
        Value resources;
        resources.SetArray();
        resources.Reserve(_resources.size(),allocator);
        for (auto const& resource : _resources) {
            // Skip virtual resources
            if (resource.virtualized()) {
                continue;
            }

            // Add the resource
            resources.PushBack(resource.to_json(allocator), allocator);

            // Get the out edges from this resource
            auto iterators = boost::out_edges(resource.vertex_id(), _graph);
            for (auto it = iterators.first; it != iterators.second; ++it) {
                // If this is not a containment relationship, ignore
                // Only containment edges end up in a catalog (for now)
                auto relationship = _graph[*it];
                if (relationship != runtime::relationship::contains) {
                    continue;
                }
                auto target = _graph[boost::target(*it, _graph)];
                // Skip virtual resources
                if (target->virtualized()) {
                    continue;
                }

                // Create an edge object from source to target
                Value edge;
                edge.SetObject();
                edge.AddMember(
                    "source",
                    rapidjson::Value(boost::lexical_cast<string>(resource.type()).c_str(), allocator),
                    allocator);
                edge.AddMember(
                    "target",
                    rapidjson::Value(boost::lexical_cast<string>(target->type()).c_str(), allocator),
                    allocator);
                edges.PushBack(rvalue_cast(edge), allocator);
            }
        }
        document.AddMember("resources", resources, allocator);

        // Write out the containment edges
        document.AddMember("edges", edges, allocator);

        // Write out the declared classes
        Value classes;
        classes.SetArray();
        classes.Reserve(_classes.size(), allocator);
        for (auto const& klass : _classes) {
            classes.PushBack(StringRef(klass.c_str(), klass.size()), allocator);
        }
        document.AddMember("classes", classes, allocator);

        // Write the document to the stream
        PrettyWriter<stream_adapter> writer{adapter};
        writer.SetIndent(' ', 2);
        document.Accept(writer);

        // Flush the stream with one last newline
        out << endl;
    }

    void catalog::write_graph(ostream& out)
    {
        boost::write_graphviz(out, _graph, [&](ostream& out, size_t v) {
            auto resource = _graph[v];
            out << "[label=" << boost::escape_dot_string(boost::lexical_cast<string>(resource->type())) << "]";
        }, [&](ostream& out, boost::detail::edge_desc_impl<boost::directed_tag, unsigned long> const& e) {
            out << "[label=\"" << _graph[e] << "\"]";
        });
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
                cycle << resource->type() << " declared at " << resource->path() << ":" << resource->position().line();
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
        throw evaluation_exception(message.str());
    }

    void catalog::populate_graph()
    {
        string before_parameter = "before";
        string notify_parameter = "notify";
        string require_parameter = "require";
        string subscribe_parameter = "subscribe";

        // Loop through each resource and add metaparameter relationships
        for (auto const& resource : _resources) {
            // Proccess the relationship metaparameters for the resource
            process_relationship_parameter(resource, before_parameter, runtime::relationship::before);
            process_relationship_parameter(resource, notify_parameter, runtime::relationship::notify);
            process_relationship_parameter(resource, require_parameter, runtime::relationship::require);
            process_relationship_parameter(resource, subscribe_parameter, runtime::relationship::subscribe);
        }
    }

    void catalog::process_relationship_parameter(resource const& source, string const& name, runtime::relationship relationship)
    {
        auto attribute = source.get(name);
        if (!attribute) {
            return;
        }
        each_resource(*attribute->value(), [&](types::resource const& target_resource) {
            // Locate the target in the catalog
            auto target = find_resource(target_resource);
            if (!target) {
                throw evaluation_exception(
                    (boost::format("resource %1% (declared at %2%:%3%) cannot form a '%4%' relationship with resource %5%: the resource does not exist in the catalog.") %
                     source.type() %
                     source.path() %
                     source.position().line() %
                     name %
                     target_resource).str(),
                    attribute->context(),
                    attribute->value_position());
            }

            if (&source == target) {
                throw evaluation_exception(
                    (boost::format("resource %1% (declared at %2%:%3%) cannot form a '%4%' relationship with resource %5%: the relationship is self-referencing.") %
                     source.type() %
                     source.path() %
                     source.position().line() %
                     name %
                     target_resource).str(),
                    attribute->context(),
                    attribute->value_position());
            }

            // Add the relationship
            add_relationship(relationship, source, *target);
        }, [&](string const& message) {
            throw evaluation_exception(
                (boost::format("resource %1% (declared at %2%:%3%) cannot form a '%4%' relationship: %5%") %
                 source.type() %
                 source.path() %
                 source.position().line() %
                 name %
                 message).str(),
                attribute->context(),
                attribute->value_position());
        });
    }

    void catalog::evaluate_defined_types(runtime::context& context, vector<pair<defined_type const*, resource*>>& virtualized)
    {
        resource* current = nullptr;

        try {
            const size_t max_iterations = 1000;
            size_t iteration = 0;

            // Because evaluation of defined types may declare more defined types, loop to exhaust the queue
            while (!_defined_types.empty()) {
                // Loop from now until the *current* end
                auto size = _defined_types.size();
                for (size_t i = 0; i < size; ++i) {
                    auto& defined_type = _defined_types[i];
                    current = defined_type.second;

                    if (current->virtualized()) {
                        // Defined type is virtual, enqueue it for later evaluation
                        virtualized.emplace_back(rvalue_cast(defined_type));
                        continue;
                    }
                    defined_type.first->evaluate(context, *current);
                }

                // Erase everything that was just evaluated
                _defined_types.erase(_defined_types.begin(), _defined_types.begin() + size);

                // Guard against infinite recursion by limiting the number of outer loop iterations
                if (iteration++ >= max_iterations) {
                    current = nullptr;
                    throw evaluation_exception("maximum defined type evaluations exceeded: a defined type may be infinitely recursive.");
                }
            }
        } catch (evaluation_exception const& ex) {
            if (!current) {
                throw;
            }
            // If the original exception has context, log an error with full context first
            if (ex.context()) {
                ex.context()->log(logging::level::error, ex.position(), ex.what());
                throw evaluation_exception(
                    (boost::format("failed to evaluate defined type '%1%'.") %
                     current->type()
                    ).str(),
                    current->context(),
                    current->position());
            }
            throw evaluation_exception(
                (boost::format("failed to evaluate defined type '%1%': %2%") %
                 current->type() %
                 ex.what()
                ).str(),
                current->context(),
                current->position());
        }
    }

}}  // namespace puppet::runtime
