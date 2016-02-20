/**
 * @file
 * Declares the Puppet language lexer.
 * The lexer is responsible for turning an input source into a stream of tokens.
 * The tokens are consumed by the Puppet language grammar.
 */
#pragma once

#include "tokens.hpp"
#include "../exceptions.hpp"
#include "../../logging/logger.hpp"
#include "../../cast.hpp"
#include <limits>
#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/spirit/include/lex_lexer.hpp>
#include <boost/spirit/include/lex_lexertl.hpp>
#include <boost/spirit/include/lex_lexertl_token.hpp>
#include <boost/spirit/include/lex_static_lexertl.hpp>
#include <boost/spirit/include/support_multi_pass.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/utility/string_ref.hpp>
#include <boost/format.hpp>
#include <boost/optional.hpp>
#include <utf8.h>
#include <string>
#include <stack>
#include <memory>
#include <locale>
#include <iostream>
#include <fstream>
#include <iterator>
#include <sstream>
#include <exception>
#include <tuple>
#include <regex>
#include <functional>
#include <array>

namespace puppet { namespace compiler { namespace lexer {
    /**
     * The lexer tab width.
     * This is how many spaces the lexer considers a tab to be.
     */
    constexpr const int LEXER_TAB_WIDTH = 4;

    /**
     * The lexer state for EPP lexing.
     */
    extern const char* const EPP_STATE;

    /**
     * The escapes for single quoted strings.
     */
    extern const char* const SQ_ESCAPES;

    /**
     * The escapes for double quoted strings.
     */
    extern const char* const DQ_ESCAPES;

    /**
     * Lexer iterator type used to support interpolated strings and heredoc parsing.
     * Heredocs require a more complicated iterator type due to the fact heredoc lines are parsed out-of-order.
     * This iterator supports skipping over lines that have already been parsed for a heredoc token.
     * This iterator also keeps track of position in the input.
     * @tparam Iterator The underlying iterator type.
     */
    template <class Iterator>
    struct lexer_iterator : boost::iterator_adaptor<lexer_iterator<Iterator>, Iterator>
    {
        /**
         * Default constructor for lexer_iterator.
         */
        lexer_iterator() :
            _position { 0, 0 }
        {
        }

        /**
         * Type conversion from the underlying iterator type.
         * @param iter The underlying iterator instance.
         * @param position The position for the iterator.
         */
        explicit lexer_iterator(Iterator const& iter, lexer::position position = { 0, 1}) :
            boost::iterator_adaptor<lexer_iterator<Iterator>, Iterator>(iter),
            _position(rvalue_cast(position))
        {
        }

        /**
         * Gets the position of the iterator.
         * @return Returns the position of the iterator.
         */
        lexer::position const& position() const
        {
            return _position;
        }

        /**
         * Sets the position of the iterator.
         * @param position The new position of the iterator.
         */
        void position(lexer::position position)
        {
            _position = rvalue_cast(position);
        }

        /**
         * Gets whether an EPP end tag was encountered.
         * @return Returns true if the iterator encountered an EPP end tag or false if the EPP end tag has not yet been encountered.
         */
        bool epp_end() const
        {
            return _epp_end;
        }

     private:
        friend class boost::iterator_core_access;
        template <typename Base> friend struct lexer;

        boost::optional<lexer_iterator<Iterator>> next() const
        {
            boost::optional<lexer_iterator<Iterator>> next;
            if (_next) {
                next = lexer_iterator<Iterator>{ _next->first, _next->second };
            }
            return next;
        }

        void next(lexer_iterator<Iterator> const& next)
        {
            _next = std::make_pair(next.base(), next.position());
        }

        void increment()
        {
            auto& base = this->base_reference();
            auto current = *base;

            // If there is a next iterator and we encounter a newline, move to that iterator
            // This will effectively skip over any heredoc lines that were already lexed
            if (_next && current == '\n') {
                base = _next->first;
                _position = _next->second;
                _next = boost::none;
                return;
            }

            // Otherwise, check for a new line and increment the line counter
            _position.increment(current == '\n');
            ++base;
        }

        lexer::position _position;
        boost::optional<std::pair<Iterator, lexer::position>> _next;
        bool _ignore_epp_end = true;
        bool _epp_end = true;
        bool _heredoc_end_tag = true;
    };

    /**
     * Implements the lexer for the Puppet language.
     * The lexer is responsible for producing a stream of tokens for parsing.
     * @tparam Base The base lexer type.
     */
    template <typename Base>
    struct lexer : boost::spirit::lex::lexer<Base>
    {
        /**
         * The base type for this lexer type.
         */
        using base_type = typename boost::spirit::lex::lexer<Base>;

        /**
         * The type of token this lexer produces.
         */
        using token_type = typename Base::token_type;

        /**
         * The token id type.
         */
        using id_type = typename Base::id_type;

        /**
         * The type of iterator for the output token stream.
         */
        using iterator_type = typename Base::iterator_type;

        /**
         * The type of character for the input stream.
         */
        using char_type = typename Base::char_type;

        /**
         * The type of string for the input stream.
         */
        using string_type = std::basic_string<char_type>;

        /**
         * The input stream iterator type.
         */
        using input_iterator_type = typename token_type::iterator_type;

        /**
         * Constructs a new lexer.
         * @param log The callback to use to log messages.
         */
        explicit lexer(std::function<void(logging::level, std::string const&, position const&, size_t)> log = nullptr) :
            _log(rvalue_cast(log))
        {
            namespace lex = boost::spirit::lex;

            // EPP lexing support
            // When in the EPP state, a match of an opening tag will transition to the normal lexer state.
            // When in the normal state, a match of a closing tag will transition to the EPP state.
            this->self(EPP_STATE) =
                lex::token_def<>(R"(<%#[^%]*%+([^%>]*%+)*>)",    static_cast<id_type>(token_id::comment))               [ epp_comment ]     |
                lex::token_def<>(R"(<%#)",                       static_cast<id_type>(token_id::unclosed_comment))                          |
                lex::token_def<>(R"(<%=)",                       static_cast<id_type>(token_id::epp_render_expression)) [ epp_render ]      |
                lex::token_def<>(R"([\t ]*<%-)",                 static_cast<id_type>(token_id::epp_start_trim))        [ epp_start ]       |
                lex::token_def<>(R"(<%%)",                       static_cast<id_type>(token_id::epp_render_string))     [ epp_escape ]      |
                lex::token_def<>(R"(<%)",                        static_cast<id_type>(token_id::epp_start))             [ epp_start ]       |
                lex::token_def<>(R"([^<]*)",                     static_cast<id_type>(token_id::epp_render_string))     [ epp_string_trim ] |
                lex::token_def<>(R"(.)",                         static_cast<id_type>(token_id::epp_render_string));
            this->self(base_type::initial_state(), EPP_STATE) =
                lex::token_def<>(R"(%>)",                        static_cast<id_type>(token_id::epp_end))               [ epp_end ] |
                lex::token_def<>(R"(-%>)",                       static_cast<id_type>(token_id::epp_end_trim))          [ epp_end ];

            // String interpolation support
            this->self(DQ_STRING_STATE) =
                lex::token_def<>(R"(\$\{)",                           static_cast<id_type>(token_id::interpolation_start)) [ action(&lexer::interpolation_start) ] |
                lex::token_def<>(VALID_VARIABLE_PATTERN,              static_cast<id_type>(token_id::variable))                                                    |
                lex::token_def<>(R"(\")",                             static_cast<id_type>(token_id::string_end))          [ action(&lexer::string_end) ]          |
                lex::token_def<>(R"(.)",                              static_cast<id_type>(token_id::string_text))         [ action(&lexer::string_text) ];
            this->self(VARIABLE_CHECK_STATE) =
                lex::token_def<>(VALID_VARIABLE_WITHOUT_SIGN_PATTERN, static_cast<id_type>(token_id::variable))            [ variable_check ] |
                lex::token_def<>(R"(.)",                              static_cast<id_type>(token_id::string_end))          [ end_variable_check ];
            this->self(HEREDOC_STATE) =
                lex::token_def<>(R"(\$\{)",                           static_cast<id_type>(token_id::interpolation_start)) [ action(&lexer::interpolation_start) ] |
                lex::token_def<>(VALID_VARIABLE_PATTERN,              static_cast<id_type>(token_id::variable))                                                    |
                lex::token_def<>(R"(.)",                              static_cast<id_type>(token_id::string_text))         [ action(&lexer::heredoc_text) ];
            this->self(HEREDOC_END_STATE) =
                lex::token_def<>(R"(.)",                              static_cast<id_type>(token_id::string_end))          [ action(&lexer::heredoc_end) ];

            // The following are lexer states that are used to parse regular expressions.
            // This solves the ambiguity between having multiple division operators on a single line (e.g. "1 / 2 / 3")
            // and parsing a regex; without this, "/ 2 /" above would parse as a regex token.
            // For SLASH_CHECK_STATE, we're doing a lookahead to see if the next token should be a division operator.
            // For FORCE_SLASH_STATE, the lookahead succeeded, so force the next token to be "/" and not a regex, and
            // reset state back to the initial state.
            this->self(SLASH_CHECK_STATE).add(R"([\x20\t\f\v]*\/[^\*])", '/');
            this->self(FORCE_SLASH_STATE, base_type::initial_state()) = lex::token_def<>(R"(\s*\/)", '/') [ use_last ];

            // The following are in match order

            // Add whitespace, comments, and regex
            // Multiline comments, regexes, and the division operator share the same opening character ('/')
            this->self +=
                lex::token_def<>(R"(\s+)",                                        static_cast<id_type>(token_id::whitespace))       [ lex::_pass = lex::pass_flags::pass_ignore ] |
                lex::token_def<>(R"((#[^\n]*)|(\/\*[^*]*\*+([^/*][^*]*\*+)*\/))", static_cast<id_type>(token_id::comment))          [ lex::_pass = lex::pass_flags::pass_ignore ] |
                lex::token_def<>(R"(\/([^\\/\n]|\\[^\n])*\/)",                    static_cast<id_type>(token_id::regex))            [ lex_regex ] |
                lex::token_def<>(R"(\/\*)",                                       static_cast<id_type>(token_id::unclosed_comment));

            // Add the three-character operators
            this->self +=
                lex::token_def<>(R"(<<\|)", static_cast<id_type>(token_id::left_double_collect))  |
                lex::token_def<>(R"(\|>>)", static_cast<id_type>(token_id::right_double_collect)) [ no_regex ];

            // Add the two-character operators
            this->self.add
                (R"(\+=)", static_cast<id_type>(token_id::append))
                (R"(-=)",  static_cast<id_type>(token_id::remove))
                (R"(==)",  static_cast<id_type>(token_id::equals))
                (R"(!=)",  static_cast<id_type>(token_id::not_equals))
                (R"(=~)",  static_cast<id_type>(token_id::match))
                (R"(!~)",  static_cast<id_type>(token_id::not_match))
                (R"(>=)",  static_cast<id_type>(token_id::greater_equals))
                (R"(<=)",  static_cast<id_type>(token_id::less_equals))
                (R"(=>)",  static_cast<id_type>(token_id::fat_arrow))
                (R"(\+>)", static_cast<id_type>(token_id::plus_arrow))
                (R"(<<)",  static_cast<id_type>(token_id::left_shift))
                (R"(<\|)", static_cast<id_type>(token_id::left_collect))
                (R"(>>)",  static_cast<id_type>(token_id::right_shift))
                (R"(@@)",  static_cast<id_type>(token_id::atat))
                (R"(->)",  static_cast<id_type>(token_id::in_edge))
                (R"(~>)",  static_cast<id_type>(token_id::in_edge_sub))
                (R"(<-)",  static_cast<id_type>(token_id::out_edge))
                (R"(<~)",  static_cast<id_type>(token_id::out_edge_sub));
            this->self +=
                lex::token_def<>("\\|>", static_cast<id_type>(token_id::right_collect)) [ no_regex ];

            // Add single character tokens
            // The ids for these tokens are the characters themselves (escape for the quotes, which start strings)
            this->self +=
                lex::token_def<>('[')                                            |
                lex::token_def<>(']')  [ no_regex ]                              |
                lex::token_def<>('{')  [ action(&lexer::increment_brace_count) ] |
                lex::token_def<>('}')  [ action(&lexer::decrement_brace_count) ] |
                lex::token_def<>('"')  [ action(&lexer::string_start) ]          |
                lex::token_def<>('\'') [ action(&lexer::lex_sq_string) ]         |
                '('                                                              |
                lex::token_def<>(')')  [ no_regex ]                              |
                '='                                                              |
                '>'                                                              |
                '<'                                                              |
                '+'                                                              |
                '-'                                                              |
                '/'                                                              |
                '*'                                                              |
                '%'                                                              |
                '!'                                                              |
                '.'                                                              |
                '|'                                                              |
                '@'                                                              |
                ':'                                                              |
                ','                                                              |
                ';'                                                              |
                '?'                                                              |
                '$'                                                              |
                '~';

            // Add the keywords
            this->self.add
                ("case",        static_cast<id_type>(token_id::keyword_case))
                ("class",       static_cast<id_type>(token_id::keyword_class))
                ("default",     static_cast<id_type>(token_id::keyword_default))
                ("define",      static_cast<id_type>(token_id::keyword_define))
                ("if",          static_cast<id_type>(token_id::keyword_if))
                ("elsif",       static_cast<id_type>(token_id::keyword_elsif))
                ("else",        static_cast<id_type>(token_id::keyword_else))
                ("inherits",    static_cast<id_type>(token_id::keyword_inherits))
                ("node",        static_cast<id_type>(token_id::keyword_node))
                ("and",         static_cast<id_type>(token_id::keyword_and))
                ("or",          static_cast<id_type>(token_id::keyword_or))
                ("undef",       static_cast<id_type>(token_id::keyword_undef))
                ("in",          static_cast<id_type>(token_id::keyword_in))
                ("unless",      static_cast<id_type>(token_id::keyword_unless))
                ("function",    static_cast<id_type>(token_id::keyword_function))
                ("type",        static_cast<id_type>(token_id::keyword_type))
                ("attr",        static_cast<id_type>(token_id::keyword_attr))
                ("private",     static_cast<id_type>(token_id::keyword_private))
                ("produces",    static_cast<id_type>(token_id::keyword_produces))
                ("consumes",    static_cast<id_type>(token_id::keyword_consumes))
                ("application", static_cast<id_type>(token_id::keyword_application))
                ("site",        static_cast<id_type>(token_id::keyword_site));
            this->self +=
                lex::token_def<>("true",  static_cast<id_type>(token_id::keyword_true)) [ no_regex ] |
                lex::token_def<>("false", static_cast<id_type>(token_id::keyword_false)) [ no_regex ];

            // Add the statement calls
            this->self.add
                ("require", static_cast<id_type>(token_id::statement_call))
                ("realize", static_cast<id_type>(token_id::statement_call))
                ("include", static_cast<id_type>(token_id::statement_call))
                ("contain", static_cast<id_type>(token_id::statement_call))
                ("tag",     static_cast<id_type>(token_id::statement_call))
                ("debug",   static_cast<id_type>(token_id::statement_call))
                ("info",    static_cast<id_type>(token_id::statement_call))
                ("notice",  static_cast<id_type>(token_id::statement_call))
                ("warning", static_cast<id_type>(token_id::statement_call))
                ("err",     static_cast<id_type>(token_id::statement_call))
                ("fail",    static_cast<id_type>(token_id::statement_call))
                ("import",  static_cast<id_type>(token_id::statement_call));

            // Variables, types, names, bare words
            this->self +=
                lex::token_def<>(VALID_VARIABLE_PATTERN,   static_cast<id_type>(token_id::variable))                                    |
                lex::token_def<>(GENERAL_VARIABLE_PATTERN, static_cast<id_type>(token_id::variable))    [ invalid_variable ]            |
                lex::token_def<>(R"(\s+\[)",               static_cast<id_type>(token_id::array_start)) [ use_last ]                    |
                lex::token_def<>(R"(((::)?[A-Z][\w]*)+)",  static_cast<id_type>(token_id::type))        [ no_regex ]                    |
                lex::token_def<>(NAME_PATTERN,             static_cast<id_type>(token_id::name))        [ no_regex ]                    |
                lex::token_def<>(BARE_WORD_PATTERN,        static_cast<id_type>(token_id::bare_word))   [ no_regex ]                    |
                lex::token_def<>(HEREDOC_PATTERN,          static_cast<id_type>(token_id::string))      [ action(&lexer::lex_heredoc) ] |
                lex::token_def<>(NUMBER_PATTERN,           static_cast<id_type>(token_id::number))      [ lex_number ];

            // Lastly, catch unclosed quotes, invalid variables, and unknown tokens
            this->self.add
                (R"(.)",    static_cast<id_type>(token_id::unknown));
        }

    private:
        using context_type = typename iterator_type::shared_functor_type;

        void lex_heredoc(input_iterator_type const& begin, input_iterator_type& end, boost::spirit::lex::pass_flags& matched, id_type& id, context_type& context)
        {
            using namespace std;

            static const regex pattern(HEREDOC_PATTERN);

            // regex needs bi-directional iterators, so we need to copy the token range (just the @(...) part)
            string_type token{ begin, end };

            // Extract the tag, format, and escapes from the token
            match_results<typename string_type::const_iterator> match;
            if (!regex_match(token, match, pattern) || match.size() != 6) {
                throw lexer_exception<input_iterator_type>("internal error: unexpected heredoc format.", begin, end);
            }

            // Trim the tag
            string_type tag{ match[1].first, match[1].second };
            boost::trim(tag);

            // Check for interpolation
            bool interpolated = false;
            if (boost::starts_with(tag, "\"") && boost::ends_with(tag, "\"")) {
                interpolated = true;
                boost::trim_if(tag, boost::is_any_of("\""));
            }

            // Check for optional format
            string_type format;
            if (match[2].first != match[2].second) {
                format.assign(match[3].first, match[3].second);
            }

            // Check for optional escapes
            string_type escapes;
            if (match[4].first != match[4].second) {
                static char const HEREDOC_ESCAPES[] = R"(trnsuL$)";

                escapes.assign(match[5].first, match[5].second);
                if (escapes.empty()) {
                    // Enable all heredoc escapes
                    escapes = HEREDOC_ESCAPES;
                } else {
                    // Verify the escapes
                    static_assert(sizeof(HEREDOC_ESCAPES) > 1, "expected at least one escape in the list.");
                    bool seen[sizeof(HEREDOC_ESCAPES) - 1] = {};
                    for (char c : escapes) {
                        char const* escape = HEREDOC_ESCAPES;
                        for (; *escape && *escape != c; ++escape);

                        size_t index = static_cast<size_t>(escape - HEREDOC_ESCAPES);
                        if (*escape == 0 || index >= sizeof(seen)) {
                            throw lexer_exception<input_iterator_type>((boost::format("invalid heredoc escape '%1%': only t, r, n, s, u, L, and $ are allowed.") % c).str(), begin, end);
                        }
                        // Can only be seen once, otherwise it is an error
                        if (seen[index]) {
                            throw lexer_exception<input_iterator_type>((boost::format("heredoc escape '%1%' may only appear once in the list.") % c).str(), begin, end);
                        }
                        seen[index] = true;
                    }
                }

                // Treat L as "escaping newlines"
                boost::replace_all(escapes, "L", "\n");

                // Escaping automatically adds '\' to the list
                escapes += "\\";
            }

            auto& eoi = context.get_eoi();

            // Move to the next line to process, skipping over any previous heredoc on the token's line
            auto value_begin = end.next();
            if (!value_begin) {
                auto next_line = end;
                for (; next_line != eoi && *next_line != '\n'; ++next_line);

                // Move past the newline
                if (next_line == eoi) {
                    throw lexer_exception<input_iterator_type>((boost::format("could not find a matching heredoc end tag '%1%'.") % tag).str(), begin, end);
                }
                ++next_line;
                value_begin = next_line;
            }

            // If interpolated, emit a start string token and change lexer states
            if (interpolated) {
                string_lex_info info;
                info.string_start = std::make_pair(begin, end);
                info.escapes = rvalue_cast(escapes);
                info.tag = rvalue_cast(tag);
                _strings.emplace(rvalue_cast(info));

                context.set_state(context.get_state_id(HEREDOC_STATE));

                id = static_cast<id_type>(token_id::string_start);
                context.set_value(string_start_token{ begin.position(), end.position(), rvalue_cast(format) });

                end = rvalue_cast(*value_begin);
                return;
            }

            size_t margin = 0;
            std::string text;
            std::string line;

            auto value_end = *value_begin;
            for (auto current = value_end; current != eoi; value_end = current) {
                read_heredoc_line(line, current, eoi, escapes);

                // Check for end tag
                bool trim = false;
                if (is_heredoc_end_tag(line, tag, margin, trim)) {
                    if (trim) {
                        if (!text.empty()) {
                            if (text.back() == '\n') {
                                text.pop_back();
                            }
                            if (text.back() == '\r') {
                                text.pop_back();
                            }
                        }
                    }
                    context.set_value(string_token{ value_begin->position(), value_end.position(), rvalue_cast(text), rvalue_cast(format), margin });
                    if (current != eoi) {
                        ++current;
                    }
                    end.next(current);
                    break;
                }

                // Append the line
                text += line;
                if (current != eoi) {
                    ++current;
                }
            }

            if (value_end == eoi) {
                throw lexer_exception<input_iterator_type>((boost::format("could not find a matching heredoc end tag '%1%'.") % tag).str(), begin, end);
            }
        }

        void lex_sq_string(input_iterator_type const& begin, input_iterator_type& end, boost::spirit::lex::pass_flags const& matched, id_type& id, context_type& context)
        {
            static const std::string escapes = SQ_ESCAPES;

            auto const& eoi = context.get_eoi();

            auto string_start_end = end;

            // Look for the closing quote, copying any characters into a value, unescaping along the way
            // Start from the current end because it is past the opening ' character
            std::string value;
            bool found_close = false;
            for (; !found_close && end != eoi; ++end) {
                if (*end == '\'') {
                    found_close = true;
                    // Continue to move past the closing ' for the exclusive end of the string
                    continue;
                }

                if (unescape(value, end, eoi, escapes, false)) {
                    continue;
                }

                value += *end;
            }

            // Ensure a closing quote was found
            if (!found_close) {
                throw lexer_exception<input_iterator_type>("could not find a matching closing quote.", begin, string_start_end);
            }

            id = static_cast<id_type>(token_id::string);
            context.set_value(string_token{ begin.position(), end.position(), rvalue_cast(value) });

            // Force any following '/' to be interpreted as a '/' token
            context.set_end(end);
            force_slash(context);
        }

        bool unescape(std::string& output, input_iterator_type& begin, input_iterator_type const& end, std::string const& escapes, bool warn_invalid_escape = true)
        {
            if (begin == end) {
                return false;
            }

            if (*begin != '\\') {
                return false;
            }

            auto next = begin;
            ++next;
            if (next != end && *next == '\r') {
                ++next;
            }
            if (next == end) {
                return false;
            }
            if (escapes.find(*next) == std::string::npos) {
                if (warn_invalid_escape && _log) {
                    _log(logging::level::warning, (boost::format("invalid escape sequence '\\%1%'.") % *next).str(), begin.position(), 2u);
                }
                return false;
            }

            switch (*next) {
                case 'r':
                    output += '\r';
                    break;

                case 'n':
                    output += '\n';
                    break;

                case 't':
                    output += '\t';
                    break;

                case 's':
                    output += ' ';
                    break;

                case 'u':
                    ++next;
                    if (!write_unicode_escape_sequence(output, next, end)) {
                        return false;
                    }
                    break;

                case '\n':
                    // Eat the new line
                    break;

                default:
                    output += *next;
                    break;
            }

            begin = next;
            return true;
        }

        template <typename Value>
        struct hex_to
        {
            operator Value() const
            {
                return value;
            }

            friend std::istream& operator>>(std::istream& in, hex_to& out)
            {
                in >> std::hex >> out.value;
                return in;
            }

         private:
            Value value;
        };

        bool write_unicode_escape_sequence(std::string& output, input_iterator_type& begin, input_iterator_type const& end)
        {
            // Check for a variable length unicode escape sequence
            bool variable_length = false;

            auto start_position = begin.position();
            if (begin != end && *begin == '{') {
                ++begin;
                variable_length = true;
            }

            std::string digits;
            digits.reserve(6);
            for (; begin != end; ++begin) {
                // Break on '}' for variable length
                if (variable_length && *begin == '}') {
                    break;
                }
                // Check for valid hex digit
                if (!isxdigit(*begin)) {
                    if (_log) {
                        if (variable_length) {
                            _log(logging::level::warning, "a closing '}' was not found before encountering a non-hexadecimal character in unicode escape sequence.", start_position, 1);
                        } else {
                            _log(logging::level::warning, (boost::format("unicode escape sequence contains non-hexadecimal character '%1%'.") % *begin).str(), begin.position(), 1);
                        }
                    }
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
                    if (_log) {
                        _log(logging::level::warning, "a closing '}' was not found for unicode escape sequence.", start_position, 1);
                    }
                    return false;
                }
                if (digits.empty() || digits.size() > 6) {
                    if (_log) {
                        _log(logging::level::warning, "expected at least 1 and at most 6 hexadecimal digits for unicode escape sequence.", start_position, begin.position().offset() - start_position.offset() + 1);
                    }
                    return false;
                }
            }

            // Convert the input to a unicode character
            try {
                char32_t character = static_cast<char32_t>(boost::lexical_cast<hex_to<uint32_t>>(digits));
                utf8::utf32to8(&character, &character + 1, std::back_inserter(output));
                return true;
            } catch (boost::bad_lexical_cast const&) {
            } catch (utf8::invalid_code_point const&) {
            }
            if (_log) {
                start_position.increment(false);
                _log(logging::level::warning, (boost::format("'%1%' is not a valid unicode codepoint.") % digits).str(), start_position, begin.position().offset() - start_position.offset());
            }
            return false;
        }

        void increment_brace_count(input_iterator_type const& begin, input_iterator_type& end, boost::spirit::lex::pass_flags const& matched, id_type const& id, context_type const& context)
        {
            auto info = current_string_info();
            if (info) {
                ++info->brace_count;
            }
        }

        void decrement_brace_count(input_iterator_type const& begin, input_iterator_type& end, boost::spirit::lex::pass_flags const& matched, id_type& id, context_type& context)
        {
            auto info = current_string_info();
            if (info && info->brace_count && (--info->brace_count == 0)) {
                id = static_cast<id_type>(token_id::interpolation_end);
                context.set_state(context.get_state_id(!info->tag.empty() ? HEREDOC_STATE : DQ_STRING_STATE));
            }
        }

        void string_start(input_iterator_type const& begin, input_iterator_type& end, boost::spirit::lex::pass_flags const& matched, id_type& id, context_type& context)
        {
            // Start a new string
            string_lex_info info;
            info.string_start = std::make_pair(begin, end);
            _strings.emplace(rvalue_cast(info));

            context.set_state(context.get_state_id(DQ_STRING_STATE));

            // Emit a string start token with empty (normal string) format
            id = static_cast<id_type>(token_id::string_start);
            context.set_value(string_start_token{ begin.position(), end.position() });
        }

        void interpolation_start(input_iterator_type const& begin, input_iterator_type& end, boost::spirit::lex::pass_flags const& matched, id_type const& id, context_type& context)
        {
            auto info = current_string_info();
            if (!info) {
                throw lexer_exception<input_iterator_type>("internal error: unexpected interpolation start when not lexing a string.", begin, end);
            }

            // Treat the opening ${ as a brace
            ++info->brace_count;

            // For heredocs, don't check the end tag on this line
            info->check_end_tag = false;

            // Transition to the variable check state
            context.set_state(context.get_state_id(VARIABLE_CHECK_STATE));
        }

        void string_text(input_iterator_type const& begin, input_iterator_type& end, boost::spirit::lex::pass_flags& matched, id_type const& id, context_type& context)
        {
            static const std::string escapes = DQ_ESCAPES;

            auto info = current_string_info();
            if (!info || !info->tag.empty()) {
                throw lexer_exception<input_iterator_type>("internal error: unexpected string text when not lexing a string.", begin, end);
            }

            auto const& eoi = context.get_eoi();

            // Eat everything up to $ or ", unescaping any escape sequences
            std::string text;
            auto current = begin;

            // If starting with $, it's not an interpolation sequence because it failed to match the interpolation rules
            if (current != eoi && *current == '$') {
                text += '$';
                ++current;
            }

            for (; current != eoi; ++current) {
                if (*current == '$' || *current == '"') {
                    break;
                }

                if (unescape(text, current, eoi, escapes)) {
                    continue;
                }

                text += *current;
            }

            if (current == eoi) {
                throw lexer_exception<input_iterator_type>("could not find a matching closing quote.", info->string_start.first, info->string_start.second);
            }

            end = current;

            // Ignore empty text spans
            if (text.empty()) {
                matched = boost::spirit::lex::pass_flags::pass_ignore;
                return;
            }

            context.set_value(string_text_token{ begin.position(), end.position(), rvalue_cast(text) });
        }

        static void string_variable(input_iterator_type const& begin, input_iterator_type const& end, boost::spirit::lex::pass_flags const& matched, id_type const& id, context_type const& context)
        {
            validate_variable(begin, end);
        }

        void string_end(input_iterator_type const& begin, input_iterator_type const& end, boost::spirit::lex::pass_flags const& matched, id_type const& id, context_type& context)
        {
            auto info = current_string_info();
            if (!info || !info->tag.empty()) {
                throw lexer_exception<input_iterator_type>("internal error: unexpected string end when not lexing a string.", begin, end);
            }

            if (info->brace_count != 0) {
                throw lexer_exception<input_iterator_type>("internal error: mismatched interpolation brace count before end of string.", begin, end);
            }

            // Force any following '/' to be interpreted as a '/' token
            force_slash(context);

            _strings.pop();

            context.set_value(string_end_token{ begin.position(), end.position() });

            // Transition to the initial state to lex the interpolation expression
            context.set_state(context.get_state_id("INITIAL"));
        }

        static void variable_check(input_iterator_type const& begin, input_iterator_type& end, boost::spirit::lex::pass_flags& matched, id_type& id, context_type& context)
        {
            // If the next token is not }, '.', or '[', then this token cannot be a variable
            auto state = context.get_state_id("INITIAL");
            if (!context.lookahead('}', state) && !context.lookahead('.', state) && !context.lookahead('[', state)) {
                end_variable_check(begin, end, matched, id, context);
                return;
            }
        }

        static void end_variable_check(input_iterator_type const& begin, input_iterator_type& end, boost::spirit::lex::pass_flags& matched, id_type const& id, context_type& context)
        {
            // Move back to the start to retry the lexing
            end = begin;
            matched = boost::spirit::lex::pass_flags::pass_ignore;

            // Transition to the initial state to lex the interpolation expression
            context.set_state(context.get_state_id("INITIAL"));
        }

        void heredoc_text(input_iterator_type const& begin, input_iterator_type& end, boost::spirit::lex::pass_flags& matched, id_type const& id, context_type& context)
        {
            auto info = current_string_info();
            if (!info || info->tag.empty()) {
                throw lexer_exception<input_iterator_type>("internal error: unexpected heredoc text when not lexing a heredoc.", begin, end);
            }

            auto const& eoi = context.get_eoi();

            bool found_end_tag = false;
            std::string text;
            std::string line;

            // Read as many lines of the heredoc as possible
            end = begin;
            for (auto current = end; current != eoi; end = current) {
                read_heredoc_line(line, current, eoi, info->escapes, true);
                if (current != eoi && current != begin && *current == '$') {
                    // Encountered a possible interpolation
                    info->check_end_tag = false;
                    text += line;
                    end = current;
                    break;
                }

                // Check for end tag
                bool trim = false;
                if (info->check_end_tag && is_heredoc_end_tag(line, info->tag, info->margin, trim)) {
                    if (trim) {
                        if (!text.empty()) {
                            if (text.back() == '\n') {
                                text.pop_back();
                            }
                            if (text.back() == '\r') {
                                text.pop_back();
                            }
                        }
                    }
                    found_end_tag = true;
                    break;
                }

                info->check_end_tag = true;

                // Append the line
                text += line;
                if (current != eoi) {
                    ++current;
                }
            }

            if (end == eoi && !found_end_tag) {
                throw lexer_exception<input_iterator_type>(
                    (boost::format("could not find a matching heredoc end tag '%1%'.") % info->tag).str(),
                    info->string_start.first,
                    info->string_start.second
                );
            }

            // Ignore empty text spans
            if (text.empty()) {
                matched = boost::spirit::lex::pass_flags::pass_ignore;
            }  else {
                context.set_value(string_text_token{ begin.position(), end.position(), rvalue_cast(text) });
            }

            if (found_end_tag) {
                context.set_state(context.get_state_id(HEREDOC_END_STATE));
            }
        }

        void heredoc_variable(input_iterator_type const& begin, input_iterator_type const& end, boost::spirit::lex::pass_flags const& matched, id_type const& id, context_type const& context)
        {
            auto info = current_string_info();
            if (!info || info->tag.empty()) {
                throw lexer_exception<input_iterator_type>("internal error: unexpected heredoc variable when not lexing a heredoc.", begin, end);
            }

            validate_variable(begin, end);

            // Don't check for an end tag after a variable
            info->check_end_tag = false;
        }

        void heredoc_end(input_iterator_type const& begin, input_iterator_type& end, boost::spirit::lex::pass_flags const& matched, id_type const& id, context_type& context)
        {
            auto info = current_string_info();
            if (!info || info->tag.empty()) {
                throw lexer_exception<input_iterator_type>("internal error: unexpected heredoc end when not lexing a heredoc.", begin, end);
            }

            if (info->brace_count != 0) {
                throw lexer_exception<input_iterator_type>("internal error: mismatched interpolation brace count before end of heredoc.", begin, end);
            }

            auto const& eoi = context.get_eoi();

            // Eat the rest of the line
            for (; end != eoi; ++end) {
                if (*end == '\n') {
                    ++end;
                    break;
                }
            }

            // Set up where the iterator should jump to
            info->string_start.second.next(end);
            end = rvalue_cast(info->string_start.second);

            // Force any following '/' to be interpreted as a '/' token
            context.set_end(end);
            force_slash(context);

            context.set_value(string_end_token{ begin.position(), end.position(), info->margin });

            _strings.pop();

            // Transition to the initial state to lex the interpolation expression
            context.set_state(context.get_state_id("INITIAL"));
        }

        void read_heredoc_line(std::string& line, input_iterator_type& begin, input_iterator_type const& end, std::string const& escapes, bool interpolated = false)
        {
            line.clear();
            for (; begin != end; ++begin) {
                if (interpolated && *begin == '$') {
                    break;
                }

                if (unescape(line, begin, end, escapes, false)) {
                    continue;
                }

                line += *begin;

                if (*begin == '\n') {
                    break;
                }
            }
        }

        static bool is_heredoc_end_tag(std::string const& line, std::string const& tag, size_t& margin, bool& trim)
        {
            static auto is_space = [](char c) { return c == ' ' || c == '\t'; };

            auto size = line.size();
            size_t pos = 0;

            margin = 0;
            trim = false;

            // Determine the margin by counting whitespace (tabs and spaces are treated the same)
            for (; pos < size && is_space(line[pos]); ++pos, ++margin);

            if (pos == size) {
                return false;
            }

            if (line[pos] == '|') {
                for (++pos; pos < size && is_space(line[pos]); ++pos);
                if (pos == size) {
                    return false;
                }
            } else {
                margin = 0;
            }

            if (line[pos] == '-') {
                trim = true;
                for (++pos; pos < size && is_space(line[pos]); ++pos);
            }

            // Ensure there's enough space for the tag and the tags are equal
            if ((pos + tag.size()) > size) {
                return false;
            }
            boost::string_ref candidate{ &line[pos], tag.size() };
            if (candidate != tag) {
                return false;
            }

            // Ensure the rest of the string is whitespace followed by an optional newline
            for (pos += tag.size(); pos < size && is_space(line[pos]); ++pos);
            if (pos < size && line[pos] == '\r') {
                ++pos;
            }
            if (pos < size && line[pos] == '\n') {
                ++pos;
            }
            // The string must be exhausted to be an end tag
            return pos == size;
        }

        static void lex_number(input_iterator_type const& begin, input_iterator_type const& end, boost::spirit::lex::pass_flags& matched, id_type& id, context_type& context)
        {
            using namespace std;

            static const regex hex_pattern(R"(0[xX][0-9A-Fa-f]+)");
            static const regex octal_pattern(R"(0\d+)");
            static const regex valid_octal_pattern(R"(0[0-7]+)");
            static const regex decimal_pattern(R"(0|([1-9]\d*))");
            static const regex double_pattern(R"([0-9]\d*(\.\d+)?([eE]-?\d+)?)");

            // Force any following '/' to be interpreted as a '/' token
            force_slash(context);

            // regex needs bi-directional iterators, so we need to copy the token range
            string_type token{ begin, end };

            // Match integral numbers
            int base = 0;
            if (regex_match(token, hex_pattern)) {
                base = 16;
            } else if (regex_match(token, octal_pattern)) {
                // Make sure the number is valid for an octal
                if (!regex_match(token, valid_octal_pattern)) {
                    throw lexer_exception<input_iterator_type>((boost::format("'%1%' is not a valid number.") % token).str(), begin, end);
                }
                base = 8;
            } else if (regex_match(token, decimal_pattern)) {
                base = 10;
            }

            if (base != 0) {
                try {
                    context.set_value(
                        number_token{
                            begin.position(),
                            end.position(),
                            static_cast<std::int64_t>(stoll(token, 0, base)),
                            base == 16 ? numeric_base::hexadecimal : (base == 8 ? numeric_base::octal : numeric_base::decimal)
                        }
                    );
                } catch (out_of_range const& ex) {
                    throw lexer_exception<input_iterator_type>(
                        (boost::format("'%1%' is not in the range of %2% to %3%.") %
                            token %
                            numeric_limits<int64_t>::min() %
                            numeric_limits<int64_t>::max()
                        ).str(),
                        begin,
                        end
                    );
                }
                return;
            }

            // Match double
            if (regex_match(token, double_pattern)) {
                try {
                    context.set_value(number_token{ begin.position(), end.position(), stod(token, 0) });
                } catch (out_of_range const& ex) {
                    throw lexer_exception<input_iterator_type>(
                        (boost::format("'%1%' is not in the range of %2% to %3%.") %
                            token %
                            numeric_limits<double>::lowest() %
                            numeric_limits<double>::max()
                        ).str(),
                        begin,
                        end
                    );
                }
                return;
            }

            // Not a valid number
            throw lexer_exception<input_iterator_type>((boost::format("'%1%' is not a valid number.") % token).str(), begin, end);
        }

        static void lex_regex(input_iterator_type const& begin, input_iterator_type const& end, boost::spirit::lex::pass_flags& matched, id_type& id, context_type& context)
        {
            auto next = begin;
            if (next != end) {
                ++next;
                // Regex literals cannot start with /*
                if (next != end && *next == '*') {
                    // Since we failed to match a multiline comment, treat this as a unclosed comment instead
                    id = static_cast<id_type>(token_id::unclosed_comment);
                    context.set_value(boost::make_iterator_range(begin, ++next));
                    return;
                }
            }

            // Force any following '/' to be interpreted as a '/' token
            force_slash(context);
        }

        static void no_regex(input_iterator_type const& begin, input_iterator_type const& end, boost::spirit::lex::pass_flags const& matched, id_type const& id, context_type& context)
        {
            // Force any following '/' to be interpreted as a '/' token
            force_slash(context);
        }

        static void force_slash(context_type& context)
        {
            // If the next token is /, then set the "no regex" state
            // This will force the next '/' to match as '/' and not the start of a regex
            if (!context.lookahead('/', context.get_state_id(SLASH_CHECK_STATE))) {
                return;
            }
            context.set_state(context.get_state_id(FORCE_SLASH_STATE));
        }

        static void use_last(input_iterator_type& begin, input_iterator_type const& end, boost::spirit::lex::pass_flags& matched, id_type& id, context_type& context)
        {
            // Use the last character in the range
            auto last = begin;
            for (auto current = begin; current != end; ++current) {
                last = current;
            }
            begin = last;
        }

        static void epp_comment(input_iterator_type const& begin, input_iterator_type& end, boost::spirit::lex::pass_flags& matched, id_type& id, context_type& context)
        {
            matched = boost::spirit::lex::pass_flags::pass_ignore;

            // Check if the comment ends with a trim specifier
            if (boost::ends_with(boost::make_iterator_range(begin, end), "-%>")) {
                epp_trim_right(begin, end, matched, id, context);
            }
        }

        static void epp_string_trim(input_iterator_type const& begin, input_iterator_type& end, boost::spirit::lex::pass_flags& matched, id_type& id, context_type& context)
        {
            // If rendering a string, ensure the next token is an epp start trim token
            if (!context.lookahead(static_cast<size_t>(token_id::epp_start_trim))) {
                return;
            }

            // Because input iterators are forward only, we have to scan all the way from the beginning for the trim
            auto new_end = begin;
            boost::optional<input_iterator_type> start_space;
            while (new_end != end) {
                bool space = ::isspace(*new_end) && *new_end != '\n';
                if (start_space && !space) {
                    start_space = boost::none;
                } else if (!start_space && space) {
                    start_space = new_end;
                }
                ++new_end;
            }
            if (start_space) {
                end = *start_space;
            }
        }

        static void epp_trim_right(input_iterator_type const& begin, input_iterator_type& end, boost::spirit::lex::pass_flags& matched, id_type& id, context_type& context)
        {
            auto const& eoi = context.get_eoi();
            auto new_end = end;

            // Find the end of line or input provided everything we encounter is whitespace to trim
            for (; new_end != eoi && *new_end != '\n' && ::isspace(*new_end); ++new_end);

            if (new_end == eoi) {
                end = new_end;
            } else if (*new_end == '\n') {
                end = ++new_end;
            }
        }

        static void epp_escape(input_iterator_type const& begin, input_iterator_type& end, boost::spirit::lex::pass_flags& matched, id_type& id, context_type& context)
        {
            auto value_end = begin;
            ++value_end;
            ++value_end;
            context.set_value(boost::make_iterator_range(begin, value_end));
        }

        static void epp_start(input_iterator_type const& begin, input_iterator_type& end, boost::spirit::lex::pass_flags& matched, id_type& id, context_type& context)
        {
            matched = boost::spirit::lex::pass_flags::pass_ignore;
            context.set_state(context.get_state_id("INITIAL"));
            end._epp_end = false;
        }

        static void epp_render(input_iterator_type const& begin, input_iterator_type& end, boost::spirit::lex::pass_flags& matched, id_type& id, context_type& context)
        {
            context.set_state(context.get_state_id("INITIAL"));
            end._ignore_epp_end = false;
        }

        static void epp_end(input_iterator_type const& begin, input_iterator_type& end, boost::spirit::lex::pass_flags& matched, id_type& id, context_type& context)
        {
            if (begin._ignore_epp_end) {
                matched = boost::spirit::lex::pass_flags::pass_ignore;
            }
            end._ignore_epp_end = true;
            end._epp_end = true;

            if (id == static_cast<size_t>(token_id::epp_end_trim)) {
                epp_trim_right(begin, end, matched, id, context);
            }
        }

        static void invalid_variable(input_iterator_type const& begin, input_iterator_type const& end, boost::spirit::lex::pass_flags const& matched, id_type const& id, context_type const& context)
        {
            auto name_begin = begin;
            if (name_begin != end && *name_begin == '$') {
                ++name_begin;
            }

            throw lexer_exception<input_iterator_type>(
                (boost::format("'%1%' is not a valid variable name: the name must conform to /%2%/.") %
                 boost::make_iterator_range(name_begin, end) %
                 VALID_VARIABLE_WITHOUT_SIGN_PATTERN
                ).str(),
                begin,
                end
            );
        }

        struct string_lex_info
        {
            // Iterators for the string start
            std::pair<input_iterator_type, input_iterator_type> string_start;

            // Escapes for string (heredoc only)
            std::string escapes;

            // End tag for string (heredoc only)
            std::string tag;

            // Count of braces encountered during interpolation
            unsigned int brace_count = 0;

            // The string's margin (heredoc only)
            size_t margin = 0;

            // Keeps track of whether or not a heredoc tag should be checked for (heredoc only)
            bool check_end_tag = true;
        };

        string_lex_info* current_string_info()
        {
            return const_cast<string_lex_info*>(static_cast<lexer<Base> const*>(this)->current_string_info());
        }

        string_lex_info const* current_string_info() const
        {
            return !_strings.empty() ? &_strings.top() : nullptr;
        }

        template <typename Func>
        auto action(Func func)
        {
            using namespace std::placeholders;
            return std::bind(func, this, _1, _2, _3, _4, _5);
        }

        std::stack<string_lex_info> _strings;
        std::function<void(logging::level, std::string const&, position const&, size_t)> _log;

        static char const* const GENERAL_VARIABLE_PATTERN;
        static char const* const VALID_VARIABLE_PATTERN;
        static char const* const VALID_VARIABLE_WITHOUT_SIGN_PATTERN;
        static char const* const NAME_PATTERN;
        static char const* const BARE_WORD_PATTERN;
        static char const* const NUMBER_PATTERN;
        static char const* const HEREDOC_PATTERN;
        static char const* const FORCE_SLASH_STATE;
        static char const* const SLASH_CHECK_STATE;
        static char const* const DQ_STRING_STATE;
        static char const* const HEREDOC_STATE;
        static char const* const HEREDOC_END_STATE;
        static char const* const VARIABLE_CHECK_STATE;
    };

    template<typename Base>
    char const* const lexer<Base>::GENERAL_VARIABLE_PATTERN = R"(\$(::)?(\w+::)*\w+)";
    template<typename Base>
    char const* const lexer<Base>::VALID_VARIABLE_PATTERN = R"(\$(0|[1-9]\d*|(::)?[a-z_]\w*(::\w*)*))";
    template<typename Base>
    char const* const lexer<Base>::VALID_VARIABLE_WITHOUT_SIGN_PATTERN = R"(0|[1-9]\d*|(::)?[a-z_]\w*(::\w*)*)";
    template<typename Base>
    char const* const lexer<Base>::NAME_PATTERN = R"(((::)?[a-z][\w]*)(::[a-z][\w]*)*)";
    template<typename Base>
    char const* const lexer<Base>::BARE_WORD_PATTERN = R"([a-z_]([\w\-]*[\w])?)";
    template<typename Base>
    char const* const lexer<Base>::NUMBER_PATTERN = R"(\d\w*(\.\d\w*)?([eE]-?\w*)?)";

    template<typename Base>
    char const* const lexer<Base>::HEREDOC_PATTERN = R"(@\(\s*([^):/\r\n]+)\s*(:\s*([a-z][a-zA-Z0-9_+]+))?\s*(\/\s*([\w|$]*)\s*)?\))";
    template<typename Base>
    char const* const lexer<Base>::FORCE_SLASH_STATE = "FS";
    template<typename Base>
    char const* const lexer<Base>::SLASH_CHECK_STATE = "SC";
    template<typename Base>
    char const* const lexer<Base>::DQ_STRING_STATE = "DQS";
    template<typename Base>
    char const* const lexer<Base>::HEREDOC_STATE = "HD";
    template<typename Base>
    char const* const lexer<Base>::HEREDOC_END_STATE = "HDE";
    template<typename Base>
    char const* const lexer<Base>::VARIABLE_CHECK_STATE = "VC";

    /**
     * The input iterator for files.
     */
    using lexer_istreambuf_iterator = lexer_iterator<boost::spirit::multi_pass<std::istreambuf_iterator<char>>>;
    /**
     * The input iterator for strings.
     */
    using lexer_string_iterator = lexer_iterator<typename std::string::const_iterator>;

    /**
     * The token type for the lexer.
     * @tparam Iterator The input iterator for the token.
     */
    template <typename Iterator>
    using lexer_token = boost::spirit::lex::lexertl::token<Iterator, boost::mpl::vector<string_token, string_start_token, string_text_token, string_end_token, number_token>>;

    /**
     * The lexer to use for files.
     */
    using file_lexer = lexer<boost::spirit::lex::lexertl::actor_lexer<lexer_token<lexer_istreambuf_iterator>>>;
    /**
     * The static lexer to use for files.
     * Include "static_lexer.hpp" before including "lexer.hpp" to use this type.
     */
    using file_static_lexer = lexer<boost::spirit::lex::lexertl::static_actor_lexer<lexer_token<lexer_istreambuf_iterator>>>;
    /**
     * The lexer to use for strings.
     */
    using string_lexer = lexer<boost::spirit::lex::lexertl::actor_lexer<lexer_token<lexer_string_iterator>>>;
    /**
     * The static lexer to use for strings.
     * Include "static_lexer.hpp" before including "lexer.hpp" to use this type.
     */
    using string_static_lexer = lexer<boost::spirit::lex::lexertl::static_actor_lexer<lexer_token<lexer_string_iterator>>>;

    /**
     * Gets the lexer's beginning iterator for the given file.
     * @param file The file to lex.
     * @return Returns the beginning input interator for the lexer.
     */
    lexer_istreambuf_iterator lex_begin(std::ifstream& file);

    /**
     * Gets the lexer's ending iterator for the given file.
     * @param file The file to lex.
     * @return Returns the ending input iterator for the lexer.
     */
    lexer_istreambuf_iterator lex_end(std::ifstream& file);

    /**
     * Gets the lexer's beginning iterator for the given string.
     * @param str The string to lex.
     * @return Returns the beginning input iterator for the lexer.
     */
    lexer_string_iterator lex_begin(std::string const& str);

    /**
     * Gets the lexer's ending iterator for the given string.
     * @param str The string to lex.
     * @return Returns the ending input iterator for the lexer.
     */
    lexer_string_iterator lex_end(std::string const& str);

    /**
     * Gets the lexer's beginning iterator for the given iterator range.
     * @param range The iterator range to parse.
     * @return Returns the beginning input iterator for the lexer.
     */
    lexer_string_iterator lex_begin(boost::iterator_range<lexer_string_iterator> const& range);

    /**
     * Gets the lexer's ending iterator for the given iterator range.
     * @param range The iterator range to parse.
     * @return Returns the ending input iterator for the lexer.
     */
    lexer_string_iterator lex_end(boost::iterator_range<lexer_string_iterator> const& range);

    /**
     * Gets the text and column for the given position in a file.
     * @param fs The file stream of the file.
     * @param position The position inside the file.
     * @param tab_width Specifies the width of a tab character for column calculations.
     * @return Returns a tuple of the line's text and the column of the position.
     */
    std::tuple<std::string, std::size_t> get_text_and_column(std::ifstream& fs, std::size_t position, std::size_t tab_width = LEXER_TAB_WIDTH);

    /**
     * Gets the text and column for the given position in a string.
     * @param input The input string to get the line and position in.
     * @param position The position inside the string.
     * @param tab_width Specifies the width of a tab character for column calculations.
     * @return Returns a tuple of the line's text and the column of the position.
     */
    std::tuple<std::string, std::size_t> get_text_and_column(std::string const& input, std::size_t position, std::size_t tab_width = LEXER_TAB_WIDTH);

    /**
     * Gets the last position for the given file stream.
     * @param input The input file stream.
     * @return Returns the last position in the file stream.
     */
    position get_last_position(std::ifstream& input);

    /**
     * Gets the last position for the given input string.
     * @param input The input string.
     * @return Returns the last position in the input string.
     */
    position get_last_position(std::string const& input);

    /**
     * Gets the last position for the given input string iterator range.
     * @param range The input string iterator range.
     * @return Returns the last position in the iterator range.
     */
    position get_last_position(boost::iterator_range<lexer_string_iterator> const& range);

}}}  // namespace puppet::compiler::lexer
