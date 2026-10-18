/* control: a test that reports a failure -> "see FAIL lines" */
#include "../test.h"

void
main(void)
{
	teststart();
	testfail("deliberate failure %d", 42);
	testend();
}
