#include <puppet/compiler/evaluation/scope.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation {

    assignment_context::assignment_context(ast::context const* context) :
        _line(0)
    {
        if (context) {
            if (context->tree) {
                _path = context->tree->shared_path();
            }
            _line = context->begin.line();
        }
    }

    shared_ptr<string> const& assignment_context::path() const
    {
        return _path;
    }

    size_t assignment_context::line() const
    {
        return _line;
    }

    scope::scope(shared_ptr<scope> parent, compiler::resource* resource) :
        _parent(rvalue_cast(parent)),
        _resource(resource)
    {
        if (!_parent) {
            throw runtime_error("expected a parent scope.");
        }
    }

    scope::scope(shared_ptr<facts::provider> facts) :
        _facts(rvalue_cast(facts)),
        _resource(nullptr)
    {
    }

    shared_ptr<scope> const& scope::parent() const
    {
        return _parent;
    }

    compiler::resource* scope::resource()
    {
        return const_cast<compiler::resource*>(static_cast<scope const*>(this)->resource());
    }

    compiler::resource const* scope::resource() const
    {
        if (_resource) {
            return _resource;
        }
        return _parent ? _parent->resource() : nullptr;
    }

    void scope::resource(compiler::resource* resource)
    {
        _resource = resource;
    }

    string scope::qualify(string const& name) const
    {
        if (boost::starts_with(name, "::")) {
            return name.substr(2);
        }

        if (!_resource) {
            return name;
        }

        return _resource->type().title() + "::" + name;
    }

    assignment_context const* scope::set(string name, shared_ptr<values::value const> value, ast::context const& context)
    {
        static assignment_context no_context(nullptr);

        // Check to see if the variable already exists
        auto it = _variables.find(name);
        if (it != _variables.end()) {
            return &it->second.second;
        }

        // If there's a fact provider, try get a fact of the given name before setting
        if (_facts) {
            auto previous = get(name);
            if (previous) {
                return &no_context;
            }
        }
        _variables.emplace(rvalue_cast(name), make_pair(rvalue_cast(value), assignment_context(&context)));
        return nullptr;
    }

    shared_ptr<values::value const> scope::get(string const& name)
    {
        auto it = _variables.find(name);
        if (it != _variables.end()) {
            return it->second.first;
        }

        // Go up the parent if there is one
        if (_parent) {
            return _parent->get(name);
        }

        // Lookup the fact if there's a fact provider
        if (!_facts) {
            return nullptr;
        }
        return _facts->lookup(name);
    }

    void scope::add_defaults(types::resource const& type, compiler::attributes attributes)
    {
        auto it = _defaults.find(type.type_name());
        if (it != _defaults.end()) {
            // The defaults already exist, so ensure there are no conflicts at this scope
            for (auto& attribute : attributes) {
                auto previous = find_if(it->second.begin(), it->second.end(), [&](auto const& previous) { return previous.second->name() == attribute.second->name(); });
                if (previous == it->second.end()) {
                    continue;
                }
                auto& name = previous->second->name();
                auto& current_context = attribute.second->name_context();
                auto& previous_context = previous->second->name_context();
                if (!previous_context.tree) {
                    throw evaluation_exception(
                        (boost::format("a default for '%1%' was already defined in a parent scope.") %
                         name
                        ).str(),
                        current_context
                    );
                }
                throw evaluation_exception(
                    (boost::format("a default for '%1%' was already defined at %2%:%3%.") %
                     name %
                     previous_context.tree->path() %
                     previous_context.begin.line()
                    ).str(),
                    current_context
                );
            }
            it->second.insert(it->second.end(), std::make_move_iterator(attributes.begin()), std::make_move_iterator(attributes.end()));
        } else {
            _defaults.emplace(type.type_name(), rvalue_cast(attributes));
        }
    }

    shared_ptr<attribute> scope::find_default(types::resource const& type, string const& name) const
    {
        auto it = _defaults.find(type.type_name());
        if (it != _defaults.end()) {
            auto attribute = find_if(it->second.begin(), it->second.end(), [&](auto const& attribute) { return attribute.second->name() == name; });
            if (attribute != it->second.end()) {
                return attribute->second;
            }
        }
        return _parent ? _parent->find_default(type, name) : nullptr;
    }

    void scope::each_default(types::resource const& type, attribute_set& set, function<bool(attribute const&)> const& callback) const
    {
        auto it = _defaults.find(type.type_name());
        if (it != _defaults.end()) {
            for (auto& attribute : it->second) {
                if (!set.insert(attribute.second.get()).second) {
                    continue;
                }
                callback(*attribute.second);
            }
        }

        if (_parent) {
            _parent->each_default(type, set, callback);
        }
    }

    ostream& operator<<(ostream& os, scope const& s)
    {
        if (!s.resource()) {
            os << "Scope(unknown)";
            return os;
        }
        os << "Scope(" << s.resource()->type() << ")";
        return os;
    }

}}}  // namespace puppet::compiler::evaluation
