/**
 * @file
 * Declares the Puppet parsing functions.
 */
#pragma once

#include "../ast/ast.hpp"
#include "../module.hpp"
#include "../../logging/logger.hpp"
#include <boost/iterator.hpp>
#include <boost/optional.hpp>
#include <string>
#include <memory>

namespace puppet { namespace compiler { namespace parser {

    /**
     * Parses the given file into a syntax tree.
     * @param logger The logger to use to log messages.
     * @param path The path to the file to parse.
     * @param module The module where the parsing is taking place.
     * @param epp True if the file is in EPP format or false if not.
     * @return Returns the parsed syntax tree.
     */
    std::shared_ptr<ast::syntax_tree> parse_file(logging::logger& logger, std::string path, compiler::module const* module = nullptr, bool epp = false);

    /**
     * Parses the given string into a syntax tree.
     * @param logger The logger to use to log messages.
     * @param source The puppet source code to parse.
     * @param path The path to the file being parsed.
     * @param module The module where the parsing is taking place.
     * @param epp True if the string is in EPP format or false if not.
     * @return Returns the parsed syntax tree.
     */
    std::shared_ptr<ast::syntax_tree> parse_string(logging::logger& logger, std::string source, std::string path = "<string>", compiler::module const* module = nullptr, bool epp = false);

    /**
     * Parses a single postfix expression from a string.
     * @param source The source to parse.
     * @return Returns the postfix expression if parsed successfully or boost::none if not.
     */
    boost::optional<ast::postfix_expression> parse_postfix(std::string const& source);

}}}  // namespace puppet::compiler::parser
