/**
 * @file
 * Declares the position type.
 */
#pragma once

#include <cstddef>
#include <ostream>

namespace puppet { namespace compiler { namespace lexer {

    /**
     * Represents a position within a lexed input.
     */
    struct position
    {
        /**
         * Default constructor for position.
         */
        position();

        /**
         * Constructs a position with the given offset and line.
         * @param offset The 0-based offset of the position.
         * @param line The 1-based line of the position.
         */
        position(size_t offset, size_t line);

        /**
         * Gets the 0-based offset of the position.
         * @return Returns the 0-based offset of the position.
         */
        size_t offset() const;

        /**
         * Gets the 1-based line of the position.
         * @return Returns the 1-based line of the position.
         */
        size_t line() const;

        /**
         * Increments the position.
         * @param newline Increments the line number if true or not if false.
         */
        void increment(bool newline);

    private:
        size_t _offset;
        size_t _line;
    };

    /**
     * Stream insertion operator for position.
     * @param os The output stream to write the position to.
     * @param position The position to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, lexer::position const& position);

    /**
     * Equality operator for position.
     * @param left The left operand.
     * @param right The right operand.
     * @return Returns true of the two positions are equal or false if not.
     */
    bool operator==(position const& left, position const& right);

    /**
     * Inequality operator for position.
     * @param left The left operand.
     * @param right The right operand.
     * @return Returns true of the two positions are not equal or false if they are equal.
     */
    bool operator!=(position const& left, position const& right);

}}}  // namespace puppet::compiler::lexer
