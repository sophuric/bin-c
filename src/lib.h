#ifndef GUARD_LIB_H
#define GUARD_LIB_H
#include <stddef.h>
#include <stdbool.h>
#define BIN_C_VAR_PLACEHOLDER_STR "%"
#define BIN_C_VAR_PLACEHOLDER '%'
/**
 * Sanitises a null-terminated string to use as a C variable name
 * `out` is a string to hold the output with a size of at least `strlen(in)+3` including NULL-terminator.
 * If `out` is NULL, then a string will be allocated with `malloc()` instead, which should be freed with `free()`.
 * Returns a null-terminated string of the variable name. Returns `NULL` if `malloc()` or `newlocale()` fails.
 */
char *bin_c_sanitise_variable_name(const char *in, char *out);

/**
 * Sanitises a string with length `in_len` to use as a C variable name
 * `out` is a string to hold the output with a size of at least `in_len+3` including NULL-terminator.
 * If `out` is NULL, then a string will be allocated with `malloc()` instead, which should be freed with `free()`.
 * Returns a null-terminated string of the variable name. Returns `NULL` if `malloc()` or `newlocale()` fails.
 */
char *bin_c_sanitise_variable_name_n(const char *in, size_t in_len, char *out);

/**
 * Encodes `data` as C code
 * `callback` takes in a buffer and a length, and returns whether to continue the function.
 * `variable_name` is the output from `bin_c_sanitise_variable_name` or `bin_c_sanitise_variable_name_n`.
 * `get_char` is a function to get the next available character. This should return an `unsigned char` value, EOF at end of stream, or another negative value such as BIN_C_ERROR for error.
 * This allows for streaming the file directly to this function. This can be NULL, which makes the variables `extern`.
 * `data_type` and `data_len_type` are the C type of those variables respectively. If this is NULL, the variable is not generated.
 * The first instance of `%` for the types are replaced with the variable name, for example: `"unsigned char %[]"` and `"size_t %_len"`.
 * The last return value of `callback` is the return value of this function.
 */
bool bin_c_encode_data(bool (*callback)(const char *buffer, size_t buffer_len, void *user_data), const char *variable_name,
                       int (*get_char)(size_t index, void *user_data), const char *data_type, const char *data_len_type, void *user_data);

#define BIN_C_ERROR ((EOF) > -4 ? -8 : (EOF) + 1)
#endif // GUARD_LIB_H
