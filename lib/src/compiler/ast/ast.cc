#include <puppet/compiler/ast/ast.hpp>
#include <puppet/compiler/ast/visitors/type.hpp>
#include <puppet/compiler/ast/visitors/validation.hpp>
#include <puppet/compiler/lexer/lexer.hpp>
#include <puppet/utility/regex.hpp>
#include <puppet/cast.hpp>
#include <boost/lexical_cast.hpp>
#include <yaml-cpp/yaml.h>

using namespace std;
namespace x3 = boost::spirit::x3;

namespace puppet { namespace compiler { namespace ast {

    struct printer : boost::static_visitor<>
    {
        explicit printer(ostream& os) :
            _os(os)
        {
        }

        template <typename T>
        void operator()(const T& operand) const
        {
            _os << operand;
        }

        template <typename T>
        void operator()(x3::forward_ast<T> const& operand) const
        {
            _os << operand.get();
        }

     private:
        ostream& _os;
    };

    struct context_visitor : boost::static_visitor<ast::context>
    {
        template <typename T>
        auto operator()(T const& node) const -> decltype(node.context())
        {
            return node.context();
        }

        template <typename T>
        auto operator()(T const& node) const -> decltype(node.get().context())
        {
            return node.get().context();
        }

        template <typename T>
        auto operator()(T const& node) const -> decltype(static_cast<ast::context const&>(node))
        {
            return node;
        }
    };

    template <typename Container>
    void pretty_print(ostream& os, Container const& container, std::string const& delimiter = ", ")
    {
        bool first = true;
        for (auto const& element : container) {
            if (first) {
                first = false;
            } else {
                os << delimiter;
            }
            os << element;
        }
    }

    static void escape(ostream& os, std::string const& text, char const* escapes = nullptr)
    {
        for (auto c : text) {
            char replacement = 0;
            for (auto ptr = escapes; ptr && *ptr && !replacement; ++ptr) {
                switch (*ptr) {
                    case 'r':
                        if (c == '\r') {
                            replacement = 'r';
                        }
                        break;

                    case 'n':
                        if (c == '\n') {
                            replacement = 'n';
                        }
                        break;

                    case 't':
                        if (c == '\t') {
                            replacement = 't';
                        }
                        break;

                    case 's':
                        // Keep the space as is
                        break;

                    case 'u':
                        // Keep any unicode characters as is
                        break;

                    default:
                        if (c == *ptr) {
                            replacement = c;
                        }
                        break;
                }
            }

            if (replacement) {
                os << '\\' << replacement;
            } else {
                os << c;
            }

        }
    }

    bool operator==(context const& left, context const& right)
    {
        return left.tree == right.tree &&
               left.begin == right.begin &&
               left.end == right.end;
    }

    bool operator!=(context const& left, context const& right)
    {
        return !(left == right);
    }

    ostream& operator<<(ostream& os, undef const&)
    {
        os << "undef";
        return os;
    }

    ostream& operator<<(ostream& os, defaulted const&)
    {
        os << "default";
        return os;
    }

    ostream& operator<<(ostream& os, boolean const& node)
    {
        os << (node.value ? "true" : "false");
        return os;
    }

    bool operator==(boolean const& left, boolean const& right)
    {
        return static_cast<context const&>(left) == static_cast<context const&>(right) &&
               left.value == right.value;
    }

    bool operator!=(boolean const& left, boolean const& right)
    {
        return !(left == right);
    }

    ostream& operator<<(ostream& os, number const& node)
    {
        os << node.value;
        return os;
    }

    bool operator==(number const& left, number const& right)
    {
        return static_cast<context const&>(left) == static_cast<context const&>(right) &&
               left.value == right.value;
    }

    bool operator!=(number const& left, number const& right)
    {
        return !(left == right);
    }

    ostream& operator<<(ostream& os, ast::string const& node)
    {
        os << '\'';
        escape(os, node.value, lexer::SQ_ESCAPES);
        os << '\'';
        return os;
    }

    bool operator==(string const& left, string const& right)
    {
        return static_cast<context const&>(left) == static_cast<context const&>(right) &&
               left.value == right.value &&
               left.format == right.format;
    }

    bool operator!=(string const& left, string const& right)
    {
        return !(left == right);
    }

    ostream& operator<<(ostream& os, regex const& node)
    {
        os << "/" << node.value << "/";
        return os;
    }

    bool operator==(regex const& left, regex const& right)
    {
        return static_cast<context const&>(left) == static_cast<context const&>(right) &&
               left.value == right.value;
    }

    bool operator!=(regex const& left, regex const& right)
    {
        return !(left == right);
    }

    ostream& operator<<(ostream& os, variable const& node)
    {
        os << "$" << node.name;
        return os;
    }

    bool operator==(variable const& left, variable const& right)
    {
        return static_cast<context const&>(left) == static_cast<context const&>(right) &&
               left.name == right.name;
    }

    bool operator!=(variable const& left, variable const& right)
    {
        return !(left == right);
    }

    ostream& operator<<(ostream& os, name const& node)
    {
        os << node.value;
        return os;
    }

    bool operator==(name const& left, name const& right)
    {
        return static_cast<context const&>(left) == static_cast<context const&>(right) &&
               left.value == right.value;
    }

    bool operator!=(name const& left, name const& right)
    {
        return !(left == right);
    }

    ostream& operator<<(ostream& os, bare_word const& node)
    {
        os << node.value;
        return os;
    }

    bool operator==(bare_word const& left, bare_word const& right)
    {
        return static_cast<context const&>(left) == static_cast<context const&>(right) &&
               left.value == right.value;
    }

    bool operator!=(bare_word const& left, bare_word const& right)
    {
        return !(left == right);
    }

    ostream& operator<<(ostream& os, type const& node)
    {
        os << node.name;
        return os;
    }

    bool operator==(type const& left, type const& right)
    {
        return static_cast<context const&>(left) == static_cast<context const&>(right) &&
               left.name == right.name;
    }

    bool operator!=(type const& left, type const& right)
    {
        return !(left == right);
    }

    ast::context basic_expression::context() const
    {
        return boost::apply_visitor(context_visitor(), *this);
    }

    bool basic_expression::is_splat() const
    {
        // Try for nested expression
        auto nested = boost::get<x3::forward_ast<nested_expression>>(this);
        if (nested) {
            return nested->get().expression.is_splat();
        }

        auto unary = boost::get<x3::forward_ast<unary_expression>>(this);
        return unary && unary->get().operator_ == unary_operator::splat;
    }

    bool basic_expression::is_default() const
    {
        // Try for nested expression
        auto nested = boost::get<x3::forward_ast<nested_expression>>(this);
        if (nested) {
            return nested->get().expression.is_default();
        }
        return static_cast<bool>(boost::get<defaulted>(this));
    }

    ostream& operator<<(ostream& os, basic_expression const& node)
    {
        printer visitor{os};
        boost::apply_visitor(visitor, node);
        return os;
    }

    ast::context postfix_operation::context() const
    {
        return boost::apply_visitor(context_visitor(), *this);
    }

    ostream& operator<<(ostream& os, postfix_operation const& node)
    {
        printer visitor{os};
        boost::apply_visitor(visitor, node);
        return os;
    }

    ast::context postfix_expression::context() const
    {
        auto context = operand.context();
        if (!operations.empty()) {
            context.end = operations.back().context().end;
        }
        return context;
    }

    void postfix_expression::validate_type() const
    {
        visitors::type visitor;
        visitor.visit(*this);
    }

    bool postfix_expression::is_splat() const
    {
        // Check the operand
        return operand.is_splat();
    }

    bool postfix_expression::is_default() const
    {
        if (!operations.empty()) {
            return false;
        }

        // Check the operand
        return operand.is_default();
    }

    ostream& operator<<(ostream& os, postfix_expression const& node)
    {
        os << node.operand;
        for (auto const& operation : node.operations) {
            os << operation;
        }
        return os;
    }

    ostream& operator<<(ostream& os, binary_operator const& node)
    {
        switch (node) {
            case binary_operator::in:
                os << "in";
                break;
            case binary_operator::match:
                os << "=~";
                break;
            case binary_operator::not_match:
                os << "!~";
                break;
            case binary_operator::multiply:
                os << "*";
                break;
            case binary_operator::divide:
                os << "/";
                break;
            case binary_operator::modulo:
                os << "%";
                break;
            case binary_operator::plus:
                os << "+";
                break;
            case binary_operator::minus:
                os << "-";
                break;
            case binary_operator::left_shift:
                os << "<<";
                break;
            case binary_operator::right_shift:
                os << ">>";
                break;
            case binary_operator::equals:
                os << "==";
                break;
            case binary_operator::not_equals:
                os << "!=";
                break;
            case binary_operator::greater_than:
                os << ">";
                break;
            case binary_operator::greater_equals:
                os << ">=";
                break;
            case binary_operator::less_than:
                os << "<";
                break;
            case binary_operator::less_equals:
                os << "<=";
                break;
            case binary_operator::logical_and:
                os << "and";
                break;
            case binary_operator::logical_or:
                os << "or";
                break;
            case binary_operator::assignment:
                os << '=';
                break;
            default:
                throw runtime_error("invalid binary operator.");
        }
        return os;
    }

    unsigned int precedence(binary_operator op)
    {
        // Return the precedence, 1-based and low to high
        switch (op) {
            case binary_operator::assignment:
                return 1;

            case binary_operator::logical_or:
                return 2;

            case binary_operator::logical_and:
                return 3;

            case binary_operator::greater_than:
            case binary_operator::greater_equals:
            case binary_operator::less_than:
            case binary_operator::less_equals:
                return 4;

            case binary_operator::equals:
            case binary_operator::not_equals:
                return 5;

            case binary_operator::left_shift:
            case binary_operator::right_shift:
                return 6;

            case binary_operator::plus:
            case binary_operator::minus:
                return 7;

            case binary_operator::multiply:
            case binary_operator::divide:
            case binary_operator::modulo:
                return 8;

            case binary_operator::match:
            case binary_operator::not_match:
                return 9;

            case binary_operator::in:
                return 10;

            default:
                break;
        }

        throw runtime_error("invalid binary operator.");
    }

    bool is_right_associative(binary_operator op)
    {
        return op == binary_operator::assignment;
    }

    size_t hash_value(binary_operator op)
    {
        using type = typename underlying_type<binary_operator>::type;
        std::hash<type> hasher;
        return hasher(static_cast<type>(op));
    }

    ast::context binary_operation::context() const
    {
        auto context = operand.context();
        context.begin = operator_position;
        return context;
    }

    ostream& operator<<(ostream& os, binary_operation const& operation)
    {
        os << " " << operation.operator_ << " " << operation.operand;
        return os;
    }

    ast::context expression::context() const
    {
        auto context = operand.context();
        if (!operations.empty()) {
            context.end = operations.back().context().end;
        }
        return context;
    }

    bool expression::is_splat() const
    {
        // A splat expression never has binary operations
        if (!operations.empty()) {
            return false;
        }
        // Check the first operand
        return operand.is_splat();
    }

    bool expression::is_default() const
    {
        // A default expression never has binary operations
        if (!operations.empty()) {
            return false;
        }
        // Check the first operand
        return operand.is_default();
    }

    ostream& operator<<(ostream& os, expression const& node)
    {
        os << node.operand;
        for (auto const& operation : node.operations) {
            os << operation;
        }
        return os;
    }

    ostream& operator<<(ostream& os, break_statement const& node)
    {
        os << "break";
        return os;
    }

    ast::context statement::context() const
    {
        return boost::apply_visitor(context_visitor(), *this);
    }

    void statement::validate(bool effective) const
    {
        visitors::validation visitor;
        visitor.visit(*this, effective);
    }

    bool statement::is_transfer_statement() const
    {
        return boost::get<break_statement>(this);
    }

    ostream& operator<<(ostream& os, statement const& node)
    {
        printer visitor{os};
        boost::apply_visitor(visitor, node);
        return os;
    }

    ostream& operator<<(ostream& os, literal_string_text const& node)
    {
        escape(os, node.text, lexer::DQ_ESCAPES);
        return os;
    }

    bool operator==(literal_string_text const& left, literal_string_text const& right)
    {
        return static_cast<context const&>(left) == static_cast<context const&>(right) &&
               left.text == right.text;
    }

    bool operator!=(literal_string_text const& left, literal_string_text const& right)
    {
        return !(left == right);
    }

    ast::context interpolated_string_part::context() const
    {
        return boost::apply_visitor(context_visitor(), *this);
    }

    ostream& operator<<(ostream& os, interpolated_string const& node)
    {
        printer visitor{ os };

        os << "\"";
        for (auto& part : node.parts) {
            bool is_expression = boost::get<x3::forward_ast<expression>>(&part);
            if (is_expression) {
                os << "${";
            }
            boost::apply_visitor(visitor, part);
            if (is_expression) {
                os << "}";
            }
        }
        os << "\"";
        return os;
    }

    bool operator==(interpolated_string const& left, interpolated_string const& right)
    {
        return static_cast<context const&>(left) == static_cast<context const&>(right);
               //left.text == right.text;
    }

    bool operator!=(interpolated_string const& left, interpolated_string const& right)
    {
        return !(left == right);
    }

    ostream& operator<<(ostream& os, array const& node)
    {
        os << '[';
        pretty_print(os, node.elements, ", ");
        os << ']';
        return os;
    }

    ostream& operator<<(ostream& os, ast::pair const& node)
    {
        os << node.first << " => " << node.second;
        return os;
    }

    ostream& operator<<(ostream& os, hash const& node)
    {
        os << "{";
        pretty_print(os, node.elements, ", ");
        os << "}";
        return os;
    }

    ostream& operator<<(ostream& os, ast::proposition const& proposition)
    {
        pretty_print(os, proposition.options, ", ");
        os << ": ";
        if (proposition.body.empty()) {
            os << "{ }";
        } else {
            os << "{ ";
            pretty_print(os, proposition.body, "; ");
            os << " }";
        }
        return os;
    }

    ostream& operator<<(ostream& os, case_expression const& node)
    {
        os << "case " << node.conditional << " { ";
        pretty_print(os, node.propositions, " ");
        os << " }";
        return os;
    }

    ostream& operator<<(ostream& os, ast::else_ const& else_)
    {
        os << "else";
        if (else_.body.empty()) {
            os << " { }";
        } else {
            os << " { ";
            pretty_print(os, else_.body, "; ");
            os << " }";
        }
        return os;
    }

    ostream& operator<<(ostream& os, ast::elsif const& elsif)
    {
        os << "elsif " << elsif.conditional;
        if (elsif.body.empty()) {
            os << " { }";
        } else {
            os << " { ";
            pretty_print(os, elsif.body, "; ");
            os << " }";
        }
        return os;
    }

    ast::context if_expression::context() const
    {
        auto context = conditional.context();
        context.begin = begin;
        context.end = end;

        if (else_) {
            context.end = else_->end;
        } else if (!elsifs.empty()) {
            context.end = elsifs.back().end;
        }
        return context;
    }

    ostream& operator<<(ostream& os, if_expression const& node)
    {
        os << "if " << node.conditional;
        if (node.body.empty()) {
            os << " { }";
        } else {
            os << " { ";
            pretty_print(os, node.body, "; ");
            os << " }";
        }
        if (!node.elsifs.empty()) {
            os << " ";
            pretty_print(os, node.elsifs, " ");
        }
        if (node.else_) {
            os << " " << *node.else_;
        }
        return os;
    }

    ast::context unless_expression::context() const
    {
        auto context = conditional.context();
        context.begin = begin;
        context.end = else_ ? else_->end : end;
        return context;
    }

    ostream& operator<<(ostream& os, unless_expression const& node)
    {
        os << "unless " << node.conditional;
        if (node.body.empty()) {
            os << " { }";
        } else {
            os << " { ";
            pretty_print(os, node.body, "; ");
            os << " }";
        }
        if (node.else_) {
            os << " " << *node.else_;
        }
        return os;
    }

    ast::context parameter::context() const
    {
        ast::context context;
        if (type) {
            context.begin = type->context().begin;
        } else if (captures) {
            context.begin = *captures;
        } else {
            context.begin = variable.begin;
        }

        if (default_value) {
            context.end = default_value->context().end;
        } else {
            context.end = variable.end;
        }

        context.tree = variable.tree;
        return context;
    }

    ostream& operator<<(ostream& os, parameter const& node)
    {
        if (node.type) {
            os << *node.type << " ";
        }
        if (node.captures) {
            os << '*';
        }
        os << node.variable;
        if (node.default_value) {
            os << " = " << *node.default_value;
        }
        return os;
    }

    ostream& operator<<(ostream& os, lambda_expression const& node)
    {
        os << "|";
        pretty_print(os, node.parameters, ", ");
        os << "|";
        if (node.body.empty()) {
            os << " { }";
        } else {
            os << " { ";
            pretty_print(os, node.body, "; ");
            os << " }";
        }
        return os;
    }

    ast::context function_call_expression::context() const
    {
        ast::context context = function;

        if (lambda) {
            context.end = lambda->end;
        } else if (end) {
            context.end = *end;
        } else if (!arguments.empty()) {
            context.end = arguments.back().context().end;
        }
        return context;
    }

    ostream& operator<<(ostream& os, function_call_expression const& node)
    {
        os << node.function << "(";
        pretty_print(os, node.arguments, ", ");
        os << ")";
        if (node.lambda) {
            os << " " << *node.lambda;
        }
        return os;
    }

    ast::context new_expression::context() const
    {
        ast::context context = type.context();

        if (lambda) {
            context.end = lambda->end;
        } else {
            context.end = end;
        }
        return context;
    }

    ostream& operator<<(ostream& os, new_expression const& node)
    {
        os << node.type << "(";
        pretty_print(os, node.arguments, ", ");
        os << ")";
        if (node.lambda) {
            os << " " << *node.lambda;
        }
        return os;
    }

    ostream& operator<<(ostream& os, epp_render_expression const& node)
    {
        os << "render(" << node.expression << ")";
        return os;
    }

    ostream& operator<<(ostream& os, epp_render_block const& node)
    {
        os << "render({ ";
        pretty_print(os, node.block, "; ");
        os << " })";
        return os;
    }

    ostream& operator<<(ostream& os, epp_render_string const& node)
    {
        os << "render('";
        escape(os, node.string, lexer::SQ_ESCAPES);
        os << "')";
        return os;
    }

    ostream& operator<<(ostream& os, unary_operator const& node)
    {
        switch (node) {
            case unary_operator::negate:
                os << '-';
                break;

            case unary_operator::logical_not:
                os << '!';
                break;

            case unary_operator::splat:
                os << '*';
                break;

            default:
                throw runtime_error("invalid unary operator.");
        }
        return os;
    }

    size_t hash_value(unary_operator const& oper)
    {
        using type = typename underlying_type<unary_operator>::type;
        std::hash<type> hasher;
        return hasher(static_cast<type>(oper));
    }

    ast::context unary_expression::context() const
    {
        ast::context context = operand.context();
        context.begin = operator_position;
        return context;
    }

    ostream& operator<<(ostream& os, unary_expression const& node)
    {
        os << node.operator_ << node.operand;
        return os;
    }

    ostream& operator<<(ostream& os, nested_expression const& node)
    {
        os << '(' << node.expression << ')';
        return os;
    }

    ostream& operator<<(ostream& os, selector_expression const& node)
    {
        os << " ? { ";
        pretty_print(os, node.cases, ", ");
        os << " }";
        return os;
    }

    ostream& operator<<(ostream& os, access_expression const& node)
    {
        os << '[';
        pretty_print(os, node.arguments, ", ");
        os << ']';
        return os;
    }

    ast::context method_call_expression::context() const
    {
        ast::context context = method;
        context.begin = begin;

        if (lambda) {
            context.end = lambda->end;
        } else if (end) {
            context.end = *end;
        } else if (!arguments.empty()) {
            context.end = arguments.back().context().end;
        }
        return context;
    }

    ostream& operator<<(ostream& os, method_call_expression const& node)
    {
        os << "." << node.method << "(";
        pretty_print(os, node.arguments, ", ");
        os << ")";
        if (node.lambda) {
            os << " " << *node.lambda;
        }
        return os;
    }

    ostream& operator<<(ostream& os, class_statement const& node)
    {
        os << "class " << node.name;
        if (!node.parameters.empty()) {
            os << "(";
            pretty_print(os, node.parameters, ", ");
            os << ")";
        }
        if (node.parent) {
            os << " inherits " << *node.parent;
        }
        if (node.body.empty()) {
            os << " { }";
        } else {
            os << " { ";
            pretty_print(os, node.body, "; ");
            os << " }";
        }
        return os;
    }

    ostream& operator<<(ostream& os, defined_type_statement const& node)
    {
        os << "define " << node.name;
        if (!node.parameters.empty()) {
            os << "(";
            pretty_print(os, node.parameters, ", ");
            os << ")";
        }
        if (node.body.empty()) {
            os << " { }";
        } else {
            os << " { ";
            pretty_print(os, node.body, "; ");
            os << " }";
        }
        return os;
    }

    ast::context hostname::context() const
    {
        if (auto ptr = boost::get<defaulted>(this)) {
            return *ptr;
        }
        if (auto ptr = boost::get<string>(this)) {
            return *ptr;
        }
        if (auto ptr = boost::get<regex>(this)) {
            return *ptr;
        }
        auto& parts = boost::get<hostname_parts>(*this);
        auto context = boost::apply_visitor(context_visitor(), parts.at(0));
        context.end = boost::apply_visitor(context_visitor(), parts.back()).end;
        return context;
    }

    bool hostname::is_default() const
    {
        return boost::get<defaulted>(this);
    }

    bool hostname::is_regex() const
    {
        return boost::get<regex>(this);
    }

    bool hostname::is_valid() const
    {
        static const utility::regex illegal{ R"([^-\w.])" };

        if (is_default() || is_regex()) {
            return true;
        }
        return !illegal.search(to_string());
    }

    struct hostname_visitor : boost::static_visitor<>
    {
        explicit hostname_visitor(ostream& ss, bool as_string = false) :
            _ss(ss),
            _as_string(as_string)
        {
        }

        void operator()(defaulted const& d)
        {
            _ss << d;
        }

        void operator()(regex const& r)
        {
            if (_as_string) {
                _ss << r.value;
            } else {
                _ss << r;
            }
        }

        void operator()(string const& s)
        {
            if (_as_string) {
                _ss << s.value;
            } else {
                _ss << s;
            }
        }

        void operator()(hostname_parts const& parts)
        {
            pretty_print(_ss, parts, ".");
        }

     private:
        ostream& _ss;
        bool _as_string;
    };

    std::string hostname::to_string() const
    {
        ostringstream ss;
        hostname_visitor visitor{ss, true};
        boost::apply_visitor(visitor, *this);
        return ss.str();
    }

    ostream& operator<<(ostream& os, hostname const& node)
    {
        hostname_visitor visitor{os};
        boost::apply_visitor(visitor, node);
        return os;
    }

    ostream& operator<<(ostream& os, node_statement const& node)
    {
        os << "node ";
        pretty_print(os, node.hostnames, ", ");
        if (node.body.empty()) {
            os << " { }";
        } else {
            os << " { ";
            pretty_print(os, node.body, "; ");
            os << " }";
        }
        return os;
    }

    ostream& operator<<(ostream& os, function_statement const& node)
    {
        os << "function " << node.name;
        if (!node.parameters.empty()) {
            os << "(";
            pretty_print(os, node.parameters, ", ");
            os << ")";
        }
        if (node.body.empty()) {
            os << " { }";
        } else {
            os << " { ";
            pretty_print(os, node.body, "; ");
            os << " }";
        }
        return os;
    }

     ostream& operator<<(ostream& os, attribute_operator const& node)
    {
        switch (node) {
            case attribute_operator::assignment:
                os << "=>";
                break;

            case attribute_operator::append:
                os << "+>";
                break;

            default:
                throw runtime_error("invalid attribute operator.");
        }
        return os;
    }

    ast::context attribute_operation::context() const
    {
        ast::context context = name;
        context.end = value.context().end;
        return context;
    }

    ostream& operator<<(ostream& os, ast::attribute_operation const& operation)
    {
        os << operation.name << " " << operation.operator_ << " " << operation.value;
        return os;
    }

    ast::context produces_statement::context() const
    {
        ast::context context = resource;
        context.end = end;
        return context;
    }

    ostream& operator<<(ostream& os, produces_statement const& node)
    {
        os << node.resource << " produces " << node.capability;
        if (node.operations.empty()) {
            os << " { }";
        } else {
            os << " { ";
            pretty_print(os, node.operations, ", ");
            os << " }";
        }
        return os;
    }

    ast::context consumes_statement::context() const
    {
        ast::context context = resource;
        context.end = end;
        return context;
    }

    ostream& operator<<(ostream& os, consumes_statement const& node)
    {
        os << node.resource << " consumes " << node.capability;
        if (node.operations.empty()) {
            os << " { }";
        } else {
            os << " { ";
            pretty_print(os, node.operations, ", ");
            os << " }";
        }
        return os;
    }

    ostream& operator<<(ostream& os, application_statement const& node)
    {
        os << "application " << node.name;
        if (!node.parameters.empty()) {
            os << "(";
            pretty_print(os, node.parameters, ", ");
            os << ")";
        }
        if (node.body.empty()) {
            os << " { }";
        } else {
            os << " { ";
            pretty_print(os, node.body, ", ");
            os << " }";
        }
        return os;
    }

    ostream& operator<<(ostream& os, site_statement const& node)
    {
        os << "site { ";
        if (node.body.empty()) {
            os << "}";
        } else {
            pretty_print(os, node.body, ", ");
            os << " }";
        }
        return os;
    }

    ast::context type_alias_statement::context() const
    {
        ast::context context;
        context.begin = begin;
        context.end = type.context().end;
        context.tree = alias.tree;
        return context;
    }

    ostream& operator<<(ostream& os, type_alias_statement const& node)
    {
        os << "type " << node.alias << " = " << node.type;
        return os;
    }

    ast::context function_call_statement::context() const
    {
        ast::context context = function;

        if (lambda) {
            context.end = lambda->end;
        } else if (!arguments.empty()) {
            context.end = arguments.back().context().end;
        }
        return context;
    }

    ostream& operator<<(ostream& os, function_call_statement const& node)
    {
        os << node.function;
        if (!node.arguments.empty()) {
            os << " ";
        }
        pretty_print(os, node.arguments, ", ");
        if (node.lambda) {
            os << " " << *node.lambda;
        }
        return os;
    }

    ostream& operator<<(ostream& os, resource_status status)
    {
        switch (status) {
            case resource_status::realized:
                os << "realized";
                break;

            case resource_status::virtualized:
                os << "virtual";
                break;

            case resource_status::exported:
                os << "exported";
                break;

            default:
                throw runtime_error("unexpected resource status.");
        }
        return os;
    }

    ast::context resource_body::context() const
    {
        auto context = title.context();
        if (!operations.empty()) {
            context.end = operations.back().context().end;
        }
        return context;
    }

    ostream& operator<<(ostream& os, resource_body const& node)
    {
        os << node.title << ": ";
        pretty_print(os, node.operations, ", ");
        return os;
    }

    ostream& operator<<(ostream& os, resource_declaration_expression const& node)
    {
        switch (node.status) {
            case resource_status::realized:
                break;

            case resource_status::virtualized:
                os << "@";
                break;

            case resource_status::exported:
                os << "@@";
                break;

            default:
                throw runtime_error("invalid resource status.");
        }
        os << node.type << " { ";
        pretty_print(os, node.bodies, "; ");
        os << " }";
        return os;
    }

    ast::context resource_override_reference::context() const
    {
        return boost::apply_visitor(context_visitor(), *this);
    }

    ostream& operator<<(ostream& os, resource_override_reference const& node)
    {
        printer visitor{os};
        boost::apply_visitor(visitor, node);
        return os;
    }

    ostream& operator<<(ostream& os, resource_override_expression const& node)
    {
        os << node.reference;
        if (node.operations.empty()) {
            os << " { }";
        } else {
            os << " { ";
            pretty_print(os, node.operations, ", ");
            os << " }";
        }
        return os;
    }

    ostream& operator<<(ostream& os, resource_defaults_expression const& node)
    {
        os << node.type;
        if (node.operations.empty()) {
            os << " { }";
        } else {
            os << " { ";
            pretty_print(os, node.operations, ", ");
            os << " }";
        }
        return os;
    }

    ostream& operator<<(ostream& os, query_operator const& node)
    {
        switch (node) {
            case query_operator::equals:
                os << "==";
                break;

            case query_operator::not_equals:
                os << "!=";
                break;

            default:
                throw runtime_error("invalid attribute operator.");
        }
        return os;
    }

    ast::context attribute_query::context() const
    {
        ast::context context = attribute;
        context.end = value.context().end;
        return context;
    }

    ostream& operator<<(ostream& os, attribute_query const& node)
    {
        os << node.attribute << " " << node.operator_ << " " << node.value;
        return os;
    }

    ast::context basic_query_expression::context() const
    {
        return boost::apply_visitor(context_visitor(), *this);
    }

    ostream& operator<<(ostream& os, basic_query_expression const& node)
    {
        printer visitor{os};
        boost::apply_visitor(visitor, node);
        return os;
    }

    ostream& operator<<(ostream& os, binary_query_operator const& node)
    {
        switch (node) {
            case binary_query_operator::logical_and:
                os << "and";
                break;

            case binary_query_operator::logical_or:
                os << "or";
                break;

            default:
                throw runtime_error("invalid binary query operator.");
        }
        return os;
    }

    ast::context binary_query_operation::context() const
    {
        auto context = operand.context();
        context.begin = operator_position;
        return context;
    }

    ostream& operator<<(ostream& os, binary_query_operation const& operation)
    {
        os << " " << operation.operator_ << " " << operation.operand;
        return os;
    }

    ast::context query_expression::context() const
    {
        auto context = operand.context();
        if (!operations.empty()) {
            context.end = operations.back().context().end;
        }
        return context;
    }

    ostream& operator<<(ostream& os, query_expression const& node)
    {
        os << node.operand;
        for (auto const& operation : node.operations) {
            os << operation;
        }
        return os;
    }

    ostream& operator<<(ostream& os, nested_query_expression const& node)
    {
        os << '(' << node.expression << ')';
        return os;
    }

    ast::context collector_expression::context() const
    {
        ast::context context = type;
        context.end = end;
        return context;
    }

    ostream& operator<<(ostream& os, collector_expression const& node)
    {
        os << node.type << (node.exported ? "<<|" : "<|");
        if (node.query) {
            os << *node.query;
        }
        os << (node.exported ? "|>>" : "|>");
        return os;
    }

    ast::context relationship_expression::context() const
    {
        return boost::apply_visitor(context_visitor(), *this);
    }

    ostream& operator<<(ostream& os, relationship_expression const& node)
    {
        printer visitor{os};
        boost::apply_visitor(visitor, node);
        return os;
    }

    ostream& operator<<(ostream& os, relationship_operator op)
    {
        switch (op) {
            case relationship_operator::in_edge:
                os << "->";
                break;
            case relationship_operator::in_edge_subscribe:
                os << "~>";
                break;
            case relationship_operator::out_edge:
                os << "<-";
                break;
            case relationship_operator::out_edge_subscribe:
                os << "<~";
                break;
            default:
                throw runtime_error("invalid relationship operator.");
        }
        return os;
    }

    size_t hash_value(relationship_operator op)
    {
        using type = typename underlying_type<relationship_operator>::type;
        std::hash<type> hasher;
        return hasher(static_cast<type>(op));
    }

    ast::context relationship_operation::context() const
    {
        auto context = operand.context();
        context.begin = operator_position;
        return context;
    }

    ostream& operator<<(ostream& os, relationship_operation const& node)
    {
        os << " " << node.operator_ << " " << node.operand;
        return os;
    }

    ast::context relationship_statement::context() const
    {
        auto context = operand.context();
        if (!operations.empty()) {
            context.end = operations.back().context().end;
        }
        return context;
    }

    ostream& operator<<(ostream& os, relationship_statement const& node)
    {
        os << node.operand;
        for (auto const& operation : node.operations) {
            os << operation;
        }
        return os;
    }

    std::string const& syntax_tree::path() const
    {
        return *_path;
    }

    shared_ptr<std::string> const& syntax_tree::shared_path() const
    {
        return _path;
    }

    std::string const& syntax_tree::source() const
    {
        return _source;
    }

    void syntax_tree::source(std::string source)
    {
        _source = rvalue_cast(source);
    }

    compiler::module const* syntax_tree::module() const
    {
        return _module;
    }

    struct yaml_writer
    {
        explicit yaml_writer(ostream& stream, bool include_path = true) :
            _emitter(stream),
            _include_path(include_path)
        {
        }

        void write(syntax_tree const& tree)
        {
            _emitter << YAML::BeginMap;
            if (_include_path) {
                write("path", tree.path());
            }
            write("parameters", tree.parameters);
            write("statements", tree.statements);
            _emitter << YAML::EndMap;
        }

     private:
        void write(lexer::position const& position)
        {
            _emitter << YAML::BeginMap;
            write("offset", position.offset());
            write("line", position.line());
            _emitter << YAML::EndMap;
        }

        void write(statement const& node)
        {
            boost::apply_visitor([this](auto const& node) {
                this->write(node);
            }, node);
        }

        void write(class_statement const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "class statement");
            write(static_cast<context const&>(node));
            write("name", node.name);
            write("parameters", node.parameters);
            write("parent", node.parent);
            write("body", node.body);
            _emitter << YAML::EndMap;
        }

        void write(defined_type_statement const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "defined type statement");
            write(static_cast<context const&>(node));
            write("name", node.name);
            write("parameters", node.parameters);
            write("body", node.body);
            _emitter << YAML::EndMap;
        }

        void write(ast::hostname const& hostname)
        {
            _emitter << YAML::BeginMap;
            if (hostname.is_default()) {
                write("default", true);
            } else {
                write("regex", hostname.is_regex());
                write("value", hostname.to_string());
            }
            _emitter << YAML::EndMap;
        }

        void write(node_statement const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "node statement");
            write(static_cast<context const&>(node));
            write("hostnames", node.hostnames);
            write("body", node.body);
            _emitter << YAML::EndMap;
        }

        void write(function_statement const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "function statement");
            write(static_cast<context const&>(node));
            write("name", node.name);
            write("parameters", node.parameters);
            write("body", node.body);
            _emitter << YAML::EndMap;
        }

        void write(produces_statement const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "produces statement");
            write("resource", node.resource);
            write("capability", node.capability);
            write("operations", node.operations);
            write("end", node.end);
            _emitter << YAML::EndMap;
        }

        void write(consumes_statement const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "consumes statement");
            write("resource", node.resource);
            write("capability", node.capability);
            write("operations", node.operations);
            write("end", node.end);
            _emitter << YAML::EndMap;
        }

        void write(application_statement const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "application statement");
            write(static_cast<context const&>(node));
            write("name", node.name);
            write("parameters", node.parameters);
            write("body", node.body);
            _emitter << YAML::EndMap;
        }

        void write(site_statement const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "site statement");
            write(static_cast<context const&>(node));
            write("body", node.body);
            _emitter << YAML::EndMap;
        }

        void write(type_alias_statement const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "type alias statement");
            write("alias", node.alias);
            write("type", node.type);
            _emitter << YAML::EndMap;
        }

        void write(function_call_statement const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "function call statement");
            write("function", node.function);
            write("arguments", node.arguments);
            write("lambda", node.lambda);
            _emitter << YAML::EndMap;
        }

        void write(relationship_statement const& node)
        {
            // If the expression has no operations, write the first operand
            if (node.operations.empty()) {
                write(node.operand);
            } else {
                _emitter << YAML::BeginMap;
                write("kind", "relationship statement");
                write("operand", node.operand);
                write("operations", node.operations);
                _emitter << YAML::EndMap;
            }
        }

        void write(relationship_expression const& node)
        {
            boost::apply_visitor([this](auto const& node) {
                this->write(node);
            }, node);
        }

        void write(relationship_operation const& node)
        {
            _emitter << YAML::BeginMap;
            write("operator_position", node.operator_position);
            write("operator", boost::lexical_cast<std::string>(node.operator_));
            write("operand", node.operand);
            _emitter << YAML::EndMap;
        }

        void write(expression const& node)
        {
            // If the expression has no binary operations, write the first operand
            if (node.operations.empty()) {
                write(node.operand);
            } else {
                _emitter << YAML::BeginMap;
                write("kind", "binary expression");
                write("operand", node.operand);
                write("operations", node.operations);
                _emitter << YAML::EndMap;
            }
        }

        void write(postfix_expression const& node)
        {
            // If the postfix has no operations, write the operand
            if (node.operations.empty()) {
                write(node.operand);
            } else {
                _emitter << YAML::BeginMap;
                write("kind", "postfix expression");
                write("operand", node.operand);
                write("operations", node.operations);
                _emitter << YAML::EndMap;
            }
        }

        void write(postfix_operation const& node)
        {
            boost::apply_visitor([this](auto const& node) {
                this->write(node);
            }, node);
        }

        void write(binary_operation const& node)
        {
            _emitter << YAML::BeginMap;
            write("operator_position", node.operator_position);
            write("operator", boost::lexical_cast<std::string>(node.operator_));
            write("operand", node.operand);
            _emitter << YAML::EndMap;
        }

        void write(selector_expression const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "selector expression");
            write(static_cast<context const&>(node));
            write("cases", node.cases);
            _emitter << YAML::EndMap;
        }

        void write(access_expression const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "access expression");
            write(static_cast<context const&>(node));
            write("arguments", node.arguments);
            _emitter << YAML::EndMap;
        }

        void write(method_call_expression const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "method call expression");
            write("begin", node.begin);
            write("method", node.method);
            write("arguments", node.arguments);
            write("end", node.end);
            write("lambda", node.lambda);
            _emitter << YAML::EndMap;
        }

        void write(basic_expression const& node)
        {
            boost::apply_visitor([this](auto const& node) {
                this->write(node);
            }, node);
        }

        void write(undef const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "undef");
            write(static_cast<context const&>(node));
            _emitter << YAML::EndMap;
        }

        void write(defaulted const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "default");
            write(static_cast<context const&>(node));
            _emitter << YAML::EndMap;
        }

        void write(boolean const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "boolean");
            write(static_cast<context const&>(node));
            write("value", node.value);
            _emitter << YAML::EndMap;
        }

        void write(number const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "number");
            write(static_cast<context const&>(node));
            write("base", boost::lexical_cast<std::string>(node.base));
            boost::apply_visitor([this](auto const& node) {
                this->write("value", node);
            }, node.value);
            _emitter << YAML::EndMap;
        }

        void write(string const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "string");
            write(static_cast<context const&>(node));
            write("value", node.value);
            if (!node.format.empty()) {
                write("format", node.format);
            }
            if (node.margin > 0) {
                write("margin", node.margin);
            }
            _emitter << YAML::EndMap;
        }

        void write(regex const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "regex");
            write(static_cast<context const&>(node));
            write("value", node.value);
            _emitter << YAML::EndMap;
        }

        void write(variable const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "variable");
            write(static_cast<context const&>(node));
            write("name", node.name);
            _emitter << YAML::EndMap;
        }

        void write(name const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "name");
            write(static_cast<context const&>(node));
            write("value", node.value);
            _emitter << YAML::EndMap;
        }

        void write(bare_word const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "bare word");
            write(static_cast<context const&>(node));
            write("value", node.value);
            _emitter << YAML::EndMap;
        }

        void write(type const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "type");
            write(static_cast<context const&>(node));
            write("name", node.name);
            _emitter << YAML::EndMap;
        }

        void write(interpolated_string const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "interpolated string");
            write(static_cast<context const&>(node));
            write("parts", node.parts);
            if (!node.format.empty()) {
                write("format", node.format);
            }
            if (node.margin > 0) {
                write("margin", node.margin);
            }
            _emitter << YAML::EndMap;
        }

        void write(literal_string_text const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "literal text");
            write(static_cast<context const&>(node));
            write("text", node.text);
            _emitter << YAML::EndMap;
        }

        void write(interpolated_string_part const& part)
        {
            boost::apply_visitor([this](auto const& node) {
                this->write(node);
            }, part);
        }

        void write(array const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "array");
            write(static_cast<context const&>(node));
            write("elements", node.elements);
            _emitter << YAML::EndMap;
        }

        void write(hash const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "hash");
            write(static_cast<context const&>(node));
            write("elements", node.elements);
            _emitter << YAML::EndMap;
        }

        void write(ast::proposition const& proposition)
        {
            _emitter << YAML::BeginMap;
            write("options", proposition.options);
            write("body", proposition.body);
            write("end", proposition.end);
            _emitter << YAML::EndMap;
        }

        void write(case_expression const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "case expression");
            write(static_cast<context const&>(node));
            write("conditional", node.conditional);
            write("propositions", node.propositions);
            _emitter << YAML::EndMap;
        }

        void write(ast::else_ const& else_)
        {
            _emitter << YAML::BeginMap;
            write("begin", else_.begin);
            write("body", else_.body);
            write("end", else_.end);
            _emitter << YAML::EndMap;
        }

        void write(ast::elsif const& elsif)
        {
            _emitter << YAML::BeginMap;
            write("begin", elsif.begin);
            write("conditional", elsif.conditional);
            write("body", elsif.body);
            write("end", elsif.end);
            _emitter << YAML::EndMap;
        }

        void write(if_expression const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "if expression");
            write("begin", node.begin);
            write("end", node.end);
            write("conditional", node.conditional);
            write("body", node.body);
            write("elsifs", node.elsifs);
            write("else", node.else_);
            _emitter << YAML::EndMap;
        }

        void write(unless_expression const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "unless expression");
            write("begin", node.begin);
            write("end", node.end);
            write("conditional", node.conditional);
            write("body", node.body);
            write("else", node.else_);
            _emitter << YAML::EndMap;
        }

        void write(ast::parameter const& parameter)
        {
            _emitter << YAML::BeginMap;
            write("type", parameter.type);
            write("captures", parameter.captures);
            write("variable", parameter.variable);
            write("default_value", parameter.default_value);
            _emitter << YAML::EndMap;
        }

        void write(lambda_expression const& lambda)
        {
            _emitter << YAML::BeginMap;
            write(static_cast<context const&>(lambda));
            write("parameters", lambda.parameters);
            write("body", lambda.body);
            _emitter << YAML::EndMap;
        }

        void write(function_call_expression const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "function call expression");
            write("function", node.function);
            write("arguments", node.arguments);
            write("lambda", node.lambda);
            write("end", node.end);
            _emitter << YAML::EndMap;
        }

        void write(new_expression const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "new expression");
            write("type", node.type);
            write("arguments", node.arguments);
            write("end", node.end);
            _emitter << YAML::EndMap;
        }

        void write(epp_render_expression const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "EPP render expression");
            write(static_cast<context const&>(node));
            write("expression", node.expression);
            _emitter << YAML::EndMap;
        }

        void write(epp_render_block const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "EPP render block");
            write(static_cast<context const&>(node));
            write("block", node.block);
            _emitter << YAML::EndMap;
        }

        void write(epp_render_string const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "EPP render string");
            write(static_cast<context const&>(node));
            write("string", node.string);
            _emitter << YAML::EndMap;
        }

        void write(unary_expression const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "unary expression");
            write("operator_position", node.operator_position);
            write("operator", boost::lexical_cast<std::string>(node.operator_));
            write("operand", node.operand);
            _emitter << YAML::EndMap;
        }

        void write(nested_expression const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "nested expression");
            write(static_cast<context const&>(node));
            write("expression", node.expression);
            _emitter << YAML::EndMap;
        }

        void write(attribute_operation const& operation)
        {
            _emitter << YAML::BeginMap;
            write("name", operation.name);
            write("operator_position", operation.operator_position);
            write("operator", boost::lexical_cast<std::string>(operation.operator_));
            write("value", operation.value);
            _emitter << YAML::EndMap;
        }

        void write(resource_body const& body)
        {
            _emitter << YAML::BeginMap;
            write("title", body.title);
            write("operations", body.operations);
            _emitter << YAML::EndMap;
        }

        void write(resource_declaration_expression const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "resource declaration expression");
            write(static_cast<context const&>(node));
            std::string status;
            switch (node.status) {
                case resource_status::realized:
                    status = "realized";
                    break;

                case resource_status::virtualized:
                    status = "virtual";
                    break;

                case resource_status::exported:
                    status = "exported";
                    break;

                default:
                    throw runtime_error("unexpected resource status.");
            }
            write("status", status);
            write("type", node.type);
            write("bodies", node.bodies);
            _emitter << YAML::EndMap;
        }

        void write(resource_override_expression const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "resource override expression");
            write(static_cast<context const&>(node));
            write("reference", node.reference);
            write("operations", node.operations);
            _emitter << YAML::EndMap;
        }

        void write(resource_override_reference const& node)
        {
            boost::apply_visitor([this](auto const& node) {
                this->write(node);
            }, node);
        }

        void write(resource_defaults_expression const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "resource defaults expression");
            write(static_cast<context const&>(node));
            write("type", node.type);
            write("operations", node.operations);
            _emitter << YAML::EndMap;
        }

        void write(basic_query_expression const& node)
        {
            boost::apply_visitor([this](auto const& node) {
                this->write(node);
            }, node);
        }

        void write(attribute_query const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "attribute query");
            write("attribute", node.attribute);
            write("operator_position", node.operator_position);
            write("operator", boost::lexical_cast<std::string>(node.operator_));
            write("value", node.value);
            _emitter << YAML::EndMap;
        }

        void write(nested_query_expression const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "nested query expression");
            write("expression", node.expression);
            _emitter << YAML::EndMap;
        }

        void write(binary_query_operation const& node)
        {
            _emitter << YAML::BeginMap;
            write("operator_position", node.operator_position);
            write("operator", boost::lexical_cast<std::string>(node.operator_));
            write("operand", node.operand);
            _emitter << YAML::EndMap;
        }

        void write(query_expression const& query)
        {
            // If the query has no binary operations, write the first operand only
            if (query.operations.empty()) {
                write(query.operand);
            } else {
                _emitter << YAML::BeginMap;
                write("kind", "query expression");
                write("operand", query.operand);
                write("operations", query.operations);
                _emitter << YAML::EndMap;
            }
        }

        void write(collector_expression const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "collector expression");
            write("type", node.type);
            write("exported", node.exported);
            write("query", node.query);
            write("end", node.end);
            _emitter << YAML::EndMap;
        }

        void write(break_statement const& node)
        {
            _emitter << YAML::BeginMap;
            write("kind", "break statement");
            _emitter << YAML::EndMap;
        }

        void write(context const& node)
        {
            write("begin", node.begin);
            write("end", node.end);
        }

        void write(std::pair<expression, expression> const& pair)
        {
            _emitter << YAML::BeginMap;
            write("first", pair.first);
            write("second", pair.second);
            _emitter << YAML::EndMap;
        }

        template <typename T>
        void write(x3::forward_ast<T> const& node)
        {
            write(node.get());
        }

        template <typename T>
        void write(T const& value)
        {
            _emitter << value;
        }

        template <typename T>
        void write(std::string const& key, boost::optional<T> const& node)
        {
            if (node) {
                write(key, *node);
            }
        }

        template <typename T>
        void write(std::string const& key, T const& value)
        {
            _emitter << YAML::Key << key << YAML::Value;
            write(value);
        }

        template <typename T>
        void write(std::string const& key, std::vector<T> const& sequence)
        {
            if (sequence.empty()) {
                return;
            }

            _emitter << YAML::Key << key << YAML::Value << YAML::BeginSeq;
            for (auto const& element : sequence) {
                write(element);
            }
            _emitter << YAML::EndSeq;
        }

        YAML::Emitter _emitter;
        bool _include_path;
    };

    void syntax_tree::write(ast::format format, ostream& stream, bool include_path) const
    {
        switch (format) {
            case ast::format::yaml: {
                yaml_writer writer{stream, include_path};
                writer.write(*this);
                break;
            }

            default:
                throw runtime_error("unexpected syntax tree format.");
        }
    }

    void syntax_tree::validate(bool epp, bool allow_catalog_statements) const
    {
        visitors::validation visitor{ epp, allow_catalog_statements };
        visitor.visit(*this);
    }

    shared_ptr<syntax_tree> syntax_tree::create(std::string path, compiler::module const* module)
    {
        // This exists because the constructor is protected
        struct make_shared_enabler : syntax_tree
        {
            explicit make_shared_enabler(std::string path, compiler::module const* module) :
                syntax_tree(rvalue_cast(path), module)
            {
            }
        };
        return make_shared<make_shared_enabler>(rvalue_cast(path), module);
    }

    syntax_tree::syntax_tree(std::string path, compiler::module const* module) :
        _path(make_shared<std::string>(rvalue_cast(path))),
        _module(module)
    {
    }

    ostream& operator<<(ostream& os, syntax_tree const& node)
    {
        if (node.parameters) {
            os << "|";
            pretty_print(os, *node.parameters, ", ");
            os << "| ";
        }
        pretty_print(os, node.statements, "; ");
        return os;
    }

}}}  // namespace puppet::compiler::ast
