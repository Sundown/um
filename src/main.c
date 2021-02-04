#include "um.h"
void print_help(FILE* stream);
bool um_global_gc_disabled, um_global_debug_enabled, do_repl = false;
size_t um_global_symbol_capacity;
Noun cur_expr;
/* So glad you could make it */
int main(int argc, char** argv) {
	char* file_name = NULL;
	um_init();
	if (argc == 1) {
repl:
		return repl(), puts(""), 0;
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
				um_global_symbol_capacity = (unsigned long)atol(argv[++i]);
				continue;
			} else if (!strcmp(argv[i] + 1, "v")) {
				fprintf(stdout, "um: we don't track versions.\n");
				return 0;
			} else {
				printf("um: invalid argument: %s\n", argv[i]);
				print_help(stdout);
			}
		} else {
			/* If file_name already exists and isn't equal to current arg */
			if (file_name && strcmp(file_name, argv[i])) {
				fprintf(stderr,
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
		Error err = um_load_file(file_name);
		if (err._) {
			fprintf(stderr, "In file %s:\n", file_name);
			print_error(err);
			return 1;
		}

		/* Run REPL after reading file */
		if (do_repl) { goto repl; }
	} else {
		goto repl;
	}
}

void print_help(FILE* stream) {
	fprintf(stream,
		"Usage: um [options] file\n"
		"    -nogc\tdisable garbage collection\n"
		"    -debug\tenable debug logging during runtime\n"
		"    -h\t\tyou_are_here.png\n"
		"    -repl\tstart REPL, to be used in conjuction with file input\n"
		"    -ss <uint>\tsymbol table size, default 1,000\n"
		"    -v\t\tversion, but don't bother\n");
}
