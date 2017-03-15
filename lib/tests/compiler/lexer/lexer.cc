#include <catch.hpp>
#include <puppet/compiler/lexer/static_lexer.hpp>
#include <puppet/compiler/lexer/lexer.hpp>
#include <limits>
#include <boost/format.hpp>

using namespace std;
using namespace puppet::compiler;
using namespace puppet::compiler::lexer;

template <typename Iterator>
void require_token(Iterator& token, Iterator const& end, token_id expected_id, string const& expected_value)
{
    CAPTURE(expected_id);
    CAPTURE(expected_value);

    REQUIRE((token != end));
    token_id id = static_cast<token_id>(token->id());
    REQUIRE(id == expected_id);

    string value = boost::lexical_cast<std::string>(token->value());
    REQUIRE(value == expected_value);

    ++token;
}

template <typename Iterator>
void require_string_token(
    Iterator& token,
    Iterator const& end,
    string const& expected_value,
    string const& expected_format = {},
    size_t expected_margin = 0)
{
    CAPTURE(expected_value);

    REQUIRE((token != end));
    token_id id = static_cast<token_id>(token->id());
    REQUIRE(id == token_id::string);

    auto value = boost::get<string_token>(&token->value());
    REQUIRE(value);
    REQUIRE(value->value == expected_value);
    REQUIRE(value->format == expected_format);
    REQUIRE(value->margin == expected_margin);

    ++token;
}

template <typename Iterator>
void require_string_text_token(
    Iterator& token,
    Iterator const& end,
    string const& expected_text)
{
    CAPTURE(expected_text);

    REQUIRE((token != end));
    token_id id = static_cast<token_id>(token->id());
    REQUIRE(id == token_id::string_text);

    auto value = boost::get<string_text_token>(&token->value());
    REQUIRE(value);
    REQUIRE(value->text == expected_text);

    ++token;
}

template <typename Iterator>
void require_interpolated_string_token(
    Iterator& token,
    Iterator const& end,
    function<void(Iterator&)> const& callback,
    string const& expected_format = {},
    size_t expected_margin = 0)
{
    REQUIRE((token != end));
    token_id id = static_cast<token_id>(token->id());
    REQUIRE(id == token_id::string_start);
    {
        auto value = boost::get<string_start_token>(&token->value());
        REQUIRE(value);
        REQUIRE(value->format == expected_format);
    }
    ++token;
    REQUIRE((token != end));
    callback(token);
    REQUIRE((token != end));
    id = static_cast<token_id>(token->id());
    REQUIRE(id == token_id::string_end);
    {
        auto value = boost::get<string_end_token>(&token->value());
        REQUIRE(value);
        REQUIRE(value->margin == expected_margin);
    }
    ++token;
}

template <typename Iterator>
void require_interpolated_string_token(
    Iterator& token,
    Iterator const& end,
    string const& expected_value,
    string const& expected_format = {},
    size_t expected_margin = 0)
{
    CAPTURE(expected_value);

    require_interpolated_string_token<Iterator>(token, end, [&](auto& t) {
        if (!expected_value.empty()) {
            require_string_text_token(t, end, expected_value);
        }
    }, expected_format, expected_margin);
}

template <typename Iterator>
void require_number_token(Iterator& token, Iterator const& end, int64_t expected_value, numeric_base expected_base, string const& expected_string)
{
    CAPTURE(expected_string);

    REQUIRE((token != end));
    token_id id = static_cast<token_id>(token->id());
    REQUIRE(id == token_id::number);

    auto num_token = boost::get<number_token>(&token->value());
    REQUIRE(num_token);
    REQUIRE(num_token->value.which() == 0);
    REQUIRE(boost::get<int64_t>(num_token->value) == expected_value);
    REQUIRE(num_token->base == expected_base);

    ostringstream ss;
    ss << *num_token;
    REQUIRE(ss.str() == expected_string);

    ++token;
}

template <typename Iterator>
void require_number_token(Iterator& token, Iterator const& end, double expected_value, string const& expected_string)
{
    CAPTURE(expected_string);

    REQUIRE((token != end));
    token_id id = static_cast<token_id>(token->id());
    REQUIRE(id == token_id::number);

    auto num_token = boost::get<number_token>(&token->value());
    REQUIRE(num_token);
    REQUIRE(num_token->value.which() == 1);
    REQUIRE(boost::get<double>(num_token->value) == Approx(expected_value));
    REQUIRE(num_token->base == numeric_base::decimal);

    ostringstream ss;
    ss << *num_token;
    REQUIRE(ss.str() == expected_string);

    ++token;
}

void lex_bad_string(string const& input, size_t expected_offset, size_t expected_line, string const& expected_message)
{
    CAPTURE(expected_message);
    try
    {
        auto input_begin = lex_begin(input);
        auto input_end = lex_end(input);

        string_static_lexer lexer;
        for (auto token = lexer.begin(input_begin, input_end), end = lexer.end(); token != end; ++token);
        FAIL("no exception was thrown");
    } catch (lexer_exception<lexer_string_iterator> const& ex) {
        REQUIRE(ex.begin().position().offset() == expected_offset);
        REQUIRE(ex.begin().position().line() == expected_line);
        REQUIRE(ex.what() == expected_message);
    }
}

SCENARIO("lexing single quoted strings", "[lexer]")
{
    ifstream input(FIXTURES_DIR "compiler/lexer/single_quoted_strings.pp");
    REQUIRE(input);

    auto input_begin = lex_begin(input);
    auto input_end = lex_end(input);

    file_static_lexer lexer;
    auto token = lexer.begin(input_begin, input_end);
    auto end = lexer.end();
    require_string_token(token, end, "");
    require_string_token(token, end, "this is a string");
    require_string_token(token, end, "' this string is quoted '");
    require_string_token(token, end, "this back\\slash is not escaped");
    require_string_token(token, end, "this back\\slash is escaped");
    require_string_token(token, end, " this line\n has a\n break!\n");
    REQUIRE(token == end);

    lex_bad_string(R"( 'this string doesn\'t close\' )", 1, 1, "could not find a matching closing quote.");
}

SCENARIO("getting ranges of tokens", "[lexer]")
{
    auto const ranges = vector<pair<position, position>>({
        make_pair(position{0, 1}, position {2, 1}),
        make_pair(position{5, 3}, position {23, 3}),
        make_pair(position{31, 5}, position {60, 5}),
        make_pair(position{62, 7}, position {94, 7}),
        make_pair(position{97, 9}, position {126, 9}),
        make_pair(position{131, 11}, position {159, 14})
    });
    WHEN("lexing a file") {
        ifstream input(FIXTURES_DIR "compiler/lexer/single_quoted_strings.pp");
        REQUIRE(input);
        REQUIRE(get_last_position(input).offset() == 159);

        auto input_begin = lex_begin(input);
        auto input_end = lex_end(input);

        file_static_lexer lexer;
        auto token = lexer.begin(input_begin, input_end);
        auto end = lexer.end();

        for (auto const& range : ranges) {
            REQUIRE((token != end));
            position begin, end;
            tie(begin, end) = boost::apply_visitor(token_range_visitor(), token->value());
            REQUIRE(begin == range.first);
            REQUIRE(end == range.second);
            ++token;
        }
        REQUIRE(token == end);

        THEN("the text and column for a position should match what's expected") {
            auto info = get_line_info(input, ranges[4].first.offset(), 1);
            REQUIRE(info.column == 2);
            REQUIRE(info.length == 1);
            REQUIRE(info.text == " 'this back\\\\slash is escaped'");
        }
        THEN("the text and column for the last position should match the last line") {
            auto info = get_line_info(input, get_last_position(input).offset(), 1);
            REQUIRE(info.column == 2);
            REQUIRE(info.length == 0);
            REQUIRE(info.text == "'");
        }
    }
    WHEN("lexing a string") {
        ifstream file(FIXTURES_DIR "compiler/lexer/single_quoted_strings.pp");
        REQUIRE(file);
        ostringstream buffer;
        buffer << file.rdbuf();
        string contents = buffer.str();

        AND_WHEN("given the string as input") {
            auto& input = contents;
            REQUIRE(get_last_position(input).offset() == 159);
            auto input_begin = lex_begin(input);
            auto input_end = lex_end(input);

            string_static_lexer lexer;
            auto token = lexer.begin(input_begin, input_end);
            auto end = lexer.end();

            for (auto const& range : ranges) {
                REQUIRE((token != end));
                position begin, end;
                tie(begin, end) = boost::apply_visitor(token_range_visitor(), token->value());
                REQUIRE(begin == range.first);
                REQUIRE(end == range.second);
                ++token;
            }
            REQUIRE(token == end);

            string text;
            size_t column;
            THEN("the text and column for a position should match what's expected") {
                auto line = get_line_info(input, ranges[4].first.offset(), 1);
                REQUIRE(line.column == 2);
                REQUIRE(line.length == 1);
                REQUIRE(line.text == " 'this back\\\\slash is escaped'");
            }
            THEN("the text and column for the last position should match the last line") {
                auto line = get_line_info(input, get_last_position(input).offset(), 1);
                REQUIRE(line.column == 2);
                REQUIRE(line.length == 0);
                REQUIRE(line.text == "'");
            }
        }
        AND_WHEN("using an iterator range as input") {
            auto input = boost::make_iterator_range(lex_begin(contents), lex_end(contents));
            REQUIRE(get_last_position(input).offset() == 159);
            auto input_begin = lex_begin(input);
            auto input_end = lex_end(input);

            string_static_lexer lexer;
            auto token = lexer.begin(input_begin, input_end);
            auto end = lexer.end();

            for (auto const& range : ranges) {
                REQUIRE((token != end));
                position begin, end;
                tie(begin, end) = boost::apply_visitor(token_range_visitor(), token->value());
                REQUIRE(begin == range.first);
                REQUIRE(end == range.second);
                ++token;
            }
            REQUIRE(token == end);
        }
    }
}

SCENARIO("lexing double quoted strings", "[lexer]")
{
    ifstream input(FIXTURES_DIR "compiler/lexer/double_quoted_strings.pp");
    REQUIRE(input);

    auto input_begin = lex_begin(input);
    auto input_end = lex_end(input);

    file_static_lexer lexer;
    auto token = lexer.begin(input_begin, input_end);
    auto end = lexer.end();
    require_interpolated_string_token(token, end, "");
    require_interpolated_string_token(token, end, "this is a string");
    require_interpolated_string_token(token, end, "\" this string is quoted \"");
    require_interpolated_string_token(token, end, "this ' is escaped");
    require_interpolated_string_token(token, end, "this \\ is escaped");
    require_interpolated_string_token(token, end, "this \" is escaped");
    require_interpolated_string_token(token, end, "this \n is escaped");
    require_interpolated_string_token(token, end, "this \r is escaped");
    require_interpolated_string_token(token, end, "this \t is escaped");
    require_interpolated_string_token(token, end, "this ' ' is escaped");
    require_interpolated_string_token(token, end, "this \u263A is a unicode character");
    require_interpolated_string_token(token, end, "this string\n   has a\n   line break!\n   ");
    require_interpolated_string_token(token, end, "this \\f is not a valid escape");
    require_interpolated_string_token<decltype(token)>(token, end, [&](auto& token) {
        require_token(token, end, token_id::variable, "$hello");
        require_string_text_token(token, end, " $world");
    });
    require_interpolated_string_token<decltype(token)>(token, end, [&](auto& token) {
        require_string_text_token(token, end, "1 + 1 = ");
        require_token(token, end, token_id::interpolation_start, "${");
        require_number_token(token, end, 1, numeric_base::decimal, "1");
        require_token(token, end, static_cast<token_id>('+'), "+");
        require_number_token(token, end, 1, numeric_base::decimal, "1");
        require_token(token, end, token_id::interpolation_end, "}");
        require_string_text_token(token, end, "!");
    });
    require_interpolated_string_token<decltype(token)>(token, end, [&](auto& token) {
        require_string_text_token(token, end, "$foo = ");
        require_token(token, end, token_id::interpolation_start, "${");
        require_token(token, end, token_id::variable, "foo");
        require_token(token, end, token_id::interpolation_end, "}");
        require_string_text_token(token, end, "!");
    });
    require_interpolated_string_token<decltype(token)>(token, end, [&](auto& token) {
        require_string_text_token(token, end, "$foo[0] = ");
        require_token(token, end, token_id::interpolation_start, "${");
        require_token(token, end, token_id::variable, "foo");
        require_token(token, end, static_cast<token_id>('['), "[");
        require_number_token(token, end, 0, numeric_base::decimal, "0");
        require_token(token, end, static_cast<token_id>(']'), "]");
        require_token(token, end, token_id::interpolation_end, "}");
        require_string_text_token(token, end, "!");
    });
    require_interpolated_string_token<decltype(token)>(token, end, [&](auto& token) {
        require_string_text_token(token, end, "$foo.bar = ");
        require_token(token, end, token_id::interpolation_start, "${");
        require_token(token, end, token_id::variable, "foo");
        require_token(token, end, static_cast<token_id>('.'), ".");
        require_token(token, end, token_id::name, "bar");
        require_token(token, end, token_id::interpolation_end, "}");
        require_string_text_token(token, end, "!");
    });
    REQUIRE(token == end);

    lex_bad_string(R"( "this string doesn't close\" )", 1, 1, "could not find a matching closing quote.");
}

SCENARIO("lexing heredocs", "[lexer]")
{
    ifstream input(FIXTURES_DIR "compiler/lexer/heredocs.pp");
    REQUIRE(input);

    auto input_begin = lex_begin(input);
    auto input_end = lex_end(input);

    file_static_lexer lexer;
    auto token = lexer.begin(input_begin, input_end);
    auto end = lexer.end();
    require_string_token(token, end, "");
    require_string_token(token, end, "this\nis\na\nheredoc\n");
    require_string_token(token, end, "first\n");
    require_string_token(token, end, "hello");
    require_string_token(token, end, "second\n");
    require_string_token(token, end, "world");
    require_string_token(token, end, "third\n");
    require_string_token(token, end, "{\n  \"hello\": \"world\"\n}\n", "json");
    require_string_token(token, end, "first: \\\\\\t\\s\\r\\n\\u263A\\$\\\nsecond!\n");
    require_string_token(token, end, "first: \\\t\\s\\r\\n\\u263A\\$\\\nsecond!\n");
    require_string_token(token, end, "first: \\\t \\r\\n\\u263A\\$\\\nsecond!\n");
    require_string_token(token, end, "first: \\\t \r\\n\\u263A\\$\\\nsecond!\n");
    require_string_token(token, end, "first: \\\t \r\n\\u263A\\$\\\nsecond!\n");
    require_string_token(token, end, "first: \\\t \r\n\u263A\\$\\\nsecond!\n");
    require_string_token(token, end, "first: \\\t \r\n\u263A$\\\nsecond!\n");
    require_string_token(token, end, "first: \\\t \r\n\u263A$second!\n");
    require_string_token(token, end, "first: \\\t \r\n\u263A$second!\n");
    require_interpolated_string_token<decltype(token)>(token, end, [&](auto& token) {
        require_token(token, end, token_id::variable, "$hello");
        require_string_text_token(token, end, " \\");
        require_token(token, end, token_id::variable, "$world");
        require_string_text_token(token, end, "\n");
    });
    require_interpolated_string_token<decltype(token)>(token, end, [&](auto& token) {
        require_token(token, end, token_id::variable, "$hello");
        require_string_text_token(token, end, " $world\n");
    });
    require_interpolated_string_token<decltype(token)>(token, end, [&](auto& token) {
        require_string_text_token(token, end, "1 + 1 = ");
        require_token(token, end, token_id::interpolation_start, "${");
        require_number_token(token, end, 1, numeric_base::decimal, "1");
        require_token(token, end, static_cast<token_id>('+'), "+");
        require_number_token(token, end, 1, numeric_base::decimal, "1");
        require_token(token, end, token_id::interpolation_end, "}");
        require_string_text_token(token, end, "\n");
    });
    require_interpolated_string_token<decltype(token)>(token, end, [&](auto& token) {
        require_string_text_token(token, end, "$foo = ");
        require_token(token, end, token_id::interpolation_start, "${");
        require_token(token, end, token_id::variable, "foo");
        require_token(token, end, token_id::interpolation_end, "}");
        require_string_text_token(token, end, "\n");
    });
    require_interpolated_string_token<decltype(token)>(token, end, [&](auto& token) {
        require_string_text_token(token, end, "$foo[0] = ");
        require_token(token, end, token_id::interpolation_start, "${");
        require_token(token, end, token_id::variable, "foo");
        require_token(token, end, static_cast<token_id>('['), "[");
        require_number_token(token, end, 0, numeric_base::decimal, "0");
        require_token(token, end, static_cast<token_id>(']'), "]");
        require_token(token, end, token_id::interpolation_end, "}");
        require_string_text_token(token, end, "\n");
    });
    require_interpolated_string_token<decltype(token)>(token, end, [&](auto& token) {
        require_string_text_token(token, end, "$foo.bar = ");
        require_token(token, end, token_id::interpolation_start, "${");
        require_token(token, end, token_id::variable, "foo");
        require_token(token, end, static_cast<token_id>('.'), ".");
        require_token(token, end, token_id::name, "bar");
        require_token(token, end, token_id::interpolation_end, "}");
        require_string_text_token(token, end, "\n");
    });
    require_string_token(token, end, "this is NOT the end\n");
    require_string_token(token, end, "this is one line");
    require_string_token(token, end, "    this text\n     is\n      aligned\n", {}, 4);
    require_string_token(token, end, "    this text\n     is\n      aligned", {}, 4);
    require_interpolated_string_token(token, end, "    this $text\n     is\n      aligned", "json", 5);
    REQUIRE(token == end);

    lex_bad_string("\n   @(MALFORMED)\nthis heredoc is MALFORMED", 4, 2, "could not find a matching heredoc end tag 'MALFORMED'.");
    lex_bad_string("\n   @(MALFORMED/tt)\nthis heredoc is MALFORMED", 4, 2, "heredoc escape 't' may only appear once in the list.");
    lex_bad_string("\n   @(MALFORMED/z)\nthis heredoc is\nMALFORMED", 4, 2, "invalid heredoc escape 'z': only t, r, n, s, u, L, and $ are allowed.");
}

SCENARIO("lexing symbolic tokens", "[lexer]")
{
    ifstream input(FIXTURES_DIR "compiler/lexer/symbolic_tokens.pp");
    REQUIRE(input);

    auto input_begin = lex_begin(input);
    auto input_end = lex_end(input);

    file_static_lexer lexer;
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

SCENARIO("lexing keywords", "[lexer]")
{
    ifstream input(FIXTURES_DIR "compiler/lexer/keywords.pp");
    REQUIRE(input);

    auto input_begin = lex_begin(input);
    auto input_end = lex_end(input);

    file_static_lexer lexer;
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
    require_token(token, end, token_id::keyword_produces, "produces");
    require_token(token, end, token_id::keyword_consumes, "consumes");
    require_token(token, end, token_id::keyword_application, "application");
    require_token(token, end, token_id::keyword_site, "site");
    require_token(token, end, token_id::keyword_break, "break");
    require_token(token, end, token_id::keyword_true, "true");
    require_token(token, end, token_id::keyword_false, "false");
    REQUIRE(token == end);
}

SCENARIO("using is_keyword", "[lexer]")
{
    GIVEN("a token that is not a keyword") {
        auto token = token_id::name;
        THEN("is_keyword should return false") {
            CAPTURE(token);
            REQUIRE_FALSE(is_keyword(token));
        }
    }
    GIVEN("any keyword token") {
        for (auto token = static_cast<size_t>(token_id::first_keyword) + 1; token < static_cast<size_t>(token_id::last_keyword); ++token) {
            THEN("is_keyword should return true") {
                CAPTURE(token);
                REQUIRE(is_keyword(static_cast<token_id>(token)));
            }
        }
    }
}

SCENARIO("lexing statement calls", "[lexer]")
{
    ifstream input(FIXTURES_DIR "compiler/lexer/statement_calls.pp");
    REQUIRE(input);

    auto input_begin = lex_begin(input);
    auto input_end = lex_end(input);

    file_static_lexer lexer;
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
    require_token(token, end, token_id::statement_call, "err");
    require_token(token, end, token_id::statement_call, "fail");
    require_token(token, end, token_id::statement_call, "import");
    REQUIRE(token == end);
}

SCENARIO("lexing numbers", "[lexer]")
{
    ifstream input(FIXTURES_DIR "compiler/lexer/numbers.pp");
    REQUIRE(input);

    auto input_begin = lex_begin(input);
    auto input_end = lex_end(input);

    file_static_lexer lexer;
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
    lex_bad_string("1e100000", 0, 1, (boost::format("'1e100000' is not in the range of %1% to %2%.") % numeric_limits<double>::lowest() % numeric_limits<double>::max()).str());
}

SCENARIO("lexing tokens with values", "[lexer]")
{
    ifstream input(FIXTURES_DIR "compiler/lexer/value_tokens.pp");
    REQUIRE(input);

    auto input_begin = lex_begin(input);
    auto input_end = lex_end(input);

    file_static_lexer lexer;
    auto token = lexer.begin(input_begin, input_end);
    auto end = lexer.end();
    require_token(token, end, token_id::variable, "$foo");
    require_token(token, end, token_id::type, "Bar::Baz");
    require_token(token, end, token_id::name, "::snap::crackle::pop");
    require_token(token, end, token_id::bare_word, "_foo_-_bar_");
    require_token(token, end, token_id::regex, "/regex/");
    REQUIRE(token == end);
}

SCENARIO("lexing comments", "[lexer]")
{
    ifstream input(FIXTURES_DIR "compiler/lexer/comments.pp");
    REQUIRE(input);

    auto input_begin = lex_begin(input);
    auto input_end = lex_end(input);

    file_static_lexer lexer;
    auto token = lexer.begin(input_begin, input_end);
    auto end = lexer.end();
    require_token(token, end, token_id::name, "foo");
    require_token(token, end, token_id::name, "bar");
    require_token(token, end, token_id::name, "baz");
    require_token(token, end, token_id::unclosed_comment, "/*");
    require_token(token, end, token_id::name, "jam");
    REQUIRE(token == end);
}

SCENARIO("lexing EPP", "[lexer]")
{
    ifstream input(FIXTURES_DIR "compiler/lexer/epp.pp");
    REQUIRE(input);

    auto input_begin = lex_begin(input);
    auto input_end = lex_end(input);

    file_static_lexer lexer;
    auto token = lexer.begin(input_begin, input_end, EPP_STATE);
    auto end = lexer.end();
    require_token(token, end, token_id::epp_render_string, "foo ");
    require_token(token, end, token_id::keyword_if, "if");
    require_token(token, end, token_id::epp_render_string, "  \nbar");
    require_token(token, end, token_id::keyword_elsif, "elsif");
    require_token(token, end, token_id::epp_render_string, "  \n");
    require_token(token, end, token_id::keyword_unless, "unless");
    require_token(token, end, token_id::epp_render_string, "  \n  ");
    require_token(token, end, token_id::keyword_class, "class");
    require_token(token, end, token_id::keyword_define, "define");
    require_token(token, end, token_id::name, "nope");
    require_token(token, end, token_id::epp_render_string, "  nope  \nbaz ");
    require_token(token, end, token_id::epp_render_expression, "<%=");
    require_token(token, end, token_id::statement_call, "notice");
    require_token(token, end, token_id::epp_end, "%>");
    require_token(token, end, token_id::epp_render_string, "  \nclass ");
    require_token(token, end, token_id::epp_render_expression, "<%=");
    require_token(token, end, token_id::statement_call, "err");
    require_token(token, end, token_id::epp_end_trim, "-%>  \n");
    require_token(token, end, token_id::epp_render_string, "  \n");
    require_token(token, end, token_id::epp_render_string, "\n");
    require_token(token, end, token_id::epp_render_string, "<%");
    REQUIRE(input_begin.epp_end());
    require_token(token, end, token_id::epp_render_string, ">  \n");
    require_token(token, end, token_id::name, "unclosed");
    REQUIRE_FALSE(input_begin.epp_end());
    REQUIRE(token == end);
}

SCENARIO("lexing regex tokens", "[lexer]")
{
    ifstream input(FIXTURES_DIR "compiler/lexer/regex.pp");
    REQUIRE(input);

    auto input_begin = lex_begin(input);
    auto input_end = lex_end(input);

    file_static_lexer lexer;
    auto token = lexer.begin(input_begin, input_end);
    auto end = lexer.end();
    require_token(token, end, token_id::regex, "//");
    require_token(token, end, static_cast<token_id>('+'), "+");
    require_token(token, end, token_id::regex, "/\\//");
    require_token(token, end, static_cast<token_id>('+'), "+");
    require_token(token, end, token_id::regex, "//");
    require_token(token, end, static_cast<token_id>('/'), "/");
    require_token(token, end, token_id::regex, "//");
    require_token(token, end, static_cast<token_id>('+'), "+");
    require_token(token, end, token_id::regex, "/foo/");
    require_token(token, end, static_cast<token_id>('/'), "/");
    require_token(token, end, token_id::regex, "/bar/");
    require_token(token, end, static_cast<token_id>('+'), "+");
    require_token(token, end, static_cast<token_id>('/'), "/");
    require_token(token, end, token_id::name, "nope");
    require_token(token, end, static_cast<token_id>('/'), "/");
    require_number_token(token, end, 6, numeric_base::decimal, "6");
    require_token(token, end, static_cast<token_id>('/'), "/");
    require_number_token(token, end, 2, numeric_base::decimal, "2");
    require_token(token, end, static_cast<token_id>('/'), "/");
    require_number_token(token, end, 1, numeric_base::decimal, "1");
    require_number_token(token, end, 10, numeric_base::decimal, "10");
    require_token(token, end, static_cast<token_id>('/'), "/");
    require_number_token(token, end, 5, numeric_base::decimal, "5");
    require_token(token, end, static_cast<token_id>('/'), "/");
    require_number_token(token, end, 2, numeric_base::decimal, "2");
    REQUIRE(token == end);
}
