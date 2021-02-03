#include "../um.h"

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
		case table_t:
			{
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
