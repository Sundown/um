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
	noreturn_t,
	pair_t,
	noun_t,
	real_t,
	builtin_t,
	closure_t,
	macro_t,
	vector_t,
	string_t,
	input_t,
	output_t,
	table_t,
	error_t,
	type_t,
	bool_t
} um_NounType;

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
} um_ErrorCode;

typedef struct {
	um_ErrorCode _;
	char* message;
} um_Error;

static const char* error_string[] = {"",
				     "Syntax error",
				     "Symbol unbound",
				     "Parameter error",
				     "Type error",
				     "File error",
				     "",
				     "Cannot mutate constant",
				     "Coercion error"};

typedef struct um_Noun um_Noun;
typedef struct um_Vector um_Vector;

typedef um_Error (*um_Builtin)(struct um_Vector* v_params,
			       struct um_Noun* result);

struct um_Noun {
	um_NounType type;
	bool mut;
	union {
		um_NounType type_v;
		bool bool_v;
		char character;
		um_Error error_v;
		double number;
		struct um_Pair* pair;
		char* symbol;
		struct um_String* string;
		FILE* fp;
		um_Builtin builtin;
		um_Vector* vector_v;
		struct um_Table* table;
		um_Error err_v;
	} value;
};

/* This should be returned by any functions which a user may interract with
 * directly */
struct um_Result {
	um_Error error;
	um_Noun data;
};
typedef struct um_Result um_Result;

struct um_Pair {
	struct um_Noun car, cdr;
	char mark;
	struct um_Pair* next;
};
typedef struct um_Pair um_Pair;

struct um_TableEntry {
	um_Noun k, v;
	struct um_TableEntry* next;
};
typedef struct um_TableEntry um_TableEntry;

struct um_Table {
	size_t capacity;
	size_t size;
	um_TableEntry** data;
	char mark;
	struct um_Table* next;
};
typedef struct um_Table um_Table;

struct um_Vector {
	um_Noun* data;
	um_Noun static_data[8];
	size_t capacity, size;
};

struct um_String {
	char* value;
	char mark;
	struct um_String* next;
};

static const um_Noun nil
    = {.type = nil_t, .mut = false, .value = {.type_v = nil_t}};

static const um_Noun um_noreturn
    = {.type = noreturn_t, .mut = false, .value = {.type_v = noreturn_t}};

um_Noun sym_quote, sym_const, sym_quasiquote, sym_unquote, sym_unquote_splicing,
    sym_def, sym_set, sym_defun, sym_fn, sym_if, sym_cond, sym_switch,
    sym_match, sym_mac, sym_apply, sym_cons, sym_string, sym_num, sym_char,
    sym_do, sym_true, sym_false,

    sym_nil_t, sym_pair_t, sym_noun_t, sym_f64_t, sym_builtin_t, sym_closure_t,
    sym_macro_t, sym_string_t, sym_vector_t, sym_input_t, sym_output_t,
    sym_error_t, sym_type_t, sym_bool_t;

um_Noun env;
static size_t stack_capacity = 0;
static size_t stack_size = 0;
static um_Noun* stack = NULL;
static um_Pair* pair_head = NULL;
static struct um_String* str_head = NULL;
static um_Table* table_head = NULL;
static size_t alloc_count = 0;
static size_t alloc_count_old = 0;
char** symbol_table;
size_t symbol_size;
size_t um_global_symbol_capacity;
um_Noun cur_expr;

/* clang-format off */
#define car(p)	    	((p).value.pair->car)
#define cdr(p)	    	((p).value.pair->cdr)
#define cdr2(p)	    	(cdr(p))
#define pop(n)	    	(n = cdr2(n))
#define isnil(n)	((n).type == nil_t || (n).type == noreturn_t)

#define ingest(s) do { um_Result _0 = um_interpret_string(s); \
	if (_0.error._) { um_print_error(_0.error); }} while (0)

#define MakeErrorCode(c) (um_Error){c, NULL}
#define MakeError(c, m)  (um_Error){c, m}
#define new(T) _Generic((T),  	\
	bool: new_bool,      	\
	char*: new_string,   	\
	double: new_number,  	\
	um_Builtin: new_builtin,\
	um_NounType: new_type  	\
)(T)

inline um_Noun new_number(double x) { return (um_Noun){real_t, true, {.number = x}}; }
inline um_Noun new_builtin(um_Builtin fn) { return (um_Noun){builtin_t, true, {.builtin = fn}}; }
inline um_Noun new_type(um_NounType t) { return (um_Noun){type_t, true, {.type_v = t}}; }
inline um_Noun new_bool(bool b) { return (um_Noun){bool_t, true, {.bool_v = b}}; }
inline um_Noun new_vector(um_Vector* v) { return (um_Noun){vector_t, true, {.vector_v = v}};}
/* clang-format on */

/*
	Begin necessary forward declarations
*/
um_Noun cons(um_Noun car_val, um_Noun cdr_val);
um_Noun intern(const char* buf);
um_Noun new_string(char* x);

void stack_add(um_Noun a);

um_Noun real_to_t(double x, um_NounType t);
um_Noun noun_to_t(char* x, um_NounType t);
um_Noun string_to_t(char* x, um_NounType t);
um_Noun bool_to_t(bool x, um_NounType t);
um_Noun type_to_t(um_NounType x, um_NounType t);
um_Noun nil_to_t(um_Noun x __attribute__((unused)), um_NounType t);

bool listp(um_Noun expr);
um_Noun reverse_list(um_Noun list);

um_Error macex_eval(um_Noun expr, um_Noun* result);
um_Error eval_expr(um_Noun expr, um_Noun env, um_Noun* result);

um_Noun env_create(um_Noun parent, size_t capacity);
um_Error env_bind(um_Noun env, um_Noun arg_names, um_Vector* v_params);
um_Error env_assign(um_Noun env, char* symbol, um_Noun value);
um_Error env_get(um_Noun env, char* symbol, um_Noun* result);

um_Noun new_table(size_t capacity);
um_TableEntry* table_get_sym(um_Table* tbl, char* k);
um_Error table_set_sym(um_Table* tbl, char* k, um_Noun v);

void garbage_collector_consider();
void garbage_collector_tag(um_Noun root);

void um_print_expr(um_Noun a);
void um_print_error(um_Error e);
void um_print_result(um_Result r);

size_t hash_code_sym(char* s);

char* um_new_string();
char* to_string(um_Noun a, bool write);
char* append_string(char** dst, char* src);

bool eq_l(um_Noun a, um_Noun b);
bool eq_h(um_Noun a, um_Noun b);
bool eq_pair_l(um_Noun a, um_Noun b);
bool eq_pair_h(um_Noun a, um_Noun b);

char* readline_fp(char* prompt, FILE* fp);
um_Error read_expr(const char* input, const char** end, um_Noun* result);

um_Noun cons(um_Noun car_val, um_Noun cdr_val) {
	um_Pair* a;
	um_Noun p;
	alloc_count++;

	a = (um_Pair*)calloc(1, sizeof(um_Pair));
	a->mark = 0;
	a->next = pair_head;
	pair_head = a;

	p.type = pair_t;
	p.value.pair = a;

	car(p) = car_val;
	cdr(p) = cdr_val;

	stack_add(p);

	return p;
}

void vector_new(um_Vector* a) {
	a->capacity = sizeof(a->static_data) / sizeof(a->static_data[0]);
	a->size = 0;
	a->data = a->static_data;
}

void vector_add(um_Vector* a, um_Noun item) {
	if (a->size + 1 > a->capacity) {
		a->capacity *= 2;
		if (a->data == a->static_data) {
			a->data
			    = (um_Noun*)calloc(a->capacity, sizeof(um_Noun));
			memcpy(
			    a->data, a->static_data, a->size * sizeof(um_Noun));
		} else {
			a->data = (um_Noun*)realloc(
			    a->data, a->capacity * sizeof(um_Noun));
		}
	}

	a->data[a->size] = item;
	a->size++;
}

void vector_clear(um_Vector* a) {
	a->size = 0;
}

void vector_free(um_Vector* a) {
	if (a->data != a->static_data) free(a->data);
}

void noun_to_vector(um_Noun a, um_Vector* v) {
	vector_new(v);
	for (; !isnil(a); a = cdr(a)) { vector_add(v, car(a)); }
}

um_Noun vector_to_noun(um_Vector* a, size_t start) {
	um_Noun r = nil;
	size_t i;
	for (i = start; i < a->size; i++) {
		if (!isnil(a->data[i])) { r = cons(a->data[i], r); }
	}

	return reverse_list(r);
}

void um_repl() {
	char* input;

	while ((input = readline_fp(_REPL_PROMPT, stdin))) {
		const char* p;
		char* line;
start:
		p = input;
		um_Noun expr;
		um_Error err = read_expr(p, &p, &expr);
		if (err._ == MakeErrorCode(ERROR_FILE)._) {
			line = readline_fp("	", stdin);
			if (!line) break;
			input = append_string(&input, "\n");
			input = append_string(&input, line);
			free(line);
			goto start;
		}

		if (!err._) {
			um_Noun result;
			while (1) {
				err = macex_eval(expr, &result);
				if (err._) {
					um_print_error(err);
					printf("Error in expression: %s\n",
					       to_string(expr, 1));
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

size_t list_len(um_Noun xs) {
	um_Noun* p = &xs;
	size_t ret = 0;
	while (!isnil(*p)) {
		if (p->type != pair_t) { return ret + 1; }

		p = &cdr(*p);
		ret++;
	}

	return ret;
}

void stack_add(um_Noun a) {
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
		stack = (um_Noun*)realloc(stack,
					  stack_capacity * sizeof(um_Noun));
	}

	stack[stack_size - 1] = a;
}

void stack_restore(int saved_size) {
	stack_size = saved_size;
	if (stack_size < stack_capacity / 4) {
		stack_capacity = stack_size * 2;
		stack = (um_Noun*)realloc(stack,
					  stack_capacity * sizeof(um_Noun));
	}

	garbage_collector_consider();
}

void stack_restore_add(int saved_size, um_Noun a) {
	stack_size = saved_size;
	if (stack_size < stack_capacity / 4) {
		stack_capacity = stack_size * 2;
		stack = (um_Noun*)realloc(stack,
					  stack_capacity * sizeof(um_Noun));
	}

	stack_add(a);
	garbage_collector_consider();
}

um_Noun cast(um_Noun a, um_NounType t) {
	if (a.type == t) { return a; }

	switch (a.type) {
		case nil_t:
		case noreturn_t: return nil_to_t(nil, t);
		case real_t: return real_to_t(a.value.number, t);
		case noun_t: return noun_to_t(a.value.symbol, t);
		case string_t: return string_to_t(a.value.string->value, t);
		case bool_t: return bool_to_t(a.value.bool_v, t);
		case type_t: return type_to_t(a.value.type_v, t);
		default:
			return nil; /* TODO can probably add more
				       coercions for semi-primitive
				       types */
	}
}

char* type_to_string(um_NounType a) {
	switch (a) {
		case nil_t: return "Nil";
		case pair_t: return "Pair";
		case string_t: return "String";
		case noun_t: return "Noun";
		case real_t: return "Float";
		case builtin_t: return "Builtin";
		case closure_t: return "Closure";
		case macro_t: return "Macro";
		case input_t: return "Input";
		case output_t: return "Output";
		case table_t: return "Table";
		case bool_t: return "Bool";
		case type_t: return "Type";
		case error_t: return "Error";
		case vector_t: return "Vector";
		default: return "Unknown";
	}
}

char* error_to_string(um_Error e) {
	char* s = calloc(e.message != NULL ? strlen(e.message) : 0 + 27,
			 sizeof(char));
	e._ != MakeErrorCode(ERROR_USER)._&& e.message
	    ? sprintf(s, "%s\n%s\n", error_string[e._], e.message)
	    : sprintf(s, "%s\n", error_string[e._]);

	return s;
}

um_Noun nil_to_t(um_Noun x __attribute__((unused)), um_NounType t) {
	switch (t) {
		case nil_t: return nil;
		case real_t: return new_number(NAN);
		case pair_t: return cons(nil, nil);
		case bool_t: return new_bool(false);
		case type_t: return new_type(nil_t);
		case string_t: return new_string("nil");
		case noun_t: return intern("nil");
		default: return nil;
	}
}

um_Noun real_to_t(double x, um_NounType t) {
	if (t == real_t) { return new_number(x); }

	char* buf = NULL;
	if (t == noun_t || t == string_t) {
		buf = calloc(65, sizeof(char));
		snprintf(buf, 65, "%f", x);
		buf = realloc(buf, strlen(buf) * sizeof(char) + 1);
	}

	switch (t) {
		case nil_t: return nil;
		case noun_t: return intern(buf);
		case bool_t: return new_bool(x > 0 && isnormal(x) && !isnan(x));
		case pair_t: return cons(new_number(x), nil);
		case string_t: return new_string(buf);
		case type_t: return new_type(real_t);
		default: return nil;
	}
}

um_Noun noun_to_t(char* x, um_NounType t) {
	switch (t) {
		case pair_t: return cons(intern(x), nil);
		case noun_t: return intern(x);
		case real_t: return new_number(strtod(x, NULL));
		case string_t: return new_string(x);
		case type_t: return new_type(noun_t);
		case bool_t:
			return new_bool(x != NULL
					&& x != intern("nil").value.symbol);
		default: return nil;
	}
}

um_Noun string_to_t(char* x, um_NounType t) {
	switch (t) {
		case pair_t: return cons(intern(x), nil);
		case noun_t: return intern(x);
		case real_t: return new_number(strtod(x, NULL));
		case string_t: return new_string(x);
		case type_t: return new_type(noun_t);
		case bool_t:
			return new_bool(x != NULL && strcmp(x, "nil")
					&& strcmp(x, "false"));
		default: return nil;
	}
}

um_Noun bool_to_t(bool x, um_NounType t) {
	switch (t) {
		case bool_t: return new_bool(x);
		case pair_t: return cons(new_bool(x), nil);
		case real_t: return new_number((double)x);
		case noun_t: return x ? intern("true") : intern("false");
		case string_t:
			return x ? new_string("true") : new_string("false");
		case type_t: return new_type(bool_t);
		default: return nil;
	}
}

um_Noun type_to_t(um_NounType x, um_NounType t) {
	switch (t) {
		case type_t: return new_type(type_t);
		case noun_t: return intern(error_string[x]);
		case string_t: return new_string((char*)error_string[x]);
		case bool_t: return new_bool(!x);
		case pair_t: return cons(new_type(x), nil);
		default: return nil;
	}
}

um_Noun intern(const char* s) {
	um_Noun a;
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

um_Error new_closure(um_Noun env, um_Noun args, um_Noun body, um_Noun* result) {
	um_Noun p;

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

um_Noun new_string(char* x) {
	um_Noun a;
	struct um_String* s;
	alloc_count++;
	s = a.value.string = calloc(1, sizeof(struct um_String));
	s->value = x;
	s->mark = 0;
	s->next = str_head;
	str_head = s;

	a.type = string_t;
	a.mut = true;
	stack_add(a);

	return a;
}

um_Noun new_input(FILE* fp) {
	um_Noun a;
	a.type = input_t;
	a.value.fp = fp;
	return a;
}

um_Noun new_output(FILE* fp) {
	um_Noun a;
	a.type = output_t;
	a.value.fp = fp;
	return a;
}

um_Result um_interpret_string(const char* text) {
	um_Error err = MakeErrorCode(OK);
	const char* p = text;
	um_Noun expr, result;
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

	return (um_Result){.error = err, .data = result};
}

um_Error um_lex(const char* um_String, const char** start, const char** end) {
start:
	um_String += strspn(um_String, " \t\r\n");

	if (um_String[0] == '\0') {
		*start = *end = NULL;
		return MakeErrorCode(ERROR_FILE);
	}

	*start = um_String;

	if (strchr("(){}[]'`!:&.", um_String[0]) != NULL) {
		*end = um_String + 1; /* Normal */
	} else if (um_String[0] == ',') {
		*end = um_String + (um_String[1] == '@' ? 2 : 1);
	} else if (um_String[0] == '"') {
		for (um_String++; *um_String != 0; um_String++) {
			if (*um_String == '\\') {
				um_String++;
			} else if (*um_String == '"') {
				break;
			}
		}

		*end = um_String + 1;
	} else if (um_String[0] == ';') {
		um_String += strcspn(um_String, "\n");
		goto start;
	} else {
		*end = um_String + strcspn(um_String, "(){}[] \t\r\n;");
	}

	return MakeErrorCode(OK);
}

um_Error parse_simple(const char* start, const char* end, um_Noun* result) {
	char *p, *buf, *pt;
	size_t length = end - start - 2;
	um_Error err;
	um_Noun a1, a2;
	long len, i;
	const char* ps;

	double val = strtod(start, &p);
	if (p == end) {
		result->type = real_t;
		result->value.number = val;
		return MakeErrorCode(OK);
	} else if (start[0] == '"') {
		result->type = string_t;
		buf = (char*)calloc(length + 1, sizeof(char));
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

	buf = calloc(end - start + 1, sizeof(char));
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
					   cons(cast(a1, real_t),
						cons(cast(a2, real_t), nil)));

				return MakeErrorCode(OK);
			}
		}

		*result = intern(buf);
	}

	free(buf);
	return MakeErrorCode(OK);
}

um_Error read_list(const char* start, const char** end, um_Noun* result) {
	um_Noun p;

	*end = start;
	p = *result = nil;

	for (;;) {
		const char* token;
		um_Noun item;
		um_Error err;

		err = um_lex(*end, &token, end);
		if (err._) { return err; }

		if (token[0] == ')') { return MakeErrorCode(OK); }

		/* TODO segfault occurs on (0 . 0), not located in
		 * following block */
		if (!isnil(p) && token[0] == '.' && *end - token == 1) {

			if (isnil(p)) { return MakeErrorCode(ERROR_SYNTAX); }
			err = read_expr(*end, end, &item);
			if (err._) { return err; }

			cdr(p) = item;

			err = um_lex(*end, &token, end);
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

um_Error read_prefix(const char* start, const char** end, um_Noun* result) {
	um_Noun p = *result = nil;
	*end = start;

	while (1) {
		const char* token;
		um_Noun item;
		um_Error err = um_lex(*end, &token, end);

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

um_Error read_block(const char* start, const char** end, um_Noun* result) {
	um_Noun p = *result = nil;
	*end = start;

	p = nil;

	while (1) {
		const char* token;
		um_Noun item;
		um_Error err = um_lex(*end, &token, end);

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

um_Error read_vector(const char* start, const char** end, um_Noun* result) {
	*result = nil;
	um_Vector* v = calloc(1, sizeof(um_Vector));
	vector_new(v);
	*end = start;

	while (1) {
		const char* token;
		um_Noun item;
		um_Error err = um_lex(*end, &token, end);

		if (err._) { return err; }

		if (token[0] == ']') {
			*result = new_vector(v);
			return MakeErrorCode(OK);
		}

		err = read_expr(token, end, &item);

		if (err._) { return err; }

		vector_add(v, item);
	}
}

um_Error read_expr(const char* input, const char** end, um_Noun* result) {
	char* token;
	um_Error err;

	err = um_lex(input, (const char**)&token, end);
	if (err._) { return err; }

	if (token[0] == '(') {
		return read_list(*end, end, result);
	} else if (token[0] == ')') {
		return MakeErrorCode(ERROR_SYNTAX);
	} else if (token[0] == '[') {
		return read_vector(*end, end, result);
	} else if (token[0] == ']') {
		return MakeErrorCode(ERROR_SYNTAX);
	} else if (token[0] == '{') {
		return read_block(*end, end, result);
	} else if (token[0] == '}') {
		return MakeErrorCode(ERROR_SYNTAX);
	} /* else if (token[0] == '[') {
		 um_Noun n0, n1;
		 um_Error e0 = read_prefix(*end, end, &n0);
		 if (e0._) { return e0; }
		 n0 = car(cdr(n0));
		 e0 = eval_expr(n0, env, &n1);
		 if (e0._) { return e0; }
		 switch (n1.type) {
			 case type_t: {
				 *result = cons(intern("cast"),
						cons(nil, cons(n1, nil)));
				 return read_expr(*end, end,
	 &car(cdr(*result)));
			 }
			 default:
				 return MakeError(
				     ERROR_ARGS,
				     "prefix: was not passed a valid prefix");
		 }

	 } else if (token[0] == ']') {
		 return MakeErrorCode(ERROR_SYNTAX);
	 }*/
	else if (token[0] == '\'') {
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

um_Error apply(um_Noun fn, um_Vector* v_params, um_Noun* result) {
	um_Noun arg_names, env, body, a;
	um_Error err;
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

		index = (size_t)(v_params->data[0]).value.number;
		*result
		    = new_string((char[]){fn.value.string->value[index], '\0'});
		return MakeErrorCode(OK);
	} else if (fn.type == pair_t && listp(fn)) {
		if (v_params->size != 1) { return MakeErrorCode(ERROR_ARGS); }

		if (v_params->data[0].type != real_t) {
			return MakeErrorCode(ERROR_TYPE);
		}

		index = (size_t)(v_params->data[0]).value.number;
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

um_Noun copy_list(um_Noun list) {
	um_Noun a, p;

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

um_Noun* list_index(um_Noun* list, size_t index) {
	size_t i;
	um_Noun a = *list;
	for (i = 0; i < index; i++) {
		pop(a);
		if (isnil(a)) { return (um_Noun*)&nil; }
	}

	return &car(a);
}

um_Error env_assign_eq(um_Noun env, char* symbol, um_Noun value) {
	while (1) {
		um_Noun parent = car(env);
		um_Table* ptbl = cdr(env).value.table;
		um_TableEntry* a = table_get_sym(ptbl, symbol);
		if (a) {
			if (!a->v.mut) { return MakeErrorCode(ERROR_NOMUT); }
			a->v = value;
			return MakeErrorCode(OK);
		}

		if (isnil(parent)) { return env_assign(env, symbol, value); }

		env = parent;
	}
}

um_Noun reverse_list(um_Noun list) {
	um_Noun tail = nil;

	while (!isnil(list)) {
		tail = cons(car(list), tail);
		pop(list);
	}

	return tail;
}

bool listp(um_Noun expr) {
	um_Noun* p = &expr;
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

um_Error eval_expr(um_Noun expr, um_Noun env, um_Noun* result) {
	um_Error err;
	um_Noun fn, op, cond, args, sym, val, name, macro, p, r, arg_names;
	size_t ss = stack_size;
	um_Vector v_params;
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
				p = args;
				while (!isnil(p)) {
					if (isnil(cdr(p))
					    || (cdr(p).type == bool_t
						&& cdr(p).value.bool_v
						       == false)) {

						expr = car(p);
						goto start;
					}

					err = eval_expr(car(p), env, &cond);
					if (err._) {
						stack_restore(ss);
						return err;
					}

					if (!isnil(cond)
					    && cast(cond, bool_t)
						   .value.bool_v) {

						expr = car(cdr(p));
						goto start;
					}

					p = cdr(cdr(p));
				}

				*result = nil;
				stack_restore_add(ss, *result);
				return MakeErrorCode(OK);
			} else if (op.value.symbol == sym_cond.value.symbol) {
				um_Noun list
				    = !isnil(args) ? reverse_list(args) : nil;
				um_Noun n = cons(nil, nil);
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
				um_Noun pred = !isnil(args) ? car(args) : nil;
				um_Noun list = !isnil(args) && !isnil(cdr(args))
						 ? reverse_list(cdr(args))
						 : nil;
				um_Noun n = cons(nil, nil);
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
				um_Noun pred = !isnil(args) ? car(args) : nil;
				um_Noun list = !isnil(args) && !isnil(cdr(args))
						 ? reverse_list(cdr(args))
						 : nil;
				um_Noun n = cons(nil, nil);

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
		p = args;
		while (!isnil(p)) {
			err = eval_expr(car(p), env, &r);
			if (err._) {
				vector_free(&v_params);
				stack_restore(ss);
				return err;
			}

			vector_add(&v_params, r);
			p = cdr(p);
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

um_Error env_get(um_Noun env, char* symbol, um_Noun* result) {
	while (1) {
		um_Table* ptbl = cdr(env).value.table;
		um_TableEntry* a = table_get_sym(ptbl, symbol);
		if (a) {
			*result = a->v;
			return MakeErrorCode(OK);
		}

		if (isnil(car(env))) { return MakeErrorCode(ERROR_UNBOUND); }

		env = car(env);
	}
}

um_Error env_assign(um_Noun env, char* symbol, um_Noun value) {
	um_Table* ptbl = cdr(env).value.table;
	return table_set_sym(ptbl, symbol, value);
}

um_Error destructuring_bind(um_Noun arg_name, um_Noun val, um_Noun env) {
	um_Error err;
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

um_Error env_bind(um_Noun env, um_Noun arg_names, um_Vector* v_params) {
	um_Noun arg_name, val;
	int val_unspecified = 0;
	um_Error err;
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

um_Noun env_create(um_Noun parent, size_t capacity) {
	return cons(parent, new_table(capacity));
}

void garbage_collector_run() {
	um_Pair *a, **p;
	struct um_String *as, **ps;
	um_Table *at, **pt;
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
				um_TableEntry* e = at->data[i];
				while (e) {
					um_TableEntry* next = e->next;
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

void garbage_collector_tag(um_Noun root) {
	um_Pair* a;
	struct um_String* as;
	um_Table* at;
	size_t i;
	um_TableEntry* e;
start:
	switch (root.type) {
		case pair_t:
		case closure_t:
		case macro_t:
			a = root.value.pair;
			if (a->mark) return;
			a->mark = 1;
			garbage_collector_tag(car(root));

			root = cdr(root);
			goto start;
			break;
		case string_t:
			as = root.value.string;
			if (as->mark) return;
			as->mark = 1;
			break;
		case table_t: {
			at = root.value.table;
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

um_Error macex(um_Noun expr, um_Noun* result) {
	um_Error err = MakeErrorCode(OK);
	um_Noun args, op, result2;
	um_Vector v_params;
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

			um_Noun expr2 = copy_list(expr);
			um_Noun h;
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

char* to_string(um_Noun a, bool write) {
	char *s = um_new_string(), *s2, buf[80];
	um_Noun a2;
	switch (a.type) {
		case nil_t: append_string(&s, "Nil"); break;
		case noreturn_t: break;
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
			append_string(&s, a.value.string->value);
			if (write) append_string(&s, "\"");
			break;
		case real_t:
			sprintf(buf, "%f", a.value.number);
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
		case vector_t: {
			char* l = to_string(vector_to_noun(a.value.vector_v, 0),
					    write);
			append_string(&s, l);
			s[0] = '[';
			s[strlen(s) - 1] = ']';

			break;
		}
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

char* um_new_string() {
	char* s = calloc(1, sizeof(char));
	s[0] = '\0';
	return s;
}

um_Error macex_eval(um_Noun expr, um_Noun* result) {
	um_Noun e0;
	um_Error err = macex(expr, &e0);
	if (err._) { return err; }
	return eval_expr(e0, env, result);
}

um_Result um_load_file(const char* path) {
	char* text = read_file(path);
	if (text) {
		um_Result err = um_interpret_string(text);
		free(text);
		return err;
	} else {
		return (um_Result){.error = MakeErrorCode(ERROR_FILE),
				   .data = nil};
	}
}

char* readline_fp(char* prompt, FILE* fp) {
	size_t size = 80;
	char* um_String;
	int ch;
	size_t len = 0;
	printf("%s", prompt);
	um_String = calloc(size, sizeof(char));
	if (!um_String) { return NULL; }

	while ((ch = fgetc(fp)) != EOF && ch != '\n') {
		um_String[len++] = ch;
		if (len == size) {
			void* p
			    = realloc(um_String, sizeof(char) * (size *= 2));
			if (!p) {
				free(um_String);
				return NULL;
			}

			um_String = p;
		}
	}

	if (ch == EOF && len == 0) {
		free(um_String);
		return NULL;
	}

	um_String[len++] = '\0';

	return realloc(um_String, sizeof(char) * len);
}

void um_print_expr(um_Noun a) {
	char* s = to_string(a, 1);
	printf("%s", s);
	free(s);
}

void um_print_error(um_Error e) {
	char* s = error_to_string(e);
	printf("%s", s);
	free(s);
}

void um_print_result(um_Result r) {
	char* e = error_to_string(r.error);
	char* d = to_string(r.data, 0);

	printf("%s%s\n", e, d);

	free(e);
	free(d);
}

bool eq_pair_l(um_Noun a, um_Noun b) {
	if (a.type != pair_t || b.type != pair_t) { return false; }

	return eq_l(car(a), car(b)) && eq_l(cdr(a), cdr(b));
}

bool eq_pair_h(um_Noun a, um_Noun b) {
	if (a.type != pair_t || b.type != pair_t) { return false; }

	return eq_h(car(a), car(b)) && eq_h(cdr(a), cdr(b));
}

bool eq_h(um_Noun a, um_Noun b) {
	if (a.type != b.type) { return false; }

	switch (a.type) {
		case nil_t: return isnil(a) && isnil(b);
		case real_t: return a.value.number == b.value.number;
		/* Equal symbols share memory */
		case noun_t: return a.value.symbol == b.value.symbol;
		case string_t:
			return !strcmp(a.value.string->value,
				       b.value.string->value);
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

bool eq_l(um_Noun a, um_Noun b) {
	if (a.type == b.type) {
		return eq_h(a, b);
	} else {
		return eq_h(a, cast(b, a.type));
	}
}

size_t hash_code(um_Noun a) {
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
			char* v = a.value.string->value;
			for (; *v != 0; v++) {
				r *= 31;
				r += *v;
			}

			return r;
		}
		case real_t:
			return (size_t)((void*)a.value.symbol)
			     + (size_t)a.value.number;
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

um_Noun new_table(size_t capacity) {
	um_Noun a;
	um_Table* s;
	size_t i;
	alloc_count++;
	s = a.value.table = calloc(1, sizeof(um_Table));
	s->capacity = capacity;
	s->size = 0;
	s->data = calloc(capacity, sizeof(um_TableEntry*));
	for (i = 0; i < capacity; i++) { s->data[i] = NULL; }

	s->mark = 0;
	s->next = table_head;
	table_head = s;
	a.value.table = s;
	a.type = table_t;
	stack_add(a);
	return a;
}

um_TableEntry* table_entry_new(um_Noun k, um_Noun v, um_TableEntry* next) {
	um_TableEntry* r = calloc(1, sizeof(*r));
	r->k = k;
	r->v = v;
	r->next = next;
	return r;
}

void table_add(um_Table* tbl, um_Noun k, um_Noun v) {
	um_TableEntry **b, **data2, *p, **p2, *next;
	size_t i, new_capacity;
	if (tbl->size + 1 > tbl->capacity) {
		new_capacity = (tbl->size + 1) * 2;
		data2 = calloc(new_capacity, sizeof(um_TableEntry*));
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

um_TableEntry* table_get_sym(um_Table* tbl, char* k) {
	um_TableEntry* p;
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

um_Error table_set_sym(um_Table* tbl, char* k, um_Noun v) {
	um_TableEntry* p = table_get_sym(tbl, k);
	um_Noun s = {noun_t, .value.symbol = NULL};
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

um_Error builtin_type(um_Vector* v_params, um_Noun* result) {
	if (v_params->size != 1) { return MakeErrorCode(ERROR_ARGS); }

	*result = new_type(v_params->data[0].type);
	return MakeErrorCode(OK);
}

um_Error builtin_getlist(um_Vector* v_params, um_Noun* result) {
	if (v_params->size != 2) { return MakeErrorCode(ERROR_ARGS); }
	if (v_params->data[0].type != real_t) {
		return MakeError(
		    ERROR_TYPE,
		    "list_index: first parameter must be number type");
	}
	if (!listp(v_params->data[1]) && v_params->data[1].type != vector_t) {
		return MakeError(
		    ERROR_TYPE,
		    "list_index: second parameter must be list or vector");
	}

	if (v_params->data[1].type != vector_t) {
		*result = v_params->data[1]
			      .value.vector_v
			      ->data[(size_t)v_params->data[0].value.number];
	} else {
		*result = *list_index(&v_params->data[1],
				      (size_t)v_params->data[0].value.number);
	}
	return MakeErrorCode(OK);
}

um_Error builtin_setlist(um_Vector* v_params, um_Noun* result) {
	if (v_params->size != 3) { return MakeErrorCode(ERROR_ARGS); }
	if (v_params->data[0].type != real_t) {
		return MakeError(
		    ERROR_TYPE,
		    "list_index: first parameter must be number type");
	}
	if (!listp(v_params->data[1])) {
		return MakeError(ERROR_TYPE,
				 "list_index: second parameter must be list");
	}
	um_Noun t = copy_list(v_params->data[1]);
	um_Noun* i = list_index(&t, v_params->data[0].value.number);
	i->type = v_params->data[2].type;
	i->value = v_params->data[2].value;
	*result = t;
	return MakeErrorCode(OK);
}

um_Error builtin_len(um_Vector* v_params, um_Noun* result) {
	if (v_params->size != 1) { return MakeErrorCode(ERROR_ARGS); }

	if (listp(v_params->data[0])) {
		*result = new ((double)list_len(v_params->data[0]));
	} else if (v_params->data[0].type == string_t) {
		*result = new (
		    (double)strlen(v_params->data[0].value.string->value));
	} else if (v_params->data[0].type == vector_t) {

		*result = new ((double)v_params->data[0].value.vector_v->size);
	} else {
		*result = new ((double)0);
		return MakeErrorCode(ERROR_TYPE);
	}

	return MakeErrorCode(OK);
}

um_Error builtin_range(um_Vector* v_params, um_Noun* result) {
	if (v_params->size > 2 || v_params->size < 1) {
		return MakeError(ERROR_ARGS,
				 "range: arg count must be nonzero below 3");
	}

	if (v_params->data[0].type != real_t
	    || (v_params->size > 1 && v_params->data[1].type != real_t)) {
		return MakeError(ERROR_TYPE,
				 "range: args must be type numeric");
	}

	double a = v_params->size > 1
		     ? cast(v_params->data[0], real_t).value.number
		     : 0,
	       b = v_params->size > 1
		     ? cast(v_params->data[1], real_t).value.number
		     : cast(v_params->data[0], real_t).value.number;

	um_Noun range = nil;

	if (a < b) {
		for (; a <= b; b--) { range = cons(new (b), range); }
	} else {
		for (; a >= b; b++) { range = cons(new (b), range); }
	}

	*result = range;
	return MakeErrorCode(OK);
}

um_Error builtin_car(um_Vector* v_params, um_Noun* result) {
	um_Noun a;
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

um_Error builtin_cdr(um_Vector* v_params, um_Noun* result) {
	um_Noun a;
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

um_Error builtin_cons(um_Vector* v_params, um_Noun* result) {
	if (v_params->size != 2) { return MakeErrorCode(ERROR_ARGS); }

	*result = cons(v_params->data[0], v_params->data[1]);

	return MakeErrorCode(OK);
}

um_Error builtin_apply(um_Vector* v_params, um_Noun* result) {
	um_Noun fn;
	um_Vector v;
	um_Error err;

	if (v_params->size != 2) { return MakeErrorCode(ERROR_ARGS); }

	fn = v_params->data[0];
	noun_to_vector(v_params->data[1], &v);
	err = apply(fn, &v, result);
	vector_free(&v);
	return err;
}

um_Error builtin_eq(um_Vector* v_params, um_Noun* result) {
	um_Noun a, b;
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

um_Error builtin_eq_l(um_Vector* v_params, um_Noun* result) {
	um_Noun a, b;
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

um_Error builtin_pairp(um_Vector* v_params, um_Noun* result) {
	if (v_params->size != 1) { return MakeErrorCode(ERROR_ARGS); }

	*result = v_params->data[0].type == pair_t ? sym_true : nil;
	return MakeErrorCode(OK);
}

um_Error builtin_not(um_Vector* v_params, um_Noun* result) {
	if (v_params->size != 1) {
		*result = nil;
		return MakeErrorCode(ERROR_ARGS);
	}

	*result = cast(v_params->data[0], bool_t).value.bool_v
		    ? new ((bool)false)
		    : sym_true;
	return MakeErrorCode(OK);
}

um_Error builtin_exit(um_Vector* v_params, um_Noun* result) {
	um_Noun code = nil;
	if (v_params->size != 1) { return MakeErrorCode(ERROR_ARGS); }

	if (isnil(code = cast(v_params->data[0], real_t))) {
		return MakeErrorCode(ERROR_TYPE);
	}

	*result = code;

	exit(code.value.number);
}

um_Error builtin_macex(um_Vector* v_params, um_Noun* result) {
	long alen = v_params->size;
	if (alen == 1) {
		um_Error err = macex(v_params->data[0], result);
		return err;
	} else
		return MakeErrorCode(ERROR_ARGS);
	return MakeErrorCode(OK);
}

um_Error builtin_vector(um_Vector* v_params, um_Noun* result) {
	um_Vector v;
	size_t i;

	vector_new(&v);
	for (i = 0; i < v_params->size; i++) {
		if (!isnil(v_params->data[i])) {
			vector_add(&v, v_params->data[i]);
		}
	}

	*result = new_vector(&v);
	return MakeErrorCode(OK);
}

um_Error builtin_string(um_Vector* v_params, um_Noun* result) {
	char* s = um_new_string();
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

um_Error builtin_print(um_Vector* v_params, um_Noun* result) {
	size_t i;
	for (i = 0; i < v_params->size; i++) {
		if (!isnil(v_params->data[i])) {
			puts(to_string(v_params->data[i], 0));
		}
	}

	*result = nil;
	return MakeErrorCode(OK);
}

um_Error builtin_eval(um_Vector* v_params, um_Noun* result) {
	if (v_params->size == 1)
		return macex_eval(v_params->data[0], result);
	else
		return MakeErrorCode(ERROR_ARGS);
}

um_Error builtin_cast(um_Vector* v_params, um_Noun* result) {
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

um_Error builtin_float(um_Vector* v_params, um_Noun* result) {
	if (v_params->size == 1) {
		*result = cast(v_params->data[0], real_t);

		return MakeErrorCode(OK);
	} else {
		return MakeErrorCode(ERROR_ARGS);
	}
}

um_Error builtin_and(um_Vector* v_params, um_Noun* result) {
	if (v_params->size != 2) { return MakeErrorCode(ERROR_ARGS); }

	*result = new ((bool)(cast(v_params->data[0], bool_t).value.bool_v
			      && cast(v_params->data[1], bool_t).value.bool_v));

	return MakeErrorCode(OK);
}

um_Error builtin_sin(um_Vector* v_params, um_Noun* result) {
	if (v_params->size == 1) {
		*result = new_number(
		    sin(cast(v_params->data[0], real_t).value.number));
		return MakeErrorCode(OK);
	} else {
		return MakeErrorCode(ERROR_ARGS);
	}
}

um_Error builtin_asin(um_Vector* v_params, um_Noun* result) {
	if (v_params->size == 1) {
		*result = new_number(
		    asin(cast(v_params->data[0], real_t).value.number));
		return MakeErrorCode(OK);
	} else {
		return MakeErrorCode(ERROR_ARGS);
	}
}

um_Error builtin_cos(um_Vector* v_params, um_Noun* result) {
	if (v_params->size == 1) {
		*result = new_number(
		    cos(cast(v_params->data[0], real_t).value.number));
		return MakeErrorCode(OK);
	} else {
		return MakeErrorCode(ERROR_ARGS);
	}
}

um_Error builtin_acos(um_Vector* v_params, um_Noun* result) {
	if (v_params->size == 1) {
		*result = new_number(
		    acos(cast(v_params->data[0], real_t).value.number));
		return MakeErrorCode(OK);
	} else {
		return MakeErrorCode(ERROR_ARGS);
	}
}

um_Error builtin_tan(um_Vector* v_params, um_Noun* result) {
	if (v_params->size == 1) {
		*result = new_number(
		    tan(cast(v_params->data[0], real_t).value.number));
		return MakeErrorCode(OK);
	} else {
		return MakeErrorCode(ERROR_ARGS);
	}
}

um_Error builtin_atan(um_Vector* v_params, um_Noun* result) {
	if (v_params->size == 1) {
		*result = new_number(
		    atan(cast(v_params->data[0], real_t).value.number));
		return MakeErrorCode(OK);
	} else {
		return MakeErrorCode(ERROR_ARGS);
	}
}

um_Error builtin_pow(um_Vector* v_params, um_Noun* result) {
	if (v_params->size != 2) { return MakeErrorCode(ERROR_ARGS); }

	double temp = pow(cast(v_params->data[0], real_t).value.number,
			  cast(v_params->data[1], real_t).value.number);

	*result = new_number(temp);

	return MakeErrorCode(OK);
}

um_Error builtin_cbrt(um_Vector* v_params, um_Noun* result) {
	if (v_params->size != 1) { return MakeErrorCode(ERROR_ARGS); }

	double temp = cbrt(cast(v_params->data[0], real_t).value.number);

	*result = new_number(temp);

	return MakeErrorCode(OK);
}

um_Error builtin_add(um_Vector* v_params, um_Noun* result) {
	size_t ac = v_params->size;
	um_Noun a0 = v_params->data[0], a1 = v_params->data[1];
	if (ac == 1) {
		*result = new (fabs(a0.value.number));
		return MakeErrorCode(OK);
	} else if (ac > 2 || ac < 1) {
		return MakeErrorCode(ERROR_ARGS);
	}

	double _temp
	    = cast(a0, real_t).value.number + cast(a1, real_t).value.number;

	*result = new (_temp);

	return MakeErrorCode(OK);
}

um_Error builtin_subtract(um_Vector* v_params, um_Noun* result) {
	size_t ac = v_params->size;
	um_Noun a0 = v_params->data[0], a1 = v_params->data[1];
	if (ac == 1) {
		*result = new (-fabs(a0.value.number));
		return MakeErrorCode(OK);
	} else if (ac > 2 || ac < 1) {
		return MakeErrorCode(ERROR_ARGS);
	}

	double _temp
	    = cast(a0, real_t).value.number - cast(a1, real_t).value.number;

	*result = new (_temp);
	return MakeErrorCode(OK);
}

um_Error builtin_modulo(um_Vector* v_params, um_Noun* result) {
	um_Noun a0 = v_params->data[0], a1 = v_params->data[1];
	if (v_params->size != 2) { return MakeErrorCode(ERROR_ARGS); }

	double _temp = (long)cast(a0, real_t).value.number
		     % (long)cast(a1, real_t).value.number;

	*result = new (_temp);

	return MakeErrorCode(OK);
}

um_Error builtin_multiply(um_Vector* v_params, um_Noun* result) {
	um_Noun a0 = v_params->data[0], a1 = v_params->data[1];
	if (v_params->size != 2) { return MakeErrorCode(ERROR_ARGS); }

	double _temp
	    = cast(a0, real_t).value.number * cast(a1, real_t).value.number;

	*result = new (_temp);

	return MakeErrorCode(OK);
}

um_Error builtin_divide(um_Vector* v_params, um_Noun* result) {
	um_Noun a0 = v_params->data[0], a1 = v_params->data[1];
	if (v_params->size != 2) { return MakeErrorCode(ERROR_ARGS); }

	double _temp
	    = cast(a0, real_t).value.number / cast(a1, real_t).value.number;

	*result = new (_temp);

	return MakeErrorCode(OK);
}

um_Error builtin_less(um_Vector* v_params, um_Noun* result) {
	um_Noun a0 = v_params->data[0], a1 = v_params->data[1];
	if (v_params->size != 2) { return MakeErrorCode(ERROR_ARGS); }

	*result = new ((bool)(cast(a0, real_t).value.number
			      < cast(a1, real_t).value.number));

	return MakeErrorCode(OK);
}

um_Error builtin_greater(um_Vector* v_params, um_Noun* result) {
	um_Noun a0 = v_params->data[0], a1 = v_params->data[1];
	if (v_params->size != 2) { return MakeErrorCode(ERROR_ARGS); }

	*result = new ((bool)(cast(a0, real_t).value.number
			      > cast(a1, real_t).value.number));

	return MakeErrorCode(OK);
}

um_Error builtin_floor(um_Vector* v_params, um_Noun* result) {
	um_Noun a0 = v_params->data[0];
	if (v_params->size != 1) { return MakeErrorCode(ERROR_ARGS); }

	*result = new (floor(cast(a0, real_t).value.number));

	return MakeErrorCode(OK);
}

um_Error builtin_ceil(um_Vector* v_params, um_Noun* result) {
	um_Noun a0 = v_params->data[0];
	if (v_params->size != 1) { return MakeErrorCode(ERROR_ARGS); }

	*result = new (ceil(cast(a0, real_t).value.number));

	return MakeErrorCode(OK);
}

um_Error builtin_hex(um_Vector* v_params, um_Noun* result) {
	um_Noun a0 = v_params->data[0];
	if (v_params->size != 1) { return MakeErrorCode(ERROR_ARGS); }

	char* str = calloc(33, sizeof(char));
	sprintf(str, "%x", (int32_t)cast(a0, real_t).value.number);

	char* tmp = strdup(str);

	*result = new (tmp);

	free(str);

	return MakeErrorCode(OK);
}

um_Error builtin_precision(um_Vector* v_params, um_Noun* result) {
	double v = cast(v_params->data[0], real_t).value.number,
	       n = cast(v_params->data[1], real_t).value.number;
	if (v_params->size != 2) { return MakeErrorCode(ERROR_ARGS); }

	/* This method isn't ideal but printf precision
	   isn't available at runtime */
	char* tmp = calloc(65, sizeof(char));
	/* Convert decimal places wanted to multiplier, 2 -> 100 */
	long places = (long)pow(10, n);
	sprintf(tmp, "%f", round(v * places) / places);
	/* 'Trim' string n chars from decimal point */
	tmp[strcspn(tmp, ".") + (size_t)n + 1] = '\0';

	/* Free excess memory */
	char* str = strdup(tmp);

	*result = new (str);

	free(tmp);

	return MakeErrorCode(OK);
}

um_Error builtin_upper(um_Vector* v_params, um_Noun* result) {
	if (v_params->size != 1) { return MakeErrorCode(ERROR_ARGS); }
	char *s = strdup(cast(v_params->data[0], string_t).value.string->value),
	     *tmp = s;

	while (*tmp) {
		*tmp = (*tmp > 'a' && *tmp <= 'z') ? toupper(*tmp) : *tmp;
		tmp++;
	}

	*result = new (s);
	return MakeErrorCode(OK);
}

um_Error builtin_lower(um_Vector* v_params, um_Noun* result) {
	if (v_params->size != 1) { return MakeErrorCode(ERROR_ARGS); }
	char *s = strdup(cast(v_params->data[0], string_t).value.string->value),
	     *tmp = s;

	while (*tmp) {
		*tmp = (*tmp > 'A' && *tmp <= 'Z') ? tolower(*tmp) : *tmp;
		tmp++;
	}

	*result = new (s);
	return MakeErrorCode(OK);
}

void um_init() {
	srand((unsigned)time(0));
	if (!um_global_symbol_capacity) { um_global_symbol_capacity = 1000; }
	env = env_create(nil, um_global_symbol_capacity);

	symbol_table = calloc(um_global_symbol_capacity, sizeof(char*));

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
	sym_string = intern("vec");
	sym_num = intern("num");
	sym_char = intern("char");
	sym_do = intern("do");
	sym_set = intern("set");
	sym_true = intern("true");
	sym_false = intern("false");

	sym_nil_t = intern("@Nil");
	sym_pair_t = intern("@Pair");
	sym_noun_t = intern("@Noun");
	sym_f64_t = intern("@Float");
	sym_builtin_t = intern("@Builtin");
	sym_closure_t = intern("@Closure");

	sym_macro_t = intern("@Macro");
	sym_string_t = intern("@String");
	sym_vector_t = intern("@Vector");
	sym_input_t = intern("@Input");
	sym_output_t = intern("@Output");
	sym_error_t = intern("@Error");
	sym_type_t = intern("@Type");
	sym_bool_t = intern("@Bool");

#define add_builtin(name, fn_ptr) \
	env_assign(env, intern(name).value.symbol, new_builtin(fn_ptr))

	env_assign(env, sym_true.value.symbol, new ((bool)true));
	env_assign(env, sym_false.value.symbol, new ((bool)false));
	env_assign(env, intern("nil").value.symbol, nil);
	env_assign(env, intern("_").value.symbol, um_noreturn);

	env_assign(env, sym_nil_t.value.symbol, new ((um_NounType)nil_t));
	env_assign(env, sym_pair_t.value.symbol, new ((um_NounType)pair_t));
	env_assign(env, sym_noun_t.value.symbol, new ((um_NounType)noun_t));
	env_assign(env, sym_f64_t.value.symbol, new ((um_NounType)real_t));
	env_assign(
	    env, sym_builtin_t.value.symbol, new ((um_NounType)builtin_t));
	env_assign(
	    env, sym_closure_t.value.symbol, new ((um_NounType)closure_t));
	env_assign(env, sym_macro_t.value.symbol, new ((um_NounType)macro_t));
	env_assign(env, sym_string_t.value.symbol, new ((um_NounType)string_t));
	env_assign(env, sym_vector_t.value.symbol, new ((um_NounType)vector_t));
	env_assign(env, sym_input_t.value.symbol, new ((um_NounType)input_t));
	env_assign(env, sym_output_t.value.symbol, new ((um_NounType)output_t));
	env_assign(env, sym_error_t.value.symbol, new ((um_NounType)error_t));
	env_assign(env, sym_type_t.value.symbol, new ((um_NounType)type_t));
	env_assign(env, sym_bool_t.value.symbol, new ((um_NounType)bool_t));

	add_builtin("car", builtin_car);
	add_builtin("cdr", builtin_cdr);
	add_builtin("cons", builtin_cons);

	add_builtin("+", builtin_add);
	add_builtin("-", builtin_subtract);
	add_builtin("*", builtin_multiply);
	add_builtin("/", builtin_divide);
	add_builtin("%", builtin_modulo);

	add_builtin(">", builtin_greater);
	add_builtin("<", builtin_less);
	add_builtin("=", builtin_eq);
	add_builtin("eq?", builtin_eq);
	add_builtin("eqv?", builtin_eq_l);

	add_builtin("__builtin_pow", builtin_pow);
	add_builtin("__builtin_cbrt", builtin_cbrt);
	add_builtin("not", builtin_not);
	add_builtin("__builtin_sin", builtin_sin);
	add_builtin("__builtin_cos", builtin_cos);
	add_builtin("__builtin_tan", builtin_tan);
	add_builtin("__builtin_asin", builtin_asin);
	add_builtin("__builtin_acos", builtin_acos);
	add_builtin("__builtin_atan", builtin_atan);
	add_builtin("len", builtin_len);
	add_builtin("eval", builtin_eval);
	add_builtin("type", builtin_type);
	add_builtin("exit", builtin_exit);
	add_builtin("apply", builtin_apply);
	add_builtin("macex", builtin_macex);
	add_builtin("str", builtin_string);
	add_builtin("print", builtin_print);
	add_builtin("pair?", builtin_pairp);
	add_builtin("float", builtin_float);
	add_builtin("range", builtin_range);
	add_builtin("cast", builtin_cast);
	add_builtin("getlist", builtin_getlist);
	add_builtin("and", builtin_and);
	add_builtin("setlist", builtin_setlist);
	add_builtin("__builtin_vector", builtin_vector);
	add_builtin("__builtin_ceil", builtin_ceil);
	add_builtin("__builtin_floor", builtin_floor);
	add_builtin("__builtin_format_hex", builtin_hex);
	add_builtin("__builtin_format_precision", builtin_precision);

	add_builtin("__builtin_format_upper", builtin_upper);
	add_builtin("__builtin_format_lower", builtin_lower);

	add_builtin("if", new_builtin(NULL).value.builtin);
	add_builtin("fn", new_builtin(NULL).value.builtin);
	add_builtin("do", new_builtin(NULL).value.builtin);
	add_builtin("def", new_builtin(NULL).value.builtin);
	add_builtin("const", new_builtin(NULL).value.builtin);
	add_builtin("mac", new_builtin(NULL).value.builtin);
	add_builtin("cond", new_builtin(NULL).value.builtin);
	add_builtin("switch", new_builtin(NULL).value.builtin);
	add_builtin("match", new_builtin(NULL).value.builtin);
	add_builtin("defun", new_builtin(NULL).value.builtin);
	add_builtin("quote", new_builtin(NULL).value.builtin);
	add_builtin("lambda", new_builtin(NULL).value.builtin);
	ingest("\
(defun compose (f g)\
	(lambda (x) (f (g x))))");

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
		('vector __builtin_vector) \
		('list list) \
		('map map) \
		('cast cast)))");

	ingest("\
(defun format (fun)\
	(switch fun\
		('hex __builtin_format_hex)\
		('precision __builtin_format_precision)\
		('upper __builtin_format_upper)\
		('lower __builtin_format_lower)\
		))");

	ingest("\
(defun math (fun)\
	(switch fun\
		('pi 3.1415926535897931)\
		('e 2.7182818284590452)\
		('ceil __builtin_ceil)\
		('floor __builtin_floor)\
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
		('sum (lambda (x) (reduce + x 0)))\
		('product (lambda (x) (reduce * x 1)))\
		('sigma (lambda (f s e)\
			(reduce + (map f (range s e)) 0)))\
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
   		_\
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

	ingest("\
(def (reduce f l id)\
        (if (nil? l)\
                id\
                (f (car l) (reduce f (cdr l) id))))");
}

#endif
