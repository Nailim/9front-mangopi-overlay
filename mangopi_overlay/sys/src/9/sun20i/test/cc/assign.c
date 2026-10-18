/* Compound assignment: integer ops, narrow destinations (result wraps to
 * the destination type), and mixed int/float/double operands (the
 * OASADD/OASMUL neighbourhood).  Expected values worked out by hand. */
#include "../test.h"

static int gi;
static char gc;
static uchar gu;
static short gs;
static ushort gus;
static uint gui;
static vlong gv;
static float gf;
static double gd;

#define EQ(nm, got, want) \
	if((got) != (want)) testfail("%s: got %lld want %lld", nm, (vlong)(got), (vlong)(want));
#define EQD(nm, got, want) \
	if((got) != (want)) testfail("%s: got %g want %g", nm, (double)(got), (double)(want));

static void
ints(void)
{
	gi = 10;
	gi += 5;  EQ("int +=", gi, 15)
	gi -= 3;  EQ("int -=", gi, 12)
	gi *= 4;  EQ("int *=", gi, 48)
	gi /= 5;  EQ("int /=", gi, 9)
	gi %= 4;  EQ("int %=", gi, 1)
	gi <<= 4; EQ("int <<=", gi, 16)
	gi >>= 2; EQ("int >>=", gi, 4)
	gi |= 3;  EQ("int |=", gi, 7)
	gi &= 5;  EQ("int &=", gi, 5)
	gi ^= 1;  EQ("int ^=", gi, 4)

	gc = 100;  gc += 100;  EQ("char += wraps", gc, -56)
	gc = 'a';  gc *= 3;    EQ("char *= wraps", gc, 35)
	gu = 250;  gu += 10;   EQ("uchar += wraps", gu, 4)
	gu = 3;    gu -= 5;    EQ("uchar -= wraps", gu, 254)
	gu = 0x81; gu <<= 1;   EQ("uchar <<= wraps", gu, 2)
	gs = 32767; gs += 1;   EQ("short += wraps", gs, -32768)
	gus = 65535; gus += 2; EQ("ushort += wraps", gus, 1)
	gui = 0xffffffff; gui += 1; EQ("uint += wraps", gui, 0)
	gv = 1;    gv <<= 40;  EQ("vlong <<=", gv, 1099511627776LL)
	gv += gv;  EQ("vlong += self", gv, 2199023255552LL)
}

static void
mixed(void)
{
	/* int op= floating: computed in floating point, truncated on store */
	gi = 7;   gi *= 1.5;  EQ("int *= 1.5", gi, 10)
	gi = 10;  gi += 0.9;  EQ("int += 0.9", gi, 10)
	gi = -7;  gi *= 1.5;  EQ("int *= 1.5 (neg)", gi, -10)
	gc = 10;  gc += 2.7;  EQ("char += 2.7", gc, 12)
	gu = 200; gu *= 0.5;  EQ("uchar *= 0.5", gu, 100)

	/* double op= int */
	gd = 3;   gi = 10;
	gd += 2;  EQD("double += 2", gd, 5.0)
	gd /= 2;  EQD("double /= 2", gd, 2.5)
	gd *= gi; EQD("double *= int", gd, 25.0)
	gd -= gi; EQD("double -= int", gd, 15.0)

	/* float op= int, double op= float, float op= double */
	gf = 1.5; gf += 2;    EQD("float += 2", gf, 3.5)
	gf *= 2;              EQD("float *= 2", gf, 7.0)
	gi = 3;   gf -= gi;   EQD("float -= int", gf, 4.0)
	gd = 1.0; gf = 0.5;
	gd += gf;             EQD("double += float", gd, 1.5)
	gf = 2.0; gf += gd;   EQD("float += double", gf, 3.5)
}

void
main(void)
{
	teststart();
	ints();
	mixed();
	testend();
}
