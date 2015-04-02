#include <puppet/runtime/string_interpolator.hpp>
#include <puppet/parser/parser.hpp>
#include <boost/format.hpp>
#include <codecvt>
#include <cctype>
#include <sstream>

using namespace std;
using namespace puppet::lexer;
using namespace puppet::parser;

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
        lexer_string_iterator begin,
        lexer_string_iterator const& end,
        function<token_position(token_position const&)> const& calculate_position)
    {
        bool bracket = begin != end && *begin == '{';
        string_static_lexer lexer;

        // Check for keyword or name
        value const* val = nullptr;
        try {
            auto token_begin = lexer.begin(begin, end);
            auto token_end = lexer.end();

            // Check for the following forms:
            // {keyword}, {name}, {decimal}
            // name, decimal

            // Skip past the opening bracket
            if (bracket && token_begin != token_end && token_begin->id() == '{') {
                ++token_begin;
            }

            if (token_begin != token_end && (is_keyword(static_cast<token_id>(token_begin->id())) || token_begin->id() == static_cast<size_t>(token_id::name))) {
                auto token = get<boost::iterator_range<lexer_string_iterator>>(&token_begin->value());
                if (!token) {
                    return false;
                }
                val = evaluator.context().lookup(string(token->begin(), token->end()));
                ++token_begin;
            } else if (token_begin != token_end && token_begin->id() == static_cast<size_t>(token_id::number)) {
                auto token = get<number_token>(&token_begin->value());
                if (!token) {
                    return false;
                }
                if (token->base() != numeric_base::decimal || token->value().which() != 0) {
                    throw evaluation_exception(calculate_position(token->position()), (boost::format("'%1%' is not a valid match variable name.") % *token).str());
                }
                val = evaluator.context().current().get(get<int64_t>(token->value()));
                ++token_begin;
            } else {
                return false;
            }

            // Check for not parsed or missing a closing } token
            if (bracket && (token_begin == token_end || token_begin->id() != '}')) {
                return false;
            }
        } catch (lexer_exception<lexer_string_iterator> const& ex) {
            throw evaluation_exception(calculate_position(ex.location().position()), ex.what());
        }

        // Output the variable
        if (val) {
            ostringstream ss;
            ss << *val;
            result += ss.str();
        }
        return true;
    }

    struct interpolation_expression_visitor : boost::static_visitor<void>
    {
        result_type operator()(ast::access_expression& expr) const
        {
            auto basic = boost::get<ast::basic_expression>(&expr.target());
            if (!basic) {
                return;
            }
            auto name = boost::get<ast::name>(basic);
            if (!name) {
                return;
            }

            // For access expressions with name targets, convert to a variable target
            *basic = ast::variable(std::move(name->value()), name->position());
        }

        result_type operator()(ast::control_flow_expression& expr) const
        {
            auto call = boost::get<ast::method_call_expression>(&expr);
            if (!call) {
                return;
            }
            auto basic = boost::get<ast::basic_expression>(&call->target());
            if (!basic) {
                return;
            }
            auto name = boost::get<ast::name>(basic);
            if (!name) {
                return;
            }

            // For method calls with name targets, convert to a variable target
            *basic = ast::variable(name->value(), name->position());
        }

        template <typename T>
        result_type operator()(T& t) const
        {
        }
    };

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
                        // Success, update the iterator
                        if (*next == '{') {
                            // Move to closing }
                            for (begin = next; begin != end && *begin != '}'; ++begin);
                        } else {
                            // Move to first whitespace character
                            for (begin = next; begin != end && !isspace(*begin); ++begin);
                        }
                        if (begin != end) {
                            ++begin;
                        }
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
                                        boost::apply_visitor(interpolation_expression_visitor(), expression.first());
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