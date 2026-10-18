/* FP state across context switches: Nproc forked processes (same image,
 * no exec) run fpwork concurrently; every one must see 0 mismatches. */
#include "../test.h"
#include "fpwork.h"

enum { Niter = 5000000, Nproc = 4 };

void
main(void)
{
	int i;
	Waitmsg *w;

	teststart();
	for(i = 0; i < Nproc; i++)
		if(rfork(RFPROC) == 0){
			long bad = fpwork((i+1)*100.0, Niter);
			if(bad)
				print("FAIL proc %d base %d: %ld mismatches of %d\n",
					i, (i+1)*100, bad, Niter);
			exits(bad? "bad": nil);
		}
	for(i = 0; i < Nproc; i++){
		w = wait();
		if(w == nil || w->msg[0] != '\0')
			testfail("child %d: %s", i, w? w->msg: "wait failed");
		free(w);
	}
	testend();
}
