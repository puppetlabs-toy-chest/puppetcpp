/**
 * @file
 * Declares the token position type.
 */
#pragma once

#include <tuple>

namespace puppet { namespace lexer {

    /**
     * Stores the offset in input stream and line for a token.
     */
    typedef std::tuple<std::size_t, std::size_t> token_position;

}}  // namespace puppet::lexer
