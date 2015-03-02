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

template <typename Token>
struct string_token_visitor : boost::static_visitor<string_token const&>
{
    template <typename Iterator>
    result_type operator()(boost::iterator_range<Iterator> const& range) const
    {
        FAIL("not a string token");
        throw runtime_error("not a string token");
    }

    result_type operator()(string_token const& token) const
    {
        return token;
    }
};

template <typename Iterator>
void require_token(
    Iterator& token,
    Iterator const& end,
    token_id expected_id,
    string const& expected_value)
{
    CAPTURE(expected_id);
    CAPTURE(expected_value);

    REQUIRE(token != end);
    token_id id = static_cast<token_id>(token->id());
    REQUIRE(id == expected_id);

    string value = boost::apply_visitor(token_value_visitor<typename Iterator::value_type>(), token->value());
    REQUIRE(value == expected_value);

    ++token;
}

template <typename Iterator>
void require_string_token(
    Iterator& token,
    Iterator const& end,
    token_id expected_id,
    string const& expected_value,
    string const& expected_format = string(),
    bool expected_interpolated = true,
    bool expected_escaped = true)
{
    CAPTURE(expected_id);
    CAPTURE(expected_value);

    REQUIRE(token != end);
    token_id id = static_cast<token_id>(token->id());
    REQUIRE(id == expected_id);

    auto const& value = boost::apply_visitor(string_token_visitor<typename Iterator::value_type>(), token->value());
    REQUIRE(value.text() == expected_value);
    REQUIRE(value.format() == expected_format);
    REQUIRE(value.interpolated() == expected_interpolated);
    REQUIRE(value.escaped() == expected_escaped);

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
    require_string_token(token, end, token_id::single_quoted_string, {}, {}, false, false);
    require_string_token(token, end, token_id::single_quoted_string, "this is a string", {}, false, false);
    require_string_token(token, end, token_id::single_quoted_string, "\' this string is quoted \'", {}, false, false);
    require_string_token(token, end, token_id::single_quoted_string, "this back\\slash is not escaped", {}, false, false);
    require_string_token(token, end, token_id::single_quoted_string, "this back\\slash is escaped", {}, false, false);
    require_string_token(token, end, token_id::single_quoted_string, " this line\n has a\n break!\n", {}, false, false);
    require_token(token, end, token_id::unclosed_quote, "'");
    require_token(token, end, token_id::name, "missing");
    require_token(token, end, token_id::name, "endquote");
    require_token(token, end, token_id::unknown, "\\");
    require_token(token, end, token_id::unclosed_quote, "'");
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
    require_string_token(token, end, token_id::double_quoted_string, "");
    require_string_token(token, end, token_id::double_quoted_string, "this is a string");
    require_string_token(token, end, token_id::double_quoted_string, "\" this string is quoted \"");
    require_string_token(token, end, token_id::double_quoted_string, "this ' is escaped");
    require_string_token(token, end, token_id::double_quoted_string, "this \\ is escaped");
    require_string_token(token, end, token_id::double_quoted_string, "this \" is escaped");
    require_string_token(token, end, token_id::double_quoted_string, "this \n is escaped");
    require_string_token(token, end, token_id::double_quoted_string, "this \r is escaped");
    require_string_token(token, end, token_id::double_quoted_string, "this \t is escaped");
    require_string_token(token, end, token_id::double_quoted_string, "this ' ' is escaped");
    require_string_token(token, end, token_id::double_quoted_string, "this \u263A is a unicode character");
    require_string_token(token, end, token_id::double_quoted_string, "this string\n   has a\n   line break!\n   ");
    require_string_token(token, end, token_id::double_quoted_string, "this \\f is not a valid escape");
    require_token(token, end, token_id::unclosed_quote, "\"");
    require_token(token, end, token_id::name, "missing");
    require_token(token, end, token_id::name, "endquote");
    require_token(token, end, token_id::unknown, "\\");
    require_token(token, end, token_id::unclosed_quote, "\"");
    REQUIRE(warning);
    REQUIRE(token == end);
}

SCENARIO("lexing heredocs")
{
    ifstream input(FIXTURES_DIR "lexer/heredocs.pp");
    REQUIRE(input);

    auto input_begin = lex_begin(input);
    auto input_end = lex_end(input);

    file_static_lexer lexer([&](token_position const& position, string const& message) {
        FAIL("unexpected warning");
    });
    auto token = lexer.begin(input_begin, input_end);
    auto end = lexer.end();
    require_string_token(token, end, token_id::heredoc, "", {}, false, false);
    require_string_token(token, end, token_id::heredoc, "this\nis\na\nheredoc\n", {}, false, false);
    require_string_token(token, end, token_id::heredoc, "first\n", {}, false, false);
    require_string_token(token, end, token_id::single_quoted_string, "hello", {}, false, false);
    require_string_token(token, end, token_id::heredoc, "second\n", {}, false, false);
    require_string_token(token, end, token_id::single_quoted_string, "world", {}, false, false);
    require_string_token(token, end, token_id::heredoc, "third\n", {}, false, false);
    require_string_token(token, end, token_id::heredoc, "{\n  \"hello\": \"world\"\n}\n", "json", false, false);
    require_string_token(token, end, token_id::heredoc, "first: \\\\\\t\\s\\r\\n\\u263A\\$\\\nsecond!\n", {}, false, false);
    require_string_token(token, end, token_id::heredoc, "first: \\\t\\s\\r\\n\\u263A\\$\\\nsecond!\n", {}, false, false);
    require_string_token(token, end, token_id::heredoc, "first: \\\t \\r\\n\\u263A\\$\\\nsecond!\n", {}, false, false);
    require_string_token(token, end, token_id::heredoc, "first: \\\t \r\\n\\u263A\\$\\\nsecond!\n", {}, false, false);
    require_string_token(token, end, token_id::heredoc, "first: \\\t \r\n\\u263A\\$\\\nsecond!\n", {}, false, false);
    require_string_token(token, end, token_id::heredoc, "first: \\\t \r\n\u263A\\$\\\nsecond!\n", {}, false, false);
    require_string_token(token, end, token_id::heredoc, "first: \\\t \r\n\u263A$\\\nsecond!\n", {}, false, false);
    require_string_token(token, end, token_id::heredoc, "first: \\\t \r\n\u263A$second!\n", {}, false, false);
    require_string_token(token, end, token_id::heredoc, "first: \\\t \r\n\u263A$second!\n", {}, false, false);
    require_string_token(token, end, token_id::heredoc, "$hello \\$world\n");
    require_string_token(token, end, token_id::heredoc, "$hello \\$world\n");
    require_string_token(token, end, token_id::heredoc, "$hello \\$world\n");
    require_string_token(token, end, token_id::heredoc, "$hello \\$world\n", {}, true, false);
    require_string_token(token, end, token_id::heredoc, "this is NOT the end\n", {}, false, false);
    require_string_token(token, end, token_id::heredoc, "this is one line", {}, false, false);
    require_string_token(token, end, token_id::heredoc, "this text\n is\n  aligned\n", {}, false, false);
    require_string_token(token, end, token_id::heredoc, "this text\n is\n  aligned", {}, false, false);
    require_string_token(token, end, token_id::heredoc, "this \\$text\nis\n aligned", "json", true, true);
    REQUIRE(token == end);
}

SCENARIO("lexing missing heredoc end tag")
{
    ifstream input(FIXTURES_DIR "lexer/missing_heredoc_tag.pp");
    REQUIRE(input);

    auto input_begin = lex_begin(input);
    auto input_end = lex_end(input);

    file_static_lexer lexer([&](token_position const& position, string const& message) {
        FAIL("unexpected warning");
    });
    auto token = lexer.begin(input_begin, input_end);
    auto end = lexer.end();
    REQUIRE_THROWS_AS(*token, lexer_exception<decltype(input_begin)>);
}