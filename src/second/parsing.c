#include "../um.h"

Error lex(const char* str, const char** start, const char** end) {
start:
	str += strspn(str, " \t\r\n");

	if (str[0] == '\0') {
		*start = *end = NULL;
		return MakeErrorCode(ERROR_FILE);
	}

	*start = str;

	if (strchr("(){}[]'`!:&.", str[0]) != NULL) {
		*end = str + 1; /* Normal */
	} else if (str[0] == ',') {
		*end = str + (str[1] == '@' ? 2 : 1);
	} else if (str[0] == '"') {
		for (str++; *str != 0; str++) {
			if (*str == '\\') {
				str++;
			} else if (*str == '"') {
				break;
			}
		}

		*end = str + 1;
	} else if (str[0] == ';') {
		str += strcspn(str, "\n");
		goto start;
	} else {
		*end = str + strcspn(str, "(){}[] \t\r\n;");
	}

	return MakeErrorCode(OK);
}

Error parse_simple(const char* start, const char* end, Noun* result) {
	char *p, *buf, *pt;
	size_t length = end - start - 2;
	Error err;
	Noun a1, a2;
	long len, i;
	const char* ps;

	double val = strtod(start, &p);
	bool haspoint = strstr(start, ".") != NULL;
	if (p == end) {
		if (!haspoint && ((isnormal(val) && val == floor(val)) || val == 0)) {
			result->type = i32_t;
			result->value.integer_v = val;
		} else {
			result->type = f64_t;
			result->value.number_v = val;
		}

		return MakeErrorCode(OK);
	} else if (start[0] == '"') {
		result->type = string_t;
		buf = (char*)malloc(length + 1);
		ps = start + 1;
		pt = buf;

		while (ps < end - 1) {
			if (*ps == '\\') {
				char c_next = *(ps + 1);

				switch (c_next) {
					case 'r': *pt = '\r'; break;
					case 'n': *pt = '\n'; break;
					case 't': *pt = '\t'; break;
					default: *pt = c_next;
				}

				ps++;
			} else {
				*pt = *ps;
			}

			ps++;
			pt++;
		}
		*pt = 0;
		buf = realloc(buf, pt - buf + 1);
		*result = new_string(buf);
		return MakeErrorCode(OK);
	}

	buf = malloc(end - start + 1);
	memcpy(buf, start, end - start);
	buf[end - start] = 0;

	if (strcmp(buf, "nil") == 0) {
		*result = nil;
	} else if (strcmp(buf, ".") == 0) {
		*result = intern(buf);
	} else {
		len = end - start;
		for (i = len - 1; i >= 0; i--) {
			if (buf[i] == '^') {
				if (i == 0 || i == len - 1) {
					free(buf);
					return MakeErrorCode(ERROR_SYNTAX);
				}

				err = parse_simple(buf, buf + i, &a1);
				if (err._) {
					free(buf);
					return MakeErrorCode(ERROR_SYNTAX);
				}

				err = parse_simple(buf + i + 1, buf + len, &a2);
				if (err._) {
					free(buf);
					return MakeErrorCode(ERROR_SYNTAX);
				}

				free(buf);
				*result = cons(a1, cons(a2, nil));
				return MakeErrorCode(OK);
			} else if (buf[i] == ':' && buf[i + 1] == ':') {
				if (i == 0 || i == len - 1) {
					free(buf);
					return MakeErrorCode(ERROR_SYNTAX);
				}

				err = parse_simple(buf, buf + i, &a1);
				if (err._) {
					free(buf);
					return MakeErrorCode(ERROR_SYNTAX);
				}

				err = parse_simple(buf + i + 2, buf + len, &a2);
				if (err._) {
					free(buf);
					return MakeErrorCode(ERROR_SYNTAX);
				}

				free(buf);
				*result = cons(a1, cons(cons(intern("quote"), cons(a2, nil)), nil));
				return MakeErrorCode(OK);
			} else if (buf[i] == '.' && buf[i + 1] == '.') {
				if (i == 0 || i == len - 1) {
					free(buf);
					return MakeErrorCode(ERROR_SYNTAX);
				}

				err = parse_simple(buf, buf + i, &a1);
				if (err._) {
					free(buf);
					return MakeErrorCode(ERROR_SYNTAX);
				}

				err = parse_simple(buf + i + 2, buf + len, &a2);
				if (err._) {
					free(buf);
					return MakeErrorCode(ERROR_SYNTAX);
				}

				free(buf);

				*result
				    = cons(intern("range"),
					   cons(coerce(a1, i32_t), cons(coerce(a2, i32_t), nil)));

				return MakeErrorCode(OK);
			}
		}

		*result = intern(buf);
	}

	free(buf);
	return MakeErrorCode(OK);
}

Error read_list(const char* start, const char** end, Noun* result) {
	Noun p;

	*end = start;
	p = *result = nil;

	for (;;) {
		const char* token;
		Noun item;
		Error err;

		err = lex(*end, &token, end);
		if (err._) { return err; }

		if (token[0] == ')') { return MakeErrorCode(OK); }

		/* TODO segfault occurs on (0 . 0), not located in following block */
		if (!isnil(p) && token[0] == '.' && *end - token == 1) {

			if (isnil(p)) { return MakeErrorCode(ERROR_SYNTAX); }
			err = read_expr(*end, end, &item);
			if (err._) { return err; }

			cdr(p) = item;

			err = lex(*end, &token, end);
			if (!err._ && token[0] != ')') { err = MakeErrorCode(ERROR_SYNTAX); }

			return err;
		}

		err = read_expr(token, end, &item);
		if (err._) { return err; }

		if (isnil(p)) {

			*result = cons(item, nil);
			p = *result;
		} else {
			cdr(p) = cons(item, nil);
			p = cdr(p);
		}
	}
}

Error read_prefix(const char* start, const char** end, Noun* result) {
	Noun p = *result = nil;
	*end = start;

	while (1) {
		const char* token;
		Noun item;
		Error err = lex(*end, &token, end);

		if (err._) { return err; }

		if (token[0] == ']') {
			*result = cons(intern("list"), reverse_list(p));
			return MakeErrorCode(OK);
		}

		err = read_expr(token, end, &item);

		if (err._) { return err; }

		p = cons(item, p);
	}
}

Error read_block(const char* start, const char** end, Noun* result) {
	Noun p = *result = nil;
	*end = start;

	p = nil;

	while (1) {
		const char* token;
		Noun item;
		Error err = lex(*end, &token, end);

		if (err._) { return err; }

		if (token[0] == '}') {
			*result = cons(intern("do"), reverse_list(p));
			return MakeErrorCode(OK);
		}

		err = read_expr(token, end, &item);

		if (err._) { return err; }

		p = cons(item, p);
	}
}

Error read_expr(const char* input, const char** end, Noun* result) {
	char* token;
	Error err;

	err = lex(input, (const char**)&token, end);
	if (err._) { return err; }

	if (token[0] == '(') {
		return read_list(*end, end, result);
	} else if (token[0] == ')') {
		return MakeErrorCode(ERROR_SYNTAX);
	} else if (token[0] == '{') {
		return read_block(*end, end, result);
	} else if (token[0] == '}') {
		return MakeErrorCode(ERROR_SYNTAX);
	} else if (token[0] == '[') {
		Noun n0, n1;
		Error e0 = read_prefix(*end, end, &n0);
		if (e0._) { return e0; }
		n0 = car(cdr(n0));
		e0 = eval_expr(n0, env, &n1);
		if (e0._) { return e0; }
		switch (n1.type) {
			case i32_t:
				{
					*result = cons(intern("getlist"), cons(n1, cons(nil, nil)));
					return read_expr(*end, end, &car(cdr(cdr(*result))));
				}
			case type_t:
				{
					*result = cons(intern("coerce"), cons(nil, cons(n1, nil)));
					return read_expr(*end, end, &car(cdr(*result)));
				}
			default:
				return MakeError(ERROR_ARGS,
						 "prefix: was not passed a valid prefix");
		}

	} else if (token[0] == ']') {
		return MakeErrorCode(ERROR_SYNTAX);
	} else if (token[0] == '\'') {
		*result = cons(intern("quote"), cons(nil, nil));
		return read_expr(*end, end, &car(cdr(*result)));
	} else if (token[0] == ']') {
		return MakeErrorCode(ERROR_SYNTAX);
	} else if (token[0] == '&') {
		*result = cons(intern("curry"), cons(nil, nil));
		return read_expr(*end, end, &car(cdr(*result)));
	} else if (token[0] == '!') {
		*result = cons(intern("not"), cons(nil, nil));
		return read_expr(*end, end, &car(cdr(*result)));
	} else if (token[0] == '`') {
		*result = cons(intern("quasiquote"), cons(nil, nil));
		return read_expr(*end, end, &car(cdr(*result)));
	} else if (token[0] == ',') {
		*result = cons(token[1] == '@' ? intern("unquote-splicing") : intern("unquote"),
			       cons(nil, nil));
		return read_expr(*end, end, &car(cdr(*result)));
	} else {
		return parse_simple(token, *end, result);
	}
}
