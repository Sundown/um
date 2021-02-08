#include "../um.h"

Noun env_create(Noun parent, size_t capacity) {
	return cons(parent, new_table(capacity));
}

Error env_get(Noun env, char* symbol, Noun* result) {
	while (1) {
		Table* ptbl = cdr(env).value.Table;
		Table_entry* a = table_get_sym(ptbl, symbol);
		if (a) {
			*result = a->v;
			return MakeErrorCode(OK);
		}

		if (isnil(car(env))) { return MakeErrorCode(ERROR_UNBOUND); }

		env = car(env);
	}
}

Error env_assign(Noun env, char* symbol, Noun value) {
	Table* ptbl = cdr(env).value.Table;
	return table_set_sym(ptbl, symbol, value);
}

Error env_assign_eq(Noun env, char* symbol, Noun value) {
	while (1) {
		Noun parent = car(env);
		Table* ptbl = cdr(env).value.Table;
		Table_entry* a = table_get_sym(ptbl, symbol);
		if (a) {
			if (!a->v.mut) { return MakeErrorCode(ERROR_NOMUT); }
			a->v = value;
			return MakeErrorCode(OK);
		}

		if (isnil(parent)) { return env_assign(env, symbol, value); }

		env = parent;
	}
}

Error destructuring_bind(Noun arg_name, Noun val, Noun env) {
	Error err;
	if (isnil(arg_name)) {
		if (isnil(val)) {
			return MakeErrorCode(OK);
		} else {
			return MakeErrorCode(ERROR_ARGS);
		}
	} else if (arg_name.type == noun_t) {
		return env_assign(env, arg_name.value.symbol, val);
	} else if (arg_name.type == pair_t) {
		if (val.type != pair_t) { return MakeErrorCode(ERROR_ARGS); }

		err = destructuring_bind(car(arg_name), car(val), env);
		if (err._) { return err; }

		return destructuring_bind(cdr(arg_name), cdr(val), env);
	} else {
		return MakeErrorCode(ERROR_ARGS);
	}
}

Error env_bind(Noun env, Noun arg_names, Vector* v_params) {
	Noun arg_name, val;
	int val_unspecified = 0;
	Error err;

	size_t i = 0;
	while (!isnil(arg_names)) {
		if (arg_names.type == noun_t) {
			env_assign(env, arg_names.value.symbol, vector_to_noun(v_params, i));
			i = v_params->size;
			break;
		}

		arg_name = car(arg_names);

		if (i < v_params->size) {
			val = v_params->data[i];
		} else {
			val = nil;
			val_unspecified = 1;
		}

		err = destructuring_bind(arg_name, val, env);
		if (err._) { return err; }

		arg_names = cdr(arg_names);
		i++;
	}

	if (i < v_params->size) { return MakeErrorCode(ERROR_ARGS); }

	return MakeErrorCode(OK);
}
