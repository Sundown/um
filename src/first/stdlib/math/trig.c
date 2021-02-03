#include "../../../um.h"

Error builtin_sin(Vector* v_params, Noun* result) {
	if (v_params->size == 1) {
		Noun a = v_params->data[0];
		if (a.type != f64_t) { return MakeErrorCode(ERROR_TYPE); }
		*result = new_number(sin(a.value.number_v));
		return MakeErrorCode(OK);
	} else
		return MakeErrorCode(ERROR_ARGS);
}

Error builtin_cos(Vector* v_params, Noun* result) {
	if (v_params->size == 1) {
		Noun a = v_params->data[0];
		if (a.type != f64_t) { return MakeErrorCode(ERROR_TYPE); }
		*result = new_number(cos(a.value.number_v));
		return MakeErrorCode(OK);
	} else
		return MakeErrorCode(ERROR_ARGS);
}

Error builtin_tan(Vector* v_params, Noun* result) {
	if (v_params->size == 1) {
		Noun a = v_params->data[0];
		if (a.type != f64_t) { return MakeErrorCode(ERROR_TYPE); }
		*result = new_number(tan(a.value.number_v));
		return MakeErrorCode(OK);
	} else
		return MakeErrorCode(ERROR_ARGS);
}
