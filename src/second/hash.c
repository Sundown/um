#include "../um.h"

size_t hash_code_sym(char* s) {
	return (size_t)s / sizeof(s) / 2;
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
		case string_t:
			{
				char* v = a.value.str->value;
				for (; *v != 0; v++) {
					r *= 31;
					r += *v;
				}

				return r;
			}
		case f64_t: return (size_t)((void*)a.value.symbol) + (size_t)a.value.number_v;
		case builtin_t: return (size_t)a.value.builtin;
		case closure_t: return hash_code(cdr(a));
		case macro_t: return hash_code(cdr(a));
		case input_t:
		case output_t: return (size_t)a.value.fp / sizeof(*a.value.fp);
		default: return 0;
	}
}
