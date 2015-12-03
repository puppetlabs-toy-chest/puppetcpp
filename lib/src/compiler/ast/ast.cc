#include <puppet/compiler/ast/ast.hpp>
#include <puppet/cast.hpp>

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

    void print_string(ostream& os, std::string const& s)
    {
        for (char c : s) {
            if (c == '\n') {
                os << "\\n";
            } else if (c == '\r') {
                os << "\\r";
            } else {
                os << c;
            }
        }
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

    ostream& operator<<(ostream& os, number const& node)
    {
        os << node.value;
        return os;
    }

    ostream& operator<<(ostream& os, ast::string const& node)
    {
        os << (node.interpolated ? '"' : '\'');
        print_string(os, node.value);
        os << (node.interpolated ? '"' : '\'');
        return os;
    }

    ostream& operator<<(ostream& os, regex const& node)
    {
        os << "/" << node.value << "/";
        return os;
    }

    ostream& operator<<(ostream& os, variable const& node)
    {
        os << "$" << node.name;
        return os;
    }

    ostream& operator<<(ostream& os, name const& node)
    {
        os << node.value;
        return os;
    }

    ostream& operator<<(ostream& os, bare_word const& node)
    {
        os << node.value;
        return os;
    }

    ostream& operator<<(ostream& os, type const& node)
    {
        os << node.name;
        return os;
    }

    ast::context primary_expression::context() const
    {
        return boost::apply_visitor(context_visitor(), *this);
    }

    bool primary_expression::is_productive() const
    {
        // Check for recursive primary expression
        if (auto ptr = boost::get<x3::forward_ast<expression>>(this)) {
            return ptr->get().is_productive();
        }

        // Check for unary expression
        if (auto ptr = boost::get<x3::forward_ast<unary_expression>>(this)) {
            return ptr->get().operand.is_productive();
        }

        // All control flow, catalog expressions, and EPP expressions are considered to be productive
        if (boost::get<x3::forward_ast<case_expression>>(this) ||
            boost::get<x3::forward_ast<if_expression>>(this) ||
            boost::get<x3::forward_ast<unless_expression>>(this) ||
            boost::get<x3::forward_ast<function_call_expression>>(this) ||
            boost::get<x3::forward_ast<resource_expression>>(this) ||
            boost::get<x3::forward_ast<resource_override_expression>>(this) ||
            boost::get<x3::forward_ast<resource_defaults_expression>>(this) ||
            boost::get<x3::forward_ast<class_expression>>(this) ||
            boost::get<x3::forward_ast<defined_type_expression>>(this) ||
            boost::get<x3::forward_ast<node_expression>>(this) ||
            boost::get<x3::forward_ast<collector_expression>>(this) ||
            boost::get<x3::forward_ast<epp_render_expression>>(this) ||
            boost::get<x3::forward_ast<epp_render_block>>(this) ||
            boost::get<x3::forward_ast<epp_render_string>>(this)) {
            return true;
        }
        return false;
    }

    bool primary_expression::is_splat() const
    {
        // Try for unary expression
        auto unary = boost::get<x3::forward_ast<unary_expression>>(this);
        if (unary) {
            return unary->get().is_splat();
        }

        // Try for nested expression
        auto nested = boost::get<x3::forward_ast<expression>>(this);
        if (nested) {
            return nested->get().is_splat();
        }
        return false;
    }

    bool primary_expression::is_default() const
    {
        // Try for nested expression
        auto nested = boost::get<x3::forward_ast<expression>>(this);
        if (nested) {
            return nested->get().is_default();
        }
        return static_cast<bool>(boost::get<defaulted>(this));
    }

    ostream& operator<<(std::ostream& os, primary_expression const& node)
    {
        printer visitor{os};
        boost::apply_visitor(visitor, node.get());
        return os;
    }

    ast::context postfix_subexpression::context() const
    {
        return boost::apply_visitor(context_visitor(), *this);
    }

    ast::context postfix_expression::context() const
    {
        auto context = primary.context();
        if (!subexpressions.empty()) {
            context.end = subexpressions.back().context().end;
        }
        return context;
    }

    bool postfix_expression::is_productive() const
    {
        if (primary.is_productive()) {
            return true;
        }

        // Postfix method calls are productive
        for (auto const& subexpression : subexpressions) {
            if (boost::get<x3::forward_ast<method_call_expression>>(&subexpression)) {
                return true;
            }
        }
        return false;
    }

    bool postfix_expression::is_splat() const
    {
        // Check the primary expression
        return primary.is_splat();
    }

    bool postfix_expression::is_default() const
    {
        if (!subexpressions.empty()) {
            return false;
        }

        // Check the primary expression
        return primary.is_default();
    }

    ostream& operator<<(std::ostream& os, postfix_subexpression const& node)
    {
        printer visitor{os};
        boost::apply_visitor(visitor, node.get());
        return os;
    }

    ostream& operator<<(ostream& os, postfix_expression const& node)
    {
        os << node.primary;
        for (auto const& subexpression : node.subexpressions) {
            os << subexpression;
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
            case binary_operator::in_edge:
                os << "->";
                break;
            case binary_operator::in_edge_subscribe:
                os << "~>";
                break;
            case binary_operator::out_edge:
                os << "<-";
                break;
            case binary_operator::out_edge_subscribe:
                os << "<~";
                break;
            default:
                throw runtime_error("invalid binary operator.");
        }
        return os;
    }

    size_t hash_value(binary_operator const& oper)
    {
        using type = typename underlying_type<binary_operator>::type;
        std::hash<type> hasher;
        return hasher(static_cast<type>(oper));
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
        auto context = postfix.context();
        if (!operations.empty()) {
            context.end = operations.back().context().end;
        }
        return context;
    }

    bool expression::is_productive() const
    {
        // Check if the postfix expression is productive
        if (postfix.is_productive()) {
            return true;
        }

        // Expressions followed by an assignment or relationship operator are productive
        for (auto const& operation : operations) {
            if (operation.operator_ == binary_operator::assignment ||
                operation.operator_ == binary_operator::in_edge ||
                operation.operator_ == binary_operator::in_edge_subscribe ||
                operation.operator_ == binary_operator::out_edge ||
                operation.operator_ == binary_operator::out_edge_subscribe) {
                return true;
            }
        }
        return false;
    }

    bool expression::is_splat() const
    {
        // A splat expression never has binary operations
        if (!operations.empty()) {
            return false;
        }
        // Check the postfix expression
        return postfix.is_splat();
    }

    bool expression::is_default() const
    {
        // A default expression never has binary operations
        if (!operations.empty()) {
            return false;
        }
        // Check the postfix expression
        return postfix.is_default();
    }

    ostream& operator<<(ostream& os, expression const& node)
    {
        if (!node.operations.empty()) {
            os << '(';
        }
        os << node.postfix;
        for (auto const& operation : node.operations) {
            os << operation;
        }
        if (!node.operations.empty()) {
            os << ')';
        }
        return os;
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

    ostream& operator<<(ostream& os, selector_expression const& node)
    {
        os << " ? { ";
        pretty_print(os, node.cases, ", ");
        os << " }";
        return os;
    }

    ostream& operator<<(ostream& os, ast::proposition const& proposition)
    {
        pretty_print(os, proposition.options, ", ");
        os << ": ";
        if (proposition.body.empty()) {
            os << "{ }";
        } else {
            os << " { ";
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

    ostream& operator<<(ostream& os, access_expression const& node)
    {
        os << '[';
        pretty_print(os, node.arguments, ", ");
        os << ']';
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

    ast::context method_call_expression::context() const
    {
        ast::context context = method;
        context.begin = begin;

        if (lambda) {
            context.end = lambda->end;
        } else if (end) {
            context.end = *end;
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

    ast::context function_call_expression::context() const
    {
        ast::context context = function;

        if (lambda) {
            context.end = lambda->end;
        } else if (end) {
            context.end = *end;
        } else {
            // Call vector::at to bounds check
            context.end = arguments.at(arguments.size() - 1).context().end;
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

    ostream& operator<<(ostream& os, resource_status const& node)
    {
        switch (node) {
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

    ostream& operator<<(ostream& os, resource_expression const& node)
    {
        os << node.status << node.type << " { ";
        pretty_print(os, node.bodies, "; ");
        os << " }";
        return os;
    }

    ostream& operator<<(ostream& os, resource_override_expression const& node)
    {
        os << node.reference << " { ";
        pretty_print(os, node.operations, ", ");
        os << " }";
        return os;
    }

    ostream& operator<<(ostream& os, resource_defaults_expression const& node)
    {
        os << node.type << " { ";
        pretty_print(os, node.operations, ", ");
        os << " }";
        return os;
    }

    ostream& operator<<(ostream& os, class_expression const& node)
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

    ostream& operator<<(ostream& os, defined_type_expression const& node)
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

    struct hostname_visitor : boost::static_visitor<>
    {
        explicit hostname_visitor(ostringstream& ss) :
            _ss(ss)
        {
        }

        void operator()(defaulted const& d)
        {
            _ss << d;
        }

        void operator()(regex const& r)
        {
            _ss << r;
        }

        void operator()(string const& s)
        {
            _ss << s;
        }

        void operator()(hostname_parts const& parts)
        {
            pretty_print(_ss, parts, ".");
        }

     private:
        ostringstream& _ss;
    };

    std::string hostname::to_string() const
    {
        ostringstream ss;
        hostname_visitor visitor{ss};
        boost::apply_visitor(visitor, *this);
        return ss.str();
    }

    ostream& operator<<(ostream& os, hostname const& node)
    {
        os << node.to_string();
        return os;
    }

    ostream& operator<<(ostream& os, node_expression const& node)
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

    ast::context primary_query_expression::context() const
    {
        return boost::apply_visitor(context_visitor(), *this);
    }

    ostream& operator<<(std::ostream& os, primary_query_expression const& node)
    {
        printer visitor{os};
        boost::apply_visitor(visitor, node.get());
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

    ostream& operator<<(std::ostream& os, binary_query_operation const& operation)
    {
        os << " " << operation.operator_ << " " << operation.operand;
        return os;
    }

    ast::context query_expression::context() const
    {
        auto context = primary.context();
        if (!operations.empty()) {
            context.end = operations.back().context().end;
        }
        return context;
    }

    ostream& operator<<(std::ostream& os, query_expression const& node)
    {
        if (!node.operations.empty()) {
            os << '(';
        }
        os << node.primary;
        for (auto const& operation : node.operations) {
            os << operation;
        }
        if (!node.operations.empty()) {
            os << ')';
        }
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

    bool unary_expression::is_splat() const
    {
        return operator_ == unary_operator::splat;
    }

    ostream& operator<<(ostream& os, unary_expression const& node)
    {
        os << node.operator_ << node.operand;
        return os;
    }

    ostream& operator<<(ostream& os, epp_render_expression const& node)
    {
        os << "render(" << node.expression << ")";
        return os;
    }

    ostream& operator<<(ostream& os, epp_render_block const& node)
    {
        os << "render({";
        pretty_print(os, node.block, "; ");
        os << "})";
        return os;
    }

    ostream& operator<<(ostream& os, epp_render_string const& node)
    {
        os << "render('";
        print_string(os, node.string);
        os << "')";
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