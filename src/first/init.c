#include "../um.h"

void um_init() {
	srand((unsigned)time(0));
	if (!um_global_symbol_capacity) { um_global_symbol_capacity = 1000; }
	env = env_create(nil, um_global_symbol_capacity);

	symbol_table = malloc(um_global_symbol_capacity * sizeof(char*));

	sym_t = sym_true;
	sym_quote = intern("quote");
	sym_quasiquote = intern("quasiquote");
	sym_unquote = intern("unquote");
	sym_unquote_splicing = intern("unquote-splicing");
	sym_def = intern("def");
	sym_defun = intern("defun");
	sym_fn = intern("lambda");
	sym_if = intern("if");
	sym_cond = intern("cond");
	sym_switch = intern("switch");
	sym_match = intern("match");

	sym_mac = intern("mac");
	sym_apply = intern("apply");
	sym_cons = intern("cons");
	sym_sym = intern("sym");
	sym_string = intern("string");
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

	env_assign(env, sym_t.value.symbol, sym_t);
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
	ASSIGN_BUILTIN("<", builtin_less);
	ASSIGN_BUILTIN("=", builtin_eq);
	ASSIGN_BUILTIN("%", builtin_modulo);
	ASSIGN_BUILTIN("~", builtin_eq_l);
	ASSIGN_BUILTIN("__builtin_pow", builtin_pow);
	ASSIGN_BUILTIN("__builtin_cbrt", builtin_cbrt);
	ASSIGN_BUILTIN("int", builtin_int);
	ASSIGN_BUILTIN("not", builtin_not);
	ASSIGN_BUILTIN("sin", builtin_sin);
	ASSIGN_BUILTIN("cos", builtin_cos);
	ASSIGN_BUILTIN("tan", builtin_tan);
	ASSIGN_BUILTIN("len", builtin_len);
	ASSIGN_BUILTIN("eval", builtin_eval);
	ASSIGN_BUILTIN("type", builtin_type);
	ASSIGN_BUILTIN("exit", builtin_exit);
	ASSIGN_BUILTIN(">", builtin_greater);
	ASSIGN_BUILTIN("apply", builtin_apply);
	ASSIGN_BUILTIN("macex", builtin_macex);
	ASSIGN_BUILTIN("string", builtin_string);
	ASSIGN_BUILTIN("print", builtin_print);
	ASSIGN_BUILTIN("pair?", builtin_pairp);
	ASSIGN_BUILTIN("float", builtin_float);
	ASSIGN_BUILTIN("range", builtin_range);
	ASSIGN_BUILTIN("coerce", builtin_coerce);
	ASSIGN_BUILTIN("getlist", builtin_getlist);
	ASSIGN_BUILTIN("setlist", builtin_setlist);

	ASSIGN_BUILTIN("if", new_builtin(NULL).value.builtin);
	ASSIGN_BUILTIN("fn", new_builtin(NULL).value.builtin);
	ASSIGN_BUILTIN("do", new_builtin(NULL).value.builtin);
	ASSIGN_BUILTIN("def", new_builtin(NULL).value.builtin);
	ASSIGN_BUILTIN("mac", new_builtin(NULL).value.builtin);
	ASSIGN_BUILTIN("cond", new_builtin(NULL).value.builtin);
	ASSIGN_BUILTIN("switch", new_builtin(NULL).value.builtin);
	ASSIGN_BUILTIN("match", new_builtin(NULL).value.builtin);
	ASSIGN_BUILTIN("defun", new_builtin(NULL).value.builtin);
	ASSIGN_BUILTIN("quote", new_builtin(NULL).value.builtin);
	ASSIGN_BUILTIN("lambda", new_builtin(NULL).value.builtin);

	ingest("\
(def (foldl proc init list)\
	(if !(null? list)\
		(foldl proc (proc init (car list)) (cdr list))\
		init))");

	ingest("\
(def (foldr p i l)\
	(if !(null? l)\
		(p (car l) (foldr p i (cdr l)))\
		i))");

	ingest("\
(def (null? x)\
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
	(if !(null? (car arg-lists))\
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
		('tan tan)\
		('range range)\
		('sqrt (lambda (x) (math::pow x (float 0.5))))\
		('cbrt __builtin_cbrt)\
		('square (lambda (x) (math::pow x 2)))\
		('cube (lambda (x) (math::pow x 3)))\
		('min (lambda (x) \
			(if (null? (cdr x))\
				(car x) \
				(foldl (lambda (a b) (if (< a b) a b)) (car x) (cdr x)))))\
		('max (lambda (x) \
			(if (null? (cdr x))\
				(car x) \
				(foldl (lambda (a b) (if (< a b) b a)) (car x) (cdr x)))))\
		('pow __builtin_pow)))");

	ingest("\
(def (for-each proc items)\
  	(if (null? items)\
   		true\
   		(if ((lambda (x) true) (proc (car items))) \
    			(for-each proc (cdr items)))))");

	ingest("\
(def (filter pred lst)\
   	(if (null? lst)\
    		()\
    		(if (pred (car lst))\
     			(cons (car lst)\
     			(filter pred (cdr lst)))\
   	(filter pred (cdr lst)))))");

	ingest("\
(defun curry (f)\
	(lambda (a) (lambda (b) (f a b))))");
}
