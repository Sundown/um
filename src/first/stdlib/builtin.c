#include "../../um.h"

Error builtin_type(Vector* v_params, Noun* result) {
	if (v_params->size != 1) { return MakeErrorCode(ERROR_ARGS); }

	*result = new_type(v_params->data[0].type);
	return MakeErrorCode(OK);
}

Error builtin_getlist(Vector* v_params, Noun* result) {
	if (v_params->size != 2) { return MakeErrorCode(ERROR_ARGS); }
	if (v_params->data[0].type != i32_t) {
		return MakeError(ERROR_TYPE, "list_index: first parameter must be integer type");
	}
	if (!listp(v_params->data[1])) {
		return MakeError(ERROR_TYPE, "list_index: second parameter must be list");
	}

	*result = *list_index(&v_params->data[1], v_params->data[0].value.integer_v);
	return MakeErrorCode(OK);
}

Error builtin_setlist(Vector* v_params, Noun* result) {
	if (v_params->size != 3) { return MakeErrorCode(ERROR_ARGS); }
	if (v_params->data[0].type != i32_t) {
		return MakeError(ERROR_TYPE, "list_index: first parameter must be integer type");
	}
	if (!listp(v_params->data[1])) {
		return MakeError(ERROR_TYPE, "list_index: second parameter must be list");
	}

	Noun* t = list_index(&v_params->data[1], v_params->data[0].value.integer_v);
	t->type = v_params->data[2].type;
	t->value = v_params->data[2].value;
	*result = *t;
	return MakeErrorCode(OK);
}

Error builtin_len(Vector* v_params, Noun* result) {
	if (v_params->size != 1) { return MakeErrorCode(ERROR_ARGS); }

	if (listp(v_params->data[0])) {
		*result = new ((long)list_len(v_params->data[0]));
	} else if (v_params->data[0].type == string_t) {
		*result = new ((long)strlen(v_params->data[0].value.str->value));
	} else {
		*result = new ((long)0);
		return MakeErrorCode(ERROR_TYPE);
	}

	return MakeErrorCode(OK);
}

Error builtin_range(Vector* v_params, Noun* result) {
	if (v_params->size > 2 || v_params->size < 1) {
		return MakeError(ERROR_ARGS, "range: arg count must be nonzero below 3");
	}

	if (!isnumber(v_params->data[0]) || !isnumber(v_params->data[0])) {
		return MakeError(ERROR_TYPE, "range: args must be type numeric");
	}

	long a = coerce(v_params->data[0], i32_t).value.integer_v,
	     b = coerce(v_params->data[1], i32_t).value.integer_v;

	Noun range = nil;

	if (a < b) {
		for (; a <= b; b--) { range = cons(new (b), range); }
	} else {
		for (; a >= b; b++) { range = cons(new (b), range); }
	}

	*result = range;
	return MakeErrorCode(OK);
}

Error builtin_car(Vector* v_params, Noun* result) {
	Noun a;
	if (v_params->size != 1) { return MakeErrorCode(ERROR_ARGS); }

	a = v_params->data[0];
	if (isnil(a)) {
		*result = nil;
	} else if (a.type != pair_t) {
		return MakeErrorCode(ERROR_TYPE);
	} else {
		*result = car(a);
	}

	return MakeErrorCode(OK);
}

Error builtin_cdr(Vector* v_params, Noun* result) {
	Noun a;
	if (v_params->size != 1) { return MakeErrorCode(ERROR_ARGS); }

	a = v_params->data[0];
	if (isnil(a)) {
		*result = nil;
	} else if (a.type != pair_t) {
		return MakeErrorCode(ERROR_TYPE);
	} else {
		*result = cdr(a);
	}

	return MakeErrorCode(OK);
}

Error builtin_cons(Vector* v_params, Noun* result) {
	if (v_params->size != 2) { return MakeErrorCode(ERROR_ARGS); }

	*result = cons(v_params->data[0], v_params->data[1]);

	return MakeErrorCode(OK);
}

Error builtin_apply(Vector* v_params, Noun* result) {
	Noun fn;
	Vector v;
	Error err;

	if (v_params->size != 2) { return MakeErrorCode(ERROR_ARGS); }

	fn = v_params->data[0];
	noun_to_vector(v_params->data[1], &v);
	err = apply(fn, &v, result);
	vector_free(&v);
	return err;
}

Error builtin_eq(Vector* v_params, Noun* result) {
	Noun a, b;
	size_t i;
	if (v_params->size <= 1) {
		*result = new ((bool)true);
		return MakeErrorCode(OK);
	}

	for (i = 0; i < v_params->size - 1; i++) {
		a = v_params->data[i];
		b = v_params->data[i + 1];
		if (!eq_h(a, b)) {
			*result = new ((bool)false);
			return MakeErrorCode(OK);
		}
	}

	*result = new ((bool)true);
	return MakeErrorCode(OK);
}

Error builtin_eq_l(Vector* v_params, Noun* result) {
	Noun a, b;
	size_t i;
	if (v_params->size <= 1) {
		*result = sym_true;
		return MakeErrorCode(OK);
	}

	for (i = 0; i < v_params->size - 1; i++) {
		a = v_params->data[i];
		b = v_params->data[i + 1];
		if (!eq_l(a, b)) {
			*result = nil;
			return MakeErrorCode(OK);
		}
	}

	*result = sym_true;
	return MakeErrorCode(OK);
}

Error builtin_pairp(Vector* v_params, Noun* result) {
	if (v_params->size != 1) { return MakeErrorCode(ERROR_ARGS); }

	*result = v_params->data[0].type == pair_t ? sym_true : nil;
	return MakeErrorCode(OK);
}

Error builtin_not(Vector* v_params, Noun* result) {
	if (v_params->size != 1) {
		*result = nil;
		return MakeErrorCode(ERROR_ARGS);
	}

	*result = coerce(v_params->data[0], bool_t).value.bool_v ? new ((bool)false) : sym_true;
	return MakeErrorCode(OK);
}

Error builtin_exit(Vector* v_params, Noun* result) {
	Noun code = nil;
	if (v_params->size != 1) { return MakeErrorCode(ERROR_ARGS); }

	if (isnil(code = coerce(v_params->data[0], i32_t))) { return MakeErrorCode(ERROR_TYPE); }

	*result = code;

	exit(code.value.integer_v);
}

Error builtin_macex(Vector* v_params, Noun* result) {
	long alen = v_params->size;
	if (alen == 1) {
		Error err = macex(v_params->data[0], result);
		return err;
	} else
		return MakeErrorCode(ERROR_ARGS);
	return MakeErrorCode(OK);
}

Error builtin_string(Vector* v_params, Noun* result) {
	char* s = str_new();
	size_t i;
	for (i = 0; i < v_params->size; i++) {
		if (!isnil(v_params->data[i])) {
			char* a = to_string(v_params->data[i], 0);
			append_string(&s, a);
			free(a);
		}
	}

	*result = new_string(s);
	return MakeErrorCode(OK);
}

Error builtin_print(Vector* v_params, Noun* result) {
	size_t i;
	for (i = 0; i < v_params->size; i++) {
		if (!isnil(v_params->data[i])) { puts(to_string(v_params->data[i], 0)); }
	}

	*result = nil;
	return MakeErrorCode(OK);
}

Error builtin_eval(Vector* v_params, Noun* result) {
	if (v_params->size == 1)
		return macex_eval(v_params->data[0], result);
	else
		return MakeErrorCode(ERROR_ARGS);
}

Error builtin_int(Vector* v_params, Noun* result) {
	if (v_params->size == 1) {
		*result = coerce(v_params->data[0], i32_t);

		return MakeErrorCode(OK);
	} else {
		return MakeErrorCode(ERROR_ARGS);
	}
}

Error builtin_coerce(Vector* v_params, Noun* result) {
	if (v_params->size == 2) {

		*result = coerce(v_params->data[0],
				 v_params->data[1].type != type_t ? v_params->data[1].type
								  : v_params->data[1].value.type_v);

		return MakeErrorCode(OK);
	} else {
		*result = nil;
		return MakeErrorCode(ERROR_ARGS);
	}
}

Error builtin_float(Vector* v_params, Noun* result) {
	if (v_params->size == 1) {
		*result = coerce(v_params->data[0], f64_t);

		return MakeErrorCode(OK);
	} else {
		return MakeErrorCode(ERROR_ARGS);
	}
}
