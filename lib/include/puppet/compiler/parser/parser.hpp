/**
 * @file
 * Declares the Puppet parsing functions.
 */
#pragma once

#include "../ast/ast.hpp"
#include "../module.hpp"
#include <boost/iterator.hpp>
#include <string>
#include <memory>

namespace puppet { namespace compiler { namespace parser {

    /**
     * Parses the given file into a syntax tree.
     * @param path The path to the file to parse.
     * @param moodule The module where the parsing is taking place.
     * @return Returns the parsed syntax tree.
     */
    std::shared_ptr<ast::syntax_tree> parse_file(std::string path, compiler::module const* module = nullptr);

    /**
     * Parses the given string into a syntax tree.
     * @param source The puppet source code to parse.
     * @param moodule The module where the parsing is taking place.
     * @return Returns the parsed syntax tree.
     */
    std::shared_ptr<ast::syntax_tree> parse_string(std::string source, compiler::module const* module = nullptr);

    /**
     * Interpolates a string iterator range into a syntax tree.
     * @param range The range of the string to interpolate.
     * @param moodule The module where the parsing is taking place.
     * @return Returns the parsed syntax tree.
     */
    std::shared_ptr<ast::syntax_tree> interpolate(boost::iterator_range<lexer::lexer_string_iterator> range, compiler::module const* module = nullptr);

}}}  // namespace puppet::compiler::parser
