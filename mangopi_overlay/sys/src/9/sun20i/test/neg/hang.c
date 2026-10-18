/* control: never finishes -> killed, "hung" */
#include "../test.h"

void
main(void)
{
	teststart();
	for(;;)
		sleep(1000);
}
