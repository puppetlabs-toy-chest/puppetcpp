#include <puppet/runtime/catalog.hpp>
#include <puppet/runtime/context.hpp>
#include <puppet/runtime/executor.hpp>
#include <puppet/runtime/expression_evaluator.hpp>
#include <puppet/compiler/node.hpp>
#include <puppet/cast.hpp>
#include <boost/graph/tiernan_all_cycles.hpp>
#include <boost/format.hpp>
#include <unordered_set>

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
            throw runtime_error("a declared resource must have a path.");
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

    bool class_definition::evaluate(
        runtime::context& context,
        runtime::resource const& resource,
        function<evaluation_exception(bool, string const&, string)> const& create_exception)
    {
        if (evaluated()) {
            return true;
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

        // Add the class' scope
        context.add_scope(scope);

        try {
            // Create a new expression evaluator based on the class' compilation context
            expression_evaluator evaluator{ _context, context };

            // Execute the body of the class
            runtime::executor executor{ evaluator, _expression->position(), _expression->parameters(), _expression->body() };
            executor.execute(resource, create_exception, scope);
        } catch (evaluation_exception const& ex) {
            // Log any evaluation exception encountered
            ex.context()->log(logging::level::error, ex.position(), ex.what());
            return false;
        }

        // Reset the context; classes cannot be evaluated more than once
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
        if (!_catalog.declare_class(context, *parent, _context->path(), _expression->parent()->position().line())) {
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

    bool defined_type::evaluate(
        runtime::context& context,
        runtime::resource const& resource,
        function<evaluation_exception(bool, string const&, string)> const& create_exception)
    {
        // Create a temporary scope for evaluating the defined type
        ostringstream display_name;
        display_name << resource.type();
        auto scope = make_shared<runtime::scope>(context.node_or_top(), _type, display_name.str());

        try {
            // Create a new expression evaluator based on the defined type's compilation context
            expression_evaluator evaluator{ _context, context };

            // Execute hte body of the defined type
            runtime::executor executor{ evaluator, _expression.position(), _expression.parameters(), _expression.body() };
            executor.execute(resource, create_exception, scope);
        } catch (evaluation_exception const& ex) {
            // Log any evaluation exception encountered
            ex.context()->log(logging::level::error, ex.position(), ex.what());
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
            // Log any evaluation exception encountered
            ex.context()->log(logging::level::error, ex.position(), ex.what());
            return false;
        }
        return true;
    }

    catalog::catalog() :
        _default_node_index(-1)
    {
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

    runtime::resource* catalog::add_resource(types::resource type, shared_ptr<string> path, size_t line, shared_ptr<runtime::attributes> attributes, bool exported)
    {
        if (!type.fully_qualified()) {
            return nullptr;
        }

        string resource_type = type.type_name();
        string title = type.title();

        auto& resources = _resources[rvalue_cast(resource_type)];

        auto result = resources.emplace(make_pair(rvalue_cast(title), runtime::resource(*this, rvalue_cast(type), rvalue_cast(path), line, rvalue_cast(attributes), exported)));
        if (!result.second) {
            return nullptr;
        }

        // Add a graph vertex for the resource
        auto resource_ptr = &result.first->second;
        resource_ptr->vertex_id(boost::add_vertex(resource_ptr, _graph));

        return resource_ptr;
    }

    void catalog::define_class(types::klass klass, shared_ptr<compiler::context> context, ast::class_definition_expression const& expression)
    {
        // Validate the class parameters
        if (expression.parameters()) {
            validate_parameters(true, context, *expression.parameters());
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
                throw evaluation_exception(context, expression.parent()->position(),
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

    runtime::resource* catalog::declare_class(
        runtime::context& context,
        types::klass const& klass,
        shared_ptr<string> path,
        size_t line,
        shared_ptr<runtime::attributes> attributes,
        function<evaluation_exception(bool, string const&, string)> const& create_exception)
    {
        if (!klass.fully_qualified()) {
            return nullptr;
        }

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
        resource = add_resource(rvalue_cast(resource_type), rvalue_cast(path), line, attributes);
        if (!resource) {
            return nullptr;
        }

        // Evaluate all definitions of the class
        for (auto& definition : it->second) {
            if (!definition.evaluate(context, *resource, create_exception)) {
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
            validate_parameters(false, context, *expression.parameters());
        }

        // Look for the existing type
        auto it = _defined_types.find(type);
        if (it != _defined_types.end()) {
            throw evaluation_exception(context, expression.name().position(),
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

    runtime::resource* catalog::declare_defined_type(
        runtime::context& context,
        string const& type,
        string const& title,
        shared_ptr<string> path,
        size_t line,
        shared_ptr<runtime::attributes> attributes,
        function<evaluation_exception(bool, string const&, string)> const& create_exception)
    {
        // Lookup the defined type
        auto it = _defined_types.find(type);
        if (it == _defined_types.end()) {
            // TODO: search node for defined type
            return nullptr;
        }

        // Add a resource to the catalog
        auto added = add_resource(types::resource(type, title), rvalue_cast(path), line, attributes);
        if (!added) {
            return nullptr;
        }

        // Evaluate the defined type
        if (!it->second.evaluate(context, *added, create_exception)) {
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
                throw evaluation_exception(context, name.position(), (boost::format("a default node was previously defined at %1%:%2%.") % previous.path() % previous.line()).str());
            }
            // Check for regular expression names
            if (name.regex()) {
                auto it = find_if(_regex_node_definitions.begin(), _regex_node_definitions.end(), [&](decltype(_regex_node_definitions.front()) existing) { return existing.first.pattern() == name.value(); });
                if (it != _regex_node_definitions.end()) {
                    auto const& previous = _nodes[it->second];
                    throw evaluation_exception(context, name.position(), (boost::format("node /%1%/ was previously defined at %2%:%3%.") % name.value() % previous.path() % previous.line()).str());
                }

                try {
                    _regex_node_definitions.emplace_back(values::regex(name.value()), node_index);
                } catch (regex_error const& ex) {
                    throw evaluation_exception(context, name.position(), (boost::format("invalid regular expression: %1%") % ex.what()).str());
                }
                continue;
            }
            // Otherwise, this is a qualified node name
            auto it = _named_nodes.find(name.value());
            if (it != _named_nodes.end()) {
                auto const& previous = _nodes[it->second];
                throw evaluation_exception(context, name.position(), (boost::format("node '%1%' was previously defined at %2%:%3%.") % name.value() % previous.path() % previous.line()).str());
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

    void catalog::finalize()
    {
        // Populate the dependency graph
        populate_graph();
    }

    void catalog::validate_parameters(bool klass, shared_ptr<compiler::context> const& context, vector<ast::parameter> const& parameters)
    {
        for (auto const& parameter : parameters) {
            auto const& name = parameter.variable().name();

            // Check for reserved names
            if (name == "title" || name == "name") {
                throw evaluation_exception(context, parameter.variable().position(), (boost::format("parameter $%1% is reserved and cannot be used.") % name).str());
            }

            // Check for capture parameters
            if (parameter.captures()) {
                throw evaluation_exception(context, parameter.variable().position(), (boost::format("%1% parameter $%2% cannot \"captures rest\".") % (klass ? "class" : "defined type") % name).str());
            }

            // Check for metaparameter names
            if (resource::is_metaparameter(name)) {
                throw evaluation_exception(context, parameter.variable().position(), (boost::format("parameter $%1% is reserved for resource metaparameter '%1%'.") % name).str());
            }
        }
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
                throw compiler::compilation_exception(
                    (boost::format("resource %1% (declared at %2%:%3%) cannot form a '%4%' relationship with resource %5%: the resource does not exist in the catalog.") %
                     source.type() %
                     *source.path() %
                     source.line() %
                     name %
                     target_resource).str());
            }

            if (&source == target) {
                throw compiler::compilation_exception(
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
            throw compiler::compilation_exception(
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
