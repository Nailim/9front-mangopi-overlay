/* control: a test that stops without a verdict (as a crash would) */
#include "../test.h"

void
main(void)
{
	teststart();
	print("working...\n");
	exits(nil);
}
