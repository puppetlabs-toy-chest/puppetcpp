#include <puppet/compiler/catalog.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <puppet/cast.hpp>
#include <boost/graph/hawick_circuits.hpp>
#include <boost/graph/graphviz.hpp>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

using namespace std;
using namespace puppet::runtime;
using namespace puppet::runtime::values;

namespace puppet { namespace compiler {

    ostream& operator<<(ostream& out, relationship relation)
    {
        // These are labels for edges in a dependency graph.
        // Thus, they should always read as "A <string> B", where the string explains why A depends on B.
        switch (relation) {
            case relationship::contains:
                out << "contains";
                break;

            case relationship::before:
                out << "after";
                break;

            case relationship::require:
                out << "requires";
                break;

            case relationship::notify:
                out << "notified by";
                break;

            case relationship::subscribe:
                out << "subscribes to";
                break;

            default:
                throw runtime_error("unexpected relationship.");
        }
        return out;
    }

    resource_cycle_exception::resource_cycle_exception(std::string const& message) :
        runtime_error(message)
    {
    }

    catalog::catalog(string node, string environment) :
        _node(rvalue_cast(node)),
        _environment(rvalue_cast(environment))
    {
    }

    string const& catalog::node() const
    {
        return _node;
    }

    string const& catalog::environment() const
    {
        return _environment;
    }

    resource* catalog::add(types::resource type, resource const* container, shared_ptr<evaluation::scope> scope, boost::optional<ast::context> context, bool virtualized, bool exported)
    {
        // Ensure the resource name is fully qualified
        if (!type.fully_qualified()) {
            throw runtime_error("resource name is not fully qualified.");
        }

        // Check for stages with a container
        if (container && type.is_stage()) {
            throw runtime_error("stages cannot have a container.");
        }

        // Attempt to find an existing resource
        if (find(type)) {
            return nullptr;
        }

        _resources.emplace_back(resource(rvalue_cast(type), container, rvalue_cast(scope), rvalue_cast(context), exported));

        auto resource = &_resources.back();

        // Map the type to the resource
        _resource_map[resource->type()] = resource;

        // Append to the type list
        _resource_lists[resource->type().type_name()].emplace_back(resource);

        // Realize the resource if not virtual
        if (!virtualized) {
            realize(*resource);
        }
        return resource;
    }

    resource* catalog::find(runtime::types::resource const& type)
    {
        // Call the const overload, but cast away the const return
        // catalog::find should never mutate the catalog's state, but this overload should allow the resource to be modified
        return const_cast<resource*>(static_cast<catalog const*>(this)->find(type));
    }

    resource const* catalog::find(runtime::types::resource const& type) const
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

    size_t catalog::size() const
    {
        return _resources.size();
    }

    void catalog::each(function<bool(resource&)> const& callback, string const& type, size_t offset)
    {
        // Adapt the given function so that we cast away const-ness of the resource
        auto adapted = [&](resource const& r) {
            return callback(const_cast<resource&>(r));
        };
        // Enumerate the resources using the const overload
        return static_cast<catalog const*>(this)->each(adapted, type);
    }

    void catalog::each(function<bool(resource const&)> const& callback, std::string const& type, size_t offset) const
    {
        // If no type given, enumerate all resources
        if (type.empty()) {
            for (size_t i = offset; i < _resources.size(); ++i) {
                if (!callback(_resources[i])) {
                    break;
                }
            }
            return;
        }

        // A type was given, enumerate resources of that type only
        auto it = _resource_lists.find(type);
        if (it == _resource_lists.end()) {
            return;
        }

        for (size_t i = offset; i < it->second.size(); ++i) {
            if (!callback(*it->second[i])) {
                break;
            }
        }
    }

    void catalog::each_edge(compiler::resource const& resource, function<bool(relationship, compiler::resource const&)> const& callback) const
    {
        if (resource.virtualized()) {
            return;
        }

        // Get the out edges from this resource
        for (auto const& edge : make_iterator_range(boost::out_edges(resource.vertex_id(), _graph))) {
            if (!callback(_graph[edge], *_graph[boost::target(edge, _graph)])) {
                break;
            }
        }
    }

    void catalog::relate(relationship relation, resource const& source, resource const& target)
    {
        if (source.virtualized()) {
            throw runtime_error("source cannot be a virtual resource.");
        }
        if (target.virtualized()) {
            throw runtime_error("target cannot be a virtual resource.");
        }

        auto source_ptr = &source;
        auto target_ptr = &target;

        // For before and notify, swap the target and source vertices so that the target depends on the source
        if (relation == relationship::before || relation == relationship::notify) {
            source_ptr = &target;
            target_ptr = &source;
        }

        // Add the edge to the graph if it doesn't already exist
        for (auto const& edge : make_iterator_range(boost::out_edges(source_ptr->vertex_id(), _graph))) {
            if (_graph[boost::target(edge, _graph)] == target_ptr && _graph[edge] == relation) {
                return;
            }
        }
        boost::add_edge(source_ptr->vertex_id(), target_ptr->vertex_id(), relation, _graph);
    }

    void catalog::realize(compiler::resource& resource)
    {
        if (!resource.virtualized()) {
            return;
        }

        // Realize the resource
        resource.realize(boost::add_vertex(&resource, _graph));

        // Add a relationship from container to this resource
        if (resource.container()) {
            // Add a relationship to the container
            relate(relationship::contains, *resource.container(), resource);
        }
    }

    void catalog::populate_graph()
    {
        string const before_parameter    = "before";
        string const notify_parameter    = "notify";
        string const require_parameter   = "require";
        string const subscribe_parameter = "subscribe";

        // Loop through each resource and add metaparameter relationships
        for (auto const& resource : _resources) {
            // Skip any virtual resources
            if (resource.virtualized()) {
                continue;
            }

            // Add the relationships from the metaparameters
            populate_relationships(resource, before_parameter, relationship::before);
            populate_relationships(resource, notify_parameter, relationship::notify);
            populate_relationships(resource, require_parameter, relationship::require);
            populate_relationships(resource, subscribe_parameter, relationship::subscribe);
        }
    }

    void catalog::write(ostream& out) const
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
        json_document document;
        document.SetObject();

        auto& allocator = document.GetAllocator();

        // Create an array to store the classes
        json_value classes;
        classes.SetArray();

        // Create an array to store the resources
        json_value resources;
        resources.SetArray();
        resources.Reserve(_resources.size(),allocator);

        // Create an array to store the dependency edges
        json_value edges;
        edges.SetArray();

        for (auto const& resource : _resources) {
            // Skip virtual resources
            if (resource.virtualized()) {
                continue;
            }

            if (resource.type().is_class()) {
                auto& title = resource.type().title();
                classes.PushBack(rapidjson::StringRef(title.c_str(), title.size()), allocator);
            }

            // Add the resource
            resources.PushBack(resource.to_json(allocator, *this), allocator);

            // Add the edges for this resource
            each_edge(resource, [&](relationship relation, compiler::resource const& target) {
                if (relation != relationship::contains) {
                    // The top level edges are only containment edges
                    return true;
                }
                // Create an edge object from source to target
                json_value edge;
                edge.SetObject();
                edge.AddMember(
                    "source",
                    json_value(boost::lexical_cast<string>(resource.type()).c_str(), allocator),
                    allocator);
                edge.AddMember(
                    "target",
                    json_value(boost::lexical_cast<string>(target.type()).c_str(), allocator),
                    allocator);
                edges.PushBack(rvalue_cast(edge), allocator);
                return true;
            });
        }

        // Write out the catalog attributes
        document.AddMember("name", rapidjson::StringRef(_node.c_str(), _node.size()), allocator);
        document.AddMember("version", static_cast<int64_t>(std::time(nullptr)), allocator);
        document.AddMember("environment", rapidjson::StringRef(_environment.c_str(), _environment.size()), allocator);

        // Write out the resources
        document.AddMember("resources", rvalue_cast(resources), allocator);

        // Write out the containment edges
        document.AddMember("edges", rvalue_cast(edges), allocator);

        // Write out the declared classes
        document.AddMember("classes", rvalue_cast(classes), allocator);

        // Write the document to the stream
        rapidjson::PrettyWriter<stream_adapter> writer{adapter};
        writer.SetIndent(' ', 2);
        document.Accept(writer);

        // Flush the stream with one last newline
        out << endl;
    }

    void catalog::write_graph(ostream& out)
    {
        out << "digraph resources {\n";

        // Output the vertices
        for (auto const& vertex : boost::make_iterator_range(boost::vertices(_graph))) {
            auto resource = _graph[vertex];
            if (resource->virtualized()) {
                // Don't write out a vertex unless the resource was realized
                continue;
            }
            out << "  " << vertex << " [label=" << boost::escape_dot_string(boost::lexical_cast<string>(resource->type())) << "];\n";
        }
        // Output the edges
        for (auto const& edge : boost::make_iterator_range(boost::edges(_graph))) {
            auto source = boost::source(edge, _graph);
            auto target = boost::target(edge, _graph);
            if (_graph[source]->virtualized() || _graph[target]->virtualized()) {
                // Don't write out vertices unless source and target are both realized
                continue;
            }
            out << "  " << source << " -> " << target << " [label=\"" << _graph[edge] << "\"];\n";
        }
        out << "}\n";
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
                cycle << resource->type() << " declared at " << resource->path() << ":" << resource->line();
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
        boost::hawick_unique_circuits(_graph, cycle_visitor(cycles));
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
        throw resource_cycle_exception(message.str());
    }

    void catalog::populate_relationships(resource const& source, string const& name, compiler::relationship relationship)
    {
        auto attribute = source.get(name);
        if (!attribute) {
            return;
        }
        attribute->value().each_resource([&](types::resource const& target_resource) {
            // Locate the target in the catalog
            auto target = find(target_resource);
            if (!target) {
                throw evaluation_exception(
                    (boost::format("resource %1% (declared at %2%:%3%) cannot form a '%4%' relationship with resource %5%: the resource does not exist in the catalog.") %
                     source.type() %
                     source.path() %
                     source.line() %
                     name %
                     target_resource
                    ).str(),
                    attribute->value_context(),
                    {}
                );
            }

            if (&source == target) {
                throw evaluation_exception(
                    (boost::format("resource %1% cannot form a relationship with itself.") %
                     source.type()
                    ).str(),
                    attribute->value_context(),
                    {}
                );
            }

            // Add the relationship
            relate(relationship, source, *target);
        }, [&](string const& message) {
            throw evaluation_exception(
                (boost::format("resource %1% (declared at %2%:%3%) cannot form a '%4%' relationship: %5%") %
                 source.type() %
                 source.path() %
                 source.line() %
                 name %
                 message
                ).str(),
                attribute->value_context(),
                {}
            );
        });
    }

}}  // namespace puppet::compiler
