### Um

:city_sunrise: <i>A Somewhat Functional Language</i>

Yes it's called Um, no it wasn't my idea.

This hobby-project is still a work in progress and may be subject to (repeated) fundamental reworks.

**Example:**

```lisp
(defun fizzbuzz (x y)
    (print (switch 0
        ((% x 15) "FizzBuzz")
        ((% x 3) "Fizz")
        ((% x 5) "Buzz")
        (0 x))) ; else

    (if (< x y) (fizzbuzz (+ x 1) y)))

(fizzbuzz 1 101)
```

**Warning:**

This interpreter is currently lacking many features one would commonly expect such as IO and a greater math library. These will be added and documented in the future. 

**Running:**

-   First, read the [doc]() (you can't, it doesn't exist yet).
-   `make` will compile Um for your system.
-   `make run` and `make install` are also available.

**Inspirations:**

-   https://www.lwh.jp/lisp/
-   https://github.com/rxi/fe
-   https://github.com/rui314/minilisp
