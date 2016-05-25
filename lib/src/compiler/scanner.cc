#include <puppet/compiler/scanner.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <puppet/compiler/resource.hpp>
#include <puppet/compiler/validation/type_validator.hpp>
#include <puppet/runtime/types/class.hpp>
#include <puppet/cast.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace puppet::compiler::validation;
using namespace puppet::runtime;

namespace puppet { namespace compiler {

    // Utility class that keeps track of class scopes
    // null scopes on the stack represent scopes where definitions may not occur
    struct scope_helper
    {
        explicit scope_helper(vector<ast::class_expression const*>& scopes) :
            _scopes(scopes)
        {
            scopes.emplace_back(nullptr);
        }

        scope_helper(vector<ast::class_expression const*>& scopes, ast::class_expression const& expression) :
            _scopes(scopes)
        {
            scopes.emplace_back(&expression);
        }

        ~scope_helper()
        {
            _scopes.pop_back();
        }

     private:
        vector<ast::class_expression const*>& _scopes;
    };

    scanner::scanner(compiler::registry& registry, evaluation::dispatcher& dispatcher) :
        _registry(registry),
        _dispatcher(dispatcher),
        _found_definition(false)
    {
    }

    void scanner::scan(ast::syntax_tree const& tree)
    {
        // Reset for the scan
        _scopes.clear();
        _found_definition = false;

        if (tree.parameters) {
            for (auto const& parameter : *tree.parameters) {
                operator()(parameter);
            }
        }

        for (auto const& statement : tree.statements) {
            operator()(statement);
        }
    }

    bool scanner::scan(ast::expression const& expression)
    {
        // Reset for the scan
        _scopes.clear();
        _found_definition = false;

        operator()(expression);
        return _found_definition;
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

    void scanner::operator()(ast::literal_string_text const& expression)
    {
    }

    void scanner::operator()(ast::interpolated_string const& expression)
    {
        for (auto const& part : expression.parts) {
            boost::apply_visitor(*this, part);
        }
    }

    void scanner::operator()(ast::array const& expression)
    {
        scope_helper scope{ _scopes };

        for (auto const& element : expression.elements) {
            operator()(element);
        }
    }

    void scanner::operator()(ast::hash const& expression)
    {
        scope_helper scope{ _scopes };

        for (auto const& pair : expression.elements) {
            operator()(pair.first);
            operator()(pair.second);
        }
    }

    void scanner::operator()(ast::expression const& expression)
    {
        if (expression.operations.empty()) {
            operator()(expression.first);
            return;
        }

        // Binary expressions cannot contain definitions
        scope_helper scope{ _scopes };

        operator()(expression.first);
        for (auto const& operation : expression.operations) {
            operator()(operation.operand);
        }
    }

    void scanner::operator()(ast::nested_expression const& expression)
    {
        operator()(expression.expression);
    }

    void scanner::operator()(ast::case_expression const& expression)
    {
        scope_helper scope{ _scopes };

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
        scope_helper scope{ _scopes };

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
        scope_helper scope{ _scopes };

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
        scope_helper scope{ _scopes };

        for (auto const& argument : expression.arguments) {
            operator()(argument);
        }
        if (expression.lambda) {
            operator()(*expression.lambda);
        }
    }

    void scanner::operator()(ast::new_expression const& expression)
    {
        scope_helper scope{ _scopes };

        operator()(expression.type);

        for (auto const& argument : expression.arguments) {
            operator()(argument);
        }
    }

    void scanner::operator()(ast::resource_expression const& expression)
    {
        scope_helper scope{ _scopes };

        for (auto const& body : expression.bodies) {
            operator()(body.title);
            for (auto const& operation : body.operations) {
                operator()(operation.value);
            }
        }
    }

    void scanner::operator()(ast::resource_override_expression const& expression)
    {
        scope_helper scope{ _scopes };

        operator()(expression.reference);

        for (auto const& operation : expression.operations) {
            operator()(operation.value);
        }
    }

    void scanner::operator()(ast::resource_defaults_expression const& expression)
    {
        scope_helper scope{ _scopes };

        for (auto const& operation : expression.operations) {
            operator()(operation.value);
        }
    }

    void scanner::operator()(ast::class_expression const& expression)
    {
        // Validate the class name
        string name = validate_name(true, expression.name);

        // Check if the class already exists
        if (auto existing = _registry.find_class(name)) {
            throw parse_exception(
                (boost::format("class '%1%' was previously defined at %2%:%3%.") %
                 existing->name() %
                 existing->expression().tree->path() %
                 existing->expression().begin.line()
                ).str(),
                expression.name.begin,
                expression.name.end
            );
        }

        // Validate the class parameters
        validate_parameters("class", expression.parameters);

        // Scan the parameters
        if (!expression.parameters.empty()) {
            scope_helper scope{ _scopes };

            for (auto const& parameter : expression.parameters) {
                operator()(parameter);
            }
        }

        // Scan the body
        if (!expression.body.empty()) {
            // Set the class scope
            scope_helper scope(_scopes, expression);

            for (auto const& statement : expression.body) {
                operator()(statement);
            }
        }

        // Register the class
        _registry.register_class(klass{ rvalue_cast(name), expression });
        _found_definition = true;
    }

    void scanner::operator()(ast::defined_type_expression const& expression)
    {
        auto name = validate_name(false, expression.name);

        // Validate the defined type parameters
        validate_parameters("defined type", expression.parameters);

        // Check if the defined type already exists
        if (auto existing = _registry.find_defined_type(name)) {
            throw parse_exception(
                (boost::format("defined type '%1%' was previously defined at %2%:%3%.") %
                 existing->name() %
                 existing->expression().tree->path() %
                 existing->expression().begin.line()
                ).str(),
                expression.name.begin,
                expression.name.end
            );
        }

        scope_helper scope{ _scopes };

        // Scan the parameters
        for (auto const& parameter : expression.parameters) {
            operator()(parameter);
        }

        // Scan the body
        for (auto const& statement : expression.body) {
            operator()(statement);
        }

        // Register the defined type
        _registry.register_defined_type(defined_type{ rvalue_cast(name), expression });
        _found_definition = true;
    }

    void scanner::operator()(ast::node_expression const& expression)
    {
        if (!can_define()) {
            throw parse_exception("node definitions can only be defined at top-level or inside a class.", expression.begin, expression.end);
        }

        // Check for valid host names
        for (auto& hostname : expression.hostnames) {
            if (!hostname.is_valid()) {
                auto context = hostname.context();
                throw parse_exception(
                    (boost::format("hostname '%1%' is not valid: only letters, digits, '_', '-', and '.' are allowed.") %
                     hostname.to_string()
                    ).str(),
                    context.begin,
                    context.end
                );
            }
        }

        // Check for existing conflicting node definition
        if (auto existing = _registry.find_node(expression)) {
            throw parse_exception(
                (boost::format("a conflicting node definition was previously defined at %1%:%2%.") %
                    existing->expression().tree->path() %
                    existing->expression().begin.line()
                ).str(),
                expression.begin,
                expression.end
            );
        }

        scope_helper scope{ _scopes };

        // Scan the body
        for (auto const& statement : expression.body) {
            operator()(statement);
        }

        // Register the node
        _registry.register_node(node_definition{ expression });
        _found_definition = true;
    }

    void scanner::operator()(ast::collector_expression const& expression)
    {
        scope_helper scope{ _scopes };

        // Scan the query
        if (expression.query) {
            operator()(*expression.query);
        }
    }

    void scanner::operator()(ast::query_expression const& expression)
    {
        scope_helper scope{ _scopes };

        operator()(expression.primary);

        for (auto const& operation : expression.operations) {
            operator()(operation.operand);
        }
    }

    void scanner::operator()(ast::nested_query_expression const& expression)
    {
        operator()(expression.expression);
    }

    void scanner::operator()(ast::primary_query_expression const& expression)
    {
        boost::apply_visitor(*this, expression);
    }

    void scanner::operator()(ast::attribute_query const& expression)
    {
        operator()(expression.value);
    }

    void scanner::operator()(ast::function_expression const& expression)
    {
        if (!_scopes.empty()) {
            throw parse_exception("function definitions can only be defined at top-level.", expression.begin, expression.end);
        }

        // First check for existing functions in the dispatcher
        if (auto descriptor = _dispatcher.find(expression.name.value)) {
            if (auto existing = descriptor->expression()) {
                throw parse_exception(
                    (boost::format("cannot define function '%1%' because it conflicts with a previous definition at %2%:%3%.") %
                     expression.name %
                     existing->tree->path() %
                     existing->begin.line()
                    ).str(),
                    expression.name.begin,
                    expression.name.end
                );
            }
            throw parse_exception(
                (boost::format("cannot define function '%1%' because it conflicts with a built-in function of the same name.") %
                 expression.name
                ).str(),
                expression.name.begin,
                expression.name.end
            );
        }

        scope_helper scope{ _scopes };

        // Scan the parameters
        for (auto const& parameter : expression.parameters) {
            operator()(parameter);
        }

        // Scan the body
        for (auto const& statement : expression.body) {
            operator()(statement);
        }

        // Add the function
        _dispatcher.add(evaluation::functions::descriptor{ expression.name.value, &expression });
        _found_definition = true;
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

    void scanner::operator()(ast::produces_expression const& expression)
    {
        // TODO: register the mapping

        scope_helper scope{ _scopes };

        for (auto const& operation : expression.operations) {
            operator()(operation.value);
        }
    }

    void scanner::operator()(ast::consumes_expression const& expression)
    {
        // TODO: register the mapping

        scope_helper scope{ _scopes };

        for (auto const& operation : expression.operations) {
            operator()(operation.value);
        }
    }

    void scanner::operator()(ast::application_expression const& expression)
    {
        // TODO: register the application

        validate_parameters("application", expression.parameters);

        // Scan the body
        for (auto const& statement : expression.body) {
            operator()(statement);
        }
    }

    void scanner::operator()(ast::site_expression const& expression)
    {
        // TODO: register the site

        for (auto const& statement : expression.body) {
            operator()(statement);
        }
    }

    void scanner::operator()(ast::type_alias_expression const& expression)
    {
        if (!_scopes.empty()) {
            auto context = expression.context();
            throw parse_exception("type aliases can only be defined at top-level.", context.begin, context.end);
        }

        if (values::type::find(expression.alias.name)) {
            throw parse_exception(
                (boost::format("type alias '%1%' conflicts with a built-in type of the same name.") %
                 expression.alias
                ).str(),
                expression.alias.begin,
                expression.alias.end
            );
        }

        auto alias = _registry.find_type_alias(expression.alias.name);
        if (alias) {
            auto context = alias->expression().context();
            throw parse_exception(
                (boost::format("type alias '%1%' was previously defined at %2%:%3%.") %
                 expression.alias %
                 context.tree->path() %
                 context.begin.line()
                ).str(),
                expression.alias.begin,
                expression.alias.end
            );
        }

        // Validate the RHS expression
        type_validator::validate(expression.type);

        scope_helper scope{ _scopes };

        operator()(expression.alias);
        operator()(expression.type);

        _registry.register_type_alias(type_alias{ expression });
    }

    bool scanner::can_define() const
    {
        return _scopes.empty() || _scopes.back();
    }

    string scanner::qualify(string const& name) const
    {
        // Walk backwards looking for the last non-empty scope
        string qualified;
        for (auto it = _scopes.rbegin(); it != _scopes.rend(); ++it) {
            if (!*it) {
                continue;
            }
            qualified = (*it)->name.value + "::" + name;
            break;
        }

        if (qualified.empty()) {
            qualified = name;
        }

        types::klass::normalize(qualified);
        return qualified;
    }

    string scanner::validate_name(bool is_class, ast::name const& name) const
    {
        if (!can_define()) {
            throw parse_exception((boost::format("%1% can only be defined at top-level or inside a class.") % (is_class ? "classes" : "defined types")).str(), name.begin, name.end);
        }

        if (name.value.empty()) {
            throw parse_exception((boost::format("a %1% cannot have an empty name.") % (is_class ? "class" : "defined type")).str(), name.begin, name.end);
        }

        // Ensure the name is valid
        if (boost::starts_with(name.value, "::")) {
            throw parse_exception((boost::format("'%1%' is not a valid %2% name.") % name % (is_class ? "class" : "defined type")).str(), name.begin, name.end);
        }

        // Cannot define a class called "main" or "settings" because they are built-in objects
        auto qualified_name = qualify(name.value);
        if (qualified_name == "main" || qualified_name == "settings") {
            throw parse_exception((boost::format("'%1%' is the name of a built-in class and cannot be used.") % qualified_name).str(), name.begin, name.end);
        }

        // Check for conflicts between defined types and classes
        if (is_class) {
            auto type = _registry.find_defined_type(qualified_name);
            if (type) {
                throw parse_exception(
                    (boost::format("'%1%' was previously defined as a defined type at %2%:%3%.") %
                     qualified_name %
                     type->expression().tree->path() %
                     type->expression().begin.line()
                    ).str(),
                    name.begin,
                    name.end
                );
            }
        } else {
            auto klass = _registry.find_class(qualified_name);
            if (klass) {
                throw parse_exception(
                    (boost::format("'%1%' was previously defined as a class at %2%:%3%.") %
                     qualified_name %
                     klass->expression().tree->path() %
                     klass->expression().begin.line()
                    ).str(),
                    name.begin,
                    name.end
                );
            }
        }
        return qualified_name;
    }

    void scanner::validate_parameters(char const* type, vector<ast::parameter> const& parameters) const
    {
        for (auto const& parameter : parameters) {
            auto const& name = parameter.variable.name;

            validate_parameter_name(parameter);

            if (parameter.type) {
                type_validator::validate(*parameter.type);
            }

            // Check for reserved names
            if (name == "title" || name == "name") {
                throw parse_exception((boost::format("parameter $%1% is reserved and cannot be used.") % name).str(), parameter.variable.begin, parameter.variable.end);
            }

            // Check for capture parameters
            if (parameter.captures) {
                throw parse_exception((boost::format("%1% parameter $%2% cannot \"captures rest\".") % type % name).str(), parameter.variable.begin, parameter.variable.end);
            }

            // Check for metaparameter names
            if (resource::is_metaparameter(name)) {
                throw parse_exception((boost::format("parameter $%1% is reserved for resource metaparameter '%1%'.") % name).str(), parameter.variable.begin, parameter.variable.end);
            }
        }
    }

    void scanner::validate_parameter_name(ast::parameter const& parameter) const
    {
        static char const* valid_name_pattern = "[a-z_]\\w*";
        static const utility::regex valid_name_regex{ valid_name_pattern };

        if (!valid_name_regex.match(parameter.variable.name)) {
            throw parse_exception(
                (boost::format("parameter $%1% has an unacceptable name: the name must conform to /%2%/.") %
                 parameter.variable.name %
                 valid_name_pattern
                ).str(),
                parameter.variable.begin,
                parameter.variable.end
            );
        }
    }

}}  // namespace puppet::compiler
