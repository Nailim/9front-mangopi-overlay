/* FP constants: bit patterns of literals and of the cached F28-F31
 * constants (0.0 0.5 1.0 2.0), in double and float. */
#include "../test.h"

static double sd = 2.5;

static uvlong
bitsd(double d)
{
	uvlong b;

	memmove(&b, &d, sizeof b);
	return b;
}

static ulong
bitsf(float f)
{
	ulong b;

	memmove(&b, &f, sizeof b);
	return b;
}

static void
chkd(char *nm, double d, uvlong want)
{
	if(bitsd(d) != want)
		testfail("double %s = %#16.16llux want %#16.16llux", nm, bitsd(d), want);
}

static void
chkf(char *nm, float f, ulong want)
{
	if(bitsf(f) != want)
		testfail("float %s = %#8.8lux want %#8.8lux", nm, bitsf(f), want);
}

void
main(void)
{
	double x;
	float f;

	teststart();

	chkd("0.0", 0.0, 0ULL);
	chkd("1.0", 1.0, 0x3ff0000000000000ULL);
	chkd("2.0", 2.0, 0x4000000000000000ULL);
	chkd("3.0", 3.0, 0x4008000000000000ULL);
	chkd("4.0", 4.0, 0x4010000000000000ULL);
	chkd("-1.0", -1.0, 0xbff0000000000000ULL);
	chkd("0.5", 0.5, 0x3fe0000000000000ULL);
	chkd("1.5", 1.5, 0x3ff8000000000000ULL);
	chkd("2.5", 2.5, 0x4004000000000000ULL);
	chkd("0.1", 0.1, 0x3fb999999999999aULL);
	chkd("static 2.5", sd, 0x4004000000000000ULL);

	/* the cached constants used in arithmetic */
	x = sd;
	chkd("2.5+2.0", x + 2.0, 0x4012000000000000ULL);	/* 4.5 */
	chkd("2.5-0.5", x - 0.5, 0x4000000000000000ULL);	/* 2.0 */
	chkd("2.5*1.0", x * 1.0, 0x4004000000000000ULL);
	chkd("2.5*2.0", x * 2.0, 0x4014000000000000ULL);	/* 5.0 */
	chkd("2.5*0.5", x * 0.5, 0x3ff4000000000000ULL);	/* 1.25 */

	/* single precision: a double pattern in F28-F31 used as a float is a NaN */
	f = 1.0;
	chkf("1.0f", f, 0x3f800000UL);
	f = sd;
	chkf("(float)2.5", f, 0x40200000UL);
	f = f + 0.5;
	chkf("2.5f+0.5", f, 0x40400000UL);	/* 3.0 */
	f = f * 2.0;
	chkf("3.0f*2.0", f, 0x40c00000UL);	/* 6.0 */
	f = f - 1.0;
	chkf("6.0f-1.0", f, 0x40a00000UL);	/* 5.0 */

	testend();
}
