#include "lib.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>

#define eprintf(...) fprintf(stderr, __VA_ARGS__)
bool print_str(const char *str, size_t len, void *data) {
	fwrite(str, 1, len, stdout);
	return true;
}

int get_char(size_t index, void *data) {
	FILE *fp = data;
	if (ferror(fp)) return BIN_C_ERROR;
	return fgetc(fp);
}

int main(int argc, char *const *argv) {
	bool invalid = false;
	int opt;

	const char *var_name = NULL;
	bool is_include = false;

	static const char *default_data_type = "unsigned char %[]", *default_data_len_type = "size_t %_len";
	const char *data_type = default_data_type, *data_len_type = default_data_len_type;

	// argument handling
	while ((opt = getopt_long(argc, argv, ":hn:it:T:", (struct option[]) {
	                                                           {"help",             no_argument,       0, 'h'},
	                                                           {"name",             required_argument, 0, 'n'},
	                                                           {"header",           no_argument,       0, 'i'},
	                                                           {"include",          no_argument,       0, 'i'},
	                                                           {"data-type",        required_argument, 0, 't'},
	                                                           {"data-length-type", required_argument, 0, 't'},
	                                                           {"data-len-type",    required_argument, 0, 'T'},
	                                                           {0,                  0,                 0, 0  }
    },
	                          NULL)) != -1) {
		if (opt == 'h') {
			printf("Usage: bin-c [OPTION]... <FILE>\n"
			       "-h --help: Shows help text\n"
			       "-n --name: Variable name\n"
			       "-i --header --include: Output extern variables to use as a header file instead of compiled\n"
			       "-t --data-type: Type of the array variable, default: `%s`\n"
			       "-T --data-length-type: Type of the buffer length variable, default: `%s`\n"
			       "<FILE>: Input file, use - for stdin\n",
			       default_data_type, default_data_len_type);
			return 0;
		} else if (invalid)
			break;
		switch (opt) {
			case 'n':
				if (var_name) invalid = true;
				var_name = optarg;
				break;
			case 'i':
				is_include = true;
				break;
			case 't':
				if (data_type != default_data_type) invalid = true;
				data_type = optarg;
				break;
			case 'T':
				if (data_len_type != default_data_len_type) invalid = true;
				data_len_type = optarg;
				break;
			default:
				invalid = true;
				break;
		}
	}

	if (optind != argc - 1 || invalid) {
		eprintf("Invalid usage, try --help\n");
		return 1;
	}

	if (!strchr(data_type, BIN_C_VAR_PLACEHOLDER)) {
		eprintf("Missing `%c` character for --data-type\n", BIN_C_VAR_PLACEHOLDER);
		return 1;
	}
	if (!strchr(data_len_type, BIN_C_VAR_PLACEHOLDER)) {
		eprintf("Missing `%c` character for --data-length-type\n", BIN_C_VAR_PLACEHOLDER);
		return 1;
	}

	char *file_name = argv[optind];
	// open file or stdin
	FILE *fp = NULL;
	if (strcmp(file_name, "-") == 0) {
		if (!var_name) {
			eprintf("Missing --name\n");
			return 1;
		}
		if (!is_include) fp = stdin;
	} else if (!is_include) {
		fp = fopen(file_name, "rb");
		if (!fp) {
			eprintf("fopen: %s: %s\n", file_name, strerror(errno));
			return errno;
		}
	}

	if (!var_name) var_name = file_name;

	char *sanitised_var_name = bin_c_sanitise_variable_name(var_name, NULL);
	if (!sanitised_var_name) {
		eprintf("bin_c_sanitise_variable_name: %s: %s\n", file_name, strerror(errno));
		if (fp != stdin && fp) fclose(fp);
		return errno;
	}
	if (!bin_c_encode_data(print_str, sanitised_var_name, is_include ? NULL : get_char, data_type, data_len_type, fp)) {
		if (fp != stdin && fp) fclose(fp);
		return 2;
	}
	if (fp != stdin && fp) fclose(fp);
	return 0;
}
