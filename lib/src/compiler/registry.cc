#include <puppet/compiler/registry.hpp>
#include <puppet/compiler/scanner.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <puppet/compiler/evaluation/context.hpp>
#include <puppet/compiler/evaluation/call_evaluator.hpp>
#include <puppet/compiler/node.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::runtime;
using namespace puppet::compiler::evaluation;

namespace puppet { namespace compiler {

    klass::klass(string name, ast::class_expression const& expression) :
        _name(rvalue_cast(name)),
        _tree(expression.context.tree->shared_from_this()),
        _expression(expression)
    {
    }

    string const& klass::name() const
    {
        return _name;
    }

    ast::class_expression const& klass::expression() const
    {
        return _expression;
    }

    void klass::evaluate(evaluation::context& context, compiler::resource& resource) const
    {
        // Create a scope for the class
        auto scope = make_shared<evaluation::scope>(evaluate_parent(context), &resource);

        // Add the class' scope
        context.add_scope(scope);

        // Use a call evaluator to "call" the class
        call_evaluator evaluator{ context, _expression.parameters, _expression.body };

        // Evaluate in the class' scope
        evaluator.evaluate(resource, scope);
    }

    shared_ptr<scope> klass::evaluate_parent(evaluation::context& context) const
    {
        // If no parent, return the node or top scope
        auto const& parent = _expression.parent;
        if (!parent) {
            return context.node_or_top();
        }

        context.declare_class(parent->value, parent->context);
        return context.find_scope(parent->value);
    }

    defined_type::defined_type(string name, ast::defined_type_expression const& expression) :
        _name(rvalue_cast(name)),
        _tree(expression.context.tree->shared_from_this()),
        _expression(expression)
    {
    }

    string const& defined_type::name() const
    {
        return _name;
    }

    ast::defined_type_expression const& defined_type::expression() const
    {
        return _expression;
    }

    void defined_type::evaluate(evaluation::context& context, compiler::resource& resource) const
    {
        // Create a temporary scope for evaluating the defined type
        auto scope = make_shared<evaluation::scope>(context.node_or_top(), &resource);

        // Use a call evaluator to "call" the defined type
        call_evaluator evaluator{ context, _expression.parameters, _expression.body };

        // Evaluate in the temporary scope
        evaluator.evaluate(resource, scope);
    }

    node_definition::node_definition(ast::node_expression const& expression) :
        _tree(expression.context.tree->shared_from_this()),
        _expression(expression)
    {
    }

    ast::node_expression const& node_definition::expression() const
    {
        return _expression;
    }

    void node_definition::evaluate(evaluation::context& context, compiler::resource& resource) const
    {
        // Set the node scope for the remainder of the evaluation
        node_scope scope{ context, resource };

        // Use a call evaluator to "call" the node expressions's body
        vector<ast::parameter> parameters;
        call_evaluator evaluator{ context, parameters, _expression.body };

        // Evaluate in node scope
        evaluator.evaluate(context.node_scope());
    }

    void registry::import(ast::syntax_tree const& tree)
    {
        // Ensure trees are only scanned once
        if (_imported.count(&tree)) {
            return;
        }

        compiler::scanner scanner{ *this };
        scanner.scan(tree);

        // Mark the tree as imported
        _imported.emplace(&tree);
    }

    vector<klass> const* registry::find_class(string const& name) const
    {
        auto it = _classes.find(name);
        if (it == _classes.end() || it->second.empty()) {
            return nullptr;
        }
        return &it->second;
    }

    void registry::register_class(compiler::klass klass)
    {
        auto& definitions = _classes[klass.name()];
        definitions.emplace_back(rvalue_cast(klass));
    }

    defined_type const* registry::find_defined_type(string const& name) const
    {
        auto it = _defined_types.find(name);
        if (it == _defined_types.end()) {
            return nullptr;
        }
        return &it->second;
    }

    defined_type const* registry::register_defined_type(defined_type type)
    {
        // Add the defined type
        auto name = type.name();

        auto result = _defined_types.emplace(make_pair(rvalue_cast(name), rvalue_cast(type)));
        if (result.second) {
            return nullptr;
        }
        return &result.first->second;
    }

    std::pair<node_definition const*, std::string> registry::find_node(compiler::node const& node) const
    {
        // If there are no node definitions, do nothing
        if (_nodes.empty()) {
            return make_pair(nullptr, string());
        }

        // Find a node definition
        string node_name;
        node_definition const* definition = nullptr;
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
                return make_pair(nullptr, string());
            }
            node_name = "default";
            definition = &_nodes[*_default_node_index];
        }
        return make_pair(definition, rvalue_cast(node_name));
    }

    node_definition const* registry::find_node(ast::node_expression const& expression) const
    {
        for (auto const& hostname : expression.hostnames) {
            // Check for default node
            if (hostname.is_default()) {
                if (_default_node_index) {
                    return &_nodes[*_default_node_index];
                }
                continue;
            }

            auto name = hostname.to_string();

            // Check for regular expression names
            if (hostname.is_regex()) {
                auto it = find_if(_regex_nodes.begin(), _regex_nodes.end(), [&](std::pair<values::regex, size_t> const& existing) { return existing.first.pattern() == name; });
                if (it != _regex_nodes.end()) {
                    return &_nodes[it->second];
                }
                continue;
            }

            // Otherwise, this is a qualified node name
            auto it = _named_nodes.find(name);
            if (it != _named_nodes.end()) {
                return &_nodes[it->second];
            }
        }
        return nullptr;
    }

    node_definition const* registry::register_node(node_definition node)
    {
        // Check for a node that would conflict with the given one
        if (auto existing = find_node(node.expression())) {
            return existing;
        }

        // Create all the regexes now before modifying any data
        vector<values::regex> regexes;
        for (auto const& hostname : node.expression().hostnames) {
            // Check for regular expressions
            if (!hostname.is_regex()) {
                continue;
            }
            try {
                regexes.emplace_back(hostname.to_string());
            } catch (regex_error const& ex) {
                throw evaluation_exception((boost::format("invalid regular expression: %1%") % ex.what()).str(), hostname.context());
            }
        }

        // Add the node
        _nodes.emplace_back(rvalue_cast(node));
        size_t node_index = _nodes.size() - 1;
        for (auto const& hostname : _nodes.back().expression().hostnames) {
            // Skip regexes
            if (hostname.is_regex()) {
                continue;
            }

            // Check for default node
            if (hostname.is_default()) {
                _default_node_index = node_index;
                continue;
            }

            // Add a named node
            _named_nodes.emplace(make_pair(boost::to_lower_copy(hostname.to_string()), node_index));
        }

        // Populate the regexes
        for (auto& regex : regexes)
        {
            _regex_nodes.emplace_back(rvalue_cast(regex), node_index);
        }
        return nullptr;
    }

}}  // namespace puppet::compiler
