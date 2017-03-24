/**
 * @file
 * Declares the yield return runtime value.
 */
#pragma once

#include "wrapper.hpp"
#include "forward.hpp"
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
     * Represents the yield return runtime value.
     */
    struct yield_return
    {
        /**
         * Constructs a yield return value from a next statement.
         * @param statement The next statement.
         * @param value The value being returned.
         * @param frames The evaluation frames when the next statement was evaluated.
         */
        explicit yield_return(compiler::ast::next_statement const& statement, wrapper<value> value = {}, std::vector<compiler::evaluation::stack_frame> frames = {});

        /**
         * Creates an "illegal next" evaluation exception.
         * @return Returns the evaluation exception.
         */
        puppet::compiler::evaluation_exception create_exception() const;

        /**
         * Unwraps the return value.
         * This will set the contained value to undef.
         * @return Returns the value being returned by the block.
         */
        values::value unwrap();

     private:
        std::shared_ptr<compiler::ast::syntax_tree> _tree;
        compiler::ast::context _context;
        wrapper<value> _value;
        std::vector<compiler::evaluation::stack_frame> _frames;
    };

    /**
     * Stream insertion operator for yield return.
     * @param os The output stream to write the value to.
     * @param value The yield return value to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, yield_return const& value);

    /**
     * Equality operator for yield return.
     * @param left The left yield return to compare.
     * @param right The right yield return to compare.
     * @return Returns true if the two values are equal or false if not.
     */
    bool operator==(yield_return const& left, yield_return const& right);

    /**
     * Inequality operator for yield return.
     * @param left The left yield return to compare.
     * @param right The right yield return to compare.
     * @return Returns true if the two values are not equal or false if they are equal.
     */
    bool operator!=(yield_return const& left, yield_return const& right);

    /**
     * Hashes the yield return value.
     * @param value The yield return to hash.
     * @return Returns the hash value for the value.
     */
    size_t hash_value(yield_return const& value);

}}}  // namespace puppet::runtime::values
