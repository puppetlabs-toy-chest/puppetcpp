#include <puppet/compiler/scanner.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <puppet/compiler/resource.hpp>
#include <puppet/runtime/types/class.hpp>
#include <puppet/cast.hpp>

using namespace std;
using namespace puppet::runtime;

namespace puppet { namespace compiler {

    // Utility class that keeps track of class scopes
    // Empty scopes on the stack represent scopes where definitions may not occur
    struct class_scope
    {
        explicit class_scope(vector<string>& scopes, string name = {}) :
            _scopes(scopes)
        {
            scopes.push_back(rvalue_cast(name));
        }

        ~class_scope()
        {
            _scopes.pop_back();
        }

     private:
        vector<string>& _scopes;
    };

    scanner::scanner(compiler::registry& registry) :
        _registry(registry)
    {
    }

    void scanner::scan(ast::syntax_tree const& tree)
    {
        // Reset for the scan
        _scopes.clear();
        _classes.clear();
        _defined_types.clear();
        _nodes.clear();
        
        // Check all parameters
        if (tree.parameters) {
            for (auto const& parameter : *tree.parameters) {
                operator()(parameter);
            }
        }

        // Scan first before registering anything
        // This attempts to make the scan transaction; if the scan throws an exception, nothing will be registered
        for (auto const& statement : tree.statements) {
            operator()(statement);
        }

        // Register the classes
        for (auto& klass : _classes) {
            _registry.register_class(rvalue_cast(klass));
        }

        // Register the defined types
        for (auto& type : _defined_types) {
            _registry.register_defined_type(rvalue_cast(type));
        }

        // Register the ndoes
        for (auto& node : _nodes) {
            _registry.register_node(rvalue_cast(node));
        }
    }

    void scanner::operator()(ast::undef const&)
    {
    }

    void scanner::operator()(ast::defaulted const&)
    {
    }

    void scanner::operator()(ast::boolean const& expression)
    {
    }

    void scanner::operator()(ast::number const& expression)
    {
    }

    void scanner::operator()(ast::string const& expression)
    {
    }

    void scanner::operator()(ast::regex const& expression)
    {
    }

    void scanner::operator()(ast::variable const& expression)
    {
        static const char* valid_variable_pattern = "0|[1-9]\\d*|((::)?[a-z]\\w*)*(::)?[a-z_]\\w*";
        static const regex valid_variable_regex(valid_variable_pattern);

        // Ensure the parameter name is valid
        if (!regex_match(expression.name, valid_variable_regex)) {
            throw parse_exception((boost::format("variable name '%1%' is not a valid variable name: the name must conform to /%2%/.") % expression.name % valid_variable_pattern).str(), expression.context.position);
        }
    }

    void scanner::operator()(ast::name const& expression)
    {
    }

    void scanner::operator()(ast::bare_word const& expression)
    {
    }

    void scanner::operator()(ast::type const& expression)
    {
    }

    void scanner::operator()(ast::array const& expression)
    {
        // Array expressions have no class scope
        class_scope scope(_scopes);

        for (auto const& element : expression.elements) {
            operator()(element);
        }
    }

    void scanner::operator()(ast::hash const& expression)
    {
        // Hash expressions have no class scope
        class_scope scope(_scopes);

        for (auto const& pair : expression.elements) {
            operator()(pair.first);
            operator()(pair.second);
        }
    }

    void scanner::operator()(ast::expression const& expression)
    {
        operator()(expression.postfix);

        for (auto const& binary : expression.remainder) {
            operator()(binary.operand);
        }
    }

    void scanner::operator()(ast::case_expression const& expression)
    {
        // Control flow expressions have no class scope
        class_scope scope(_scopes);

        operator()(expression.conditional);

        for (auto const& proposition : expression.propositions) {
            for (auto const& option : proposition.options) {
                operator()(option);
            }
            for (auto const& statement : proposition.body) {
                operator()(statement);
            }
        }
    }

    void scanner::operator()(ast::if_expression const& expression)
    {
        // Control flow expressions have no class scope
        class_scope scope(_scopes);

        operator()(expression.conditional);
        for (auto const& statement : expression.body) {
            operator()(statement);
        }

        for (auto const& elsif : expression.elsifs) {
            operator()(elsif.conditional);
            for (auto const& statement : elsif.body) {
                operator()(statement);
            }
        }

        if (expression.else_) {
            for (auto const& statement : expression.else_->body) {
                operator()(statement);
            }
        }
    }

    void scanner::operator()(ast::unless_expression const& expression)
    {
        // Control flow expressions have no class scope
        class_scope scope(_scopes);

        operator()(expression.conditional);
        for (auto const& statement : expression.body) {
            operator()(statement);
        }

        if (expression.else_) {
            for (auto const& statement : expression.else_->body) {
                operator()(statement);
            }
        }
    }

    void scanner::operator()(ast::function_call_expression const& expression)
    {
        // Control flow expressions have no class scope
        class_scope scope(_scopes);

        for (auto const& argument : expression.arguments) {
            operator()(argument);
        }
        if (expression.lambda) {
            operator()(*expression.lambda);
        }
    }

    void scanner::operator()(ast::resource_expression const& expression)
    {
        // Resource expressions have no class scope
        class_scope scope(_scopes);

        for (auto const& body : expression.bodies) {
            operator()(body.title);
            for (auto const& attribute : body.attributes) {
                operator()(attribute.value);
            }
        }
    }

    void scanner::operator()(ast::resource_override_expression const& expression)
    {
        // Resource expressions have no class scope
        class_scope scope(_scopes);

        operator()(expression.reference);

        for (auto const& attribute : expression.attributes) {
            operator()(attribute.value);
        }
    }

    void scanner::operator()(ast::resource_defaults_expression const& expression)
    {
        // Resource expressions have no class scope
        class_scope scope(_scopes);

        for (auto const& attribute : expression.attributes) {
            operator()(attribute.value);
        }
    }

    void scanner::operator()(ast::class_expression const& expression)
    {
        // Validate the class name
        string name = validate_name(true, expression.name);

        // Validate the class parameters
        validate_parameters(true, expression.parameters);

        // Validate the class' parent
        if (expression.parent) {
            auto definitions = _registry.find_class(name);
            if (definitions) {
                for (auto const& definition : *definitions) {
                    auto& existing = definition.expression();
                    auto& parent = existing.parent;
                    if (!parent || parent->value == expression.parent->value) {
                        // No parent or parents match
                        continue;
                    }
                    auto& context = existing.context;
                    throw parse_exception(
                        (boost::format("class '%1%' cannot inherit from '%2%' because the class already inherits from '%3%' at %4%:%5%.") %
                         name %
                         expression.parent->value %
                         parent->value %
                         context.tree->path() %
                         context.position.line()
                        ).str(),
                        expression.parent->context.position
                    );
                }
            }
        }

        // Add to list to register later
        _classes.emplace_back(name, expression);

        // Scan the parameters
        if (!expression.parameters.empty()) {
            // Parameters have no class scope
            class_scope scope(_scopes, {});

            for (auto const& parameter : expression.parameters) {
                operator()(parameter);
            }
        }

        // Scan the body
        if (!expression.body.empty()) {
            // Set the class scope
            class_scope scope(_scopes, name);

            for (auto const& statement : expression.body) {
                operator()(statement);
            }
        }

    }

    void scanner::operator()(ast::defined_type_expression const& expression)
    {
        auto name = validate_name(false, expression.name);

        // Validate the defined type parameters
        validate_parameters(false, expression.parameters);

        // Check if the defined type already exists
        if (auto existing = _registry.find_defined_type(name)) {
            throw parse_exception(
                (boost::format("defined type '%1%' was previously defined at %2%:%3%.") %
                 existing->name() %
                 existing->expression().context.tree->path() %
                 existing->expression().context.position.line()
                ).str(),
                expression.context.position);
        }

        // Add the defined type for later registration
        _defined_types.emplace_back(rvalue_cast(name), expression);

        // Defined types have no class scope
        class_scope scope(_scopes, {});

        // Scan the parameters
        for (auto const& parameter : expression.parameters) {
            operator()(parameter);
        }

        // Scan the body
        for (auto const& statement : expression.body) {
            operator()(statement);
        }
    }

    void scanner::operator()(ast::node_expression const& expression)
    {
        if (!can_define()) {
            throw parse_exception("node definitions can only be defined at top-level or inside a class.", expression.context.position);
        }

        // Check for existing conflicting node definition
        if (auto existing = _registry.find_node(expression)) {
            throw parse_exception(
                (boost::format("a conflicting node definition was previously defined at %1%:%2%.") %
                    existing->expression().context.tree->path() %
                    existing->expression().context.position.line()
                ).str(), expression.context.position);
        }

        // Add it for later registration
        _nodes.emplace_back(expression);

        // Node definitions have no class scope
        class_scope scope(_scopes, {});

        // Scan the body
        for (auto const& statement : expression.body) {
            operator()(statement);
        }
    }

    void scanner::operator()(ast::collector_expression const& expression)
    {
        // Collector expressions have no class scope
        class_scope scope(_scopes, {});

        // Scan the query
        if (expression.query) {
            operator()(*expression.query);
        }
    }

    void scanner::operator()(ast::collector_query_expression const& expression)
    {
        // Queries have no class scope
        class_scope scope(_scopes, {});

        operator()(expression.primary);

        for (auto const& binary : expression.remainder) {
            operator()(binary.operand);
        }
    }

    void scanner::operator()(ast::attribute_query_expression const& expression)
    {
        boost::apply_visitor(*this, expression);
    }

    void scanner::operator()(ast::attribute_query const& expression)
    {
        operator()(expression.value);
    }

    void scanner::operator()(ast::unary_expression const& expression)
    {
        operator()(expression.operand);
    }

    void scanner::operator()(ast::postfix_expression const& expression)
    {
        operator()(expression.primary);

        for (auto const& subexpression : expression.subexpressions)
        {
            boost::apply_visitor(*this, subexpression);
        }
    }

    void scanner::operator()(ast::selector_expression const& expression)
    {
        for (auto const& pair : expression.cases) {
            operator()(pair.first);
            operator()(pair.second);
        }
    }

    void scanner::operator()(ast::access_expression const& expression)
    {
        for (auto const& argument : expression.arguments) {
            operator()(argument);
        }
    }

    void scanner::operator()(ast::method_call_expression const& expression)
    {
        for (auto const& argument : expression.arguments) {
            operator()(argument);
        }
        if (expression.lambda) {
            operator()(*expression.lambda);
        }
    }

    void scanner::operator()(ast::lambda_expression const& expression)
    {
        for (auto const& parameter : expression.parameters) {
            validate_parameter_name(parameter);

            operator()(parameter);
        }

        // Scan the body
        for (auto const& statement : expression.body) {
            operator()(statement);
        }
    }

    void scanner::operator()(ast::parameter const& expression)
    {
        if (expression.type) {
            operator()(*expression.type);
        }
        if (expression.default_value) {
            operator()(*expression.default_value);
        }
    }

    void scanner::operator()(ast::primary_expression const& expression)
    {
        boost::apply_visitor(*this, expression);
    }

    void scanner::operator()(ast::epp_render_expression const& expression)
    {
        operator()(expression.expression);
    }

    void scanner::operator()(ast::epp_render_block const& expression)
    {
        for (auto const& statement : expression.block) {
            operator()(statement);
        }
    }

    void scanner::operator()(ast::epp_render_string const& expression)
    {
    }

    bool scanner::can_define() const
    {
        return _scopes.empty() || !_scopes.back().empty();
    }

    string scanner::qualify(string const& name) const
    {
        // Treat both class and defined types as normalized class names
        string normalized = name;
        types::klass::normalize(normalized);

        // Walk backwards looking for the last non-empty scope
        for (auto it = _scopes.rbegin(); it != _scopes.rend(); ++it) {
            if (it->empty()) {
                continue;
            }

            return *it + "::" + normalized;
        }
        return normalized;
    }

    string scanner::validate_name(bool is_class, ast::name const& name) const
    {
        if (!can_define()) {
            throw parse_exception((boost::format("%1% can only be defined at top-level or inside a class.") % (is_class ? "classes" : "defined types")).str(), name.context.position);
        }

        if (name.value.empty()) {
            throw parse_exception((boost::format("a %1% cannot have an empty name.") % (is_class ? "class" : "defined type")).str(), name.context.position);
        }

        // Ensure the name is valid
        if (boost::starts_with(name.value, "::")) {
            throw parse_exception((boost::format("'%1%' is not a valid %2% name.") % name % (is_class ? "class" : "defined type")).str(), name.context.position);
        }

        // Cannot define a class called "main" or "settings" because they are built-in objects
        auto qualified_name = qualify(name.value);
        if (qualified_name == "main" || qualified_name == "settings") {
            throw parse_exception((boost::format("'%1%' is the name of a built-in class and cannot be used.") % qualified_name).str(), name.context.position);
        }

        // Check for conflicts between defined types and classes
        if (is_class) {
            auto type = _registry.find_defined_type(qualified_name);
            if (type) {
                throw parse_exception(
                    (boost::format("'%1%' was previously defined as a defined type at %2%:%3%.") %
                     qualified_name %
                     type->expression().context.tree->path() %
                     type->expression().context.position.line()
                    ).str(),
                    name.context.position);
            }
        } else {
            auto definitions = _registry.find_class(qualified_name);
            if (definitions) {
                auto& first = definitions->front();
                throw parse_exception(
                    (boost::format("'%1%' was previously defined as a class at %2%:%3%.") %
                     qualified_name %
                     first.expression().context.tree->path() %
                     first.expression().context.position.line()
                    ).str(),
                    name.context.position);
            }
        }
        return qualified_name;
    }

    void scanner::validate_parameters(bool is_class, vector<ast::parameter> const& parameters) const
    {
        for (auto const& parameter : parameters) {
            auto const& name = parameter.variable.name;

            // Validate the name
            validate_parameter_name(parameter);

            // Check for reserved names for classes and defined types
            if (name == "title" || name == "name") {
                throw parse_exception((boost::format("parameter $%1% is reserved and cannot be used.") % name).str(), parameter.context().position);
            }

            // Check for capture parameters
            if (parameter.captures) {
                throw parse_exception((boost::format("%1% parameter $%2% cannot \"captures rest\".") % (is_class ? "class" : "defined type") % name).str(), parameter.context().position);
            }

            // Check for metaparameter names
            if (resource::is_metaparameter(name)) {
                throw parse_exception((boost::format("parameter $%1% is reserved for resource metaparameter '%1%'.") % name).str(), parameter.context().position);
            }
        }
    }

    void scanner::validate_parameter_name(ast::parameter const& parameter) const
    {
        static char const* valid_name_pattern = "[a-z_]\\w*";
        static const std::regex valid_name_regex(valid_name_pattern);

        if (!regex_match(parameter.variable.name, valid_name_regex)) {
            throw parse_exception(
                (boost::format("parameter $%1% has an unacceptable name: the name must conform to /%2%/.") %
                 parameter.variable.name %
                 valid_name_pattern
                ).str(),
                parameter.variable.context.position);
        }
    }

}}  // namespace puppet::compiler
