#include <puppet/lexer/token_id.hpp>
#include <map>

using namespace std;

namespace puppet { namespace lexer {

    ostream& operator<<(ostream& os, token_id const& id)
    {
        static map<token_id, string> token_names = {
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
            { token_id::statement_call,         "name" },  // Statement calls are technically names
            { token_id::single_quoted_string,   "string" },
            { token_id::double_quoted_string,   "string" },
            { token_id::bare_word,              "bare word" },
            { token_id::variable,               "variable" },
            { token_id::type,                   "type" },
            { token_id::name,                   "name" },
            { token_id::regex,                  "regular expression" },
            { token_id::heredoc,                "heredoc" },
            { token_id::number,                 "number" },
            { token_id::array_start,            "'['" },
            { token_id::comment,                "comment" },
            { token_id::whitespace,             "whitespace" },
            { token_id::unclosed_quote,         "unclosed quote" },
            { token_id::unclosed_comment,       "unclosed comment" },
            { token_id::invalid_variable,       "invalid variable"},
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

}}  // namespace puppet::lexer
