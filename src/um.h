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

#define PROMPT "> "

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
	ERROR_COERCION_FAIL
} ErrorCode;

typedef struct {
	char* message;
	ErrorCode _;
} Error;

#define MakeErrorCode(c) \
	(Error) {        \
		NULL, c  \
	}

#define MakeError(c, m) \
	(Error) {       \
		m, c    \
	}

static const char* error_string[] = {"",
				     "Syntax error",
				     "Symbol unbound",
				     "Parameter error",
				     "Type error",
				     "File error",
				     "",
				     "Coercion error"};

typedef struct Noun Noun;
typedef struct Vector Vector;

typedef Error (*builtin)(struct Vector* v_params, struct Noun* result);

struct Noun {
	noun_type type;
	bool constant;
	union {
		noun_type type_v;
		bool bool_v;
		Error error_v;
		double number_v;
		long integer_v;
		struct Pair* pair_v;
		char* symbol;
		struct str* str;
		builtin builtin;
		FILE* fp;
		struct Table* Table;
		Error err_v;
	} value;
	bool reference;
};

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

static const Noun nil = {nil_t, true, {nil_t}, false};

Noun sym_t, sym_quote, sym_quasiquote, sym_unquote, sym_unquote_splicing, sym_def, sym_set,
    sym_defun, sym_fn, sym_if, sym_cond, sym_switch, sym_match, sym_mac, sym_apply, sym_cons,
    sym_sym, sym_string, sym_num, sym_int, sym_char, sym_do, sym_true, sym_false,

    sym_nil_t, sym_pair_t, sym_noun_t, sym_f64_t, sym_i32_t, sym_builtin_t, sym_closure_t,
    sym_macro_t, sym_string_t, sym_input_t, sym_output_t, sym_error_t, sym_type_t, sym_bool_t;

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
static Noun cur_expr;
static Noun thrown;

#define car(p)	    ((p).value.pair_v->car)
#define cdr(p)	    ((p).value.pair_v->cdr)
#define isnil(Noun) ((Noun).type == nil_t)

#define isnumber(a)  (a.type == f64_t || a.type == i32_t)
#define getnumber(a) (a.type == f64_t ? a.value.number_v : a.type == i32_t ? a.value.integer_v : 0)

#define cdr2(p) (cdr(p))
#define pop(n)	(n = cdr2(n))

#define ingest(s)                                          \
	do {                                               \
		Error __tmperr = interpret_string(s);      \
		if (__tmperr._) { print_error(__tmperr); } \
	} while (0)

Noun coerce(Noun a, noun_type t);
Noun new_number(double x);
Noun new_type(noun_type t);
Noun new_integer(long x);
Noun new_string(char* x);
Noun new_bool(bool b);

void vector_new(Vector* a);
void vector_add(Vector* a, Noun item);
void vector_clear(Vector* a);
void vector_free(Vector* a);
void noun_to_vector(Noun a, Vector* v);

Noun vector_to_noun(Vector* a, int start);

size_t list_len(Noun xs);

void stack_add(Noun a);
void stack_restore(int saved_size);
void stack_restore_add(int saved_size, Noun a);

/* clang-format off */
#define new(T) _Generic((T),  \
 bool: new_bool,              \
 long: new_integer,           \
 char*: new_string,           \
 double: new_number,          \
 builtin: new_builtin,        \
 noun_type: new_type          \
)(T)
/* clang-format on */

#define Ok(i) return MakeErrorCode(OK);

char* type_to_string(noun_type a);
char* error_to_string(Error e);

Noun cons(Noun car_val, Noun cdr_val);

Noun intern(const char* s);
Noun new_builtin(builtin fn);
Noun new_input(FILE* fp);
Noun new_output(FILE* fp);
Error new_closure(Noun env, Noun args, Noun body, Noun* result);

Error interpret_string(const char* text);
void repl();

Noun new_vector(Vector* v);

Error lex(const char* str, const char** start, const char** end);
Error parse_simple(const char* start, const char* end, Noun* result);
Error read_expr(const char* input, const char** end, Noun* result);

Error apply(Noun fn, Vector* v_params, Noun* result);
Noun copy_list(Noun list);
Noun* list_index(Noun* list, size_t index);
Error env_assign_eq(Noun env, char* symbol, Noun value);

Noun reverse_list(Noun list);
bool listp(Noun expr);
char* read_file(const char* path);

Error eval_expr(Noun expr, Noun env, Noun* result);

Error env_get(Noun env, char* symbol, Noun* result);
Error env_assign(Noun env, char* symbol, Noun value);
Error destructuring_bind(Noun arg_name, Noun val, Noun env);
Error env_bind(Noun env, Noun arg_names, Vector* v_params);
Noun env_create(Noun parent, size_t capacity);

void garbage_collector_run();
void garbage_collector_consider();
void garbage_collector_tag(Noun root);

Error macex(Noun expr, Noun* result);
char* to_string(Noun a, bool write);
char* append_string(char** dst, char* src);
char* str_new();
Error macex_eval(Noun expr, Noun* result);
Error um_load_file(const char* path);
void um_init();
char* readline_fp(char* prompt, FILE* fp);
Error read_expr(const char* input, const char** end, Noun* result);
void print_expr(Noun a);
void print_error(Error e);

bool eq_h(Noun a, Noun b);
bool eq_l(Noun a, Noun b);

size_t hash_code(Noun a);
size_t hash_code_sym(char* s);

Noun new_table(size_t capacity);
void table_add(Table* tbl, Noun k, Noun v);
Table_entry* table_get(Table* tbl, Noun k);
Table_entry* table_get_sym(Table* tbl, char* k);
bool table_set(Table* tbl, Noun k, Noun v);
bool table_set_sym(Table* tbl, char* k, Noun v);

#define DECLARE_BUILTIN(name) Error builtin_##name(Vector* v_params, Noun* result)

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
DECLARE_BUILTIN(not );
DECLARE_BUILTIN(pairp);
DECLARE_BUILTIN(type);
DECLARE_BUILTIN(float);
DECLARE_BUILTIN(coerce);
DECLARE_BUILTIN(range);
DECLARE_BUILTIN(len);
DECLARE_BUILTIN(getlist);
DECLARE_BUILTIN(setlist);
DECLARE_BUILTIN(modulo);
DECLARE_BUILTIN(pow);
DECLARE_BUILTIN(cbrt);

#undef DECLARE_BUILTIN

#endif
