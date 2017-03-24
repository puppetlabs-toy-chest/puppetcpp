/**
 * @file
 * Declares the "return value" runtime value.
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
     * Represents the "return value" runtime value.
     */
    struct return_value
    {
        /**
         * Constructs a "return value" from a return statement.
         * @param statement The return statement.
         * @param value The value being returned.
         * @param frames The evaluation frames when the return statement was evaluated.
         */
        explicit return_value(compiler::ast::return_statement const& statement, wrapper<value> value = {}, std::vector<compiler::evaluation::stack_frame> frames = {});

        /**
         * Creates an "illegal return" evaluation exception.
         * @return Returns the evaluation exception.
         */
        puppet::compiler::evaluation_exception create_exception() const;

        /**
         * Unwraps the return value.
         * This will set the contained value to undef.
         * @return Returns the value being returned.
         */
        values::value unwrap();

     private:
        std::shared_ptr<compiler::ast::syntax_tree> _tree;
        compiler::ast::context _context;
        wrapper<value> _value;
        std::vector<compiler::evaluation::stack_frame> _frames;
    };

    /**
     * Stream insertion operator for return value.
     * @param os The output stream to write the value to.
     * @param value The return value to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, return_value const& value);

    /**
     * Equality operator for return value.
     * @param left The left return value to compare.
     * @param right The right return value to compare.
     * @return Returns true if the two values are equal or false if not.
     */
    bool operator==(return_value const& left, return_value const& right);

    /**
     * Inequality operator for return value.
     * @param left The left return value to compare.
     * @param right The right return value to compare.
     * @return Returns true if the two values are not equal or false if they are equal.
     */
    bool operator!=(return_value const& left, return_value const& right);

    /**
     * Hashes the return value.
     * @param value The return value to hash.
     * @return Returns the hash value for the return value.
     */
    size_t hash_value(return_value const& value);

}}}  // namespace puppet::runtime::values
