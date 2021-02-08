#include "../um.h"

Noun sym_quote, sym_quasiquote, sym_const, sym_unquote, sym_unquote_splicing, sym_def, sym_defun,
    sym_fn, sym_if, sym_cond, sym_switch, sym_match, sym_mac, sym_apply, sym_cons, sym_string,
    sym_num, sym_int, sym_char, sym_do, sym_set, sym_true, sym_false, sym_nil_t, sym_pair_t,
    sym_noun_t, sym_f64_t, sym_i32_t, sym_builtin_t, sym_closure_t, sym_macro_t, sym_string_t,
    sym_input_t, sym_output_t, sym_error_t, sym_type_t, sym_bool_t;

Noun env;

void um_init() {
	srand((unsigned)time(0));
	if (!um_global_symbol_capacity) { um_global_symbol_capacity = 1000; }
	env = env_create(nil, um_global_symbol_capacity);

	symbol_table = malloc(um_global_symbol_capacity * sizeof(char*));

	sym_quote = intern("quote");
	sym_quasiquote = intern("quasiquote");
	sym_unquote = intern("unquote");
	sym_unquote_splicing = intern("unquote-splicing");
	sym_def = intern("def");
	sym_const = intern("const");
	sym_defun = intern("defun");
	sym_fn = intern("lambda");
	sym_if = intern("if");
	sym_cond = intern("cond");
	sym_switch = intern("switch");
	sym_match = intern("match");

	sym_mac = intern("mac");
	sym_apply = intern("apply");
	sym_cons = intern("cons");
	sym_string = intern("str");
	sym_num = intern("num");
	sym_int = intern("int");
	sym_char = intern("char");
	sym_do = intern("do");
	sym_set = intern("set");
	sym_true = intern("true");
	sym_false = intern("false");

	sym_nil_t = intern("@Nil");
	sym_pair_t = intern("@Pair");
	sym_noun_t = intern("@Noun");
	sym_f64_t = intern("@Float");
	sym_i32_t = intern("@Int");
	sym_builtin_t = intern("@Builtin");
	sym_closure_t = intern("@Closure");

	sym_macro_t = intern("@Macro");
	sym_string_t = intern("@String");
	sym_input_t = intern("@Input");
	sym_output_t = intern("@Output");
	sym_error_t = intern("@Error");
	sym_type_t = intern("@Type");
	sym_bool_t = intern("@Bool");

#define ASSIGN_BUILTIN(name, fn_ptr) env_assign(env, intern(name).value.symbol, new_builtin(fn_ptr))

	env_assign(env, sym_true.value.symbol, new ((bool)true));
	env_assign(env, sym_false.value.symbol, new ((bool)false));
	env_assign(env, intern("nil").value.symbol, nil);

	env_assign(env, sym_nil_t.value.symbol, new ((noun_type)nil_t));
	env_assign(env, sym_pair_t.value.symbol, new ((noun_type)pair_t));
	env_assign(env, sym_noun_t.value.symbol, new ((noun_type)noun_t));
	env_assign(env, sym_f64_t.value.symbol, new ((noun_type)f64_t));
	env_assign(env, sym_i32_t.value.symbol, new ((noun_type)i32_t));
	env_assign(env, sym_builtin_t.value.symbol, new ((noun_type)builtin_t));
	env_assign(env, sym_closure_t.value.symbol, new ((noun_type)closure_t));
	env_assign(env, sym_macro_t.value.symbol, new ((noun_type)macro_t));
	env_assign(env, sym_string_t.value.symbol, new ((noun_type)string_t));
	env_assign(env, sym_input_t.value.symbol, new ((noun_type)input_t));
	env_assign(env, sym_output_t.value.symbol, new ((noun_type)output_t));
	env_assign(env, sym_error_t.value.symbol, new ((noun_type)error_t));
	env_assign(env, sym_type_t.value.symbol, new ((noun_type)type_t));
	env_assign(env, sym_bool_t.value.symbol, new ((noun_type)bool_t));

	ASSIGN_BUILTIN("car", builtin_car);
	ASSIGN_BUILTIN("cdr", builtin_cdr);
	ASSIGN_BUILTIN("cons", builtin_cons);

	ASSIGN_BUILTIN("+", builtin_add);
	ASSIGN_BUILTIN("-", builtin_subtract);
	ASSIGN_BUILTIN("*", builtin_multiply);
	ASSIGN_BUILTIN("/", builtin_divide);
	ASSIGN_BUILTIN("%", builtin_modulo);

	ASSIGN_BUILTIN(">", builtin_greater);
	ASSIGN_BUILTIN("<", builtin_less);
	ASSIGN_BUILTIN("=", builtin_eq);
	ASSIGN_BUILTIN("eq?", builtin_eq);

	ASSIGN_BUILTIN("eqv?", builtin_eq_l);
	ASSIGN_BUILTIN("__builtin_pow", builtin_pow);
	ASSIGN_BUILTIN("__builtin_cbrt", builtin_cbrt);
	ASSIGN_BUILTIN("int", builtin_int);
	ASSIGN_BUILTIN("not", builtin_not);
	ASSIGN_BUILTIN("sin", builtin_sin);
	ASSIGN_BUILTIN("__builtin_cos", builtin_cos);
	ASSIGN_BUILTIN("__builtin_tan", builtin_tan);
	ASSIGN_BUILTIN("__builtin_asin", builtin_asin);
	ASSIGN_BUILTIN("__builtin_acos", builtin_acos);
	ASSIGN_BUILTIN("__builtin_atan", builtin_atan);
	ASSIGN_BUILTIN("len", builtin_len);
	ASSIGN_BUILTIN("eval", builtin_eval);
	ASSIGN_BUILTIN("type", builtin_type);
	ASSIGN_BUILTIN("exit", builtin_exit);
	ASSIGN_BUILTIN("apply", builtin_apply);
	ASSIGN_BUILTIN("macex", builtin_macex);
	ASSIGN_BUILTIN("str", builtin_string);
	ASSIGN_BUILTIN("print", builtin_print);
	ASSIGN_BUILTIN("pair?", builtin_pairp);
	ASSIGN_BUILTIN("float", builtin_float);
	ASSIGN_BUILTIN("range", builtin_range);
	ASSIGN_BUILTIN("coerce", builtin_coerce);
	ASSIGN_BUILTIN("getlist", builtin_getlist);
	ASSIGN_BUILTIN("and", builtin_and);
	ASSIGN_BUILTIN("setlist", builtin_setlist);

	ASSIGN_BUILTIN("if", new_builtin(NULL).value.builtin);
	ASSIGN_BUILTIN("fn", new_builtin(NULL).value.builtin);
	ASSIGN_BUILTIN("do", new_builtin(NULL).value.builtin);
	ASSIGN_BUILTIN("def", new_builtin(NULL).value.builtin);
	ASSIGN_BUILTIN("const", new_builtin(NULL).value.builtin);
	ASSIGN_BUILTIN("mac", new_builtin(NULL).value.builtin);
	ASSIGN_BUILTIN("cond", new_builtin(NULL).value.builtin);
	ASSIGN_BUILTIN("switch", new_builtin(NULL).value.builtin);
	ASSIGN_BUILTIN("match", new_builtin(NULL).value.builtin);
	ASSIGN_BUILTIN("defun", new_builtin(NULL).value.builtin);
	ASSIGN_BUILTIN("quote", new_builtin(NULL).value.builtin);
	ASSIGN_BUILTIN("lambda", new_builtin(NULL).value.builtin);

	ingest("\
(def (foldl proc init list)\
	(if !(nil? list)\
		(foldl proc (proc init (car list)) (cdr list))\
		init))");

	ingest("\
(def (foldr p i l)\
	(if !(nil? l)\
		(p (car l) (foldr p i (cdr l)))\
		i))");

	ingest("\
(def (nil? x)\
	(= x ()))");

	ingest("\
(def (list . items)\
	(foldr cons nil items))");

	ingest("\
(def (unary-map proc list)\
	(foldr\
		(lambda (x rest) (cons (proc x) rest))\
		nil\
		list))");

	ingest("\
(def (map proc . arg-lists)\
	(if !(nil? (car arg-lists))\
		(cons\
			(apply proc (unary-map car arg-lists))\
			(apply map (cons proc (unary-map cdr arg-lists))))\
		nil))");

	ingest("\
(def (caar x)\
	(car (car x)))");

	ingest("\
(def (cadr x)\
	(car (cdr x)))");

	ingest("\
(mac unless (cond expr)\
	(list 'if condition () expr))");

	ingest("\
(def (append a b)\
	(foldr cons b a))");

	ingest("\
(mac quasiquote (x)\
 	(if (pair? x)\
  		(if (= (car x) 'unquote)\
   			(cadr x)\
    			(if (if (pair? (car x)) (= (caar x) 'unquote-splicing))\
     				(list 'append (cadr (car x)) (list 'quasiquote (cdr x)))\
     				(list 'cons (list 'quasiquote (car x)) (list 'quasiquote (cdr x)))))\
    		(list 'quote x)))");

	ingest("\
(mac let (defs . body)\
	`((lambda ,(map car defs) ,@body) ,@(map cadr defs)))");

	ingest("\
(defun std (fun)\
	(switch fun \
		('list list) \
		('map map) \
		('coerce coerce)))");

	ingest("\
(defun math (fun)\
	(switch fun\
		('pi 3.1415926535897931)\
		('e 2.7182818284590452)\
		('tan __builtin_tan)\
		('sin __builtin_sin)\
		('cos __builtin_cos)\
		('atan __builtin_atan)\
		('asin __builtin_asin)\
		('acos __builtin_acos)\
		('range range)\
		('sqrt (lambda (x) (math::pow x (float 0.5))))\
		('cbrt __builtin_cbrt)\
		('square (lambda (x) (math::pow x 2)))\
		('cube (lambda (x) (math::pow x 3)))\
		('min (lambda (x) \
			(if (nil? (cdr x))\
				(car x) \
				(foldl (lambda (a b) (if (< a b) a b)) (car x) (cdr x)))))\
		('max (lambda (x) \
			(if (nil? (cdr x))\
				(car x) \
				(foldl (lambda (a b) (if (< a b) b a)) (car x) (cdr x)))))\
		('pow __builtin_pow)))");

	ingest("\
(def (for-each proc items)\
  	(if (nil? items)\
   		true\
   		(if ((lambda (x) true) (proc (car items))) \
    			(for-each proc (cdr items)))))");

	ingest("\
(def (filter pred lst)\
   	(if (nil? lst)\
    		()\
    		(if (pred (car lst))\
     			(cons (car lst)\
     			(filter pred (cdr lst)))\
   	(filter pred (cdr lst)))))");

	ingest("\
(defun curry (f)\
	(lambda (a) (lambda (b) (f a b))))");
}
