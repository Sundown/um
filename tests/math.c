#include "../um.h"

#include <assert.h>

void math(void) {
	um_Result r0 = um_interpret_string("1");
	assert(!isnil(r0.data));
	assert(r0.data.type == number_t);
	assert(r0.data.value.number == 1);

	r0 = um_interpret_string("(type (math::tan 1.2))");
	assert(!isnil(r0.data));
	assert(r0.data.type == type_t);
	assert(r0.data.value.type_v == number_t);
}

int main(void) {
	um_init();
	math();
	return 0;
}
