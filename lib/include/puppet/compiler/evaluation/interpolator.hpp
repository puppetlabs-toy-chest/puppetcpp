/**
 * @file
 * Declares the string interpolator.
 */
#pragma once

#include "../ast/ast.hpp"

namespace puppet { namespace compiler { namespace evaluation {

    /**
     * Forward declaration of context.
     */
    struct context;

    /**
     * Represents the string interpolator.
     */
    struct interpolator
    {
        /**
         * Constructs an string interpolator.
         * @param context The current evaluation context.
         */
        explicit interpolator(evaluation::context& context);

        /**
         * Interpolates the given string expression.
         * @param expression The expression to interpolate.
         * @return Returns the interpolated string.
         */
        std::string interpolate(ast::string const& expression);

     private:
        evaluation::context& _context;
    };

}}}  // puppet::compiler::evaluation
