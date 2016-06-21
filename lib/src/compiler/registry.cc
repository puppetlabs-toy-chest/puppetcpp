#include <puppet/compiler/registry.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <puppet/compiler/node.hpp>
#include <puppet/cast.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace puppet::runtime;
using namespace puppet::compiler::evaluation;

namespace puppet { namespace compiler {

    klass::klass(string name, ast::class_statement const& statement) :
        _name(rvalue_cast(name)),
        _tree(statement.tree->shared_from_this()),
        _statement(statement)
    {
    }

    string const& klass::name() const
    {
        return _name;
    }

    ast::class_statement const& klass::statement() const
    {
        return _statement;
    }

    defined_type::defined_type(string name, ast::defined_type_statement const& statement) :
        _name(rvalue_cast(name)),
        _tree(statement.tree->shared_from_this()),
        _statement(statement)
    {
    }

    string const& defined_type::name() const
    {
        return _name;
    }

    ast::defined_type_statement const& defined_type::statement() const
    {
        return _statement;
    }

    node_definition::node_definition(ast::node_statement const& statement) :
        _tree(statement.tree->shared_from_this()),
        _statement(statement)
    {
    }

    ast::node_statement const& node_definition::statement() const
    {
        return _statement;
    }

    type_alias::type_alias(ast::type_alias_statement const& statement) :
        _tree(statement.alias.tree->shared_from_this()),
        _statement(statement)
    {
    }

    ast::type_alias_statement const& type_alias::statement() const
    {
        return _statement;
    }

    klass const* registry::find_class(string const& name) const
    {
        auto it = _classes.find(name);
        if (it == _classes.end()) {
            return nullptr;
        }
        return &it->second;
    }

    void registry::register_class(compiler::klass klass)
    {
        auto name = klass.name();
        _classes.emplace(rvalue_cast(name), rvalue_cast(klass));
    }

    defined_type const* registry::find_defined_type(string const& name) const
    {
        auto it = _defined_types.find(name);
        if (it == _defined_types.end()) {
            return nullptr;
        }
        return &it->second;
    }

    void registry::register_defined_type(defined_type type)
    {
        auto name = type.name();
        _defined_types.emplace(rvalue_cast(name), rvalue_cast(type));
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
                if (kvp.first.search(name)) {
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

    node_definition const* registry::find_node(ast::node_statement const& statement) const
    {
        for (auto const& hostname : statement.hostnames) {
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
        if (auto existing = find_node(node.statement())) {
            return existing;
        }

        // Create all the regexes now before modifying any data
        vector<values::regex> regexes;
        for (auto const& hostname : node.statement().hostnames) {
            // Check for regular expressions
            if (!hostname.is_regex()) {
                continue;
            }
            try {
                regexes.emplace_back(hostname.to_string());
            } catch (utility::regex_exception const& ex) {
                throw parse_exception(
                    (boost::format("invalid regular expression: %1%") %
                     ex.what()
                    ).str(),
                    hostname.context().begin,
                    hostname.context().end
                );
            }
        }

        // Add the node
        _nodes.emplace_back(rvalue_cast(node));
        size_t node_index = _nodes.size() - 1;
        for (auto const& hostname : _nodes.back().statement().hostnames) {
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
            _named_nodes.emplace(boost::to_lower_copy(hostname.to_string()), node_index);
        }

        // Populate the regexes
        for (auto& regex : regexes)
        {
            _regex_nodes.emplace_back(rvalue_cast(regex), node_index);
        }
        return nullptr;
    }

    bool registry::has_nodes() const
    {
        return !_nodes.empty();
    }

    void registry::register_type_alias(type_alias alias)
    {
        _aliases.emplace(alias.statement().alias.name, rvalue_cast(alias));
    }

    type_alias* registry::find_type_alias(string const& name)
    {
        return const_cast<type_alias*>(static_cast<registry const*>(this)->find_type_alias(name));
    }

    type_alias const* registry::find_type_alias(string const& name) const
    {
        auto it = _aliases.find(name);
        if (it == _aliases.end()) {
            return nullptr;
        }
        return &it->second;
    }

}}  // namespace puppet::compiler
