#include "coercion.h"

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

Noun noun_to_t(char* x, noun_type t) {
	switch (t) {
		case pair_t: return cons(intern(x), nil);
		case noun_t: return intern(x);
		case i32_t: return new_integer(atol(x));
		case f64_t: return new_number(strtod(x, NULL));
		case string_t: return new_string(x);
		case type_t: return new_type(noun_t);
		case bool_t: return new_bool(x != NULL && x != intern("nil").value.symbol);
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
		case bool_t: return new_bool(x != NULL && strcmp(x, "nil") && strcmp(x, "false"));
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
		case string_t: return x ? new_string("true") : new_string("false");
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

char* to_string(Noun a, bool write) {
	char *s = str_new(), *s2, buf[80];
	Noun a2;
	switch (a.type) {
		case nil_t: append_string(&s, "Nil"); break;
		case pair_t:
			{
				if (listp(a) && list_len(a) == 2 && eq_h(car(a), sym_quote)) {
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
			(void*)a.value.builtin ? sprintf(buf, "Builtin") : sprintf(buf, "Internal");

			append_string(&s, buf);
			break;
		case closure_t:
			{
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
		case bool_t: append_string(&s, a.value.bool_v ? "True" : "False"); break;
		default: append_string(&s, ":Unknown"); break;
	}

	return s;
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
	char* s = calloc(e.message != NULL ? strlen(e.message) : 0 + 27, sizeof(char));
	e._ != MakeErrorCode(ERROR_USER)._&& e.message
	    ? sprintf(s, "%s\n%s\n", error_string[e._], e.message)
	    : sprintf(s, "%s\n", error_string[e._]);

	return s;
}
