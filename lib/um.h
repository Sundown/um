/* MIT License

Copyright (c) 2021 sundown

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. */

#ifndef um_H
#define um_H

#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define _REPL_PROMPT "> "

typedef enum {
	nil_t,
	pair_t,
	noun_t,
	f64_t,
	i32_t,
	builtin_t,
	closure_t,
	macro_t,
	string_t,
	input_t,
	output_t,
	table_t,
	error_t,
	type_t,
	bool_t
} noun_type;

typedef enum {
	OK = 0,
	ERROR_SYNTAX,
	ERROR_UNBOUND,
	ERROR_ARGS,
	ERROR_TYPE,
	ERROR_FILE,
	ERROR_USER,
	ERROR_NOMUT,
	ERROR_COERCION_FAIL
} ErrorCode;

typedef struct {
	ErrorCode _;
	char* message;
} Error;

static const char* error_string[] = {"",
				     "Syntax error",
				     "Symbol unbound",
				     "Parameter error",
				     "Type error",
				     "File error",
				     "",
				     "Cannot mutate constant",
				     "Coercion error"};

typedef struct Noun Noun;
typedef struct Vector Vector;

typedef Error (*builtin)(struct Vector* v_params, struct Noun* result);

struct Noun {
	noun_type type;
	bool mut;
	union {
		noun_type type_v;
		bool bool_v;
		Error error_v;
		double number_v;
		long integer_v;
		struct Pair* pair_v;
		char* symbol;
		struct str* str;
		FILE* fp;
		builtin builtin;
		struct Table* Table;
		Error err_v;
	} value;
};

/* This should be returned by any functions which a user may interract with
 * directly */
struct Result {
	Error error;
	Noun data;
};
typedef struct Result Result;

struct Pair {
	struct Noun car, cdr;
	char mark;
	struct Pair* next;
};
typedef struct Pair Pair;

struct Table_entry {
	Noun k, v;
	struct Table_entry* next;
};
typedef struct Table_entry Table_entry;

struct Table {
	size_t capacity;
	size_t size;
	Table_entry** data;
	char mark;
	struct Table* next;
};
typedef struct Table Table;

struct Vector {
	Noun* data;
	Noun static_data[8];
	size_t capacity, size;
};

struct str {
	char* value;
	char mark;
	struct str* next;
};

bool um_global_gc_disabled, um_global_debug_enabled;

static const Noun nil
    = {.type = nil_t, .mut = false, .value = {.type_v = nil_t}};

Noun sym_quote, sym_const, sym_quasiquote, sym_unquote, sym_unquote_splicing,
    sym_def, sym_set, sym_defun, sym_fn, sym_if, sym_cond, sym_switch,
    sym_match, sym_mac, sym_apply, sym_cons, sym_string, sym_num, sym_int,
    sym_char, sym_do, sym_true, sym_false,

    sym_nil_t, sym_pair_t, sym_noun_t, sym_f64_t, sym_i32_t, sym_builtin_t,
    sym_closure_t, sym_macro_t, sym_string_t, sym_input_t, sym_output_t,
    sym_error_t, sym_type_t, sym_bool_t;

Noun env;
static size_t stack_capacity = 0;
static size_t stack_size = 0;
static Noun* stack = NULL;
static Pair* pair_head = NULL;
static struct str* str_head = NULL;
static Table* table_head = NULL;
static size_t alloc_count = 0;
static size_t alloc_count_old = 0;
char** symbol_table;
size_t symbol_size;
size_t um_global_symbol_capacity;
Noun cur_expr;
Noun thrown;

/* clang-format off */
#define car(p)	    ((p).value.pair_v->car)
#define cdr(p)	    ((p).value.pair_v->cdr)
#define cdr2(p)	    (cdr(p))
#define pop(n)	    (n = cdr2(n))
#define isnil(Noun) ((Noun).type == nil_t)
#define isnumber(a) (a.type == f64_t || a.type == i32_t)
#define getnumber(a)                           \
	(a.type == f64_t   ? a.value.number_v  \
	 : a.type == i32_t ? a.value.integer_v \
			   : 0)
#define ingest(s)                                                         \
	do {                                                              \
		Result __tmperr = um_interpret_string(s);                 \
		if (__tmperr.error._) { um_print_error(__tmperr.error); } \
	} while (0)
#define MakeErrorCode(c) (Error){c, NULL}
#define MakeError(c, m)  (Error){c, m}
#define new(T) _Generic((T),  	\
	bool: new_bool,      	\
	long: new_integer,   	\
	char*: new_string,   	\
	double: new_number,  	\
	builtin: new_builtin,	\
	noun_type: new_type  	\
)(T)

inline Noun new_number(double x) { return (Noun){f64_t, true, {.number_v = x}}; }
inline Noun new_integer(long x) { return (Noun){i32_t, true, {.integer_v = x}}; }
inline Noun new_builtin(builtin fn) { return (Noun){builtin_t, true, {.builtin = fn}}; }
inline Noun new_type(noun_type t) { return (Noun){type_t, true, {.type_v = t}}; }
inline Noun new_bool(bool b) { return (Noun){bool_t, true, {.bool_v = b}}; }
/* clang-format on */

/*
	Begin necessary forward declarations
*/
Noun cons(Noun car_val, Noun cdr_val);
Noun intern(const char* buf);
Noun new_string(char* x);

void stack_add(Noun a);

Noun integer_to_t(long x, noun_type t);
Noun number_to_t(double x, noun_type t);
Noun noun_to_t(char* x, noun_type t);
Noun string_to_t(char* x, noun_type t);
Noun bool_to_t(bool x, noun_type t);
Noun type_to_t(noun_type x, noun_type t);
Noun nil_to_t(Noun x __attribute__((unused)), noun_type t);

bool listp(Noun expr);
Noun reverse_list(Noun list);

Error macex_eval(Noun expr, Noun* result);
Error eval_expr(Noun expr, Noun env, Noun* result);

Noun env_create(Noun parent, size_t capacity);
Error env_bind(Noun env, Noun arg_names, Vector* v_params);
Error env_assign(Noun env, char* symbol, Noun value);
Error env_get(Noun env, char* symbol, Noun* result);

Noun new_table(size_t capacity);
Table_entry* table_get_sym(Table* tbl, char* k);
Error table_set_sym(Table* tbl, char* k, Noun v);

void garbage_collector_consider();
void garbage_collector_tag(Noun root);

void um_print_expr(Noun a);
void um_print_error(Error e);
void um_print_result(Result r);

size_t hash_code_sym(char* s);

char* str_new();
char* to_string(Noun a, bool write);
char* append_string(char** dst, char* src);

bool eq_l(Noun a, Noun b);
bool eq_h(Noun a, Noun b);
bool eq_pair_l(Noun a, Noun b);
bool eq_pair_h(Noun a, Noun b);

char* readline_fp(char* prompt, FILE* fp);
Error read_expr(const char* input, const char** end, Noun* result);

Noun cons(Noun car_val, Noun cdr_val) {
	Pair* a;
	Noun p;
	alloc_count++;

	a = (Pair*)calloc(1, sizeof(Pair));
	a->mark = 0;
	a->next = pair_head;
	pair_head = a;

	p.type = pair_t;
	p.value.pair_v = a;

	car(p) = car_val;
	cdr(p) = cdr_val;

	stack_add(p);

	return p;
}

void vector_new(Vector* a) {
	a->capacity = sizeof(a->static_data) / sizeof(a->static_data[0]);
	a->size = 0;
	a->data = a->static_data;
}

void vector_add(Vector* a, Noun item) {
	if (a->size + 1 > a->capacity) {
		a->capacity *= 2;
		if (a->data == a->static_data) {
			a->data = (Noun*)malloc(a->capacity * sizeof(Noun));
			memcpy(a->data, a->static_data, a->size * sizeof(Noun));
		} else {
			a->data = (Noun*)realloc(a->data,
						 a->capacity * sizeof(Noun));
		}
	}

	a->data[a->size] = item;
	a->size++;
}

void vector_clear(Vector* a) {
	a->size = 0;
}

void vector_free(Vector* a) {
	if (a->data != a->static_data) free(a->data);
}

void noun_to_vector(Noun a, Vector* v) {
	vector_new(v);
	for (; !isnil(a); a = cdr(a)) { vector_add(v, car(a)); }
}

Noun vector_to_noun(Vector* a, int start) {
	Noun r = nil;
	int i;
	for (i = a->size - 1; i >= start; i--) { r = cons(a->data[i], r); }

	return r;
}

void um_repl() {
	char* input;

	while ((input = readline_fp(_REPL_PROMPT, stdin))) {
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
					um_print_error(err);
					printf("Error in expression: %s\n",
					       to_string(expr, 0));
					break;
				} else {
					um_print_expr(result);
					puts("");
				}

				err = read_expr(p, &p, &expr);
				if (err._ != MakeErrorCode(OK)._) { break; }
			}
		} else {
			um_print_error(err);
		}

		free(input);
	}

	putchar('\n');
}

size_t list_len(Noun xs) {
	Noun* p = &xs;
	size_t ret = 0;
	while (!isnil(*p)) {
		if (p->type != pair_t) { return ret + 1; }

		p = &cdr(*p);
		ret++;
	}

	return ret;
}

void stack_add(Noun a) {
	switch (a.type) {
		case pair_t:
		case closure_t:
		case macro_t:
		case string_t:
		case table_t: break;
		default: return;
	}

	stack_size++;

	if (stack_size > stack_capacity) {
		stack_capacity = stack_size * 2;
		stack = (Noun*)realloc(stack, stack_capacity * sizeof(Noun));
	}

	stack[stack_size - 1] = a;
}

void stack_restore(int saved_size) {
	stack_size = saved_size;
	if (stack_size < stack_capacity / 4) {
		stack_capacity = stack_size * 2;
		stack = (Noun*)realloc(stack, stack_capacity * sizeof(Noun));
	}

	garbage_collector_consider();
}

void stack_restore_add(int saved_size, Noun a) {
	stack_size = saved_size;
	if (stack_size < stack_capacity / 4) {
		stack_capacity = stack_size * 2;
		stack = (Noun*)realloc(stack, stack_capacity * sizeof(Noun));
	}

	stack_add(a);
	garbage_collector_consider();
}

Noun cast(Noun a, noun_type t) {
	if (a.type == t) { return a; }

	switch (a.type) {
		case nil_t: return nil_to_t(nil, t);
		case i32_t: return integer_to_t(a.value.integer_v, t);
		case f64_t: return number_to_t(a.value.number_v, t);
		case noun_t: return noun_to_t(a.value.symbol, t);
		case string_t: return string_to_t(a.value.str->value, t);
		case bool_t: return bool_to_t(a.value.bool_v, t);
		case type_t: return type_to_t(a.value.type_v, t);
		default:
			return nil; /* TODO can probably add more coercions for
				       semi-primitive types */
	}
}

char* type_to_string(noun_type a) {
	switch (a) {
		case nil_t: return "Nil";
		case pair_t: return "Pair";
		case string_t: return "String";
		case noun_t: return "Noun";
		case f64_t: return "Float";
		case i32_t: return "Int";
		case builtin_t: return "Builtin";
		case closure_t: return "Closure";
		case macro_t: return "Macro";
		case input_t: return "Input";
		case output_t: return "Output";
		case table_t: return "Table";
		case bool_t: return "Bool";
		case type_t: return "Type";
		case error_t: return "Error";
		default: return "Unknown";
	}
}

char* error_to_string(Error e) {
	char* s = calloc(e.message != NULL ? strlen(e.message) : 0 + 27,
			 sizeof(char));
	e._ != MakeErrorCode(ERROR_USER)._&& e.message
	    ? sprintf(s, "%s\n%s\n", error_string[e._], e.message)
	    : sprintf(s, "%s\n", error_string[e._]);

	return s;
}

Noun nil_to_t(Noun x __attribute__((unused)), noun_type t) {
	switch (t) {
		case nil_t: return nil;
		case i32_t: return new_integer(0);
		case f64_t: return new_number(NAN);
		case pair_t: return cons(nil, nil);
		case bool_t: return new_bool(false);
		case type_t: return new_type(nil_t);
		case string_t: return new_string("nil");
		case noun_t: return intern("nil");
		default: return nil;
	}
}

Noun integer_to_t(long x, noun_type t) {
	if (t == i32_t) { return new_integer(x); }

	char* buf = NULL;
	if (t == noun_t || t == string_t) {
		buf = calloc(21, sizeof(char));
		snprintf(buf, 21, "%ld", x);
		buf = realloc(buf, strlen(buf) * sizeof(char) + 1);
	}

	switch (t) {
		case nil_t: return nil;
		case f64_t: return new_number((double)x);
		case noun_t: return intern(buf);
		case bool_t: return new_bool(x > 0);
		case pair_t: return cons(new_integer(x), nil);
		case string_t: return new_string(buf);
		case type_t: return new_type(i32_t);
		default: return nil;
	}
}

Noun number_to_t(double x, noun_type t) {
	if (t == f64_t) { return new_number(x); }

	char* buf = NULL;
	if (t == noun_t || t == string_t) {
		buf = calloc(65, sizeof(char));
		snprintf(buf, 65, "%f", x);
		buf = realloc(buf, strlen(buf) * sizeof(char) + 1);
	}

	switch (t) {
		case nil_t: return nil;
		case i32_t: return new_integer((long)x);
		case noun_t: return intern(buf);
		case bool_t: return new_bool(x > 0 && isnormal(x) && !isnan(x));
		case pair_t: return cons(new_number(x), nil);
		case string_t: return new_string(buf);
		case type_t: return new_type(f64_t);
		default: return nil;
	}
}

Noun noun_to_t(char* x, noun_type t) {
	switch (t) {
		case pair_t: return cons(intern(x), nil);
		case noun_t: return intern(x);
		case i32_t: return new_integer(atol(x));
		case f64_t: return new_number(strtod(x, NULL));
		case string_t: return new_string(x);
		case type_t: return new_type(noun_t);
		case bool_t:
			return new_bool(x != NULL
					&& x != intern("nil").value.symbol);
		default: return nil;
	}
}

Noun string_to_t(char* x, noun_type t) {
	switch (t) {
		case pair_t: return cons(intern(x), nil);
		case noun_t: return intern(x);
		case i32_t: return new_integer(atol(x));
		case f64_t: return new_number(strtod(x, NULL));
		case string_t: return new_string(x);
		case type_t: return new_type(noun_t);
		case bool_t:
			return new_bool(x != NULL && strcmp(x, "nil")
					&& strcmp(x, "false"));
		default: return nil;
	}
}

Noun bool_to_t(bool x, noun_type t) {
	switch (t) {
		case bool_t: return new_bool(x);
		case pair_t: return cons(new_bool(x), nil);
		case f64_t: return new_number((double)x);
		case i32_t: return new_integer((long)x);
		case noun_t: return x ? intern("true") : intern("false");
		case string_t:
			return x ? new_string("true") : new_string("false");
		case type_t: return new_type(bool_t);
		default: return nil;
	}
}

Noun type_to_t(noun_type x, noun_type t) {
	switch (t) {
		case type_t: return new_type(type_t);
		case noun_t: return intern(error_string[x]);
		case string_t: return new_string((char*)error_string[x]);
		case bool_t: return new_bool(!x);
		case pair_t: return cons(new_type(x), nil);
		default: return nil;
	}
}

Noun intern(const char* s) {
	Noun a;
	int i;

	for (i = symbol_size - 1; i >= 0; i--) {
		char* t = symbol_table[i];
		if (!strcmp(t, s)) {
			a.type = noun_t;
			a.value.symbol = t;
			return a;
		}
	}

	a.type = noun_t;
	a.mut = true;
	a.value.symbol = calloc(strlen(s) + 1, sizeof(char));
	strcpy(a.value.symbol, s);
	if (symbol_size >= um_global_symbol_capacity) {
		um_global_symbol_capacity *= 2;
		symbol_table = realloc(
		    symbol_table, um_global_symbol_capacity * sizeof(char*));
	}

	symbol_table[symbol_size] = a.value.symbol;
	symbol_size++;

	return a;
}

Error new_closure(Noun env, Noun args, Noun body, Noun* result) {
	Noun p;

	if (!listp(body)) { return MakeErrorCode(ERROR_SYNTAX); }

	p = args;
	while (!isnil(p)) {
		if (p.type == noun_t)
			break;
		else if (p.type != pair_t
			 || (car(p).type != noun_t && car(p).type != pair_t))
			return MakeErrorCode(ERROR_TYPE);
		p = cdr(p);
	}

	if (isnil(cdr(body))) {
		p = body;
	} else {
		p = cons(cons(sym_do, body), nil);
	}

	*result = cons(env, cons(args, p));
	result->mut = true;
	result->type = closure_t;

	return MakeErrorCode(OK);
}

Noun new_string(char* x) {
	Noun a;
	struct str* s;
	alloc_count++;
	s = a.value.str = malloc(sizeof(struct str));
	s->value = x;
	s->mark = 0;
	s->next = str_head;
	str_head = s;

	a.type = string_t;
	a.mut = true;
	stack_add(a);

	return a;
}

Noun new_input(FILE* fp) {
	Noun a;
	a.type = input_t;
	a.value.fp = fp;
	return a;
}

Noun new_output(FILE* fp) {
	Noun a;
	a.type = output_t;
	a.value.fp = fp;
	return a;
}

Result um_interpret_string(const char* text) {
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

	return (Result){.error = err, .data = result};
}

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
		if (!haspoint
		    && ((isnormal(val) && val == floor(val)) || val == 0)) {
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
				*result = cons(
				    a1,
				    cons(cons(intern("quote"), cons(a2, nil)),
					 nil));
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
					   cons(cast(a1, i32_t),
						cons(cast(a2, i32_t), nil)));

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

		/* TODO segfault occurs on (0 . 0), not located in following
		 * block */
		if (!isnil(p) && token[0] == '.' && *end - token == 1) {

			if (isnil(p)) { return MakeErrorCode(ERROR_SYNTAX); }
			err = read_expr(*end, end, &item);
			if (err._) { return err; }

			cdr(p) = item;

			err = lex(*end, &token, end);
			if (!err._ && token[0] != ')') {
				err = MakeErrorCode(ERROR_SYNTAX);
			}

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
			case i32_t: {
				*result = cons(intern("getlist"),
					       cons(n1, cons(nil, nil)));
				return read_expr(
				    *end, end, &car(cdr(cdr(*result))));
			}
			case type_t: {
				*result = cons(intern("cast"),
					       cons(nil, cons(n1, nil)));
				return read_expr(*end, end, &car(cdr(*result)));
			}
			default:
				return MakeError(
				    ERROR_ARGS,
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
		*result = cons(token[1] == '@' ? intern("unquote-splicing")
					       : intern("unquote"),
			       cons(nil, nil));
		return read_expr(*end, end, &car(cdr(*result)));
	} else {
		return parse_simple(token, *end, result);
	}
}

Error apply(Noun fn, Vector* v_params, Noun* result) {
	Noun arg_names, env, body, a;
	Error err;
	size_t index, i;

	if (fn.type == builtin_t) {
		return (*fn.value.builtin)(v_params, result);
	} else if (fn.type == closure_t) {
		arg_names = car(cdr(fn));
		env = env_create(car(fn), list_len(arg_names));
		body = cdr(cdr(fn));

		err = env_bind(env, arg_names, v_params);
		if (err._) { return err; }

		err = eval_expr(car(body), env, result);
		if (err._) { return err; }

		return MakeErrorCode(OK);
	} else if (fn.type == string_t) {
		if (v_params->size != 1) { return MakeErrorCode(ERROR_ARGS); }

		index = (size_t)(v_params->data[0]).value.number_v;
		*result
		    = new_string((char[]){fn.value.str->value[index], '\0'});
		return MakeErrorCode(OK);
	} else if (fn.type == pair_t && listp(fn)) {
		if (v_params->size != 1) { return MakeErrorCode(ERROR_ARGS); }

		if (v_params->data[0].type != i32_t) {
			return MakeErrorCode(ERROR_TYPE);
		}

		index = (size_t)(v_params->data[0]).value.integer_v;
		a = fn;

		for (i = 0; i < index; i++) {
			a = cdr(a);
			if (isnil(a)) {
				*result = nil;
				return MakeErrorCode(OK);
			}
		}

		*result = car(a);
		return MakeErrorCode(OK);
	} else {
		return MakeErrorCode(ERROR_TYPE);
	}
}

Noun copy_list(Noun list) {
	Noun a, p;

	if (isnil(list)) { return nil; }

	a = cons(car(list), nil);
	p = a;
	list = cdr(list);

	while (!isnil(list)) {
		cdr(p) = cons(car(list), nil);
		p = cdr(p);
		list = cdr(list);
		if (list.type != pair_t) {
			p = list;
			break;
		}
	}

	return a;
}

Noun* list_index(Noun* list, size_t index) {
	size_t i;
	Noun a = *list;
	for (i = 0; i < index; i++) {
		pop(a);
		if (isnil(a)) { return (Noun*)&nil; }
	}

	return &car(a);
}

Error env_assign_eq(Noun env, char* symbol, Noun value) {
	while (1) {
		Noun parent = car(env);
		Table* ptbl = cdr(env).value.Table;
		Table_entry* a = table_get_sym(ptbl, symbol);
		if (a) {
			if (!a->v.mut) { return MakeErrorCode(ERROR_NOMUT); }
			a->v = value;
			return MakeErrorCode(OK);
		}

		if (isnil(parent)) { return env_assign(env, symbol, value); }

		env = parent;
	}
}

Noun reverse_list(Noun list) {
	Noun tail = nil;

	while (!isnil(list)) {
		tail = cons(car(list), tail);
		pop(list);
	}

	return tail;
}

bool listp(Noun expr) {
	Noun* p = &expr;
	while (!isnil(*p)) {
		if (p->type != pair_t) { return 0; }

		p = &cdr(*p);
	}

	return 1;
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

Error eval_expr(Noun expr, Noun env, Noun* result) {
	Error err;
	Noun fn, op, args, cond, sym, val, name, macro, *p, r, arg_names;
	size_t ss = stack_size;
	Vector v_params;
start:
	stack_add(env);
	cur_expr = isnil(expr) ? cur_expr : expr;
	if (expr.type == noun_t) {
		err = env_get(env, expr.value.symbol, result);
		return err;
	} else if (expr.type != pair_t) {
		*result = expr;
		return MakeErrorCode(OK);
	} else {
		op = car(expr);
		args = cdr(expr);

		if (op.type == noun_t) {
			if (op.value.symbol == sym_if.value.symbol) {
				p = &args;
				while (!isnil(*p)) {
					if (isnil(cdr(*p))
					    || (cdr(*p).type == bool_t
						&& cdr(*p).value.bool_v
						       == false)) {

						expr = car(*p);
						goto start;
					}

					err = eval_expr(car(*p), env, &cond);
					if (err._) {
						stack_restore(ss);
						return err;
					}

					if (!isnil(cond)
					    && cast(cond, bool_t)
						   .value.bool_v) {

						expr = car(cdr(*p));
						goto start;
					}

					p = &cdr(cdr(*p));
				}

				*result = nil;
				stack_restore_add(ss, *result);
				return MakeErrorCode(OK);
			} else if (op.value.symbol == sym_cond.value.symbol) {
				Noun list
				    = !isnil(args) ? reverse_list(args) : nil;
				Noun n = cons(nil, nil);
				while (!isnil(list)) {
					n = cons(
					    cons(
						sym_if,
						cons(
						    !isnil(car(list))
							? car(car(list))
							: nil,
						    cons(!isnil(car(list))
								 && !isnil(cdr(
								     car(list)))
							     ? car(
								 cdr(car(list)))
							     : nil,
							 n))),
					    nil);
					pop(list);
				}

				err = eval_expr(car(n), env, &cond);
				if (err._) {
					stack_restore(ss);
					return err;
				}

				*result = cond;
				stack_restore_add(ss, *result);
				return MakeErrorCode(OK);
			} else if (op.value.symbol == sym_match.value.symbol) {
				Noun pred = !isnil(args) ? car(args) : nil;
				Noun list = !isnil(args) && !isnil(cdr(args))
					      ? reverse_list(cdr(args))
					      : nil;
				Noun n = cons(nil, nil);
				while (!isnil(list)) {
					n = cons(
					    cons(
						sym_if,
						cons(
						    cons(
							intern("apply"),
							cons(
							    !isnil(car(list))
								? car(car(list))
								: nil,
							    cons(
								cons(
								    intern(
									"quote"),
								    cons(
									cons(
									    pred,
									    nil),
									nil)),
								nil))),
						    cons(!isnil(car(list))
								 && !isnil(cdr(
								     car(list)))
							     ? car(
								 cdr(car(list)))
							     : nil,
							 n))),
					    nil);

					pop(list);
				}

				err = eval_expr(car(n), env, &cond);
				if (err._) {
					stack_restore(ss);
					return err;
				}

				*result = cond;
				stack_restore_add(ss, *result);
				return MakeErrorCode(OK);
			} else if (op.value.symbol == sym_switch.value.symbol) {
				Noun pred = !isnil(args) ? car(args) : nil;
				Noun list = !isnil(args) && !isnil(cdr(args))
					      ? reverse_list(cdr(args))
					      : nil;
				Noun n = cons(nil, nil);

				while (!isnil(list)) {
					n = cons(
					    cons(
						sym_if,
						cons(
						    cons(intern("="),
							 cons(!isnil(car(list))
								  ? car(
								      car(list))
								  : nil,
							      cons(pred, nil))),
						    cons(!isnil(car(list))
								 && !isnil(cdr(
								     car(list)))
							     ? car(
								 cdr(car(list)))
							     : nil,
							 n))),
					    nil);

					pop(list);
				}

				err = eval_expr(car(n), env, &cond);
				if (err._) {
					stack_restore(ss);
					return err;
				}

				*result = cond;
				stack_restore_add(ss, *result);
				return MakeErrorCode(OK);
			} else if (op.value.symbol == sym_set.value.symbol) {
				if (isnil(args) || isnil(cdr(args))) {
					stack_restore(ss);
					return MakeErrorCode(ERROR_ARGS);
				}

				sym = car(args);

				if (env_get(env, sym.value.symbol, result)._) {
					cur_expr = sym;
					return MakeErrorCode(ERROR_UNBOUND);
				}

				if (sym.type == pair_t) {

					err = new_closure(
					    env, cdr(sym), cdr(args), result);
					sym = car(sym);

					if (sym.type != noun_t) {
						return MakeErrorCode(
						    ERROR_TYPE);
					}

					err = env_assign_eq(
					    env, sym.value.symbol, *result);
					*result = sym;
					stack_restore_add(ss, *result);
					return err;
				} else if (sym.type == noun_t) {

					err = eval_expr(
					    car(cdr(args)), env, &val);
					if (err._) {
						stack_restore(ss);
						return err;
					}

					*result = val;
					err = env_assign_eq(
					    env, sym.value.symbol, val);
					stack_restore_add(ss, *result);
					return err;
				} else {
					stack_restore(ss);
					return MakeErrorCode(ERROR_TYPE);
				}
			} else if (op.value.symbol == sym_def.value.symbol) {
				if (isnil(args) || isnil(cdr(args))) {
					stack_restore(ss);
					return MakeErrorCode(ERROR_ARGS);
				}

				sym = car(args);

				if (sym.type == pair_t) {

					err = new_closure(
					    env, cdr(sym), cdr(args), result);
					sym = car(sym);

					if (sym.type != noun_t) {
						return MakeErrorCode(
						    ERROR_TYPE);
					}

					err = env_assign(
					    env, sym.value.symbol, *result);
					*result = sym;
					stack_restore_add(ss, *result);
					return err;
				} else if (sym.type == noun_t) {

					err = eval_expr(
					    car(cdr(args)), env, &val);
					if (err._) {
						stack_restore(ss);
						return err;
					}

					*result = val;
					err = env_assign(
					    env, sym.value.symbol, val);
					stack_restore_add(ss, *result);
					return err;
				} else {
					stack_restore(ss);
					return MakeErrorCode(ERROR_TYPE);
				}
			} else if (op.value.symbol == sym_const.value.symbol) {
				if (isnil(args) || isnil(cdr(args))) {
					stack_restore(ss);
					return MakeErrorCode(ERROR_ARGS);
				}

				sym = car(args);

				if (!env_get(env, sym.value.symbol, result)._) {
					if (!result->mut) {
						return MakeErrorCode(
						    ERROR_NOMUT);
					}
				}

				if (sym.type == pair_t) {

					err = new_closure(
					    env, cdr(sym), cdr(args), result);
					sym = car(sym);

					if (sym.type != noun_t) {
						return MakeErrorCode(
						    ERROR_TYPE);
					}

					result->mut = false;
					err = env_assign_eq(
					    env, sym.value.symbol, *result);
					*result = sym;
					stack_restore_add(ss, *result);
					return err;
				} else if (sym.type == noun_t) {
					err = eval_expr(
					    car(cdr(args)), env, &val);
					if (err._) {
						stack_restore(ss);
						return err;
					}

					val.mut = false;
					*result = val;
					err = env_assign_eq(
					    env, sym.value.symbol, val);
					stack_restore_add(ss, *result);
					return err;
				} else {
					stack_restore(ss);
					return MakeErrorCode(ERROR_TYPE);
				}
			} else if (op.value.symbol == sym_defun.value.symbol) {
				if (isnil(args) || isnil(cdr(args))
				    || isnil(cdr(cdr(args)))) {
					stack_restore(ss);
					return MakeErrorCode(ERROR_ARGS);
				}

				sym = car(args);

				if (sym.type == noun_t
				    && car(cdr(args)).type == pair_t
				    && cdr(cdr(args)).type == pair_t) {

					err = new_closure(env,
							  car(cdr(args)),
							  cdr(cdr(args)),
							  result);

					if (sym.type != noun_t) {
						return MakeErrorCode(
						    ERROR_TYPE);
					}

					err = env_assign_eq(
					    env, sym.value.symbol, *result);
					*result = sym;
					stack_restore_add(ss, *result);
					return err;
				} else {
					stack_restore(ss);
					return MakeErrorCode(ERROR_TYPE);
				}
			} else if (op.value.symbol == sym_quote.value.symbol) {
				if (isnil(args) || !isnil(cdr(args))) {
					stack_restore(ss);
					return MakeErrorCode(ERROR_ARGS);
				}

				*result = car(args);
				stack_restore_add(ss, *result);
				return MakeErrorCode(OK);
			} else if (op.value.symbol == sym_fn.value.symbol
				   || op.value.symbol
					  == intern("\\").value.symbol) {
				if (isnil(args) || isnil(cdr(args))) {
					stack_restore(ss);
					return MakeErrorCode(ERROR_ARGS);
				}

				err = new_closure(
				    env, car(args), cdr(args), result);
				stack_restore_add(ss, *result);
				return err;
			} else if (op.value.symbol == sym_do.value.symbol) {

				*result = nil;
				while (!isnil(args)) {
					if (isnil(cdr(args))) {

						expr = car(args);
						stack_restore(ss);
						goto start;
					}

					err = eval_expr(car(args), env, result);
					if (err._) { return err; }

					args = cdr(args);
				}

				return MakeErrorCode(OK);
			} else if (op.value.symbol == sym_mac.value.symbol) {

				if (isnil(args) || isnil(cdr(args))
				    || isnil(cdr(cdr(args)))) {
					stack_restore(ss);
					return MakeErrorCode(ERROR_ARGS);
				}

				name = car(args);
				if (name.type != noun_t) {
					stack_restore(ss);
					return MakeErrorCode(ERROR_TYPE);
				}

				err = new_closure(env,
						  car(cdr(args)),
						  cdr(cdr(args)),
						  &macro);
				if (!err._) {
					macro.type = macro_t;
					*result = name;
					err = env_assign(
					    env, name.value.symbol, macro);
					stack_restore_add(ss, *result);
					return err;
				} else {
					stack_restore(ss);
					return err;
				}
			}
		}

		err = eval_expr(op, env, &fn);
		if (err._) {
			stack_restore(ss);
			return err;
		}

		vector_new(&v_params);
		p = &args;
		while (!isnil(*p)) {
			err = eval_expr(car(*p), env, &r);
			if (err._) {
				vector_free(&v_params);
				stack_restore(ss);
				return err;
			}

			vector_add(&v_params, r);
			p = &cdr(*p);
		}

		if (fn.type == closure_t) {
			arg_names = car(cdr(fn));
			env = env_create(car(fn), list_len(arg_names));
			expr = car(cdr(cdr(fn)));

			err = env_bind(env, arg_names, &v_params);
			if (err._) { return err; }

			vector_free(&v_params);
			goto start;
		} else {
			err = apply(fn, &v_params, result);
			vector_free(&v_params);
		}

		stack_restore_add(ss, *result);
		return err;
	}
}

Error env_get(Noun env, char* symbol, Noun* result) {
	while (1) {
		Table* ptbl = cdr(env).value.Table;
		Table_entry* a = table_get_sym(ptbl, symbol);
		if (a) {
			*result = a->v;
			return MakeErrorCode(OK);
		}

		if (isnil(car(env))) { return MakeErrorCode(ERROR_UNBOUND); }

		env = car(env);
	}
}

Error env_assign(Noun env, char* symbol, Noun value) {
	Table* ptbl = cdr(env).value.Table;
	return table_set_sym(ptbl, symbol, value);
}

Error destructuring_bind(Noun arg_name, Noun val, Noun env) {
	Error err;
	if (isnil(arg_name)) {
		if (isnil(val)) {
			return MakeErrorCode(OK);
		} else {
			return MakeErrorCode(ERROR_ARGS);
		}
	} else if (arg_name.type == noun_t) {
		return env_assign(env, arg_name.value.symbol, val);
	} else if (arg_name.type == pair_t) {
		if (val.type != pair_t) { return MakeErrorCode(ERROR_ARGS); }

		err = destructuring_bind(car(arg_name), car(val), env);
		if (err._) { return err; }

		return destructuring_bind(cdr(arg_name), cdr(val), env);
	} else {
		return MakeErrorCode(ERROR_ARGS);
	}
}

Error env_bind(Noun env, Noun arg_names, Vector* v_params) {
	Noun arg_name, val;
	int val_unspecified = 0;
	Error err;

	size_t i = 0;
	while (!isnil(arg_names)) {
		if (arg_names.type == noun_t) {
			env_assign(env,
				   arg_names.value.symbol,
				   vector_to_noun(v_params, i));
			i = v_params->size;
			break;
		}

		arg_name = car(arg_names);

		if (i < v_params->size) {
			val = v_params->data[i];
		} else {
			val = nil;
			val_unspecified = 1;
		}

		err = destructuring_bind(arg_name, val, env);
		if (err._) { return err; }

		arg_names = cdr(arg_names);
		i++;
	}

	if (i < v_params->size) { return MakeErrorCode(ERROR_ARGS); }

	return MakeErrorCode(OK);
}

Noun env_create(Noun parent, size_t capacity) {
	return cons(parent, new_table(capacity));
}

void garbage_collector_run() {
	Pair *a, **p;
	struct str *as, **ps;
	Table *at, **pt;
	size_t i;

	for (i = 0; i < stack_size; i++) { garbage_collector_tag(stack[i]); }

	alloc_count_old = 0;

	p = &pair_head;
	while (*p != NULL) {
		a = *p;
		if (!a->mark) {
			*p = a->next;
			free(a);
		} else {
			p = &a->next;
			a->mark = 0;
			alloc_count_old++;
		}
	}

	ps = &str_head;
	while (*ps != NULL) {
		as = *ps;
		if (!as->mark) {
			*ps = as->next;
			free(as->value);
			free(as);
		} else {
			ps = &as->next;
			as->mark = 0;
			alloc_count_old++;
		}
	}

	pt = &table_head;
	while (*pt != NULL) {
		at = *pt;
		if (!at->mark) {
			*pt = at->next;
			for (i = 0; i < at->capacity; i++) {
				Table_entry* e = at->data[i];
				while (e) {
					Table_entry* next = e->next;
					free(e);
					e = next;
					/* If you're reading this,
					 * please go to sleep;
					 * it's late */
				}
			}

			free(at->data);
			free(at);
		} else {
			pt = &at->next;
			at->mark = 0;
			alloc_count_old++;
		}
	}

	alloc_count = alloc_count_old;
}

void garbage_collector_consider() {
	if (alloc_count > 2 * alloc_count_old) { garbage_collector_run(); }
}

void garbage_collector_tag(Noun root) {
	Pair* a;
	struct str* as;
	Table* at;
	size_t i;
	Table_entry* e;
start:
	switch (root.type) {
		case pair_t:
		case closure_t:
		case macro_t:
			a = root.value.pair_v;
			if (a->mark) return;
			a->mark = 1;
			garbage_collector_tag(car(root));

			root = cdr(root);
			goto start;
			break;
		case string_t:
			as = root.value.str;
			if (as->mark) return;
			as->mark = 1;
			break;
		case table_t: {
			at = root.value.Table;
			if (at->mark) return;
			at->mark = 1;
			for (i = 0; i < at->capacity; i++) {
				e = at->data[i];
				while (e) {
					garbage_collector_tag(e->k);
					garbage_collector_tag(e->v);
					e = e->next;
				}
			}

			break;
		}
		default: return;
	}
}

Error macex(Noun expr, Noun* result) {
	Error err = MakeErrorCode(OK);
	Noun args, op, result2;
	Vector v_params;
	int ss;
	cur_expr = expr;

	if (expr.type != pair_t || !listp(expr)) {
		*result = expr;
		return MakeErrorCode(OK);
	} else {
		ss = stack_size;
		op = car(expr);

		if (op.type == noun_t
		    && op.value.symbol == sym_quote.value.symbol) {
			*result = expr;
			return MakeErrorCode(OK);
		}

		args = cdr(expr);

		if (op.type == noun_t
		    && !env_get(env, op.value.symbol, result)._
		    && result->type == macro_t) {

			op = *result;

			op.type = closure_t;

			noun_to_vector(args, &v_params);
			err = apply(op, &v_params, &result2);
			if (err._) {
				vector_free(&v_params);
				stack_restore(ss);
				return err;
			}

			err = macex(result2, result);
			if (err._) {
				vector_free(&v_params);
				stack_restore(ss);
				return err;
			}

			vector_free(&v_params);
			stack_restore_add(ss, *result);
			return MakeErrorCode(OK);
		} else {

			Noun expr2 = copy_list(expr);
			Noun h;
			for (h = expr2; !isnil(h); h = cdr(h)) {
				err = macex(car(h), &car(h));
				if (err._) {
					stack_restore(ss);
					return err;
				}
			}

			*result = expr2;
			stack_restore_add(ss, *result);
			return MakeErrorCode(OK);
		}
	}
}

char* to_string(Noun a, bool write) {
	char *s = str_new(), *s2, buf[80];
	Noun a2;
	switch (a.type) {
		case nil_t: append_string(&s, "Nil"); break;
		case pair_t: {
			if (listp(a) && list_len(a) == 2
			    && eq_h(car(a), sym_quote)) {
				append_string(&s, "'");
				s2 = to_string(car(cdr(a)), write);
				append_string(&s, s2);
				free(s2);
			} else if (listp(a) && list_len(a) == 2
				   && eq_h(car(a), sym_quasiquote)) {
				append_string(&s, "`");
				s2 = to_string(car(cdr(a)), write);
				append_string(&s, s2);
				free(s2);
			} else if (listp(a) && list_len(a) == 2
				   && eq_h(car(a), sym_unquote)) {
				append_string(&s, ",");
				s2 = to_string(car(cdr(a)), write);
				append_string(&s, s2);
				free(s2);
			} else if (listp(a) && list_len(a) == 2
				   && eq_h(car(a), sym_unquote_splicing)) {
				append_string(&s, ",@");
				s2 = to_string(car(cdr(a)), write);
				append_string(&s, s2);
				free(s2);
			} else {
				append_string(&s, "(");
				s2 = to_string(car(a), write);
				append_string(&s, s2);
				free(s2);
				a = cdr(a);
				while (!isnil(a)) {
					if (a.type == pair_t) {
						append_string(&s, " ");
						s2 = to_string(car(a), write);
						append_string(&s, s2);
						free(s2);
						a = cdr(a);
					} else {
						append_string(&s, " . ");
						s2 = to_string(a, write);
						append_string(&s, s2);
						free(s2);
						break;
					}
				}

				append_string(&s, ")");
			}

			break;
		}
		case noun_t: append_string(&s, a.value.symbol); break;
		case string_t:
			if (write) append_string(&s, "\"");
			append_string(&s, a.value.str->value);
			if (write) append_string(&s, "\"");
			break;
		case i32_t:
			sprintf(buf, "%ld", a.value.integer_v);
			append_string(&s, buf);
			break;
		case f64_t:
			sprintf(buf, "%.16g", a.value.number_v);
			append_string(&s, buf);
			break;
		case builtin_t:
			(void*)a.value.builtin ? sprintf(buf, "Builtin")
					       : sprintf(buf, "Internal");

			append_string(&s, buf);
			break;
		case closure_t: {
			a2 = cons(sym_fn, cdr(a));
			s2 = to_string(a2, write);
			append_string(&s, s2);
			free(s2);
			break;
		}
		case macro_t:
			append_string(&s, "Macro:");
			s2 = to_string(cdr(a), write);
			append_string(&s, s2);
			free(s2);
			append_string(&s, ">");
			break;
		case input_t: append_string(&s, "Input"); break;
		case output_t: append_string(&s, "Output"); break;
		case type_t:
			append_string(&s, "@");
			append_string(&s, type_to_string(a.value.type_v));
			break;
		case bool_t:
			append_string(&s, a.value.bool_v ? "True" : "False");
			break;
		case error_t:
			append_string(&s, error_to_string(a.value.error_v));
			break;
		default: append_string(&s, ":Unknown"); break;
	}

	return s;
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

Error macex_eval(Noun expr, Noun* result) {
	Noun e0;
	Error err = macex(expr, &e0);
	if (err._) { return err; }

	return eval_expr(e0, env, result);
}

Result um_load_file(const char* path) {
	char* text = read_file(path);
	if (text) {
		Result err = um_interpret_string(text);
		free(text);
		return err;
	} else {
		return (Result){.error = MakeErrorCode(ERROR_FILE),
				.data = nil};
	}
}

void print_help(FILE* stream) {
	fprintf(
	    stream,
	    "Usage: um [options] file\n"
	    "    -nogc\tdisable garbage collection\n"
	    "    -debug\tenable debug logging during runtime\n"
	    "    -h\t\tyou_are_here.png\n"
	    "    -repl\tstart REPL, to be used in conjuction with file input\n"
	    "    -ss <uint>\tsymbol table size, default 1,000\n"
	    "    -v\t\tversion, but don't bother\n");
}
#define DECLARE_BUILTIN(name) \
	Error builtin_##name(Vector* v_params, Noun* result)

DECLARE_BUILTIN(car);
DECLARE_BUILTIN(cdr);
DECLARE_BUILTIN(cons);
DECLARE_BUILTIN(apply);
DECLARE_BUILTIN(eq);
DECLARE_BUILTIN(eq_l);
DECLARE_BUILTIN(exit);
DECLARE_BUILTIN(macex);
DECLARE_BUILTIN(string);
DECLARE_BUILTIN(eval);
DECLARE_BUILTIN(int);
DECLARE_BUILTIN(print);
DECLARE_BUILTIN(add);
DECLARE_BUILTIN(subtract);
DECLARE_BUILTIN(multiply);
DECLARE_BUILTIN(divide);
DECLARE_BUILTIN(less);
DECLARE_BUILTIN(greater);
DECLARE_BUILTIN(trunc);
DECLARE_BUILTIN(sin);
DECLARE_BUILTIN(cos);
DECLARE_BUILTIN(tan);
DECLARE_BUILTIN(asin);
DECLARE_BUILTIN(acos);
DECLARE_BUILTIN(atan);
DECLARE_BUILTIN(not );
DECLARE_BUILTIN(pairp);
DECLARE_BUILTIN(type);
DECLARE_BUILTIN(float);
DECLARE_BUILTIN(cast);
DECLARE_BUILTIN(range);
DECLARE_BUILTIN(len);
DECLARE_BUILTIN(getlist);
DECLARE_BUILTIN(setlist);
DECLARE_BUILTIN(modulo);
DECLARE_BUILTIN(pow);
DECLARE_BUILTIN(cbrt);
DECLARE_BUILTIN(and);

#undef DECLARE_BUILTIN
void um_init() {
	srand((unsigned)time(0));
	if (!um_global_symbol_capacity) { um_global_symbol_capacity = 1000; }
	env = env_create(nil, um_global_symbol_capacity);

	symbol_table = malloc(um_global_symbol_capacity * sizeof(char*));

	sym_quote = intern("quote");
	sym_quasiquote = intern("quasiquote");
	sym_unquote = intern("unquote");
	sym_unquote_splicing = intern("unquote-splicing");
	sym_def = intern("def");
	sym_const = intern("const");
	sym_defun = intern("defun");
	sym_fn = intern("lambda");
	sym_if = intern("if");
	sym_cond = intern("cond");
	sym_switch = intern("switch");
	sym_match = intern("match");

	sym_mac = intern("mac");
	sym_apply = intern("apply");
	sym_cons = intern("cons");
	sym_string = intern("str");
	sym_num = intern("num");
	sym_int = intern("int");
	sym_char = intern("char");
	sym_do = intern("do");
	sym_set = intern("set");
	sym_true = intern("true");
	sym_false = intern("false");

	sym_nil_t = intern("@Nil");
	sym_pair_t = intern("@Pair");
	sym_noun_t = intern("@Noun");
	sym_f64_t = intern("@Float");
	sym_i32_t = intern("@Int");
	sym_builtin_t = intern("@Builtin");
	sym_closure_t = intern("@Closure");

	sym_macro_t = intern("@Macro");
	sym_string_t = intern("@String");
	sym_input_t = intern("@Input");
	sym_output_t = intern("@Output");
	sym_error_t = intern("@Error");
	sym_type_t = intern("@Type");
	sym_bool_t = intern("@Bool");

#define ASSIGN_BUILTIN(name, fn_ptr) \
	env_assign(env, intern(name).value.symbol, new_builtin(fn_ptr))

	env_assign(env, sym_true.value.symbol, new ((bool)true));
	env_assign(env, sym_false.value.symbol, new ((bool)false));
	env_assign(env, intern("nil").value.symbol, nil);

	env_assign(env, sym_nil_t.value.symbol, new ((noun_type)nil_t));
	env_assign(env, sym_pair_t.value.symbol, new ((noun_type)pair_t));
	env_assign(env, sym_noun_t.value.symbol, new ((noun_type)noun_t));
	env_assign(env, sym_f64_t.value.symbol, new ((noun_type)f64_t));
	env_assign(env, sym_i32_t.value.symbol, new ((noun_type)i32_t));
	env_assign(env, sym_builtin_t.value.symbol, new ((noun_type)builtin_t));
	env_assign(env, sym_closure_t.value.symbol, new ((noun_type)closure_t));
	env_assign(env, sym_macro_t.value.symbol, new ((noun_type)macro_t));
	env_assign(env, sym_string_t.value.symbol, new ((noun_type)string_t));
	env_assign(env, sym_input_t.value.symbol, new ((noun_type)input_t));
	env_assign(env, sym_output_t.value.symbol, new ((noun_type)output_t));
	env_assign(env, sym_error_t.value.symbol, new ((noun_type)error_t));
	env_assign(env, sym_type_t.value.symbol, new ((noun_type)type_t));
	env_assign(env, sym_bool_t.value.symbol, new ((noun_type)bool_t));

	ASSIGN_BUILTIN("car", builtin_car);
	ASSIGN_BUILTIN("cdr", builtin_cdr);
	ASSIGN_BUILTIN("cons", builtin_cons);

	ASSIGN_BUILTIN("+", builtin_add);
	ASSIGN_BUILTIN("-", builtin_subtract);
	ASSIGN_BUILTIN("*", builtin_multiply);
	ASSIGN_BUILTIN("/", builtin_divide);
	ASSIGN_BUILTIN("%", builtin_modulo);

	ASSIGN_BUILTIN(">", builtin_greater);
	ASSIGN_BUILTIN("<", builtin_less);
	ASSIGN_BUILTIN("=", builtin_eq);
	ASSIGN_BUILTIN("eq?", builtin_eq);

	ASSIGN_BUILTIN("eqv?", builtin_eq_l);
	ASSIGN_BUILTIN("__builtin_pow", builtin_pow);
	ASSIGN_BUILTIN("__builtin_cbrt", builtin_cbrt);
	ASSIGN_BUILTIN("int", builtin_int);
	ASSIGN_BUILTIN("not", builtin_not);
	ASSIGN_BUILTIN("sin", builtin_sin);
	ASSIGN_BUILTIN("__builtin_cos", builtin_cos);
	ASSIGN_BUILTIN("__builtin_tan", builtin_tan);
	ASSIGN_BUILTIN("__builtin_asin", builtin_asin);
	ASSIGN_BUILTIN("__builtin_acos", builtin_acos);
	ASSIGN_BUILTIN("__builtin_atan", builtin_atan);
	ASSIGN_BUILTIN("len", builtin_len);
	ASSIGN_BUILTIN("eval", builtin_eval);
	ASSIGN_BUILTIN("type", builtin_type);
	ASSIGN_BUILTIN("exit", builtin_exit);
	ASSIGN_BUILTIN("apply", builtin_apply);
	ASSIGN_BUILTIN("macex", builtin_macex);
	ASSIGN_BUILTIN("str", builtin_string);
	ASSIGN_BUILTIN("print", builtin_print);
	ASSIGN_BUILTIN("pair?", builtin_pairp);
	ASSIGN_BUILTIN("float", builtin_float);
	ASSIGN_BUILTIN("range", builtin_range);
	ASSIGN_BUILTIN("cast", builtin_cast);
	ASSIGN_BUILTIN("getlist", builtin_getlist);
	ASSIGN_BUILTIN("and", builtin_and);
	ASSIGN_BUILTIN("setlist", builtin_setlist);

	ASSIGN_BUILTIN("if", new_builtin(NULL).value.builtin);
	ASSIGN_BUILTIN("fn", new_builtin(NULL).value.builtin);
	ASSIGN_BUILTIN("do", new_builtin(NULL).value.builtin);
	ASSIGN_BUILTIN("def", new_builtin(NULL).value.builtin);
	ASSIGN_BUILTIN("const", new_builtin(NULL).value.builtin);
	ASSIGN_BUILTIN("mac", new_builtin(NULL).value.builtin);
	ASSIGN_BUILTIN("cond", new_builtin(NULL).value.builtin);
	ASSIGN_BUILTIN("switch", new_builtin(NULL).value.builtin);
	ASSIGN_BUILTIN("match", new_builtin(NULL).value.builtin);
	ASSIGN_BUILTIN("defun", new_builtin(NULL).value.builtin);
	ASSIGN_BUILTIN("quote", new_builtin(NULL).value.builtin);
	ASSIGN_BUILTIN("lambda", new_builtin(NULL).value.builtin);

	ingest("\
(def (foldl proc init list)\
	(if !(nil? list)\
		(foldl proc (proc init (car list)) (cdr list))\
		init))");

	ingest("\
(def (foldr p i l)\
	(if !(nil? l)\
		(p (car l) (foldr p i (cdr l)))\
		i))");

	ingest("\
(def (nil? x)\
	(= x ()))");

	ingest("\
(def (list . items)\
	(foldr cons nil items))");

	ingest("\
(def (unary-map proc list)\
	(foldr\
		(lambda (x rest) (cons (proc x) rest))\
		nil\
		list))");

	ingest("\
(def (map proc . arg-lists)\
	(if !(nil? (car arg-lists))\
		(cons\
			(apply proc (unary-map car arg-lists))\
			(apply map (cons proc (unary-map cdr arg-lists))))\
		nil))");

	ingest("\
(def (caar x)\
	(car (car x)))");

	ingest("\
(def (cadr x)\
	(car (cdr x)))");

	ingest("\
(mac unless (cond expr)\
	(list 'if condition () expr))");

	ingest("\
(def (append a b)\
	(foldr cons b a))");

	ingest("\
(mac quasiquote (x)\
 	(if (pair? x)\
  		(if (= (car x) 'unquote)\
   			(cadr x)\
    			(if (if (pair? (car x)) (= (caar x) 'unquote-splicing))\
     				(list 'append (cadr (car x)) (list 'quasiquote (cdr x)))\
     				(list 'cons (list 'quasiquote (car x)) (list 'quasiquote (cdr x)))))\
    		(list 'quote x)))");

	ingest("\
(mac let (defs . body)\
	`((lambda ,(map car defs) ,@body) ,@(map cadr defs)))");

	ingest("\
(defun std (fun)\
	(switch fun \
		('list list) \
		('map map) \
		('cast cast)))");

	ingest("\
(defun math (fun)\
	(switch fun\
		('pi 3.1415926535897931)\
		('e 2.7182818284590452)\
		('tan __builtin_tan)\
		('sin __builtin_sin)\
		('cos __builtin_cos)\
		('atan __builtin_atan)\
		('asin __builtin_asin)\
		('acos __builtin_acos)\
		('range range)\
		('sqrt (lambda (x) (math::pow x (float 0.5))))\
		('cbrt __builtin_cbrt)\
		('square (lambda (x) (math::pow x 2)))\
		('cube (lambda (x) (math::pow x 3)))\
		('min (lambda (x) \
			(if (nil? (cdr x))\
				(car x) \
				(foldl (lambda (a b) (if (< a b) a b)) (car x) (cdr x)))))\
		('max (lambda (x) \
			(if (nil? (cdr x))\
				(car x) \
				(foldl (lambda (a b) (if (< a b) b a)) (car x) (cdr x)))))\
		('pow __builtin_pow)))");

	ingest("\
(def (for-each proc items)\
  	(if (nil? items)\
   		true\
   		(if ((lambda (x) true) (proc (car items))) \
    			(for-each proc (cdr items)))))");

	ingest("\
(def (filter pred lst)\
   	(if (nil? lst)\
    		()\
    		(if (pred (car lst))\
     			(cons (car lst)\
     			(filter pred (cdr lst)))\
   	(filter pred (cdr lst)))))");

	ingest("\
(defun curry (f)\
	(lambda (a) (lambda (b) (f a b))))");
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

void um_print_expr(Noun a) {
	char* s = to_string(a, 1);
	printf("%s", s);
	free(s);
}

void um_print_error(Error e) {
	char* s = error_to_string(e);
	printf("%s", s);
	free(s);
}

void um_print_result(Result r) {
	char* e = error_to_string(r.error);
	char* d = to_string(r.data, 0);

	printf("%s%s\n", e, d);

	free(e);
	free(d);
}

bool eq_pair_l(Noun a, Noun b) {
	if (a.type != pair_t || b.type != pair_t) { return false; }

	return eq_l(car(a), car(b)) && eq_l(cdr(a), cdr(b));
}

bool eq_pair_h(Noun a, Noun b) {
	if (a.type != pair_t || b.type != pair_t) { return false; }

	return eq_h(car(a), car(b)) && eq_h(cdr(a), cdr(b));
}

bool eq_h(Noun a, Noun b) {
	if (a.type != b.type) { return false; }

	switch (a.type) {
		case nil_t: return isnil(a) && isnil(b);
		case i32_t: return a.value.integer_v == b.value.integer_v;
		case f64_t: return a.value.number_v == b.value.number_v;
		/* Equal symbols share memory */
		case noun_t: return a.value.symbol == b.value.symbol;
		case string_t:
			return !strcmp(a.value.str->value, b.value.str->value);
		case builtin_t: return a.value.builtin == b.value.builtin;
		case input_t:
		case output_t: return a.value.fp == b.value.fp;
		case type_t: return a.value.type_v == b.value.type_v;
		case bool_t: return a.value.bool_v == b.value.bool_v;
		case error_t: return a.value.error_v._ == b.value.error_v._;
		case pair_t:
		case macro_t:
		case closure_t:
			return eq_pair_h(a, b); /* Frequently recursive */
		default: return false;
	}
}

bool eq_l(Noun a, Noun b) {
	if (a.type == b.type) {
		return eq_h(a, b);
	} else {
		return eq_h(a, cast(b, a.type));
	}
}

size_t hash_code(Noun a) {
	size_t r = 1;
	switch (a.type) {
		case nil_t: return 0;
		case pair_t:
			while (!isnil(a)) {
				r *= 31;
				if (a.type == pair_t) {
					r += hash_code(car(a));
					a = cdr(a);
				} else {
					r += hash_code(a);
					break;
				}
			}

			return r;
		case noun_t: return hash_code_sym(a.value.symbol);
		case string_t: {
			char* v = a.value.str->value;
			for (; *v != 0; v++) {
				r *= 31;
				r += *v;
			}

			return r;
		}
		case f64_t:
			return (size_t)((void*)a.value.symbol)
			     + (size_t)a.value.number_v;
		case builtin_t: return (size_t)a.value.builtin;
		case closure_t: return hash_code(cdr(a));
		case macro_t: return hash_code(cdr(a));
		case input_t:
		case output_t: return (size_t)a.value.fp / sizeof(*a.value.fp);
		default: return 0;
	}
}

size_t hash_code_sym(char* s) {
	return (size_t)s / sizeof(s) / 2;
}

Noun new_table(size_t capacity) {
	Noun a;
	Table* s;
	size_t i;
	alloc_count++;
	s = a.value.Table = malloc(sizeof(Table));
	s->capacity = capacity;
	s->size = 0;
	s->data = malloc(capacity * sizeof(Table_entry*));
	for (i = 0; i < capacity; i++) { s->data[i] = NULL; }

	s->mark = 0;
	s->next = table_head;
	table_head = s;
	a.value.Table = s;
	a.type = table_t;
	stack_add(a);
	return a;
}

Table_entry* table_entry_new(Noun k, Noun v, Table_entry* next) {
	Table_entry* r = malloc(sizeof(*r));
	r->k = k;
	r->v = v;
	r->next = next;
	return r;
}

void table_add(Table* tbl, Noun k, Noun v) {
	Table_entry **b, **data2, *p, **p2, *next;
	size_t i, new_capacity;
	if (tbl->size + 1 > tbl->capacity) {
		new_capacity = (tbl->size + 1) * 2;
		data2 = malloc(new_capacity * sizeof(Table_entry*));
		for (i = 0; i < new_capacity; i++) { data2[i] = NULL; }

		for (i = 0; i < tbl->capacity; i++) {
			p = tbl->data[i];
			while (p) {
				p2 = &data2[hash_code(p->k) % new_capacity];
				next = p->next;
				*p2 = table_entry_new(p->k, p->v, *p2);
				free(p);
				p = next;
			}
		}

		free(tbl->data);
		tbl->data = data2;
		tbl->capacity = new_capacity;
	}

	b = &tbl->data[hash_code(k) % tbl->capacity];
	*b = table_entry_new(k, v, *b);
	tbl->size++;
}

Table_entry* table_get_sym(Table* tbl, char* k) {
	Table_entry* p;
	size_t pos;
	if (tbl->size == 0) { return NULL; }
	pos = hash_code_sym(k) % tbl->capacity;
	p = tbl->data[pos];
	while (p) {
		if (p->k.value.symbol == k) { return p; }
		p = p->next;
	}

	return NULL;
}

Error table_set_sym(Table* tbl, char* k, Noun v) {
	Table_entry* p = table_get_sym(tbl, k);
	Noun s = {noun_t, .value.symbol = NULL};
	if (p) {
		if (!p->v.mut) { return MakeErrorCode(ERROR_NOMUT); }
		p->v = v;
		return MakeErrorCode(OK);
	} else {
		s.value.symbol = k;
		table_add(tbl, s, v);
		return MakeErrorCode(OK);
	}
}

Error builtin_type(Vector* v_params, Noun* result) {
	if (v_params->size != 1) { return MakeErrorCode(ERROR_ARGS); }

	*result = new_type(v_params->data[0].type);
	return MakeErrorCode(OK);
}

Error builtin_getlist(Vector* v_params, Noun* result) {
	if (v_params->size != 2) { return MakeErrorCode(ERROR_ARGS); }
	if (v_params->data[0].type != i32_t) {
		return MakeError(
		    ERROR_TYPE,
		    "list_index: first parameter must be integer type");
	}
	if (!listp(v_params->data[1])) {
		return MakeError(ERROR_TYPE,
				 "list_index: second parameter must be list");
	}

	*result = *list_index(&v_params->data[1],
			      v_params->data[0].value.integer_v);
	return MakeErrorCode(OK);
}

Error builtin_setlist(Vector* v_params, Noun* result) {
	if (v_params->size != 3) { return MakeErrorCode(ERROR_ARGS); }
	if (v_params->data[0].type != i32_t) {
		return MakeError(
		    ERROR_TYPE,
		    "list_index: first parameter must be integer type");
	}
	if (!listp(v_params->data[1])) {
		return MakeError(ERROR_TYPE,
				 "list_index: second parameter must be list");
	}

	Noun* t
	    = list_index(&v_params->data[1], v_params->data[0].value.integer_v);
	t->type = v_params->data[2].type;
	t->value = v_params->data[2].value;
	*result = *t;
	return MakeErrorCode(OK);
}

Error builtin_len(Vector* v_params, Noun* result) {
	if (v_params->size != 1) { return MakeErrorCode(ERROR_ARGS); }

	if (listp(v_params->data[0])) {
		*result = new ((long)list_len(v_params->data[0]));
	} else if (v_params->data[0].type == string_t) {
		*result
		    = new ((long)strlen(v_params->data[0].value.str->value));
	} else {
		*result = new ((long)0);
		return MakeErrorCode(ERROR_TYPE);
	}

	return MakeErrorCode(OK);
}

Error builtin_range(Vector* v_params, Noun* result) {
	if (v_params->size > 2 || v_params->size < 1) {
		return MakeError(ERROR_ARGS,
				 "range: arg count must be nonzero below 3");
	}

	if (!isnumber(v_params->data[0]) || !isnumber(v_params->data[0])) {
		return MakeError(ERROR_TYPE,
				 "range: args must be type numeric");
	}

	long a = cast(v_params->data[0], i32_t).value.integer_v,
	     b = cast(v_params->data[1], i32_t).value.integer_v;

	Noun range = nil;

	if (a < b) {
		for (; a <= b; b--) { range = cons(new (b), range); }
	} else {
		for (; a >= b; b++) { range = cons(new (b), range); }
	}

	*result = range;
	return MakeErrorCode(OK);
}

Error builtin_car(Vector* v_params, Noun* result) {
	Noun a;
	if (v_params->size != 1) { return MakeErrorCode(ERROR_ARGS); }

	a = v_params->data[0];
	if (isnil(a)) {
		*result = nil;
	} else if (a.type != pair_t) {
		return MakeErrorCode(ERROR_TYPE);
	} else {
		*result = car(a);
	}

	return MakeErrorCode(OK);
}

Error builtin_cdr(Vector* v_params, Noun* result) {
	Noun a;
	if (v_params->size != 1) { return MakeErrorCode(ERROR_ARGS); }

	a = v_params->data[0];
	if (isnil(a)) {
		*result = nil;
	} else if (a.type != pair_t) {
		return MakeErrorCode(ERROR_TYPE);
	} else {
		*result = cdr(a);
	}

	return MakeErrorCode(OK);
}

Error builtin_cons(Vector* v_params, Noun* result) {
	if (v_params->size != 2) { return MakeErrorCode(ERROR_ARGS); }

	*result = cons(v_params->data[0], v_params->data[1]);

	return MakeErrorCode(OK);
}

Error builtin_apply(Vector* v_params, Noun* result) {
	Noun fn;
	Vector v;
	Error err;

	if (v_params->size != 2) { return MakeErrorCode(ERROR_ARGS); }

	fn = v_params->data[0];
	noun_to_vector(v_params->data[1], &v);
	err = apply(fn, &v, result);
	vector_free(&v);
	return err;
}

Error builtin_eq(Vector* v_params, Noun* result) {
	Noun a, b;
	size_t i;
	if (v_params->size <= 1) {
		*result = new ((bool)true);
		return MakeErrorCode(OK);
	}

	for (i = 0; i < v_params->size - 1; i++) {
		a = v_params->data[i];
		b = v_params->data[i + 1];
		if (!eq_h(a, b)) {
			*result = new ((bool)false);
			return MakeErrorCode(OK);
		}
	}

	*result = new ((bool)true);
	return MakeErrorCode(OK);
}

Error builtin_eq_l(Vector* v_params, Noun* result) {
	Noun a, b;
	size_t i;
	if (v_params->size <= 1) {
		*result = sym_true;
		return MakeErrorCode(OK);
	}

	for (i = 0; i < v_params->size - 1; i++) {
		a = v_params->data[i];
		b = v_params->data[i + 1];
		if (!eq_l(a, b)) {
			*result = new ((bool)false);
			return MakeErrorCode(OK);
		}
	}

	*result = new ((bool)true);
	return MakeErrorCode(OK);
}

Error builtin_pairp(Vector* v_params, Noun* result) {
	if (v_params->size != 1) { return MakeErrorCode(ERROR_ARGS); }

	*result = v_params->data[0].type == pair_t ? sym_true : nil;
	return MakeErrorCode(OK);
}

Error builtin_not(Vector* v_params, Noun* result) {
	if (v_params->size != 1) {
		*result = nil;
		return MakeErrorCode(ERROR_ARGS);
	}

	*result = cast(v_params->data[0], bool_t).value.bool_v
		    ? new ((bool)false)
		    : sym_true;
	return MakeErrorCode(OK);
}

Error builtin_exit(Vector* v_params, Noun* result) {
	Noun code = nil;
	if (v_params->size != 1) { return MakeErrorCode(ERROR_ARGS); }

	if (isnil(code = cast(v_params->data[0], i32_t))) {
		return MakeErrorCode(ERROR_TYPE);
	}

	*result = code;

	exit(code.value.integer_v);
}

Error builtin_macex(Vector* v_params, Noun* result) {
	long alen = v_params->size;
	if (alen == 1) {
		Error err = macex(v_params->data[0], result);
		return err;
	} else
		return MakeErrorCode(ERROR_ARGS);
	return MakeErrorCode(OK);
}

Error builtin_string(Vector* v_params, Noun* result) {
	char* s = str_new();
	size_t i;
	for (i = 0; i < v_params->size; i++) {
		if (!isnil(v_params->data[i])) {
			char* a = to_string(v_params->data[i], 0);
			append_string(&s, a);
			free(a);
		}
	}

	*result = new_string(s);
	return MakeErrorCode(OK);
}

Error builtin_print(Vector* v_params, Noun* result) {
	size_t i;
	for (i = 0; i < v_params->size; i++) {
		if (!isnil(v_params->data[i])) {
			puts(to_string(v_params->data[i], 0));
		}
	}

	*result = nil;
	return MakeErrorCode(OK);
}

Error builtin_eval(Vector* v_params, Noun* result) {
	if (v_params->size == 1)
		return macex_eval(v_params->data[0], result);
	else
		return MakeErrorCode(ERROR_ARGS);
}

Error builtin_int(Vector* v_params, Noun* result) {
	if (v_params->size == 1) {
		*result = cast(v_params->data[0], i32_t);

		return MakeErrorCode(OK);
	} else {
		return MakeErrorCode(ERROR_ARGS);
	}
}

Error builtin_cast(Vector* v_params, Noun* result) {
	if (v_params->size == 2) {

		*result = cast(v_params->data[0],
			       v_params->data[1].type != type_t
				   ? v_params->data[1].type
				   : v_params->data[1].value.type_v);

		return MakeErrorCode(OK);
	} else {
		*result = nil;
		return MakeErrorCode(ERROR_ARGS);
	}
}

Error builtin_float(Vector* v_params, Noun* result) {
	if (v_params->size == 1) {
		*result = cast(v_params->data[0], f64_t);

		return MakeErrorCode(OK);
	} else {
		return MakeErrorCode(ERROR_ARGS);
	}
}

Error builtin_and(Vector* v_params, Noun* result) {
	if (v_params->size != 2) { return MakeErrorCode(ERROR_ARGS); }

	*result = new ((bool)(cast(v_params->data[0], bool_t).value.bool_v
			      && cast(v_params->data[1], bool_t).value.bool_v));

	return MakeErrorCode(OK);
}

Error builtin_sin(Vector* v_params, Noun* result) {
	if (v_params->size == 1) {
		*result = new_number(
		    sin(cast(v_params->data[0], f64_t).value.number_v));
		return MakeErrorCode(OK);
	} else {
		return MakeErrorCode(ERROR_ARGS);
	}
}

Error builtin_asin(Vector* v_params, Noun* result) {
	if (v_params->size == 1) {
		*result = new_number(
		    asin(cast(v_params->data[0], f64_t).value.number_v));
		return MakeErrorCode(OK);
	} else {
		return MakeErrorCode(ERROR_ARGS);
	}
}

Error builtin_cos(Vector* v_params, Noun* result) {
	if (v_params->size == 1) {
		*result = new_number(
		    cos(cast(v_params->data[0], f64_t).value.number_v));
		return MakeErrorCode(OK);
	} else {
		return MakeErrorCode(ERROR_ARGS);
	}
}

Error builtin_acos(Vector* v_params, Noun* result) {
	if (v_params->size == 1) {
		*result = new_number(
		    acos(cast(v_params->data[0], f64_t).value.number_v));
		return MakeErrorCode(OK);
	} else {
		return MakeErrorCode(ERROR_ARGS);
	}
}

Error builtin_tan(Vector* v_params, Noun* result) {
	if (v_params->size == 1) {
		*result = new_number(
		    tan(cast(v_params->data[0], f64_t).value.number_v));
		return MakeErrorCode(OK);
	} else {
		return MakeErrorCode(ERROR_ARGS);
	}
}

Error builtin_atan(Vector* v_params, Noun* result) {
	if (v_params->size == 1) {
		*result = new_number(
		    atan(cast(v_params->data[0], f64_t).value.number_v));
		return MakeErrorCode(OK);
	} else {
		return MakeErrorCode(ERROR_ARGS);
	}
}

Error builtin_pow(Vector* v_params, Noun* result) {
	if (v_params->size != 2) { return MakeErrorCode(ERROR_ARGS); }

	double temp = pow(cast(v_params->data[0], f64_t).value.number_v,
			  cast(v_params->data[1], f64_t).value.number_v);

	*result = isnormal(temp) && (temp == floor(temp))
		    ? new_integer((long)temp)
		    : new_number(temp);

	return MakeErrorCode(OK);
}

Error builtin_cbrt(Vector* v_params, Noun* result) {
	if (v_params->size != 1) { return MakeErrorCode(ERROR_ARGS); }

	double temp = cbrt(cast(v_params->data[0], f64_t).value.number_v);

	*result = isnormal(temp) && (temp == floor(temp))
		    ? new_integer((long)temp)
		    : new_number(temp);

	return MakeErrorCode(OK);
}

#define intp(d) (bool)(isnormal(d) && (d == floor(d)))

Error builtin_add(Vector* v_params, Noun* result) {
	size_t ac = v_params->size;
	Noun a0 = v_params->data[0], a1 = v_params->data[1];
	if (ac == 1) {
		*result = a0.type == i32_t ? new (labs(a0.value.integer_v))
			: a0.type == f64_t ? new (fabs(a0.value.number_v))
					   : new ((long)0);
		return MakeErrorCode(OK);
	} else if (ac > 2 || ac < 1) {
		return MakeErrorCode(ERROR_ARGS);
	}

	if (a0.type == i32_t && a1.type == i32_t) {
		*result = new (a0.value.integer_v + a1.value.integer_v);
	} else {
		double _temp = cast(a0, f64_t).value.number_v
			     + cast(a1, f64_t).value.number_v;

		*result = intp(_temp) ? new ((long)_temp) : new (_temp);
	}

	return MakeErrorCode(OK);
}

Error builtin_subtract(Vector* v_params, Noun* result) {
	size_t ac = v_params->size;
	Noun a0 = v_params->data[0], a1 = v_params->data[1];
	if (ac == 1) {
		*result = a0.type == i32_t ? new (-labs(a0.value.integer_v))
			: a0.type == f64_t ? new (-fabs(a0.value.number_v))
					   : new ((long)0);
		return MakeErrorCode(OK);
	} else if (ac > 2 || ac < 1) {
		return MakeErrorCode(ERROR_ARGS);
	}

	if (a0.type == i32_t && a1.type == i32_t) {
		*result = new (a0.value.integer_v - a1.value.integer_v);
	} else {
		double _temp = cast(a0, f64_t).value.number_v
			     - cast(a1, f64_t).value.number_v;

		*result = intp(_temp) ? new ((long)_temp) : new (_temp);
	}

	return MakeErrorCode(OK);
}

Error builtin_modulo(Vector* v_params, Noun* result) {
	Noun a0 = v_params->data[0], a1 = v_params->data[1];
	if (v_params->size != 2) { return MakeErrorCode(ERROR_ARGS); }

	if (a0.type == i32_t && a1.type == i32_t) {
		*result = new (a0.value.integer_v % a1.value.integer_v);
	} else {
		long _temp = cast(a0, i32_t).value.integer_v
			   % cast(a1, i32_t).value.integer_v;

		*result = new (_temp);
	}

	return MakeErrorCode(OK);
}

Error builtin_multiply(Vector* v_params, Noun* result) {
	Noun a0 = v_params->data[0], a1 = v_params->data[1];
	if (v_params->size != 2) { return MakeErrorCode(ERROR_ARGS); }

	if (a0.type == i32_t && a1.type == i32_t) {
		*result = new (a0.value.integer_v * a1.value.integer_v);
	} else {
		double _temp = cast(a0, f64_t).value.number_v
			     * cast(a1, f64_t).value.number_v;

		*result = intp(_temp) ? new ((long)_temp) : new (_temp);
	}

	return MakeErrorCode(OK);
}

Error builtin_divide(Vector* v_params, Noun* result) {
	Noun a0 = v_params->data[0], a1 = v_params->data[1];
	if (v_params->size != 2) { return MakeErrorCode(ERROR_ARGS); }

	if (a0.type == i32_t && a1.type == i32_t) {
		*result = new (a0.value.integer_v / a1.value.integer_v);
	} else {
		double _temp = cast(a0, f64_t).value.number_v
			     / cast(a1, f64_t).value.number_v;

		*result = intp(_temp) ? new ((long)_temp) : new (_temp);
	}

	return MakeErrorCode(OK);
}

Error builtin_less(Vector* v_params, Noun* result) {
	Noun a0 = v_params->data[0], a1 = v_params->data[1];
	if (v_params->size != 2) { return MakeErrorCode(ERROR_ARGS); }

	if (a0.type == i32_t && a1.type == i32_t) {
		*result = new ((bool)(a0.value.integer_v < a1.value.integer_v));
	} else {
		*result = new ((bool)(cast(a0, f64_t).value.number_v
				      < cast(a1, f64_t).value.number_v));
	}

	return MakeErrorCode(OK);
}

Error builtin_greater(Vector* v_params, Noun* result) {
	Noun a0 = v_params->data[0], a1 = v_params->data[1];
	if (v_params->size != 2) { return MakeErrorCode(ERROR_ARGS); }

	if (a0.type == i32_t && a1.type == i32_t) {
		*result = new ((bool)(a0.value.integer_v > a1.value.integer_v));
	} else {
		*result = new ((bool)(cast(a0, f64_t).value.number_v
				      > cast(a1, f64_t).value.number_v));
	}

	return MakeErrorCode(OK);
}

#endif
