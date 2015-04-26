/**
 * @file
 * Declares the type runtime value.
 */
#pragma once

#include "../types/any.hpp"
#include "../types/array.hpp"
#include "../types/boolean.hpp"
#include "../types/callable.hpp"
#include "../types/data.hpp"
#include "../types/defaulted.hpp"
#include "../types/floating.hpp"
#include "../types/hash.hpp"
#include "../types/integer.hpp"
#include "../types/numeric.hpp"
#include "../types/regexp.hpp"
#include "../types/scalar.hpp"
#include "../types/string.hpp"
#include "../types/struct.hpp"
#include "../types/tuple.hpp"
#include "../types/type.hpp"
#include "../types/undef.hpp"

namespace puppet { namespace runtime { namespace values {

    /**
     * Represents a runtime type.
     */
    typedef boost::make_recursive_variant<
        types::any,
        types::basic_array<boost::recursive_variant_>,
        types::basic_hash<boost::recursive_variant_>,
        types::basic_struct<boost::recursive_variant_>,
        types::basic_tuple<boost::recursive_variant_>,
        types::basic_type<boost::recursive_variant_>,
        types::boolean,
        types::callable,
        types::data,
        types::defaulted,
        types::floating,
        types::integer,
        types::numeric,
        types::regexp,
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
     * Represents the Hash type.
     */
    typedef basic_hash<values::type> hash;

    /**
     * Represents the Tuple type.
     */
    typedef basic_tuple<values::type> tuple;

    /**
     * Represents the Type type.
     */
    typedef basic_type<values::type> type;

    /**
     * Represents the Struct type.
     */
    typedef basic_struct<values::type> structure;

}}}  // namespace puppet::runtime::types