#include "../../../um.h"

#define intp(d) (bool)(isnormal(d) && (d == floor(d)))

Error builtin_add(Vector* v_params, Noun* result) {
	size_t ac = v_params->size;
	Noun a0 = v_params->data[0], a1 = v_params->data[1];
	if (ac == 1) {
		*result = a0.type == i32_t ? new (labs(a0.value.integer_v))
			: a0.type == f64_t ? new (fabs(a0.value.number_v))
					   : new ((long)0);
		return MakeErrorCode(OK);
	} else if (ac > 2 || ac < 1) {
		return MakeErrorCode(ERROR_ARGS);
	}

	if (a0.type == i32_t && a1.type == i32_t) {
		*result = new (a0.value.integer_v + a1.value.integer_v);
	} else {
		double _temp = coerce(a0, f64_t).value.number_v + coerce(a1, f64_t).value.number_v;

		*result = intp(_temp) ? new ((long)_temp) : new (_temp);
	}

	return MakeErrorCode(OK);
}

Error builtin_subtract(Vector* v_params, Noun* result) {
	size_t ac = v_params->size;
	Noun a0 = v_params->data[0], a1 = v_params->data[1];
	if (ac == 1) {
		*result = a0.type == i32_t ? new (-labs(a0.value.integer_v))
			: a0.type == f64_t ? new (-fabs(a0.value.number_v))
					   : new ((long)0);
		return MakeErrorCode(OK);
	} else if (ac > 2 || ac < 1) {
		return MakeErrorCode(ERROR_ARGS);
	}

	if (a0.type == i32_t && a1.type == i32_t) {
		*result = new (a0.value.integer_v - a1.value.integer_v);
	} else {
		double _temp = coerce(a0, f64_t).value.number_v - coerce(a1, f64_t).value.number_v;

		*result = intp(_temp) ? new ((long)_temp) : new (_temp);
	}

	return MakeErrorCode(OK);
}

Error builtin_modulo(Vector* v_params, Noun* result) {
	Noun a0 = v_params->data[0], a1 = v_params->data[1];
	if (v_params->size != 2) { return MakeErrorCode(ERROR_ARGS); }

	if (a0.type == i32_t && a1.type == i32_t) {
		*result = new (a0.value.integer_v % a1.value.integer_v);
	} else {
		long _temp = coerce(a0, i32_t).value.integer_v % coerce(a1, i32_t).value.integer_v;

		*result = new (_temp);
	}

	return MakeErrorCode(OK);
}

Error builtin_multiply(Vector* v_params, Noun* result) {
	Noun a0 = v_params->data[0], a1 = v_params->data[1];
	if (v_params->size != 2) { return MakeErrorCode(ERROR_ARGS); }

	if (a0.type == i32_t && a1.type == i32_t) {
		*result = new (a0.value.integer_v * a1.value.integer_v);
	} else {
		double _temp = coerce(a0, f64_t).value.number_v * coerce(a1, f64_t).value.number_v;

		*result = intp(_temp) ? new ((long)_temp) : new (_temp);
	}

	return MakeErrorCode(OK);
}

Error builtin_divide(Vector* v_params, Noun* result) {
	Noun a0 = v_params->data[0], a1 = v_params->data[1];
	if (v_params->size != 2) { return MakeErrorCode(ERROR_ARGS); }

	if (a0.type == i32_t && a1.type == i32_t) {
		*result = new (a0.value.integer_v / a1.value.integer_v);
	} else {
		double _temp = coerce(a0, f64_t).value.number_v / coerce(a1, f64_t).value.number_v;

		*result = intp(_temp) ? new ((long)_temp) : new (_temp);
	}

	return MakeErrorCode(OK);
}

Error builtin_less(Vector* v_params, Noun* result) {
	Noun a0 = v_params->data[0], a1 = v_params->data[1];
	if (v_params->size != 2) { return MakeErrorCode(ERROR_ARGS); }

	if (a0.type == i32_t && a1.type == i32_t) {
		*result = new ((bool)(a0.value.integer_v < a1.value.integer_v));
	} else {
		*result = new (
		    (bool)(coerce(a0, f64_t).value.number_v < coerce(a1, f64_t).value.number_v));
	}

	return MakeErrorCode(OK);
}

Error builtin_greater(Vector* v_params, Noun* result) {
	Noun a0 = v_params->data[0], a1 = v_params->data[1];
	if (v_params->size != 2) { return MakeErrorCode(ERROR_ARGS); }

	if (a0.type == i32_t && a1.type == i32_t) {
		*result = new ((bool)(a0.value.integer_v > a1.value.integer_v));
	} else {
		*result = new (
		    (bool)(coerce(a0, f64_t).value.number_v > coerce(a1, f64_t).value.number_v));
	}

	return MakeErrorCode(OK);
}
