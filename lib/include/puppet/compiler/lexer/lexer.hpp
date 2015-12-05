/**
 * @file
 * Declares the Puppet language lexer.
 * The lexer is responsible for turning an input source into a stream of tokens.
 * The tokens are consumed by the Puppet language grammar.
 */
#pragma once

#include "token_id.hpp"
#include "string_token.hpp"
#include "number_token.hpp"
#include "../../cast.hpp"
#include <string>
#include <locale>
#include <iostream>
#include <fstream>
#include <iterator>
#include <sstream>
#include <exception>
#include <tuple>
#include <regex>
#include <limits>
#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/spirit/include/lex_lexer.hpp>
#include <boost/spirit/include/lex_lexertl.hpp>
#include <boost/spirit/include/lex_lexertl_token.hpp>
#include <boost/spirit/include/lex_static_lexertl.hpp>
#include <boost/spirit/include/support_multi_pass.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/optional.hpp>

namespace puppet { namespace compiler { namespace lexer {
    /**
     * The lexer tab width.
     * This is how many spaces the lexer considers a tab to be.
     */
    constexpr const int LEXER_TAB_WIDTH = 4;

    /**
     * Exception for lexer errors.
     * @tparam Iterator The location iterator type.
     */
    template <typename Iterator>
    struct lexer_exception : std::runtime_error
    {
        /**
         * Constructs a lexer exception.
         * @param message The lexer exception message.
         * @param location The location where lexing failed.
         */
        lexer_exception(std::string const& message, Iterator location) :
            std::runtime_error(message),
            _location(location)
        {
        }

        /**
         * Gets the location where lexing failed.
         * @return Returns the location where lexing failed.
         */
        Iterator const& location() const
        {
            return _location;
        }

     private:
        Iterator _location;
    };

    /**
     * Lexer iterator type used to support heredoc parsing.
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
         */
        explicit lexer_iterator(Iterator const& iter) :
            boost::iterator_adaptor<lexer_iterator<Iterator>, Iterator>(iter),
            _position { 0, 1 }
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

        void set_next(lexer_iterator<Iterator> const& next)
        {
            // Set the next underlying iterator
            _next_iter = next.base_reference();
            _next_position = next._position;
        }

        bool get_next(lexer_iterator<Iterator>& next) const
        {
            if (!_next_iter) {
                return false;
            }

            // Return the next iterator data
            next.base_reference() = *_next_iter;
            next._position = _next_position;
            return true;
        }

        void increment()
        {
            auto& base = this->base_reference();

            auto current = *base;

            // If there is a next and we've reach the end of the line, "skip" to next
            // This will effectively skip over any heredoc lines that were parsed
            if (_next_iter && current == '\n') {
                base = *_next_iter;
                _next_iter = boost::none;

                _position = _next_position;
                _next_position = lexer::position();
            } else {
                // Otherwise, check for a new line and increment the line counter
                _position.increment(current == '\n');
                ++base;
            }
        }

        lexer::position _position;
        boost::optional<Iterator> _next_iter;
        lexer::position _next_position;
        bool _ignore_epp_end = true;
        bool _epp_end = true;
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
         * The string token type.
         */
        using string_token_type = string_token<input_iterator_type>;

        /**
         * Constructs a new lexer.
         */
        lexer()
        {
            namespace lex = boost::spirit::lex;

            // EPP lexing support
            // When in the EPP state, a match of an opening tag will transition to the normal lexer state.
            // When in the normal state, a match of a closing tag will transition to the EPP state.
            this->self(EPP_STATE) =
                lex::token_def<>(R"(<%#[^%]*%+([^%>]*%+)*>)",    static_cast<id_type>(token_id::comment))               [ epp_comment ] |
                lex::token_def<>(R"(<%#)",                       static_cast<id_type>(token_id::unclosed_comment))                      |
                lex::token_def<>(R"(<%=)",                       static_cast<id_type>(token_id::epp_render_expression)) [ epp_render ]  |
                lex::token_def<>(R"([\t ]*<%-)",                 static_cast<id_type>(token_id::epp_start_trim))        [ epp_start ]   |
                lex::token_def<>(R"(<%%)",                       static_cast<id_type>(token_id::epp_render_string))     [ escape_epp ]  |
                lex::token_def<>(R"(<%)",                        static_cast<id_type>(token_id::epp_start))             [ epp_start ]   |
                lex::token_def<>(R"([^<]*)",                     static_cast<id_type>(token_id::epp_render_string))     [ string_trim ] |
                lex::token_def<>(R"(.)",                         static_cast<id_type>(token_id::epp_render_string));
            this->self(base_type::initial_state(), EPP_STATE) =
                lex::token_def<>(R"(%>)",                        static_cast<id_type>(token_id::epp_end))               [ epp_end ] |
                lex::token_def<>(R"(-%>)",                       static_cast<id_type>(token_id::epp_end_trim))          [ epp_end ];

            // The following are lexer states that are used to parse regular expressions.
            // This solves the ambiguity between having multiple division operators on a single line (e.g. "1 / 2 / 3")
            // and parsing a regex; without this, "/ 2 /" above would parse as a regex token.
            // For SLASH_CHECK_STATE, we're doing a lookahead to see if the next token should be a division operator.
            // For FORCE_SLASH_STATE, the lookahead succeeded, so force the next token to be "/" and not a regex, and
            // reset state back to the initial state.
            this->self(SLASH_CHECK_STATE).add(SLASH_CHECK_PATTERN, '/');
            this->self(FORCE_SLASH_STATE, base_type::initial_state()) = lex::token_def<>(SLASH_CHECK_PATTERN, '/') [ use_last ];

            // The following are in match order
            // Add the three-character operators
            this->self.add
                (R"(<<\|)",                 static_cast<id_type>(token_id::left_double_collect));
            this->self +=
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

            // Add single character operators
            // The ids for these tokens are the characters themselves
            this->self += lex::token_def<>('[') |
                 lex::token_def<>(']') [ no_regex ] |
                '{' |
                '}' |
                '(' |
                 lex::token_def<>(')') [ no_regex ] |
                '=' |
                '>' |
                '<' |
                '+' |
                '-' |
                '/' |
                '*' |
                '%' |
                '!' |
                '.' |
                '|' |
                '@' |
                ':' |
                ',' |
                ';' |
                '?' |
                '~';

            // Add the keywords
            this->self.add
                ("case",     static_cast<id_type>(token_id::keyword_case))
                ("class",    static_cast<id_type>(token_id::keyword_class))
                ("default",  static_cast<id_type>(token_id::keyword_default))
                ("define",   static_cast<id_type>(token_id::keyword_define))
                ("if",       static_cast<id_type>(token_id::keyword_if))
                ("elsif",    static_cast<id_type>(token_id::keyword_elsif))
                ("else",     static_cast<id_type>(token_id::keyword_else))
                ("inherits", static_cast<id_type>(token_id::keyword_inherits))
                ("node",     static_cast<id_type>(token_id::keyword_node))
                ("and",      static_cast<id_type>(token_id::keyword_and))
                ("or",       static_cast<id_type>(token_id::keyword_or))
                ("undef",    static_cast<id_type>(token_id::keyword_undef))
                ("in",       static_cast<id_type>(token_id::keyword_in))
                ("unless",   static_cast<id_type>(token_id::keyword_unless))
                ("function", static_cast<id_type>(token_id::keyword_function))
                ("type",     static_cast<id_type>(token_id::keyword_type))
                ("attr",     static_cast<id_type>(token_id::keyword_attr))
                ("private",  static_cast<id_type>(token_id::keyword_private));
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

            // Comments, variables, types, names, bare words, regexes, and whitespace
            this->self +=
                lex::token_def<>(R"((#[^\n]*)|(\/\*[^*]*\*+([^/*][^*]*\*+)*\/))", static_cast<id_type>(token_id::comment))              [ lex::_pass = lex::pass_flags::pass_ignore ] |
                lex::token_def<>(R"(\$(::)?(\w+::)*\w+)",                         static_cast<id_type>(token_id::variable))             [ no_regex ] |
                lex::token_def<>(R"(\s+\[)",                                      static_cast<id_type>(token_id::array_start))          [ use_last ] |
                lex::token_def<>(R"(((::)?[A-Z][\w]*)+)",                         static_cast<id_type>(token_id::type))                 [ no_regex ] |
                lex::token_def<>(R"(((::)?[a-z][\w]*)(::[a-z][\w]*)*)",           static_cast<id_type>(token_id::name))                 [ no_regex ] |
                lex::token_def<>(R"([a-z_]([\w\-]*[\w])?)",                       static_cast<id_type>(token_id::bare_word))            [ no_regex ] |
                lex::token_def<>(R"((\/\/)|(\/[^*]([^\\/\n]|\\.)*\/))",           static_cast<id_type>(token_id::regex))                [ no_regex ] |
                lex::token_def<>(R"('([^\\']|\\.)*')",                            static_cast<id_type>(token_id::single_quoted_string)) [ parse_single_quoted_string ] |
                lex::token_def<>(R"(\"([^\\"]|\\.)*\")",                          static_cast<id_type>(token_id::double_quoted_string)) [ parse_double_quoted_string ] |
                lex::token_def<>(HEREDOC_PATTERN,                                 static_cast<id_type>(token_id::heredoc))              [ parse_heredoc ] |
                lex::token_def<>(R"(\d\w*(\.\d\w*)?([eE]-?\w*)?)",                static_cast<id_type>(token_id::number))               [ parse_number ] |
                lex::token_def<>(R"(\s+)",                                        static_cast<id_type>(token_id::whitespace))           [ lex::_pass = lex::pass_flags::pass_ignore ];

            // Lastly, a catch for unclosed quotes and unknown tokens
            this->self.add
                (R"(['"])", static_cast<id_type>(token_id::unclosed_quote))
                (R"(\/\*)", static_cast<id_type>(token_id::unclosed_comment))
                (R"(\$)",   static_cast<id_type>(token_id::invalid_variable))
                (R"(.)",    static_cast<id_type>(token_id::unknown));
        }

        /**
         * The state for EPP lexing.
         */
        static const char* const EPP_STATE;

    private:
        using context_type = typename iterator_type::shared_functor_type;

        static void parse_heredoc(input_iterator_type const& start, input_iterator_type& end, boost::spirit::lex::pass_flags& matched, id_type& id, context_type& context)
        {
            using namespace std;

            static const regex pattern(HEREDOC_PATTERN);

            // Force any following '/' to be interpreted as a '/' token
            force_slash(context);

            // Helper functions
            static auto is_space = [](char c) { return c == ' ' || c == '\t'; };
            static auto throw_not_found = [](input_iterator_type const& location, std::string const& tag) {
                throw lexer_exception<input_iterator_type>((boost::format("unexpected end of input while looking for heredoc end tag '%1%'.") % tag).str(), location);
            };
            static auto move_next_line = [](input_iterator_type& begin, input_iterator_type const& end) -> bool {
                for (; begin != end && *begin != '\n'; ++begin);
                if (begin == end) {
                    return false;
                }

                // Move past the newline
                ++begin;
                return true;
            };

            // regex needs bi-directional iterators, so we need to copy the token range (just the @(...) part)
            string_type token(start, end);

            // Extract the tag, format, and escapes from the token
            match_results<typename string_type::const_iterator> match;
            if (!regex_match(token, match, pattern) || match.size() != 6) {
                throw lexer_exception<input_iterator_type>("unexpected heredoc format.", start);
            }

            // Trim the tag
            string_type tag(match[1].first, match[1].second);
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
                escapes.assign(match[5].first, match[5].second);
                if (escapes.empty()) {
                    // Enable all heredoc escapes
                    escapes = HEREDOC_ESCAPES;
                } else {
                    // Verify the escapes
                    if (!boost::all(escapes, boost::is_any_of(HEREDOC_ESCAPES))) {
                        throw lexer_exception<input_iterator_type>((boost::format("invalid heredoc escapes '%1%': only t, r, n, s, u, L, and $ are allowed.") % escapes).str(), start);
                    }
                    // TODO: verify uniqueness of each character (i.e. is this really important)?
                }

                // Treat L as "escaping newlines"
                boost::replace_all(escapes, "L", "\n");

                // Escaping automatically adds '\' to the list
                escapes += "\\";
            }

            auto& eoi = context.get_eoi();

            // Move to the next line to process, skipping over any previous heredoc on the token's line
            input_iterator_type value_begin;
            if (!start.get_next(value_begin)) {
                value_begin = end;
                if (!move_next_line(value_begin, eoi)) {
                    throw_not_found(start, tag);
                }
            }

            bool remove_break = false;
            int margin = 0;
            auto value_end = value_begin;

            // Search for the end tag
            while (value_end != eoi) {
                auto line_end = value_end;
                for (; line_end != end && is_space(*line_end); ++line_end) {
                    margin += (*line_end == ' ') ? 1 : LEXER_TAB_WIDTH;
                }
                if (line_end == eoi) {
                    throw_not_found(start, tag);
                }
                if (*line_end == '|') {
                    for (++line_end; line_end != end && is_space(*line_end); ++line_end);
                }
                if (line_end == eoi) {
                    throw_not_found(start, tag);
                }
                if (*line_end == '-') {
                    remove_break = true;
                    for (++line_end; line_end != end && is_space(*line_end); ++line_end);
                }
                if (line_end == eoi) {
                    throw_not_found(start, tag);
                }

                // Look for the end tag
                auto search_it = tag.begin();
                for (; line_end != eoi && search_it != tag.end() && *search_it == *line_end; ++search_it, ++line_end);
                if (search_it == tag.end()) {
                    // Possibly found the tag; ensure the remainder of the line is whitespace
                    for (; line_end != eoi && is_space(*line_end); ++line_end);
                    if (line_end != eoi && *line_end == '\r') {
                        ++line_end;
                    }
                    if (line_end == eoi || *line_end == '\n') {
                        break;
                    }

                    // Not found
                }

                // Move to the next line
                move_next_line(line_end, eoi);
                value_end = line_end;
                margin = 0;
            }

            if (value_end == eoi) {
                throw_not_found(start, tag);
            }

            auto next = value_end;
            move_next_line(next, eoi);
            end.set_next(next);
            context.set_value(
                string_token_type(
                    range{ start.position(), end.position() },
                    boost::make_iterator_range(rvalue_cast(value_begin), rvalue_cast(value_end)),
                    rvalue_cast(escapes),
                    0,
                    interpolated,
                    rvalue_cast(format),
                    margin,
                    remove_break
                )
            );
        }

        static void parse_single_quoted_string(input_iterator_type const& start, input_iterator_type const& end, boost::spirit::lex::pass_flags& matched, id_type& id, context_type& context)
        {
            // Force any following '/' to be interpreted as a '/' token
            force_slash(context);

            // Find the end of the string, not including the quote
            auto value_start = start;
            ++value_start;
            input_iterator_type value_end;
            for (auto current = value_start; current != end; ++current) {
                value_end = current;
            }
            context.set_value(
                string_token_type(
                    range{ start.position(), end.position() },
                    boost::make_iterator_range(rvalue_cast(value_start), rvalue_cast(value_end)),
                    "\\'",
                    '\'',
                    false
                )
            );
        }

        static void parse_double_quoted_string(input_iterator_type const& start, input_iterator_type const& end, boost::spirit::lex::pass_flags& matched, id_type& id, context_type& context)
        {
            // Force any following '/' to be interpreted as a '/' token
            force_slash(context);

            /// Find the end of the string, not including the quote
            auto value_start = start;
            ++value_start;
            input_iterator_type value_end;
            for (auto current = value_start; current != end; ++current) {
                value_end = current;
            }
            context.set_value(
                string_token_type(
                    range{ start.position(), end.position() },
                    boost::make_iterator_range(rvalue_cast(value_start), rvalue_cast(value_end)),
                    "\\\"'nrtsu$",
                    '"'
                )
            );
        }

        static void parse_number(input_iterator_type const& start, input_iterator_type const& end, boost::spirit::lex::pass_flags& matched, id_type& id, context_type& context)
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
            string_type token(start, end);

            // Match integral numbers
            int base = 0;
            if (regex_match(token, hex_pattern)) {
                base = 16;
            } else if (regex_match(token, octal_pattern)) {
                // Make sure the number is valid for an octal
                if (!regex_match(token, valid_octal_pattern)) {
                    throw lexer_exception<input_iterator_type>((boost::format("'%1%' is not a valid number.") % token).str(), start);
                }
                base = 8;
            } else if (regex_match(token, decimal_pattern)) {
                base = 10;
            }

            if (base != 0) {
                try {
                    context.set_value(
                        number_token(
                            range{ start.position(), end.position() },
                            stoll(token, 0, base),
                            base == 16 ? numeric_base::hexadecimal : (base == 8 ? numeric_base::octal : numeric_base::decimal)
                        )
                    );
                } catch (out_of_range const& ex) {
                    throw lexer_exception<input_iterator_type>(
                        (boost::format("'%1%' is not in the range of %2% to %3%.") %
                            token %
                            numeric_limits<int64_t>::min() %
                            numeric_limits<int64_t>::max()
                        ).str(),
                        start);
                }
                return;
            }

            // Match double
            if (regex_match(token, double_pattern)) {
                try {
                    context.set_value(
                        number_token(
                            range{ start.position(), end.position() },
                            stold(token, 0)
                        )
                    );
                } catch (out_of_range const& ex) {
                    throw lexer_exception<input_iterator_type>(
                        (boost::format("'%1%' is not in the range of %2% to %3%.") %
                            token %
                            numeric_limits<double>::min() %
                            numeric_limits<double>::max()
                        ).str(),
                        start);
                }
                return;
            }

            // Not a valid number
            throw lexer_exception<input_iterator_type>((boost::format("'%1%' is not a valid number.") % token).str(), start);
        }

        static void no_regex(input_iterator_type const& start, input_iterator_type const& end, boost::spirit::lex::pass_flags& matched, id_type& id, context_type& context)
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

        static void use_last(input_iterator_type& start, input_iterator_type const& end, boost::spirit::lex::pass_flags& matched, id_type& id, context_type& context)
        {
            // Use the last character in the range
            auto last = start;
            for (auto current = start; current != end; ++current) {
                last = current;
            }
            start = last;
        }

        static void epp_comment(input_iterator_type const& start, input_iterator_type& end, boost::spirit::lex::pass_flags& matched, id_type& id, context_type& context)
        {
            matched = boost::spirit::lex::pass_flags::pass_ignore;

            // Check if the comment ends with a trim specifier
            if (boost::ends_with(boost::make_iterator_range(start, end), "-%>")) {
                trim_right(start, end, matched, id, context);
            }
        }

        static void string_trim(input_iterator_type const& start, input_iterator_type& end, boost::spirit::lex::pass_flags& matched, id_type& id, context_type& context)
        {
            // If rendering a string, ensure the next token is an epp start trim token
            if (!context.lookahead(static_cast<size_t>(token_id::epp_start_trim))) {
                return;
            }

            // Because input iterators are forward only, we have to scan all the way from the beginning for the trim
            auto new_end = start;
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

        static void trim_right(input_iterator_type const& start, input_iterator_type& end, boost::spirit::lex::pass_flags& matched, id_type& id, context_type& context)
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

        static void escape_epp(input_iterator_type const& start, input_iterator_type& end, boost::spirit::lex::pass_flags& matched, id_type& id, context_type& context)
        {
            auto value_end = start;
            ++value_end;
            ++value_end;
            context.set_value(boost::make_iterator_range(start, value_end));
        }

        static void epp_start(input_iterator_type const& start, input_iterator_type& end, boost::spirit::lex::pass_flags& matched, id_type& id, context_type& context)
        {
            matched = boost::spirit::lex::pass_flags::pass_ignore;
            context.set_state(context.get_state_id("INITIAL"));
            end._epp_end = false;
        }

        static void epp_render(input_iterator_type const& start, input_iterator_type& end, boost::spirit::lex::pass_flags& matched, id_type& id, context_type& context)
        {
            context.set_state(context.get_state_id("INITIAL"));
            end._ignore_epp_end = false;
        }

        static void epp_end(input_iterator_type const& start, input_iterator_type& end, boost::spirit::lex::pass_flags& matched, id_type& id, context_type& context)
        {
            if (start._ignore_epp_end) {
                matched = boost::spirit::lex::pass_flags::pass_ignore;
            }
            end._ignore_epp_end = true;
            end._epp_end = true;

            if (id == static_cast<size_t>(token_id::epp_end_trim)) {
                trim_right(start, end, matched, id, context);
            }
        }

        static const char* const HEREDOC_PATTERN;
        static const char* const HEREDOC_ESCAPES;
        static const char* const FORCE_SLASH_STATE;
        static const char* const SLASH_CHECK_STATE;
        static const char* const SLASH_CHECK_PATTERN;
    };

    template<typename Base>
    char const* const lexer<Base>::HEREDOC_PATTERN = R"(@\(\s*([^):/\r\n]+)\s*(:\s*([a-z][a-zA-Z0-9_+]+))?\s*(\/\s*([\w|$]*)\s*)?\))";
    template<typename Base>
    char const* const lexer<Base>::HEREDOC_ESCAPES = "trnsuL$";
    template<typename Base>
    char const* const lexer<Base>::FORCE_SLASH_STATE = "FS";
    template<typename Base>
    char const* const lexer<Base>::SLASH_CHECK_STATE = "SC";
    template<typename Base>
    char const* const lexer<Base>::SLASH_CHECK_PATTERN = R"(\s*\/\s+)";
    template<typename Base>
    char const* const lexer<Base>::EPP_STATE = "EPP";

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
    using lexer_token = boost::spirit::lex::lexertl::token<Iterator, boost::mpl::vector<string_token<Iterator>, number_token>>;

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
     * @param input The input string iterator range.
     * @return Returns the last position in the iterator range.
     */
    position get_last_position(boost::iterator_range<lexer_string_iterator> const& range);

    /**
     * Utility visitor for obtaining token range information.
     */
    struct token_range_visitor : boost::static_visitor<range>
    {
        /**
         * Gets the beginning position and length of a token.
         * @tparam Token The type of token.
         * @param token The token to get the position and length of.
         * @return Returns the token range.
         */
        template <typename Iterator>
        result_type operator()(boost::iterator_range<Iterator> const& token) const
        {
            return range{ token.begin().position(), token.end().position() };
        }

        /**
         * Gets the beginning position and length of a token.
         * @tparam Token The type of token.
         * @param token The token to get the position and length of.
         * @return Returns the token range.
         */
        template <typename Token>
        result_type operator()(Token const& token) const
        {
            return token.range();
        }
    };

    /**
     * Gets the given token's range.
     * @tparam Input The input type.
     * @tparam Token The type of token.
     * @param input The input to use when calculating the last token position.
     * @param token The token to get the position for.
     * @return Returns the token's range (pair of position and length).
     */
    template <typename Input, typename Token>
    range get_range(Input& input, Token const& token)
    {
        if (token == Token()) {
            return range{ get_last_position(input), 1 };
        }
        return boost::apply_visitor(token_range_visitor(), token.value());
    }

}}}  // namespace puppet::compiler::lexer
