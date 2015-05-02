#include <puppet/runtime/string_interpolator.hpp>
#include <puppet/parser/parser.hpp>
#include <boost/format.hpp>
#include <codecvt>
#include <cctype>
#include <sstream>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::parser;
using namespace puppet::runtime::values;

namespace puppet { namespace runtime {

    template <typename Value>
    struct hex_to
    {
        operator Value() const
        {
            return value;
        }

        friend istream& operator>>(istream& in, hex_to& out)
        {
            in >> std::hex >> out.value;
            return in;
        }

     private:
        Value value;
    };

    static bool tokenize_interpolation(
        string& result,
        expression_evaluator& evaluator,
        lexer_string_iterator& begin,
        lexer_string_iterator const& end,
        function<token_position(token_position const&)> const& calculate_position)
    {
        try {
            bool bracket = begin != end && *begin == '{';
            string_static_lexer lexer;

            // Check for keyword or name
            value const* val = nullptr;

            auto current = begin;
            auto token_begin = lexer.begin(current, end);
            auto token_end = lexer.end();

            // Check for the following forms:
            // {keyword}, {name/bare_word}, {decimal}
            // name, decimal

            // Skip past the opening bracket
            if (bracket && token_begin != token_end && token_begin->id() == '{') {
                ++token_begin;
            }

            if (token_begin != token_end &&
                (
                    is_keyword(static_cast<token_id>(token_begin->id())) ||
                    token_begin->id() == static_cast<size_t>(token_id::name) ||
                    token_begin->id() == static_cast<size_t>(token_id::bare_word)
                )) {
                auto token = get<boost::iterator_range<lexer_string_iterator>>(&token_begin->value());
                if (!token) {
                    return false;
                }
                val = evaluator.context().lookup(string(token->begin(), token->end()));
            } else if (token_begin != token_end && token_begin->id() == static_cast<size_t>(token_id::number)) {
                auto token = get<number_token>(&token_begin->value());
                if (!token) {
                    return false;
                }
                if (token->base() != numeric_base::decimal || token->value().which() != 0) {
                    throw evaluation_exception(calculate_position(token->position()), (boost::format("'%1%' is not a valid match variable name.") % *token).str());
                }
                val = evaluator.context().current().get(get<int64_t>(token->value()));
            } else {
                return false;
            }

            // If bracketed, look for the closing bracket
            if (bracket) {
                ++token_begin;

                // Check for not parsed or missing a closing } token
                if (bracket && (token_begin == token_end || token_begin->id() != '}')) {
                    return false;
                }
            }

            // Output the variable
            if (val) {
                ostringstream ss;
                ss << *val;
                result += ss.str();
            }

            // Update to where we stopped lexing
            begin = current;
            return true;
        } catch (lexer_exception<lexer_string_iterator> const& ex) {
            throw evaluation_exception(calculate_position(ex.location().position()), ex.what());
        }
    }

    static boost::optional<value> transform_expression(expression_evaluator& evaluator, ast::expression const& expression)
    {
        // Check for a postfix expression
        auto postfix = boost::get<ast::postfix_expression>(&expression.primary());
        if (!postfix || postfix->subexpressions().empty()) {
            return boost::none;
        }

        // Check for access or method call
        auto& subexpression = postfix->subexpressions().front();
        if (!boost::get<ast::access_expression>(&subexpression) &&
            !boost::get<ast::method_call_expression>(&subexpression)) {
            return boost::none;
        }

        // If the expression is a name followed by an access operation or method call, treat as a variable
        auto basic = boost::get<ast::basic_expression>(&postfix->primary());
        if (!basic) {
            return boost::none;
        }

        token_position variable_position;
        string variable_name;

        // Check for name
        auto name = boost::get<ast::name>(basic);
        if (name) {
            variable_position = name->position();
            variable_name = name->value();
        } else {
            // Also check for bare word
            auto word = boost::get<ast::bare_word>(basic);
            if (word) {
                variable_position = word->position();
                variable_name = word->value();
            }
        }
        if (variable_name.empty()) {
            return boost::none;
        }
        return evaluator.evaluate(ast::expression(ast::postfix_expression(ast::basic_expression(ast::variable(std::move(variable_name), std::move(variable_position))), postfix->subexpressions()), expression.binary()));
    }

    string_interpolator::string_interpolator(expression_evaluator& evaluator) :
        _evaluator(evaluator)
    {
    }

    string string_interpolator::interpolate(token_position const& position, string const& text, string const& escapes, char quote, bool full, int margin, bool remove_break)
    {
        // Helper function for calculating a position inside the string being interpolated
        function<token_position(token_position const&)> calculate_position = [&](token_position const& other) {
            return make_tuple(get<0>(position) + get<0>(other) + (quote ? 1 : 0), get<1>(position) + get<1>(other) - 1);
        };

        lexer_string_iterator begin(text.begin());
        lexer_string_iterator end(text.end());

        string result;
        result.reserve(text.size());

        int current_margin = margin;
        while (begin != end) {
            // This logic handles heredocs with margin specifiers (margin > 0)
            for (; current_margin > 0 && begin != end; ++begin) {
                // If we've found a non-whitespace character, we're done
                if (*begin != ' ' && *begin != '\t') {
                    break;
                }
                // If we've found a tab, decrement by the tab width
                if (*begin == '\t') {
                    current_margin -= (current_margin > LEXER_TAB_WIDTH ? LEXER_TAB_WIDTH : current_margin);
                } else {
                    current_margin -= 1;
                }
            }
            if (begin == end) {
                break;
            }

            // No more margin for this line
            current_margin = 0;

            // Perform escape replacements
            if (*begin == '\\' && !escapes.empty()) {
                auto next = begin;
                ++next;
                if (next != end && *next == '\r') {
                    ++next;
                }
                if (next != end && escapes.find(*next) != string::npos) {
                    bool success = true;
                    switch (*next) {
                        case 'r':
                            result += '\r';
                            break;

                        case 'n':
                            result += '\n';
                            break;

                        case 't':
                            result += '\t';
                            break;

                        case 's':
                            result += ' ';
                            break;

                        case 'u':
                            success = write_unicode_escape_sequence(calculate_position(begin.position()), ++next, end, result);
                            break;

                        case '\n':
                            // Treat as new line, so reset the margin
                            current_margin = margin;
                            break;

                        case '$':
                            result += '$';
                            break;

                        default:
                            result += *next;
                            break;
                    }
                    if (success) {
                        begin = ++next;
                        continue;
                    }
                } else if (next != end) {
                    // Emit a warning for invalid escape sequence (unless single quoted string)
                    if (quote != '\'') {
                        _evaluator.context().warn(calculate_position(begin.position()), (boost::format("invalid escape sequence '\\%1%'.") % *next).str());
                    }
                }
            } else if (*begin == '\n') {
                // Reset the margin
                current_margin = margin;
            } else if (full && *begin == '$') {
                auto next = begin;
                ++next;

                if (next != end && !isspace(*next)) {
                    // First attempt to interpolate using the lexer
                    if (tokenize_interpolation(result, _evaluator, next, end, calculate_position)) {
                        begin = next;
                        continue;
                    }
                    // Otherwise, check for expression form
                    if (*next == '{') {
                        try {
                            // Parse the rest of the string as a manifest
                            // The parsing will stop at the first unmatched } token
                            auto manifest = parser::parser::parse(next, end, true);
                            if (manifest.body()) {
                                // Evaluate the body and add the result to the string
                                value val;
                                bool first = true;
                                for (auto& expression : *manifest.body()) {
                                    // For the first expression, transform certain constructs to their "variable" forms
                                    if (first) {
                                        first = false;
                                        auto result = transform_expression(_evaluator, expression);
                                        if (result) {
                                            val = *result;
                                            continue;
                                        }
                                    }
                                    val = _evaluator.evaluate(expression);
                                }
                                ostringstream ss;
                                ss << val;
                                result += ss.str();
                            }

                            // Move past where parsing stopped (must have been at the closing })
                            begin = lexer_string_iterator(text.begin() + get<0>(manifest.end()));
                            begin.position(manifest.end());
                            ++begin;
                            continue;
                        } catch (parser::parse_exception const& ex) {
                            throw evaluation_exception(calculate_position(ex.position()), ex.what());
                        } catch (evaluation_exception const& ex) {
                            throw evaluation_exception(calculate_position(ex.position()), ex.what());
                        }
                    }
                }
            }

            result += *begin++;
        }

        // Remove the trailing line break if instructed to do so
        if (remove_break) {
            if (boost::ends_with(text, "\n")) {
                result.pop_back();
            }
            if (boost::ends_with(text, "\r")) {
                result.pop_back();
            }
        }
        return result;
    }

    bool string_interpolator::write_unicode_escape_sequence(token_position const& position, lexer_string_iterator& begin, lexer_string_iterator const& end, string& result, bool four_characters)
    {
        size_t count = four_characters ? 4 : 8;

        // Use a buffer that can store up to 8 characters (nnnn or nnnnnnnn)
        char buffer[9] = {};
        size_t read = 0;
        for (; begin != end; ++begin) {
            if (!isxdigit(*begin)) {
                break;
            }
            buffer[read] = *begin;
            if (++read == 4) {
                break;
            }
        }
        if (read != count) {
            _evaluator.context().warn(position, (boost::format("expected %1% hexadecimal digits but found %2% for unicode escape sequence.") % count % read).str());
            return false;
        }

        // Convert the input to a utf32 character (supports both four or eight hex characters)
        char32_t from;
        try {
            from = static_cast<char32_t>(boost::lexical_cast<hex_to<uint32_t>>(buffer));
        } catch (boost::bad_lexical_cast const&) {
            _evaluator.context().warn(position, "invalid unicode escape sequence.");
            return false;
        }

        // Convert the utf32 character to utf8 bytes (maximum is 4 bytes)
        codecvt_utf8<char32_t> converter;
        char32_t const* next_from = nullptr;
        char* next_to = nullptr;
        auto state = mbstate_t();
        converter.out(state, &from, &from + 1, next_from, buffer, &buffer[0] + 4, next_to);

        // Ensure all characters were converted (there was only one)
        if (next_from != &from + 1) {
            _evaluator.context().warn(position, "invalid unicode code point.");
            return false;
        }

        // Output the number of bytes converted
        for (size_t i = 0; (&buffer[0] + i) < next_to; ++i) {
            result += buffer[i];
        }
        return true;
    }
}}  // namespace puppet::runtime