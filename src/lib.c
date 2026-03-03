#include "lib.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <locale.h>
#include <stdint.h>
#include <limits.h>

#include <stdio.h>

char *bin_c_sanitise_variable_name(const char *in, char *out) {
	return bin_c_sanitise_variable_name_n(in, strlen(in), out);
}
char *bin_c_sanitise_variable_name_n(const char *in, size_t in_len, char *out) {
	if (!out) {
		out = malloc(in_len + 3);
		if (!out) return NULL;
	}
	// get "C" local
	locale_t locale = newlocale(LC_CTYPE, "C", NULL);
	if (!locale) {
		free(out);
		return NULL;
	}

	size_t out_i = 0;
	// cannot start with a digit
	if (in_len < 1 || isdigit_l(*in, locale)) {
		out[out_i++] = '_';
	}
	for (size_t i = 0; i < in_len; ++i) {
		char c = in[i];
		// allow only numbers/letters
		if (isalnum_l(c, locale)) out[out_i++] = c;
		else if (out_i == 0 || out[out_i - 1] != '_') // replace invalid character with underscore, unless the character before it also is
			out[out_i++] = '_';
	}

	out[out_i] = '\0'; // null-terminate

	freelocale(locale);
	return out;
}

// helper macros

#define OUTPUT(buffer, buffer_len)                                  \
	{                                                               \
		if (!callback(buffer, buffer_len, user_data)) return false; \
	}
#define OUTPUT_STR(buffer) OUTPUT(buffer, strlen(buffer))

static int print_variable(bool (*callback)(const char *buffer, size_t buffer_len, void *user_data), const char *variable_name, const char *type, const char *prefix, void *user_data) {
	// separate a string by the first instance of BIN_C_VAR_PLACEHOLDER
	if (!type) return 2;
	const char *type_sep = strchr(type, BIN_C_VAR_PLACEHOLDER);
	if (!type_sep) return 2;
	const char *type_post = type_sep + 1;
	ptrdiff_t type_pre_len = type_sep - type;

	if (prefix) OUTPUT_STR(prefix);
	OUTPUT(type, type_pre_len); // output type before name
	OUTPUT_STR(variable_name);  // output variable name
	OUTPUT_STR(type_post);      // output type after name

	return 1;
}

static size_t log10t(size_t s) {
	size_t i = 1;
	for (; s >= 10; s /= 10) ++i;
	return i;
}
#define LENGTH(arr) (sizeof(arr) / sizeof(*arr))

bool bin_c_encode_data(bool (*callback)(const char *buffer, size_t buffer_len, void *user_data), const char *variable_name,
                       int (*get_char)(size_t index, void *user_data), const char *data_type, const char *data_len_type, void *user_data) {
	const char *prefix = get_char ? "" : "extern ";

	static size_t temp_len = 0;
	if (temp_len == 0) temp_len = log10t(SIZE_MAX) + 32;
	char temp[temp_len];

	size_t data_len = 0;

	int res = print_variable(callback, variable_name, data_type, prefix, user_data); // add extern if necessary
	if (!res) return false;
	else if (res == 1) {
		if (get_char) {
			// write data
			OUTPUT_STR("={");
			int c;
			for (; (c = get_char(data_len, user_data)) >= 0; ++data_len) {
				// write each char
				int res = snprintf(temp, LENGTH(temp), "%i,", (unsigned char) c);
				if (res < 0 || res >= LENGTH(temp)) return false;
				OUTPUT(temp, res);
			}
			if (c != EOF) return false; // error
			OUTPUT_STR("0}");           // null-terminate just in case
		}
		OUTPUT_STR(";\n");
	}

	res = print_variable(callback, variable_name, data_len_type, prefix, user_data); // add extern if necessary
	if (!res) return false;
	else if (res == 1) {
		if (get_char) {
			// write length
			OUTPUT_STR("=");
			res = snprintf(temp, LENGTH(temp), "%zu", data_len);
			if (res < 0 || res >= LENGTH(temp)) return false;
			OUTPUT(temp, res);
		}
		OUTPUT_STR(";\n");
	}

	return true;
}
