#include <puppet/compiler/evaluation/interpolator.hpp>
#include <puppet/compiler/evaluation/context.hpp>
#include <puppet/compiler/evaluation/evaluator.hpp>
#include <puppet/compiler/parser/parser.hpp>
#include <puppet/compiler/lexer/static_lexer.hpp>
#include <puppet/compiler/exceptions.hpp>
#include <puppet/runtime/values/value.hpp>
#include <puppet/cast.hpp>
#include <boost/format.hpp>
#include <utf8.h>
#include <cctype>
#include <sstream>

using namespace std;
using namespace puppet::compiler;
using namespace puppet::compiler::lexer;
using namespace puppet::runtime;
namespace x3 = boost::spirit::x3;

namespace puppet { namespace compiler { namespace evaluation {

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
        evaluation::context& context,
        ast::context const& expression_context,
        lexer_string_iterator& begin,
        lexer_string_iterator const& end,
        function<lexer::position(lexer::position const&)> const& calculate_position)
    {
        try {
            bool bracket = begin != end && *begin == '{';
            string_static_lexer lexer;

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

            shared_ptr<values::value const> value;
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
                // Lookup as a variable expression
                ast::variable expression{ expression_context, string(token->begin(), token->end()) };
                value = context.lookup(expression);
            } else if (token_begin != token_end && token_begin->id() == static_cast<size_t>(token_id::number)) {
                auto token = get<number_token>(&token_begin->value());
                if (!token) {
                    return false;
                }
                if (token->base() != numeric_base::decimal || token->value().which() != 0) {
                    throw evaluation_exception((boost::format("'%1%' is not a valid match variable name.") % *token).str(), expression_context);
                }
                value = context.lookup(get<int64_t>(token->value()));
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
            if (value) {
                ostringstream ss;
                ss << *value;
                result += ss.str();
            }

            // Update to where we stopped lexing
            begin = current;
            return true;
        } catch (lexer_exception<lexer_string_iterator> const& ex) {
            // Offset the context again to point at where lexing failed
            auto expression_context_offset = expression_context;
            expression_context_offset.position = calculate_position(ex.location().position());
            throw evaluation_exception(ex.what(), expression_context_offset);
        }
    }

    static values::value transform(evaluation::evaluator& evaluator, ast::expression const& expression)
    {
        // Check for a postfix expression that is not part of a binary expression
        if (expression.postfix.subexpressions.empty()) {
            return evaluator.evaluate(expression);
        }

        // Check for access or method call subexpressions
        auto& subexpression = expression.postfix.subexpressions.front();
        if (!boost::get<x3::forward_ast<ast::access_expression>>(&subexpression) &&
            !boost::get<x3::forward_ast<ast::method_call_expression>>(&subexpression)) {
            return evaluator.evaluate(expression);
        }

        string const* name = nullptr;

        // Check for name expression
        auto name_expression = boost::get<ast::name>(&expression.postfix.primary);
        if (name_expression) {
            name = &name_expression->value;
        } else {
            // Also check for bare word expression
            auto word_expression = boost::get<ast::bare_word>(&expression.postfix.primary);
            if (word_expression) {
                name = &word_expression->value;
            }
        }
        if (!name) {
            return evaluator.evaluate(expression);
        }

        // Copy the expression and replace the primary expression with a variable expression
        auto transformed = expression;
        transformed.postfix.primary = ast::variable{ expression.postfix.primary.context(), *name };
        return evaluator.evaluate(transformed);
    }

    bool write_unicode_escape_sequence(evaluation::context& context, ast::context const& expression_context, lexer_string_iterator& begin, lexer_string_iterator const& end, string& result)
    {
        // Check for a variable length unicode escape sequence
        bool variable_length = false;
        if (begin != end && *begin == '{') {
            ++begin;
            variable_length = true;
        }

        string digits;
        digits.reserve(6);
        for (; begin != end; ++begin) {
            // Break on '}' for variable length
            if (variable_length && *begin == '}') {
                break;
            }
            // Check for valid hex digit
            if (!isxdigit(*begin)) {
                context.log(logging::level::warning, (boost::format("unicode escape sequence contains non-hexadecimal character '%1%'.") % *begin).str(), &expression_context);
                return false;
            }

            digits.push_back(*begin);

            // Break on 4 digits for fixed length
            if (!variable_length && digits.size() == 4) {
                break;
            }
        }

        if (variable_length) {
            if (begin == end || *begin != '}') {
                context.log(logging::level::warning, "a closing '}' was not found for unicode escape sequence.", &expression_context);
                return false;
            }
            if (digits.empty() || digits.size() > 6) {
                context.log(logging::level::warning, "expected at least 1 and at most 6 hexadecimal digits for unicode escape sequence.", &expression_context);
                return false;
            }
        }

        // Convert the input to a unicode character
        try {
            char32_t character = static_cast<char32_t>(boost::lexical_cast<hex_to<uint32_t>>(digits));
            utf8::utf32to8(&character, &character + 1, back_inserter(result));
        } catch (boost::bad_lexical_cast const&) {
            context.log(logging::level::warning, "invalid unicode escape sequence.", &expression_context);
            return false;
        } catch (utf8::invalid_code_point const&) {
            context.log(logging::level::warning, "invalid unicode escape sequence.", &expression_context);
            return false;
        }
        return true;
    }

    interpolator::interpolator(evaluation::context& context) :
        _context(context)
    {
    }

    std::string interpolator::interpolate(ast::string const& expression)
    {
        // Helper function for calculating a position inside the string being interpolated
        auto calculate_position = [&](position const& other) {
            return position(expression.context.position.offset() + other.offset() + (expression.quote ? 1 : 0), expression.context.position.line() + other.line() - 1);
        };

        lexer_string_iterator begin(expression.value.begin());
        lexer_string_iterator end(expression.value.end());

        string result;
        result.reserve(expression.value.size());

        int current_margin = expression.margin;
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
            if (*begin == '\\' && !expression.escapes.empty()) {
                auto next = begin;
                ++next;
                if (next != end && *next == '\r') {
                    ++next;
                }
                if (next != end && expression.escapes.find(*next) != string::npos) {
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
                            {
                                auto current_context = expression.context;
                                current_context.position = calculate_position(begin.position());
                                success = write_unicode_escape_sequence(_context, current_context, ++next, end, result);
                            }
                            break;

                        case '\n':
                            // Treat as new line, so reset the margin
                            current_margin = expression.margin;
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
                    if (expression.quote != '\'') {
                        auto current_context = expression.context;
                        current_context.position = calculate_position(begin.position());
                        _context.log(logging::level::warning, (boost::format("invalid escape sequence '\\%1%'.") % *next).str(), &current_context);
                    }
                }
            } else if (*begin == '\n') {
                // Reset the margin
                current_margin = expression.margin;
            } else if (expression.interpolated && *begin == '$') {
                auto next = begin;
                ++next;

                if (next != end && !isspace(*next)) {
                    // First attempt to interpolate using the lexer
                    auto current_context = expression.context;
                    current_context.position = calculate_position(begin.position());
                    if (tokenize_interpolation(result, _context, current_context, next, end, calculate_position)) {
                        begin = next;
                        continue;
                    }
                    // Otherwise, check for expression form
                    if (*next == '{') {
                        try {
                            // Parse the rest of the string
                            // The parsing will stop at the first unmatched } token
                            auto tree = parser::interpolate(boost::make_iterator_range<lexer_string_iterator>(next, end), expression.context.tree->module());
                            evaluation::evaluator evaluator { _context };

                            // Evaluate the body and add the result to the string
                            values::value value;
                            bool first = true;
                            for (auto const& statement : tree->statements) {
                                // For the first expression, transform certain constructs to their "variable" forms
                                if (first) {
                                    first = false;
                                    value = transform(evaluator, statement);
                                    continue;
                                }
                                value = evaluator.evaluate(statement);
                            }
                            ostringstream ss;
                            ss << value;
                            result += ss.str();

                            // Move past where parsing stopped (must have been at the closing })
                            begin = lexer_string_iterator(expression.value.begin() + tree->closing_position.offset());
                            begin.position(tree->closing_position);
                            ++begin;
                            continue;
                        } catch (compiler::parse_exception const& ex) {
                            auto calculated_context = expression.context;
                            calculated_context.position = calculate_position(ex.position());
                            throw evaluation_exception(ex.what(), calculated_context);
                        } catch (evaluation_exception const& ex) {
                            auto calculated_context = expression.context;
                            calculated_context.position = calculate_position(ex.context().position);
                            throw evaluation_exception(ex.what(), calculated_context);
                        }
                    }
                }
            }

            result += *begin++;
        }

        // Remove the trailing line break if instructed to do so
        if (expression.remove_break) {
            if (boost::ends_with(expression.value, "\n")) {
                result.pop_back();
            }
            if (boost::ends_with(expression.value, "\r")) {
                result.pop_back();
            }
        }

        // Ensure the string is valid UTF8
        auto invalid = utf8::find_invalid(result.begin(), result.end());
        if (invalid != result.end()) {
            throw evaluation_exception("invalid UTF8 sequence in string.", expression.context);
        }
        return result;
    }

}}}  // namespace puppet::compiler::evaluation
