#include <puppet/runtime/catalog.hpp>
#include <puppet/runtime/expression_evaluator.hpp>
#include <puppet/runtime/executor.hpp>
#include <puppet/compiler/node.hpp>
#include <puppet/ast/expression_def.hpp>
#include <boost/graph/tiernan_all_cycles.hpp>
#include <boost/graph/graphviz.hpp>
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
        bool exported) :
            _type(rvalue_cast(type)),
            _context(rvalue_cast(context)),
            _position(rvalue_cast(position)),
            _vertex_id(static_cast<size_t>(-1)),
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
        if (!boost::edge(source_ptr->vertex_id(), target_ptr->vertex_id(), _graph).second) {
            boost::add_edge(source_ptr->vertex_id(), target_ptr->vertex_id(), relationship, _graph);
        }
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

    runtime::resource& catalog::add_resource(
        types::resource type,
        shared_ptr<compiler::context> const& compilation_context,
        lexer::position const& position,
        resource const* container,
        bool exported)
    {
        if (!compilation_context) {
            throw evaluation_exception("expected a compilation context.");
        }
        if (!type.fully_qualified()) {
            throw evaluation_exception("resource name is not fully qualified.", compilation_context, position);
        }

        bool is_class = type.is_class();
        bool is_stage = type.is_stage();

        string title = type.title();

        auto& resources = _resources[type.type_name()];
        auto result = resources.emplace(make_pair(rvalue_cast(title), runtime::resource(rvalue_cast(type), compilation_context, position, exported)));
        auto& resource = result.first->second;
        if (!result.second) {
            throw evaluation_exception((boost::format("resource %1% was previously declared at %2%:%3%.") % resource.type() % resource.path() % resource.position().line()).str(), compilation_context, position);
        }

        // Add a graph vertex for the resource
        resource.vertex_id(boost::add_vertex(&resource, _graph));

        if (!is_stage && !is_class) {
            // If given a container, add a containment relationship
            if (container) {
                add_relationship(relationship::contains, *container, resource);
            }

            // If a defined type, add it to the list of declared defined types
            if (auto definition = find_defined_type(boost::to_lower_copy(resource.type().type_name()))) {
                _declared_defined_types.emplace_back(make_pair(definition, &resource));
            }
        }
        return resource;
    }

    vector<class_definition> const* catalog::find_class(types::klass const& klass, compiler::node const* node)
    {
        auto it = _classes.find(klass);
        if (it == _classes.end() || it->second.empty()) {
            if (node) {
                // TODO: load class
            }
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

        // TODO: check if the class has already been evaluated

        // Find the class definition
        auto definitions = find_class(types::klass(type.title()), &compilation_context->node());
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
        if (!_declared_classes.insert(type.title()).second) {
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

        // Add the relationship to the stage
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

    void catalog::finalize(runtime::context& context)
    {
        // Evaluate defined types
        evaluate_defined_types(context);

        // Populate the dependency graph
        populate_graph();
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
        for (auto const& types_pair : _resources) {
            for (auto const& resource_pair : types_pair.second) {
                auto const& source = resource_pair.second;

                // Proccess the relationship metaparameters for the resource
                process_relationship_parameter(source, before_parameter, runtime::relationship::before);
                process_relationship_parameter(source, notify_parameter, runtime::relationship::notify);
                process_relationship_parameter(source, require_parameter, runtime::relationship::require);
                process_relationship_parameter(source, subscribe_parameter, runtime::relationship::subscribe);
            }
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

    void catalog::evaluate_defined_types(runtime::context& context)
    {
        resource* current = nullptr;

        try {
            // Evaluate all the declared defined types in order
            for (auto& kvp : _declared_defined_types) {
                current = kvp.second;
                kvp.first->evaluate(context, *current);
            }
        } catch (evaluation_exception const& ex) {
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

        _declared_defined_types.clear();
    }

}}  // namespace puppet::runtime
