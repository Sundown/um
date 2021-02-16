### Um

:city_sunrise: <i>A Somewhat Functional Language</i>

Yes it's called Um, no it wasn't my idea.

This hobby-project is still a work in progress and may be subject to (repeated) fundamental reworks.

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

**Warning:**

This interpreter is currently lacking many features one would commonly expect such as IO and a greater math library. These will be added and documented in the future.

**Running:**

First, read the [doc]() (you can't, it doesn't exist yet).

Um is not ultimately intented to be a standalone interpreter, however `um_repl()` and `interpret_string()` functions are included as well as accompanying examples within `examples/`.

To build examples run `cd examples/ && make`.

Otherwise `#include path/um.h` in your project and build as you normally would. Specifying `-ansi` or `std=C89/C99` will not work as the interpreter is written in C11. Eventual elimination of C11 extentions will make this C89 compatible but this will take time.

**Inspirations:**

-   https://www.lwh.jp/lisp/
-   https://github.com/rxi/fe
-   https://github.com/rui314/minilisp
