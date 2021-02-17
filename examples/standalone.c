#include "../um.h"

bool do_repl = false;
/* So glad you could make it */
int main(int argc, char** argv) {
	char* file_name = NULL;
	um_init();
	if (argc == 1) {
repl:
		return um_repl(), 0;
	}

	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (!strcmp(argv[i] + 1, "nogc")) {
				um_global_gc_disabled = true;
				continue;
			} else if (!strcmp(argv[i] + 1, "debug")) {
				um_global_debug_enabled = true;
				continue;
			} else if (!strcmp(argv[i] + 1, "repl")) {
				do_repl = true;
				continue;
			} else if (!strcmp(argv[i] + 1, "ss")) {
				um_global_symbol_capacity
				    = (unsigned long)atol(argv[++i]);
				continue;
			} else if (!strcmp(argv[i] + 1, "v")) {
				fprintf(stdout,
					"um: we don't track versions.\n");
				return 0;
			}
		} else {
			/* If file_name already exists and isn't equal to
			 * current arg */
			if (file_name && strcmp(file_name, argv[i])) {
				fprintf(
				    stderr,
				    "um: cannot open 2 files, \"%s\" and \"%s\"\n",
				    file_name,
				    argv[i]);
				return 1;
			} else {
				file_name = argv[i];
			}
		}
	}

	if (file_name) {
		um_Result err = um_load_file(file_name);
		if (err.error._) {
			fprintf(stderr, "In file %s:\n", file_name);
			um_print_result(err);
			return 1;
		}

		/* Run REPL after reading file */
		if (do_repl) { goto repl; }
	} else {
		goto repl;
	}
}
