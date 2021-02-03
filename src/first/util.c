#include "../um.h"

void repl() {
	char* input;

	while ((input = readline_fp(PROMPT, stdin))) {
		const char* p;
		char* line;
start:
		p = input;
		Noun expr;
		Error err = read_expr(p, &p, &expr);
		if (err._ == MakeErrorCode(ERROR_FILE)._) {
			line = readline_fp("	", stdin);
			if (!line) break;
			input = append_string(&input, "\n");
			input = append_string(&input, line);
			free(line);
			goto start;
		}

		if (!err._) {
			Noun result;
			while (1) {
				err = macex_eval(expr, &result);
				if (err._) {
					print_error(err);
					printf("Error in expression: %s\n", to_string(expr, 0));
					break;
				} else {
					print_expr(result);
					puts("");
				}

				err = read_expr(p, &p, &expr);
				if (err._ != MakeErrorCode(OK)._) { break; }
			}
		} else {
			print_error(err);
		}

		free(input);
	}
}

char* readline_fp(char* prompt, FILE* fp) {
	size_t size = 80;
	char* str;
	int ch;
	size_t len = 0;
	printf("%s", prompt);
	str = calloc(size, sizeof(char));
	if (!str) { return NULL; }

	while ((ch = fgetc(fp)) != EOF && ch != '\n') {
		str[len++] = ch;
		if (len == size) {
			void* p = realloc(str, sizeof(char) * (size *= 2));
			if (!p) {
				free(str);
				return NULL;
			}

			str = p;
		}
	}

	if (ch == EOF && len == 0) {
		free(str);
		return NULL;
	}

	str[len++] = '\0';

	return realloc(str, sizeof(char) * len);
}

Error interpret_string(const char* text) {
	Error err = MakeErrorCode(OK);
	const char* p = text;
	Noun expr, result;
	while (*p) {
		if (isspace(*p)) {
			p++;
			continue;
		}

		err = read_expr(p, &p, &expr);
		if (err._) { break; }

		err = macex_eval(expr, &result);
		if (err._) { break; }
	}

	return err;
}

char* read_file(const char* path) {
	FILE* fp = fopen(path, "rb");
	if (!fp) { return NULL; }

	size_t len;
	char* buf;

	fseek(fp, 0, SEEK_END); /* Seek to end */
	len = ftell(fp);	/* record position as length */
	if (len < 0) { return NULL; }
	fseek(fp, 0, SEEK_SET);			    /* seek to start */
	buf = (char*)calloc(len + 1, sizeof(char)); /*alloc based on length*/
	if (!buf) { return NULL; }

	if (fread(buf, 1, len, fp) != len) { return NULL; }
	buf[len] = '\0';

	fclose(fp);
	return buf;
}

Error um_load_file(const char* path) {
	char* text = read_file(path);
	if (text) {
		Error err = interpret_string(text);
		free(text);
		return err;
	} else {
		return MakeErrorCode(ERROR_FILE);
	}
}
