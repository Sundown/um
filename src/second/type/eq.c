#include "../../um.h"

bool eq_pair_h(Noun a, Noun b);

/* Type prejudiced comparison
 * eq high */
bool eq_h(Noun a, Noun b) {
	if (a.type != b.type) { return false; }

	switch (a.type) {
		case nil_t: return isnil(a) && isnil(b);
		case i32_t: return a.value.integer_v == b.value.integer_v;
		case f64_t: return a.value.number_v == b.value.number_v;
		/* Equal symbols share memory */
		case noun_t: return a.value.symbol == b.value.symbol;
		case string_t: return !strcmp(a.value.str->value, b.value.str->value);
		case builtin_t: return a.value.builtin == b.value.builtin;
		case input_t:
		case output_t: return a.value.fp == b.value.fp;
		case type_t: return a.value.type_v == b.value.type_v;
		case bool_t: return a.value.bool_v == b.value.bool_v;
		case error_t: return a.value.error_v._ == b.value.error_v._;
		case pair_t:
		case macro_t:
		case closure_t: return eq_pair_h(a, b); /* Frequently recursive */
		default: return false;
	}
}

/* Non-prejudiced comparison
 * eq low */
bool eq_l(Noun a, Noun b) {
	if (a.type == b.type) {
		return eq_h(a, b);
	} else {
		return eq_h(a, coerce(b, a.type));
	}
}

bool eq_pair_h(Noun a, Noun b) {
	if (a.type != pair_t || b.type != pair_t) { return false; }

	return eq_h(car(a), car(b)) && eq_h(cdr(a), cdr(b));
}

bool eq_pair_l(Noun a, Noun b) {
	if (a.type != pair_t || b.type != pair_t) { return false; }

	return eq_l(car(a), car(b)) && eq_l(cdr(a), cdr(b));
}

Noun coerce(Noun a, noun_type t) {
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
