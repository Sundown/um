#include "../um.h"

/* Opted to return pointer so this can serve
 * as a part in setting list elements by index */
Noun* list_index(Noun* list, size_t index) {
	size_t i;
	Noun a = *list;
	for (i = 0; i < index; i++) {
		pop(a);
		if (isnil(a)) { return (Noun*)&nil; }
	}

	return &car(a);
}

bool listp(Noun expr) {
	Noun* p = &expr;
	while (!isnil(*p)) {
		if (p->type != pair_t) { return 0; }

		p = &cdr(*p);
	}

	return 1;
}

size_t list_len(Noun xs) {
	Noun* p = &xs;
	size_t ret = 0;
	while (!isnil(*p)) {
		if (p->type != pair_t) { return ret + 1; }

		p = &cdr(*p);
		ret++;
	}

	return ret;
}

Noun copy_list(Noun list) {
	Noun a, p;

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

Noun reverse_list(Noun list) {
	Noun tail = nil;

	while (!isnil(list)) {
		tail = cons(car(list), tail);
		pop(list);
	}

	return tail;
}
