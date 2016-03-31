/**
 * @file
 * Declares the validator for type specifications.
 */
#pragma once

#include "../ast/ast.hpp"
#include <boost/variant.hpp>

namespace puppet { namespace compiler { namespace validation {

    /**
     * Represents a validator for type specificationss.
     */
    struct type_validator
    {
        /**
         * Validates the given postfix expression representing a type specification.
         * @param expression The expression to validate.
         */
        static void validate(ast::postfix_expression const& expression);
    };

}}}  // namespace puppet::compiler::validation
