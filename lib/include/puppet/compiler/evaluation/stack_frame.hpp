/**
 * @file
 * Declares the evaluation stack frame.
 */
#pragma once

#include "../ast/ast.hpp"
#include <boost/variant.hpp>
#include <memory>

namespace puppet { namespace compiler { namespace evaluation {

    // Forward declaration of scope
    struct scope;

    /**
     * Represents a Puppet stack frame.
     */
    struct stack_frame
    {
        /**
         * Represents the different types of Puppet statements/expressions that can be on the call stack.
         */
        using expression_type = boost::variant<
            ast::function_statement const*,
            ast::class_statement const*,
            ast::defined_type_statement const*,
            ast::node_statement const*,
            ast::collector_expression const*,
            ast::type_alias_statement const*
        >;

        /**
         * Constructs a stack frame for a native function.
         * @param name The name of the function.
         * @param scope The associated scope.
         * @param external True if the frame is external (not Puppet) or false if the frame is Puppet.
         */
        explicit stack_frame(char const* name, std::shared_ptr<evaluation::scope> scope, bool external = true);

        /**
         * Constructs a stack frame for the given expression.
         * @param expression The expression associated with the stack frame.
         * @param scope The associated scope.
         */
        explicit stack_frame(expression_type expression, std::shared_ptr<evaluation::scope> scope);

        /**
         * Gets the name of frame.
         * @return Returns the name of the frame.
         */
        std::string name() const;

        /**
         * Gets whether or not the frame is external (not Puppet).
         * @return Returns true if the frame is external (not Puppet) or false if the frame represents Puppet code.
         */
        bool external() const;

        /**
         * Gets the expression related to the frame.
         * @tparam The expression type (e.g. ast::function_expression, ast::class_expression, etc.).
         * @return Returns the pointer to the requested expression or nullptr if the frame is not associated with an expression of the given type.
         */
        template <typename T>
        T const* as() const
        {
            auto ptr = boost::get<T const*>(&_expression);
            if (ptr) {
                return *ptr;
            }
            return nullptr;
        }

        /**
         * Gets the scope of the stack frame.
         * @return Returns the scope of the stack frame.
         */
        std::shared_ptr<evaluation::scope> const& scope() const;

        /**
         * Gets the current AST context (i.e. context of currently evaluating expression).
         * @return Returns the current context.
         */
        ast::context const& current() const;

        /**
         * Sets the current AST context (i.e. context of currently evaluating expression).
         * @param value The new AST context.
         */
        void current(ast::context value);

     private:
        static ast::context expression_context(expression_type const& expression);

        char const* _name;
        expression_type _expression;
        std::shared_ptr<evaluation::scope> _scope;
        ast::context _current;
        bool _external;
    };

    /**
     * Stream insertion operator for stack frame.
     * @param os The stream to write the stack frame to.
     * @param frame The frame to write.
     * @return Returns the given stream.
     */
    std::ostream& operator<<(std::ostream& os, stack_frame const& frame);

}}}  // namespace puppet::compiler::evaluation
