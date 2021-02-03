#include "../../../um.h"

Error builtin_add(Vector* v_params, Noun* result) {
	double temp = 0;
	size_t i;

	if (v_params->size == 1) {
		temp = getnumber(v_params->data[0]);
		*result = new_number(temp < 0 ? temp * -1 : temp);
		return MakeErrorCode(OK);
	}

	for (i = 0; i < v_params->size; i++) {
		temp += coerce(v_params->data[i], f64_t).value.number_v;
	}

	*result
	    = isnormal(temp) && (temp == floor(temp)) ? new_integer((long)temp) : new_number(temp);

	return MakeErrorCode(OK);
}

Error builtin_subtract(Vector* v_params, Noun* result) {
	double temp;
	size_t i;

	if (v_params->size == 1) {
		*result = new_number(-coerce(v_params->data[0], f64_t).value.number_v);
		return MakeErrorCode(OK);
	}

	temp = coerce(v_params->data[0], f64_t).value.number_v;

	for (i = 1; i < v_params->size; i++) {
		temp -= coerce(v_params->data[i], f64_t).value.number_v;
	}

	*result
	    = isnormal(temp) && (temp == floor(temp)) ? new_integer((long)temp) : new_number(temp);

	return MakeErrorCode(OK);
}

Error builtin_modulo(Vector* v_params, Noun* result) {
	long temp;
	size_t i;

	if (v_params->size == 1) {
		return MakeError(ERROR_ARGS, "modulo: requires 2 or more parameters");
	}

	temp = coerce(v_params->data[0], i32_t).value.integer_v;

	for (i = 1; i < v_params->size; i++) {
		temp %= coerce(v_params->data[i], i32_t).value.integer_v;
	}

	*result = new (temp);

	return MakeErrorCode(OK);
}

Error builtin_multiply(Vector* v_params, Noun* result) {
	double temp = 1;
	size_t i;
	for (i = 0; i < v_params->size; i++) {
		temp *= coerce(v_params->data[i], f64_t).value.number_v;
	}

	*result
	    = isnormal(temp) && (temp == floor(temp)) ? new_integer((long)temp) : new_number(temp);

	return MakeErrorCode(OK);
}

Error builtin_divide(Vector* v_params, Noun* result) {
	double r;
	size_t i;
	if (v_params->size == 0) {
		*result = new_number(1);
		return MakeErrorCode(OK);
	}

	if (!isnumber(v_params->data[0])) { return MakeErrorCode(ERROR_TYPE); }

	if (v_params->size == 1) {
		*result = new_number(1.0 / getnumber(v_params->data[0]));
		return MakeErrorCode(OK);
	}

	r = v_params->data[0].value.number_v;

	for (i = 1; i < v_params->size; i++) {
		if (!isnumber(v_params->data[i])) { return MakeErrorCode(ERROR_TYPE); }
		r /= getnumber(v_params->data[i]);
	}

	*result = new_number(r);
	return MakeErrorCode(OK);
}

Error builtin_less(Vector* v_params, Noun* result) {
	size_t i;
	if (v_params->size <= 1) {
		*result = sym_t;
		return MakeErrorCode(OK);
	}

	for (i = 0; i < v_params->size - 1; i++) {
		if (getnumber(coerce(v_params->data[i], f64_t))
		    > getnumber(coerce(v_params->data[i + 1], f64_t))) {
			*result = nil;
			return MakeErrorCode(OK);
		}
	}

	*result = sym_true;
	return MakeErrorCode(OK);
}

Error builtin_greater(Vector* v_params, Noun* result) {
	size_t i;
	if (v_params->size <= 1) {
		*result = sym_t;
		return MakeErrorCode(OK);
	}

	for (i = 0; i < v_params->size - 1; i++) {
		if (getnumber(coerce(v_params->data[i], f64_t))
		    < getnumber(coerce(v_params->data[i + 1], f64_t))) {
			*result = nil;
			return MakeErrorCode(OK);
		}
	}

	*result = sym_true;
	return MakeErrorCode(OK);
}
