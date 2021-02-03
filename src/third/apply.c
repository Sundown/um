#include "../um.h"
Error apply(Noun fn, Vector* v_params, Noun* result) {
	Table_entry* pair_v;
	Noun arg_names, env, body, a;
	Error err;
	size_t index, i;
	long len1;

	if (fn.type == builtin_t) {
		return (*fn.value.builtin)(v_params, result);
	} else if (fn.type == closure_t) {
		arg_names = car(cdr(fn));
		env = env_create(car(fn), list_len(arg_names));
		body = cdr(cdr(fn));

		err = env_bind(env, arg_names, v_params);
		if (err._) { return err; }

		err = eval_expr(car(body), env, result);
		if (err._) { return err; }

		return MakeErrorCode(OK);
	} else if (fn.type == string_t) {
		if (v_params->size != 1) { return MakeErrorCode(ERROR_ARGS); }

		index = (size_t)(v_params->data[0]).value.number_v;
		*result = new_string((char[]){fn.value.str->value[index], '\0'});
		return MakeErrorCode(OK);
	} else if (fn.type == pair_t && listp(fn)) {
		if (v_params->size != 1) { return MakeErrorCode(ERROR_ARGS); }

		if (v_params->data[0].type != i32_t) { return MakeErrorCode(ERROR_TYPE); }

		index = (size_t)(v_params->data[0]).value.integer_v;
		a = fn;

		for (i = 0; i < index; i++) {
			a = cdr(a);
			if (isnil(a)) {
				*result = nil;
				return MakeErrorCode(OK);
			}
		}

		*result = car(a);
		return MakeErrorCode(OK);
	} else if (fn.type == table_t) {
		len1 = v_params->size;
		if (len1 != 1 && len1 != 2) { return MakeErrorCode(ERROR_ARGS); }

		pair_v = table_get(fn.value.Table, v_params->data[0]);
		if (pair_v) {
			*result = pair_v->v;
		} else {
			if (len1 == 2) {
				*result = v_params->data[1];
			} else {
				*result = nil;
			}
		}

		return MakeErrorCode(OK);
	} else {
		return MakeErrorCode(ERROR_TYPE);
	}
}
