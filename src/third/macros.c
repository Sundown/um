#include "../um.h"

Error macex(Noun expr, Noun* result) {
	Error err = MakeErrorCode(OK);
	Noun args, op, result2;
	Vector v_params;
	int ss;
	cur_expr = expr;

	if (expr.type != pair_t || !listp(expr)) {
		*result = expr;
		return MakeErrorCode(OK);
	} else {
		ss = stack_size;
		op = car(expr);

		if (op.type == noun_t && op.value.symbol == sym_quote.value.symbol) {
			*result = expr;
			return MakeErrorCode(OK);
		}

		args = cdr(expr);

		if (op.type == noun_t && !env_get(env, op.value.symbol, result)._
		    && result->type == macro_t) {

			op = *result;

			op.type = closure_t;

			noun_to_vector(args, &v_params);
			err = apply(op, &v_params, &result2);
			if (err._) {
				vector_free(&v_params);
				stack_restore(ss);
				return err;
			}

			err = macex(result2, result);
			if (err._) {
				vector_free(&v_params);
				stack_restore(ss);
				return err;
			}

			vector_free(&v_params);
			stack_restore_add(ss, *result);
			return MakeErrorCode(OK);
		} else {

			Noun expr2 = copy_list(expr);
			Noun h;
			for (h = expr2; !isnil(h); h = cdr(h)) {
				err = macex(car(h), &car(h));
				if (err._) {
					stack_restore(ss);
					return err;
				}
			}

			*result = expr2;
			stack_restore_add(ss, *result);
			return MakeErrorCode(OK);
		}
	}
}

Error macex_eval(Noun expr, Noun* result) {
	Noun e0;
	Error err = macex(expr, &e0);
	if (err._) { return err; }

	return eval_expr(e0, env, result);
}
