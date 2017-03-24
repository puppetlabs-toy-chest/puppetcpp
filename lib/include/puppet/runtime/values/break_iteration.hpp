/**
 * @file
 * Declares the break iteration runtime value.
 */
#pragma once

#include "../../compiler/ast/ast.hpp"
#include "../../compiler/evaluation/stack_frame.hpp"
#include <boost/variant.hpp>
#include <memory>

namespace puppet { namespace compiler {

    // Forward declare evaluation exception
    struct evaluation_exception;

}}  // namespace puppet::compiler

namespace puppet { namespace runtime { namespace values {

    /**
     * Represents the break iteration runtime value.
     */
    struct break_iteration
    {
        /**
         * Constructs a break iteration value from a break statement.
         * @param statement The break statement.
         * @param frames The evaluation frames when the break statement was evaluated.
         */
        explicit break_iteration(compiler::ast::break_statement const& statement, std::vector<compiler::evaluation::stack_frame> frames = {});

        /**
         * Creates an illegal break evaluation exception.
         * @return Returns the evaluation exception.
         */
        puppet::compiler::evaluation_exception create_exception() const;

     private:
        std::shared_ptr<compiler::ast::syntax_tree> _tree;
        compiler::ast::context _context;
        std::vector<compiler::evaluation::stack_frame> _frames;
    };

    /**
     * Stream insertion operator for break iteration.
     * @param os The output stream to write the value to.
     * @param value The break iteration value to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, break_iteration const& value);

    /**
     * Equality operator for break iteration.
     * @param left The left break iteration to compare.
     * @param right The right break iteration to compare.
     * @return Returns true if the two values are equal or false if not.
     */
    bool operator==(break_iteration const& left, break_iteration const& right);

    /**
     * Inequality operator for break iteration.
     * @param left The left break iteration to compare.
     * @param right The right break iteration to compare.
     * @return Returns true if the two values are not equal or false if they are equal.
     */
    bool operator!=(break_iteration const& left, break_iteration const& right);

    /**
     * Hashes the break iteration value.
     * @param value The break iteration to hash.
     * @return Returns the hash value for the value.
     */
    size_t hash_value(break_iteration const& value);

}}}  // namespace puppet::runtime::values
