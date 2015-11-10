#include <puppet/compiler/evaluation/scope.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler { namespace evaluation {

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

    ast::context const* scope::set(string name, shared_ptr<values::value const> value, ast::context const* context)
    {
        static ast::context no_context{ nullptr, lexer::position(0, 0) };

        // Check to see if the variable already exists
        auto it = _variables.find(name);
        if (it != _variables.end()) {
            return it->second.second ? it->second.second : &no_context;
        }

        // If there's a fact provider, try get a fact of the given name before setting
        if (_facts) {
            auto previous = get(name);
            if (previous) {
                return &no_context;
            }
        }
        _variables.emplace(rvalue_cast(name), make_pair(rvalue_cast(value), context));
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
        auto value = _facts->lookup(name);
        if (!value) {
            return nullptr;
        }

        // Add the fact as an assigned variable
        return _variables.emplace(make_pair(name, make_pair(rvalue_cast(value), nullptr))).first->second.first;
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
