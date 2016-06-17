#include <puppet/compiler/ast/visitors/validation.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <puppet/compiler/resource.hpp>
#include <puppet/runtime/values/type.hpp>
#include <puppet/utility/regex.hpp>
#include <puppet/utility/indirect_collection.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;

namespace puppet { namespace compiler { namespace ast { namespace visitors {

    void validation::visit(syntax_tree const& tree, bool epp)
    {
        location_helper helper{*this, epp ? location::epp : location::top };

        if (tree.parameters) {
            validate_parameters(*tree.parameters, false, false);
        }

        for (auto const& statement : tree.statements) {
            operator()(statement);
        }
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
        location_helper helper{*this, location::case_ };

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

    void validation::operator()(if_expression const& expression)
    {
        location_helper helper{*this, location::if_ };

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

    void validation::operator()(unless_expression const& expression)
    {
        location_helper helper{*this, location::unless };

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

    void validation::operator()(function_call_expression const& expression)
    {
        for (auto const& argument : expression.arguments) {
            operator()(argument);
        }
        if (expression.lambda) {
            operator()(*expression.lambda);
        }
    }

    void validation::operator()(lambda_expression const& expression)
    {
        location_helper helper{*this, location::lambda };

        validate_parameters(expression.parameters);

        // Treat a lambda like an anonymous function
        for (auto const& statement : expression.body) {
            operator()(statement);
        }
    }

    void validation::operator()(new_expression const& expression)
    {
        operator()(expression.type);

        for (auto const& argument : expression.arguments) {
            operator()(argument);
        }
    }

    void validation::operator()(ast::epp_render_expression const& expression)
    {
        operator()(expression.expression);
    }

    void validation::operator()(ast::epp_render_block const& expression)
    {
        for (auto const& expr : expression.block) {
            operator()(expr);
        }
    }

    void validation::operator()(ast::epp_render_string const& expression)
    {
    }

    void validation::operator()(ast::unary_expression const& expression)
    {
        operator()(expression.operand);
    }

    void validation::operator()(ast::nested_expression const& expression)
    {
        operator()(expression.expression);
    }

    void validation::operator()(ast::expression const& expression)
    {
        operator()(expression.operand);

        ast::postfix_expression const* left = &expression.operand;
        for (auto const& operation : expression.operations) {
            if (operation.operator_ == binary_operator::assignment) {
                validate_assignment_operand(*left);
            }

            operator()(operation.operand);
            left = &operation.operand;
        }
    }

    void validation::operator()(ast::postfix_expression const& expression)
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
            operator()(*expression.lambda);
        }
    }

    void validation::operator()(ast::statement const& statement)
    {
        boost::apply_visitor(*this, statement);
    }

    void validation::operator()(ast::class_statement const& statement)
    {
        auto current = current_location();
        if (current != location::top && current != location::class_) {
            throw parse_exception("classes can only be defined at top-level or inside another class.", statement.begin, statement.end);
        }

        if (!is_class_name_valid(statement.name.value)) {
            throw parse_exception((boost::format("'%1%' is not a valid name for a class.") % statement.name).str(), statement.name.begin, statement.name.end);
        }

        location_helper helper{*this, location::class_ };

        validate_parameters(statement.parameters, true, false);
        for (auto const& stmt : statement.body) {
            operator()(stmt);
        }
    }

    void validation::operator()(ast::defined_type_statement const& statement)
    {
        auto current = current_location();
        if (current != location::top && current != location::class_) {
            throw parse_exception("defined types can only be defined at top-level or inside a class.", statement.begin, statement.end);
        }

        // Same rules apply to defined type names
        if (!is_class_name_valid(statement.name.value)) {
            throw parse_exception((boost::format("'%1%' is not a valid name for a defined type.") % statement.name).str(), statement.name.begin, statement.name.end);
        }

        location_helper helper{*this, location::defined_type };

        validate_parameters(statement.parameters, true, false);
        for (auto const& stmt : statement.body) {
            operator()(stmt);
        }
    }

    void validation::operator()(ast::node_statement const& statement)
    {
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

        for (auto const& stmt : statement.body) {
            operator()(stmt);
        }
    }

    void validation::operator()(ast::function_statement const& statement)
    {
        auto current = current_location();
        if (current != location::top) {
            throw parse_exception("functions can only be defined at top-level.", statement.begin, statement.end);
        }

        location_helper helper{*this, location::function };

        validate_parameters(statement.parameters);
        for (auto const& stmt : statement.body) {
            operator()(stmt);
        }
    }

    void validation::operator()(ast::produces_statement const& statement)
    {
        auto current = current_location();
        if (current != location::top && current != location::site) {
            auto context = statement.context();
            throw parse_exception("capability mappings can only be defined at top-level or inside a site.", context.begin, context.end);
        }

        for (auto const& operation : statement.operations) {
            operator()(operation.value);
        }
    }

    void validation::operator()(ast::consumes_statement const& statement)
    {
        auto current = current_location();
        if (current != location::top && current != location::site) {
            auto context = statement.context();
            throw parse_exception("capability mappings can only be defined at top-level or inside a site.", context.begin, context.end);
        }

        for (auto const& operation : statement.operations) {
            operator()(operation.value);
        }
    }

    void validation::operator()(ast::application_statement const& statement)
    {
        auto current = current_location();
        if (current != location::top) {
            throw parse_exception("applications can only be defined at top-level.", statement.begin, statement.end);
        }

        location_helper helper{*this, location::application };

        validate_parameters(statement.parameters, true, false);
        for (auto const& stmt : statement.body) {
            operator()(stmt);
        }
    }

    void validation::operator()(ast::site_statement const& statement)
    {
        auto current = current_location();
        if (current != location::top) {
            throw parse_exception("sites can only be defined at top-level.", statement.begin, statement.end);
        }

        location_helper helper{*this, location::site };

        for (auto const& stmt : statement.body) {
            operator()(stmt);
        }
    }

    void validation::operator()(ast::type_alias_statement const& statement)
    {
        auto current = current_location();
        if (current != location::top) {
            auto context = statement.context();
            throw parse_exception("type aliases can only be defined at top-level.", context.begin, context.end);
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

    void validation::operator()(ast::function_call_statement const& statement)
    {
        for (auto const& argument : statement.arguments) {
            operator()(argument);
        }
        if (statement.lambda) {
            operator()(*statement.lambda);
        }
    }

    void validation::operator()(ast::relationship_statement const& statement)
    {
        operator()(statement.operand);

        for (auto const& operation : statement.operations) {
            operator()(operation.operand);
        }
    }

    void validation::operator()(ast::relationship_expression const& expression)
    {
        boost::apply_visitor(*this, expression);
    }

    void validation::operator()(ast::resource_declaration_expression const& expression)
    {
        for (auto const& body : expression.bodies) {
            operator()(body.title);

            for (auto const& operation : body.operations) {
                operator()(operation.value);
            }
        }
    }

    void validation::operator()(ast::resource_override_expression const& expression)
    {
        boost::apply_visitor(*this, expression.reference);

        for (auto const& operation : expression.operations) {
            operator()(operation.value);
        }
    }

    void validation::operator()(ast::resource_defaults_expression const& expression)
    {
        for (auto const& operation : expression.operations) {
            operator()(operation.value);
        }
    }

    void validation::operator()(ast::collector_expression const& expression)
    {
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

    bool validation::is_class_name_valid(std::string const& name) const
    {
        // Don't allow empty names, non-local names, or names of built-in classes
        return !name.empty() &&
               !boost::starts_with(name, "::") &&
               !(current_location() == location::top && (name == "main" || name == "settings"));
    }

    void validation::validate_parameters(vector<parameter> const& parameters, bool is_resource, bool allow_capture)
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
                if (i != (parameters.size() - 1)) {
                    throw parse_exception(
                        (boost::format("parameter %1% is a capture parameter but is not the last parameter.") %
                         parameter.variable
                        ).str(),
                        parameter.variable.begin,
                        parameter.variable.end
                    );
                }
                if (!allow_capture) {
                    throw parse_exception(
                        (boost::format("parameter %1% cannot be a capture parameter.") %
                         parameter.variable
                        ).str(),
                        parameter.variable.begin,
                        parameter.variable.end
                    );
                }
            } else {
                if (has_optional_parameters && !parameter.default_value) {
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
