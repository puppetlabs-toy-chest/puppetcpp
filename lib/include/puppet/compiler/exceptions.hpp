/**
 * @file
 * Declares the compiler-related exceptions.
 */
#pragma once

#include "lexer/position.hpp"
#include "evaluation/stack_frame.hpp"
#include "../cast.hpp"
#include <boost/spirit/home/x3.hpp>
#include <string>
#include <exception>
#include <memory>

namespace puppet { namespace compiler {

    namespace ast {
        // Forward declaration of context
        struct context;

        // Forward declaration of syntax tree
        struct syntax_tree;
    }

    /**
     * Exception for arguments passed by index.
     */
    struct argument_exception : std::runtime_error
    {
        /**
         * Constructs a new argument exception.
         * @param message The exception message.
         * @param index The index of the argument.
         */
        argument_exception(std::string const& message, size_t index);

        /**
         * Gets the index of the argument that caused the exception.
         * @return Returns the index of the argument that caused the exception.
         */
        size_t index() const;

     private:
        size_t _index;
    };

    /**
     * Exception for lexer errors.
     * @tparam Iterator The iterator type.
     */
    template <typename Iterator>
    struct lexer_exception : std::runtime_error
    {
        /**
         * Constructs a lexer exception.
         * @param message The lexer exception message.
         * @param begin The beginning iterator where lexing failed.
         * @param end The ending iterator (exclusive) where lexing failed.
         */
        lexer_exception(std::string const& message, Iterator begin, Iterator end) :
            std::runtime_error(message),
            _begin(rvalue_cast(begin)),
            _end(rvalue_cast(end))
        {
        }

        /**
         * Gets the beginning iterator where lexing failed.
         * @return Returns the beginning iterator where lexing failed.
         */
        Iterator const& begin() const
        {
            return _begin;
        }

        /**
         * Gets the ending iterator where lexing failed.
         * @return Returns the ending iterator where lexing failed.
         */
        Iterator const& end() const
        {
            return _end;
        }

     private:
        Iterator _begin;
        Iterator _end;
    };

    /**
     * Exception for parse errors.
     */
    struct parse_exception : std::runtime_error
    {
        /**
         * Constructs a parse exception for the token where lexing failed.
         * @tparam Token The token type.
         * @param token The token where lexing failed.
         */
        template <typename Token>
        explicit parse_exception(Token const& token) :
            std::runtime_error(format_message(static_cast<lexer::token_id>(token.id())))
        {
            std::tie(_begin, _end) = boost::apply_visitor(lexer::token_range_visitor(), token.value());
        }

        /**
         * Constructs a parse exception for the input iterators where lexing failed.
         * @tparam Iterator The input iterator type.
         * @param begin The beginning input iterator.
         * @param end The ending input iterator.
         */
        template <typename Iterator>
        parse_exception(Iterator const& begin, Iterator const& end) :
            parse_exception(
                format_message(begin == end ? static_cast<boost::optional<char>>(boost::none) : *begin),
                begin.position(),
                lexer::position{ begin.position().offset() + 1, begin.position().line() }
            )
        {
        }

        /**
         * Constructs a parse exception for a lexer exception.
         * @tparam Iterator The input iterator type.
         * @param ex The lexer exception.
         */
        template <typename Iterator>
        parse_exception(lexer_exception<Iterator> const& ex) :
            parse_exception(ex.what(), ex.begin().position(), ex.end().position())
        {
        }

        /**
         * Constructs a parse exception for an expectation failure exception.
         * @tparam Iterator The token iterator type.
         * @param ex The expectation failure exception.
         * @param begin The beginning position for the parse exception.
         * @param end The ending position for the parse exception.
         */
        template <typename Iterator>
        parse_exception(boost::spirit::x3::expectation_failure<Iterator> const& ex, lexer::position begin, lexer::position end) :
            parse_exception(format_message(ex.which(), static_cast<lexer::token_id>(ex.where()->id())), rvalue_cast(begin), rvalue_cast(end))
        {
        }

        /**
         * Constructs a parse exception.
         * @param message The exception message.
         * @param begin The beginning position for the parse exception.
         * @param end The ending position for the parse exception.
         */
        parse_exception(std::string const& message, lexer::position begin, lexer::position end);

        /**
         * Gets the beginning position for the parse exception.
         * @return Returns the beginning position for the parse exception.
         */
        lexer::position const& begin() const;

        /**
         * Gets the ending position for the parse exception.
         * @return Returns the ending position for the parse exception.
         */
        lexer::position const& end() const;

    private:
        static std::string format_message(lexer::token_id id);
        static std::string format_message(boost::optional<char> character);
        static std::string format_message(std::string const& expected, lexer::token_id found);

        lexer::position _begin;
        lexer::position _end;
    };

    /**
     * Exception for evaluation errors.
     */
    struct evaluation_exception : std::runtime_error
    {
        /**
         * Constructs an evaluation exception.
         * @param message The exception message.
         */
        explicit evaluation_exception(std::string const& message);

        /**
         * Constructs an evaluation exception.
         * @param message The exception message.
         * @param context The AST context where evaluation failed.
         * @param backtrace The evaluation backtrace.
         */
        evaluation_exception(std::string const& message, ast::context context, std::vector<evaluation::stack_frame> backtrace);

        /**
         * Gets the AST context where evaluation failed.
         * @return Returns the AST context where evaluation failed.
         */
        ast::context const& context() const;

        /**
         * Gets the backtrace where evaluation failed.
         * @return Returns the backtrace where evaluation failed.
         */
        std::vector<evaluation::stack_frame> const& backtrace() const;

     private:
        std::shared_ptr<ast::syntax_tree> _tree;
        ast::context _context;
        std::vector<evaluation::stack_frame> _backtrace;
    };

    /**
     * Exception for compilation errors.
     */
    struct compilation_exception : std::runtime_error
    {
        /**
         * Constructs a compilation exception.
         * @param message The exception message.
         * @param path The path to the input file.
         * @param line The line containing the compilation error.
         * @param column The column containing the compilation error.
         * @param length The length of the compilation error.
         * @param text The line of text containing the compilation error.
         */
        explicit compilation_exception(std::string const& message, std::string path = std::string(), size_t line = 0, size_t column = 0, size_t length = 0, std::string text = std::string());

        /**
         * Constructs a compilation exception from a parse exception.
         * @param ex The parse exception.
         * @param path The path to the file that was parsed.
         * @param source The source code; if empty, the source text will be read from the file.
         */
        compilation_exception(parse_exception const& ex, std::string const& path, std::string const& source = {});

        /**
         * Constructs a compilation exception from an evaluation exception.
         * @param ex The evaluation exception.
         */
        explicit compilation_exception(evaluation_exception const& ex);

        /**
         * Gets the path of the input file.
         * @return Returns the path of the input file.
         */
        std::string const& path() const;

        /**
         * Gets the line of the compilation error.
         * @return Returns the line of the compilation error.
         */
        size_t line() const;

        /**
         * Gets the column of the compilation error.
         * @return Returns the column of the compilation error.
         */
        size_t column() const;

        /**
         * Gets the length of the compilation error.
         * @return Returns the length of the compilation error.
         */
        size_t length() const;

        /**
         * Gets the line of text containing the compilation error.
         * @return Returns the line of text containing the compilation error.
         */
        std::string const& text() const;

        /**
         * Gets the backtrace where evaluation failed.
         * @return Returns the backtrace where evaluation failed.
         */
        std::vector<evaluation::stack_frame> const& backtrace() const;

    private:
        std::string _path;
        size_t _line;
        size_t _column;
        size_t _length;
        std::string _text;
        std::vector<evaluation::stack_frame> _backtrace;
    };

}}  // puppet::compiler
