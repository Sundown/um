#include "../um.h"

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
		stack = realloc(stack, stack_capacity * sizeof(Noun));
	}

	stack[stack_size - 1] = a;
}

void stack_restore(int saved_size) {
	stack_size = saved_size;
	if (stack_size < stack_capacity / 4) {
		stack_capacity = stack_size * 2;
		stack = realloc(stack, stack_capacity * sizeof(Noun));
	}

	garbage_collector_consider();
}

void stack_restore_add(int saved_size, Noun a) {
	stack_size = saved_size;
	if (stack_size < stack_capacity / 4) {
		stack_capacity = stack_size * 2;
		stack = realloc(stack, stack_capacity * sizeof(Noun));
	}

	stack_add(a);
	garbage_collector_consider();
}
