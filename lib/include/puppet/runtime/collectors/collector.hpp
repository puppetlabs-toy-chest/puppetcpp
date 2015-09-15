/**
 * @file
 * Declares the base collector.
 */
#pragma once

#include "../../compiler/context.hpp"

namespace puppet { namespace runtime {

    // Forward declaration of resource.
    struct resource;

    // Forward declaration of catalog.
    struct catalog;

}}  // namespace puppet::runtime

namespace puppet { namespace runtime { namespace collectors {

    /**
     * Represents the base collector.
     */
    struct collector
    {
        /**
         * Collects the resources.
         * @param catalog The catalog to collect resources from.
         */
        virtual void collect(runtime::catalog& catalog) = 0;

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

     protected:
        /**
         * Stores the resources that were collected by this collector.
         */
        std::vector<resource*> _resources;
    };

}}}  // namespace puppet::runtime:collectors
