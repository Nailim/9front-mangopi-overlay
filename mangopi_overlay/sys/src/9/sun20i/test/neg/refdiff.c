/* control: output that differs from refdiff.ref (which says "goodbye") */
#include "../test.h"

void
main(void)
{
	teststart();
	print("hello\n");
	exits(nil);
}
