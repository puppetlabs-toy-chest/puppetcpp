/**
 * @file
 * Declares the type runtime value.
 */
#pragma once

#include "../types/any.hpp"
#include "../types/array.hpp"
#include "../types/boolean.hpp"
#include "../types/callable.hpp"
#include "../types/catalog_entry.hpp"
#include "../types/class.hpp"
#include "../types/collection.hpp"
#include "../types/data.hpp"
#include "../types/defaulted.hpp"
#include "../types/enumeration.hpp"
#include "../types/floating.hpp"
#include "../types/hash.hpp"
#include "../types/integer.hpp"
#include "../types/numeric.hpp"
#include "../types/optional.hpp"
#include "../types/pattern.hpp"
#include "../types/regexp.hpp"
#include "../types/resource.hpp"
#include "../types/runtime.hpp"
#include "../types/scalar.hpp"
#include "../types/string.hpp"
#include "../types/struct.hpp"
#include "../types/tuple.hpp"
#include "../types/type.hpp"
#include "../types/undef.hpp"
#include "../types/variant.hpp"

namespace puppet { namespace runtime { namespace values {

    /**
     * Represents a runtime type.
     */
    typedef boost::make_recursive_variant<
        types::any,
        types::basic_array<boost::recursive_variant_>,
        types::basic_class<boost::recursive_variant_>,
        types::basic_catalog_entry<boost::recursive_variant_>,
        types::basic_hash<boost::recursive_variant_>,
        types::basic_optional<boost::recursive_variant_>,
        types::basic_resource<boost::recursive_variant_>,
        types::basic_struct<boost::recursive_variant_>,
        types::basic_tuple<boost::recursive_variant_>,
        types::basic_type<boost::recursive_variant_>,
        types::basic_variant<boost::recursive_variant_>,
        types::boolean,
        types::callable,
        types::collection,
        types::data,
        types::defaulted,
        types::enumeration,
        types::floating,
        types::integer,
        types::numeric,
        types::pattern,
        types::regexp,
        types::runtime,
        types::scalar,
        types::string,
        types::undef
    >::type type;

}}}  // puppet::runtime::values

// Now that type has been defined, typedef the dependent types
namespace puppet { namespace runtime { namespace types {

    /**
     * Represents the Array type.
     */
    typedef basic_array<values::type> array;

    /**
     * Represents the CatalogEntry type.
     */
    typedef basic_catalog_entry<values::type> catalog_entry;

    /**
     * Represents the Class type.
     */
    typedef basic_class<values::type> klass;

    /**
     * Represents the Hash type.
     */
    typedef basic_hash<values::type> hash;

    /**
     * Represents the Optional type.
     */
    typedef basic_optional<values::type> optional;

    /**
     * Represents the Resource type.
     */
    typedef basic_resource<values::type> resource;

    /**
     * Represents the Struct type.
     */
    typedef basic_struct<values::type> structure;

    /**
     * Represents the Tuple type.
     */
    typedef basic_tuple<values::type> tuple;

    /**
     * Represents the Type type.
     */
    typedef basic_type<values::type> type;

    /**
     * Represents the Variant type.
     */
    typedef basic_variant<values::type> variant;

}}}  // namespace puppet::runtime::types