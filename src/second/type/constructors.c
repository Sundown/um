#include "../../um.h"

size_t symbol_size;

Noun cons(Noun car_val, Noun cdr_val) {
	Pair* a;
	Noun p;
	alloc_count++;

	a = calloc(1, sizeof(Pair));
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
		symbol_table = realloc(symbol_table, um_global_symbol_capacity * sizeof(char*));
	}

	symbol_table[symbol_size] = a.value.symbol;
	symbol_size++;

	return a;
}

Noun new_number(double x) {
	return (Noun){f64_t, true, {.number_v = x}};
}

Noun new_integer(long x) {
	return (Noun){i32_t, true, {.integer_v = x}};
}

Noun new_builtin(builtin fn) {
	return (Noun){builtin_t, true, {.builtin = fn}};
}

Noun new_type(noun_type t) {
	return (Noun){type_t, true, {.type_v = t}};
}

Noun new_bool(bool b) {
	return (Noun){bool_t, true, {.bool_v = b}};
}

Error new_closure(Noun env, Noun args, Noun body, Noun* result) {
	Noun p;

	if (!listp(body)) { return MakeErrorCode(ERROR_SYNTAX); }

	p = args;
	while (!isnil(p)) {
		if (p.type == noun_t)
			break;
		else if (p.type != pair_t || (car(p).type != noun_t && car(p).type != pair_t))
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
