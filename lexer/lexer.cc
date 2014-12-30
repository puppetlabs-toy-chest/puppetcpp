#include "lexer.hpp"
#include <map>
#include <limits>

using namespace std;
using namespace boost::spirit;

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

    ostream& operator<<(ostream& os, string_token const& token)
    {
        os << (token.interpolated() ? '"' : '\'') << token.text() << (token.interpolated() ? '"' : '\'');
        return os;
    }

    lexer_istreambuf_iterator lex_begin(ifstream& file)
    {
        return lexer_istreambuf_iterator(typename lexer_istreambuf_iterator::base_type(make_default_multi_pass(istreambuf_iterator<char>(file))));
    }

    lexer_istreambuf_iterator lex_end(ifstream& file)
    {
        return lexer_istreambuf_iterator(typename lexer_istreambuf_iterator::base_type(make_default_multi_pass(istreambuf_iterator<char>())));
    }

    lexer_string_iterator lex_begin(string const& str)
    {
        return lexer_string_iterator(typename lexer_string_iterator::base_type(str.begin()));
    }

    lexer_string_iterator lex_end(string const& str)
    {
        return lexer_string_iterator(typename lexer_string_iterator::base_type(str.end()));
    }

    tuple<string, size_t> get_line_and_column(ifstream& fs, size_t position, size_t tab_width)
    {
        const size_t READ_SIZE = 4096;
        char buf[READ_SIZE];

        // Read backwards in chunks looking for the closest newline before the given position
        size_t start;
        for (start = (position > (READ_SIZE + 1) ? position - READ_SIZE - 1 : 0); fs; start -= (start < READ_SIZE ? start : READ_SIZE)) {
            if (!fs.seekg(start)) {
                return make_tuple("", 1);
            }

            // Read data into the buffer
            if (!fs.read(buf, position < READ_SIZE ? position : READ_SIZE)) {
                return make_tuple("", 1);
            }

            // Find the last newline in the buffer
            auto it = find(reverse_iterator<char*>(buf + fs.gcount()), reverse_iterator<char*>(buf), '\n');
            if (it != reverse_iterator<char*>(buf)) {
                start += distance(buf, it.base());
                break;
            }

            if (start == 0) {
                break;
            }
        }

        // Calculate the column
        size_t column = (position - start) + 1;

        // Find the end of the current line
        size_t end = position;
        fs.seekg(end);
        auto eof =  istreambuf_iterator<char>();
        for (auto it = istreambuf_iterator<char>(fs.rdbuf()); it != eof; ++it) {
            if (*it == '\n') {
                break;
            }
            ++end;
        }

        // Read the line
        size_t size = end - start;
        fs.seekg(start);
        vector<char> line_buffer(size);
        if (!fs.read(line_buffer.data(), line_buffer.size())) {
            return make_tuple("", 1);
        }

        // Convert tabs to spaces
        string line = string(line_buffer.data(), line_buffer.size());
        if (tab_width > 1) {
            column += count(line.begin(), line.begin() + column, '\t') * (tab_width - 1);
        }

        return make_tuple(move(line), column);
    }

}}  // namespace puppet::lexer
