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
#include "../cast.hpp"
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

namespace puppet { namespace lexer {
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
         * @param location The location where lexing failed.
         * @param message The lexer exception message.
         */
        lexer_exception(Iterator location, std::string const& message) :
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
        typedef typename boost::spirit::lex::lexer<Base> base_type;

        /**
         * The type of token this lexer produces.
         */
        typedef typename Base::token_type token_type;

        /**
         * The token id type.
         */
        typedef typename Base::id_type id_type;

        /**
         * The type of iterator for the output token stream.
         */
        typedef typename Base::iterator_type iterator_type;

        /**
         * The type of character for the input stream.
         */
        typedef typename Base::char_type char_type;

        /**
         * The type of string for the input stream.
         */
        typedef std::basic_string<char_type> string_type;

        /**
         * The input stream iterator type.
         */
        typedef typename token_type::iterator_type input_iterator_type;

        /**
         * The string token type.
         */
        typedef string_token<input_iterator_type> string_token_type;

        /**
         * Constructs a new lexer.
         */
        lexer() :
            single_quoted_string("'([^\\\\']|\\\\\\\\|\\\\.)*'",        static_cast<id_type>(token_id::single_quoted_string)),
            double_quoted_string("\\\"([^\\\\\"]|\\\\\\\\|\\\\.)*\\\"", static_cast<id_type>(token_id::double_quoted_string)),
            heredoc(HEREDOC_PATTERN,                                    static_cast<id_type>(token_id::heredoc)),
            number("\\d\\w*(\\.\\d\\w*)?([eE]-?\\w*)?",                 static_cast<id_type>(token_id::number))
        {
            namespace lex = boost::spirit::lex;
            using namespace std::placeholders;

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
                ("<<\\|", static_cast<id_type>(token_id::left_double_collect));
            this->self +=
                lex::token_def<>("\\|>>", static_cast<id_type>(token_id::right_double_collect)) [ no_regex ];

            // Add the two-character operators
            this->self.add
                ("\\+=", static_cast<id_type>(token_id::append))
                ("-=",   static_cast<id_type>(token_id::remove))
                ("==",   static_cast<id_type>(token_id::equals))
                ("!=",   static_cast<id_type>(token_id::not_equals))
                ("=~",   static_cast<id_type>(token_id::match))
                ("!~",   static_cast<id_type>(token_id::not_match))
                (">=",   static_cast<id_type>(token_id::greater_equals))
                ("<=",   static_cast<id_type>(token_id::less_equals))
                ("=>",   static_cast<id_type>(token_id::fat_arrow))
                ("\\+>", static_cast<id_type>(token_id::plus_arrow))
                ("<<",   static_cast<id_type>(token_id::left_shift))
                ("<\\|", static_cast<id_type>(token_id::left_collect))
                (">>",   static_cast<id_type>(token_id::right_shift))
                ("@@",   static_cast<id_type>(token_id::atat))
                ("->",   static_cast<id_type>(token_id::in_edge))
                ("~>",   static_cast<id_type>(token_id::in_edge_sub))
                ("<-",   static_cast<id_type>(token_id::out_edge))
                ("<~",   static_cast<id_type>(token_id::out_edge_sub));
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

            // Variables, bare words, numbers, class references, names, regexes, strings, comments, and whitespace
            this->self +=
                lex::token_def<>("\\s+\\[",                                             static_cast<id_type>(token_id::array_start))    [ use_last ] |
                lex::token_def<>("((::)?[A-Z][\\w]*)+",                                 static_cast<id_type>(token_id::type))           [ no_regex ] |
                lex::token_def<>("((::)?[a-z][\\w]*)(::[a-z][\\w]*)*",                  static_cast<id_type>(token_id::name))           [ no_regex ] |
                lex::token_def<>("[a-z_]([\\w\\-]*[\\w])?",                             static_cast<id_type>(token_id::bare_word))      [ no_regex ] |
                lex::token_def<>("(\\/\\/)|(\\/[^*][^/\\n]*\\/)",                       static_cast<id_type>(token_id::regex))          [ no_regex ] |
                single_quoted_string                                                                                                    [ bind(&lexer::parse_single_quoted_string, this, _1, _2, _3, _4, _5) ] |
                double_quoted_string                                                                                                    [ bind(&lexer::parse_double_quoted_string, this, _1, _2, _3, _4, _5) ] |
                heredoc                                                                                                                 [ bind(&lexer::parse_heredoc, this, _1, _2, _3, _4, _5) ] |
                number                                                                                                                  [ parse_number ] |
                lex::token_def<>("(#[^\\n]*)|(\\/\\*[^*]*\\*+([^/*][^*]*\\*+)*\\/)",    static_cast<id_type>(token_id::comment))        [ lex::_pass = lex::pass_flags::pass_ignore ] |
                lex::token_def<>("\\s+",                                                static_cast<id_type>(token_id::whitespace))     [ lex::_pass = lex::pass_flags::pass_ignore ];
            this->self.add
                ("\\$(::)?(\\w+::)*\\w+",                                               static_cast<id_type>(token_id::variable));

            // Lastly, a catch for unclosed quotes and unknown tokens
            this->self.add
                ("['\"]", static_cast<id_type>(token_id::unclosed_quote))
                (".",     static_cast<id_type>(token_id::unknown));
        }

        /**
         * The token representing single quoted strings.
         */
        boost::spirit::lex::token_def<string_token_type> single_quoted_string;

        /**
         * The token representing double quoted strings.
         */
        boost::spirit::lex::token_def<string_token_type> double_quoted_string;

        /**
         * The token representing heredocs.
         */
        boost::spirit::lex::token_def<string_token_type> heredoc;

        /**
         * The token representing numbers.
         */
        boost::spirit::lex::token_def<number_token> number;

    private:
        typedef typename iterator_type::shared_functor_type context_type;

        void parse_heredoc(input_iterator_type const& start, input_iterator_type& end, boost::spirit::lex::pass_flags& matched, id_type& id, context_type& context)
        {
            using namespace std;

            static const regex pattern(HEREDOC_PATTERN);

            // Force any following '/' to be interpreted as a '/' token
            force_slash(context);

            // Helper functions
            static auto is_space = [](char c) { return c == ' ' || c == '\t'; };
            static auto throw_not_found = [](input_iterator_type const& location, std::string const& tag) {
                throw lexer_exception<input_iterator_type>(location, (boost::format("unexpected end of input while looking for heredoc end tag '%1%'.") % tag).str());
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
                throw lexer_exception<input_iterator_type>(start, "unexpected heredoc format.");
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
                        throw lexer_exception<input_iterator_type>(start, (boost::format("invalid heredoc escapes '%1%': only t, r, n, s, u, L, and $ are allowed.") % escapes).str());
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
            input_iterator_type doc_begin;
            if (!start.get_next(doc_begin)) {
                doc_begin = end;
                if (!move_next_line(doc_begin, eoi)) {
                    throw_not_found(start, tag);
                }
            }

            bool remove_break = false;
            bool has_margin = false;
            int margin = 0;
            auto doc_end = doc_begin;

            // Search for the end tag
            while (doc_end != eoi) {
                auto line_end = doc_end;
                for (; line_end != end && is_space(*line_end); ++line_end) {
                    margin += (*line_end == ' ') ? 1 : LEXER_TAB_WIDTH;
                }
                if (line_end == eoi) {
                    throw_not_found(start, tag);
                }
                if (*line_end == '|') {
                    has_margin = true;
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
                doc_end = line_end;
                margin = 0;
            }

            if (doc_end == eoi) {
                throw_not_found(start, tag);
            }

            auto next = doc_end;
            move_next_line(next, eoi);
            end.set_next(next);
            context.set_value(string_token_type(doc_begin.position(), doc_begin, rvalue_cast(doc_end), rvalue_cast(escapes), 0, interpolated, rvalue_cast(format), margin, remove_break));
        }

        void parse_single_quoted_string(input_iterator_type start, input_iterator_type const& end, boost::spirit::lex::pass_flags& matched, id_type& id, context_type& context)
        {
            auto position = start.position();

            // Force any following '/' to be interpreted as a '/' token
            force_slash(context);

            // Find the end of the string, not including the quote
            auto last = ++start;
            for (auto current = start; current != end; ++current) {
                last = current;
            }
            context.set_value(string_token_type(rvalue_cast(position), rvalue_cast(start), rvalue_cast(last), "\\'", '\'', false));
        }

        void parse_double_quoted_string(input_iterator_type start, input_iterator_type const& end, boost::spirit::lex::pass_flags& matched, id_type& id, context_type& context)
        {
            auto position = start.position();

            // Force any following '/' to be interpreted as a '/' token
            force_slash(context);

            // Find the end of the string, not including the quote
            auto last = ++start;
            for (auto current = start; current != end; ++current) {
                last = current;
            }
            context.set_value(string_token_type(rvalue_cast(position), rvalue_cast(start), rvalue_cast(last), "\\\"'nrtsu$", '"'));
        }

        static void parse_number(input_iterator_type const& start, input_iterator_type const& end, boost::spirit::lex::pass_flags& matched, id_type& id, context_type& context)
        {
            using namespace std;

            static const regex hex_pattern("0[xX][0-9A-Fa-f]+");
            static const regex octal_pattern("0\\d+");
            static const regex valid_octal_pattern("0[0-7]+");
            static const regex decimal_pattern("0|([1-9]\\d*)");
            static const regex double_pattern("[0-9]\\d*(\\.\\d+)?([eE]-?\\d+)?)");

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
                    throw lexer_exception<input_iterator_type>(start, (boost::format("'%1%' is not a valid number.") % token).str());
                }
                base = 8;
            } else if (regex_match(token, decimal_pattern)) {
                base = 10;
            }

            if (base != 0) {
                try {
                    context.set_value(number_token(start.position(), stoll(token, 0, base), base == 16 ? numeric_base::hexadecimal : (base == 8 ? numeric_base::octal : numeric_base::decimal)));
                } catch (out_of_range const& ex) {
                    throw lexer_exception<input_iterator_type>(start,
                        (boost::format("'%1%' is not in the range of %2% to %3%.") %
                            token %
                            numeric_limits<int64_t>::min() %
                            numeric_limits<int64_t>::max()
                        ).str());
                }
                return;
            }

            // Match double
            if (regex_match(token, double_pattern)) {
                try {
                    context.set_value(number_token(start.position(), stold(token, 0)));
                } catch (out_of_range const& ex) {
                    throw lexer_exception<input_iterator_type>(start,
                        (boost::format("'%1%' is not in the range of %2% to %3%.") %
                            token %
                            numeric_limits<long double>::min() %
                            numeric_limits<long double>::max()
                        ).str());
                }
                return;
            }

            // Not a valid number
            throw lexer_exception<input_iterator_type>(start, (boost::format("'%1%' is not a valid number.") % token).str());
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

        static const char* const HEREDOC_PATTERN;
        static const char* const HEREDOC_ESCAPES;
        static const char* const FORCE_SLASH_STATE;
        static const char* const SLASH_CHECK_STATE;
        static const char* const SLASH_CHECK_PATTERN;
    };

    template<typename Base>
    char const* const lexer<Base>::HEREDOC_PATTERN = "@\\(\\s*([^):/\\r\\n]+)\\s*(:\\s*([a-z][a-zA-Z0-9_+]+))?\\s*(\\/\\s*([\\w|$]*)\\s*)?\\)";
    template<typename Base>
    char const* const lexer<Base>::HEREDOC_ESCAPES = "trnsuL$";
    template<typename Base>
    char const* const lexer<Base>::FORCE_SLASH_STATE = "FS";
    template<typename Base>
    char const* const lexer<Base>::SLASH_CHECK_STATE = "SC";
    template<typename Base>
    char const* const lexer<Base>::SLASH_CHECK_PATTERN = "\\s*(\\/\\*[^*]*\\*+([^/*][^*]*\\*+)*\\/\\s*)*\\/";

    /**
     * The input iterator for files.
     */
    typedef lexer_iterator<boost::spirit::multi_pass<std::istreambuf_iterator<char>>> lexer_istreambuf_iterator;
    /**
     * The input iterator for strings.
     */
    typedef lexer_iterator<typename std::string::const_iterator> lexer_string_iterator;

    /**
     * The token type for the lexer.
     * @tparam Iterator The input iterator for the token.
     */
    template <typename Iterator>
    using lexer_token = boost::spirit::lex::lexertl::token<Iterator, boost::mpl::vector<string_token<Iterator>, number_token>>;

    /**
     * The lexer to use for files.
     */
    typedef lexer<boost::spirit::lex::lexertl::actor_lexer<lexer_token<lexer_istreambuf_iterator>>>        file_lexer;
    /**
     * The static lexer to use for files.
     * Include "static_lexer.hpp" before including "lexer.hpp" to use this type.
     */
    typedef lexer<boost::spirit::lex::lexertl::static_actor_lexer<lexer_token<lexer_istreambuf_iterator>>> file_static_lexer;
    /**
     * The lexer to use for strings.
     */
    typedef lexer<boost::spirit::lex::lexertl::actor_lexer<lexer_token<lexer_string_iterator>>>            string_lexer;
    /**
     * The static lexer to use for strings.
     * Include "static_lexer.hpp" before including "lexer.hpp" to use this type.
     */
    typedef lexer<boost::spirit::lex::lexertl::static_actor_lexer<lexer_token<lexer_string_iterator>>>     string_static_lexer;

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
     * Utility type for visiting tokens for position and line information.
     */
    struct token_position_visitor : boost::static_visitor<position>
    {
        /**
         * Called for tokens that contain iterator ranges.
         * @param range The iterator range of the token.
         * @return Returns the token position.
         */
        template <typename Iterator>
        result_type operator()(boost::iterator_range<Iterator> const& range) const
        {
            return range.begin().position();
        }

        /**
         * Called for custom tokens.
         * @tparam Token The type of token.
         * @param token The custom token.
         * @return Returns the token position.
         */
        template <typename Token>
        result_type operator()(Token const& token) const
        {
            return token.position();
        }
    };

    /**
     * Gets the given token's position.
     * @tparam Input The input type.
     * @tparam Token The type of token.
     * @param input The input to use when calculating the last token position.
     * @param token The token to get the position for.
     * @return Returns the token's position.
     */
    template <typename Input, typename Token>
    position get_position(Input& input, Token const& token)
    {
        if (token == Token()) {
            return get_last_position(input);
        }
        return boost::apply_visitor(token_position_visitor(), token.value());
    }

}}  // namespace puppet::lexer
