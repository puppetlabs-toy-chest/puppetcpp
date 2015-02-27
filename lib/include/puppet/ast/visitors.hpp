/**
 * @file
 * Declares AST vistor functors.
 */
#pragma once

#include "../lexer/token_position.hpp"
#include <boost/variant.hpp>
#include <iostream>

namespace puppet { namespace ast {

    /**
     * Implements a visitor for retrieving the position from AST elements.
     */
    struct position_visitor : boost::static_visitor<lexer::token_position const&>
    {
        /**
         * Called when the element being visited is blank.
         * @return Returns the default position.
         */
        result_type operator()(boost::blank const&) const
        {
            return default_position;
        }

        /**
         * Called to visit an AST element.
         * @param element The element being visited.
         * @return Returns the position of the element.
         */
        template <typename T>
        result_type operator()(T const& element) const
        {
            return element.position();
        }

     private:
        static lexer::token_position default_position;
    };

    /**
     * Implements a visitor for printing AST elements.
     */
    struct insertion_visitor : boost::static_visitor<std::ostream&>
    {
        /**
         * Constructs an insertion visitor with the given output stream.
         * @param os The output stream to write the elements to.
         */
        explicit insertion_visitor(std::ostream& os) :
            _os(os)
        {
        }

        /**
         * Called when the element being visited is blank.
         * @return Returns the original output stream.
         */
        result_type operator()(boost::blank const&) const
        {
            return _os;
        }

        /**
         * Called to visit an AST element.
         * @param element The element being visited.
         * @return Returns the original output stream.
         */
        template <typename T>
        result_type operator()(T const& element) const
        {
            return (_os << element);
        }

     private:
        std::ostream& _os;
    };

}}  // namespace puppet::ast
