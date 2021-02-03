#include "../um.h"

void vector_new(Vector* a) {
	a->capacity = sizeof(a->static_data) / sizeof(a->static_data[0]);
	a->size = 0;
	a->data = a->static_data;
}

void vector_add(Vector* a, Noun item) {
	if (a->size + 1 > a->capacity) {
		a->capacity *= 2;
		if (a->data == a->static_data) {
			a->data = malloc(a->capacity * sizeof(Noun));
			memcpy(a->data, a->static_data, a->size * sizeof(Noun));
		} else {
			a->data = realloc(a->data, a->capacity * sizeof(Noun));
		}
	}

	a->data[a->size] = item;
	a->size++;
}

void vector_clear(Vector* a) {
	a->size = 0;
}

void vector_free(Vector* a) {
	if (a->data != a->static_data) free(a->data);
}

Noun vector_to_noun(Vector* a, int start) {
	Noun r = nil;
	int i;
	for (i = a->size - 1; i >= start; i--) { r = cons(a->data[i], r); }

	return r;
}

void noun_to_vector(Noun a, Vector* v) {
	vector_new(v);
	for (; !isnil(a); a = cdr(a)) { vector_add(v, car(a)); }
}
