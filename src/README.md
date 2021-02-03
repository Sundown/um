### Source layout

The interpreter's source code is split between 3 stages:

1. `first/` Initialisation, standard library definition, and REPL/read-file functionality.
2. `second/` Declaration of the supporting structure and foundations of the interpreter.
3. `third/` The final stages in interpretation: evaluation, application, and memory management.

`um.h` contains global declarations required across files.

`main.c` why are you here?
