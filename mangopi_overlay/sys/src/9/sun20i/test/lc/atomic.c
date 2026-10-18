/* Atomic primitives (libc/riscv64/atomic*.s), single-process semantics:
 * return values and wraparound.  (aswap returns the OLD value, ainc the NEW one, acas 1 on success.)  Concurrency is covered by the kernel
 * tests, there is one hart. */
#include "../test.h"

static void
longs(void)
{
	Along x;

	x.v = 0;
	if(aswapl(&x, 5) != 0 || agetl(&x) != 5)
		testfail("aswapl/agetl");
	if(aswapl(&x, 9) != 5)
		testfail("aswapl returns old");
	if(aincl(&x, 1) != 10 || agetl(&x) != 10)
		testfail("aincl +1 returns new");
	if(aincl(&x, -1) != 9 || agetl(&x) != 9)
		testfail("aincl -1 returns new");
	if(acasl(&x, 9, 20) != 1 || agetl(&x) != 20)
		testfail("acasl success");
	if(acasl(&x, 9, 30) != 0 || agetl(&x) != 20)
		testfail("acasl failure leaves value");
	x.v = 0x7fffffff;
	if(aincl(&x, 1) != -2147483647-1)
		testfail("aincl wraps INT_MAX+1: %ld", x.v);
	x.v = -2147483647-1;
	if(aincl(&x, -1) != 0x7fffffff)
		testfail("aincl wraps INT_MIN-1: %ld", x.v);
}

static void
vlongs(void)
{
	Avlong v;

	v.v = 0;
	if(aswapv(&v, 5) != 0 || agetv(&v) != 5)
		testfail("aswapv/agetv");
	if(aincv(&v, 0x100000000LL) != 0x100000005LL)
		testfail("aincv returns new, crosses 32 bits");
	v.v = 1;
	if(aincv(&v, -2) != -1 || agetv(&v) != -1)
		testfail("aincv negative");
}

void
main(void)
{
	teststart();
	longs();
	vlongs();
	testend();
}
