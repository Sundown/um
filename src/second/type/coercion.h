#include "../../um.h"

Noun nil_to_t(Noun x __attribute__((unused)), noun_type t);
Noun integer_to_t(long x, noun_type t);
Noun number_to_t(double x, noun_type t);
Noun noun_to_t(char* x, noun_type t);
Noun string_to_t(char* x, noun_type t);
Noun bool_to_t(bool x, noun_type t);
Noun type_to_t(noun_type x, noun_type t);
