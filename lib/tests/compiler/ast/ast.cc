#include <catch.hpp>
#include <puppet/compiler/ast/ast.hpp>
#include <puppet/cast.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace puppet::compiler::ast;
using namespace puppet::compiler::lexer;
using puppet::rvalue_cast;
using boost::lexical_cast;

namespace ast = puppet::compiler::ast;

namespace puppet { namespace compiler {
    struct module;
}}  // namespace puppet::compiler

position create_position()
{
    static size_t counter = 0;
    auto offset = counter++;
    auto line = counter++;
    return position{ offset, line };
}

syntax_tree* create_dummy_tree()
{
    static size_t counter = 0;
    return reinterpret_cast<syntax_tree*>(++counter);
}

puppet::compiler::module* create_dummy_module()
{
    static size_t counter = 0;
    return reinterpret_cast<puppet::compiler::module*>(++counter);
}

void set_dummy_context(ast::context& context)
{
    context.begin = create_position();
    context.end = create_position();
    context.tree = create_dummy_tree();
}

static undef create_undef()
{
    undef node;
    set_dummy_context(node);
    return node;
}

static defaulted create_default()
{
    defaulted node;
    set_dummy_context(node);
    return node;
}

static boolean create_boolean(bool value)
{
    boolean node;
    set_dummy_context(node);
    node.value = value;
    return node;
}

static number create_number(int64_t value)
{
    number node;
    set_dummy_context(node);
    node.value = value;
    return node;
}

static ast::string create_string(std::string value, std::string format = {}, size_t margin = 0)
{
    ast::string node;
    set_dummy_context(node);
    node.value = rvalue_cast(value);
    node.format = rvalue_cast(format);
    node.margin = margin;
    return node;
}

static literal_string_text create_literal_string_text(std::string text)
{
    literal_string_text node;
    set_dummy_context(node);
    node.text = rvalue_cast(text);
    return node;
}

template <typename T>
static interpolated_string_part string_part(T&& node)
{
    return interpolated_string_part(std::forward<T>(node));
}

static ast::interpolated_string create_interpolated_string(vector<interpolated_string_part> parts, std::string format = {}, size_t margin = 0)
{
    ast::interpolated_string node;
    set_dummy_context(node);
    node.parts = rvalue_cast(parts);
    node.format = rvalue_cast(format);
    node.margin = margin;
    return node;
}

static ast::regex create_regex(std::string value)
{
    ast::regex node;
    set_dummy_context(node);
    node.value = rvalue_cast(value);
    return node;
}

static variable create_variable(std::string name)
{
    variable node;
    set_dummy_context(node);
    node.name = rvalue_cast(name);
    return node;
}

static name create_name(std::string value)
{
    name node;
    set_dummy_context(node);
    node.value = rvalue_cast(value);
    return node;
}

static bare_word create_bare_word(std::string value)
{
    bare_word node;
    set_dummy_context(node);
    node.value = rvalue_cast(value);
    return node;
}

static type create_type(std::string name)
{
    type node;
    set_dummy_context(node);
    node.name = rvalue_cast(name);
    return node;
}

static ast::array create_array(vector<expression> elements = {})
{
    ast::array node;
    set_dummy_context(node);
    node.elements = rvalue_cast(elements);
    return node;
}

static ast::hash create_hash(vector<ast::pair> elements = {})
{
    ast::hash node;
    set_dummy_context(node);
    node.elements = rvalue_cast(elements);
    return node;
}

static proposition create_proposition(vector<expression> options, vector<expression> body)
{
    proposition node;
    node.options = rvalue_cast(options);
    node.body = rvalue_cast(body);
    node.end = create_position();
    return node;
}

static case_expression create_case(expression conditional, vector<proposition> propositions)
{
    case_expression node;
    set_dummy_context(node);
    node.conditional = rvalue_cast(conditional);
    node.propositions = rvalue_cast(propositions);
    return node;
}

static else_ create_else(vector<expression> body = {})
{
    else_ node;
    node.begin = create_position();
    node.end = create_position();
    node.body = rvalue_cast(body);
    return node;
}

static elsif create_elsif(expression conditional, vector<expression> body = {})
{
    elsif node;
    node.begin = create_position();
    node.end = create_position();
    node.conditional = rvalue_cast(conditional);
    node.body = rvalue_cast(body);
    return node;
}

static if_expression create_if(expression conditional, vector<expression> body = {}, vector<elsif> elsifs = {}, boost::optional<ast::else_> else_ = boost::none)
{
    if_expression node;
    node.begin = create_position();
    node.end = create_position();
    node.conditional = rvalue_cast(conditional);
    node.body = rvalue_cast(body);
    node.elsifs = rvalue_cast(elsifs);
    node.else_ = rvalue_cast(else_);
    return node;
}

static unless_expression create_unless(expression conditional, vector<expression> body = {}, boost::optional<ast::else_> else_ = boost::none)
{
    unless_expression node;
    node.begin = create_position();
    node.end = create_position();
    node.conditional = rvalue_cast(conditional);
    node.body = rvalue_cast(body);
    node.else_ = rvalue_cast(else_);
    return node;
}

static function_call_expression create_function_call(std::string function, vector<expression> arguments = {}, boost::optional<lambda_expression> lambda = boost::none)
{
    function_call_expression node;
    node.function = create_name(rvalue_cast(function));
    node.arguments = rvalue_cast(arguments);
    node.lambda = rvalue_cast(lambda);
    node.end = create_position();
    return node;
}

static attribute_operation create_attribute(std::string name, attribute_operator operator_, expression value)
{
    attribute_operation node;
    node.name = create_name(rvalue_cast(name));
    node.operator_position = create_position();
    node.operator_ = operator_;
    node.value = rvalue_cast(value);
    return node;
}

static resource_body create_resource_body(expression title, vector<attribute_operation> operations = {})
{
    resource_body node;
    node.title = rvalue_cast(title);
    node.operations = rvalue_cast(operations);
    return node;
}

static resource_expression create_resource(resource_status status, postfix_expression type, vector<resource_body> bodies)
{
    resource_expression node;
    set_dummy_context(node);
    node.status = status;
    node.type = rvalue_cast(type);
    node.bodies = rvalue_cast(bodies);
    return node;
}

static resource_override_expression create_resource_override(postfix_expression reference, vector<attribute_operation> operations = {})
{
    resource_override_expression node;
    set_dummy_context(node);
    node.reference = rvalue_cast(reference);
    node.operations = rvalue_cast(operations);
    return node;
}

static resource_defaults_expression create_resource_defaults(std::string type, vector<attribute_operation> operations = {})
{
    resource_defaults_expression node;
    set_dummy_context(node);
    node.type = create_type(type);
    node.operations = rvalue_cast(operations);
    return node;
}

static parameter create_parameter(std::string name, boost::optional<postfix_expression> type = boost::none, bool captures = false, boost::optional<expression> default_value = boost::none)
{
    parameter node;
    node.type = rvalue_cast(type);
    if (captures) {
        node.captures = create_position();
    }
    node.variable = create_variable(name);
    node.default_value = rvalue_cast(default_value);
    return node;
}

static class_expression create_class(std::string name, vector<parameter> parameters = {}, boost::optional<ast::name> parent = boost::none, vector<expression> body = {})
{
    class_expression node;
    set_dummy_context(node);
    node.name = create_name(rvalue_cast(name));
    node.parameters = rvalue_cast(parameters);
    node.parent = rvalue_cast(parent);
    node.body = rvalue_cast(body);
    return node;
}

static defined_type_expression create_defined_type(std::string name, vector<parameter> parameters = {}, vector<expression> body = {})
{
    defined_type_expression node;
    set_dummy_context(node);
    node.name = create_name(rvalue_cast(name));
    node.parameters = rvalue_cast(parameters);
    node.body = rvalue_cast(body);
    return node;
}

static node_expression create_node(vector<hostname> hostnames, vector<expression> body = {})
{
    node_expression node;
    set_dummy_context(node);
    node.hostnames = rvalue_cast(hostnames);
    node.body = rvalue_cast(body);
    return node;
}

static attribute_query create_attribute_query(std::string name, query_operator operator_, primary_expression value)
{
    attribute_query node;
    node.attribute = create_name(name);
    node.operator_position = create_position();
    node.operator_ = operator_;
    node.value = rvalue_cast(value);
    return node;
}

template <typename T>
primary_query_expression primary_query(T&& node)
{
    return primary_query_expression(std::forward<T>(node));
}

static binary_query_operation create_binary_query(binary_query_operator operator_, primary_query_expression operand)
{
    binary_query_operation node;
    node.operator_position = create_position();
    node.operator_ = operator_;
    node.operand = rvalue_cast(operand);
    return node;
}

static query_expression create_query(primary_query_expression primary, vector<binary_query_operation> operations = {})
{
    query_expression node;
    node.primary = rvalue_cast(primary);
    node.operations = rvalue_cast(operations);
    return node;
}

static nested_query_expression create_nested_query(ast::query_expression expression)
{
    nested_query_expression node;
    set_dummy_context(node);
    node.expression = rvalue_cast(expression);
    return node;
}

static collector_expression create_collector(std::string type, bool exported = false, boost::optional<query_expression> query = boost::none)
{
    collector_expression node;
    node.end = create_position();
    node.type = create_type(type);
    node.exported = exported;
    node.query = rvalue_cast(query);
    return node;
}

static epp_render_expression create_render_expression(ast::expression expression)
{
    epp_render_expression node;
    set_dummy_context(node);
    node.expression = rvalue_cast(expression);
    return node;
}

static epp_render_block create_render_block(vector<ast::expression> block)
{
    epp_render_block node;
    set_dummy_context(node);
    node.block = rvalue_cast(block);
    return node;
}

static epp_render_string create_render_string(std::string string)
{
    epp_render_string node;
    set_dummy_context(node);
    node.string = rvalue_cast(string);
    return node;
}

static function_expression create_function(std::string name, vector<parameter> parameters = {}, vector<expression> body = {})
{
    function_expression node;
    set_dummy_context(node);
    node.name = create_name(rvalue_cast(name));
    node.parameters = rvalue_cast(parameters);
    node.body = rvalue_cast(body);
    return node;
}

static produces_expression create_produces(std::string resource, std::string capability, vector<attribute_operation> operations = {})
{
    produces_expression node;
    node.resource = create_type(rvalue_cast(resource));
    node.capability = create_type(rvalue_cast(capability));
    node.operations = rvalue_cast(operations);
    node.end = create_position();
    return node;
}

static consumes_expression create_consumes(std::string resource, std::string capability, vector<attribute_operation> operations = {})
{
    consumes_expression node;
    node.resource = create_type(rvalue_cast(resource));
    node.capability = create_type(rvalue_cast(capability));
    node.operations = rvalue_cast(operations);
    node.end = create_position();
    return node;
}

static application_expression create_application(std::string name, vector<parameter> parameters = {}, vector<expression> body = {})
{
    application_expression node;
    set_dummy_context(node);
    node.name = create_name(rvalue_cast(name));
    node.parameters = rvalue_cast(parameters);
    node.body = rvalue_cast(body);
    return node;
}

static site_expression create_site(vector<expression> body = {})
{
    site_expression node;
    set_dummy_context(node);
    node.body = rvalue_cast(body);
    return node;
}

static unary_expression create_unary(unary_operator operator_, postfix_expression operand)
{
    unary_expression node;
    node.operator_position = create_position();
    node.operator_ = operator_;
    node.operand = rvalue_cast(operand);
    return node;
}

static nested_expression create_nested(ast::expression expression)
{
    nested_expression node;
    set_dummy_context(node);
    node.expression = rvalue_cast(expression);
    return node;
}

template <typename T>
static postfix_subexpression subexpression(T&& node)
{
    return postfix_subexpression(std::forward<T>(node));
}

static selector_expression create_selector(vector<ast::pair> cases)
{
    selector_expression node;
    set_dummy_context(node);
    node.cases = rvalue_cast(cases);
    return node;
}

static access_expression create_access(vector<expression> arguments)
{
    access_expression node;
    set_dummy_context(node);
    node.arguments = rvalue_cast(arguments);
    return node;
}

static lambda_expression create_lambda(vector<parameter> parameters = {}, vector<expression> body = {})
{
    lambda_expression node;
    set_dummy_context(node);
    node.parameters = rvalue_cast(parameters);
    node.body = rvalue_cast(body);
    return node;
}

static method_call_expression create_method_call(std::string method, vector<expression> arguments = {}, boost::optional<lambda_expression> lambda = boost::none)
{
    method_call_expression node;
    node.begin = create_position();
    node.end = create_position();
    node.method = create_name(method);
    node.arguments = rvalue_cast(arguments);
    node.lambda = rvalue_cast(lambda);
    return node;
}

static postfix_expression create_postfix(primary_expression primary, vector<postfix_subexpression> subexpressions = {})
{
    postfix_expression node;
    node.primary = rvalue_cast(primary);
    node.subexpressions = rvalue_cast(subexpressions);
    return node;
}

static expression create_expression(primary_expression primary)
{
    expression node;
    node.first = create_postfix(rvalue_cast(primary));
    return node;
}

static binary_operation create_binary(binary_operator operator_, postfix_expression operand)
{
    binary_operation node;
    node.operator_position = create_position();
    node.operator_ = operator_;
    node.operand = rvalue_cast(operand);
    return node;
}

static expression create_expression(postfix_expression postfix, vector<binary_operation> operations = {})
{
    expression node;
    node.first = rvalue_cast(postfix);
    node.operations = rvalue_cast(operations);
    return node;
}

template <typename T>
static primary_expression primary(T&& node)
{
    return primary_expression(std::forward<T>(node));
}

static shared_ptr<syntax_tree> create_syntax_tree(std::string path, boost::optional<vector<parameter>> parameters = boost::none, vector<expression> statements = {})
{
    auto tree = syntax_tree::create(rvalue_cast(path), create_dummy_module());
    tree->parameters = rvalue_cast(parameters);
    tree->statements = rvalue_cast(statements);
    return tree;
}

SCENARIO("undef", "[ast]")
{
    auto node = create_undef();

    WHEN("using undef") {
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2 == node);
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3 == node);
        }
        THEN("it should be convertible to context") {
            context ctx = node;
            REQUIRE(ctx == node);
        }
        THEN("it should output the expected format") {
            REQUIRE(lexical_cast<std::string>(node) == "undef");
        }
        GIVEN("another non-equal object") {
            decltype(node) other;
            THEN("the objects should not be equal") {
                REQUIRE(node != other);
            }
        }
    }
}

SCENARIO("default", "[ast]")
{
    auto node = create_default();

    WHEN("using defaulted") {
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2 == node);
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3 == node);
        }
        THEN("it should be convertible to context") {
            context ctx = node;
            REQUIRE(ctx == node);
        }
        THEN("it should output the expected format") {
            REQUIRE(lexical_cast<std::string>(node) == "default");
        }
        GIVEN("another non-equal object") {
            decltype(node) other;
            THEN("the objects should not be equal") {
                REQUIRE(node != other);
            }
        }
    }
}

SCENARIO("boolean", "[ast]")
{
    auto node = create_boolean(true);

    WHEN("using boolean") {
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2 == node);
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3 == node);
        }
        THEN("it should be convertible to context") {
            context ctx = node;
            REQUIRE(ctx == node);
        }
        THEN("it should output the expected format") {
            REQUIRE(lexical_cast<std::string>(node) == "true");
            node.value = false;
            REQUIRE(lexical_cast<std::string>(node) == "false");
        }
        GIVEN("another non-equal object") {
            decltype(node) other;
            THEN("the objects should not be equal") {
                REQUIRE(node != other);
            }
        }
    }
}

SCENARIO("number", "[ast]")
{
    auto node = create_number(1234);

    WHEN("using number") {
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2 == node);
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3 == node);
        }
        THEN("it should be convertible to context") {
            context ctx = node;
            REQUIRE(ctx == node);
        }
        THEN("it should output the expected format") {
            REQUIRE(lexical_cast<std::string>(node) == "1234");
        }
        THEN("it should be assignable from double") {
            node.value = 12.34;
            REQUIRE(lexical_cast<std::string>(node) == "12.34");
        }
        GIVEN("another non-equal object") {
            decltype(node) other;
            THEN("the objects should not be equal") {
                REQUIRE(node != other);
            }
        }
    }
}

SCENARIO("string", "[ast]")
{
    auto node = create_string("hello");

    WHEN("using string") {
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2 == node);
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3 == node);
        }
        THEN("it should be convertible to context") {
            context ctx = node;
            REQUIRE(ctx == node);
        }
        THEN("it should output the expected format") {
            REQUIRE(lexical_cast<std::string>(node) == "'hello'");
        }
        WHEN("the value contains a quote") {
            node.value = "hello'world";
            THEN("it should be escaped when output") {
                REQUIRE(lexical_cast<std::string>(node) == "'hello\\'world'");
            }
        }
        GIVEN("another non-equal object") {
            decltype(node) other;
            THEN("the objects should not be equal") {
                REQUIRE(node != other);
            }
        }
    }
}

SCENARIO("interpolated string", "[ast]")
{
    auto node = create_interpolated_string({
        string_part(create_literal_string_text("hello \"")),
        string_part(create_variable("world")),
        string_part(create_literal_string_text("\"\n1 + 1 = ")),
        string_part(create_expression(
            create_postfix(primary(create_number(1))),
                {
                    create_binary(
                        binary_operator::plus,
                        create_postfix(primary(create_number(1)))
                    )
                }
            )
        )
    });

    WHEN("using interpolated string") {
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2 == node);
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3 == node);
        }
        THEN("it should be convertible to context") {
            context ctx = node;
            REQUIRE(ctx == node);
        }
        THEN("it should output the expected format") {
            REQUIRE(lexical_cast<std::string>(node) == "\"hello \\\"$world\\\"\\n1 + 1 = ${1 + 1}\"");
        }
        GIVEN("another non-equal object") {
            decltype(node) other;
            THEN("the objects should not be equal") {
                REQUIRE(node != other);
            }
        }
    }
}

SCENARIO("regex", "[ast]")
{
    auto node = create_regex("^foo.*bar$");

    WHEN("using regex") {
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2 == node);
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3 == node);
        }
        THEN("it should be convertible to context") {
            context ctx = node;
            REQUIRE(ctx == node);
        }
        THEN("it should output the expected format") {
            REQUIRE(lexical_cast<std::string>(node) == "/^foo.*bar$/");
        }
        GIVEN("another non-equal object") {
            decltype(node) other;
            THEN("the objects should not be equal") {
                REQUIRE(node != other);
            }
        }
    }
}

SCENARIO("variable", "[ast]")
{
    auto node = create_variable("foo");

    WHEN("using variable") {
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2 == node);
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3 == node);
        }
        THEN("it should be convertible to context") {
            context ctx = node;
            REQUIRE(ctx == node);
        }
        THEN("it should output the expected format") {
            REQUIRE(lexical_cast<std::string>(node) == "$foo");
        }
        GIVEN("another non-equal object") {
            decltype(node) other;
            THEN("the objects should not be equal") {
                REQUIRE(node != other);
            }
        }
    }
}

SCENARIO("name", "[ast]")
{
    auto node = create_name("foo::bar");

    WHEN("using name") {
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2 == node);
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3 == node);
        }
        THEN("it should be convertible to context") {
            context ctx = node;
            REQUIRE(ctx == node);
        }
        THEN("it should output the expected format") {
            REQUIRE(lexical_cast<std::string>(node) == "foo::bar");
        }
        GIVEN("another non-equal object") {
            decltype(node) other;
            THEN("the objects should not be equal") {
                REQUIRE(node != other);
            }
        }
    }
}

SCENARIO("bare word", "[ast]")
{
    auto node = create_bare_word("foo");

    WHEN("using bare word") {
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2 == node);
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3 == node);
        }
        THEN("it should be convertible to context") {
            context ctx = node;
            REQUIRE(ctx == node);
        }
        THEN("it should output the expected format") {
            REQUIRE(lexical_cast<std::string>(node) == "foo");
        }
        GIVEN("another non-equal object") {
            decltype(node) other;
            THEN("the objects should not be equal") {
                REQUIRE(node != other);
            }
        }
    }
}

SCENARIO("type", "[ast]")
{
    auto node = create_type("Foo::Bar");

    WHEN("using type") {
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2 == node);
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3 == node);
        }
        THEN("it should be convertible to context") {
            context ctx = node;
            REQUIRE(ctx == node);
        }
        THEN("it should output the expected format") {
            REQUIRE(lexical_cast<std::string>(node) == "Foo::Bar");
        }
        GIVEN("another non-equal object") {
            decltype(node) other;
            THEN("the objects should not be equal") {
                REQUIRE(node != other);
            }
        }
    }
}

SCENARIO("primary expression", "[ast]")
{
    primary_expression node;
    THEN("it should have the expected number of types") {
        REQUIRE(boost::mpl::size<primary_expression::types>::value == 34);
    }
    WHEN("using a primary expression") {
        GIVEN("an undef") {
            auto subnode = create_undef();
            node = subnode;
            THEN("the same context should be returned") {
                REQUIRE(node.context() == subnode);
            }
            THEN("it should not be default") {
                REQUIRE_FALSE(node.is_default());
            }
            THEN("it should not be productive") {
                REQUIRE_FALSE(node.is_productive());
            }
            THEN("it should not be a splat expression") {
                REQUIRE_FALSE(node.is_splat());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "undef");
            }
        }
        GIVEN("a default") {
            auto subnode = create_default();
            node = subnode;
            THEN("the same context should be returned") {
                REQUIRE(node.context() == subnode);
            }
            THEN("it should be default") {
                REQUIRE(node.is_default());
            }
            THEN("it should not be productive") {
                REQUIRE_FALSE(node.is_productive());
            }
            THEN("it should not be a splat expression") {
                REQUIRE_FALSE(node.is_splat());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "default");
            }
        }
        GIVEN("a boolean") {
            auto subnode = create_boolean(true);
            node = subnode;
            THEN("the same context should be returned") {
                REQUIRE(node.context() == subnode);
            }
            THEN("it should not be default") {
                REQUIRE_FALSE(node.is_default());
            }
            THEN("it should not be productive") {
                REQUIRE_FALSE(node.is_productive());
            }
            THEN("it should not be a splat expression") {
                REQUIRE_FALSE(node.is_splat());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "true");
            }
        }
        GIVEN("a number") {
            auto subnode = create_number(1234);
            node = subnode;
            THEN("the same context should be returned") {
                REQUIRE(node.context() == subnode);
            }
            THEN("it should not be default") {
                REQUIRE_FALSE(node.is_default());
            }
            THEN("it should not be productive") {
                REQUIRE_FALSE(node.is_productive());
            }
            THEN("it should not be a splat expression") {
                REQUIRE_FALSE(node.is_splat());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "1234");
            }
        }
        GIVEN("a string") {
            auto subnode = create_string("foobar");
            node = subnode;
            THEN("the same context should be returned") {
                REQUIRE(node.context() == subnode);
            }
            THEN("it should not be default") {
                REQUIRE_FALSE(node.is_default());
            }
            THEN("it should not be productive") {
                REQUIRE_FALSE(node.is_productive());
            }
            THEN("it should not be a splat expression") {
                REQUIRE_FALSE(node.is_splat());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "'foobar'");
            }
        }
        GIVEN("a regex") {
            auto subnode = create_regex("^foo.*bar$");
            node = subnode;
            THEN("the same context should be returned") {
                REQUIRE(node.context() == subnode);
            }
            THEN("it should not be default") {
                REQUIRE_FALSE(node.is_default());
            }
            THEN("it should not be productive") {
                REQUIRE_FALSE(node.is_productive());
            }
            THEN("it should not be a splat expression") {
                REQUIRE_FALSE(node.is_splat());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "/^foo.*bar$/");
            }
        }
        GIVEN("a variable") {
            auto subnode = create_variable("foo");
            node = subnode;
            THEN("the same context should be returned") {
                REQUIRE(node.context() == subnode);
            }
            THEN("it should not be default") {
                REQUIRE_FALSE(node.is_default());
            }
            THEN("it should not be productive") {
                REQUIRE_FALSE(node.is_productive());
            }
            THEN("it should not be a splat expression") {
                REQUIRE_FALSE(node.is_splat());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "$foo");
            }
        }
        GIVEN("a name") {
            auto subnode = create_name("foobar");
            node = subnode;
            THEN("the same context should be returned") {
                REQUIRE(node.context() == subnode);
            }
            THEN("it should not be default") {
                REQUIRE_FALSE(node.is_default());
            }
            THEN("it should not be productive") {
                REQUIRE_FALSE(node.is_productive());
            }
            THEN("it should not be a splat expression") {
                REQUIRE_FALSE(node.is_splat());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "foobar");
            }
        }
        GIVEN("a bare word") {
            auto subnode = create_bare_word("foobar");
            node = subnode;
            THEN("the same context should be returned") {
                REQUIRE(node.context() == subnode);
            }
            THEN("it should not be default") {
                REQUIRE_FALSE(node.is_default());
            }
            THEN("it should not be productive") {
                REQUIRE_FALSE(node.is_productive());
            }
            THEN("it should not be a splat expression") {
                REQUIRE_FALSE(node.is_splat());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "foobar");
            }
        }
        GIVEN("a type") {
            auto subnode = create_type("Foo::Bar");
            node = subnode;
            THEN("the same context should be returned") {
                REQUIRE(node.context() == subnode);
            }
            THEN("it should not be default") {
                REQUIRE_FALSE(node.is_default());
            }
            THEN("it should not be productive") {
                REQUIRE_FALSE(node.is_productive());
            }
            THEN("it should not be a splat expression") {
                REQUIRE_FALSE(node.is_splat());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "Foo::Bar");
            }
        }
        GIVEN("a nested unproductive expression") {
            auto subnode = create_nested(create_expression(primary(create_variable("foo"))));
            node = subnode;
            THEN("the same context should be returned") {
                REQUIRE(node.context() == subnode);
            }
            THEN("it should not be default") {
                REQUIRE_FALSE(node.is_default());
            }
            THEN("it should not be productive") {
                REQUIRE_FALSE(node.is_productive());
            }
            THEN("it should not be a splat expression") {
                REQUIRE_FALSE(node.is_splat());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "($foo)");
            }
        }
        GIVEN("a nested productive expression") {
            auto subnode = create_nested(create_expression(primary(create_function_call("notice"))));
            node = subnode;
            THEN("it should be productive") {
                REQUIRE(node.is_productive());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "(notice())");
            }
        }
        GIVEN("a nested expression that is default") {
            auto subnode = create_nested(create_expression(primary(create_default())));
            node = subnode;
            THEN("it should be default") {
                REQUIRE(node.is_default());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "(default)");
            }
        }
        GIVEN("a nested unproductive unary expression") {
            auto subnode = create_unary(unary_operator::logical_not, create_postfix(primary(create_variable("foo"))));
            node = subnode;
            THEN("it should not be productive") {
                REQUIRE_FALSE(node.is_productive());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "!$foo");
            }
        }
        GIVEN("a nested productive unary expression") {
            auto subnode = create_unary(unary_operator::negate, create_postfix(primary(create_function_call("notice"))));
            node = subnode;
            THEN("it should be productive") {
                REQUIRE(node.is_productive());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "-notice()");
            }
        }
        GIVEN("an array") {
            auto subnode = create_array({
                create_expression(primary(create_number(1))),
                create_expression(primary(create_number(2))),
                create_expression(primary(create_number(3)))
            });
            node = subnode;
            THEN("the same context should be returned") {
                REQUIRE(node.context() == subnode);
            }
            THEN("it should not be default") {
                REQUIRE_FALSE(node.is_default());
            }
            THEN("it should not be productive") {
                REQUIRE_FALSE(node.is_productive());
            }
            THEN("it should not be a splat expression") {
                REQUIRE_FALSE(node.is_splat());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "[1, 2, 3]");
            }
        }
        GIVEN("a hash") {
            auto subnode = create_hash({
                make_pair(create_expression(primary(create_string("foo"))), create_expression(primary(create_number(1)))),
                make_pair(create_expression(primary(create_string("bar"))), create_expression(primary(create_number(2)))),
                make_pair(create_expression(primary(create_string("baz"))), create_expression(primary(create_number(3))))
            });
            node = subnode;
            THEN("the same context should be returned") {
                REQUIRE(node.context() == subnode);
            }
            THEN("it should not be default") {
                REQUIRE_FALSE(node.is_default());
            }
            THEN("it should not be productive") {
                REQUIRE_FALSE(node.is_productive());
            }
            THEN("it should not be a splat expression") {
                REQUIRE_FALSE(node.is_splat());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "{'foo' => 1, 'bar' => 2, 'baz' => 3}");
            }
        }
        GIVEN("a case expression") {
            auto subnode = create_case(
                create_expression(primary(create_variable("foo"))),
                {
                    create_proposition(
                    {
                        create_expression(primary(create_default()))
                    },
                    {
                    })
                });
            node = subnode;
            THEN("the same context should be returned") {
                REQUIRE(node.context() == subnode);
            }
            THEN("it should not be default") {
                REQUIRE_FALSE(node.is_default());
            }
            THEN("it should be productive") {
                REQUIRE(node.is_productive());
            }
            THEN("it should not be a splat expression") {
                REQUIRE_FALSE(node.is_splat());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "case $foo { default: { } }");
            }
        }
        GIVEN("an if expression") {
            auto subnode = create_if(
                create_expression(primary(create_variable("foo"))),
                {},
                {
                    create_elsif(create_expression(primary(create_variable("bar"))))
                },
                create_else()
            );
            node = subnode;
            THEN("the same context should be returned") {
                REQUIRE(node.context() == subnode.context());
            }
            THEN("it should not be default") {
                REQUIRE_FALSE(node.is_default());
            }
            THEN("it should be productive") {
                REQUIRE(node.is_productive());
            }
            THEN("it should not be a splat expression") {
                REQUIRE_FALSE(node.is_splat());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "if $foo { } elsif $bar { } else { }");
            }
        }
        GIVEN("an unless expression") {
            auto subnode = create_unless(
                create_expression(primary(create_variable("foo"))),
                {},
                create_else()
            );
            node = subnode;
            THEN("the same context should be returned") {
                REQUIRE(node.context() == subnode.context());
            }
            THEN("it should not be default") {
                REQUIRE_FALSE(node.is_default());
            }
            THEN("it should be productive") {
                REQUIRE(node.is_productive());
            }
            THEN("it should not be a splat expression") {
                REQUIRE_FALSE(node.is_splat());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "unless $foo { } else { }");
            }
        }
        GIVEN("a function call expression") {
            auto subnode = create_function_call("notice");
            node = subnode;
            THEN("the same context should be returned") {
                REQUIRE(node.context() == subnode.context());
            }
            THEN("it should not be default") {
                REQUIRE_FALSE(node.is_default());
            }
            THEN("it should be productive") {
                REQUIRE(node.is_productive());
            }
            THEN("it should not be a splat expression") {
                REQUIRE_FALSE(node.is_splat());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "notice()");
            }
        }
        GIVEN("a resource expression") {
            auto subnode = create_resource(
                resource_status::virtualized,
                create_postfix(primary(create_name("foo"))),
                {
                    create_resource_body(
                        create_expression(primary(create_name("bar"))),
                        {
                            create_attribute(
                                "baz",
                                attribute_operator::assignment,
                                create_expression(primary(create_string("jam")))
                            )
                        }
                    )
                }
            );
            node = subnode;
            THEN("the same context should be returned") {
                REQUIRE(node.context() == subnode);
            }
            THEN("it should not be default") {
                REQUIRE_FALSE(node.is_default());
            }
            THEN("it should be productive") {
                REQUIRE(node.is_productive());
            }
            THEN("it should not be a splat expression") {
                REQUIRE_FALSE(node.is_splat());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "@foo { bar: baz => 'jam' }");
            }
        }
        GIVEN("a resource override expression") {
            auto subnode = create_resource_override(
                create_postfix(primary(create_variable("foo"))),
                {
                    create_attribute(
                        "baz",
                        attribute_operator::assignment,
                        create_expression(primary(create_string("jam")))
                    )
                }
            );
            node = subnode;
            THEN("the same context should be returned") {
                REQUIRE(node.context() == subnode);
            }
            THEN("it should not be default") {
                REQUIRE_FALSE(node.is_default());
            }
            THEN("it should be productive") {
                REQUIRE(node.is_productive());
            }
            THEN("it should not be a splat expression") {
                REQUIRE_FALSE(node.is_splat());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "$foo { baz => 'jam' }");
            }
        }
        GIVEN("a resource defaults expression") {
            auto subnode = create_resource_defaults(
                "Foo::Bar",
                {
                    create_attribute(
                        "baz",
                        attribute_operator::assignment,
                        create_expression(primary(create_string("jam")))
                    )
                }
            );
            node = subnode;
            THEN("the same context should be returned") {
                REQUIRE(node.context() == subnode);
            }
            THEN("it should not be default") {
                REQUIRE_FALSE(node.is_default());
            }
            THEN("it should be productive") {
                REQUIRE(node.is_productive());
            }
            THEN("it should not be a splat expression") {
                REQUIRE_FALSE(node.is_splat());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "Foo::Bar { baz => 'jam' }");
            }
        }
        GIVEN("a class expression") {
            auto subnode = create_class(
                "foo",
                {
                    create_parameter(
                        "bar",
                        boost::none,
                        true,
                        create_expression(primary(create_array()))
                    )
                },
                create_name("baz")
            );
            node = subnode;
            THEN("the same context should be returned") {
                REQUIRE(node.context() == subnode);
            }
            THEN("it should not be default") {
                REQUIRE_FALSE(node.is_default());
            }
            THEN("it should be productive") {
                REQUIRE(node.is_productive());
            }
            THEN("it should not be a splat expression") {
                REQUIRE_FALSE(node.is_splat());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "class foo(*$bar = []) inherits baz { }");
            }
        }
        GIVEN("a defined type expression") {
            auto subnode = create_defined_type(
                "foo",
                {
                    create_parameter(
                        "bar",
                        boost::none,
                        true,
                        create_expression(primary(create_array()))
                    )
                }
            );
            node = subnode;
            THEN("the same context should be returned") {
                REQUIRE(node.context() == subnode);
            }
            THEN("it should not be default") {
                REQUIRE_FALSE(node.is_default());
            }
            THEN("it should be productive") {
                REQUIRE(node.is_productive());
            }
            THEN("it should not be a splat expression") {
                REQUIRE_FALSE(node.is_splat());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "define foo(*$bar = []) { }");
            }
        }
        GIVEN("a node expression") {
            hostname_parts hostname = {
                create_name("foo"),
                create_name("bar"),
                create_name("baz")
            };
            auto subnode = create_node({ ast::hostname{ hostname } });
            node = subnode;
            THEN("the same context should be returned") {
                REQUIRE(node.context() == subnode);
            }
            THEN("it should not be default") {
                REQUIRE_FALSE(node.is_default());
            }
            THEN("it should be productive") {
                REQUIRE(node.is_productive());
            }
            THEN("it should not be a splat expression") {
                REQUIRE_FALSE(node.is_splat());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "node foo.bar.baz { }");
            }
        }
        GIVEN("a collector expression") {
            auto subnode = create_collector("File");
            node = subnode;
            THEN("the same context should be returned") {
                REQUIRE(node.context() == subnode.context());
            }
            THEN("it should not be default") {
                REQUIRE_FALSE(node.is_default());
            }
            THEN("it should be productive") {
                REQUIRE(node.is_productive());
            }
            THEN("it should not be a splat expression") {
                REQUIRE_FALSE(node.is_splat());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "File<||>");
            }
        }
        GIVEN("a function expression") {
            auto subnode = create_function(
                "foo",
                {
                    create_parameter(
                        "bar",
                        boost::none,
                        true,
                        create_expression(primary(create_array()))
                    )
                }
            );
            node = subnode;
            THEN("the same context should be returned") {
                REQUIRE(node.context() == subnode);
            }
            THEN("it should not be default") {
                REQUIRE_FALSE(node.is_default());
            }
            THEN("it should be productive") {
                REQUIRE(node.is_productive());
            }
            THEN("it should not be a splat expression") {
                REQUIRE_FALSE(node.is_splat());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "function foo(*$bar = []) { }");
            }
        }
        GIVEN("a non-splat unary expression") {
            auto subnode = create_unary(unary_operator::logical_not, create_postfix(primary(create_variable("foo"))));
            node = subnode;
            THEN("the same context should be returned") {
                REQUIRE(node.context() == subnode.context());
            }
            THEN("it should not be default") {
                REQUIRE_FALSE(node.is_default());
            }
            THEN("it should not be productive") {
                REQUIRE_FALSE(node.is_productive());
            }
            THEN("it should not be a splat expression") {
                REQUIRE_FALSE(node.is_splat());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "!$foo");
            }
        }
        GIVEN("a splat unary expression") {
            auto subnode = create_unary(unary_operator::splat, create_postfix(primary(create_variable("foo"))));
            node = subnode;
            THEN("it should be a splat expression") {
                REQUIRE(node.is_splat());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "*$foo");
            }
        }
        GIVEN("an EPP render expression") {
            auto subnode = create_render_expression(
                create_expression(primary(create_variable("foo")))
            );
            node = subnode;
            THEN("the same context should be returned") {
                REQUIRE(node.context() == subnode);
            }
            THEN("it should not be default") {
                REQUIRE_FALSE(node.is_default());
            }
            THEN("it should be productive") {
                REQUIRE(node.is_productive());
            }
            THEN("it should not be a splat expression") {
                REQUIRE_FALSE(node.is_splat());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "render($foo)");
            }
        }
        GIVEN("an EPP block expression") {
            auto subnode = create_render_block(
                {
                    create_expression(primary(create_variable("foo")))
                }
            );
            node = subnode;
            THEN("the same context should be returned") {
                REQUIRE(node.context() == subnode);
            }
            THEN("it should not be default") {
                REQUIRE_FALSE(node.is_default());
            }
            THEN("it should be productive") {
                REQUIRE(node.is_productive());
            }
            THEN("it should not be a splat expression") {
                REQUIRE_FALSE(node.is_splat());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "render({ $foo })");
            }
        }
        GIVEN("an EPP render string expression") {
            auto subnode = create_render_string("hello");
            node = subnode;
            THEN("the same context should be returned") {
                REQUIRE(node.context() == subnode);
            }
            THEN("it should not be default") {
                REQUIRE_FALSE(node.is_default());
            }
            THEN("it should be productive") {
                REQUIRE(node.is_productive());
            }
            THEN("it should not be a splat expression") {
                REQUIRE_FALSE(node.is_splat());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "render('hello')");
            }
        }
        GIVEN("a produces expression") {
            auto subnode = create_produces(
                "Foo::Bar",
                "Sql",
                {
                    create_attribute(
                        "baz",
                        attribute_operator::assignment,
                        create_expression(primary(create_variable("jam")))
                    )
                }
            );
            node = subnode;
            THEN("the same context should be returned") {
                REQUIRE(node.context() == subnode.context());
            }
            THEN("it should not be default") {
                REQUIRE_FALSE(node.is_default());
            }
            THEN("it should be productive") {
                REQUIRE(node.is_productive());
            }
            THEN("it should not be a splat expression") {
                REQUIRE_FALSE(node.is_splat());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "Foo::Bar produces Sql { baz => $jam }");
            }
        }
        GIVEN("a consumes expression") {
            auto subnode = create_consumes(
                "Foo::Bar",
                "Sql",
                {
                    create_attribute(
                        "baz",
                        attribute_operator::assignment,
                        create_expression(primary(create_variable("jam")))
                    )
                }
            );
            node = subnode;
            THEN("the same context should be returned") {
                REQUIRE(node.context() == subnode.context());
            }
            THEN("it should not be default") {
                REQUIRE_FALSE(node.is_default());
            }
            THEN("it should be productive") {
                REQUIRE(node.is_productive());
            }
            THEN("it should not be a splat expression") {
                REQUIRE_FALSE(node.is_splat());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "Foo::Bar consumes Sql { baz => $jam }");
            }
        }
        GIVEN("an application expression") {
            auto subnode = create_application(
                "foo",
                {
                    create_parameter(
                        "bar",
                        boost::none,
                        false,
                        create_expression(primary(create_number(80)))
                    )
                }
            );
            node = subnode;
            THEN("the same context should be returned") {
                REQUIRE(node.context() == subnode);
            }
            THEN("it should not be default") {
                REQUIRE_FALSE(node.is_default());
            }
            THEN("it should be productive") {
                REQUIRE(node.is_productive());
            }
            THEN("it should not be a splat expression") {
                REQUIRE_FALSE(node.is_splat());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "application foo($bar = 80) { }");
            }
        }
        GIVEN("a site expression") {
            auto subnode = create_site(
                {
                    create_expression(primary(
                        create_resource(
                            resource_status::realized,
                            create_postfix(primary(create_name("foo"))),
                            {
                                create_resource_body(
                                    create_expression(primary(create_name("something"))),
                                    {
                                        create_attribute(
                                            "bar",
                                            attribute_operator::assignment,
                                            create_expression(primary(create_number(8080)))
                                        )
                                    }
                                )
                            }
                        ))
                    )
                }
            );
            node = subnode;
            THEN("the same context should be returned") {
                REQUIRE(node.context() == subnode);
            }
            THEN("it should not be default") {
                REQUIRE_FALSE(node.is_default());
            }
            THEN("it should be productive") {
                REQUIRE(node.is_productive());
            }
            THEN("it should not be a splat expression") {
                REQUIRE_FALSE(node.is_splat());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "site { foo { something: bar => 8080 } }");
            }
        }
    }
}

SCENARIO("postfix subexpression", "[ast]")
{
    postfix_subexpression node;
    THEN("it should have the expected number of types") {
        REQUIRE(boost::mpl::size<postfix_subexpression::types>::value == 3);
    }
    WHEN("using a postfix subexpression") {
        GIVEN("a selector expression") {
            auto subnode = create_selector(
                {
                    make_pair(create_expression(primary(create_default())), create_expression(primary(create_number(1))))
                }
            );
            node = subnode;
            THEN("the same context should be returned") {
                REQUIRE(node.context() == subnode);
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == " ? { default => 1 }");
            }
        }
        GIVEN("an access expression") {
            auto subnode = create_access(
                {
                    create_expression(primary(create_number(1))),
                    create_expression(primary(create_number(2)))
                }
            );
            node = subnode;
            THEN("the same context should be returned") {
                REQUIRE(node.context() == subnode);
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "[1, 2]");
            }
        }
        GIVEN("a method call expression") {
            auto subnode = create_method_call(
                "foobar",
                {
                    create_expression(primary(create_number(1))),
                    create_expression(primary(create_number(2)))
                }
            );
            node = subnode;
            THEN("the same context should be returned") {
                REQUIRE(node.context() == subnode.context());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == ".foobar(1, 2)");
            }
        }
    }
}

SCENARIO("postfix expression", "[ast]")
{
    postfix_expression node;
    WHEN("the primary expression is productive") {
        node = create_postfix(primary(create_function_call("foo")));
        THEN("the postfix expression is productive") {
            REQUIRE(node.is_productive());
        }
        THEN("it should output the expected format") {
            REQUIRE(lexical_cast<std::string>(node) == "foo()");
        }
    }
    WHEN("the primary expression is not productive") {
        node = create_postfix(primary(create_variable("foo")));
        AND_WHEN("there is no method call subexpression") {
            THEN("the postfix expression is not productive") {
                REQUIRE_FALSE(node.is_productive());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "$foo");
            }
        }
        AND_WHEN("there is a method call subexpression") {
            node.subexpressions.emplace_back(
                subexpression(create_method_call("bar"))
            );
            THEN("the postfix expression is productive") {
                REQUIRE(node.is_productive());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "$foo.bar()");
            }
            THEN("the context should be from the primary expression to the end of the subexpression") {
                auto context = node.context();
                REQUIRE(context.begin == node.primary.context().begin);
                REQUIRE(context.end == node.subexpressions.back().context().end);
                REQUIRE(context.tree == node.primary.context().tree);
            }
        }
    }
    WHEN("the primary expression is not a splat") {
        node = create_postfix(primary(create_variable("foo")));
        THEN("the postfix expression is not a splat") {
            REQUIRE_FALSE(node.is_splat());
        }
        THEN("it should output the expected format") {
            REQUIRE(lexical_cast<std::string>(node) == "$foo");
        }
    }
    WHEN("the primary expression is a splat") {
        node = create_postfix(
            primary(create_unary(unary_operator::splat, create_postfix(primary(create_variable("foo"))))),
            {
                subexpression(create_access({ create_expression(primary(create_number(1))) }))
            }
        );
        THEN("the postfix expression is a splat") {
            REQUIRE(node.is_splat());
        }
        THEN("it should output the expected format") {
            REQUIRE(lexical_cast<std::string>(node) == "*$foo[1]");
        }
    }
    WHEN("the primary expression is default") {
        node = create_postfix(primary(create_default()));
        AND_WHEN("there are no subexpressions") {
            THEN("the postfix expression is default") {
                REQUIRE(node.is_default());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "default");
            }
        }
        AND_WHEN("there are subexpressions") {
            node.subexpressions.emplace_back(
                subexpression(create_method_call("foo"))
            );
            THEN("the postfix expression is not default") {
                REQUIRE_FALSE(node.is_default());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "default.foo()");
            }
        }
    }
}

SCENARIO("output binary operator", "[ast]")
{
    REQUIRE(lexical_cast<std::string>(binary_operator::in) == "in");
    REQUIRE(lexical_cast<std::string>(binary_operator::match) == "=~");
    REQUIRE(lexical_cast<std::string>(binary_operator::not_match) == "!~");
    REQUIRE(lexical_cast<std::string>(binary_operator::multiply) == "*");
    REQUIRE(lexical_cast<std::string>(binary_operator::divide) == "/");
    REQUIRE(lexical_cast<std::string>(binary_operator::modulo) == "%");
    REQUIRE(lexical_cast<std::string>(binary_operator::plus) == "+");
    REQUIRE(lexical_cast<std::string>(binary_operator::minus) == "-");
    REQUIRE(lexical_cast<std::string>(binary_operator::left_shift) == "<<");
    REQUIRE(lexical_cast<std::string>(binary_operator::right_shift) == ">>");
    REQUIRE(lexical_cast<std::string>(binary_operator::equals) == "==");
    REQUIRE(lexical_cast<std::string>(binary_operator::not_equals) == "!=");
    REQUIRE(lexical_cast<std::string>(binary_operator::greater_than) == ">");
    REQUIRE(lexical_cast<std::string>(binary_operator::greater_equals) == ">=");
    REQUIRE(lexical_cast<std::string>(binary_operator::less_than) == "<");
    REQUIRE(lexical_cast<std::string>(binary_operator::less_equals) == "<=");
    REQUIRE(lexical_cast<std::string>(binary_operator::logical_and) == "and");
    REQUIRE(lexical_cast<std::string>(binary_operator::logical_or) == "or");
    REQUIRE(lexical_cast<std::string>(binary_operator::assignment) == "=");
    REQUIRE(lexical_cast<std::string>(binary_operator::in_edge) == "->");
    REQUIRE(lexical_cast<std::string>(binary_operator::in_edge_subscribe) == "~>");
    REQUIRE(lexical_cast<std::string>(binary_operator::out_edge) == "<-");
    REQUIRE(lexical_cast<std::string>(binary_operator::out_edge_subscribe) == "<~");
    REQUIRE_THROWS(lexical_cast<std::string>(static_cast<binary_operator>(numeric_limits<size_t>::max())));
}

SCENARIO("hash binary operator", "[ast]")
{
    REQUIRE(hash_value(binary_operator::in) == hash_value(binary_operator::in));
    REQUIRE(hash_value(binary_operator::in) != hash_value(binary_operator::match));
}

SCENARIO("binary operator precedence", "[ast]")
{
    REQUIRE(precedence(binary_operator::in) == 11);
    REQUIRE(precedence(binary_operator::match) == 10);
    REQUIRE(precedence(binary_operator::not_match) == 10);
    REQUIRE(precedence(binary_operator::multiply) == 9);
    REQUIRE(precedence(binary_operator::divide) == 9);
    REQUIRE(precedence(binary_operator::modulo) == 9);
    REQUIRE(precedence(binary_operator::plus) == 8);
    REQUIRE(precedence(binary_operator::minus) == 8);
    REQUIRE(precedence(binary_operator::left_shift) == 7);
    REQUIRE(precedence(binary_operator::right_shift) == 7);
    REQUIRE(precedence(binary_operator::equals) == 6);
    REQUIRE(precedence(binary_operator::not_equals) == 6);
    REQUIRE(precedence(binary_operator::greater_than) == 5);
    REQUIRE(precedence(binary_operator::greater_equals) == 5);
    REQUIRE(precedence(binary_operator::less_than) == 5);
    REQUIRE(precedence(binary_operator::less_equals) == 5);
    REQUIRE(precedence(binary_operator::logical_and) == 4);
    REQUIRE(precedence(binary_operator::logical_or) == 3);
    REQUIRE(precedence(binary_operator::assignment) == 2);
    REQUIRE(precedence(binary_operator::in_edge) == 1);
    REQUIRE(precedence(binary_operator::in_edge_subscribe) == 1);
    REQUIRE(precedence(binary_operator::out_edge) == 1);
    REQUIRE(precedence(binary_operator::out_edge_subscribe) == 1);
    REQUIRE_THROWS(precedence(static_cast<binary_operator>(numeric_limits<size_t>::max())));
}

SCENARIO("binary operator right associativity", "[ast]")
{
    REQUIRE_FALSE(is_right_associative(binary_operator::in));
    REQUIRE_FALSE(is_right_associative(binary_operator::match));
    REQUIRE_FALSE(is_right_associative(binary_operator::not_match));
    REQUIRE_FALSE(is_right_associative(binary_operator::multiply));
    REQUIRE_FALSE(is_right_associative(binary_operator::divide));
    REQUIRE_FALSE(is_right_associative(binary_operator::modulo));
    REQUIRE_FALSE(is_right_associative(binary_operator::plus));
    REQUIRE_FALSE(is_right_associative(binary_operator::minus));
    REQUIRE_FALSE(is_right_associative(binary_operator::left_shift));
    REQUIRE_FALSE(is_right_associative(binary_operator::right_shift));
    REQUIRE_FALSE(is_right_associative(binary_operator::equals));
    REQUIRE_FALSE(is_right_associative(binary_operator::not_equals));
    REQUIRE_FALSE(is_right_associative(binary_operator::greater_than));
    REQUIRE_FALSE(is_right_associative(binary_operator::greater_equals));
    REQUIRE_FALSE(is_right_associative(binary_operator::less_than));
    REQUIRE_FALSE(is_right_associative(binary_operator::less_equals));
    REQUIRE_FALSE(is_right_associative(binary_operator::logical_and));
    REQUIRE_FALSE(is_right_associative(binary_operator::logical_or));
    REQUIRE(is_right_associative(binary_operator::assignment));
    REQUIRE_FALSE(is_right_associative(binary_operator::in_edge));
    REQUIRE_FALSE(is_right_associative(binary_operator::in_edge_subscribe));
    REQUIRE_FALSE(is_right_associative(binary_operator::out_edge));
    REQUIRE_FALSE(is_right_associative(binary_operator::out_edge_subscribe));
    REQUIRE_FALSE(is_right_associative(static_cast<binary_operator>(numeric_limits<size_t>::max())));
}

SCENARIO("binary operator productivity", "[ast]")
{
    REQUIRE_FALSE(is_productive(binary_operator::in));
    REQUIRE_FALSE(is_productive(binary_operator::match));
    REQUIRE_FALSE(is_productive(binary_operator::not_match));
    REQUIRE_FALSE(is_productive(binary_operator::multiply));
    REQUIRE_FALSE(is_productive(binary_operator::divide));
    REQUIRE_FALSE(is_productive(binary_operator::modulo));
    REQUIRE_FALSE(is_productive(binary_operator::plus));
    REQUIRE_FALSE(is_productive(binary_operator::minus));
    REQUIRE_FALSE(is_productive(binary_operator::left_shift));
    REQUIRE_FALSE(is_productive(binary_operator::right_shift));
    REQUIRE_FALSE(is_productive(binary_operator::equals));
    REQUIRE_FALSE(is_productive(binary_operator::not_equals));
    REQUIRE_FALSE(is_productive(binary_operator::greater_than));
    REQUIRE_FALSE(is_productive(binary_operator::greater_equals));
    REQUIRE_FALSE(is_productive(binary_operator::less_than));
    REQUIRE_FALSE(is_productive(binary_operator::less_equals));
    REQUIRE_FALSE(is_productive(binary_operator::logical_and));
    REQUIRE_FALSE(is_productive(binary_operator::logical_or));
    REQUIRE(is_productive(binary_operator::assignment));
    REQUIRE(is_productive(binary_operator::in_edge));
    REQUIRE(is_productive(binary_operator::in_edge_subscribe));
    REQUIRE(is_productive(binary_operator::out_edge));
    REQUIRE(is_productive(binary_operator::out_edge_subscribe));
    REQUIRE_FALSE(is_productive(static_cast<binary_operator>(numeric_limits<size_t>::max())));
}

SCENARIO("expression", "[ast]")
{
    expression node;
    WHEN("there are no binary operations") {
        AND_WHEN("the postfix expression is productive") {
            node = create_expression(
                primary(create_function_call("foo"))
            );
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "foo()");
            }
            THEN("it should be productive") {
                REQUIRE(node.is_productive());
            }
            THEN("it should not be splat") {
                REQUIRE_FALSE(node.is_splat());
            }
        }
        AND_WHEN("the postfix expression is not productive") {
            node = create_expression(
                primary(create_number(1234))
            );
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "1234");
            }
            THEN("it should not be productive") {
                REQUIRE_FALSE(node.is_productive());
            }
            THEN("it should not be splat") {
                REQUIRE_FALSE(node.is_splat());
            }
        }
        AND_WHEN("the postfix expression is a splat") {
            node = create_expression(
                primary(create_unary(unary_operator::splat, create_postfix(primary(create_array()))))
            );
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "*[]");
            }
            THEN("it should not be productive") {
                REQUIRE_FALSE(node.is_productive());
            }
            THEN("it should be splat") {
                REQUIRE(node.is_splat());
            }
        }
    }
    WHEN("there are binary operations") {
        THEN("is never default")
        AND_WHEN("the binary operator is assignment") {
            node = create_expression(
                create_postfix(primary(create_variable("foo"))),
                {
                    create_binary(
                        binary_operator::assignment,
                        create_postfix(primary(create_number(1)))
                    )
                }
            );
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "$foo = 1");
            }
            THEN("it should be productive") {
                REQUIRE(node.is_productive());
            }
        }
        AND_WHEN("the binary operator is in_edge") {
            node = create_expression(
                create_postfix(primary(create_variable("foo"))),
                {
                    create_binary(
                        binary_operator::in_edge,
                        create_postfix(primary(create_variable("bar")))
                    )
                }
            );
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "$foo -> $bar");
            }
            THEN("it should be productive") {
                REQUIRE(node.is_productive());
            }
        }
        AND_WHEN("the binary operator is in_edge_subscribe") {
            node = create_expression(
                create_postfix(primary(create_variable("foo"))),
                {
                    create_binary(
                        binary_operator::in_edge_subscribe,
                        create_postfix(primary(create_variable("bar")))
                    )
                }
            );
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "$foo ~> $bar");
            }
            THEN("it should be productive") {
                REQUIRE(node.is_productive());
            }
        }
        AND_WHEN("the binary operator is out_edge") {
            node = create_expression(
                create_postfix(primary(create_variable("foo"))),
                {
                    create_binary(
                        binary_operator::out_edge,
                        create_postfix(primary(create_variable("bar")))
                    )
                }
            );
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "$foo <- $bar");
            }
            THEN("it should be productive") {
                REQUIRE(node.is_productive());
            }
        }
        AND_WHEN("the binary operator is out_edge") {
            node = create_expression(
                create_postfix(primary(create_variable("foo"))),
                {
                    create_binary(
                        binary_operator::out_edge_subscribe,
                        create_postfix(primary(create_variable("bar")))
                    )
                }
            );
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "$foo <~ $bar");
            }
            THEN("it should be productive") {
                REQUIRE(node.is_productive());
            }
        }
        AND_WHEN("the postfix expression is not productive") {
            node = create_expression(
                create_postfix(primary(create_number(1))),
                {
                    create_binary(
                        binary_operator::plus,
                        create_postfix(primary(create_number(1)))
                    )
                }
            );
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "1 + 1");
            }
            THEN("it should not be productive") {
                REQUIRE_FALSE(node.is_productive());
            }
            THEN("it should not be splat") {
                REQUIRE_FALSE(node.is_splat());
            }
        }
        AND_WHEN("the postfix expression is a splat") {
            node = create_expression(
                primary(create_unary(unary_operator::splat, create_postfix(primary(create_array()))))
            );
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "*[]");
            }
            THEN("it should not be productive") {
                REQUIRE_FALSE(node.is_productive());
            }
            THEN("it should be splat") {
                REQUIRE(node.is_splat());
            }
        }
    }
}

SCENARIO("nested expression", "[ast]")
{
    auto node = create_nested(
        create_expression(
            create_postfix(primary(create_number(1))),
            {
                create_binary(
                    binary_operator::greater_equals,
                    create_postfix(primary(create_number(0)))
                )
            }
        )
    );

    WHEN("using nested expression") {
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2 == node);
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3 == node);
        }
        THEN("it should be convertible to context") {
            context ctx = node;
            REQUIRE(ctx == node);
        }
        THEN("it should output the expected format") {
            REQUIRE(lexical_cast<std::string>(node) == "(1 >= 0)");
        }
        GIVEN("another non-equal object") {
            decltype(node) other;
            THEN("the objects should not be equal") {
                REQUIRE(node != other);
            }
        }
    }
}

SCENARIO("array", "[ast]")
{
    auto node = create_array(
        {
            create_expression(
                create_postfix(primary(create_number(1))),
                {
                    create_binary(
                        binary_operator::plus,
                        create_postfix(primary(create_number(1)))
                    )
                }
            ),
            create_expression(primary(create_string("foo"))),
            create_expression(primary(create_regex("^.*bar$")))
        }
    );

    WHEN("using array") {
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2 == node);
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3 == node);
        }
        THEN("it should be convertible to context") {
            context ctx = node;
            REQUIRE(ctx == node);
        }
        THEN("it should output the expected format") {
            REQUIRE(lexical_cast<std::string>(node) == "[1 + 1, 'foo', /^.*bar$/]");
        }
        GIVEN("another non-equal object") {
            decltype(node) other;
            THEN("the objects should not be equal") {
                REQUIRE(node != other);
            }
        }
    }
}

SCENARIO("hash", "[ast]")
{
    auto node = create_hash(
        {
            make_pair(
                create_expression(primary(create_string("foo"))),
                create_expression(
                    create_postfix(primary(create_number(1))),
                    {
                        create_binary(
                            binary_operator::plus,
                            create_postfix(primary(create_number(1)))
                        )
                    }
                )
            ),
            make_pair(
                create_expression(primary(create_number(1234))),
                create_expression(primary(create_string("foo")))
            ),
            make_pair(
                create_expression(primary(create_string("bar"))),
                create_expression(primary(create_regex("^.*bar$")))
            )
        }
    );

    WHEN("using hash") {
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2 == node);
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3 == node);
        }
        THEN("it should be convertible to context") {
            context ctx = node;
            REQUIRE(ctx == node);
        }
        THEN("it should output the expected format") {
            REQUIRE(lexical_cast<std::string>(node) == "{'foo' => 1 + 1, 1234 => 'foo', 'bar' => /^.*bar$/}");
        }
        GIVEN("another non-equal object") {
            decltype(node) other;
            THEN("the objects should not be equal") {
                REQUIRE(node != other);
            }
        }
    }
}

SCENARIO("selector expression", "[ast]")
{
    auto node = create_selector(
        {
            make_pair(
                create_expression(primary(create_string("foo"))),
                create_expression(
                    create_postfix(primary(create_number(1))),
                    {
                        create_binary(
                            binary_operator::plus,
                            create_postfix(primary(create_number(1)))
                        )
                    }
                )
            ),
            make_pair(
                create_expression(primary(create_number(1234))),
                create_expression(primary(create_string("foo")))
            ),
            make_pair(
                create_expression(primary(create_string("bar"))),
                create_expression(primary(create_regex("^.*bar$")))
            )
        }
    );

    WHEN("using selector expression") {
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2 == node);
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3 == node);
        }
        THEN("it should be convertible to context") {
            context ctx = node;
            REQUIRE(ctx == node);
        }
        THEN("it should output the expected format") {
            REQUIRE(lexical_cast<std::string>(node) == " ? { 'foo' => 1 + 1, 1234 => 'foo', 'bar' => /^.*bar$/ }");
        }
        GIVEN("another non-equal object") {
            decltype(node) other;
            THEN("the objects should not be equal") {
                REQUIRE(node != other);
            }
        }
    }
}

SCENARIO("case expression", "[ast]")
{
    auto node = create_case(
        create_expression(primary(create_variable("foo"))),
        {
            create_proposition(
                {
                    create_expression(primary(create_string("foo"))),
                    create_expression(primary(create_string("bar"))),
                    create_expression(primary(create_string("baz")))
                },
                {
                    create_expression(primary(create_function_call("foo")))
                }
            ),
            create_proposition(
                {
                    create_expression(primary(create_boolean(true))),
                    create_expression(primary(create_boolean(false))),
                    create_expression(primary(create_undef()))
                },
                {
                    create_expression(primary(create_function_call("bar")))
                }
            ),
            create_proposition(
                {
                    create_expression(primary(create_default()))
                },
                {
                    create_expression(primary(create_function_call("baz")))
                }
            )
        }
    );

    WHEN("using case expression") {
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2 == node);
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3 == node);
        }
        THEN("it should be convertible to context") {
            context ctx = node;
            REQUIRE(ctx == node);
        }
        THEN("it should output the expected format") {
            REQUIRE(lexical_cast<std::string>(node) == "case $foo { 'foo', 'bar', 'baz': { foo() } true, false, undef: { bar() } default: { baz() } }");
        }
        GIVEN("another non-equal object") {
            decltype(node) other;
            THEN("the objects should not be equal") {
                REQUIRE(node != other);
            }
        }
    }
}

SCENARIO("if expression", "[ast]")
{
    auto node = create_if(
        create_expression(
            create_postfix(primary(create_string("foo"))),
            {
                create_binary(
                    binary_operator::in,
                    create_postfix(primary(create_string("foobar")))
                )
            }
        ),
        {
            create_expression(primary(create_function_call("foo"))),
            create_expression(primary(create_function_call("bar")))
        },
        {
            create_elsif(
                create_expression(primary(create_variable("jam"))),
                {
                    create_expression(primary(create_string("baz")))
                }
            ),
            create_elsif(
                create_expression(primary(create_variable("snapple"))),
                {
                }
            )
        },
        create_else(
            {
                create_expression(primary(create_function_call("snausage")))
            }
        )
    );

    WHEN("using if expression") {
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2.context() == node.context());
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3.context() == node.context());
        }
        WHEN("there are no elsif and no else") {
            node.elsifs.clear();
            node.else_ = boost::none;
            THEN("the end should be the end of if expression") {
                context ctx = node.context();
                REQUIRE(ctx.begin == node.begin);
                REQUIRE(ctx.end == node.end);
                REQUIRE(ctx.tree == node.conditional.context().tree);
            }
        }
        WHEN("there are elsif and no else") {
            node.else_ = boost::none;
            THEN("the end should be the end of the last elsif") {
                context ctx = node.context();
                REQUIRE(ctx.begin == node.begin);
                REQUIRE(ctx.end == node.elsifs.back().end);
                REQUIRE(ctx.tree == node.conditional.context().tree);
            }
        }
        WHEN("there are elsif and an else") {
            THEN("the end should be the end of the else") {
                context ctx = node.context();
                REQUIRE(ctx.begin == node.begin);
                REQUIRE(ctx.end == node.else_->end);
                REQUIRE(ctx.tree == node.conditional.context().tree);
            }
        }
        THEN("it should output the expected format") {
            REQUIRE(lexical_cast<std::string>(node) == "if 'foo' in 'foobar' { foo(); bar() } elsif $jam { 'baz' } elsif $snapple { } else { snausage() }");
        }
        GIVEN("another non-equal object") {
            decltype(node) other;
            THEN("the objects should not be equal") {
                REQUIRE(node.context() != other.context());
            }
        }
    }
}

SCENARIO("unless expression", "[ast]")
{
    auto node = create_unless(
        create_expression(
            create_postfix(primary(create_variable("foo"))),
            {
                create_binary(
                    binary_operator::logical_and,
                    create_postfix(primary(create_variable("bar")))
                )
            }
        ),
        {
            create_expression(primary(create_function_call("foo"))),
        },
        create_else(
            {
            }
        )
    );

    WHEN("using unless expression") {
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2.context() == node.context());
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3.context() == node.context());
        }
        WHEN("there is no else") {
            node.else_ = boost::none;
            THEN("the end should be the end of unless expression") {
                context ctx = node.context();
                REQUIRE(ctx.begin == node.begin);
                REQUIRE(ctx.end == node.end);
                REQUIRE(ctx.tree == node.conditional.context().tree);
            }
        }
        WHEN("there is an else") {
            THEN("the end should be the end of the else") {
                context ctx = node.context();
                REQUIRE(ctx.begin == node.begin);
                REQUIRE(ctx.end == node.else_->end);
                REQUIRE(ctx.tree == node.conditional.context().tree);
            }
        }
        THEN("it should output the expected format") {
            REQUIRE(lexical_cast<std::string>(node) == "unless $foo and $bar { foo() } else { }");
        }
        GIVEN("another non-equal object") {
            decltype(node) other;
            THEN("the objects should not be equal") {
                REQUIRE(node.context() != other.context());
            }
        }
    }
}

SCENARIO("access expression", "[ast]")
{
    auto node = create_access(
        {
            create_expression(primary(create_type("Foo"))),
            create_expression(primary(create_number(1))),
            create_expression(primary(create_string("bar")))
        }
    );

    WHEN("using access expression") {
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2 == node);
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3 == node);
        }
        THEN("it should be convertible to context") {
            context ctx = node;
            REQUIRE(ctx == node);
        }
        THEN("it should output the expected format") {
            REQUIRE(lexical_cast<std::string>(node) == "[Foo, 1, 'bar']");
        }
        GIVEN("another non-equal object") {
            decltype(node) other;
            THEN("the objects should not be equal") {
                REQUIRE(node != other);
            }
        }
    }
}

SCENARIO("parameter", "[ast]")
{
    auto node = create_parameter("foo");
    THEN("it should output the expected format") {
        REQUIRE(lexical_cast<std::string>(node) == "$foo");
    }
    THEN("the context should be the same as the variable") {
        REQUIRE(node.context() == node.variable);
    }
    WHEN("capturing ouptut") {
        node.captures = create_position();
        THEN("it should output the expected format") {
            REQUIRE(lexical_cast<std::string>(node) == "*$foo");
        }
        THEN("the context should start at the capture's position") {
            auto context = node.context();
            REQUIRE(context.begin == *node.captures);
            REQUIRE(context.end == node.variable.end);
            REQUIRE(context.tree == node.variable.tree);
        }
        AND_WHEN("a default value is specified") {
            node.default_value = create_expression(primary(create_number(1)));
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "*$foo = 1");
            }
            THEN("the context should be from the captures to the end of the default") {
                auto context = node.context();
                REQUIRE(context.begin == *node.captures);
                REQUIRE(context.end == node.default_value->context().end);
                REQUIRE(context.tree == node.variable.tree);
            }
        }
    }
    WHEN("a type is specified") {
        node.type = create_postfix(
            primary(create_type("Integer")),
            {
                subexpression(
                    create_access(
                        {
                            create_expression(primary(create_number(0))),
                            create_expression(primary(create_number(10))),
                        }
                    )
                )
            }
        );
        THEN("it should output the expected format") {
            REQUIRE(lexical_cast<std::string>(node) == "Integer[0, 10] $foo");
        }
        THEN("the context should start at the type and end with the variable") {
            auto context = node.context();
            REQUIRE(context.begin == node.type->context().begin);
            REQUIRE(context.end == node.variable.end);
            REQUIRE(context.tree == node.variable.tree);
        }
        AND_WHEN("a default value is specified") {
            node.default_value = create_expression(primary(create_number(1)));
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "Integer[0, 10] $foo = 1");
            }
            THEN("the context should be from the type to the end of the default") {
                auto context = node.context();
                REQUIRE(context.begin == node.type->context().begin);
                REQUIRE(context.end == node.default_value->context().end);
                REQUIRE(context.tree == node.variable.tree);
            }
        }
    }
    WHEN("a default value is specified") {
        node.default_value = create_expression(primary(create_number(1)));
        THEN("it should output the expected format") {
            REQUIRE(lexical_cast<std::string>(node) == "$foo = 1");
        }
        THEN("the context should be from the variable to the end of the default") {
            auto context = node.context();
            REQUIRE(context.begin == node.variable.begin);
            REQUIRE(context.end == node.default_value->context().end);
            REQUIRE(context.tree == node.variable.tree);
        }
    }
    WHEN("a type, captures, and default value are specified") {
        node.type = create_postfix(
            primary(create_type("Integer")),
            {
                subexpression(
                    create_access(
                        {
                            create_expression(primary(create_number(0))),
                            create_expression(primary(create_number(10))),
                        }
                    )
                )
            }
        );
        node.captures = create_position();
        node.default_value = create_expression(primary(create_number(1)));
        THEN("it should output the expected format") {
            REQUIRE(lexical_cast<std::string>(node) == "Integer[0, 10] *$foo = 1");
        }
        THEN("the context should be from the type to the end of the default") {
            auto context = node.context();
            REQUIRE(context.begin == node.type->context().begin);
            REQUIRE(context.end == node.default_value->context().end);
            REQUIRE(context.tree == node.variable.tree);
        }
    }
}


SCENARIO("lambda expression", "[ast]")
{
    auto node = create_lambda(
        {
            create_parameter(
                "foo",
                create_postfix(
                    primary(create_type("Integer")),
                    {
                        subexpression(
                            create_access(
                                {
                                    create_expression(primary(create_number(0))),
                                    create_expression(primary(create_number(10))),
                                }
                            )
                        )
                    }
                ),
                false,
                create_expression(primary(create_number(5)))
            ),
            create_parameter(
                "bar",
                create_postfix(primary(create_type("Array"))),
                true
            )
        },
        {
            create_expression(primary(create_function_call("something"))),
        }
    );

    WHEN("using lambda expression") {
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2 == node);
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3 == node);
        }
        THEN("it should be convertible to context") {
            context ctx = node;
            REQUIRE(ctx == node);
        }
        THEN("it should output the expected format") {
            REQUIRE(lexical_cast<std::string>(node) == "|Integer[0, 10] $foo = 5, Array *$bar| { something() }");
        }
        GIVEN("another non-equal object") {
            decltype(node) other;
            THEN("the objects should not be equal") {
                REQUIRE(node != other);
            }
        }
    }
}

SCENARIO("method call expression", "[ast]")
{
    auto node = create_method_call(
        "foo",
        {
            create_expression(primary(create_number(1))),
            create_expression(primary(create_string("bar"))),
            create_expression(primary(create_array()))
        },
        create_lambda(
            {
                create_parameter("foo"),
                create_parameter("bar")
            },
            {
            }
        )
    );

    WHEN("using method call expression") {
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2.context() == node.context());
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3.context() == node.context());
        }
        WHEN("there are no arguments and no lambda and no end") {
            node.arguments.clear();
            node.end = boost::none;
            node.lambda = boost::none;
            THEN("the end should be the end method name") {
                context ctx = node.context();
                REQUIRE(ctx.begin == node.begin);
                REQUIRE(ctx.end == node.method.end);
                REQUIRE(ctx.tree == node.method.tree);
            }
        }
        WHEN("there are arguments but no lambda and no end") {
            node.end = boost::none;
            node.lambda = boost::none;
            THEN("the end should be the end of last argument") {
                context ctx = node.context();
                REQUIRE(ctx.begin == node.begin);
                REQUIRE(ctx.end == node.arguments.back().context().end);
                REQUIRE(ctx.tree == node.method.tree);
            }
        }
        WHEN("there is no lambda") {
            node.lambda = boost::none;
            THEN("the end should be the end of method call expression") {
                context ctx = node.context();
                REQUIRE(ctx.begin == node.begin);
                REQUIRE(ctx.end == *node.end);
                REQUIRE(ctx.tree == node.method.tree);
            }
        }
        WHEN("there is lambda") {
            THEN("the end should be the end of the lambda") {
                context ctx = node.context();
                REQUIRE(ctx.begin == node.begin);
                REQUIRE(ctx.end == node.lambda->end);
                REQUIRE(ctx.tree == node.method.tree);
            }
        }
        THEN("it should output the expected format") {
            REQUIRE(lexical_cast<std::string>(node) == ".foo(1, 'bar', []) |$foo, $bar| { }");
        }
        GIVEN("another non-equal object") {
            decltype(node) other;
            THEN("the objects should not be equal") {
                REQUIRE(node.context() != other.context());
            }
        }
    }
}

SCENARIO("function call expression", "[ast]")
{
    auto node = create_function_call(
        "foo",
        {
            create_expression(primary(create_number(1))),
            create_expression(primary(create_string("bar"))),
            create_expression(primary(create_array()))
        },
        create_lambda(
            {
                create_parameter("foo"),
                create_parameter("bar")
            },
            {
            }
        )
    );

    WHEN("using function call expression") {
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2.context() == node.context());
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3.context() == node.context());
        }
        WHEN("there are no arguments and no lambda and no end") {
            node.arguments.clear();
            node.end = boost::none;
            node.lambda = boost::none;
            THEN("the end should be the end method name") {
                context ctx = node.context();
                REQUIRE(ctx.begin == node.function.begin);
                REQUIRE(ctx.end == node.function.end);
                REQUIRE(ctx.tree == node.function.tree);
            }
        }
        WHEN("there are arguments but no lambda and no end") {
            node.end = boost::none;
            node.lambda = boost::none;
            THEN("the end should be the end of last argument") {
                context ctx = node.context();
                REQUIRE(ctx.begin == node.function.begin);
                REQUIRE(ctx.end == node.arguments.back().context().end);
                REQUIRE(ctx.tree == node.function.tree);
            }
        }
        WHEN("there is no lambda") {
            node.lambda = boost::none;
            THEN("the end should be the end of method call expression") {
                context ctx = node.context();
                REQUIRE(ctx.begin == node.function.begin);
                REQUIRE(ctx.end == *node.end);
                REQUIRE(ctx.tree == node.function.tree);
            }
        }
        WHEN("there is lambda") {
            THEN("the end should be the end of the lambda") {
                context ctx = node.context();
                REQUIRE(ctx.begin == node.function.begin);
                REQUIRE(ctx.end == node.lambda->end);
                REQUIRE(ctx.tree == node.function.tree);
            }
        }
        THEN("it should output the expected format") {
            REQUIRE(lexical_cast<std::string>(node) == "foo(1, 'bar', []) |$foo, $bar| { }");
        }
        GIVEN("another non-equal object") {
            decltype(node) other;
            THEN("the objects should not be equal") {
                REQUIRE(node.context() != other.context());
            }
        }
    }
}

SCENARIO("output attribute operator", "[ast]")
{
    REQUIRE(lexical_cast<std::string>(attribute_operator::assignment) == "=>");
    REQUIRE(lexical_cast<std::string>(attribute_operator::append) == "+>");
    REQUIRE_THROWS(lexical_cast<std::string>(static_cast<attribute_operator>(numeric_limits<size_t>::max())));
}

SCENARIO("attribute operation") {
    auto node = create_attribute(
        "foo",
        attribute_operator::assignment,
        create_expression(primary(create_number(1234)))
    );

    WHEN("using attribute operation") {
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2.context() == node.context());
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3.context() == node.context());
        }
        THEN("the context should start at the name and end with the value") {
            auto context = node.context();
            REQUIRE(context.begin == node.name.begin);
            REQUIRE(context.end == node.value.context().end);
            REQUIRE(context.tree == node.name.tree);
        }
        THEN("it should output the expected format") {
            REQUIRE(lexical_cast<std::string>(node) == "foo => 1234");
        }
        GIVEN("another non-equal object") {
            decltype(node) other;
            THEN("the objects should not be equal") {
                REQUIRE(node.context() != other.context());
            }
        }
    }
}

SCENARIO("resource body") {
    auto node = create_resource_body(
        create_expression(primary(create_name("foo"))),
        {
            create_attribute(
                "bar",
                attribute_operator::assignment,
                create_expression(primary(create_number(1234)))
            )
        }
    );

    WHEN("using resource body") {
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2.context() == node.context());
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3.context() == node.context());
        }
        THEN("the context should start at the title and end with the last operation") {
            auto context = node.context();
            REQUIRE(context.begin == node.title.context().begin);
            REQUIRE(context.end == node.operations.back().context().end);
            REQUIRE(context.tree == node.title.context().tree);
        }
        THEN("it should output the expected format") {
            REQUIRE(lexical_cast<std::string>(node) == "foo: bar => 1234");
        }
        GIVEN("another non-equal object") {
            decltype(node) other;
            THEN("the objects should not be equal") {
                REQUIRE(node.context() != other.context());
            }
        }
    }
}

SCENARIO("resource status", "[ast]")
{
    REQUIRE(lexical_cast<std::string>(resource_status::realized) == "realized");
    REQUIRE(lexical_cast<std::string>(resource_status::virtualized) == "virtual");
    REQUIRE(lexical_cast<std::string>(resource_status::exported) == "exported");
    REQUIRE_THROWS(lexical_cast<std::string>(static_cast<resource_status>(numeric_limits<size_t>::max())));
}

SCENARIO("resource expression", "[ast]")
{
    auto node = create_resource(
        resource_status::realized,
        create_postfix(primary(create_name("foo"))),
        {
            create_resource_body(
                create_expression(primary(create_name("bar"))),
                {
                    create_attribute(
                        "foo",
                        attribute_operator::assignment,
                        create_expression(primary(create_name("bar")))
                    ),
                    create_attribute(
                        "baz",
                        attribute_operator::append,
                        create_expression(primary(create_string("cake")))
                    )
                }
            ),
            create_resource_body(
                create_expression(primary(create_name("baz"))),
                {
                    create_attribute(
                        "jam",
                        attribute_operator::assignment,
                        create_expression(primary(create_number(9876)))
                    )
                }
            )
        }
    );

    WHEN("using resource expression") {
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2 == node);
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3 == node);
        }
        THEN("it should be convertible to context") {
            context ctx = node;
            REQUIRE(ctx == node);
        }
        WHEN("the resource is realized") {
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "foo { bar: foo => bar, baz +> 'cake'; baz: jam => 9876 }");
            }
        }
        WHEN("the resource is virtualized") {
            node.status = resource_status::virtualized;
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "@foo { bar: foo => bar, baz +> 'cake'; baz: jam => 9876 }");
            }
        }
        WHEN("the resource is exported") {
            node.status = resource_status::exported;
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "@@foo { bar: foo => bar, baz +> 'cake'; baz: jam => 9876 }");
            }
        }
        GIVEN("another non-equal object") {
            decltype(node) other;
            THEN("the objects should not be equal") {
                REQUIRE(node != other);
            }
        }
    }
}

SCENARIO("resource override expression", "[ast]")
{
    auto node = create_resource_override(
        create_postfix(
            primary(create_type("Foo")),
            {
                subexpression(create_access({
                    create_expression(primary(create_name("bar")))
                }))
            }
        ),
        {
            create_attribute(
                "foo",
                attribute_operator::assignment,
                create_expression(primary(create_name("bar")))
            ),
            create_attribute(
                "baz",
                attribute_operator::append,
                create_expression(primary(create_string("jam")))
            )
        }
    );

    WHEN("using resource override expression") {
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2 == node);
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3 == node);
        }
        THEN("it should be convertible to context") {
            context ctx = node;
            REQUIRE(ctx == node);
        }
        THEN("it should output the expected format") {
            REQUIRE(lexical_cast<std::string>(node) == "Foo[bar] { foo => bar, baz +> 'jam' }");
        }
        GIVEN("another non-equal object") {
            decltype(node) other;
            THEN("the objects should not be equal") {
                REQUIRE(node != other);
            }
        }
    }
}

SCENARIO("resource defaults expression", "[ast]")
{
    auto node = create_resource_defaults(
        "Foo",
        {
            create_attribute(
                "foo",
                attribute_operator::assignment,
                create_expression(primary(create_name("bar")))
            ),
            create_attribute(
                "baz",
                attribute_operator::append,
                create_expression(primary(create_string("jam")))
            )
        }
    );

    WHEN("using resource defaults expression") {
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2 == node);
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3 == node);
        }
        THEN("it should be convertible to context") {
            context ctx = node;
            REQUIRE(ctx == node);
        }
        THEN("it should output the expected format") {
            REQUIRE(lexical_cast<std::string>(node) == "Foo { foo => bar, baz +> 'jam' }");
        }
        GIVEN("another non-equal object") {
            decltype(node) other;
            THEN("the objects should not be equal") {
                REQUIRE(node != other);
            }
        }
    }
}

SCENARIO("class expression", "[ast]")
{
    auto node = create_class(
        "foo::bar",
        {
            create_parameter(
                "foo",
                create_postfix(
                    primary(create_type("Integer")),
                    {
                        subexpression(
                            create_access(
                                {
                                    create_expression(primary(create_number(1000))),
                                    create_expression(primary(create_number(2000))),
                                }
                            )
                        )
                    }
                ),
                false,
                create_expression(primary(create_number(1234)))
            ),
            create_parameter("bar")
        },
        create_name("foo::baz"),
        {
            create_expression(primary(create_function_call("foo")))
        }
    );

    WHEN("using class expression") {
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2 == node);
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3 == node);
        }
        THEN("it should be convertible to context") {
            context ctx = node;
            REQUIRE(ctx == node);
        }
        WHEN("given no parameters") {
            node.parameters.clear();
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "class foo::bar inherits foo::baz { foo() }");
            }
            AND_WHEN("given no parent") {
                node.parent = boost::none;
                THEN("it should output the expected format") {
                    REQUIRE(lexical_cast<std::string>(node) == "class foo::bar { foo() }");
                }
                AND_WHEN("given no body") {
                    node.body.clear();
                    THEN("it should output the expected format") {
                        REQUIRE(lexical_cast<std::string>(node) == "class foo::bar { }");
                    }

                }
            }
            AND_WHEN("given no body") {
                node.body.clear();
                THEN("it should output the expected format") {
                    REQUIRE(lexical_cast<std::string>(node) == "class foo::bar inherits foo::baz { }");
                }

            }
        }
        WHEN("given parameters") {
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "class foo::bar(Integer[1000, 2000] $foo = 1234, $bar) inherits foo::baz { foo() }");
            }
            AND_WHEN("given no parent") {
                node.parent = boost::none;
                THEN("it should output the expected format") {
                    REQUIRE(lexical_cast<std::string>(node) == "class foo::bar(Integer[1000, 2000] $foo = 1234, $bar) { foo() }");
                }
                AND_WHEN("given no body") {
                    node.body.clear();
                    THEN("it should output the expected format") {
                        REQUIRE(lexical_cast<std::string>(node) == "class foo::bar(Integer[1000, 2000] $foo = 1234, $bar) { }");
                    }

                }
            }
            AND_WHEN("given no body") {
                node.body.clear();
                THEN("it should output the expected format") {
                    REQUIRE(lexical_cast<std::string>(node) == "class foo::bar(Integer[1000, 2000] $foo = 1234, $bar) inherits foo::baz { }");
                }

            }
        }
        GIVEN("another non-equal object") {
            decltype(node) other;
            THEN("the objects should not be equal") {
                REQUIRE(node != other);
            }
        }
    }
}

SCENARIO("defined type expression", "[ast]")
{
    auto node = create_defined_type(
        "foo::bar",
        {
            create_parameter(
                "foo",
                create_postfix(
                    primary(create_type("Integer")),
                    {
                        subexpression(
                            create_access(
                                {
                                    create_expression(primary(create_number(1000))),
                                    create_expression(primary(create_number(2000))),
                                }
                            )
                        )
                    }
                ),
                false,
                create_expression(primary(create_number(1234)))
            ),
            create_parameter("bar")
        },
        {
            create_expression(primary(create_function_call("foo")))
        }
    );

    WHEN("using defined type expression") {
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2 == node);
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3 == node);
        }
        THEN("it should be convertible to context") {
            context ctx = node;
            REQUIRE(ctx == node);
        }
        WHEN("given no parameters") {
            node.parameters.clear();
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "define foo::bar { foo() }");
            }
            AND_WHEN("given no body") {
                node.body.clear();
                THEN("it should output the expected format") {
                    REQUIRE(lexical_cast<std::string>(node) == "define foo::bar { }");
                }

            }
        }
        WHEN("given parameters") {
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "define foo::bar(Integer[1000, 2000] $foo = 1234, $bar) { foo() }");
            }
            AND_WHEN("given no body") {
                node.body.clear();
                THEN("it should output the expected format") {
                    REQUIRE(lexical_cast<std::string>(node) == "define foo::bar(Integer[1000, 2000] $foo = 1234, $bar) { }");
                }

            }
        }
        GIVEN("another non-equal object") {
            decltype(node) other;
            THEN("the objects should not be equal") {
                REQUIRE(node != other);
            }
        }
    }
}

SCENARIO("hostname", "[ast]")
{
    hostname node;
    THEN("it should have the expected number of types") {
        REQUIRE(boost::mpl::size<hostname::types>::value == 4);
    }
    WHEN("using hostname") {
        THEN("it should be constructible from default") {
            auto subnode = create_default();
            node = subnode;
            AND_THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "default");
            }
            AND_THEN("the context should be the same as the default") {
                REQUIRE(node.context() == subnode);
            }
            AND_THEN("is_default should be true") {
                REQUIRE(node.is_default());
            }
            AND_THEN("is_regex should be false") {
                REQUIRE_FALSE(node.is_regex());
            }
            AND_THEN("to_string should return the expected format") {
                REQUIRE(node.to_string() == "default");
            }
        }
        THEN("it should be constructible from string") {
            auto subnode = create_string("foo.bar.baz");
            node = subnode;
            AND_THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "'foo.bar.baz'");
            }
            AND_THEN("the context should be the same as the default") {
                REQUIRE(node.context() == subnode);
            }
            AND_THEN("is_default should be false") {
                REQUIRE_FALSE(node.is_default());
            }
            AND_THEN("is_regex should be false") {
                REQUIRE_FALSE(node.is_regex());
            }
            AND_THEN("to_string should return the expected format") {
                REQUIRE(node.to_string() == "foo.bar.baz");
            }
            AND_THEN("it should be valid") {
                REQUIRE(node.is_valid());
            }
            AND_WHEN("the name is invalid") {
                node = create_string("not$valid");
                THEN("it should not be valid") {
                    REQUIRE_FALSE(node.is_valid());
                }
            }
        }
        THEN("it should be constructible from regex") {
            auto subnode = create_regex("^.*\\.foo\\.com$");
            node = subnode;
            AND_THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "/^.*\\.foo\\.com$/");
            }
            AND_THEN("the context should be the same as the default") {
                REQUIRE(node.context() == subnode);
            }
            AND_THEN("is_default should be false") {
                REQUIRE_FALSE(node.is_default());
            }
            AND_THEN("is_regex should be true") {
                REQUIRE(node.is_regex());
            }
            AND_THEN("to_string should return the expected format") {
                REQUIRE(node.to_string() == "^.*\\.foo\\.com$");
            }
        }
        THEN("it should be constructible from parts") {
            hostname_parts parts = {
                create_number(1234),
                create_name("foo"),
                create_bare_word("com")
            };
            auto& front = boost::get<number>(parts.front());
            auto& back = boost::get<bare_word>(parts.back());
            node = parts;
            AND_THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "1234.foo.com");
            }
            AND_THEN("the context should be the same as the parts") {
                auto context = node.context();
                REQUIRE(context.begin == front.begin);
                REQUIRE(context.end == back.end);
                REQUIRE(context.tree == front.tree);
            }
            AND_THEN("is_default should be false") {
                REQUIRE_FALSE(node.is_default());
            }
            AND_THEN("is_regex should be false") {
                REQUIRE_FALSE(node.is_regex());
            }
            AND_THEN("to_string should return the expected format") {
                REQUIRE(node.to_string() == "1234.foo.com");
            }
        }
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2.context() == node.context());
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3.context() == node.context());
        }
    }
}

SCENARIO("node expression", "[ast]")
{
    auto node = create_node(
        {
            hostname(create_string("foo.bar.baz")),
            hostname(create_default()),
            hostname(create_regex(".*"))
        },
        {
            create_expression(primary(create_function_call("foo")))
        }
    );

    WHEN("using node expression") {
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2 == node);
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3 == node);
        }
        THEN("it should be convertible to context") {
            context ctx = node;
            REQUIRE(ctx == node);
        }
        WHEN("given no body") {
            node.body.clear();
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "node 'foo.bar.baz', default, /.*/ { }");
            }
        }
        WHEN("given a body") {
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "node 'foo.bar.baz', default, /.*/ { foo() }");
            }
        }
        GIVEN("another non-equal object") {
            decltype(node) other;
            THEN("the objects should not be equal") {
                REQUIRE(node != other);
            }
        }
    }
}

SCENARIO("output query operator", "[ast]")
{
    REQUIRE(lexical_cast<std::string>(query_operator::equals) == "==");
    REQUIRE(lexical_cast<std::string>(query_operator::not_equals) == "!=");
    REQUIRE_THROWS(lexical_cast<std::string>(static_cast<query_operator>(numeric_limits<size_t>::max())));
}

SCENARIO("primary query expression", "[ast]")
{
    primary_query_expression node;
    THEN("it should have the expected number of types") {
        REQUIRE(boost::mpl::size<primary_query_expression::types>::value == 2);
    }
    WHEN("using primary query expression") {
        GIVEN("an attribute query") {
            auto subnode = create_attribute_query(
                "foo",
                query_operator::equals,
                primary(create_number(1234))
            );
            node = subnode;
            THEN("the same context should be returned") {
                REQUIRE(node.context() == subnode.context());
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "foo == 1234");
            }
            THEN("it should be movable") {
                auto copied = node;
                auto moved = rvalue_cast(copied);
                REQUIRE(moved.context() == node.context());
            }
        }
        GIVEN("a nested query expression") {
            auto subnode = create_nested_query(create_query(
                primary_query(
                    create_attribute_query(
                        "foo",
                        query_operator::equals,
                        primary(create_string("bar"))
                    )
                ),
                {
                    create_binary_query(
                        binary_query_operator::logical_and,
                        primary_query(
                            create_attribute_query(
                                "baz",
                                query_operator::not_equals,
                                primary(create_bare_word("jam"))
                            )
                        )
                    )
                }
            ));
            node = subnode;
            THEN("the same context should be returned") {
                REQUIRE(node.context() == subnode);
            }
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "(foo == 'bar' and baz != jam)");
            }
        }
    }
}

SCENARIO("output binary query operator", "[ast]")
{
    REQUIRE(lexical_cast<std::string>(binary_query_operator::logical_and) == "and");
    REQUIRE(lexical_cast<std::string>(binary_query_operator::logical_or) == "or");
    REQUIRE_THROWS(lexical_cast<std::string>(static_cast<binary_query_operator>(numeric_limits<size_t>::max())));
}

SCENARIO("query expression", "[ast]")
{
    query_expression node;
    THEN("it should be copy constructible") {
        auto node2 = node;
        REQUIRE(node2.context() == node.context());
    }
    THEN("it should be movable") {
        auto node2 = node;
        auto node3 = rvalue_cast(node2);
        REQUIRE(node3.context() == node.context());
    }
    WHEN("there are no binary operations") {
        node = create_query(primary_query(
            create_attribute_query(
                "foo",
                query_operator::not_equals,
                primary(create_name("bar"))
            )
        ));
        THEN("it should output the expected format") {
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "foo != bar");
            }
        }
        THEN("the context should be the same as the primary") {
            REQUIRE(node.context() == node.primary.context());
        }
    }
    WHEN("there are binary operations") {
        node = create_query(
            primary_query(
                create_attribute_query(
                    "foo",
                    query_operator::not_equals,
                    primary(create_name("bar"))
                )
            ),
            {
                create_binary_query(
                    binary_query_operator::logical_and,
                    primary_query(
                        create_attribute_query(
                            "baz",
                            query_operator::equals,
                            primary(create_number(1234))
                        )
                    )
                ),
                create_binary_query(
                    binary_query_operator::logical_or,
                    primary_query(
                        create_attribute_query(
                            "cake",
                            query_operator::equals,
                            primary(create_string("jam"))
                        )
                    )
                )
            }
        );
        THEN("it should output the expected format") {
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "foo != bar and baz == 1234 or cake == 'jam'");
            }
        }
        THEN("the context should end with the last binary operation") {
            auto context = node.context();
            REQUIRE(context.begin == node.primary.context().begin);
            REQUIRE(context.end == node.operations.back().context().end);
            REQUIRE(context.tree == node.primary.context().tree);
        }
    }
}

SCENARIO("nested query expression", "[ast]")
{
    auto node = create_nested_query(
        create_query(
            primary_query(
                create_attribute_query(
                    "foo",
                    query_operator::equals,
                    primary(create_string("bar"))
                )
            ),
            {
                create_binary_query(
                    binary_query_operator::logical_or,
                    primary_query(
                        create_attribute_query(
                            "baz",
                            query_operator::not_equals,
                            primary(create_bare_word("cakes"))
                        )
                    )
                )
            }
        )
    );
    WHEN("using nested query expression") {
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2 == node);
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3 == node);
        }
        THEN("it should be convertible to context") {
            context ctx = node;
            REQUIRE(ctx == node);
        }
        THEN("it should output the expected format") {
            REQUIRE(lexical_cast<std::string>(node) == "(foo == 'bar' or baz != cakes)");
        }
        GIVEN("another non-equal object") {
            decltype(node) other;
            THEN("the objects should not be equal") {
                REQUIRE(node != other);
            }
        }
    }
}

SCENARIO("collector expression", "[ast]")
{
    auto node = create_collector(
        "Foo",
        false,
        create_query(
            primary_query(
                create_attribute_query(
                    "foo",
                    query_operator::equals,
                    primary(create_number(1234))
                )
            ),
            {
                create_binary_query(
                    binary_query_operator::logical_or,
                    primary_query(
                        create_attribute_query(
                            "baz",
                            query_operator::equals,
                            primary(create_number(5678))
                        )
                    )
                )
            }
        )
    );
    WHEN("using collector expression") {
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2.context() == node.context());
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3.context() == node.context());
        }
        WHEN("there is no query") {
            node.query = boost::none;
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "Foo<||>");
            }
            AND_WHEN("it is an exported collector") {
                node.exported = true;
                THEN("it should output the expected format") {
                    REQUIRE(lexical_cast<std::string>(node) == "Foo<<||>>");
                }

            }
        }
        WHEN("there is no query") {
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "Foo<|foo == 1234 or baz == 5678|>");
            }
            AND_WHEN("it is an exported collector") {
                node.exported = true;
                THEN("it should output the expected format") {
                    REQUIRE(lexical_cast<std::string>(node) == "Foo<<|foo == 1234 or baz == 5678|>>");
                }

            }
        }
        THEN("the context should go from the type to the closing token") {
            auto context = node.context();
            REQUIRE(context.begin == node.type.begin);
            REQUIRE(context.end == node.end);
            REQUIRE(context.tree == node.type.tree);
        }
        GIVEN("another non-equal object") {
            decltype(node) other;
            THEN("the objects should not be equal") {
                REQUIRE(node.context() != other.context());
            }
        }
    }
}

SCENARIO("function expression", "[ast]")
{
    auto node = create_function(
        "foo::bar",
        {
            create_parameter(
                "foo",
                create_postfix(
                    primary(create_type("Integer")),
                    {
                        subexpression(
                            create_access(
                                {
                                    create_expression(primary(create_number(1000))),
                                    create_expression(primary(create_number(2000))),
                                }
                            )
                        )
                    }
                ),
                false,
                create_expression(primary(create_number(1234)))
            ),
            create_parameter("bar")
        },
        {
            create_expression(primary(create_function_call("foo")))
        }
    );

    WHEN("using function expression") {
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2 == node);
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3 == node);
        }
        THEN("it should be convertible to context") {
            context ctx = node;
            REQUIRE(ctx == node);
        }
        WHEN("given no parameters") {
            node.parameters.clear();
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "function foo::bar { foo() }");
            }
            AND_WHEN("given no body") {
                node.body.clear();
                THEN("it should output the expected format") {
                    REQUIRE(lexical_cast<std::string>(node) == "function foo::bar { }");
                }
            }
        }
        WHEN("given parameters") {
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "function foo::bar(Integer[1000, 2000] $foo = 1234, $bar) { foo() }");
            }
            AND_WHEN("given no body") {
                node.body.clear();
                THEN("it should output the expected format") {
                    REQUIRE(lexical_cast<std::string>(node) == "function foo::bar(Integer[1000, 2000] $foo = 1234, $bar) { }");
                }

            }
        }
        GIVEN("another non-equal object") {
            decltype(node) other;
            THEN("the objects should not be equal") {
                REQUIRE(node != other);
            }
        }
    }
}

SCENARIO("unary operator", "[ast]")
{
    REQUIRE(lexical_cast<std::string>(unary_operator::logical_not) == "!");
    REQUIRE(lexical_cast<std::string>(unary_operator::negate) == "-");
    REQUIRE(lexical_cast<std::string>(unary_operator::splat) == "*");
    REQUIRE_THROWS(lexical_cast<std::string>(static_cast<unary_operator>(numeric_limits<size_t>::max())));
}

SCENARIO("hash unary operator", "[ast]")
{
    REQUIRE(hash_value(unary_operator::logical_not) == hash_value(unary_operator::logical_not));
    REQUIRE(hash_value(unary_operator::logical_not) != hash_value(unary_operator::negate));
}

SCENARIO("unary expression", "[ast]")
{
    auto node = create_unary(unary_operator::logical_not, create_postfix(primary(create_variable("foo"))));
    WHEN("using unary expression") {
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2.context() == node.context());
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3.context() == node.context());
        }
        THEN("it should not be a splat expression") {
            REQUIRE_FALSE(node.is_splat());
        }
        THEN("the context should go from the operator to the end of the operand") {
            auto context = node.context();
            REQUIRE(context.begin == node.operator_position);
            REQUIRE(context.end == node.operand.context().end);
            REQUIRE(context.tree == node.operand.context().tree);
        }
        THEN("it should output the expected format") {
            REQUIRE(lexical_cast<std::string>(node) == "!$foo");
        }
        WHEN("the operator is splat") {
            node.operator_ = unary_operator::splat;
            THEN("it should be a splat expression") {
                REQUIRE(node.is_splat());
            }
        }
        GIVEN("another non-equal object") {
            decltype(node) other;
            THEN("the objects should not be equal") {
                REQUIRE(node.context() != other.context());
            }
        }
    }
}

SCENARIO("EPP render expression", "[ast]")
{
    auto node = create_render_expression(
        create_expression(
            create_postfix(primary(create_number(1))),
            {
                create_binary(binary_operator::plus, create_postfix(primary(create_number(1))))
            }
        )
    );
    WHEN("EPP render expression") {
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2 == node);
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3 == node);
        }
        THEN("it should be convertible to context") {
            context ctx = node;
            REQUIRE(ctx == node);
        }
        THEN("it should output the expected format") {
            REQUIRE(lexical_cast<std::string>(node) == "render(1 + 1)");
        }
        GIVEN("another non-equal object") {
            decltype(node) other;
            THEN("the objects should not be equal") {
                REQUIRE(node != other);
            }
        }
    }
}

SCENARIO("EPP render block", "[ast]")
{
    auto node = create_render_block(
        {
            create_expression(
                create_postfix(primary(create_number(1))),
                {
                    create_binary(binary_operator::plus, create_postfix(primary(create_number(1))))
                }
            ),
            create_expression(
                create_postfix(primary(create_number(1))),
                {
                    create_binary(binary_operator::minus, create_postfix(primary(create_number(1))))
                }
            )
        }
    );
    WHEN("EPP render block") {
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2 == node);
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3 == node);
        }
        THEN("it should be convertible to context") {
            context ctx = node;
            REQUIRE(ctx == node);
        }
        THEN("it should output the expected format") {
            REQUIRE(lexical_cast<std::string>(node) == "render({ 1 + 1; 1 - 1 })");
        }
        GIVEN("another non-equal object") {
            decltype(node) other;
            THEN("the objects should not be equal") {
                REQUIRE(node != other);
            }
        }
    }
}

SCENARIO("EPP render string", "[ast]")
{
    auto node = create_render_string("foo");
    WHEN("EPP render string") {
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2 == node);
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3 == node);
        }
        THEN("it should be convertible to context") {
            context ctx = node;
            REQUIRE(ctx == node);
        }
        THEN("it should output the expected format") {
            REQUIRE(lexical_cast<std::string>(node) == "render('foo')");
        }
        GIVEN("another non-equal object") {
            decltype(node) other;
            THEN("the objects should not be equal") {
                REQUIRE(node != other);
            }
        }
    }
}

SCENARIO("produces expression", "[ast]")
{
    auto node = create_produces(
        "Foo",
        "Sql",
        {
            create_attribute(
                "foo",
                attribute_operator::assignment,
                create_expression(primary(create_variable("foo")))
            ),
            create_attribute(
                "bar",
                attribute_operator::assignment,
                create_expression(primary(create_variable("bar")))
            )
        }
    );

    WHEN("using produces expression") {
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2.context() == node.context());
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3.context() == node.context());
        }
        THEN("it should output the expected format") {
            REQUIRE(lexical_cast<std::string>(node) == "Foo produces Sql { foo => $foo, bar => $bar }");
        }
        THEN("the context should be from the resource type to the end") {
            auto context = node.context();
            REQUIRE(context.begin == node.resource.begin);
            REQUIRE(context.end == node.end);
            REQUIRE(context.tree == node.resource.tree);
        }
        WHEN("given no attributes") {
            node.operations.clear();
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "Foo produces Sql { }");
            }
        }
        GIVEN("another non-equal object") {
            decltype(node) other;
            THEN("the objects should not be equal") {
                REQUIRE(node.context() != other.context());
            }
        }
    }
}

SCENARIO("consumes expression", "[ast]")
{
    auto node = create_consumes(
        "Foo",
        "Sql",
        {
            create_attribute(
                "foo",
                attribute_operator::assignment,
                create_expression(primary(create_variable("foo")))
            ),
            create_attribute(
                "bar",
                attribute_operator::assignment,
                create_expression(primary(create_variable("bar")))
            )
        }
    );

    WHEN("using consumes expression") {
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2.context() == node.context());
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3.context() == node.context());
        }
        THEN("it should output the expected format") {
            REQUIRE(lexical_cast<std::string>(node) == "Foo consumes Sql { foo => $foo, bar => $bar }");
        }
        THEN("the context should be from the resource type to the end") {
            auto context = node.context();
            REQUIRE(context.begin == node.resource.begin);
            REQUIRE(context.end == node.end);
            REQUIRE(context.tree == node.resource.tree);
        }
        WHEN("given no attributes") {
            node.operations.clear();
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "Foo consumes Sql { }");
            }
        }
        GIVEN("another non-equal object") {
            decltype(node) other;
            THEN("the objects should not be equal") {
                REQUIRE(node.context() != other.context());
            }
        }
    }
}

SCENARIO("application expression", "[ast]")
{
    auto node = create_application(
        "foo",
        {
            create_parameter(
                "bar",
                create_postfix(
                    primary(create_type("Integer")),
                    {
                        subexpression(
                            create_access(
                                {
                                    create_expression(primary(create_number(1000))),
                                    create_expression(primary(create_number(2000))),
                                }
                            )
                        )
                    }
                ),
                false,
                create_expression(primary(create_number(1234)))
            ),
            create_parameter("baz")
        },
        {
            create_expression(primary(create_function_call("cake")))
        }
    );

    WHEN("using application expression") {
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2 == node);
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3 == node);
        }
        THEN("it should be convertible to context") {
            context ctx = node;
            REQUIRE(ctx == node);
        }
        WHEN("given no parameters") {
            node.parameters.clear();
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "application foo { cake() }");
            }
            AND_WHEN("given no body") {
                node.body.clear();
                THEN("it should output the expected format") {
                    REQUIRE(lexical_cast<std::string>(node) == "application foo { }");
                }

            }
        }
        WHEN("given parameters") {
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "application foo(Integer[1000, 2000] $bar = 1234, $baz) { cake() }");
            }
            AND_WHEN("given no body") {
                node.body.clear();
                THEN("it should output the expected format") {
                    REQUIRE(lexical_cast<std::string>(node) == "application foo(Integer[1000, 2000] $bar = 1234, $baz) { }");
                }

            }
        }
        GIVEN("another non-equal object") {
            decltype(node) other;
            THEN("the objects should not be equal") {
                REQUIRE(node != other);
            }
        }
    }
}

SCENARIO("site expression", "[ast]")
{
    auto node = create_site(
        {
            create_expression(primary(
                create_resource(
                    resource_status::realized,
                    create_postfix(primary(create_name("app"))),
                    {
                        create_resource_body(
                            create_expression(primary(create_name("lamp"))),
                            {
                                create_attribute(
                                    "nodes",
                                    attribute_operator::assignment,
                                    create_expression(primary(
                                        create_hash(
                                        {
                                            make_pair(
                                                create_expression(
                                                    create_postfix(
                                                        primary(create_type("Node")),
                                                        {
                                                            subexpression(create_access({ create_expression(primary(create_name("foo"))) }))
                                                        }
                                                    )
                                                ),
                                                create_expression(primary(
                                                    create_array(
                                                    {
                                                        create_expression(
                                                            create_postfix(
                                                            primary(create_type("Lamp::Db")),
                                                            {
                                                                subexpression(create_access({ create_expression(primary(create_name("foo"))) }))
                                                            }
                                                        ))
                                                    }
                                                ))
                                            ))
                                        }
                                    )))
                                )
                            }
                        )
                    }
                ))
            )
        }
    );

    WHEN("using node expression") {
        THEN("it should be copy constructible") {
            auto node2 = node;
            REQUIRE(node2 == node);
        }
        THEN("it should be movable") {
            auto node2 = node;
            auto node3 = rvalue_cast(node2);
            REQUIRE(node3 == node);
        }
        THEN("it should be convertible to context") {
            context ctx = node;
            REQUIRE(ctx == node);
        }
        WHEN("given no body") {
            node.body.clear();
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "site { }");
            }
        }
        WHEN("given a body") {
            THEN("it should output the expected format") {
                REQUIRE(lexical_cast<std::string>(node) == "site { app { lamp: nodes => {Node[foo] => [Lamp::Db[foo]]} } }");
            }
        }
        GIVEN("another non-equal object") {
            decltype(node) other;
            THEN("the objects should not be equal") {
                REQUIRE(node != other);
            }
        }
    }
}

SCENARIO("syntax tree", "[ast]")
{
    auto tree = create_syntax_tree(
        "foo",
        boost::none,
        {
            create_expression(
                create_postfix(primary(create_variable("foo"))),
                {
                    create_binary(binary_operator::assignment, create_postfix(primary(create_number(1))))
                }
            ),
            create_expression(
                create_postfix(primary(create_variable("foo"))),
                {
                    create_binary(binary_operator::minus, create_postfix(primary(create_number(1))))
                }
            )
        }
    );
    THEN("the source should be empty") {
        REQUIRE(tree->source().empty());
    }
    THEN("the module should not be null") {
        REQUIRE(tree->module());
    }
    THEN("the path should be set") {
        REQUIRE(tree->path() == "foo");
        REQUIRE(tree->shared_path());
    }
    THEN("it should output the expected format") {
        REQUIRE(lexical_cast<std::string>(*tree) == "$foo = 1; $foo - 1");
    }
    WHEN("parameters are specified") {
        tree->parameters = vector<parameter>({
            create_parameter("bar", create_postfix(primary(create_type("Integer"))))
        });
        THEN("it should output the expected format") {
            REQUIRE(lexical_cast<std::string>(*tree) == "|Integer $bar| $foo = 1; $foo - 1");
        }
    }
}
