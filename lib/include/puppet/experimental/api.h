/**
 * @file
 * Declares the experimental C API.
 */
#pragma once

#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represents data for a UTF-8 encoded string.
 */
struct puppet_utf8_string
{
    /**
     * The size of the string, in bytes.
     */
    uint64_t size;
    /**
     * The pointer to the string's UTF-8 encoded data bytes.
     */
    char const* bytes;
};

/**
 * Represents the possible Puppet log levels.
 */
enum puppet_log_level {
    /**
     * The debug log level.
     */
    PUPPET_DEBUG_LEVEL,
    /**
     * The info log level.
     */
    PUPPET_INFO_LEVEL,
    /**
     * The notice log level.
     */
    PUPPET_NOTICE_LEVEL,
    /**
     * The warning log level.
     */
    PUPPET_WARNING_LEVEL,
    /**
     * The error log level.
     */
    PUPPET_ERROR_LEVEL,
    /**
     * The alert log level.
     */
    PUPPET_ALERT_LEVEL,
    /**
     * The emergency log level.
     */
    PUPPET_EMERGENCY_LEVEL,
    /**
     * The critical log level.
     */
    PUPPET_CRITICAL_LEVEL
};

/**
 * Represents a log entry.
 * Log entries do not need to be freed; string data should be copied if stored.
 */
struct puppet_log_entry
{
    /**
     * The log level.
     */
    enum puppet_log_level level;
    /**
     * The line number of the message.
     */
    uint64_t line;
    /**
     * The column number of the message.
     */
    uint64_t column;
    /**
     * The span (length of relevant portion of the source file) of the message.
     */
    uint64_t span;
    /**
     * The source text of the message.
     */
    puppet_utf8_string text;
    /**
     * The path to the source file of the message.
     */
    puppet_utf8_string path;
    /**
     * The message that was logged.
     */
    puppet_utf8_string message;
};

/**
 * Represents a source file position.
 */
struct puppet_source_position
{
    /**
     * The line in the file.
     */
    uint64_t line;
    /**
     * The byte offset in the file.
     */
    uint64_t offset;
};

/**
 * Represents an backtrace stack frame.
 */
struct puppet_stack_frame
{
    /**
     * The name of the function associated with the stack frame.
     */
    puppet_utf8_string name;
    /**
     * The path to the source file.
     */
    puppet_utf8_string path;
    /**
     * The beginning position of the source context.
     */
    struct puppet_source_position begin;
    /**
     * The ending position of the source context.
     */
    struct puppet_source_position end;
};

/**
 * Represents Puppet exception data
 */
struct puppet_exception_data
{
    /**
     * The null-terminated exception message (UTF-8).
     */
    char const* message;
    /**
     * The line number where the exception occurred.
     */
    uint64_t line;
    /**
     * The column number where the exception occurred.
     */
    uint64_t column;
    /**
     * The span (length of relevant portion of the source file) of the exception.
     */
    uint64_t span;
    /**
     * The source text of the exception.
     */
    puppet_utf8_string text;
    /**
     * The path to the source file of the exception.
     */
    puppet_utf8_string path;
    /**
     * The number of stack frames for the exception.
     */
    uint32_t frame_count;
    /**
     * The stack frames for the exception.
     */
    struct puppet_stack_frame const* frames;
};

// Forward declaration of opaque value type
struct puppet_value;

// Forward declaration of opaque exception type
struct puppet_exception;

/**
 * Represents an evaluation result.
 */
struct puppet_evaluation_result
{
    /**
     * The evaluation result (if evaluation was successful).
     * Use `puppet_free_value` to free the value.
     */
    struct puppet_value* value;
    /**
     * The evaluation exception (if evaluation was unsuccessful).
     * Use `puppet_free_exception` to free the exception.
     */
    struct puppet_exception* exception;
};

/**
 * Represents the possible kinds of runtime values.
 */
enum puppet_value_kind
{
    /**
     * The undef value kind.
     */
    PUPPET_VALUE_UNDEF,
    /**
     * The default value kind.
     */
    PUPPET_VALUE_DEFAULT,
    /**
     * The integer value kind.
     */
    PUPPET_VALUE_INTEGER,
    /**
     * The float value kind.
     */
    PUPPET_VALUE_FLOAT,
    /**
     * The boolean value kind.
     */
    PUPPET_VALUE_BOOLEAN,
    /**
     * The string value kind.
     */
    PUPPET_VALUE_STRING,
    /**
     * The regexp value kind.
     */
    PUPPET_VALUE_REGEXP,
    /**
     * The type value kind.
     */
    PUPPET_VALUE_TYPE,
    /**
     * The array value kind.
     */
    PUPPET_VALUE_ARRAY,
    /**
     * The hash value kind.
     */
    PUPPET_VALUE_HASH,
    /**
     * The sequence iterator (i.e. array) value kind.
     */
    PUPPET_VALUE_SEQUENCE_ITERATOR,
    /**
     * The key-value iterator (i.e. hash) value kind.
     */
    PUPPET_VALUE_KEY_VALUE_ITERATOR
};

/**
 * Represents an element of a hash value.
 */
struct puppet_hash_element
{
    /**
     * The key of a hash element.
     */
    struct puppet_value const* key;
    /**
     * The value of a hash element.
     */
    struct puppet_value const* value;
};

/**
 * Represents an element of an iterator.
 */
struct puppet_iterator_element
{
    /**
     * The iterator element key; if the iterator is not for a hash, this will be nullptr.
     */
    struct puppet_value const* key;
    /**
     * The iterator element value.
     */
    struct puppet_value const* value;
};

/**
 * Represents a Puppet compiler session.
 */
struct puppet_compiler_session;

/**
 * Represents context for a Puppet function call.
 */
struct puppet_call_context;

/**
 * Represents data about the caller.
 */
struct puppet_caller_data
{
    /**
     * The caller's file path.
     */
    puppet_utf8_string path;
    /**
     * The line number of the caller.
     */
    uint64_t line;
};

/**
 * Represents Puppet function dispatch information.
 */
struct puppet_function_dispatch
{
    /**
     * The null-terminated Puppet dispatch specification (e.g. "Callable[Integer]") (UTF-8).
     */
    char const* specification;
    /**
     * The user defined data to pass to the callback.
     */
    void* data;
    /**
     * The callback to invoke when the function is dispatched.
     */
    struct puppet_evaluation_result (*callback)(struct puppet_call_context*, void*, struct puppet_value const** arguments, uint64_t count);
};

/**
 * Creates a new compiler session.
 * Currently the compiler will use the default compilation settings for the session.
 * @param name The null-terminated name of the compilation node (and environment) to use (UTF-8).
 * @param directory The null-terminated directory for the environment being compiled (UTF-8).
 * @param level The minimum logging level to use.
 * @param callback The logging callback to use for the session. If nullptr, no messages will be logged.
 * @return Returns the compiler session or nullptr if the session could not be created.
 */
struct puppet_compiler_session* puppet_create_session(char const* name, char const* directory, enum puppet_log_level level, void (*callback)(struct puppet_log_entry const*));

/**
 * Defines a Puppet function for the given compiler session.
 * @param session The compiler session to define a function for.
 * @param name The null-terminated name of the function being defined (UTF-8).
 * @param dispatches The dispatches for the function.
 * @param count The count of dispatches for the function.
 * @return Returns non-zero if the function is successfully defined or false if the function cannot be defined (already exists, invalid dispatch specification, etc).
 */
int puppet_define_function(struct puppet_compiler_session* session, char const* name, struct puppet_function_dispatch const* dispatches, uint64_t count);

/**
 * Determines if a block was passed to a Puppet function invocation.
 * @param context The function call context.
 * @return Returns non-zero if a block was passed or zero if a block was not passed.
 */
int puppet_block_passed(struct puppet_call_context* context);

/**
 * Gets the data for the caller.
 * Note: the data is only valid for the current function call.
 * @param context The current call context.
 * @param data The caller data to populate.
 * @return Returns none-zero if the caller data was retrieved or 0 if there is no caller data.
 */
int puppet_get_caller_data(struct puppet_call_context const* context, struct puppet_caller_data* data);

/**
 * Yields to the function's block.
 * The yield will take ownership of the passed in values; the arguments do not need to be freed.
 * @param context The function call context.
 * @param arguments The arguments to pass to the block.
 * @param count The count of arguments being passed to the block.
 * @return Returns the result of yielding to the block.
 */
struct puppet_evaluation_result puppet_yield(struct puppet_call_context* context, struct puppet_value** arguments, uint64_t count);

/**
 * Frees the compiler session.
 * @param session The compiler session to delete.
 */
void puppet_free_session(struct puppet_compiler_session* session);

/**
 * Evaluates the given source file.
 * Evaluation occurs in isolation: variables assigned in one file are not visible to subsequent files.
 * Free the returned value upon successful evaluation.
 * Free the returned exception if evaluation was unsuccessful.
 * @param session The compiler session to use.
 * @param path The null-terminated path to the file to evaluate (UTF-8).
 * @return Returns the evaluation result.
 */
struct puppet_evaluation_result puppet_evaluate_file(struct puppet_compiler_session* session, char const* path);

/**
 * Creates a new Puppet exception.
 * @param message The null-terminated exception message (UTF-8).
 * @return Returns the new Puppet exception or nullptr if the exception failed to allocate.
 */
struct puppet_exception* puppet_create_exception(char const* message);

/**
 * Creates a new Puppet exception with backtrace and source context.
 * @param message The null-terminated exception message (UTF-8).
 * @param context The call context to use; if nullptr, no backtrace or source code context will be provided.
 * @return Returns the new Puppet exception or nullptr if the exception failed to allocate.
 */
struct puppet_exception* puppet_create_exception_with_context(char const* message, struct puppet_call_context const* context);

/**
 * Gets the data of the given Puppet exception.
 * The returned data is valid until the exception is freed.
 * @param exception The exception to get the data for.
 * @param data The returned exception data.
 * @return Returns non-zero if the exception data was successfully returned or zero if not.
 */
int puppet_get_exception_data(struct puppet_exception const* exception, struct puppet_exception_data* data);

/**
 * Frees the given exception.
 * @param exception The exception to free.
 */
void puppet_free_exception(struct puppet_exception* exception);

/**
 * Creates a new Puppet value.
 * The value will initially be set to undef.
 * Values should be freed using `puppet_free_value`.
 * @return Returns the new Puppet value.
 */
struct puppet_value* puppet_create_value();

/**
 * Clones a Puppet value.
 * @param value The value to clone.
 * @return Returns the cloned value; use `puppet_free_value` to free the returned value.
 */
struct puppet_value* puppet_value_clone(struct puppet_value const* value);

/**
 * Frees a Puppet value.
 * @param value The value to free.
 */
void puppet_free_value(struct puppet_value* value);

/**
 * Gets the kind of the given Puppet value.
 * @param value The value to get the kind for.
 * @param kind The returned value kind.
 * @return Returns non-zero if the kind was successfully returned or zero if not.
 */
int puppet_get_value_kind(struct puppet_value const* value, enum puppet_value_kind* kind);

/**
 * Determines if the given value is immutable (i.e. an immutable variable reference).
 * @param value The value to check.
 * @return Returns non-zero if the value is immutable or zero if the value is mutable.
 */
int puppet_is_immutable(struct puppet_value const* value);

/**
 * Sets the given value to undef.
 * @param value The value to set to undef.
 * @return Returns non-zero if the value was set or zero if the value is immutable.
 */
int puppet_set_undef(struct puppet_value* value);

/**
 * Sets the given value to default.
 * @param value The value to set to default.
 * @return Returns non-zero if the value was set or zero if the value is immutable.
 */
int puppet_set_default(struct puppet_value* value);

/**
 * Gets the data for a Puppet integer value.
 * @param value The integer value.
 * @param data The returned integer data.
 * @return Returns non-zero if the data was retrieved or zero if not.
 */
int puppet_get_integer(struct puppet_value const* value, int64_t* data);

/**
 * Sets the value to the given integer.
 * @param value The value to set to the given integer.
 * @param data The integer value.
 * @return Returns non-zero if the value was set or zero if the value is immutable.
 */
int puppet_set_integer(struct puppet_value* value, int64_t data);

/**
 * Gets the data for a Puppet float value.
 * @param value The float value.
 * @param data The returned float data.
 * @return Returns non-zero if the data was retrieved or zero if not.
 */
int puppet_get_float(struct puppet_value const* value, double* data);

/**
 * Sets the value to the given float.
 * @param value The value to set to the given float.
 * @param data The float value.
 * @return Returns non-zero if the value was set or zero if the value is immutable.
 */
int puppet_set_float(struct puppet_value* value, double data);

/**
 * Gets the data for a Puppet boolean value.
 * @param value The boolean value.
 * @param data The returned boolean data.
 * @return Returns non-zero if the data was retrieved or zero if not.
 */
int puppet_get_boolean(struct puppet_value const* value, uint8_t* data);

/**
 * Sets the value to the given boolean.
 * @param value The value to set to the given boolean.
 * @param data The boolean value (non-zero for true, zero for false).
 * @return Returns non-zero if the value was set or zero if the value is immutable.
 */
int puppet_set_boolean(struct puppet_value* value, uint8_t data);

/**
 * Gets the data for a Puppet string value.
 * @param value The string value.
 * @param data The returned string data.
 * @return Returns non-zero if the data was retrieved or zero if not.
 */
int puppet_get_string(struct puppet_value const* value, struct puppet_utf8_string* data);

/**
 * Sets the value to the given string.
 * @param value The value to set to the given string.
 * @param data The string data to set; the data will be copied from.
 * @return Returns non-zero if the value was set or zero if the value is immutable.
 */
int puppet_set_string(struct puppet_value* value, struct puppet_utf8_string const* data);

/**
 * Gets the data for a Puppet regexp value.
 * @param value The regexp value.
 * @param data The returned regex data.
 * @return Returns non-zero if the data was retrieved or zero if not.
 */
int puppet_get_regexp(struct puppet_value const* value, struct puppet_utf8_string* data);

/**
 * Sets the value to a regexp with the given pattern.
 * @param value The value to set to the given regexp.
 * @param data The string data to set; the data will be copied from.
 * @return Returns non-zero if the value was set or zero if the value is immutable or the given pattern is invalid.
 */
int puppet_set_regexp(struct puppet_value* value, struct puppet_utf8_string const* data);

/**
 * Sets the value to a Puppet type.
 * @param value The value to set to a Puppet type.
 * @param specification The null-terminated Puppet type specification (e.g. "String[0, 10]") (UTF-8).
 * @return Returns non-zero if the value was set or zero if the value is immutable  or the given specification is invalid.
 */
int puppet_set_type(struct puppet_value* value, char const* specification);

/**
 * Creates a new array value with the given initial capacity.
 * @param capacity The initial capacity for the array.
 * @return Returns the new array value.
 */
struct puppet_value* puppet_create_array(uint64_t capacity);

/**
 * Gets the size of the given array value.
 * @param value The array value.
 * @param size The returned array size.
 * @return Returns non-zero if the size was retrieved or zero if not.
 */
int puppet_array_size(struct puppet_value const* value, uint64_t* size);

/**
 * Gets the array's elements.
 * The returned elements are only valid until the array is modified.
 * The elements are owned by the array and should not be freed.
 * @param value The array value to get the elements of.
 * @param elements The array of elements to populate.
 * @param count The number of elements to get.
 * @return Returns non-zero if the elements were retrieved or zero if not.
 */
int puppet_array_elements(struct puppet_value const* value, struct puppet_value const** elements, uint64_t count);

/**
 * Gets an element of an array value.
 * The returned element is only valid until the array itself is modified.
 * The value is owned by the array and should not be freed.
 * @param value The array value to get the element of.
 * @param index The index of the element.
 * @param element The returned element.
 * @return Returns non-zero if the element was retrieved or zero if not.
 */
int puppet_array_get(struct puppet_value const* value, uint64_t index, struct puppet_value const** element);

/**
 * Sets an element in a Puppet array value.
 * @param value The array value to set the element for.
 * @param index The index of the element to set.
 * @param element The element to set. The array will take ownership of the element; it does not need to be freed.
 * @return Returns non-zero if the element was set or zero if the array value is immutable.
 */
int puppet_array_set(struct puppet_value* value, uint64_t index, struct puppet_value* element);

/**
 * Pushes a new element onto an array.
 * The array will take ownership of the element; it does not need to be freed.
 * @param value The array value to push onto.
 * @param element The element to push onto the array.
 * @return Returns non-zero if the element was pushed or zero if the array value is immutable.
 */
int puppet_array_push(struct puppet_value* value, struct puppet_value* element);

/**
 * Creates a new hash value.
 * @return Returns the new hash value.
 */
struct puppet_value* puppet_create_hash();

/**
 * Gets the size of the given hash value.
 * @param value The hash value.
 * @param size The returned hash size.
 * @return Returns non-zero if the size was retrieved or zero if not.
 */
int puppet_hash_size(struct puppet_value const* value, uint64_t* size);

/**
 * Gets the hash's elements.
 * The returned elements are only valid until the hash is modified.
 * The elements are owned by the hash and should not be freed.
 * @param value The hash value to get the elements of.
 * @param elements The array of elements to populate.
 * @param count The number of elements to get.
 * @return Returns non-zero if the elements were retrieved or zero if not.
 */
int puppet_hash_elements(struct puppet_value const* value, struct puppet_hash_element* elements, uint64_t count);

/**
 * Gets an element's value of a Puppet hash.
 * The returned elements are only valid until the hash is modified.
 * @param hash The Puppet hash value.
 * @param key The key to get the value for.
 * @param value The returned value; will be set to nullptr if the key does not exist in the hash.
 * @return Returns non-zero if the element was retrieved or zero if not.
 */
int puppet_hash_get(struct puppet_value const* hash, struct puppet_value const* key, struct puppet_value const** value);

/**
 * Sets an element in a Puppet hash.
 * @param hash The hash to set the element for.
 * @param key The element's key. The hash will take ownership of the key; it does not need to be freed.
 * @param value The element's value. The hash will take ownership of the value; it does not need to be freed.
 * @return Returns non-zero if the element was set or zero if the hash value is immutable.
 */
int puppet_hash_set(struct puppet_value* hash, struct puppet_value* key, struct puppet_value* value);

/**
 * Iterates over a Puppet value.
 * @param value The vaule to iterate.
 * @param data The data to pass to the callback.
 * @param callback The callback to invoke for element in the sequence.
 *        The first argument is the key (hash iteration only) and the second argument is the value.
 *        If the callback returns 0, the iteration will stop early.
 * @return Returns non-zero if the value is iterable or 0 if the value is not iterable.
 */
int puppet_iterate(struct puppet_value const* value, void const* data, int (*callback)(void const*, struct puppet_value const*, struct puppet_value const*));

/**
 * Converts the given value to a Puppet string value.
 * If the value is already a string, a new string value with copied contents is returned.
 * @param value The value to convert to a string.
 * @return Returns the new string value.  Free with `puppet_free_value`.
 */
struct puppet_value* puppet_value_to_string(struct puppet_value const* value);

#ifdef __cplusplus
}
#endif
