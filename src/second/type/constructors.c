#include "../../um.h"

Noun new_number(double x) {
	return (Noun){f64_t, false, {.number_v = x}, false};
}

Noun new_integer(long x) {
	return (Noun){i32_t, false, {.integer_v = x}, false};
}

Noun new_builtin(builtin fn) {
	return (Noun){builtin_t, false, {.builtin = fn}, false};
}

Noun new_type(noun_type t) {
	return (Noun){type_t, false, {.type_v = t}, false};
}

Noun new_bool(bool b) {
	return (Noun){bool_t, false, {.bool_v = b}, false};
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
