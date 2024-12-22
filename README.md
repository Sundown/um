### Um

:city_sunrise: <i>A single-header LISP interpreter featuring garbage collection, tail-call optimisation, and macros.</i>

This repository is no longer being maintained, future updates to Um may appear in [sundown/Bias](https://github.com/sundown/Bias).

**Example:**

```lisp
(defun fizzbuzz (x y)                       (defun fact (n)
    (print (switch 0                            (if (= n 1)
        ((% x 15) "FizzBuzz")                       1
        ((% x 3) "Fizz")                            (* n (fact (- n 1)))))
        ((% x 5) "Buzz")
        (0 x)))                             (defun fib (n)
                                                (switch n
    (if (< x y)                                     (1 0)
        (fizzbuzz (+ x 1) y)                        (2 1)
        nil))                                       (n (+ (fib (- n 1)) (fib (- n 2))))))
```

**Running:**
To build examples run `cd examples/ && make`.

Um is not ultimately intented to be a standalone interpreter, however `um_repl()` and `interpret_string()` functions are included as well as accompanying examples within `examples/` which demonstrate it's use as a traditional command-line interpreter. 

Otherwise `#include path/um.h` in your project and build as you normally would. Specifying `-ansi` or `std=C89/C99` will not work as the interpreter is written in C11.

**Inspirations:**

-   https://www.lwh.jp/lisp/
-   https://github.com/rxi/fe
-   https://github.com/rui314/minilisp
