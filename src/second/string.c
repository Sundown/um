#include "../um.h"

void print_error(Error e) {
	char* s = error_to_string(e);
	printf("%s", s);
	free(s);
}

void print_expr(Noun a) {
	char* s = to_string(a, 1);
	printf("%s", s);
	free(s);
}

char* append_string(char** dst, char* src) {
	size_t len = strlen(*dst) + strlen(src);
	*dst = realloc(*dst, (len + 1) * sizeof(char));
	strcat(*dst, src);
	return *dst;
}

char* str_new() {
	char* s = calloc(1, sizeof(char));
	s[0] = '\0';
	return s;
}
