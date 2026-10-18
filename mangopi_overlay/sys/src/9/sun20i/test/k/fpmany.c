/* Many short-lived processes, each with its own FP values, forked in
 * waves so process creation, exit and reuse overlap with switching. */
#include "../test.h"
#include "fpwork.h"

enum { Niter = 200000, Nproc = 24, Nwave = 3 };

void
main(void)
{
	int i, wv;
	Waitmsg *w;

	teststart();
	for(wv = 0; wv < Nwave; wv++){
		for(i = 0; i < Nproc; i++)
			if(rfork(RFPROC) == 0){
				long bad = fpwork((wv*Nproc + i + 1)*10.0, Niter);
				if(bad)
					print("FAIL wave %d proc %d: %ld mismatches of %d\n",
						wv, i, bad, Niter);
				exits(bad? "bad": nil);
			}
		for(i = 0; i < Nproc; i++){
			w = wait();
			if(w == nil || w->msg[0] != '\0')
				testfail("wave %d child: %s", wv, w? w->msg: "wait failed");
			free(w);
		}
	}
	testend();
}
