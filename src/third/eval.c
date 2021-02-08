#include "../um.h"

Error eval_expr(Noun expr, Noun env, Noun* result) {
	Error err;
	Noun fn, op, args, cond, sym, val, name, macro, *p, r, arg_names;
	size_t ss = stack_size;
	Vector v_params;
start:
	stack_add(env);
	cur_expr = isnil(expr) ? cur_expr : expr;
	if (expr.type == noun_t) {
		err = env_get(env, expr.value.symbol, result);
		return err;
	} else if (expr.type != pair_t) {
		*result = expr;
		return MakeErrorCode(OK);
	} else {
		op = car(expr);
		args = cdr(expr);

		if (op.type == noun_t) {
			if (op.value.symbol == sym_if.value.symbol) {
				p = &args;
				while (!isnil(*p)) {
					if (isnil(cdr(*p))
					    || (cdr(*p).type == bool_t
						&& cdr(*p).value.bool_v == false)) {

						expr = car(*p);
						goto start;
					}

					err = eval_expr(car(*p), env, &cond);
					if (err._) {
						stack_restore(ss);
						return err;
					}

					if (!isnil(cond) && coerce(cond, bool_t).value.bool_v) {

						expr = car(cdr(*p));
						goto start;
					}

					p = &cdr(cdr(*p));
				}

				*result = nil;
				stack_restore_add(ss, *result);
				Ok(())
			} else if (op.value.symbol == sym_cond.value.symbol) {
				Noun list = !isnil(args) ? reverse_list(args) : nil;
				Noun n = cons(nil, nil);
				while (!isnil(list)) {
					n = cons(cons(sym_if,
						      cons(!isnil(car(list)) ? car(car(list)) : nil,
							   cons(!isnil(car(list))
									&& !isnil(cdr(car(list)))
								    ? car(cdr(car(list)))
								    : nil,
								n))),
						 nil);
					pop(list);
				}

				err = eval_expr(car(n), env, &cond);
				if (err._) {
					stack_restore(ss);
					return err;
				}

				*result = cond;
				stack_restore_add(ss, *result);
				return MakeErrorCode(OK);
			} else if (op.value.symbol == sym_match.value.symbol) {
				Noun pred = !isnil(args) ? car(args) : nil;
				Noun list = !isnil(args) && !isnil(cdr(args))
					      ? reverse_list(cdr(args))
					      : nil;
				Noun n = cons(nil, nil);
				while (!isnil(list)) {
					n = cons(
					    cons(
						sym_if,
						cons(
						    cons(intern("apply"),
							 cons(!isnil(car(list)) ? car(car(list))
										: nil,
							      cons(cons(intern("quote"),
									cons(cons(pred, nil), nil)),
								   nil))),
						    cons(!isnil(car(list)) && !isnil(cdr(car(list)))
							     ? car(cdr(car(list)))
							     : nil,
							 n))),
					    nil);

					pop(list);
				}

				err = eval_expr(car(n), env, &cond);
				if (err._) {
					stack_restore(ss);
					return err;
				}

				*result = cond;
				stack_restore_add(ss, *result);
				return MakeErrorCode(OK);
			} else if (op.value.symbol == sym_switch.value.symbol) {
				Noun pred = !isnil(args) ? car(args) : nil;
				Noun list = !isnil(args) && !isnil(cdr(args))
					      ? reverse_list(cdr(args))
					      : nil;
				Noun n = cons(nil, nil);

				while (!isnil(list)) {
					n = cons(
					    cons(sym_if,
						 cons(cons(intern("="),
							   cons(!isnil(car(list)) ? car(car(list))
										  : nil,
								cons(pred, nil))),
						      cons(!isnil(car(list))
								   && !isnil(cdr(car(list)))
							       ? car(cdr(car(list)))
							       : nil,
							   n))),
					    nil);

					pop(list);
				}

				err = eval_expr(car(n), env, &cond);
				if (err._) {
					stack_restore(ss);
					return err;
				}

				*result = cond;
				stack_restore_add(ss, *result);
				return MakeErrorCode(OK);
			} else if (op.value.symbol == sym_set.value.symbol) {
				if (isnil(args) || isnil(cdr(args))) {
					stack_restore(ss);
					return MakeErrorCode(ERROR_ARGS);
				}

				sym = car(args);

				if (env_get(env, sym.value.symbol, result)._) {
					cur_expr = sym;
					return MakeErrorCode(ERROR_UNBOUND);
				}

				if (sym.type == pair_t) {

					err = new_closure(env, cdr(sym), cdr(args), result);
					sym = car(sym);

					if (sym.type != noun_t) {
						return MakeErrorCode(ERROR_TYPE);
					}

					err = env_assign_eq(env, sym.value.symbol, *result);
					*result = sym;
					stack_restore_add(ss, *result);
					return err;
				} else if (sym.type == noun_t) {

					err = eval_expr(car(cdr(args)), env, &val);
					if (err._) {
						stack_restore(ss);
						return err;
					}

					*result = val;
					err = env_assign_eq(env, sym.value.symbol, val);
					stack_restore_add(ss, *result);
					return err;
				} else {
					stack_restore(ss);
					return MakeErrorCode(ERROR_TYPE);
				}
			} else if (op.value.symbol == sym_def.value.symbol) {
				if (isnil(args) || isnil(cdr(args))) {
					stack_restore(ss);
					return MakeErrorCode(ERROR_ARGS);
				}

				sym = car(args);

				if (sym.type == pair_t) {

					err = new_closure(env, cdr(sym), cdr(args), result);
					sym = car(sym);

					if (sym.type != noun_t) {
						return MakeErrorCode(ERROR_TYPE);
					}

					err = env_assign(env, sym.value.symbol, *result);
					*result = sym;
					stack_restore_add(ss, *result);
					return err;
				} else if (sym.type == noun_t) {

					err = eval_expr(car(cdr(args)), env, &val);
					if (err._) {
						stack_restore(ss);
						return err;
					}

					*result = val;
					err = env_assign(env, sym.value.symbol, val);
					stack_restore_add(ss, *result);
					return err;
				} else {
					stack_restore(ss);
					return MakeErrorCode(ERROR_TYPE);
				}
			} else if (op.value.symbol == sym_const.value.symbol) {
				if (isnil(args) || isnil(cdr(args))) {
					stack_restore(ss);
					return MakeErrorCode(ERROR_ARGS);
				}

				sym = car(args);

				if (!env_get(env, sym.value.symbol, result)._) {
					if (!result->mut) { return MakeErrorCode(ERROR_NOMUT); }
				}

				if (sym.type == pair_t) {

					err = new_closure(env, cdr(sym), cdr(args), result);
					sym = car(sym);

					if (sym.type != noun_t) {
						return MakeErrorCode(ERROR_TYPE);
					}

					result->mut = false;
					err = env_assign_eq(env, sym.value.symbol, *result);
					*result = sym;
					stack_restore_add(ss, *result);
					return err;
				} else if (sym.type == noun_t) {
					err = eval_expr(car(cdr(args)), env, &val);
					if (err._) {
						stack_restore(ss);
						return err;
					}

					val.mut = false;
					*result = val;
					err = env_assign_eq(env, sym.value.symbol, val);
					stack_restore_add(ss, *result);
					return err;
				} else {
					stack_restore(ss);
					return MakeErrorCode(ERROR_TYPE);
				}
			} else if (op.value.symbol == sym_defun.value.symbol) {
				if (isnil(args) || isnil(cdr(args)) || isnil(cdr(cdr(args)))) {
					stack_restore(ss);
					return MakeErrorCode(ERROR_ARGS);
				}

				sym = car(args);

				if (sym.type == noun_t && car(cdr(args)).type == pair_t
				    && cdr(cdr(args)).type == pair_t) {

					err = new_closure(
					    env, car(cdr(args)), cdr(cdr(args)), result);

					if (sym.type != noun_t) {
						return MakeErrorCode(ERROR_TYPE);
					}

					err = env_assign_eq(env, sym.value.symbol, *result);
					*result = sym;
					stack_restore_add(ss, *result);
					return err;
				} else {
					stack_restore(ss);
					return MakeErrorCode(ERROR_TYPE);
				}
			} else if (op.value.symbol == sym_quote.value.symbol) {
				if (isnil(args) || !isnil(cdr(args))) {
					stack_restore(ss);
					return MakeErrorCode(ERROR_ARGS);
				}

				*result = car(args);
				stack_restore_add(ss, *result);
				return MakeErrorCode(OK);
			} else if (op.value.symbol == sym_fn.value.symbol
				   || op.value.symbol == intern("\\").value.symbol) {
				if (isnil(args) || isnil(cdr(args))) {
					stack_restore(ss);
					return MakeErrorCode(ERROR_ARGS);
				}

				err = new_closure(env, car(args), cdr(args), result);
				stack_restore_add(ss, *result);
				return err;
			} else if (op.value.symbol == sym_do.value.symbol) {

				*result = nil;
				while (!isnil(args)) {
					if (isnil(cdr(args))) {

						expr = car(args);
						stack_restore(ss);
						goto start;
					}

					err = eval_expr(car(args), env, result);
					if (err._) { return err; }

					args = cdr(args);
				}

				return MakeErrorCode(OK);
			} else if (op.value.symbol == sym_mac.value.symbol) {

				if (isnil(args) || isnil(cdr(args)) || isnil(cdr(cdr(args)))) {
					stack_restore(ss);
					return MakeErrorCode(ERROR_ARGS);
				}

				name = car(args);
				if (name.type != noun_t) {
					stack_restore(ss);
					return MakeErrorCode(ERROR_TYPE);
				}

				err = new_closure(env, car(cdr(args)), cdr(cdr(args)), &macro);
				if (!err._) {
					macro.type = macro_t;
					*result = name;
					err = env_assign(env, name.value.symbol, macro);
					stack_restore_add(ss, *result);
					return err;
				} else {
					stack_restore(ss);
					return err;
				}
			}
		}

		err = eval_expr(op, env, &fn);
		if (err._) {
			stack_restore(ss);
			return err;
		}

		vector_new(&v_params);
		p = &args;
		while (!isnil(*p)) {
			err = eval_expr(car(*p), env, &r);
			if (err._) {
				vector_free(&v_params);
				stack_restore(ss);
				return err;
			}

			vector_add(&v_params, r);
			p = &cdr(*p);
		}

		if (fn.type == closure_t) {
			arg_names = car(cdr(fn));
			env = env_create(car(fn), list_len(arg_names));
			expr = car(cdr(cdr(fn)));

			err = env_bind(env, arg_names, &v_params);
			if (err._) { return err; }

			vector_free(&v_params);
			goto start;
		} else {
			err = apply(fn, &v_params, result);
			vector_free(&v_params);
		}

		stack_restore_add(ss, *result);
		return err;
	}
}
