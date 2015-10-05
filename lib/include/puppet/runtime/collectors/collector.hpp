/**
 * @file
 * Declares the base collector.
 */
#pragma once

#include "../../ast/syntax_tree.hpp"
#include <ostream>
#include <vector>
#include <memory>

namespace puppet { namespace runtime {

    // Forward declaration of resource.
    struct resource;

    // Forward declaration of catalog.
    struct catalog;

    // Forward declaration of context.
    struct context;

    // Forward declaration of attribute.
    struct attribute;

    // Using attributes
    using attributes = std::vector<std::pair<ast::attribute_operator, std::shared_ptr<attribute>>>;

}}  // namespace puppet::runtime

namespace puppet { namespace runtime { namespace collectors {

    /**
     * Represents the base collector.
     */
    struct collector
    {
        /**
         * Collects the resources.
         * @param context The current evaluation context.
         */
        virtual void collect(runtime::context& context) = 0;

        /**
         * Detects uncollected resources.
         * Throws an evaluation exception if there are any uncollected resources.
         */
        virtual void detect_uncollected() const;

        /**
         * Gets the resources that have been collected by this collector.
         * @return Returns the resources that have been collected by this collector.
         */
        std::vector<resource*> const& resources() const;

        /**
         * Sets the attributes to override when the collector collects a resource.
         * @param attributes The attributes to override.
         */
        void attributes(runtime::attributes attributes);

     protected:
        /**
         * Collects the given resource.
         * @param catalog The current catalog.
         * @param resource The resource to collect.
         * @param check True to check if the resource is already in the list or false to always add the resource.
         */
        void collect_resource(runtime::catalog& catalog, runtime::resource& resource, bool check = true);

     private:
        std::vector<resource*> _resources;
        runtime::attributes _attributes;
    };

}}}  // namespace puppet::runtime:collectors
