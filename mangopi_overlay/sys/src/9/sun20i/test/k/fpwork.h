/*
 * FP work for the context-switch tests.  The three operations use the
 * cached constants (FREGTWO+FREGHALF, FREGHALF, FREGONE: F28-F31) and
 * the compare uses a live FP register, so a process whose FP state is
 * lost or clobbered by a switch gets a mismatch.
 */
static long
fpwork(double base, long n)
{
	double a;
	long bad, i;

	bad = 0;
	for(i = 0; i < n; i++){
		a = base;
		a += 2.5;
		a -= 0.5;
		a *= 1.0;
		if(a != base + 2.0)
			bad++;
	}
	return bad;
}
