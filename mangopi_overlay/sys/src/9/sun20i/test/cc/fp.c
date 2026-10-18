#include "../test.h"

static double base[] = {0.0, 1.0, -1.0, 0.5, -0.5, 1.5, -2.5, 3.0, 0.1, -0.3, 1e-10, 1e10, 123456789.0, 2147483648.0, 4294967296.0, 1e300, -1e300, 1e-300, 4.9e-324, 1.7976931348623157e308, 16777217.0, 0.3333333333333333};
static vlong ivals[] = {0, 1, -1, 2, -2, 127, 128, 255, 256, -128, -129, 32767, 32768, 65535, 65536, -32768, -32769, 0x7fffffffLL, 0x80000000LL, 0xffffffffLL, 0x100000000LL, -0x80000000LL, -0x80000001LL, 0x7fffffffffffffffLL, (-0x7fffffffffffffffLL-1)};
#define NV 25
#define NB 22
#define NDV (NB+3)	/* + inf, -inf, -0.0 */
#define IINF NB
#define H0 0xcbf29ce484222325ULL

static double dv[NDV];
static float fv[NDV];
static double nanv;

/* all values arrive through calls so the shared front end cannot fold */
static vlong gv(int i) { return ivals[i]; }
static double gd(int i) { return dv[i]; }
static float gf(int i) { return fv[i]; }

static void
init(void)
{
	int i;

	for(i = 0; i < NB; i++)
		dv[i] = base[i];
	dv[NB] = dv[15] * dv[15];	/* +inf */
	dv[NB+1] = dv[15] * dv[16];	/* -inf */
	dv[NB+2] = dv[0] * dv[2];	/* -0.0, without relying on unary minus */
	nanv = dv[NB] - dv[NB];
	for(i = 0; i < NDV; i++)
		fv[i] = dv[i];
}

static uvlong mix(uvlong h, uvlong x) { return (h ^ x) * 0x100000001b3ULL; }

/* NaN payload and sign differ legitimately between x86 and RISC-V */
static uvlong
hd(uvlong h, double d)
{
	uvlong b;

	memmove(&b, &d, sizeof b);
	if((b & 0x7ff0000000000000ULL) == 0x7ff0000000000000ULL
	&& (b & 0x000fffffffffffffULL) != 0)
		b = 0x7ff8000000000000ULL;	/* any NaN */
	return mix(h, b);
}

static void
t_float(void)
{
	int i, j, c;
	float a, b, r;
	vlong w;
	uvlong h;

	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NDV; j++){
		a = gf(i); b = gf(j);
		r = a + b; h = hd(h, r);
	}
	print("float + %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NDV; j++){
		a = gf(i); b = gf(j);
		r = a - b; h = hd(h, r);
	}
	print("float - %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NDV; j++){
		a = gf(i); b = gf(j);
		r = a * b; h = hd(h, r);
	}
	print("float * %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NDV; j++){
		a = gf(i); b = gf(j);
		r = a / b; h = hd(h, r);
	}
	print("float / %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NDV; j++){
		a = gf(i); b = gf(j);
		c = a == b; h = mix(h, c);
		if(a == b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("float == %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NDV; j++){
		a = gf(i); b = gf(j);
		c = a != b; h = mix(h, c);
		if(a != b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("float != %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NDV; j++){
		a = gf(i); b = gf(j);
		c = a < b; h = mix(h, c);
		if(a < b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("float < %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NDV; j++){
		a = gf(i); b = gf(j);
		c = a <= b; h = mix(h, c);
		if(a <= b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("float <= %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NDV; j++){
		a = gf(i); b = gf(j);
		c = a > b; h = mix(h, c);
		if(a > b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("float > %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NDV; j++){
		a = gf(i); b = gf(j);
		c = a >= b; h = mix(h, c);
		if(a >= b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("float >= %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NDV; j++){
		a = gf(i); b = gf(j);
		c = a && b; h = mix(h, c); c = a || b; h = mix(h, c);
		if(a && b) h = mix(h, 1); else h = mix(h, 2);
		if(a || b) h = mix(h, 3); else h = mix(h, 4);
	}
	print("float &&|| %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++){
		a = gf(i); r = -a; h = hd(h, r); c = !a; h = mix(h, c);
		if(a) h = mix(h, 1); else h = mix(h, 2);
		r = a; r++; h = hd(h, r); r--; r--; h = hd(h, r);
	}
	print("float neg!++-- %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NDV; j++){
		a = gf(i); b = gf(j);
		r = a; r += b; h = hd(h, r);
	}
	print("float += %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NDV; j++){
		a = gf(i); b = gf(j);
		r = a; r -= b; h = hd(h, r);
	}
	print("float -= %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NDV; j++){
		a = gf(i); b = gf(j);
		r = a; r *= b; h = hd(h, r);
	}
	print("float *= %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NDV; j++){
		a = gf(i); b = gf(j);
		r = a; r /= b; h = hd(h, r);
	}
	print("float /= %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++){
		double x; x = gf(i); h = hd(h, x);
	}
	print("float->double %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++){
		char x; a = gf(i);
		if(a > -129.0 && a < 128.0){ x = a; h = mix(h, (vlong)x); }
	}
	print("float->char %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++){
		uchar x; a = gf(i);
		if(a > -1.0 && a < 256.0){ x = a; h = mix(h, (vlong)x); }
	}
	print("float->uchar %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++){
		short x; a = gf(i);
		if(a > -32769.0 && a < 32768.0){ x = a; h = mix(h, (vlong)x); }
	}
	print("float->short %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++){
		ushort x; a = gf(i);
		if(a > -1.0 && a < 65536.0){ x = a; h = mix(h, (vlong)x); }
	}
	print("float->ushort %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++){
		int x; a = gf(i);
		if(a > -2147483649.0 && a < 2147483648.0){ x = a; h = mix(h, (vlong)x); }
	}
	print("float->int %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++){
		uint x; a = gf(i);
		if(a > -1.0 && a < 4294967296.0){ x = a; h = mix(h, (vlong)x); }
	}
	print("float->uint %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++){
		long x; a = gf(i);
		if(a > -2147483649.0 && a < 2147483648.0){ x = a; h = mix(h, (vlong)x); }
	}
	print("float->long %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++){
		ulong x; a = gf(i);
		if(a > -1.0 && a < 4294967296.0){ x = a; h = mix(h, (vlong)x); }
	}
	print("float->ulong %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++){
		vlong x; a = gf(i);
		if(a > -9.0e18 && a < 9.0e18){ x = a; h = mix(h, (vlong)x); }
	}
	print("float->vlong %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++){
		uvlong x; a = gf(i);
		if(a > -1.0 && a < 9.0e18){ x = a; h = mix(h, (vlong)x); }
	}
	print("float->uvlong %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		char x; x = gv(i); r = x; h = hd(h, r);
	}
	print("char->float %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		uchar x; x = gv(i); r = x; h = hd(h, r);
	}
	print("uchar->float %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		short x; x = gv(i); r = x; h = hd(h, r);
	}
	print("short->float %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		ushort x; x = gv(i); r = x; h = hd(h, r);
	}
	print("ushort->float %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		int x; x = gv(i); r = x; h = hd(h, r);
	}
	print("int->float %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		uint x; x = gv(i); r = x; h = hd(h, r);
	}
	print("uint->float %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		long x; x = gv(i); r = x; h = hd(h, r);
	}
	print("long->float %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		ulong x; x = gv(i); r = x; h = hd(h, r);
	}
	print("ulong->float %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		vlong x; x = gv(i); r = x; h = hd(h, r);
	}
	print("vlong->float %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		uvlong x; x = gv(i); r = x; h = hd(h, r);
	}
	print("uvlong->float %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NV; j++){
		char x; a = gf(i); x = gv(j);
		r = a + x; h = hd(h, r); r = a - x; h = hd(h, r);
		r = a * x; h = hd(h, r); r = a / x; h = hd(h, r);
		c = a < x; h = mix(h, c); c = a == x; h = mix(h, c);
		if(a > x) h = mix(h, 1); else h = mix(h, 2);
		w = gv(j); r = (i & 1) ? a : w; h = hd(h, r);
	}
	print("float mixed char %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NV; j++){
		uchar x; a = gf(i); x = gv(j);
		r = a + x; h = hd(h, r); r = a - x; h = hd(h, r);
		r = a * x; h = hd(h, r); r = a / x; h = hd(h, r);
		c = a < x; h = mix(h, c); c = a == x; h = mix(h, c);
		if(a > x) h = mix(h, 1); else h = mix(h, 2);
		w = gv(j); r = (i & 1) ? a : w; h = hd(h, r);
	}
	print("float mixed uchar %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NV; j++){
		short x; a = gf(i); x = gv(j);
		r = a + x; h = hd(h, r); r = a - x; h = hd(h, r);
		r = a * x; h = hd(h, r); r = a / x; h = hd(h, r);
		c = a < x; h = mix(h, c); c = a == x; h = mix(h, c);
		if(a > x) h = mix(h, 1); else h = mix(h, 2);
		w = gv(j); r = (i & 1) ? a : w; h = hd(h, r);
	}
	print("float mixed short %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NV; j++){
		ushort x; a = gf(i); x = gv(j);
		r = a + x; h = hd(h, r); r = a - x; h = hd(h, r);
		r = a * x; h = hd(h, r); r = a / x; h = hd(h, r);
		c = a < x; h = mix(h, c); c = a == x; h = mix(h, c);
		if(a > x) h = mix(h, 1); else h = mix(h, 2);
		w = gv(j); r = (i & 1) ? a : w; h = hd(h, r);
	}
	print("float mixed ushort %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NV; j++){
		int x; a = gf(i); x = gv(j);
		r = a + x; h = hd(h, r); r = a - x; h = hd(h, r);
		r = a * x; h = hd(h, r); r = a / x; h = hd(h, r);
		c = a < x; h = mix(h, c); c = a == x; h = mix(h, c);
		if(a > x) h = mix(h, 1); else h = mix(h, 2);
		w = gv(j); r = (i & 1) ? a : w; h = hd(h, r);
	}
	print("float mixed int %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NV; j++){
		uint x; a = gf(i); x = gv(j);
		r = a + x; h = hd(h, r); r = a - x; h = hd(h, r);
		r = a * x; h = hd(h, r); r = a / x; h = hd(h, r);
		c = a < x; h = mix(h, c); c = a == x; h = mix(h, c);
		if(a > x) h = mix(h, 1); else h = mix(h, 2);
		w = gv(j); r = (i & 1) ? a : w; h = hd(h, r);
	}
	print("float mixed uint %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NV; j++){
		long x; a = gf(i); x = gv(j);
		r = a + x; h = hd(h, r); r = a - x; h = hd(h, r);
		r = a * x; h = hd(h, r); r = a / x; h = hd(h, r);
		c = a < x; h = mix(h, c); c = a == x; h = mix(h, c);
		if(a > x) h = mix(h, 1); else h = mix(h, 2);
		w = gv(j); r = (i & 1) ? a : w; h = hd(h, r);
	}
	print("float mixed long %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NV; j++){
		ulong x; a = gf(i); x = gv(j);
		r = a + x; h = hd(h, r); r = a - x; h = hd(h, r);
		r = a * x; h = hd(h, r); r = a / x; h = hd(h, r);
		c = a < x; h = mix(h, c); c = a == x; h = mix(h, c);
		if(a > x) h = mix(h, 1); else h = mix(h, 2);
		w = gv(j); r = (i & 1) ? a : w; h = hd(h, r);
	}
	print("float mixed ulong %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NV; j++){
		vlong x; a = gf(i); x = gv(j);
		r = a + x; h = hd(h, r); r = a - x; h = hd(h, r);
		r = a * x; h = hd(h, r); r = a / x; h = hd(h, r);
		c = a < x; h = mix(h, c); c = a == x; h = mix(h, c);
		if(a > x) h = mix(h, 1); else h = mix(h, 2);
		w = gv(j); r = (i & 1) ? a : w; h = hd(h, r);
	}
	print("float mixed vlong %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NV; j++){
		uvlong x; a = gf(i); x = gv(j);
		r = a + x; h = hd(h, r); r = a - x; h = hd(h, r);
		r = a * x; h = hd(h, r); r = a / x; h = hd(h, r);
		c = a < x; h = mix(h, c); c = a == x; h = mix(h, c);
		if(a > x) h = mix(h, 1); else h = mix(h, 2);
		w = gv(j); r = (i & 1) ? a : w; h = hd(h, r);
	}
	print("float mixed uvlong %llux\n", h);
	h = H0;
	{
		double d;

		r = 0.0f; h = hd(h, r);
		r = 0.5f; h = hd(h, r);
		r = 1.0f; h = hd(h, r);
		r = 2.0f; h = hd(h, r);
		r = 1.5f; h = hd(h, r);
		r = 2.5f; h = hd(h, r);
		r = 3.0f; h = hd(h, r);
		r = -0.5f; h = hd(h, r);
		r = -1.0f; h = hd(h, r);
		r = -2.0f; h = hd(h, r);
		r = 0.25f; h = hd(h, r);
		r = 100.0f; h = hd(h, r);
		for(i = 0; i < NDV; i++){
			a = gf(i);
			r = a * 0.5f; h = hd(h, r);
			r = a + 1.0f; h = hd(h, r);
			r = a - 2.0f; h = hd(h, r);
			r = a * 2.0f; h = hd(h, r);
			r = a / 2.0f; h = hd(h, r);
			r = 1.0f - a; h = hd(h, r);
			r = 0.0f - a; h = hd(h, r);
			c = a < 0.5f; h = mix(h, c);
			c = a == 0.0f; h = mix(h, c);
			c = a > 2.0f; h = mix(h, c);
			d = a; d = d * 0.5 + 1.0; h = hd(h, d);
		}
	}
	print("float literals %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++){
		float n; a = gf(i); n = nanv;
		c = a == n; h = mix(h, c); c = n == a; h = mix(h, c);
		if(a == n) h = mix(h, 1); else h = mix(h, 2);
		c = a != n; h = mix(h, c); c = n != a; h = mix(h, c);
		if(a != n) h = mix(h, 1); else h = mix(h, 2);
		c = a < n; h = mix(h, c); c = n < a; h = mix(h, c);
		if(a < n) h = mix(h, 1); else h = mix(h, 2);
		c = a <= n; h = mix(h, c); c = n <= a; h = mix(h, c);
		if(a <= n) h = mix(h, 1); else h = mix(h, 2);
		c = a > n; h = mix(h, c); c = n > a; h = mix(h, c);
		if(a > n) h = mix(h, 1); else h = mix(h, 2);
		c = a >= n; h = mix(h, c); c = n >= a; h = mix(h, c);
		if(a >= n) h = mix(h, 1); else h = mix(h, 2);
		r = a + n; h = hd(h, r); r = n * a; h = hd(h, r);
		if(n) h = mix(h, 1); else h = mix(h, 2);
		c = !n; h = mix(h, c);
	}
	(void)h;	/* nan float: not compared with 6c */
}

static void
t_double(void)
{
	int i, j, c;
	double a, b, r;
	vlong w;
	uvlong h;

	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NDV; j++){
		a = gd(i); b = gd(j);
		r = a + b; h = hd(h, r);
	}
	print("double + %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NDV; j++){
		a = gd(i); b = gd(j);
		r = a - b; h = hd(h, r);
	}
	print("double - %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NDV; j++){
		a = gd(i); b = gd(j);
		r = a * b; h = hd(h, r);
	}
	print("double * %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NDV; j++){
		a = gd(i); b = gd(j);
		r = a / b; h = hd(h, r);
	}
	print("double / %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NDV; j++){
		a = gd(i); b = gd(j);
		c = a == b; h = mix(h, c);
		if(a == b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("double == %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NDV; j++){
		a = gd(i); b = gd(j);
		c = a != b; h = mix(h, c);
		if(a != b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("double != %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NDV; j++){
		a = gd(i); b = gd(j);
		c = a < b; h = mix(h, c);
		if(a < b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("double < %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NDV; j++){
		a = gd(i); b = gd(j);
		c = a <= b; h = mix(h, c);
		if(a <= b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("double <= %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NDV; j++){
		a = gd(i); b = gd(j);
		c = a > b; h = mix(h, c);
		if(a > b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("double > %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NDV; j++){
		a = gd(i); b = gd(j);
		c = a >= b; h = mix(h, c);
		if(a >= b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("double >= %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NDV; j++){
		a = gd(i); b = gd(j);
		c = a && b; h = mix(h, c); c = a || b; h = mix(h, c);
		if(a && b) h = mix(h, 1); else h = mix(h, 2);
		if(a || b) h = mix(h, 3); else h = mix(h, 4);
	}
	print("double &&|| %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++){
		a = gd(i); r = -a; h = hd(h, r); c = !a; h = mix(h, c);
		if(a) h = mix(h, 1); else h = mix(h, 2);
		r = a; r++; h = hd(h, r); r--; r--; h = hd(h, r);
	}
	print("double neg!++-- %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NDV; j++){
		a = gd(i); b = gd(j);
		r = a; r += b; h = hd(h, r);
	}
	print("double += %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NDV; j++){
		a = gd(i); b = gd(j);
		r = a; r -= b; h = hd(h, r);
	}
	print("double -= %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NDV; j++){
		a = gd(i); b = gd(j);
		r = a; r *= b; h = hd(h, r);
	}
	print("double *= %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NDV; j++){
		a = gd(i); b = gd(j);
		r = a; r /= b; h = hd(h, r);
	}
	print("double /= %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++){
		float x; x = gd(i); h = hd(h, x);
	}
	print("double->float %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++){
		char x; a = gd(i);
		if(a > -129.0 && a < 128.0){ x = a; h = mix(h, (vlong)x); }
	}
	print("double->char %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++){
		uchar x; a = gd(i);
		if(a > -1.0 && a < 256.0){ x = a; h = mix(h, (vlong)x); }
	}
	print("double->uchar %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++){
		short x; a = gd(i);
		if(a > -32769.0 && a < 32768.0){ x = a; h = mix(h, (vlong)x); }
	}
	print("double->short %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++){
		ushort x; a = gd(i);
		if(a > -1.0 && a < 65536.0){ x = a; h = mix(h, (vlong)x); }
	}
	print("double->ushort %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++){
		int x; a = gd(i);
		if(a > -2147483649.0 && a < 2147483648.0){ x = a; h = mix(h, (vlong)x); }
	}
	print("double->int %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++){
		uint x; a = gd(i);
		if(a > -1.0 && a < 4294967296.0){ x = a; h = mix(h, (vlong)x); }
	}
	print("double->uint %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++){
		long x; a = gd(i);
		if(a > -2147483649.0 && a < 2147483648.0){ x = a; h = mix(h, (vlong)x); }
	}
	print("double->long %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++){
		ulong x; a = gd(i);
		if(a > -1.0 && a < 4294967296.0){ x = a; h = mix(h, (vlong)x); }
	}
	print("double->ulong %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++){
		vlong x; a = gd(i);
		if(a > -9.0e18 && a < 9.0e18){ x = a; h = mix(h, (vlong)x); }
	}
	print("double->vlong %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++){
		uvlong x; a = gd(i);
		if(a > -1.0 && a < 9.0e18){ x = a; h = mix(h, (vlong)x); }
	}
	print("double->uvlong %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		char x; x = gv(i); r = x; h = hd(h, r);
	}
	print("char->double %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		uchar x; x = gv(i); r = x; h = hd(h, r);
	}
	print("uchar->double %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		short x; x = gv(i); r = x; h = hd(h, r);
	}
	print("short->double %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		ushort x; x = gv(i); r = x; h = hd(h, r);
	}
	print("ushort->double %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		int x; x = gv(i); r = x; h = hd(h, r);
	}
	print("int->double %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		uint x; x = gv(i); r = x; h = hd(h, r);
	}
	print("uint->double %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		long x; x = gv(i); r = x; h = hd(h, r);
	}
	print("long->double %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		ulong x; x = gv(i); r = x; h = hd(h, r);
	}
	print("ulong->double %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		vlong x; x = gv(i); r = x; h = hd(h, r);
	}
	print("vlong->double %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		uvlong x; x = gv(i); r = x; h = hd(h, r);
	}
	print("uvlong->double %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NV; j++){
		char x; a = gd(i); x = gv(j);
		r = a + x; h = hd(h, r); r = a - x; h = hd(h, r);
		r = a * x; h = hd(h, r); r = a / x; h = hd(h, r);
		c = a < x; h = mix(h, c); c = a == x; h = mix(h, c);
		if(a > x) h = mix(h, 1); else h = mix(h, 2);
		w = gv(j); r = (i & 1) ? a : w; h = hd(h, r);
	}
	print("double mixed char %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NV; j++){
		uchar x; a = gd(i); x = gv(j);
		r = a + x; h = hd(h, r); r = a - x; h = hd(h, r);
		r = a * x; h = hd(h, r); r = a / x; h = hd(h, r);
		c = a < x; h = mix(h, c); c = a == x; h = mix(h, c);
		if(a > x) h = mix(h, 1); else h = mix(h, 2);
		w = gv(j); r = (i & 1) ? a : w; h = hd(h, r);
	}
	print("double mixed uchar %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NV; j++){
		short x; a = gd(i); x = gv(j);
		r = a + x; h = hd(h, r); r = a - x; h = hd(h, r);
		r = a * x; h = hd(h, r); r = a / x; h = hd(h, r);
		c = a < x; h = mix(h, c); c = a == x; h = mix(h, c);
		if(a > x) h = mix(h, 1); else h = mix(h, 2);
		w = gv(j); r = (i & 1) ? a : w; h = hd(h, r);
	}
	print("double mixed short %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NV; j++){
		ushort x; a = gd(i); x = gv(j);
		r = a + x; h = hd(h, r); r = a - x; h = hd(h, r);
		r = a * x; h = hd(h, r); r = a / x; h = hd(h, r);
		c = a < x; h = mix(h, c); c = a == x; h = mix(h, c);
		if(a > x) h = mix(h, 1); else h = mix(h, 2);
		w = gv(j); r = (i & 1) ? a : w; h = hd(h, r);
	}
	print("double mixed ushort %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NV; j++){
		int x; a = gd(i); x = gv(j);
		r = a + x; h = hd(h, r); r = a - x; h = hd(h, r);
		r = a * x; h = hd(h, r); r = a / x; h = hd(h, r);
		c = a < x; h = mix(h, c); c = a == x; h = mix(h, c);
		if(a > x) h = mix(h, 1); else h = mix(h, 2);
		w = gv(j); r = (i & 1) ? a : w; h = hd(h, r);
	}
	print("double mixed int %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NV; j++){
		uint x; a = gd(i); x = gv(j);
		r = a + x; h = hd(h, r); r = a - x; h = hd(h, r);
		r = a * x; h = hd(h, r); r = a / x; h = hd(h, r);
		c = a < x; h = mix(h, c); c = a == x; h = mix(h, c);
		if(a > x) h = mix(h, 1); else h = mix(h, 2);
		w = gv(j); r = (i & 1) ? a : w; h = hd(h, r);
	}
	print("double mixed uint %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NV; j++){
		long x; a = gd(i); x = gv(j);
		r = a + x; h = hd(h, r); r = a - x; h = hd(h, r);
		r = a * x; h = hd(h, r); r = a / x; h = hd(h, r);
		c = a < x; h = mix(h, c); c = a == x; h = mix(h, c);
		if(a > x) h = mix(h, 1); else h = mix(h, 2);
		w = gv(j); r = (i & 1) ? a : w; h = hd(h, r);
	}
	print("double mixed long %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NV; j++){
		ulong x; a = gd(i); x = gv(j);
		r = a + x; h = hd(h, r); r = a - x; h = hd(h, r);
		r = a * x; h = hd(h, r); r = a / x; h = hd(h, r);
		c = a < x; h = mix(h, c); c = a == x; h = mix(h, c);
		if(a > x) h = mix(h, 1); else h = mix(h, 2);
		w = gv(j); r = (i & 1) ? a : w; h = hd(h, r);
	}
	print("double mixed ulong %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NV; j++){
		vlong x; a = gd(i); x = gv(j);
		r = a + x; h = hd(h, r); r = a - x; h = hd(h, r);
		r = a * x; h = hd(h, r); r = a / x; h = hd(h, r);
		c = a < x; h = mix(h, c); c = a == x; h = mix(h, c);
		if(a > x) h = mix(h, 1); else h = mix(h, 2);
		w = gv(j); r = (i & 1) ? a : w; h = hd(h, r);
	}
	print("double mixed vlong %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NV; j++){
		uvlong x; a = gd(i); x = gv(j);
		r = a + x; h = hd(h, r); r = a - x; h = hd(h, r);
		r = a * x; h = hd(h, r); r = a / x; h = hd(h, r);
		c = a < x; h = mix(h, c); c = a == x; h = mix(h, c);
		if(a > x) h = mix(h, 1); else h = mix(h, 2);
		w = gv(j); r = (i & 1) ? a : w; h = hd(h, r);
	}
	print("double mixed uvlong %llux\n", h);
	h = H0;
	{
		double d;

		r = 0.0; h = hd(h, r);
		r = 0.5; h = hd(h, r);
		r = 1.0; h = hd(h, r);
		r = 2.0; h = hd(h, r);
		r = 1.5; h = hd(h, r);
		r = 2.5; h = hd(h, r);
		r = 3.0; h = hd(h, r);
		r = -0.5; h = hd(h, r);
		r = -1.0; h = hd(h, r);
		r = -2.0; h = hd(h, r);
		r = 0.25; h = hd(h, r);
		r = 100.0; h = hd(h, r);
		for(i = 0; i < NDV; i++){
			a = gd(i);
			r = a * 0.5; h = hd(h, r);
			r = a + 1.0; h = hd(h, r);
			r = a - 2.0; h = hd(h, r);
			r = a * 2.0; h = hd(h, r);
			r = a / 2.0; h = hd(h, r);
			r = 1.0 - a; h = hd(h, r);
			r = 0.0 - a; h = hd(h, r);
			c = a < 0.5; h = mix(h, c);
			c = a == 0.0; h = mix(h, c);
			c = a > 2.0; h = mix(h, c);
			d = a; d = d * 0.5 + 1.0; h = hd(h, d);
		}
	}
	print("double literals %llux\n", h);
	h = H0;
	for(i = 0; i < NDV; i++){
		double n; a = gd(i); n = nanv;
		c = a == n; h = mix(h, c); c = n == a; h = mix(h, c);
		if(a == n) h = mix(h, 1); else h = mix(h, 2);
		c = a != n; h = mix(h, c); c = n != a; h = mix(h, c);
		if(a != n) h = mix(h, 1); else h = mix(h, 2);
		c = a < n; h = mix(h, c); c = n < a; h = mix(h, c);
		if(a < n) h = mix(h, 1); else h = mix(h, 2);
		c = a <= n; h = mix(h, c); c = n <= a; h = mix(h, c);
		if(a <= n) h = mix(h, 1); else h = mix(h, 2);
		c = a > n; h = mix(h, c); c = n > a; h = mix(h, c);
		if(a > n) h = mix(h, 1); else h = mix(h, 2);
		c = a >= n; h = mix(h, c); c = n >= a; h = mix(h, c);
		if(a >= n) h = mix(h, 1); else h = mix(h, 2);
		r = a + n; h = hd(h, r); r = n * a; h = hd(h, r);
		if(n) h = mix(h, 1); else h = mix(h, 2);
		c = !n; h = mix(h, c);
	}
	(void)h;	/* nan double: not compared with 6c */
}

static int pi[] = {4, 5, 6, 7}, si[] = {-7, -5, 4, 6};
static double pf[] = {0.5, 1.5, 2.25, 3.75}, sf[] = {-3.75, -1.5, 0.5, 2.25};

static void
t_compound(void)
{
	int i, j;
	uvlong h;

	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		char r; char b; b = si[j];
		r = si[i]; r += b; h = mix(h, (vlong)r);
		r = si[i]; r -= b; h = mix(h, (vlong)r);
		r = si[i]; r *= b; h = mix(h, (vlong)r);
		r = si[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp char <- char %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		char r; uchar b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp char <- uchar %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		char r; short b; b = si[j];
		r = si[i]; r += b; h = mix(h, (vlong)r);
		r = si[i]; r -= b; h = mix(h, (vlong)r);
		r = si[i]; r *= b; h = mix(h, (vlong)r);
		r = si[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp char <- short %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		char r; ushort b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp char <- ushort %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		char r; int b; b = si[j];
		r = si[i]; r += b; h = mix(h, (vlong)r);
		r = si[i]; r -= b; h = mix(h, (vlong)r);
		r = si[i]; r *= b; h = mix(h, (vlong)r);
		r = si[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp char <- int %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		char r; uint b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp char <- uint %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		char r; long b; b = si[j];
		r = si[i]; r += b; h = mix(h, (vlong)r);
		r = si[i]; r -= b; h = mix(h, (vlong)r);
		r = si[i]; r *= b; h = mix(h, (vlong)r);
		r = si[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp char <- long %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		char r; ulong b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp char <- ulong %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		char r; vlong b; b = si[j];
		r = si[i]; r += b; h = mix(h, (vlong)r);
		r = si[i]; r -= b; h = mix(h, (vlong)r);
		r = si[i]; r *= b; h = mix(h, (vlong)r);
		r = si[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp char <- vlong %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		char r; uvlong b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp char <- uvlong %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		char r; float b; b = sf[j];
		r = si[i]; r += b; h = mix(h, (vlong)r);
		r = si[i]; r -= b; h = mix(h, (vlong)r);
		r = si[i]; r *= b; h = mix(h, (vlong)r);
		r = si[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp char <- float %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		char r; double b; b = sf[j];
		r = si[i]; r += b; h = mix(h, (vlong)r);
		r = si[i]; r -= b; h = mix(h, (vlong)r);
		r = si[i]; r *= b; h = mix(h, (vlong)r);
		r = si[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp char <- double %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		uchar r; char b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp uchar <- char %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		uchar r; uchar b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp uchar <- uchar %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		uchar r; short b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp uchar <- short %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		uchar r; ushort b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp uchar <- ushort %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		uchar r; int b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp uchar <- int %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		uchar r; uint b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp uchar <- uint %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		uchar r; long b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp uchar <- long %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		uchar r; ulong b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp uchar <- ulong %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		uchar r; vlong b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp uchar <- vlong %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		uchar r; uvlong b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp uchar <- uvlong %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		uchar r; float b; b = pf[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp uchar <- float %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		uchar r; double b; b = pf[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp uchar <- double %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		short r; char b; b = si[j];
		r = si[i]; r += b; h = mix(h, (vlong)r);
		r = si[i]; r -= b; h = mix(h, (vlong)r);
		r = si[i]; r *= b; h = mix(h, (vlong)r);
		r = si[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp short <- char %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		short r; uchar b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp short <- uchar %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		short r; short b; b = si[j];
		r = si[i]; r += b; h = mix(h, (vlong)r);
		r = si[i]; r -= b; h = mix(h, (vlong)r);
		r = si[i]; r *= b; h = mix(h, (vlong)r);
		r = si[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp short <- short %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		short r; ushort b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp short <- ushort %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		short r; int b; b = si[j];
		r = si[i]; r += b; h = mix(h, (vlong)r);
		r = si[i]; r -= b; h = mix(h, (vlong)r);
		r = si[i]; r *= b; h = mix(h, (vlong)r);
		r = si[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp short <- int %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		short r; uint b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp short <- uint %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		short r; long b; b = si[j];
		r = si[i]; r += b; h = mix(h, (vlong)r);
		r = si[i]; r -= b; h = mix(h, (vlong)r);
		r = si[i]; r *= b; h = mix(h, (vlong)r);
		r = si[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp short <- long %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		short r; ulong b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp short <- ulong %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		short r; vlong b; b = si[j];
		r = si[i]; r += b; h = mix(h, (vlong)r);
		r = si[i]; r -= b; h = mix(h, (vlong)r);
		r = si[i]; r *= b; h = mix(h, (vlong)r);
		r = si[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp short <- vlong %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		short r; uvlong b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp short <- uvlong %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		short r; float b; b = sf[j];
		r = si[i]; r += b; h = mix(h, (vlong)r);
		r = si[i]; r -= b; h = mix(h, (vlong)r);
		r = si[i]; r *= b; h = mix(h, (vlong)r);
		r = si[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp short <- float %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		short r; double b; b = sf[j];
		r = si[i]; r += b; h = mix(h, (vlong)r);
		r = si[i]; r -= b; h = mix(h, (vlong)r);
		r = si[i]; r *= b; h = mix(h, (vlong)r);
		r = si[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp short <- double %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		ushort r; char b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp ushort <- char %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		ushort r; uchar b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp ushort <- uchar %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		ushort r; short b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp ushort <- short %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		ushort r; ushort b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp ushort <- ushort %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		ushort r; int b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp ushort <- int %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		ushort r; uint b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp ushort <- uint %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		ushort r; long b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp ushort <- long %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		ushort r; ulong b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp ushort <- ulong %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		ushort r; vlong b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp ushort <- vlong %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		ushort r; uvlong b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp ushort <- uvlong %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		ushort r; float b; b = pf[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp ushort <- float %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		ushort r; double b; b = pf[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp ushort <- double %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		int r; char b; b = si[j];
		r = si[i]; r += b; h = mix(h, (vlong)r);
		r = si[i]; r -= b; h = mix(h, (vlong)r);
		r = si[i]; r *= b; h = mix(h, (vlong)r);
		r = si[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp int <- char %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		int r; uchar b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp int <- uchar %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		int r; short b; b = si[j];
		r = si[i]; r += b; h = mix(h, (vlong)r);
		r = si[i]; r -= b; h = mix(h, (vlong)r);
		r = si[i]; r *= b; h = mix(h, (vlong)r);
		r = si[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp int <- short %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		int r; ushort b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp int <- ushort %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		int r; int b; b = si[j];
		r = si[i]; r += b; h = mix(h, (vlong)r);
		r = si[i]; r -= b; h = mix(h, (vlong)r);
		r = si[i]; r *= b; h = mix(h, (vlong)r);
		r = si[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp int <- int %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		int r; uint b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp int <- uint %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		int r; long b; b = si[j];
		r = si[i]; r += b; h = mix(h, (vlong)r);
		r = si[i]; r -= b; h = mix(h, (vlong)r);
		r = si[i]; r *= b; h = mix(h, (vlong)r);
		r = si[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp int <- long %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		int r; ulong b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp int <- ulong %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		int r; vlong b; b = si[j];
		r = si[i]; r += b; h = mix(h, (vlong)r);
		r = si[i]; r -= b; h = mix(h, (vlong)r);
		r = si[i]; r *= b; h = mix(h, (vlong)r);
		r = si[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp int <- vlong %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		int r; uvlong b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp int <- uvlong %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		int r; float b; b = sf[j];
		r = si[i]; r += b; h = mix(h, (vlong)r);
		r = si[i]; r -= b; h = mix(h, (vlong)r);
		r = si[i]; r *= b; h = mix(h, (vlong)r);
		r = si[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp int <- float %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		int r; double b; b = sf[j];
		r = si[i]; r += b; h = mix(h, (vlong)r);
		r = si[i]; r -= b; h = mix(h, (vlong)r);
		r = si[i]; r *= b; h = mix(h, (vlong)r);
		r = si[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp int <- double %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		uint r; char b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp uint <- char %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		uint r; uchar b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp uint <- uchar %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		uint r; short b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp uint <- short %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		uint r; ushort b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp uint <- ushort %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		uint r; int b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp uint <- int %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		uint r; uint b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp uint <- uint %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		uint r; long b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp uint <- long %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		uint r; ulong b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp uint <- ulong %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		uint r; vlong b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp uint <- vlong %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		uint r; uvlong b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp uint <- uvlong %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		uint r; float b; b = pf[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp uint <- float %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		uint r; double b; b = pf[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp uint <- double %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		long r; char b; b = si[j];
		r = si[i]; r += b; h = mix(h, (vlong)r);
		r = si[i]; r -= b; h = mix(h, (vlong)r);
		r = si[i]; r *= b; h = mix(h, (vlong)r);
		r = si[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp long <- char %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		long r; uchar b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp long <- uchar %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		long r; short b; b = si[j];
		r = si[i]; r += b; h = mix(h, (vlong)r);
		r = si[i]; r -= b; h = mix(h, (vlong)r);
		r = si[i]; r *= b; h = mix(h, (vlong)r);
		r = si[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp long <- short %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		long r; ushort b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp long <- ushort %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		long r; int b; b = si[j];
		r = si[i]; r += b; h = mix(h, (vlong)r);
		r = si[i]; r -= b; h = mix(h, (vlong)r);
		r = si[i]; r *= b; h = mix(h, (vlong)r);
		r = si[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp long <- int %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		long r; uint b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp long <- uint %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		long r; long b; b = si[j];
		r = si[i]; r += b; h = mix(h, (vlong)r);
		r = si[i]; r -= b; h = mix(h, (vlong)r);
		r = si[i]; r *= b; h = mix(h, (vlong)r);
		r = si[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp long <- long %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		long r; ulong b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp long <- ulong %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		long r; vlong b; b = si[j];
		r = si[i]; r += b; h = mix(h, (vlong)r);
		r = si[i]; r -= b; h = mix(h, (vlong)r);
		r = si[i]; r *= b; h = mix(h, (vlong)r);
		r = si[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp long <- vlong %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		long r; uvlong b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp long <- uvlong %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		long r; float b; b = sf[j];
		r = si[i]; r += b; h = mix(h, (vlong)r);
		r = si[i]; r -= b; h = mix(h, (vlong)r);
		r = si[i]; r *= b; h = mix(h, (vlong)r);
		r = si[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp long <- float %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		long r; double b; b = sf[j];
		r = si[i]; r += b; h = mix(h, (vlong)r);
		r = si[i]; r -= b; h = mix(h, (vlong)r);
		r = si[i]; r *= b; h = mix(h, (vlong)r);
		r = si[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp long <- double %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		ulong r; char b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp ulong <- char %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		ulong r; uchar b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp ulong <- uchar %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		ulong r; short b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp ulong <- short %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		ulong r; ushort b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp ulong <- ushort %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		ulong r; int b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp ulong <- int %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		ulong r; uint b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp ulong <- uint %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		ulong r; long b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp ulong <- long %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		ulong r; ulong b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp ulong <- ulong %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		ulong r; vlong b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp ulong <- vlong %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		ulong r; uvlong b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp ulong <- uvlong %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		ulong r; float b; b = pf[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp ulong <- float %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		ulong r; double b; b = pf[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp ulong <- double %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		vlong r; char b; b = si[j];
		r = si[i]; r += b; h = mix(h, (vlong)r);
		r = si[i]; r -= b; h = mix(h, (vlong)r);
		r = si[i]; r *= b; h = mix(h, (vlong)r);
		r = si[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp vlong <- char %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		vlong r; uchar b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp vlong <- uchar %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		vlong r; short b; b = si[j];
		r = si[i]; r += b; h = mix(h, (vlong)r);
		r = si[i]; r -= b; h = mix(h, (vlong)r);
		r = si[i]; r *= b; h = mix(h, (vlong)r);
		r = si[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp vlong <- short %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		vlong r; ushort b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp vlong <- ushort %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		vlong r; int b; b = si[j];
		r = si[i]; r += b; h = mix(h, (vlong)r);
		r = si[i]; r -= b; h = mix(h, (vlong)r);
		r = si[i]; r *= b; h = mix(h, (vlong)r);
		r = si[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp vlong <- int %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		vlong r; uint b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp vlong <- uint %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		vlong r; long b; b = si[j];
		r = si[i]; r += b; h = mix(h, (vlong)r);
		r = si[i]; r -= b; h = mix(h, (vlong)r);
		r = si[i]; r *= b; h = mix(h, (vlong)r);
		r = si[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp vlong <- long %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		vlong r; ulong b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp vlong <- ulong %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		vlong r; vlong b; b = si[j];
		r = si[i]; r += b; h = mix(h, (vlong)r);
		r = si[i]; r -= b; h = mix(h, (vlong)r);
		r = si[i]; r *= b; h = mix(h, (vlong)r);
		r = si[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp vlong <- vlong %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		vlong r; uvlong b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp vlong <- uvlong %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		vlong r; float b; b = sf[j];
		r = si[i]; r += b; h = mix(h, (vlong)r);
		r = si[i]; r -= b; h = mix(h, (vlong)r);
		r = si[i]; r *= b; h = mix(h, (vlong)r);
		r = si[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp vlong <- float %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		vlong r; double b; b = sf[j];
		r = si[i]; r += b; h = mix(h, (vlong)r);
		r = si[i]; r -= b; h = mix(h, (vlong)r);
		r = si[i]; r *= b; h = mix(h, (vlong)r);
		r = si[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp vlong <- double %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		uvlong r; char b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp uvlong <- char %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		uvlong r; uchar b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp uvlong <- uchar %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		uvlong r; short b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp uvlong <- short %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		uvlong r; ushort b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp uvlong <- ushort %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		uvlong r; int b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp uvlong <- int %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		uvlong r; uint b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp uvlong <- uint %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		uvlong r; long b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp uvlong <- long %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		uvlong r; ulong b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp uvlong <- ulong %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		uvlong r; vlong b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp uvlong <- vlong %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		uvlong r; uvlong b; b = pi[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp uvlong <- uvlong %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		uvlong r; float b; b = pf[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp uvlong <- float %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		uvlong r; double b; b = pf[j];
		r = pi[i]; r += b; h = mix(h, (vlong)r);
		r = pi[i]; r -= b; h = mix(h, (vlong)r);
		r = pi[i]; r *= b; h = mix(h, (vlong)r);
		r = pi[i]; r /= b; h = mix(h, (vlong)r);
	}
	print("cmp uvlong <- double %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		float r; char b; b = si[j];
		r = sf[i]; r += b; h = hd(h, r);
		r = sf[i]; r -= b; h = hd(h, r);
		r = sf[i]; r *= b; h = hd(h, r);
		r = sf[i]; r /= b; h = hd(h, r);
	}
	print("cmp float <- char %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		float r; uchar b; b = pi[j];
		r = pf[i]; r += b; h = hd(h, r);
		r = pf[i]; r -= b; h = hd(h, r);
		r = pf[i]; r *= b; h = hd(h, r);
		r = pf[i]; r /= b; h = hd(h, r);
	}
	print("cmp float <- uchar %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		float r; short b; b = si[j];
		r = sf[i]; r += b; h = hd(h, r);
		r = sf[i]; r -= b; h = hd(h, r);
		r = sf[i]; r *= b; h = hd(h, r);
		r = sf[i]; r /= b; h = hd(h, r);
	}
	print("cmp float <- short %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		float r; ushort b; b = pi[j];
		r = pf[i]; r += b; h = hd(h, r);
		r = pf[i]; r -= b; h = hd(h, r);
		r = pf[i]; r *= b; h = hd(h, r);
		r = pf[i]; r /= b; h = hd(h, r);
	}
	print("cmp float <- ushort %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		float r; int b; b = si[j];
		r = sf[i]; r += b; h = hd(h, r);
		r = sf[i]; r -= b; h = hd(h, r);
		r = sf[i]; r *= b; h = hd(h, r);
		r = sf[i]; r /= b; h = hd(h, r);
	}
	print("cmp float <- int %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		float r; uint b; b = pi[j];
		r = pf[i]; r += b; h = hd(h, r);
		r = pf[i]; r -= b; h = hd(h, r);
		r = pf[i]; r *= b; h = hd(h, r);
		r = pf[i]; r /= b; h = hd(h, r);
	}
	print("cmp float <- uint %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		float r; long b; b = si[j];
		r = sf[i]; r += b; h = hd(h, r);
		r = sf[i]; r -= b; h = hd(h, r);
		r = sf[i]; r *= b; h = hd(h, r);
		r = sf[i]; r /= b; h = hd(h, r);
	}
	print("cmp float <- long %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		float r; ulong b; b = pi[j];
		r = pf[i]; r += b; h = hd(h, r);
		r = pf[i]; r -= b; h = hd(h, r);
		r = pf[i]; r *= b; h = hd(h, r);
		r = pf[i]; r /= b; h = hd(h, r);
	}
	print("cmp float <- ulong %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		float r; vlong b; b = si[j];
		r = sf[i]; r += b; h = hd(h, r);
		r = sf[i]; r -= b; h = hd(h, r);
		r = sf[i]; r *= b; h = hd(h, r);
		r = sf[i]; r /= b; h = hd(h, r);
	}
	print("cmp float <- vlong %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		float r; uvlong b; b = pi[j];
		r = pf[i]; r += b; h = hd(h, r);
		r = pf[i]; r -= b; h = hd(h, r);
		r = pf[i]; r *= b; h = hd(h, r);
		r = pf[i]; r /= b; h = hd(h, r);
	}
	print("cmp float <- uvlong %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		float r; float b; b = sf[j];
		r = sf[i]; r += b; h = hd(h, r);
		r = sf[i]; r -= b; h = hd(h, r);
		r = sf[i]; r *= b; h = hd(h, r);
		r = sf[i]; r /= b; h = hd(h, r);
	}
	print("cmp float <- float %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		float r; double b; b = sf[j];
		r = sf[i]; r += b; h = hd(h, r);
		r = sf[i]; r -= b; h = hd(h, r);
		r = sf[i]; r *= b; h = hd(h, r);
		r = sf[i]; r /= b; h = hd(h, r);
	}
	print("cmp float <- double %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		double r; char b; b = si[j];
		r = sf[i]; r += b; h = hd(h, r);
		r = sf[i]; r -= b; h = hd(h, r);
		r = sf[i]; r *= b; h = hd(h, r);
		r = sf[i]; r /= b; h = hd(h, r);
	}
	print("cmp double <- char %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		double r; uchar b; b = pi[j];
		r = pf[i]; r += b; h = hd(h, r);
		r = pf[i]; r -= b; h = hd(h, r);
		r = pf[i]; r *= b; h = hd(h, r);
		r = pf[i]; r /= b; h = hd(h, r);
	}
	print("cmp double <- uchar %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		double r; short b; b = si[j];
		r = sf[i]; r += b; h = hd(h, r);
		r = sf[i]; r -= b; h = hd(h, r);
		r = sf[i]; r *= b; h = hd(h, r);
		r = sf[i]; r /= b; h = hd(h, r);
	}
	print("cmp double <- short %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		double r; ushort b; b = pi[j];
		r = pf[i]; r += b; h = hd(h, r);
		r = pf[i]; r -= b; h = hd(h, r);
		r = pf[i]; r *= b; h = hd(h, r);
		r = pf[i]; r /= b; h = hd(h, r);
	}
	print("cmp double <- ushort %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		double r; int b; b = si[j];
		r = sf[i]; r += b; h = hd(h, r);
		r = sf[i]; r -= b; h = hd(h, r);
		r = sf[i]; r *= b; h = hd(h, r);
		r = sf[i]; r /= b; h = hd(h, r);
	}
	print("cmp double <- int %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		double r; uint b; b = pi[j];
		r = pf[i]; r += b; h = hd(h, r);
		r = pf[i]; r -= b; h = hd(h, r);
		r = pf[i]; r *= b; h = hd(h, r);
		r = pf[i]; r /= b; h = hd(h, r);
	}
	print("cmp double <- uint %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		double r; long b; b = si[j];
		r = sf[i]; r += b; h = hd(h, r);
		r = sf[i]; r -= b; h = hd(h, r);
		r = sf[i]; r *= b; h = hd(h, r);
		r = sf[i]; r /= b; h = hd(h, r);
	}
	print("cmp double <- long %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		double r; ulong b; b = pi[j];
		r = pf[i]; r += b; h = hd(h, r);
		r = pf[i]; r -= b; h = hd(h, r);
		r = pf[i]; r *= b; h = hd(h, r);
		r = pf[i]; r /= b; h = hd(h, r);
	}
	print("cmp double <- ulong %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		double r; vlong b; b = si[j];
		r = sf[i]; r += b; h = hd(h, r);
		r = sf[i]; r -= b; h = hd(h, r);
		r = sf[i]; r *= b; h = hd(h, r);
		r = sf[i]; r /= b; h = hd(h, r);
	}
	print("cmp double <- vlong %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		double r; uvlong b; b = pi[j];
		r = pf[i]; r += b; h = hd(h, r);
		r = pf[i]; r -= b; h = hd(h, r);
		r = pf[i]; r *= b; h = hd(h, r);
		r = pf[i]; r /= b; h = hd(h, r);
	}
	print("cmp double <- uvlong %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		double r; float b; b = sf[j];
		r = sf[i]; r += b; h = hd(h, r);
		r = sf[i]; r -= b; h = hd(h, r);
		r = sf[i]; r *= b; h = hd(h, r);
		r = sf[i]; r /= b; h = hd(h, r);
	}
	print("cmp double <- float %llux\n", h);
	h = H0;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++){
		double r; double b; b = sf[j];
		r = sf[i]; r += b; h = hd(h, r);
		r = sf[i]; r -= b; h = hd(h, r);
		r = sf[i]; r *= b; h = hd(h, r);
		r = sf[i]; r /= b; h = hd(h, r);
	}
	print("cmp double <- double %llux\n", h);
}


static double
argsum(char a, uchar b, short c, ushort d, int e, uint f, vlong g, uvlong h,
	float i, double j, float k, double l, int m, double n)
{
	return a*1.5 + b - c + d*0.25 + e - f + g*0.5 - h*0.125 + i*2.0 + j
		- k + l*3.0 + m - n;
}

static float fneg(float x) { return -x; }
static float f3(float a, float b, float c) { return a*b + c; }
static double d3(double a, float b, int c) { return a*b + c; }
static float (*fnp[])(float, float, float) = { f3 };

static double
vsumd(int n, ...)
{
	va_list ap;
	double s;
	int i;

	s = 0;
	va_start(ap, n);
	for(i = 0; i < n; i++)
		s += va_arg(ap, double);
	va_end(ap);
	return s;
}

static vlong
vmix(int n, ...)
{
	va_list ap;
	vlong s;
	double x;
	int i;

	s = 0;
	va_start(ap, n);
	for(i = 0; i < n; i++){
		s += va_arg(ap, int);
		x = va_arg(ap, double);
		if(x > -1e15 && x < 1e15)
			s ^= (vlong)x;
		s += va_arg(ap, vlong);
	}
	va_end(ap);
	return s;
}

static void
t_calls(void)
{
	int i, j, k;
	uvlong h;

	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NDV; j++){
		k = (i + j) % NV;
		h = hd(h, argsum(gv(i), gv(j % NV), gv(k), gv((i+3) % NV),
			gv((j+5) % NV), gv((i+7) % NV), gv(k), gv((i+9) % NV),
			gf(j), gd((i+1) % NDV), gf((j+2) % NDV), gd(k % NDV),
			gv((i+4) % NV), gd((j+6) % NDV)));
	}
	print("calls argsum %llux\n", h);

	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NDV; j++){
		k = (i + j) % NDV;
		h = hd(h, fneg(gf(i)));
		h = hd(h, f3(gf(i), gf(j), gf(k)));
		h = hd(h, fnp[0](gf(i), gf(j), gf(k)));
		h = hd(h, d3(gd(i), gf(j), gv(k % NV)));
	}
	print("calls float %llux\n", h);

	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NDV; j++){
		k = (i + j) % NDV;
		h = hd(h, vsumd(3, gd(i), (double)gf(j), gd(k)));
		h = mix(h, vmix(2, (int)gv(i % NV), gd(j), gv(k % NV),
			(int)gv(j % NV), gd(i), gv((k+1) % NV)));
	}
	print("calls varargs %llux\n", h);
}

typedef struct S S;
struct S {
	char c;
	short s;
	int i;
	vlong v;
	float f;
	double d;
	uchar u[3];
};

static S
mkS(int k)
{
	S s;

	memset(&s, 0, sizeof s);
	s.c = gv(k % NV);
	s.s = gv((k+1) % NV);
	s.i = gv((k+2) % NV);
	s.v = gv((k+3) % NV);
	s.f = gf((k+4) % NDV);
	s.d = gd((k+5) % NDV);
	s.u[0] = gv((k+6) % NV);
	s.u[1] = gv((k+7) % NV);
	s.u[2] = gv((k+8) % NV);
	return s;
}

static S
bump(S s, int k)
{
	s.c += k;
	s.s -= k;
	s.i *= 3;
	s.v ^= k;
	s.f += 0.5;
	s.d *= 1.5;
	s.u[1] += k;
	return s;
}

static uvlong
hashS(uvlong h, S s)
{
	h = mix(h, (vlong)s.c);
	h = mix(h, (vlong)s.s);
	h = mix(h, (vlong)s.i);
	h = mix(h, (uvlong)s.v);
	h = hd(h, s.f);
	h = hd(h, s.d);
	h = mix(h, s.u[0]);
	h = mix(h, s.u[1]);
	return mix(h, s.u[2]);
}

static void
t_struct(void)
{
	int k;
	uvlong h;
	S a, b, arr[3];

	h = H0;
	for(k = 0; k < NV*2; k++){
		a = mkS(k);
		b = bump(a, k);
		arr[0] = a;
		arr[1] = b;
		arr[2] = bump(arr[1], k+1);
		h = hashS(h, a);
		h = hashS(h, b);
		h = hashS(h, arr[2]);
		a = arr[2];
		a.i++;
		a.c++;
		h = hashS(h, a);
	}
	print("struct byvalue %llux\n", h);
}

void
main(void)
{
	/* amd64 Plan 9 traps on invalid/zdiv/overflow; init() already makes NaN/inf,
	 * so the masks must be set before it.  RISC-V never traps: a no-op there. */
	setfcr(getfcr() & ~(FPINVAL|FPZDIV|FPOVFL|FPUNFL|FPINEX));
	init();
	teststart();
	print("sizes f%d d%d S%d\n", (int)sizeof(float), (int)sizeof(double),
		(int)sizeof(S));
	t_float();
	t_double();
	t_compound();
	t_calls();
	t_struct();
	exits(nil);
}
