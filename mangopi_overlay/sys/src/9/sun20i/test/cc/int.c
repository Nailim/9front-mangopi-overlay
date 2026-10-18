#include "../test.h"

static vlong vals[] = {0, 1, -1, 2, -2, 127, 128, 255, 256, -128, -129, 32767, 32768, 65535, 65536, -32768, -32769, 0x7fffffffLL, 0x80000000LL, 0xffffffffLL, 0x100000000LL, -0x80000000LL, -0x80000001LL, 0x7fffffffffffffffLL, (-0x7fffffffffffffffLL-1)};
#define NV 25
#define H0 0xcbf29ce484222325ULL

/* through a call, so the shared front end cannot constant-fold */
static vlong get(int i) { return vals[i]; }
static uvlong mix(uvlong h, uvlong x) { return (h ^ x) * 0x100000001b3ULL; }

static void
t_char(void)
{
	int i, j, k, c;
	char a, b, r;
	uvlong h, bits;
	double d;
	static int sh[] = {0, 1, 7};

	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a + b; h = mix(h, r);
	}
	print("char + %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a - b; h = mix(h, r);
	}
	print("char - %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a * b; h = mix(h, r);
	}
	print("char * %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a & b; h = mix(h, r);
	}
	print("char & %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a | b; h = mix(h, r);
	}
	print("char | %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a ^ b; h = mix(h, r);
	}
	print("char ^ %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		if(b == 0) continue;
		if(b == -1) continue;	/* MIN/-1 traps on amd64 */
		r = a / b; h = mix(h, r);
	}
	print("char / %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		if(b == 0) continue;
		if(b == -1) continue;	/* MIN/-1 traps on amd64 */
		r = a % b; h = mix(h, r);
	}
	print("char % %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r += b; h = mix(h, r);
	}
	print("char += %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r -= b; h = mix(h, r);
	}
	print("char -= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r *= b; h = mix(h, r);
	}
	print("char *= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r &= b; h = mix(h, r);
	}
	print("char &= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r |= b; h = mix(h, r);
	}
	print("char |= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r ^= b; h = mix(h, r);
	}
	print("char ^= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		r = get(i); r++; h = mix(h, r); r--; r--; h = mix(h, r);
	}
	print("char ++-- %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a == b; h = mix(h, c);
		if(a == b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("char == %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a != b; h = mix(h, c);
		if(a != b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("char != %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a < b; h = mix(h, c);
		if(a < b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("char < %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a <= b; h = mix(h, c);
		if(a <= b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("char <= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a > b; h = mix(h, c);
		if(a > b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("char > %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a >= b; h = mix(h, c);
		if(a >= b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("char >= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(k = 0; k < nelem(sh); k++){
		a = get(i); r = a << sh[k]; h = mix(h, r);
	}
	print("char << %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(k = 0; k < nelem(sh); k++){
		a = get(i); r = a >> sh[k]; h = mix(h, r);
	}
	print("char >> %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		char x, y[1];
		a = get(i);
		x = (char)a; h = mix(h, x);
		y[0] = (char)a;
		if(y[0] != (char)a) h = mix(h, 0xdead);
		if((char)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("char->char %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		uchar x, y[1];
		a = get(i);
		x = (uchar)a; h = mix(h, x);
		y[0] = (uchar)a;
		if(y[0] != (uchar)a) h = mix(h, 0xdead);
		if((uchar)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("char->uchar %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		short x, y[1];
		a = get(i);
		x = (short)a; h = mix(h, x);
		y[0] = (short)a;
		if(y[0] != (short)a) h = mix(h, 0xdead);
		if((short)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("char->short %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		ushort x, y[1];
		a = get(i);
		x = (ushort)a; h = mix(h, x);
		y[0] = (ushort)a;
		if(y[0] != (ushort)a) h = mix(h, 0xdead);
		if((ushort)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("char->ushort %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		int x, y[1];
		a = get(i);
		x = (int)a; h = mix(h, x);
		y[0] = (int)a;
		if(y[0] != (int)a) h = mix(h, 0xdead);
		if((int)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("char->int %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		uint x, y[1];
		a = get(i);
		x = (uint)a; h = mix(h, x);
		y[0] = (uint)a;
		if(y[0] != (uint)a) h = mix(h, 0xdead);
		if((uint)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("char->uint %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		long x, y[1];
		a = get(i);
		x = (long)a; h = mix(h, x);
		y[0] = (long)a;
		if(y[0] != (long)a) h = mix(h, 0xdead);
		if((long)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("char->long %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		ulong x, y[1];
		a = get(i);
		x = (ulong)a; h = mix(h, x);
		y[0] = (ulong)a;
		if(y[0] != (ulong)a) h = mix(h, 0xdead);
		if((ulong)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("char->ulong %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		vlong x, y[1];
		a = get(i);
		x = (vlong)a; h = mix(h, x);
		y[0] = (vlong)a;
		if(y[0] != (vlong)a) h = mix(h, 0xdead);
		if((vlong)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("char->vlong %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		uvlong x, y[1];
		a = get(i);
		x = (uvlong)a; h = mix(h, x);
		y[0] = (uvlong)a;
		if(y[0] != (uvlong)a) h = mix(h, 0xdead);
		if((uvlong)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("char->uvlong %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		a = get(i); d = a;
		memmove(&bits, &d, sizeof bits); h = mix(h, bits);
		if(d > -9.0e18 && d < 9.0e18){ r = d; h = mix(h, r); }
	}
	print("char double %llux\n", h);
}

static void
t_uchar(void)
{
	int i, j, k, c;
	uchar a, b, r;
	uvlong h, bits;
	double d;
	static int sh[] = {0, 1, 7};

	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a + b; h = mix(h, r);
	}
	print("uchar + %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a - b; h = mix(h, r);
	}
	print("uchar - %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a * b; h = mix(h, r);
	}
	print("uchar * %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a & b; h = mix(h, r);
	}
	print("uchar & %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a | b; h = mix(h, r);
	}
	print("uchar | %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a ^ b; h = mix(h, r);
	}
	print("uchar ^ %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		if(b == 0) continue;
		r = a / b; h = mix(h, r);
	}
	print("uchar / %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		if(b == 0) continue;
		r = a % b; h = mix(h, r);
	}
	print("uchar % %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r += b; h = mix(h, r);
	}
	print("uchar += %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r -= b; h = mix(h, r);
	}
	print("uchar -= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r *= b; h = mix(h, r);
	}
	print("uchar *= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r &= b; h = mix(h, r);
	}
	print("uchar &= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r |= b; h = mix(h, r);
	}
	print("uchar |= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r ^= b; h = mix(h, r);
	}
	print("uchar ^= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		r = get(i); r++; h = mix(h, r); r--; r--; h = mix(h, r);
	}
	print("uchar ++-- %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a == b; h = mix(h, c);
		if(a == b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uchar == %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a != b; h = mix(h, c);
		if(a != b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uchar != %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a < b; h = mix(h, c);
		if(a < b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uchar < %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a <= b; h = mix(h, c);
		if(a <= b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uchar <= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a > b; h = mix(h, c);
		if(a > b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uchar > %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a >= b; h = mix(h, c);
		if(a >= b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uchar >= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(k = 0; k < nelem(sh); k++){
		a = get(i); r = a << sh[k]; h = mix(h, r);
	}
	print("uchar << %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(k = 0; k < nelem(sh); k++){
		a = get(i); r = a >> sh[k]; h = mix(h, r);
	}
	print("uchar >> %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		char x, y[1];
		a = get(i);
		x = (char)a; h = mix(h, x);
		y[0] = (char)a;
		if(y[0] != (char)a) h = mix(h, 0xdead);
		if((char)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uchar->char %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		uchar x, y[1];
		a = get(i);
		x = (uchar)a; h = mix(h, x);
		y[0] = (uchar)a;
		if(y[0] != (uchar)a) h = mix(h, 0xdead);
		if((uchar)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uchar->uchar %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		short x, y[1];
		a = get(i);
		x = (short)a; h = mix(h, x);
		y[0] = (short)a;
		if(y[0] != (short)a) h = mix(h, 0xdead);
		if((short)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uchar->short %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		ushort x, y[1];
		a = get(i);
		x = (ushort)a; h = mix(h, x);
		y[0] = (ushort)a;
		if(y[0] != (ushort)a) h = mix(h, 0xdead);
		if((ushort)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uchar->ushort %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		int x, y[1];
		a = get(i);
		x = (int)a; h = mix(h, x);
		y[0] = (int)a;
		if(y[0] != (int)a) h = mix(h, 0xdead);
		if((int)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uchar->int %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		uint x, y[1];
		a = get(i);
		x = (uint)a; h = mix(h, x);
		y[0] = (uint)a;
		if(y[0] != (uint)a) h = mix(h, 0xdead);
		if((uint)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uchar->uint %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		long x, y[1];
		a = get(i);
		x = (long)a; h = mix(h, x);
		y[0] = (long)a;
		if(y[0] != (long)a) h = mix(h, 0xdead);
		if((long)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uchar->long %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		ulong x, y[1];
		a = get(i);
		x = (ulong)a; h = mix(h, x);
		y[0] = (ulong)a;
		if(y[0] != (ulong)a) h = mix(h, 0xdead);
		if((ulong)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uchar->ulong %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		vlong x, y[1];
		a = get(i);
		x = (vlong)a; h = mix(h, x);
		y[0] = (vlong)a;
		if(y[0] != (vlong)a) h = mix(h, 0xdead);
		if((vlong)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uchar->vlong %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		uvlong x, y[1];
		a = get(i);
		x = (uvlong)a; h = mix(h, x);
		y[0] = (uvlong)a;
		if(y[0] != (uvlong)a) h = mix(h, 0xdead);
		if((uvlong)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uchar->uvlong %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		a = get(i); d = a;
		memmove(&bits, &d, sizeof bits); h = mix(h, bits);
		if(d > -9.0e18 && d < 9.0e18){ r = d; h = mix(h, r); }
	}
	print("uchar double %llux\n", h);
}

static void
t_short(void)
{
	int i, j, k, c;
	short a, b, r;
	uvlong h, bits;
	double d;
	static int sh[] = {0, 1, 7, 8, 15};

	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a + b; h = mix(h, r);
	}
	print("short + %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a - b; h = mix(h, r);
	}
	print("short - %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a * b; h = mix(h, r);
	}
	print("short * %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a & b; h = mix(h, r);
	}
	print("short & %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a | b; h = mix(h, r);
	}
	print("short | %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a ^ b; h = mix(h, r);
	}
	print("short ^ %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		if(b == 0) continue;
		if(b == -1) continue;	/* MIN/-1 traps on amd64 */
		r = a / b; h = mix(h, r);
	}
	print("short / %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		if(b == 0) continue;
		if(b == -1) continue;	/* MIN/-1 traps on amd64 */
		r = a % b; h = mix(h, r);
	}
	print("short % %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r += b; h = mix(h, r);
	}
	print("short += %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r -= b; h = mix(h, r);
	}
	print("short -= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r *= b; h = mix(h, r);
	}
	print("short *= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r &= b; h = mix(h, r);
	}
	print("short &= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r |= b; h = mix(h, r);
	}
	print("short |= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r ^= b; h = mix(h, r);
	}
	print("short ^= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		r = get(i); r++; h = mix(h, r); r--; r--; h = mix(h, r);
	}
	print("short ++-- %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a == b; h = mix(h, c);
		if(a == b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("short == %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a != b; h = mix(h, c);
		if(a != b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("short != %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a < b; h = mix(h, c);
		if(a < b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("short < %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a <= b; h = mix(h, c);
		if(a <= b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("short <= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a > b; h = mix(h, c);
		if(a > b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("short > %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a >= b; h = mix(h, c);
		if(a >= b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("short >= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(k = 0; k < nelem(sh); k++){
		a = get(i); r = a << sh[k]; h = mix(h, r);
	}
	print("short << %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(k = 0; k < nelem(sh); k++){
		a = get(i); r = a >> sh[k]; h = mix(h, r);
	}
	print("short >> %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		char x, y[1];
		a = get(i);
		x = (char)a; h = mix(h, x);
		y[0] = (char)a;
		if(y[0] != (char)a) h = mix(h, 0xdead);
		if((char)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("short->char %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		uchar x, y[1];
		a = get(i);
		x = (uchar)a; h = mix(h, x);
		y[0] = (uchar)a;
		if(y[0] != (uchar)a) h = mix(h, 0xdead);
		if((uchar)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("short->uchar %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		short x, y[1];
		a = get(i);
		x = (short)a; h = mix(h, x);
		y[0] = (short)a;
		if(y[0] != (short)a) h = mix(h, 0xdead);
		if((short)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("short->short %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		ushort x, y[1];
		a = get(i);
		x = (ushort)a; h = mix(h, x);
		y[0] = (ushort)a;
		if(y[0] != (ushort)a) h = mix(h, 0xdead);
		if((ushort)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("short->ushort %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		int x, y[1];
		a = get(i);
		x = (int)a; h = mix(h, x);
		y[0] = (int)a;
		if(y[0] != (int)a) h = mix(h, 0xdead);
		if((int)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("short->int %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		uint x, y[1];
		a = get(i);
		x = (uint)a; h = mix(h, x);
		y[0] = (uint)a;
		if(y[0] != (uint)a) h = mix(h, 0xdead);
		if((uint)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("short->uint %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		long x, y[1];
		a = get(i);
		x = (long)a; h = mix(h, x);
		y[0] = (long)a;
		if(y[0] != (long)a) h = mix(h, 0xdead);
		if((long)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("short->long %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		ulong x, y[1];
		a = get(i);
		x = (ulong)a; h = mix(h, x);
		y[0] = (ulong)a;
		if(y[0] != (ulong)a) h = mix(h, 0xdead);
		if((ulong)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("short->ulong %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		vlong x, y[1];
		a = get(i);
		x = (vlong)a; h = mix(h, x);
		y[0] = (vlong)a;
		if(y[0] != (vlong)a) h = mix(h, 0xdead);
		if((vlong)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("short->vlong %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		uvlong x, y[1];
		a = get(i);
		x = (uvlong)a; h = mix(h, x);
		y[0] = (uvlong)a;
		if(y[0] != (uvlong)a) h = mix(h, 0xdead);
		if((uvlong)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("short->uvlong %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		a = get(i); d = a;
		memmove(&bits, &d, sizeof bits); h = mix(h, bits);
		if(d > -9.0e18 && d < 9.0e18){ r = d; h = mix(h, r); }
	}
	print("short double %llux\n", h);
}

static void
t_ushort(void)
{
	int i, j, k, c;
	ushort a, b, r;
	uvlong h, bits;
	double d;
	static int sh[] = {0, 1, 7, 8, 15};

	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a + b; h = mix(h, r);
	}
	print("ushort + %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a - b; h = mix(h, r);
	}
	print("ushort - %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a * b; h = mix(h, r);
	}
	print("ushort * %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a & b; h = mix(h, r);
	}
	print("ushort & %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a | b; h = mix(h, r);
	}
	print("ushort | %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a ^ b; h = mix(h, r);
	}
	print("ushort ^ %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		if(b == 0) continue;
		r = a / b; h = mix(h, r);
	}
	print("ushort / %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		if(b == 0) continue;
		r = a % b; h = mix(h, r);
	}
	print("ushort % %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r += b; h = mix(h, r);
	}
	print("ushort += %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r -= b; h = mix(h, r);
	}
	print("ushort -= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r *= b; h = mix(h, r);
	}
	print("ushort *= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r &= b; h = mix(h, r);
	}
	print("ushort &= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r |= b; h = mix(h, r);
	}
	print("ushort |= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r ^= b; h = mix(h, r);
	}
	print("ushort ^= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		r = get(i); r++; h = mix(h, r); r--; r--; h = mix(h, r);
	}
	print("ushort ++-- %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a == b; h = mix(h, c);
		if(a == b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("ushort == %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a != b; h = mix(h, c);
		if(a != b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("ushort != %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a < b; h = mix(h, c);
		if(a < b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("ushort < %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a <= b; h = mix(h, c);
		if(a <= b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("ushort <= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a > b; h = mix(h, c);
		if(a > b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("ushort > %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a >= b; h = mix(h, c);
		if(a >= b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("ushort >= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(k = 0; k < nelem(sh); k++){
		a = get(i); r = a << sh[k]; h = mix(h, r);
	}
	print("ushort << %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(k = 0; k < nelem(sh); k++){
		a = get(i); r = a >> sh[k]; h = mix(h, r);
	}
	print("ushort >> %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		char x, y[1];
		a = get(i);
		x = (char)a; h = mix(h, x);
		y[0] = (char)a;
		if(y[0] != (char)a) h = mix(h, 0xdead);
		if((char)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("ushort->char %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		uchar x, y[1];
		a = get(i);
		x = (uchar)a; h = mix(h, x);
		y[0] = (uchar)a;
		if(y[0] != (uchar)a) h = mix(h, 0xdead);
		if((uchar)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("ushort->uchar %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		short x, y[1];
		a = get(i);
		x = (short)a; h = mix(h, x);
		y[0] = (short)a;
		if(y[0] != (short)a) h = mix(h, 0xdead);
		if((short)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("ushort->short %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		ushort x, y[1];
		a = get(i);
		x = (ushort)a; h = mix(h, x);
		y[0] = (ushort)a;
		if(y[0] != (ushort)a) h = mix(h, 0xdead);
		if((ushort)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("ushort->ushort %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		int x, y[1];
		a = get(i);
		x = (int)a; h = mix(h, x);
		y[0] = (int)a;
		if(y[0] != (int)a) h = mix(h, 0xdead);
		if((int)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("ushort->int %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		uint x, y[1];
		a = get(i);
		x = (uint)a; h = mix(h, x);
		y[0] = (uint)a;
		if(y[0] != (uint)a) h = mix(h, 0xdead);
		if((uint)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("ushort->uint %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		long x, y[1];
		a = get(i);
		x = (long)a; h = mix(h, x);
		y[0] = (long)a;
		if(y[0] != (long)a) h = mix(h, 0xdead);
		if((long)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("ushort->long %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		ulong x, y[1];
		a = get(i);
		x = (ulong)a; h = mix(h, x);
		y[0] = (ulong)a;
		if(y[0] != (ulong)a) h = mix(h, 0xdead);
		if((ulong)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("ushort->ulong %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		vlong x, y[1];
		a = get(i);
		x = (vlong)a; h = mix(h, x);
		y[0] = (vlong)a;
		if(y[0] != (vlong)a) h = mix(h, 0xdead);
		if((vlong)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("ushort->vlong %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		uvlong x, y[1];
		a = get(i);
		x = (uvlong)a; h = mix(h, x);
		y[0] = (uvlong)a;
		if(y[0] != (uvlong)a) h = mix(h, 0xdead);
		if((uvlong)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("ushort->uvlong %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		a = get(i); d = a;
		memmove(&bits, &d, sizeof bits); h = mix(h, bits);
		if(d > -9.0e18 && d < 9.0e18){ r = d; h = mix(h, r); }
	}
	print("ushort double %llux\n", h);
}

static void
t_int(void)
{
	int i, j, k, c;
	int a, b, r;
	uvlong h, bits;
	double d;
	static int sh[] = {0, 1, 7, 8, 15, 16, 31};

	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a + b; h = mix(h, r);
	}
	print("int + %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a - b; h = mix(h, r);
	}
	print("int - %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a * b; h = mix(h, r);
	}
	print("int * %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a & b; h = mix(h, r);
	}
	print("int & %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a | b; h = mix(h, r);
	}
	print("int | %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a ^ b; h = mix(h, r);
	}
	print("int ^ %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		if(b == 0) continue;
		if(b == -1) continue;	/* MIN/-1 traps on amd64 */
		r = a / b; h = mix(h, r);
	}
	print("int / %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		if(b == 0) continue;
		if(b == -1) continue;	/* MIN/-1 traps on amd64 */
		r = a % b; h = mix(h, r);
	}
	print("int % %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r += b; h = mix(h, r);
	}
	print("int += %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r -= b; h = mix(h, r);
	}
	print("int -= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r *= b; h = mix(h, r);
	}
	print("int *= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r &= b; h = mix(h, r);
	}
	print("int &= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r |= b; h = mix(h, r);
	}
	print("int |= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r ^= b; h = mix(h, r);
	}
	print("int ^= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		r = get(i); r++; h = mix(h, r); r--; r--; h = mix(h, r);
	}
	print("int ++-- %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a == b; h = mix(h, c);
		if(a == b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("int == %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a != b; h = mix(h, c);
		if(a != b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("int != %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a < b; h = mix(h, c);
		if(a < b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("int < %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a <= b; h = mix(h, c);
		if(a <= b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("int <= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a > b; h = mix(h, c);
		if(a > b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("int > %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a >= b; h = mix(h, c);
		if(a >= b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("int >= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(k = 0; k < nelem(sh); k++){
		a = get(i); r = a << sh[k]; h = mix(h, r);
	}
	print("int << %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(k = 0; k < nelem(sh); k++){
		a = get(i); r = a >> sh[k]; h = mix(h, r);
	}
	print("int >> %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		char x, y[1];
		a = get(i);
		x = (char)a; h = mix(h, x);
		y[0] = (char)a;
		if(y[0] != (char)a) h = mix(h, 0xdead);
		if((char)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("int->char %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		uchar x, y[1];
		a = get(i);
		x = (uchar)a; h = mix(h, x);
		y[0] = (uchar)a;
		if(y[0] != (uchar)a) h = mix(h, 0xdead);
		if((uchar)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("int->uchar %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		short x, y[1];
		a = get(i);
		x = (short)a; h = mix(h, x);
		y[0] = (short)a;
		if(y[0] != (short)a) h = mix(h, 0xdead);
		if((short)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("int->short %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		ushort x, y[1];
		a = get(i);
		x = (ushort)a; h = mix(h, x);
		y[0] = (ushort)a;
		if(y[0] != (ushort)a) h = mix(h, 0xdead);
		if((ushort)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("int->ushort %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		int x, y[1];
		a = get(i);
		x = (int)a; h = mix(h, x);
		y[0] = (int)a;
		if(y[0] != (int)a) h = mix(h, 0xdead);
		if((int)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("int->int %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		uint x, y[1];
		a = get(i);
		x = (uint)a; h = mix(h, x);
		y[0] = (uint)a;
		if(y[0] != (uint)a) h = mix(h, 0xdead);
		if((uint)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("int->uint %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		long x, y[1];
		a = get(i);
		x = (long)a; h = mix(h, x);
		y[0] = (long)a;
		if(y[0] != (long)a) h = mix(h, 0xdead);
		if((long)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("int->long %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		ulong x, y[1];
		a = get(i);
		x = (ulong)a; h = mix(h, x);
		y[0] = (ulong)a;
		if(y[0] != (ulong)a) h = mix(h, 0xdead);
		if((ulong)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("int->ulong %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		vlong x, y[1];
		a = get(i);
		x = (vlong)a; h = mix(h, x);
		y[0] = (vlong)a;
		if(y[0] != (vlong)a) h = mix(h, 0xdead);
		if((vlong)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("int->vlong %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		uvlong x, y[1];
		a = get(i);
		x = (uvlong)a; h = mix(h, x);
		y[0] = (uvlong)a;
		if(y[0] != (uvlong)a) h = mix(h, 0xdead);
		if((uvlong)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("int->uvlong %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		a = get(i); d = a;
		memmove(&bits, &d, sizeof bits); h = mix(h, bits);
		if(d > -9.0e18 && d < 9.0e18){ r = d; h = mix(h, r); }
	}
	print("int double %llux\n", h);
}

static void
t_uint(void)
{
	int i, j, k, c;
	uint a, b, r;
	uvlong h, bits;
	double d;
	static int sh[] = {0, 1, 7, 8, 15, 16, 31};

	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a + b; h = mix(h, r);
	}
	print("uint + %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a - b; h = mix(h, r);
	}
	print("uint - %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a * b; h = mix(h, r);
	}
	print("uint * %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a & b; h = mix(h, r);
	}
	print("uint & %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a | b; h = mix(h, r);
	}
	print("uint | %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a ^ b; h = mix(h, r);
	}
	print("uint ^ %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		if(b == 0) continue;
		r = a / b; h = mix(h, r);
	}
	print("uint / %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		if(b == 0) continue;
		r = a % b; h = mix(h, r);
	}
	print("uint % %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r += b; h = mix(h, r);
	}
	print("uint += %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r -= b; h = mix(h, r);
	}
	print("uint -= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r *= b; h = mix(h, r);
	}
	print("uint *= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r &= b; h = mix(h, r);
	}
	print("uint &= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r |= b; h = mix(h, r);
	}
	print("uint |= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r ^= b; h = mix(h, r);
	}
	print("uint ^= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		r = get(i); r++; h = mix(h, r); r--; r--; h = mix(h, r);
	}
	print("uint ++-- %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a == b; h = mix(h, c);
		if(a == b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uint == %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a != b; h = mix(h, c);
		if(a != b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uint != %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a < b; h = mix(h, c);
		if(a < b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uint < %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a <= b; h = mix(h, c);
		if(a <= b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uint <= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a > b; h = mix(h, c);
		if(a > b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uint > %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a >= b; h = mix(h, c);
		if(a >= b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uint >= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(k = 0; k < nelem(sh); k++){
		a = get(i); r = a << sh[k]; h = mix(h, r);
	}
	print("uint << %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(k = 0; k < nelem(sh); k++){
		a = get(i); r = a >> sh[k]; h = mix(h, r);
	}
	print("uint >> %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		char x, y[1];
		a = get(i);
		x = (char)a; h = mix(h, x);
		y[0] = (char)a;
		if(y[0] != (char)a) h = mix(h, 0xdead);
		if((char)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uint->char %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		uchar x, y[1];
		a = get(i);
		x = (uchar)a; h = mix(h, x);
		y[0] = (uchar)a;
		if(y[0] != (uchar)a) h = mix(h, 0xdead);
		if((uchar)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uint->uchar %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		short x, y[1];
		a = get(i);
		x = (short)a; h = mix(h, x);
		y[0] = (short)a;
		if(y[0] != (short)a) h = mix(h, 0xdead);
		if((short)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uint->short %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		ushort x, y[1];
		a = get(i);
		x = (ushort)a; h = mix(h, x);
		y[0] = (ushort)a;
		if(y[0] != (ushort)a) h = mix(h, 0xdead);
		if((ushort)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uint->ushort %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		int x, y[1];
		a = get(i);
		x = (int)a; h = mix(h, x);
		y[0] = (int)a;
		if(y[0] != (int)a) h = mix(h, 0xdead);
		if((int)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uint->int %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		uint x, y[1];
		a = get(i);
		x = (uint)a; h = mix(h, x);
		y[0] = (uint)a;
		if(y[0] != (uint)a) h = mix(h, 0xdead);
		if((uint)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uint->uint %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		long x, y[1];
		a = get(i);
		x = (long)a; h = mix(h, x);
		y[0] = (long)a;
		if(y[0] != (long)a) h = mix(h, 0xdead);
		if((long)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uint->long %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		ulong x, y[1];
		a = get(i);
		x = (ulong)a; h = mix(h, x);
		y[0] = (ulong)a;
		if(y[0] != (ulong)a) h = mix(h, 0xdead);
		if((ulong)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uint->ulong %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		vlong x, y[1];
		a = get(i);
		x = (vlong)a; h = mix(h, x);
		y[0] = (vlong)a;
		if(y[0] != (vlong)a) h = mix(h, 0xdead);
		if((vlong)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uint->vlong %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		uvlong x, y[1];
		a = get(i);
		x = (uvlong)a; h = mix(h, x);
		y[0] = (uvlong)a;
		if(y[0] != (uvlong)a) h = mix(h, 0xdead);
		if((uvlong)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uint->uvlong %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		a = get(i); d = a;
		memmove(&bits, &d, sizeof bits); h = mix(h, bits);
		if(d > -9.0e18 && d < 9.0e18){ r = d; h = mix(h, r); }
	}
	print("uint double %llux\n", h);
}

static void
t_long(void)
{
	int i, j, k, c;
	long a, b, r;
	uvlong h, bits;
	double d;
	static int sh[] = {0, 1, 7, 8, 15, 16, 31};

	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a + b; h = mix(h, r);
	}
	print("long + %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a - b; h = mix(h, r);
	}
	print("long - %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a * b; h = mix(h, r);
	}
	print("long * %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a & b; h = mix(h, r);
	}
	print("long & %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a | b; h = mix(h, r);
	}
	print("long | %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a ^ b; h = mix(h, r);
	}
	print("long ^ %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		if(b == 0) continue;
		if(b == -1) continue;	/* MIN/-1 traps on amd64 */
		r = a / b; h = mix(h, r);
	}
	print("long / %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		if(b == 0) continue;
		if(b == -1) continue;	/* MIN/-1 traps on amd64 */
		r = a % b; h = mix(h, r);
	}
	print("long % %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r += b; h = mix(h, r);
	}
	print("long += %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r -= b; h = mix(h, r);
	}
	print("long -= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r *= b; h = mix(h, r);
	}
	print("long *= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r &= b; h = mix(h, r);
	}
	print("long &= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r |= b; h = mix(h, r);
	}
	print("long |= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r ^= b; h = mix(h, r);
	}
	print("long ^= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		r = get(i); r++; h = mix(h, r); r--; r--; h = mix(h, r);
	}
	print("long ++-- %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a == b; h = mix(h, c);
		if(a == b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("long == %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a != b; h = mix(h, c);
		if(a != b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("long != %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a < b; h = mix(h, c);
		if(a < b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("long < %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a <= b; h = mix(h, c);
		if(a <= b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("long <= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a > b; h = mix(h, c);
		if(a > b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("long > %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a >= b; h = mix(h, c);
		if(a >= b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("long >= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(k = 0; k < nelem(sh); k++){
		a = get(i); r = a << sh[k]; h = mix(h, r);
	}
	print("long << %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(k = 0; k < nelem(sh); k++){
		a = get(i); r = a >> sh[k]; h = mix(h, r);
	}
	print("long >> %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		char x, y[1];
		a = get(i);
		x = (char)a; h = mix(h, x);
		y[0] = (char)a;
		if(y[0] != (char)a) h = mix(h, 0xdead);
		if((char)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("long->char %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		uchar x, y[1];
		a = get(i);
		x = (uchar)a; h = mix(h, x);
		y[0] = (uchar)a;
		if(y[0] != (uchar)a) h = mix(h, 0xdead);
		if((uchar)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("long->uchar %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		short x, y[1];
		a = get(i);
		x = (short)a; h = mix(h, x);
		y[0] = (short)a;
		if(y[0] != (short)a) h = mix(h, 0xdead);
		if((short)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("long->short %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		ushort x, y[1];
		a = get(i);
		x = (ushort)a; h = mix(h, x);
		y[0] = (ushort)a;
		if(y[0] != (ushort)a) h = mix(h, 0xdead);
		if((ushort)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("long->ushort %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		int x, y[1];
		a = get(i);
		x = (int)a; h = mix(h, x);
		y[0] = (int)a;
		if(y[0] != (int)a) h = mix(h, 0xdead);
		if((int)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("long->int %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		uint x, y[1];
		a = get(i);
		x = (uint)a; h = mix(h, x);
		y[0] = (uint)a;
		if(y[0] != (uint)a) h = mix(h, 0xdead);
		if((uint)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("long->uint %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		long x, y[1];
		a = get(i);
		x = (long)a; h = mix(h, x);
		y[0] = (long)a;
		if(y[0] != (long)a) h = mix(h, 0xdead);
		if((long)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("long->long %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		ulong x, y[1];
		a = get(i);
		x = (ulong)a; h = mix(h, x);
		y[0] = (ulong)a;
		if(y[0] != (ulong)a) h = mix(h, 0xdead);
		if((ulong)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("long->ulong %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		vlong x, y[1];
		a = get(i);
		x = (vlong)a; h = mix(h, x);
		y[0] = (vlong)a;
		if(y[0] != (vlong)a) h = mix(h, 0xdead);
		if((vlong)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("long->vlong %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		uvlong x, y[1];
		a = get(i);
		x = (uvlong)a; h = mix(h, x);
		y[0] = (uvlong)a;
		if(y[0] != (uvlong)a) h = mix(h, 0xdead);
		if((uvlong)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("long->uvlong %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		a = get(i); d = a;
		memmove(&bits, &d, sizeof bits); h = mix(h, bits);
		if(d > -9.0e18 && d < 9.0e18){ r = d; h = mix(h, r); }
	}
	print("long double %llux\n", h);
}

static void
t_ulong(void)
{
	int i, j, k, c;
	ulong a, b, r;
	uvlong h, bits;
	double d;
	static int sh[] = {0, 1, 7, 8, 15, 16, 31};

	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a + b; h = mix(h, r);
	}
	print("ulong + %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a - b; h = mix(h, r);
	}
	print("ulong - %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a * b; h = mix(h, r);
	}
	print("ulong * %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a & b; h = mix(h, r);
	}
	print("ulong & %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a | b; h = mix(h, r);
	}
	print("ulong | %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a ^ b; h = mix(h, r);
	}
	print("ulong ^ %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		if(b == 0) continue;
		r = a / b; h = mix(h, r);
	}
	print("ulong / %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		if(b == 0) continue;
		r = a % b; h = mix(h, r);
	}
	print("ulong % %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r += b; h = mix(h, r);
	}
	print("ulong += %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r -= b; h = mix(h, r);
	}
	print("ulong -= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r *= b; h = mix(h, r);
	}
	print("ulong *= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r &= b; h = mix(h, r);
	}
	print("ulong &= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r |= b; h = mix(h, r);
	}
	print("ulong |= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r ^= b; h = mix(h, r);
	}
	print("ulong ^= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		r = get(i); r++; h = mix(h, r); r--; r--; h = mix(h, r);
	}
	print("ulong ++-- %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a == b; h = mix(h, c);
		if(a == b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("ulong == %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a != b; h = mix(h, c);
		if(a != b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("ulong != %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a < b; h = mix(h, c);
		if(a < b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("ulong < %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a <= b; h = mix(h, c);
		if(a <= b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("ulong <= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a > b; h = mix(h, c);
		if(a > b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("ulong > %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a >= b; h = mix(h, c);
		if(a >= b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("ulong >= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(k = 0; k < nelem(sh); k++){
		a = get(i); r = a << sh[k]; h = mix(h, r);
	}
	print("ulong << %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(k = 0; k < nelem(sh); k++){
		a = get(i); r = a >> sh[k]; h = mix(h, r);
	}
	print("ulong >> %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		char x, y[1];
		a = get(i);
		x = (char)a; h = mix(h, x);
		y[0] = (char)a;
		if(y[0] != (char)a) h = mix(h, 0xdead);
		if((char)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("ulong->char %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		uchar x, y[1];
		a = get(i);
		x = (uchar)a; h = mix(h, x);
		y[0] = (uchar)a;
		if(y[0] != (uchar)a) h = mix(h, 0xdead);
		if((uchar)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("ulong->uchar %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		short x, y[1];
		a = get(i);
		x = (short)a; h = mix(h, x);
		y[0] = (short)a;
		if(y[0] != (short)a) h = mix(h, 0xdead);
		if((short)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("ulong->short %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		ushort x, y[1];
		a = get(i);
		x = (ushort)a; h = mix(h, x);
		y[0] = (ushort)a;
		if(y[0] != (ushort)a) h = mix(h, 0xdead);
		if((ushort)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("ulong->ushort %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		int x, y[1];
		a = get(i);
		x = (int)a; h = mix(h, x);
		y[0] = (int)a;
		if(y[0] != (int)a) h = mix(h, 0xdead);
		if((int)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("ulong->int %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		uint x, y[1];
		a = get(i);
		x = (uint)a; h = mix(h, x);
		y[0] = (uint)a;
		if(y[0] != (uint)a) h = mix(h, 0xdead);
		if((uint)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("ulong->uint %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		long x, y[1];
		a = get(i);
		x = (long)a; h = mix(h, x);
		y[0] = (long)a;
		if(y[0] != (long)a) h = mix(h, 0xdead);
		if((long)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("ulong->long %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		ulong x, y[1];
		a = get(i);
		x = (ulong)a; h = mix(h, x);
		y[0] = (ulong)a;
		if(y[0] != (ulong)a) h = mix(h, 0xdead);
		if((ulong)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("ulong->ulong %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		vlong x, y[1];
		a = get(i);
		x = (vlong)a; h = mix(h, x);
		y[0] = (vlong)a;
		if(y[0] != (vlong)a) h = mix(h, 0xdead);
		if((vlong)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("ulong->vlong %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		uvlong x, y[1];
		a = get(i);
		x = (uvlong)a; h = mix(h, x);
		y[0] = (uvlong)a;
		if(y[0] != (uvlong)a) h = mix(h, 0xdead);
		if((uvlong)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("ulong->uvlong %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		a = get(i); d = a;
		memmove(&bits, &d, sizeof bits); h = mix(h, bits);
		if(d > -9.0e18 && d < 9.0e18){ r = d; h = mix(h, r); }
	}
	print("ulong double %llux\n", h);
}

static void
t_vlong(void)
{
	int i, j, k, c;
	vlong a, b, r;
	uvlong h, bits;
	double d;
	static int sh[] = {0, 1, 7, 8, 15, 16, 31, 32, 63};

	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a + b; h = mix(h, r);
	}
	print("vlong + %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a - b; h = mix(h, r);
	}
	print("vlong - %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a * b; h = mix(h, r);
	}
	print("vlong * %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a & b; h = mix(h, r);
	}
	print("vlong & %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a | b; h = mix(h, r);
	}
	print("vlong | %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a ^ b; h = mix(h, r);
	}
	print("vlong ^ %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		if(b == 0) continue;
		if(b == -1) continue;	/* MIN/-1 traps on amd64 */
		r = a / b; h = mix(h, r);
	}
	print("vlong / %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		if(b == 0) continue;
		if(b == -1) continue;	/* MIN/-1 traps on amd64 */
		r = a % b; h = mix(h, r);
	}
	print("vlong % %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r += b; h = mix(h, r);
	}
	print("vlong += %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r -= b; h = mix(h, r);
	}
	print("vlong -= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r *= b; h = mix(h, r);
	}
	print("vlong *= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r &= b; h = mix(h, r);
	}
	print("vlong &= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r |= b; h = mix(h, r);
	}
	print("vlong |= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r ^= b; h = mix(h, r);
	}
	print("vlong ^= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		r = get(i); r++; h = mix(h, r); r--; r--; h = mix(h, r);
	}
	print("vlong ++-- %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a == b; h = mix(h, c);
		if(a == b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("vlong == %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a != b; h = mix(h, c);
		if(a != b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("vlong != %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a < b; h = mix(h, c);
		if(a < b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("vlong < %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a <= b; h = mix(h, c);
		if(a <= b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("vlong <= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a > b; h = mix(h, c);
		if(a > b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("vlong > %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a >= b; h = mix(h, c);
		if(a >= b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("vlong >= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(k = 0; k < nelem(sh); k++){
		a = get(i); r = a << sh[k]; h = mix(h, r);
	}
	print("vlong << %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(k = 0; k < nelem(sh); k++){
		a = get(i); r = a >> sh[k]; h = mix(h, r);
	}
	print("vlong >> %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		char x, y[1];
		a = get(i);
		x = (char)a; h = mix(h, x);
		y[0] = (char)a;
		if(y[0] != (char)a) h = mix(h, 0xdead);
		if((char)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("vlong->char %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		uchar x, y[1];
		a = get(i);
		x = (uchar)a; h = mix(h, x);
		y[0] = (uchar)a;
		if(y[0] != (uchar)a) h = mix(h, 0xdead);
		if((uchar)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("vlong->uchar %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		short x, y[1];
		a = get(i);
		x = (short)a; h = mix(h, x);
		y[0] = (short)a;
		if(y[0] != (short)a) h = mix(h, 0xdead);
		if((short)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("vlong->short %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		ushort x, y[1];
		a = get(i);
		x = (ushort)a; h = mix(h, x);
		y[0] = (ushort)a;
		if(y[0] != (ushort)a) h = mix(h, 0xdead);
		if((ushort)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("vlong->ushort %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		int x, y[1];
		a = get(i);
		x = (int)a; h = mix(h, x);
		y[0] = (int)a;
		if(y[0] != (int)a) h = mix(h, 0xdead);
		if((int)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("vlong->int %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		uint x, y[1];
		a = get(i);
		x = (uint)a; h = mix(h, x);
		y[0] = (uint)a;
		if(y[0] != (uint)a) h = mix(h, 0xdead);
		if((uint)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("vlong->uint %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		long x, y[1];
		a = get(i);
		x = (long)a; h = mix(h, x);
		y[0] = (long)a;
		if(y[0] != (long)a) h = mix(h, 0xdead);
		if((long)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("vlong->long %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		ulong x, y[1];
		a = get(i);
		x = (ulong)a; h = mix(h, x);
		y[0] = (ulong)a;
		if(y[0] != (ulong)a) h = mix(h, 0xdead);
		if((ulong)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("vlong->ulong %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		vlong x, y[1];
		a = get(i);
		x = (vlong)a; h = mix(h, x);
		y[0] = (vlong)a;
		if(y[0] != (vlong)a) h = mix(h, 0xdead);
		if((vlong)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("vlong->vlong %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		uvlong x, y[1];
		a = get(i);
		x = (uvlong)a; h = mix(h, x);
		y[0] = (uvlong)a;
		if(y[0] != (uvlong)a) h = mix(h, 0xdead);
		if((uvlong)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("vlong->uvlong %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		a = get(i); d = a;
		memmove(&bits, &d, sizeof bits); h = mix(h, bits);
		if(d > -9.0e18 && d < 9.0e18){ r = d; h = mix(h, r); }
	}
	print("vlong double %llux\n", h);
}

static void
t_uvlong(void)
{
	int i, j, k, c;
	uvlong a, b, r;
	uvlong h, bits;
	double d;
	static int sh[] = {0, 1, 7, 8, 15, 16, 31, 32, 63};

	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a + b; h = mix(h, r);
	}
	print("uvlong + %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a - b; h = mix(h, r);
	}
	print("uvlong - %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a * b; h = mix(h, r);
	}
	print("uvlong * %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a & b; h = mix(h, r);
	}
	print("uvlong & %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a | b; h = mix(h, r);
	}
	print("uvlong | %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		r = a ^ b; h = mix(h, r);
	}
	print("uvlong ^ %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		if(b == 0) continue;
		r = a / b; h = mix(h, r);
	}
	print("uvlong / %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		if(b == 0) continue;
		r = a % b; h = mix(h, r);
	}
	print("uvlong % %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r += b; h = mix(h, r);
	}
	print("uvlong += %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r -= b; h = mix(h, r);
	}
	print("uvlong -= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r *= b; h = mix(h, r);
	}
	print("uvlong *= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r &= b; h = mix(h, r);
	}
	print("uvlong &= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r |= b; h = mix(h, r);
	}
	print("uvlong |= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j); r = a; r ^= b; h = mix(h, r);
	}
	print("uvlong ^= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		r = get(i); r++; h = mix(h, r); r--; r--; h = mix(h, r);
	}
	print("uvlong ++-- %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a == b; h = mix(h, c);
		if(a == b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uvlong == %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a != b; h = mix(h, c);
		if(a != b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uvlong != %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a < b; h = mix(h, c);
		if(a < b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uvlong < %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a <= b; h = mix(h, c);
		if(a <= b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uvlong <= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a > b; h = mix(h, c);
		if(a > b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uvlong > %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(j = 0; j < NV; j++){
		a = get(i); b = get(j);
		c = a >= b; h = mix(h, c);
		if(a >= b) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uvlong >= %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(k = 0; k < nelem(sh); k++){
		a = get(i); r = a << sh[k]; h = mix(h, r);
	}
	print("uvlong << %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++) for(k = 0; k < nelem(sh); k++){
		a = get(i); r = a >> sh[k]; h = mix(h, r);
	}
	print("uvlong >> %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		char x, y[1];
		a = get(i);
		x = (char)a; h = mix(h, x);
		y[0] = (char)a;
		if(y[0] != (char)a) h = mix(h, 0xdead);
		if((char)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uvlong->char %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		uchar x, y[1];
		a = get(i);
		x = (uchar)a; h = mix(h, x);
		y[0] = (uchar)a;
		if(y[0] != (uchar)a) h = mix(h, 0xdead);
		if((uchar)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uvlong->uchar %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		short x, y[1];
		a = get(i);
		x = (short)a; h = mix(h, x);
		y[0] = (short)a;
		if(y[0] != (short)a) h = mix(h, 0xdead);
		if((short)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uvlong->short %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		ushort x, y[1];
		a = get(i);
		x = (ushort)a; h = mix(h, x);
		y[0] = (ushort)a;
		if(y[0] != (ushort)a) h = mix(h, 0xdead);
		if((ushort)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uvlong->ushort %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		int x, y[1];
		a = get(i);
		x = (int)a; h = mix(h, x);
		y[0] = (int)a;
		if(y[0] != (int)a) h = mix(h, 0xdead);
		if((int)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uvlong->int %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		uint x, y[1];
		a = get(i);
		x = (uint)a; h = mix(h, x);
		y[0] = (uint)a;
		if(y[0] != (uint)a) h = mix(h, 0xdead);
		if((uint)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uvlong->uint %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		long x, y[1];
		a = get(i);
		x = (long)a; h = mix(h, x);
		y[0] = (long)a;
		if(y[0] != (long)a) h = mix(h, 0xdead);
		if((long)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uvlong->long %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		ulong x, y[1];
		a = get(i);
		x = (ulong)a; h = mix(h, x);
		y[0] = (ulong)a;
		if(y[0] != (ulong)a) h = mix(h, 0xdead);
		if((ulong)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uvlong->ulong %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		vlong x, y[1];
		a = get(i);
		x = (vlong)a; h = mix(h, x);
		y[0] = (vlong)a;
		if(y[0] != (vlong)a) h = mix(h, 0xdead);
		if((vlong)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uvlong->vlong %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		uvlong x, y[1];
		a = get(i);
		x = (uvlong)a; h = mix(h, x);
		y[0] = (uvlong)a;
		if(y[0] != (uvlong)a) h = mix(h, 0xdead);
		if((uvlong)a == y[0]) h = mix(h, 1); else h = mix(h, 2);
	}
	print("uvlong->uvlong %llux\n", h);
	h = H0;
	for(i = 0; i < NV; i++){
		a = get(i); d = a;
		memmove(&bits, &d, sizeof bits); h = mix(h, bits);
		if(d > -9.0e18 && d < 9.0e18){ r = d; h = mix(h, r); }
	}
	print("uvlong double %llux\n", h);
}

void
main(void)
{
	teststart();
	print("sizes c%d s%d i%d l%d v%d p%d charsigned %d\n",
		(int)sizeof(char), (int)sizeof(short), (int)sizeof(int),
		(int)sizeof(long), (int)sizeof(vlong), (int)sizeof(void*),
		(char)get(7) < 0);
	t_char();
	t_uchar();
	t_short();
	t_ushort();
	t_int();
	t_uint();
	t_long();
	t_ulong();
	t_vlong();
	t_uvlong();
	exits(nil);
}
