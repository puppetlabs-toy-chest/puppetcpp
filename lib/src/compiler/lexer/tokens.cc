#include <puppet/compiler/lexer/tokens.hpp>
#include <iomanip>
#include <map>

using namespace std;

namespace puppet { namespace compiler { namespace lexer {

    ostream& operator<<(ostream& os, token_id const& id)
    {
        static const map<token_id, string> token_names = {
            { token_id::append,                 "unsupported '+=' operator" },
            { token_id::remove,                 "'-='" },
            { token_id::equals,                 "'=='" },
            { token_id::not_equals,             "'!='" },
            { token_id::match,                  "'=~'" },
            { token_id::not_match,              "'!~'" },
            { token_id::greater_equals,         "'>='" },
            { token_id::less_equals,            "'<='" },
            { token_id::fat_arrow,              "'=>'" },
            { token_id::plus_arrow,             "'+>'" },
            { token_id::left_shift,             "'<<'" },
            { token_id::left_collect,           "'<|'" },
            { token_id::left_double_collect,    "'<<|'" },
            { token_id::right_shift,            "'>>'" },
            { token_id::right_collect,          "'|>'" },
            { token_id::right_double_collect,   "'|>>'" },
            { token_id::atat,                   "'@@'" },
            { token_id::in_edge,                "'->'" },
            { token_id::in_edge_sub,            "'~>'" },
            { token_id::out_edge,               "'<-'" },
            { token_id::out_edge_sub,           "'<~'" },
            { token_id::keyword_case,           "case keyword" },
            { token_id::keyword_class,          "class keyword" },
            { token_id::keyword_default,        "default keyword" },
            { token_id::keyword_define,         "define keyword" },
            { token_id::keyword_if,             "if keyword" },
            { token_id::keyword_elsif,          "elsif keyword" },
            { token_id::keyword_else,           "else keyword" },
            { token_id::keyword_inherits,       "inherits keyword" },
            { token_id::keyword_node,           "node keyword" },
            { token_id::keyword_and,            "and keyword" },
            { token_id::keyword_or,             "or keyword" },
            { token_id::keyword_undef,          "undef keyword" },
            { token_id::keyword_false,          "false keyword" },
            { token_id::keyword_true,           "true keyword" },
            { token_id::keyword_in,             "in keyword" },
            { token_id::keyword_unless,         "unless keyword" },
            { token_id::keyword_function,       "function keyword" },
            { token_id::keyword_type,           "type keyword" },
            { token_id::keyword_attr,           "attr keyword" },
            { token_id::keyword_private,        "private keyword" },
            { token_id::keyword_produces,       "produces keyword" },
            { token_id::keyword_consumes,       "consumes keyword" },
            { token_id::keyword_application,    "application keyword" },
            { token_id::keyword_site,           "site keyword" },
            { token_id::statement_call,         "name" },  // Statement calls are technically names
            { token_id::string,                 "string" },
            { token_id::string_start,           "interpolated string" },
            { token_id::string_end,             "\"" },
            { token_id::string_text,            "string" },
            { token_id::interpolation_start,    "${" },
            { token_id::interpolation_end,      "}" },
            { token_id::bare_word,              "bare word" },
            { token_id::variable,               "variable" },
            { token_id::type,                   "type" },
            { token_id::name,                   "name" },
            { token_id::regex,                  "regular expression" },
            { token_id::number,                 "number" },
            { token_id::array_start,            "'['" },
            { token_id::epp_start,              "'<%'" },
            { token_id::epp_end,                "'%>'" },
            { token_id::epp_start_trim,         "'<%-'" },
            { token_id::epp_end_trim,           "'-%>'" },
            { token_id::epp_render_string,      "EPP render string" },
            { token_id::epp_render_expression,  "EPP render expression" },
            { token_id::comment,                "comment" },
            { token_id::whitespace,             "whitespace" },
            { token_id::unclosed_comment,       "unclosed comment" },
        };

        if (static_cast<size_t>(id) == boost::lexer::npos) {
            os << "end of input";
            return os;
        }

        // If the id is less than 128, the id is the token
        if (static_cast<size_t>(id) != 0 && static_cast<size_t>(id) < 128) {
            os << "'" << static_cast<char>(id) << "'";
            return os;
        }

        auto it = token_names.find(id);
        if (it == token_names.end()) {
            os << "unknown token";
            return os;
        }

        os << it->second;
        return os;
    }

    bool is_keyword(token_id id)
    {
        return static_cast<size_t>(id) > static_cast<size_t>(token_id::first_keyword) &&
               static_cast<size_t>(id) < static_cast<size_t>(token_id::last_keyword);
    }

    ostream& operator<<(ostream& os, string_token const& token)
    {
        os << '\'' << token.value << '\'';
        return os;
    }

    ostream& operator<<(ostream& os, string_start_token const& token)
    {
        os << '"';
        return os;
    }

    ostream& operator<<(ostream& os, string_text_token const& token)
    {
        os << token.text;
        return os;
    }

    ostream& operator<<(ostream& os, string_end_token const& token)
    {
        os << '"';
        return os;
    }

    ostream& operator<<(ostream& os, numeric_base base)
    {
        switch (base) {
            case numeric_base::decimal:
                os << "decimal";
                break;

            case numeric_base::octal:
                os << "octal";
                break;

            case lexer::numeric_base::hexadecimal:
                os << "hexadecimal";
                break;

            default:
                throw runtime_error("unexpected numeric base.");
        }
        return os;
    }

    struct insertion_visitor : boost::static_visitor<ostream&>
    {
        insertion_visitor(ostream& os, numeric_base base) :
            _os(os),
            _base(base)
        {
        }

        result_type operator()(int64_t value) const
        {
            switch (_base) {
                case numeric_base::decimal:
                    _os << value;
                    break;

                case numeric_base::octal:
                    _os << "0" << oct << value << dec;
                    break;

                case numeric_base::hexadecimal:
                    _os << "0x" << hex << value << dec;
                    break;

                default:
                    throw runtime_error("unexpected numeric base for number token.");
            }
            return _os;
        }

        result_type operator()(double value) const
        {
            return (_os << value);
        }

     private:
        ostream& _os;
        numeric_base _base;
    };

    ostream& operator<<(ostream& os, number_token const& token)
    {
        return boost::apply_visitor(insertion_visitor(os, token.base), token.value);
    }

}}}  // namespace puppet::compiler::lexer
