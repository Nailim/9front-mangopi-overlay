/* Narrowing casts in every context: store/compare/rvalue, via variables so
 * nothing is constant-folded.  Regression for the jc cast-comparison bug
 * (uchar == (uchar)i was compared untruncated). */
#include "../test.h"

static uchar ub[300];
static char  sb[300];
static ushort usb[300];
static short  ssb[300];

void
main(void)
{
	int i, n;
	uchar c;

	teststart();

	for(i = 0; i < 300; i++){
		ub[i] = i;
		sb[i] = i;
		usb[i] = i*300;
		ssb[i] = i*300;
	}
	for(i = 0; i < 300; i++){
		if(ub[i] != (uchar)i){
			testfail("uchar array i=%d b=%d cast=%d", i, ub[i], (uchar)i);
			break;
		}
	}
	for(i = 0; i < 300; i++){
		if(sb[i] != (char)i){
			testfail("char array i=%d b=%d cast=%d", i, sb[i], (char)i);
			break;
		}
	}
	for(i = 0; i < 300; i++){
		if(usb[i] != (ushort)(i*300)){
			testfail("ushort array i=%d b=%d cast=%d", i, usb[i], (ushort)(i*300));
			break;
		}
	}
	for(i = 0; i < 300; i++){
		if(ssb[i] != (short)(i*300)){
			testfail("short array i=%d b=%d cast=%d", i, ssb[i], (short)(i*300));
			break;
		}
	}

	i = 70000;
	if((ushort)i != 4464)
		testfail("(ushort)70000 = %d, want 4464", (ushort)i);
	if((short)i != 4464)
		testfail("(short)70000 = %d, want 4464", (short)i);
	i = 300;
	if((uchar)i != 44)
		testfail("(uchar)300 = %d, want 44", (uchar)i);
	if((char)i != 44)
		testfail("(char)300 = %d, want 44", (char)i);
	i = 200;
	if((char)i != -56)
		testfail("(char)200 = %d, want -56", (char)i);
	if((char)i >= 0)
		testfail("(char)200 >= 0");
	i = 40000;
	if((short)i != -25536)
		testfail("(short)40000 = %d, want -25536", (short)i);

	/* the contexts that always worked, as controls */
	i = 300;
	c = i;
	n = (uchar)i;
	if(c != 44 || n != 44 || (i & 0xff) != 44)
		testfail("controls: assign %d via-int %d mask %d", c, n, i & 0xff);

	testend();
}
