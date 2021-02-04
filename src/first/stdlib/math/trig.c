#include "../../../um.h"

Error builtin_sin(Vector* v_params, Noun* result) {
	if (v_params->size == 1) {
		*result = new_number(sin(coerce(v_params->data[0], f64_t).value.number_v));
		return MakeErrorCode(OK);
	} else {
		return MakeErrorCode(ERROR_ARGS);
	}
}

Error builtin_asin(Vector* v_params, Noun* result) {
	if (v_params->size == 1) {
		*result = new_number(asin(coerce(v_params->data[0], f64_t).value.number_v));
		return MakeErrorCode(OK);
	} else {
		return MakeErrorCode(ERROR_ARGS);
	}
}

Error builtin_cos(Vector* v_params, Noun* result) {
	if (v_params->size == 1) {
		*result = new_number(cos(coerce(v_params->data[0], f64_t).value.number_v));
		return MakeErrorCode(OK);
	} else {
		return MakeErrorCode(ERROR_ARGS);
	}
}

Error builtin_acos(Vector* v_params, Noun* result) {
	if (v_params->size == 1) {
		*result = new_number(acos(coerce(v_params->data[0], f64_t).value.number_v));
		return MakeErrorCode(OK);
	} else {
		return MakeErrorCode(ERROR_ARGS);
	}
}

Error builtin_tan(Vector* v_params, Noun* result) {
	if (v_params->size == 1) {
		*result = new_number(tan(coerce(v_params->data[0], f64_t).value.number_v));
		return MakeErrorCode(OK);
	} else {
		return MakeErrorCode(ERROR_ARGS);
	}
}

Error builtin_atan(Vector* v_params, Noun* result) {
	if (v_params->size == 1) {
		*result = new_number(atan(coerce(v_params->data[0], f64_t).value.number_v));
		return MakeErrorCode(OK);
	} else {
		return MakeErrorCode(ERROR_ARGS);
	}
}
