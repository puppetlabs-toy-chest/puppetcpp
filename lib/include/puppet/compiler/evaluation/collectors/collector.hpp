/**
 * @file
 * Declares the base collector.
 */
#pragma once

#include "../../resource.hpp"
#include <ostream>
#include <vector>
#include <memory>

namespace puppet { namespace compiler { namespace evaluation {

    // Forward declaration of evaluation context
    struct context;

}}}  // namespace puppet::compiler::evaluation

namespace puppet { namespace compiler { namespace evaluation { namespace collectors {

    /**
     * Represents the base collector.
     */
    struct collector
    {
        /**
         * Collects the resources.
         * @param context The current evaluation context.
         */
        virtual void collect(evaluation::context& context) = 0;

        /**
         * Detects uncollected resources.
         * @param context The current evaluation context.
         * Throws an evaluation exception if there are any uncollected resources.
         */
        virtual void detect_uncollected(evaluation::context& context) const;

        /**
         * Gets the resources that have been collected by this collector.
         * @return Returns the resources that have been collected by this collector.
         */
        std::vector<resource*> const& resources() const;

        /**
         * Sets the attributes to apply when a resource is collected by the collector.
         * @param attributes The attributes to apply.
         */
        void attributes(compiler::attributes attributes);

     protected:
        /**
         * Collects the given resource.
         * @param context The current evaluation context.
         * @param resource The resource to collect.
         * @param check True to check if the resource is already in the list or false to always add the resource.
         */
        void collect_resource(evaluation::context& context, compiler::resource& resource, bool check = true);

     private:
        std::vector<resource*> _resources;
        compiler::attributes _attributes;
    };

}}}}  // namespace puppet::compiler::evaluation::collectors
