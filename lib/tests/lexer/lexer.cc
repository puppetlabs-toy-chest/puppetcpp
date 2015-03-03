#include <catch.hpp>
#include <puppet/lexer/static_lexer.hpp>
#include <puppet/lexer/lexer.hpp>
#include <limits>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::lexer;

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

    result_type operator()(number_token const& token) const
    {
        ostringstream os;
        os << token;
        return os.str();
    }
};

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

    result_type operator()(number_token const& token) const
    {
        FAIL("not a string token");
        throw runtime_error("not a string token");
    }
};

struct number_token_visitor : boost::static_visitor<number_token const&>
{
    template <typename Iterator>
    result_type operator()(boost::iterator_range<Iterator> const& range) const
    {
        FAIL("not a number token");
        throw runtime_error("not a number token");
    }

    result_type operator()(string_token const& token) const
    {
        FAIL("not a number token");
        throw runtime_error("not a number token");
    }

    result_type operator()(number_token const& token) const
    {
        return token;
    }
};

template <typename Iterator>
void require_token(Iterator& token, Iterator const& end, token_id expected_id, string const& expected_value)
{
    CAPTURE(expected_id);
    CAPTURE(expected_value);

    REQUIRE(token != end);
    token_id id = static_cast<token_id>(token->id());
    REQUIRE(id == expected_id);

    string value = boost::apply_visitor(token_value_visitor(), token->value());
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

    auto const& value = boost::apply_visitor(string_token_visitor(), token->value());
    REQUIRE(value.text() == expected_value);
    REQUIRE(value.format() == expected_format);
    REQUIRE(value.interpolated() == expected_interpolated);
    REQUIRE(value.escaped() == expected_escaped);

    ++token;
}

template <typename Iterator>
void require_number_token(Iterator& token, Iterator const& end, int64_t expected_value, numeric_base expected_base, string const& expected_string)
{
    CAPTURE(expected_string);

    REQUIRE(token != end);
    token_id id = static_cast<token_id>(token->id());
    REQUIRE(id == token_id::number);

    auto const& value = boost::apply_visitor(number_token_visitor(), token->value());
    REQUIRE(value.value().which() == 0);
    REQUIRE(boost::get<int64_t>(value.value()) == expected_value);
    REQUIRE(value.base() == expected_base);

    ostringstream ss;
    ss << value;
    REQUIRE(ss.str() == expected_string);

    ++token;
}

template <typename Iterator>
void require_number_token(Iterator& token, Iterator const& end, long double expected_value, string const& expected_string)
{
    CAPTURE(expected_string);

    REQUIRE(token != end);
    token_id id = static_cast<token_id>(token->id());
    REQUIRE(id == token_id::number);

    auto const& value = boost::apply_visitor(number_token_visitor(), token->value());
    REQUIRE(value.value().which() == 1);
    REQUIRE(boost::get<long double>(value.value()) == Approx(expected_value));
    REQUIRE(value.base() == numeric_base::decimal);

    ostringstream ss;
    ss << value;
    REQUIRE(ss.str() == expected_string);

    ++token;
}

void lex_bad_string(string const& input, size_t expected_offset, size_t expected_line, string const& expected_message)
{
    try
    {
        auto input_begin = lex_begin(input);
        auto input_end = lex_end(input);

        string_static_lexer lexer;
        for (auto token = lexer.begin(input_begin, input_end), end = lexer.end(); token != end; ++token);
        FAIL("no exception was thrown");
    } catch (lexer_exception<lexer_string_iterator> const& ex) {
        REQUIRE(get<0>(ex.location().position()) == expected_offset);
        REQUIRE(get<1>(ex.location().position()) == expected_line);
        REQUIRE(ex.what() == expected_message);
    }
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

    lex_bad_string("\n   @(MALFORMED)\nthis heredoc is MALFORMED", 4, 2, "unexpected end of input while looking for heredoc end tag 'MALFORMED'.");
}

SCENARIO("lexing symbolic tokens")
{
    ifstream input(FIXTURES_DIR "lexer/symbolic_tokens.pp");
    REQUIRE(input);

    auto input_begin = lex_begin(input);
    auto input_end = lex_end(input);

    file_static_lexer lexer([&](token_position const& position, string const& message) {
        FAIL("unexpected warning");
    });
    auto token = lexer.begin(input_begin, input_end);
    auto end = lexer.end();
    require_token(token, end, token_id::left_double_collect, "<<|");
    require_token(token, end, token_id::right_double_collect, "|>>");
    require_token(token, end, token_id::append, "+=");
    require_token(token, end, token_id::remove, "-=");
    require_token(token, end, token_id::equals, "==");
    require_token(token, end, token_id::not_equals, "!=");
    require_token(token, end, token_id::match, "=~");
    require_token(token, end, token_id::not_match, "!~");
    require_token(token, end, token_id::greater_equals, ">=");
    require_token(token, end, token_id::less_equals, "<=");
    require_token(token, end, token_id::fat_arrow, "=>");
    require_token(token, end, token_id::plus_arrow, "+>");
    require_token(token, end, token_id::left_shift, "<<");
    require_token(token, end, token_id::left_collect, "<|");
    require_token(token, end, token_id::right_collect, "|>");
    require_token(token, end, token_id::right_shift, ">>");
    require_token(token, end, token_id::atat, "@@");
    require_token(token, end, token_id::in_edge, "->");
    require_token(token, end, token_id::in_edge_sub, "~>");
    require_token(token, end, token_id::out_edge, "<-");
    require_token(token, end, token_id::out_edge_sub, "<~");
    require_token(token, end, static_cast<token_id>('['), "[");
    require_token(token, end, token_id::array_start, "[");
    require_token(token, end, static_cast<token_id>(']'), "]");
    require_token(token, end, static_cast<token_id>('{'), "{");
    require_token(token, end, static_cast<token_id>('}'), "}");
    require_token(token, end, static_cast<token_id>('('), "(");
    require_token(token, end, static_cast<token_id>(')'), ")");
    require_token(token, end, static_cast<token_id>('='), "=");
    require_token(token, end, static_cast<token_id>('>'), ">");
    require_token(token, end, static_cast<token_id>('<'), "<");
    require_token(token, end, static_cast<token_id>('+'), "+");
    require_token(token, end, static_cast<token_id>('-'), "-");
    require_token(token, end, static_cast<token_id>('/'), "/");
    require_token(token, end, static_cast<token_id>('*'), "*");
    require_token(token, end, static_cast<token_id>('%'), "%");
    require_token(token, end, static_cast<token_id>('.'), ".");
    require_token(token, end, static_cast<token_id>('|'), "|");
    require_token(token, end, static_cast<token_id>('@'), "@");
    require_token(token, end, static_cast<token_id>(':'), ":");
    require_token(token, end, static_cast<token_id>(','), ",");
    require_token(token, end, static_cast<token_id>(';'), ";");
    require_token(token, end, static_cast<token_id>('?'), "?");
    require_token(token, end, static_cast<token_id>('~'), "~");
    REQUIRE(token == end);
}

SCENARIO("lexing keywords")
{
    ifstream input(FIXTURES_DIR "lexer/keywords.pp");
    REQUIRE(input);

    auto input_begin = lex_begin(input);
    auto input_end = lex_end(input);

    file_static_lexer lexer([&](token_position const& position, string const& message) {
        FAIL("unexpected warning");
    });
    auto token = lexer.begin(input_begin, input_end);
    auto end = lexer.end();
    require_token(token, end, token_id::keyword_case, "case");
    require_token(token, end, token_id::keyword_class, "class");
    require_token(token, end, token_id::keyword_default, "default");
    require_token(token, end, token_id::keyword_define, "define");
    require_token(token, end, token_id::keyword_if, "if");
    require_token(token, end, token_id::keyword_elsif, "elsif");
    require_token(token, end, token_id::keyword_else, "else");
    require_token(token, end, token_id::keyword_inherits, "inherits");
    require_token(token, end, token_id::keyword_node, "node");
    require_token(token, end, token_id::keyword_and, "and");
    require_token(token, end, token_id::keyword_or, "or");
    require_token(token, end, token_id::keyword_undef, "undef");
    require_token(token, end, token_id::keyword_in, "in");
    require_token(token, end, token_id::keyword_unless, "unless");
    require_token(token, end, token_id::keyword_function, "function");
    require_token(token, end, token_id::keyword_type, "type");
    require_token(token, end, token_id::keyword_attr, "attr");
    require_token(token, end, token_id::keyword_private, "private");
    require_token(token, end, token_id::keyword_true, "true");
    require_token(token, end, token_id::keyword_false, "false");
    REQUIRE(token == end);
}

SCENARIO("lexing statement calls")
{
    ifstream input(FIXTURES_DIR "lexer/statement_calls.pp");
    REQUIRE(input);

    auto input_begin = lex_begin(input);
    auto input_end = lex_end(input);

    file_static_lexer lexer([&](token_position const& position, string const& message) {
        FAIL("unexpected warning");
    });
    auto token = lexer.begin(input_begin, input_end);
    auto end = lexer.end();
    require_token(token, end, token_id::statement_call, "require");
    require_token(token, end, token_id::statement_call, "realize");
    require_token(token, end, token_id::statement_call, "include");
    require_token(token, end, token_id::statement_call, "contain");
    require_token(token, end, token_id::statement_call, "tag");
    require_token(token, end, token_id::statement_call, "debug");
    require_token(token, end, token_id::statement_call, "info");
    require_token(token, end, token_id::statement_call, "notice");
    require_token(token, end, token_id::statement_call, "warning");
    require_token(token, end, token_id::statement_call, "error");
    require_token(token, end, token_id::statement_call, "fail");
    require_token(token, end, token_id::statement_call, "import");
    REQUIRE(token == end);
}

SCENARIO("lexing numbers")
{
    ifstream input(FIXTURES_DIR "lexer/numbers.pp");
    REQUIRE(input);

    auto input_begin = lex_begin(input);
    auto input_end = lex_end(input);

    file_static_lexer lexer([&](token_position const& position, string const& message) {
        FAIL("unexpected warning");
    });
    auto token = lexer.begin(input_begin, input_end);
    auto end = lexer.end();
    require_number_token(token, end, 0, numeric_base::hexadecimal, "0x0");
    require_number_token(token, end, 0, numeric_base::hexadecimal, "0x0");
    require_number_token(token, end, 0x123456789ABCDEFLL, numeric_base::hexadecimal, "0x123456789abcdef");
    require_number_token(token, end, 0x123456789abcdefLL, numeric_base::hexadecimal, "0x123456789abcdef");
    require_number_token(token, end, 0, numeric_base::octal, "00");
    require_number_token(token, end, 01234567, numeric_base::octal, "01234567");
    require_number_token(token, end, 0, numeric_base::decimal, "0");
    require_number_token(token, end, 1, numeric_base::decimal, "1");
    require_number_token(token, end, 123456789, numeric_base::decimal, "123456789");
    require_number_token(token, end, 123.456, "123.456");
    require_number_token(token, end, 412.000, "412");
    require_number_token(token, end, 583e22, "5.83e+24");
    require_number_token(token, end, 9.456e1, "94.56");
    require_number_token(token, end, 478.456E256, "4.78456e+258");
    require_number_token(token, end, 833.0e-10, "8.33e-08");
    REQUIRE(token == end);

    // Bad hex numbers
    lex_bad_string("0x", 0, 1, "'0x' is not a valid number.");
    lex_bad_string("0X", 0, 1, "'0X' is not a valid number.");
    lex_bad_string("0xnotgood", 0, 1, "'0xnotgood' is not a valid number.");
    lex_bad_string("0x1234NOPE", 0, 1, "'0x1234NOPE' is not a valid number.");
    lex_bad_string("0xFFFFFFFFFFFFFFFFFFFFFFFF", 0, 1, "'0xFFFFFFFFFFFFFFFFFFFFFFFF' is not in the range of -9223372036854775808 to 9223372036854775807.");

    // Bad octal numbers
    lex_bad_string("08", 0, 1, "'08' is not a valid number.");
    lex_bad_string("012349", 0, 1, "'012349' is not a valid number.");
    lex_bad_string("0123bad", 0, 1, "'0123bad' is not a valid number.");
    lex_bad_string("07777777777777777777777777777777777777777777", 0, 1, "'07777777777777777777777777777777777777777777' is not in the range of -9223372036854775808 to 9223372036854775807.");

    // Bad decimal numbers
    lex_bad_string("123bAd", 0, 1, "'123bAd' is not a valid number.");
    lex_bad_string("9223372036854775808", 0, 1, "'9223372036854775808' is not in the range of -9223372036854775808 to 9223372036854775807.");

    // Bad floating point numbers
    lex_bad_string("123Bad.456", 0, 1, "'123Bad.456' is not a valid number.");
    lex_bad_string("123.456D0H", 0, 1, "'123.456D0H' is not a valid number.");
    lex_bad_string("123bAd.456D0H3", 0, 1, "'123bAd.456D0H3' is not a valid number.");
    lex_bad_string("123.0e", 0, 1, "'123.0e' is not a valid number.");
    lex_bad_string("123.0e-", 0, 1, "'123.0e-' is not a valid number.");
    lex_bad_string("123.0ebad", 0, 1, "'123.0ebad' is not a valid number.");
    lex_bad_string("123bad.2bad2e-bad", 0, 1, "'123bad.2bad2e-bad' is not a valid number.");
    lex_bad_string("1e100000", 0, 1, (boost::format("'1e100000' is not in the range of %1% to %2%.") % numeric_limits<long double>::min() % numeric_limits<long double>::max()).str());
}
