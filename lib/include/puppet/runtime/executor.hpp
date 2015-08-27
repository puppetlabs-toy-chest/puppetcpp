/**
 * @file
 * Declares the runtime executor.
 */
#pragma once

#include "values/value.hpp"
#include "expression_evaluator.hpp"
#include <boost/optional.hpp>
#include <vector>
#include <unordered_map>
#include <string>
#include <exception>

namespace puppet { namespace runtime {

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
     * Represents the runtime executor.
     */
    struct executor
    {
        /**
         * Constructs a runtime executor.
         * @param evaluator The current expression evaluator.
         * @param position The default position of the expression being executed.
         * @param parameters The parameters of the expression.
         * @param body The body of the expression.
         */
        executor(expression_evaluator& evaluator, lexer::position const& position, boost::optional<std::vector<ast::parameter>> const& parameters, boost::optional<std::vector<ast::expression>> const& body);

        /**
         * Gets the default position of the expression being executed.
         * @return Returns the default position of the expression being executed.
         */
        lexer::position const& position() const;

        /**
         * Gets the position of a parameter.
         * @param index The index of the parameter.
         * @return Returns the position of the parameter.
         */
        lexer::position const& position(size_t index) const;

        /**
         * Gets the count of parameters.
         * @return Returns the count of parameters.
         */
        size_t parameter_count() const;

        /**
         * Executes the expression.
         * @param scope The scope to execute under or nullptr to use an ephemeral scope.
         * @return Returns the value that was returned from the body.
         */
        values::value execute(std::shared_ptr<runtime::scope> const& scope = nullptr) const;

        /**
         * Executes the expression with the given positional arguments.
         * @param arguments The positional arguments to set in the scope.
         * @param scope The scope to execute under or nullptr to use an ephemeral scope.
         * @return Returns the value that was returned from the body.
         */
        values::value execute(values::array& arguments, std::shared_ptr<runtime::scope> const& scope = nullptr) const;

        /**
         * Executes the expression with the given resource's attributes.
         * @param resource The resource attributes to set in the scope.
         * @param scope The scope to execute under or nullptr to use an ephemeral scope.
         * @return Returns the value that was returned from the body.
         */
        values::value execute(runtime::resource& resource, std::shared_ptr<runtime::scope> const& scope = nullptr) const;

     private:
        void validate_type(ast::parameter const& parameter, values::value const& value, std::function<void(std::string)> const& type_error) const;
        values::value evaluate_body() const;

        expression_evaluator& _evaluator;
        lexer::position const& _position;
        boost::optional<std::vector<ast::parameter>> const& _parameters;
        boost::optional<std::vector<ast::expression>> const& _body;
    };

}}  // puppet::runtime
