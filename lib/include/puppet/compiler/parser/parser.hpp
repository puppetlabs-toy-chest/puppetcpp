/**
 * @file
 * Declares the Puppet parsing functions.
 */
#pragma once

#include "../ast/ast.hpp"
#include "../lexer/lexer.hpp"
#include "../module.hpp"
#include <boost/iterator.hpp>
#include <boost/optional.hpp>
#include <string>
#include <memory>

namespace puppet { namespace compiler { namespace parser {

    /**
     * Parses the given file into a syntax tree.
     * @param path The path to the file to parse.
     * @param module The module where the parsing is taking place.
     * @param epp True if the file is in EPP format or false if not.
     * @return Returns the parsed syntax tree.
     */
    std::shared_ptr<ast::syntax_tree> parse_file(std::string path, compiler::module const* module = nullptr, bool epp = false);

    /**
     * Parses the given string into a syntax tree.
     * @param source The puppet source code to parse.
     * @param module The module where the parsing is taking place.
     * @param epp True if the string is in EPP format or false if not.
     * @return Returns the parsed syntax tree.
     */
    std::shared_ptr<ast::syntax_tree> parse_string(std::string source, std::string path = "<string>", compiler::module const* module = nullptr, bool epp = false);

    /**
     * Interpolates a string iterator range into a syntax tree.
     * @param range The range of the string to interpolate.
     * @param module The module where the parsing is taking place.
     * @return Returns the parsed syntax tree.
     */
    std::shared_ptr<ast::syntax_tree> interpolate(boost::iterator_range<lexer::lexer_string_iterator> range, compiler::module const* module = nullptr);

    /**
     * Parses a single postfix expression from a string.
     * @param source The source to parse.
     * @return Returns the postfix expression if parsed successfully or boost::none if not.
     */
    boost::optional<ast::postfix_expression> parse_postfix(std::string const& source);

}}}  // namespace puppet::compiler::parser
