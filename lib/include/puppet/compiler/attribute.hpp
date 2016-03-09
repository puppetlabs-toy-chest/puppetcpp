/**
 * @file
 * Declares the resource attribute.
 */
#pragma once

#include "ast/ast.hpp"
#include "../runtime/values/value.hpp"
#include <string>
#include <memory>

namespace puppet { namespace compiler {

    /**
     * Represents a resource attribute.
     */
    struct attribute
    {
        /**
         * Constructs a resource attribute.
         * @param name The name of the attribute.
         * @param name_context The AST context of the name.
         * @param value The attribute's value.
         * @param value_context The AST context of the value.
         */
        attribute(std::string name, ast::context name_context, std::shared_ptr<runtime::values::value> value, ast::context value_context);

        /**
         * Gets the name of the attribute.
         * @return Returns the name of the attribute.
         */
        std::string const& name() const;

        /**
         * Gets the AST context of the name.
         * @return Returns the AST context of the name.
         */
        ast::context const& name_context() const;

        /**
         * Gets the attribute's value.
         * @return Returns the attribute's value.
         */
        runtime::values::value& value();

        /**
         * Gets the attribute's value.
         * @return Returns the attribute's value.
         */
        runtime::values::value const& value() const;

        /**
         * Gets the attribute's shared value.
         * @return Returns the attribute's shared value.
         */
        std::shared_ptr<runtime::values::value> shared_value();

        /**
         * Gets the attribute's shared value.
         * @return Returns the attribute's shared value.
         */
        std::shared_ptr<runtime::values::value const> shared_value() const;

        /**
         * Gets the AST context of the value.
         * @return Returns the AST context of the value.
         */
        ast::context const& value_context() const;

        /**
         * Determines if any other attribute is referring to the same value as this one.
         * @return Returns true if the value is unique or false if it is shared.
         */
        bool unique() const;

     private:
        std::shared_ptr<ast::syntax_tree> _tree;
        std::string _name;
        ast::context _name_context;
        std::shared_ptr<runtime::values::value> _value;
        ast::context _value_context;
    };

    /**
     * Represents a list of attributes paired with the attribute operator.
     */
    using attributes = std::vector<std::pair<ast::attribute_operator, std::shared_ptr<attribute>>>;

    /**
     * Utility class for attribute_set.
     */
    struct attribute_set_less
    {
        /**
         * Determines if one indirected attribute is "less than" another.
         * @param left The left attribute to compare.
         * @param right The right attribute to compare.
         * @return Returns true if the left is less than the right or false if not.
         */
        bool operator()(attribute const* left, attribute const* right) const;
    };

    /**
     * Represents a set of attribute pointers.
     */
    using attribute_set = std::set<attribute const*, attribute_set_less>;

}}  // namespace puppet::compiler
