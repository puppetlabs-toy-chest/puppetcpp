#include <catch.hpp>
#include <puppet/lexer/static_lexer.hpp>
#include <puppet/lexer/lexer.hpp>

using namespace std;
using namespace puppet::lexer;

template <typename Token>
struct token_value_visitor : boost::static_visitor<string>
{
    template <typename Iterator>
    result_type operator()(boost::iterator_range<Iterator> const& range) const
    {
        return string(range.begin(), range.end());
    }

    result_type operator()(string_token const& token) const
    {
        return token.text();
    }
};

template <typename Iterator>
void require_token(Iterator& token, Iterator const& end, token_id id, std::string const& value)
{
    REQUIRE(token != end);
    REQUIRE(token->id() == static_cast<size_t>(id));
    REQUIRE(boost::apply_visitor(token_value_visitor<typename Iterator::value_type>(), token->value()) == value);
    ++token;
}

SCENARIO("lexing single quoted strings")
{
    ifstream input(FIXTURES_DIR "lexer/single_quoted_strings.pp");
    REQUIRE(input);

    auto input_begin = lex_begin(input);
    auto input_end = lex_end(input);

    file_static_lexer lexer([&](token_position const& position, string const& message) {
        FAIL("unexpected warning");
    });
    auto token = lexer.begin(input_begin, input_end);
    auto end = lexer.end();
    require_token(token, end, token_id::single_quoted_string, "");
    require_token(token, end, token_id::single_quoted_string, "this is a string");
    require_token(token, end, token_id::single_quoted_string, "this string has a back\\slash");
    require_token(token, end, token_id::single_quoted_string, "this string has an escaped \'");
    require_token(token, end, token_id::single_quoted_string, "\' this string is quoted \'");
    require_token(token, end, token_id::single_quoted_string, "this \\ is escaped");
    require_token(token, end, token_id::single_quoted_string, " this line\n has a\n break!\n");
    REQUIRE(token == end);
}

SCENARIO("lexing double quoted strings")
{
    ifstream input(FIXTURES_DIR "lexer/double_quoted_strings.pp");
    REQUIRE(input);

    auto input_begin = lex_begin(input);
    auto input_end = lex_end(input);

    bool warning = false;
    file_static_lexer lexer([&](token_position const& position, string const& message) {
        REQUIRE_FALSE(warning);
        string line;
        size_t column;
        tie(line, column) = get_line_and_column(input, get<0>(position));
        REQUIRE(line == "\"this \\f is not a valid escape\"");
        REQUIRE(get<1>(position) == 28);
        REQUIRE(column == 7);
        REQUIRE(message == "unexpected escape sequence '\\f'.");
        warning = true;
    });
    auto token = lexer.begin(input_begin, input_end);
    auto end = lexer.end();
    require_token(token, end, token_id::double_quoted_string, "");
    require_token(token, end, token_id::double_quoted_string, "this is a string");
    require_token(token, end, token_id::double_quoted_string, "\" this string is quoted \"");
    require_token(token, end, token_id::double_quoted_string, "this ' is escaped");
    require_token(token, end, token_id::double_quoted_string, "this \\ is escaped");
    require_token(token, end, token_id::double_quoted_string, "this \" is escaped");
    require_token(token, end, token_id::double_quoted_string, "this \n is escaped");
    require_token(token, end, token_id::double_quoted_string, "this \r is escaped");
    require_token(token, end, token_id::double_quoted_string, "this \t is escaped");
    require_token(token, end, token_id::double_quoted_string, "this ' ' is escaped");
    require_token(token, end, token_id::double_quoted_string, "this \u263A is a unicode character");
    require_token(token, end, token_id::double_quoted_string, "this string\n   has a\n   line break!\n   ");
    require_token(token, end, token_id::double_quoted_string, "this \\f is not a valid escape");
    REQUIRE(warning);
    REQUIRE(token == end);
}