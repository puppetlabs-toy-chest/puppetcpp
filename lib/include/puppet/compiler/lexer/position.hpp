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
         * @param newline Increments the line position if true or not if false.
         */
        void increment(bool newline);

    private:
        size_t _offset;
        size_t _line;
    };

    /**
     * Represents a range within a lexed input.
     */
    struct range
    {
        /**
         * Default constructor for range.
         */
        range() = default;

        /**
         * Constructs a range with the given begin and end positions.
         * @param begin The beginning position of the range.
         * @param end The ending position of the range.
         */
        range(position begin, position end);

        /**
         * Constructs a range with the given beginning position and length.
         * @param begin The beginning position of the range.
         * @param length The length of the range.
         */
        range(position begin, size_t length);

        /**
         * Gets the beginning position of the range.
         * @return Returns the beginning position of the range.
         */
        position const& begin() const;

        /**
         * Sets the beginning position of the range.
         * @param begin The beginning position of the range.
         */
        void begin(position begin);

        /**
         * Gets the ending position of the range.
         * @return Returns the ending position of the range.
         */
        position const& end() const;

        /**
         * Sets the ending position of the range.
         * @param end The ending position of the range.
         */
        void end(position end);

        /**
         * Gets the length of the range, in bytes.
         * @return Returns the length of the range, in bytes.
         */
        size_t length() const;

     private:
        position _begin;
        position _end;
    };

    /**
     * Stream insertion operator for position.
     * @param os The output stream to write the position to.
     * @param position The position to write.
     * @return Returns the given output stream.
     */
    std::ostream& operator<<(std::ostream& os, lexer::position const& position);

}}}  // namespace puppet::compiler::lexer
