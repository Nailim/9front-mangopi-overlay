/*
 * Common header for the sun20i regression tests.
 *
 * Every test prints, in this order:
 *	build <stamp>		(first line; the runner compares it with the
 *				 suite's STAMP, a mismatch means a stale binary)
 *	FAIL <what>		(zero or more)
 *	ok | FAILED		(last line)
 * and exits nil on success, "failed" otherwise.  Tests whose output is
 * compared with a reference file (the generated ones) print the build
 * line first and then only data; the runner supplies the verdict.
 */
#include <u.h>
#include <libc.h>

#ifndef BUILD
#define BUILD 0
#endif

static int testbad;

static void
teststart(void)
{
	/* RISC-V never traps on FP exceptions; amd64 Plan 9 does, so
	 * make the 6c reference builds behave the same */
	setfcr(getfcr() & ~(FPINVAL|FPZDIV|FPOVFL|FPUNFL|FPINEX));
	print("build %lud\n", (ulong)(BUILD+0));
}

static void
testfail(char *fmt, ...)
{
	va_list ap;
	char buf[512];

	va_start(ap, fmt);
	vseprint(buf, buf+sizeof buf, fmt, ap);
	va_end(ap);
	print("FAIL %s\n", buf);
	testbad++;
}

static void
testend(void)
{
	print("%s\n", testbad == 0? "ok": "FAILED");
	exits(testbad == 0? nil: "failed");
}
