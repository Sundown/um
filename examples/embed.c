#include "../lib/um.h"

int main(int argc, char** argv) {
	/* Start the interpreter, instantiate builtins and environment */
	um_init();
	/* Define a count function, this code is correct and we don't care about
	 * the return value but it serves as a fine place to declare ret */
	Result ret = um_interpret_string(
	    "(defun count (n) { (print (str \"counting to: \" n)) (defun recurse (n) (if (= n 1) (print \"Done!\") (recurse (- n 1)))) (recurse n)})");

	if (argc > 1) {
		/* 9 is length of "(count )\0" */
		size_t l = strlen(argv[1]) + 9;
		char* s = calloc(l, sizeof(char));
		/* Substitute input number as count depth */
		snprintf(s, l, "(count %ld)", atol(argv[1]));
		ret = um_interpret_string(s);
	} else {
		ret = um_interpret_string("(count 1000)");
	}

	um_print_result(ret);
	return 0;
}
