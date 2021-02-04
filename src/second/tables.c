#include "../um.h"

Noun new_table(size_t capacity) {
	Noun a;
	Table* s;
	size_t i;
	alloc_count++;
	s = a.value.Table = malloc(sizeof(Table));
	s->capacity = capacity;
	s->size = 0;
	s->data = malloc(capacity * sizeof(Table_entry*));
	for (i = 0; i < capacity; i++) { s->data[i] = NULL; }

	s->mark = 0;
	s->next = table_head;
	table_head = s;
	a.value.Table = s;
	a.type = table_t;
	stack_add(a);
	return a;
}

Table_entry* table_entry_new(Noun k, Noun v, Table_entry* next) {
	Table_entry* r = malloc(sizeof(*r));
	r->k = k;
	r->v = v;
	r->next = next;
	return r;
}

Error table_set_sym(Table* tbl, char* k, Noun v) {
	Table_entry* p = table_get_sym(tbl, k);
	Noun s = {noun_t, .value.symbol = NULL};
	if (p) {
		if (!p->v.mutable) { return MakeErrorCode(ERROR_NOMUT); }
		p->v = v;
		return MakeErrorCode(OK);
	} else {
		s.value.symbol = k;
		table_add(tbl, s, v);
		return MakeErrorCode(OK);
	}
}

void table_add(Table* tbl, Noun k, Noun v) {
	Table_entry **b, **data2, *p, **p2, *next;
	size_t i, new_capacity;
	if (tbl->size + 1 > tbl->capacity) {
		new_capacity = (tbl->size + 1) * 2;
		data2 = malloc(new_capacity * sizeof(Table_entry*));
		for (i = 0; i < new_capacity; i++) { data2[i] = NULL; }

		for (i = 0; i < tbl->capacity; i++) {
			p = tbl->data[i];
			while (p) {
				p2 = &data2[hash_code(p->k) % new_capacity];
				next = p->next;
				*p2 = table_entry_new(p->k, p->v, *p2);
				free(p);
				p = next;
			}
		}

		free(tbl->data);
		tbl->data = data2;
		tbl->capacity = new_capacity;
	}

	b = &tbl->data[hash_code(k) % tbl->capacity];
	*b = table_entry_new(k, v, *b);
	tbl->size++;
}

Table_entry* table_get_sym(Table* tbl, char* k) {
	Table_entry* p;
	size_t pos;
	if (tbl->size == 0) { return NULL; }
	pos = hash_code_sym(k) % tbl->capacity;
	p = tbl->data[pos];
	while (p) {
		if (p->k.value.symbol == k) { return p; }
		p = p->next;
	}

	return NULL;
}
