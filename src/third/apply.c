#include "../um.h"
Error apply(Noun fn, Vector* v_params, Noun* result) {
	Noun arg_names, env, body, a;
	Error err;
	size_t index, i;

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
	} else {
		return MakeErrorCode(ERROR_TYPE);
	}
}
