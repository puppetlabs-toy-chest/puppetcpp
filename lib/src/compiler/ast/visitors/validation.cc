#include <puppet/compiler/ast/visitors/validation.hpp>
#include <puppet/compiler/ast/visitors/ineffective.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <puppet/compiler/resource.hpp>
#include <puppet/runtime/values/type.hpp>
#include <puppet/utility/regex.hpp>
#include <puppet/utility/indirect_collection.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;

namespace puppet { namespace compiler { namespace ast { namespace visitors {

    validation::validation(bool epp, bool allow_catalog_statements) :
        _epp(epp),
        _allow_catalog_statements(allow_catalog_statements)
    {
    }

    void validation::visit(syntax_tree const& tree)
    {
        location_helper helper{*this, _epp ? location::epp : location::top };

        if (tree.parameters) {
            validate_parameters(*tree.parameters, false, true);
        }

        validate_body(tree.statements, true);
    }

    void validation::visit(ast::statement const& statement, bool effective)
    {
        operator()(statement, effective);
    }

    void validation::operator()(basic_expression const& expression)
    {
        boost::apply_visitor(*this, expression);
    }

    void validation::operator()(undef const&)
    {
    }

    void validation::operator()(defaulted const&)
    {
    }

    void validation::operator()(boolean const& expression)
    {
    }

    void validation::operator()(number const& expression)
    {
    }

    void validation::operator()(ast::string const& expression)
    {
    }

    void validation::operator()(regex const& expression)
    {
    }

    void validation::operator()(variable const& expression)
    {
        // Check for illegal parameter references
        for (auto parameter = _parameter_begin; parameter != _parameter_end; ++parameter) {
            if (parameter->variable.name == expression.name) {
                throw parse_exception(
                    (boost::format("parameter %1% cannot be referenced before it is evaluated (evaluation is left-to-right).") %
                     expression
                    ).str(),
                    expression.begin,
                    expression.end
                );
            }
        }
    }

    void validation::operator()(name const& expression)
    {
    }

    void validation::operator()(bare_word const& expression)
    {
    }

    void validation::operator()(type const& expression)
    {
    }

    void validation::operator()(interpolated_string const& expression)
    {
        location_helper helper{*this, location::interpolated_string };
        for (auto& part : expression.parts) {
            boost::apply_visitor(*this, part);
        }
    }

    void validation::operator()(literal_string_text const&)
    {
    }

    void validation::operator()(ast::array const& expression)
    {
        for (auto& element : expression.elements) {
            operator()(element);
        }
    }

    void validation::operator()(hash const& expression)
    {
        for (auto& element : expression.elements) {
            operator()(element.first);
            operator()(element.second);
        }
    }

    void validation::operator()(case_expression const& expression)
    {
        operator()(expression.conditional);

        for (auto const& proposition : expression.propositions) {
            for (auto const& option : proposition.options) {
                operator()(option);
            }

            validate_body(proposition.body, true);
        }
    }

    void validation::operator()(if_expression const& expression)
    {
        operator()(expression.conditional);
        validate_body(expression.body, true);

        for (auto const& elsif : expression.elsifs) {
            operator()(elsif.conditional);
            validate_body(elsif.body, true);
        }

        if (expression.else_) {
            validate_body(expression.else_->body, true);
        }
    }

    void validation::operator()(unless_expression const& expression)
    {
        operator()(expression.conditional);
        validate_body(expression.body, true);

        if (expression.else_) {
            validate_body(expression.else_->body, true);
        }
    }

    void validation::operator()(function_call_expression const& expression)
    {
        for (auto const& argument : expression.arguments) {
            operator()(argument);
        }
        if (expression.lambda) {
            operator()(expression.function, *expression.lambda);
        }
    }

    void validation::operator()(name const& function, lambda_expression const& expression)
    {
        location_helper helper{ *this, location::lambda };
        validate_parameters(expression.parameters);
        validate_body(expression.body, true);
    }

    void validation::operator()(new_expression const& expression)
    {
        operator()(expression.type);

        for (auto const& argument : expression.arguments) {
            operator()(argument);
        }
    }

    void validation::operator()(epp_render_expression const& expression)
    {
        operator()(expression.expression);
    }

    void validation::operator()(epp_render_block const& expression)
    {
        for (auto const& expr : expression.block) {
            operator()(expr);
        }
    }

    void validation::operator()(epp_render_string const& expression)
    {
    }

    void validation::operator()(unary_expression const& expression)
    {
        operator()(expression.operand);
    }

    void validation::operator()(nested_expression const& expression)
    {
        operator()(expression.expression);
    }

    void validation::operator()(ast::expression const& expression)
    {
        if (expression.operations.empty()) {
            operator()(expression.operand);
            return;
        }

        operator()(expression.operand);

        postfix_expression const* left = &expression.operand;
        for (auto const& operation : expression.operations) {
            if (operation.operator_ == binary_operator::assignment) {
                // If we're checking for illegal parameter references, then the assignment operation is illegal inside a default value
                if (_parameter_begin) {
                    auto context = operation.context();
                    context.begin = left->context().begin;
                    throw parse_exception("assignment expressions are not allowed in parameter default values.", context.begin, context.end);
                }
                validate_assignment_operand(*left);
            }

            operator()(operation.operand);
            left = &operation.operand;
        }
    }

    void validation::operator()(postfix_expression const& expression)
    {
        operator()(expression.operand);

        for (auto const& operation : expression.operations)
        {
            boost::apply_visitor(*this, operation);
        }
    }

    void validation::operator()(selector_expression const& expression)
    {
        for (auto const& pair : expression.cases) {
            operator()(pair.first);
            operator()(pair.second);
        }
    }

    void validation::operator()(access_expression const& expression)
    {
        for (auto const& argument : expression.arguments) {
            operator()(argument);
        }
    }

    void validation::operator()(method_call_expression const& expression)
    {
        for (auto const& argument : expression.arguments) {
            operator()(argument);
        }
        if (expression.lambda) {
            operator()(expression.method, *expression.lambda);
        }
    }

    void validation::operator()(ast::statement const& statement, bool effective)
    {
        if (effective) {
            visitors::ineffective visitor;
            if (visitor.visit(statement)) {
                auto context = statement.context();
                throw parse_exception("ineffective statements may only appear as the last statement at top-level or inside functions and conditional expressions.", context.begin, context.end);
            }
        }
        boost::apply_visitor(*this, statement);
    }

    void validation::operator()(class_statement const& statement)
    {
        validate_catalog_statement(statement);

        auto current = current_location();
        if (current != location::top && current != location::class_) {
            throw parse_exception("classes can only be defined at top-level or inside another class.", statement.begin, statement.end);
        }

        // Don't allow empty names, non-local names, or names of built-in classes
        if (statement.name.value.empty() ||
            boost::starts_with(statement.name.value, "::") ||
            (current_location() == location::top && (statement.name.value == "main" || statement.name.value == "settings"))) {
            throw parse_exception((boost::format("'%1%' is not a valid name for a class.") % statement.name).str(), statement.name.begin, statement.name.end);
        }

        location_helper helper{*this, location::class_ };

        validate_parameters(statement.parameters, true, true);
        validate_body(statement.body, false);
    }

    void validation::operator()(defined_type_statement const& statement)
    {
        validate_catalog_statement(statement);

        auto current = current_location();
        if (current != location::top && current != location::class_) {
            throw parse_exception("defined types can only be defined at top-level or inside a class.", statement.begin, statement.end);
        }

        if (statement.name.value.empty() || boost::starts_with(statement.name.value, "::")) {
            throw parse_exception((boost::format("'%1%' is not a valid name for a defined type.") % statement.name).str(), statement.name.begin, statement.name.end);
        }

        location_helper helper{*this, location::defined_type };

        validate_parameters(statement.parameters, true, true);
        validate_body(statement.body, false);
    }

    void validation::operator()(node_statement const& statement)
    {
        validate_catalog_statement(statement);

        auto current = current_location();
        if (current != location::top && current != location::class_) {
            throw parse_exception("node definitions can only be defined at top-level or inside a class.", statement.begin, statement.end);
        }

        // Check for valid host names
        for (auto& hostname : statement.hostnames) {
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

        location_helper helper{*this, location::node };
        validate_body(statement.body, false);
    }

    void validation::operator()(function_statement const& statement)
    {
        auto current = current_location();
        if (current != location::top) {
            throw parse_exception("functions can only be defined at top-level.", statement.begin, statement.end);
        }

        if (statement.name.value.empty() || boost::starts_with(statement.name.value, "::")) {
            throw parse_exception((boost::format("'%1%' is not a valid name for a function.") % statement.name).str(), statement.name.begin, statement.name.end);
        }

        location_helper helper{*this, location::function };

        validate_parameters(statement.parameters);
        validate_body(statement.body, true);
    }

    void validation::operator()(produces_statement const& statement)
    {
        validate_catalog_statement(statement.context());

        auto current = current_location();
        if (current != location::top && current != location::site) {
            auto context = statement.context();
            throw parse_exception("capability mappings can only be defined at top-level or inside a site.", context.begin, context.end);
        }

        for (auto const& operation : statement.operations) {
            operator()(operation.value);
        }
    }

    void validation::operator()(consumes_statement const& statement)
    {
        validate_catalog_statement(statement.context());

        auto current = current_location();
        if (current != location::top && current != location::site) {
            auto context = statement.context();
            throw parse_exception("capability mappings can only be defined at top-level or inside a site.", context.begin, context.end);
        }

        for (auto const& operation : statement.operations) {
            operator()(operation.value);
        }
    }

    void validation::operator()(application_statement const& statement)
    {
        validate_catalog_statement(statement);

        auto current = current_location();
        if (current != location::top) {
            throw parse_exception("applications can only be defined at top-level.", statement.begin, statement.end);
        }

        if (statement.name.value.empty() || boost::starts_with(statement.name.value, "::")) {
            throw parse_exception((boost::format("'%1%' is not a valid name for an application.") % statement.name).str(), statement.name.begin, statement.name.end);
        }

        location_helper helper{*this, location::application };
        validate_parameters(statement.parameters, true, true);
        validate_body(statement.body, false);
    }

    void validation::operator()(site_statement const& statement)
    {
        validate_catalog_statement(statement);

        auto current = current_location();
        if (current != location::top) {
            throw parse_exception("sites can only be defined at top-level.", statement.begin, statement.end);
        }

        location_helper helper{*this, location::site };
        validate_body(statement.body, false);
    }

    void validation::operator()(type_alias_statement const& statement)
    {
        auto current = current_location();
        if (current != location::top) {
            auto context = statement.context();
            throw parse_exception("type aliases can only be defined at top-level.", context.begin, context.end);
        }

        if (statement.alias.name.empty() || boost::starts_with(statement.alias.name, "::")) {
            throw parse_exception((boost::format("'%1%' is not a valid name for a type alias.") % statement.alias).str(), statement.alias.begin, statement.alias.end);
        }

        if (runtime::values::type::find(statement.alias.name)) {
            throw parse_exception(
                (boost::format("type alias '%1%' conflicts with a built-in type of the same name.") %
                 statement.alias
                ).str(),
                statement.alias.begin,
                statement.alias.end
            );
        }

        statement.type.validate_type();
    }

    void validation::operator()(function_call_statement const& statement)
    {
        for (auto const& argument : statement.arguments) {
            operator()(argument);
        }
        if (statement.lambda) {
            operator()(statement.function, *statement.lambda);
        }
    }

    void validation::operator()(relationship_statement const& statement)
    {
        // If there are no operations, then this isn't actually a relationship statement
        if (statement.operations.empty()) {
            operator()(statement.operand);
            return;
        }

        validate_catalog_statement(statement.context());

        location_helper helper{*this, location::relationship };
        operator()(statement.operand);

        for (auto const& operation : statement.operations) {
            operator()(operation.operand);
        }
    }

    void validation::operator()(relationship_expression const& expression)
    {
        boost::apply_visitor(*this, expression);
    }

    void validation::operator()(resource_declaration_expression const& expression)
    {
        validate_catalog_statement(expression);

        location_helper helper{*this, location::resource_declaration };
        for (auto const& body : expression.bodies) {
            operator()(body.title);

            for (auto const& operation : body.operations) {
                operator()(operation.value);
            }
        }
    }

    void validation::operator()(resource_override_expression const& expression)
    {
        validate_catalog_statement(expression);

        location_helper helper{*this, location::resource_override };
        boost::apply_visitor(*this, expression.reference);
        for (auto const& operation : expression.operations) {
            operator()(operation.value);
        }
    }

    void validation::operator()(resource_defaults_expression const& expression)
    {
        validate_catalog_statement(expression);

        location_helper helper{*this, location::resource_defaults };
        for (auto const& operation : expression.operations) {
            operator()(operation.value);
        }
    }

    void validation::operator()(collector_expression const& expression)
    {
        validate_catalog_statement(expression.context());

        if (expression.query) {
            operator()(*expression.query);
        }
    }

    void validation::operator()(query_expression const& expression)
    {
        operator()(expression.operand);

        for (auto const& operation : expression.operations) {
            operator()(operation.operand);
        }
    }

    void validation::operator()(nested_query_expression const& expression)
    {
        operator()(expression.expression);
    }

    void validation::operator()(basic_query_expression const& expression)
    {
        boost::apply_visitor(*this, expression);
    }

    void validation::operator()(attribute_query const& expression)
    {
        operator()(expression.value);
    }

    void validation::operator()(break_statement const& statement)
    {
        auto result = validate_transfer_statement(false);
        if (result.first) {
            throw parse_exception(
                (boost::format("break statement cannot be used inside %1%.") % result.first).str(),
                statement.begin,
                statement.end
            );
        }

        if (result.second != location::lambda) {
            throw parse_exception(
                "break statement must be used from within a block.",
                statement.begin,
                statement.end
            );
        }
    }

    void validation::operator()(next_statement const& statement)
    {
        auto result = validate_transfer_statement(false);
        if (result.first) {
            auto context = statement.context();
            throw parse_exception(
                (boost::format("next statement cannot be used inside %1%.") % result.first).str(),
                context.begin,
                context.end
            );
        }

        if (result.second != location::lambda) {
            auto context = statement.context();
            throw parse_exception(
                "next statement must be used from within a block.",
                context.begin,
                context.end
            );
        }

        if (statement.value) {
            operator()(*statement.value);
        }
    }

    void validation::operator()(return_statement const& statement)
    {
        auto result = validate_transfer_statement(true);
        if (result.first) {
            throw parse_exception(
                (boost::format("return statement cannot be used inside %1%.") % result.first).str(),
                statement.begin,
                statement.end
            );
        }

        if (result.second == location::class_) {
            if (statement.value) {
                auto context = statement.context();
                throw parse_exception(
                    "classes cannot return a value.",
                    context.begin,
                    context.end
                );
            }
        } else if (result.second == location::defined_type) {
            if (statement.value) {
                auto context = statement.context();
                throw parse_exception(
                    "defined types cannot return a value.",
                    context.begin,
                    context.end
                );
            }
        } else if (result.second != location::function) {
            auto context = statement.context();
            throw parse_exception(
                "return statement must be used from within a function, class, or defined type.",
                context.begin,
                context.end
            );
        }

        if (statement.value) {
            operator()(*statement.value);
        }
    }

    void validation::validate_parameters(vector<parameter> const& parameters, bool is_resource, bool pass_by_hash)
    {
        utility::indirect_set<std::string> names;

        bool has_optional_parameters = false;
        for (size_t i = 0; i < parameters.size(); ++i) {
            auto const& parameter = parameters[i];

            validate_parameter_name(parameter, is_resource);

            if (!names.insert(&parameter.variable.name).second) {
                throw parse_exception(
                    (boost::format("parameter %1% already exists in the parameter list.") %
                     parameter.variable
                    ).str(),
                    parameter.variable.begin,
                    parameter.variable.end
                );
            }

            if (parameter.type) {
                parameter.type->validate_type();
            }

            if (parameter.captures) {
                if (pass_by_hash) {
                    throw parse_exception(
                        (boost::format("parameter %1% cannot be a capture parameter.") %
                         parameter.variable
                        ).str(),
                        parameter.variable.begin,
                        parameter.variable.end
                    );
                }
                if (i != (parameters.size() - 1)) {
                    throw parse_exception(
                        (boost::format("parameter %1% is a capture parameter but is not the last parameter.") %
                         parameter.variable
                        ).str(),
                        parameter.variable.begin,
                        parameter.variable.end
                    );
                }
            } else {
                if (!pass_by_hash && has_optional_parameters && !parameter.default_value) {
                    throw parse_exception(
                        (boost::format("parameter %1% is required but appears after optional parameters.") %
                         parameter.variable
                        ).str(),
                        parameter.variable.begin,
                        parameter.variable.end
                    );
                }

                has_optional_parameters = static_cast<bool>(parameter.default_value);
            }

            if (parameter.default_value) {
                // Set the range of parameters to check for illegal references
                parameter_helper helper{
                    *this,
                    parameters.data() + i + 1,
                    parameters.data() + parameters.size()
                };
                operator()(*parameter.default_value);
            }
        }
    }

    void validation::validate_parameter_name(parameter const& parameter, bool is_resource_parameter) const
    {
        static char const* valid_name_pattern = "[a-z_]\\w*";
        static const utility::regex valid_name_regex{ valid_name_pattern };

        if (!valid_name_regex.match(parameter.variable.name)) {
            throw parse_exception(
                (boost::format("parameter %1% has an unacceptable name: the name must conform to /%2%/.") %
                 parameter.variable %
                 valid_name_pattern
                ).str(),
                parameter.variable.begin,
                parameter.variable.end
            );
        }

        if (is_resource_parameter) {
            if (parameter.variable.name == "title" || parameter.variable.name == "name") {
                throw parse_exception(
                    (boost::format("parameter %1% is reserved and cannot be used.") % parameter.variable).str(),
                    parameter.variable.begin,
                    parameter.variable.end
                );
            }
            if (resource::is_metaparameter(parameter.variable.name)) {
                throw parse_exception(
                    (boost::format("parameter $%1% is reserved for resource metaparameter '%1%'.") % parameter.variable.name).str(),
                    parameter.variable.begin,
                    parameter.variable.end
                );
            }
        }
    }

    validation::location validation::current_location() const
    {
        if (_locations.empty()) {
            return location::top;
        }
        return _locations.back();
    }

    void validation::validate_body(vector<statement> const& body, bool has_return_value)
    {
        for (size_t i = 0; i < body.size(); ++i) {
            auto& statement = body[i];

            bool is_last = i == (body.size() - 1);

            // The last statement in the block is allowed to be ineffective if there is a return value
            operator()(statement, !has_return_value || !is_last);

            // Check for unreachable code following a transfer statement
            if (!is_last && statement.is_transfer_statement()) {
                auto context = body[i + 1].context();

                throw parse_exception(
                    "code is unreachable due to previous control flow statement.",
                    context.begin,
                    context.end
                );
            }
        }
    }

    void validation::validate_assignment_operand(postfix_expression const& operand)
    {
        if (operand.operations.empty()) {
            if (auto variable = boost::get<ast::variable>(&operand.operand)) {
                validate_assignment_operand(*variable);
                return;
            }
            if (auto array = boost::get<boost::spirit::x3::forward_ast<ast::array>>(&operand.operand)) {
                validate_assignment_operand(array->get());
                return;
            }
        }

        auto context = operand.context();
        throw parse_exception(
            "illegal assignment expression: assignment can only be performed on variables and arrays of variables.",
            context.begin,
            context.end
        );
    }

    void validation::validate_assignment_operand(ast::array const& operand)
    {
        for (auto const& element : operand.elements) {
            if (!element.operations.empty()) {
                auto context = element.context();
                throw parse_exception(
                    "illegal assignment expression: assignment can only be performed on variables and arrays of variables.",
                    context.begin,
                    context.end
                );
            }
            validate_assignment_operand(element.operand);
        }
    }

    void validation::validate_assignment_operand(variable const& operand)
    {
        if (operand.name.empty()) {
            throw parse_exception(
                "cannot assign to a variable with an empty name.",
                operand.begin,
                operand.end
            );
        }
        // Ensure the variable isn't a match variable
        if (isdigit(operand.name[0])) {
            throw parse_exception(
                (boost::format("cannot assign to $%1%: the name is reserved as a match variable.") %
                 operand.name
                ).str(),
                operand.begin,
                operand.end
            );
        }
        // Ensure the variable is local to the current scope
        if (operand.name.find(':') != std::string::npos) {
            throw parse_exception(
                (boost::format("cannot assign to $%1%: assignment can only be performed on variables local to the current scope.") %
                 operand.name
                ).str(),
                operand.begin,
                operand.end
            );
        }
    }

    void validation::validate_catalog_statement(ast::context const& context)
    {
        if (!_allow_catalog_statements) {
            throw parse_exception("catalog statements are not allowed.", context.begin, context.end);
        }
    }

    std::pair<char const*, validation::location> validation::validate_transfer_statement(bool ret)
    {
        // Look backward up the AST for illegal uses
        location inside = location::top;
        char const* message = nullptr;
        for (auto it = _locations.rbegin(); it != _locations.rend(); ++it) {
            auto current = *it;
            // Check if we've found the location where the statement is ultimately contained
            if ((ret && (current == location::function || current == location::class_ || current == location::defined_type)) ||
                (!ret && current == location::lambda)) {
                inside = current;
                continue;
            }

            // Check for use in illegal expressions
            if (current == location::relationship ||
                current == location::resource_declaration ||
                current == location::resource_override ||
                current == location::resource_defaults) {
                message = where(current);
                break;
            }

            // Check for evaluation inside parameters
            if (_parameter_begin) {
                message = "parameter default value";
                break;
            }

            // If we haven't discovered the containing location yet, check for illegal uses
            if (inside == location::top) {
                if (current == location::interpolated_string ||
                    (!ret && (current == location::function || current == location::class_ || current == location::defined_type)) ||
                    current == location::node ||
                    current == location::application ||
                    current == location::site) {
                    message = where(current);
                    break;
                }
            }
        }
        return make_pair(message, inside);
    }

    char const* validation::where(location l)
    {
        switch (l) {
            case location::top:
                return "the top level";

            case location::epp:
                return "an EPP template";

            case location::interpolated_string:
                return "an interpolated string";

            case location::lambda:
                return "a block";

            case location::class_:
                return "a class";

            case location::defined_type:
                return "a defined type";

            case location::node:
                return "a node statement";

            case location::function:
                return "a function statement";

            case location::application:
                return "an application statement";

            case location::site:
                return "a site statement";

            case location::relationship:
                return "a relationship expression";

            case location::resource_declaration:
                return "a resource declaration expression";

            case location::resource_override:
                return "a resource override expression";

            case location::resource_defaults:
                return "a resource defaults expression";
        }

        throw runtime_error("unexpected location type.");
    }

    validation::location_helper::location_helper(validation& visitor, location where) :
        _visitor(visitor)
    {
        _visitor._locations.push_back(where);
    }

    validation::location_helper::~location_helper()
    {
        _visitor._locations.pop_back();
    }

    validation::parameter_helper::parameter_helper(validation& visitor, parameter const* begin, parameter const* end) :
        _visitor(visitor)
    {
        _visitor._parameter_begin = begin;
        _visitor._parameter_end = end;
    }

    validation::parameter_helper::~parameter_helper()
    {
        _visitor._parameter_begin = nullptr;
        _visitor._parameter_end = nullptr;
    }

}}}}  // namespace puppet::compiler::visitors
