/**
 * @file
 * Declares the definition visitor.
 */
#pragma once

#include "../ast.hpp"
#include <boost/variant.hpp>
#include <functional>
#include <string>
#include <vector>

namespace puppet { namespace compiler { namespace ast { namespace visitors {

    /**
     * A visitor for finding definition statements in an AST.
     * This visitor only visits top-level and class-nested definition statements.
     */
    struct definition : boost::static_visitor<void>
    {
        /**
         * A variant of all definition statements.
         */
        using statement = boost::variant<
            class_statement const*,
            defined_type_statement const*,
            node_statement const*,
            type_alias_statement const*,
            function_statement const*,
            produces_statement const*,
            consumes_statement const*,
            application_statement const*,
            site_statement const*
        >;

        /**
         * The callback function type.
         * For classes and defined types, the first parameter is the qualified name.
         * The second parameter is a variant containing a pointer to the definition statement.
         */
        using callback_type = std::function<void(std::string, statement const&)>;

        /**
         * Constructs a definition visitor.
         * @param callback The callback to use for each definition found in the AST.
         */
        explicit definition(callback_type callback);

        /**
         * Visits the given AST.
         * This requires that the syntax tree has been validated.
         * Only top-level and class statements are visited.
         * @param tree The tree to visit.
         */
        void visit(syntax_tree const& tree);

     private:
        template<class> friend class ::boost::detail::variant::invoke_visitor;
        void operator()(ast::statement const& statement);
        void operator()(class_statement const& statement);
        void operator()(defined_type_statement const& statement);
        void operator()(node_statement const& statement);
        void operator()(function_statement const& statement);
        void operator()(produces_statement const& statement);
        void operator()(consumes_statement const& statement);
        void operator()(application_statement const& statement);
        void operator()(site_statement const& statement);
        void operator()(type_alias_statement const& statement);
        void operator()(function_call_statement const& statement);
        void operator()(relationship_statement const& statement);
        void operator()(break_statement const& statement);
        void operator()(next_statement const& statement);
        void operator()(return_statement const& statement);

        std::string qualify(std::string const& name) const;

        struct class_helper
        {
            class_helper(definition& visitor, class_statement const& statement);
            ~class_helper();

         private:
            definition& _visitor;
        };

        std::vector<class_statement const*> _classes;
        callback_type _callback;
    };

}}}}  // namespace puppet::compiler::visitors
