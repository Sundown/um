#include "../../../um.h"

Error builtin_pow(Vector* v_params, Noun* result) {
	if (v_params->size != 2) { return MakeErrorCode(ERROR_ARGS); }

	double temp = pow(coerce(v_params->data[0], f64_t).value.number_v,
			  coerce(v_params->data[1], f64_t).value.number_v);

	*result
	    = isnormal(temp) && (temp == floor(temp)) ? new_integer((long)temp) : new_number(temp);

	return MakeErrorCode(OK);
}

Error builtin_cbrt(Vector* v_params, Noun* result) {
	if (v_params->size != 1) { return MakeErrorCode(ERROR_ARGS); }

	double temp = cbrt(coerce(v_params->data[0], f64_t).value.number_v);

	*result
	    = isnormal(temp) && (temp == floor(temp)) ? new_integer((long)temp) : new_number(temp);

	return MakeErrorCode(OK);
}
