/* FP state across fork+exec: the children run fpwork in a freshly exec'd
 * image (exercises forkretfp / procsetup).  A loss of FP state on the
 * exec path shows as mismatches, not as a crash. */
#include "../test.h"
#include "fpwork.h"

#define SELF "/rc/test/k/fpexec"	/* where the bootfs puts this binary */

enum { Niter = 5000000, Nproc = 4 };

void
main(int argc, char **argv)
{
	int i;
	char b[16];
	Waitmsg *w;

	if(argc == 3 && strcmp(argv[1], "child") == 0){
		long bad = fpwork((double)atoi(argv[2]), Niter);
		if(bad)
			print("FAIL base %s: %ld mismatches of %d\n", argv[2], bad, Niter);
		exits(bad? "bad": nil);
	}

	teststart();
	fpwork(1.0, 1000);	/* warm: pull the pages into the image cache */
	for(i = 0; i < Nproc; i++)
		if(rfork(RFPROC) == 0){
			snprint(b, sizeof b, "%d", (i+1)*100);
			execl(SELF, "fpexec", "child", b, nil);
			sysfatal("exec %s: %r", SELF);
		}
	for(i = 0; i < Nproc; i++){
		w = wait();
		if(w == nil || w->msg[0] != '\0')
			testfail("child %d: %s", i, w? w->msg: "wait failed");
		free(w);
	}
	testend();
}
