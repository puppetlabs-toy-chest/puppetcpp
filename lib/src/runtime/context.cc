#include <puppet/runtime/context.hpp>
#include <puppet/cast.hpp>

using namespace std;

namespace puppet { namespace runtime {

    class_definition::class_definition(shared_ptr<ast::syntax_tree> tree, ast::class_definition_expression const* expression) :
        _tree(rvalue_cast(tree)),
        _expression(expression),
        _line(expression->position().line())
    {
    }

    bool class_definition::evaluated() const
    {
        return _expression == nullptr;
    }

    ast::syntax_tree const* class_definition::tree() const
    {
        return _tree.get();
    }

    ast::class_definition_expression const* class_definition::expression() const
    {
        return _expression;
    }

    string const& class_definition::path() const
    {
        return _tree ? _tree->path() : _path;
    }

    size_t class_definition::line() const
    {
        return _line;
    }

    void class_definition::release()
    {
        // Copy the path for later use
        _path = _tree->path();
        _expression = nullptr;
        _tree.reset();
    }

    context::context(logging::logger& logger, compiler::node& node, runtime::catalog& catalog, function<void(lexer::position const&, string const&)> const& warning) :
        _logger(logger),
        _node(node),
        _catalog(catalog),
        _warn(warning)
    {
        // Add the top scope
        push_scope(*add_scope(runtime::scope("", "Class[main]")));
    }

    logging::logger& context::logger()
    {
        return _logger;
    }

    logging::logger const& context::logger() const
    {
        return _logger;
    }

    compiler::node& context::node()
    {
        return _node;
    }

    compiler::node const& context::node() const
    {
        return _node;
    }

    runtime::catalog& context::catalog()
    {
        return _catalog;
    }

    runtime::catalog const& context::catalog() const
    {
        return _catalog;
    }

    runtime::scope& context::scope()
    {
        return *_scope_stack.back();
    }

    runtime::scope const& context::scope() const
    {
        return *_scope_stack.back();
    }

    runtime::scope& context::top()
    {
        return *_scope_stack.front();
    }

    runtime::scope const& context::top() const
    {
        return *_scope_stack.front();
    }

    runtime::scope* context::add_scope(runtime::scope scope)
    {
        return &_scopes.emplace(make_pair(scope.name(), rvalue_cast(scope))).first->second;
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

    values::value const* context::lookup(std::string const& name, lexer::position const* position)
    {
        // Look for the last :: delimiter
        // If not found, use the current scope
        auto pos = name.rfind("::");
        if (pos == string::npos) {
            return this->scope().get(name);
        }

        // Split into namespace and variable name
        auto ns = name.substr(0, pos);
        auto var = name.substr(pos + 2);

        // An empty namespace is the top scope
        if (ns.empty()) {
            return top().get(var);
        }

        // Lookup the namespace
        auto scope = find_scope(ns);
        if (scope) {
            return scope->get(var);
        }

        // Warn if the scope was not found
        if (position) {
            auto definition = find_class(ns);
            if (!definition) {
                warn(*position, (boost::format("could not look up variable $%1% because class '%2%' is not defined.") % name % ns).str());
            } else if (!definition->evaluated()) {
                warn(*position, (boost::format("could not look up variable $%1% because class '%2%' has not been declared.") % name % ns).str());
            }
        }
        return nullptr;
    }

    void context::warn(lexer::position const& position, string const& message) const
    {
        if (!_warn) {
            return;
        }
        _warn(position, message);
    }

    class_definition* context::define_class(shared_ptr<ast::syntax_tree> tree, ast::class_definition_expression const& expression)
    {
        // Add a new resource
        auto result = _classes.emplace(make_pair(expression.name().value(), class_definition(tree, &expression)));
        if (!result.second) {
            return nullptr;
        }
        return &result.first->second;
    }

    class_definition* context::find_class(string const& name)
    {
        auto klass = _classes.find(name);
        if (klass == _classes.end()) {
            return nullptr;
        }
        return &klass->second;
    }

    class_definition const* context::find_class(string const& name) const
    {
        auto klass = _classes.find(name);
        if (klass == _classes.end()) {
            return nullptr;
        }
        return &klass->second;
    }

    ephemeral_scope::ephemeral_scope(runtime::context& context) :
        _context(context),
        _scope(&context.scope())
    {
        _context.push_scope(_scope);
    }

    ephemeral_scope::~ephemeral_scope()
    {
        _context.pop_scope();
    }

}}  // namespace puppet::runtime
