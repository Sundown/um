#include "../um.h"

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
