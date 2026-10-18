/* control: built with a stamp other than the suite's -> "stale".
 * -DBUILD=<stamp> comes from the command line, so undefine it first. */
#undef BUILD
#define BUILD 1
#include "../test.h"

void
main(void)
{
	teststart();
	testend();
}
