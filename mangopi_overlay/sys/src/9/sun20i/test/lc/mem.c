/* libc memory routines (SWAR assembly in libc/riscv64) against plain C
 * byte loops, over all alignments and lengths 0..40, with the target byte
 * at every position, 0x00/0x80/0xff bytes around it, and canaries to
 * catch writes past the end. */
#include "../test.h"

enum { Max = 40, Pad = 16 };

static uchar a[Max+2*Pad+8], b[Max+2*Pad+8], c[Max+2*Pad+8];

static void *
refchr(uchar *s, int ch, ulong n)
{
	ulong i;

	for(i = 0; i < n; i++)
		if(s[i] == (uchar)ch)
			return s+i;
	return nil;
}

static int
refcmp(uchar *s1, uchar *s2, ulong n)
{
	ulong i;

	for(i = 0; i < n; i++)
		if(s1[i] != s2[i])
			return s1[i] < s2[i]? -1: 1;
	return 0;
}

static int
sgn(int x)
{
	return x < 0? -1: x > 0;
}

static void
fill(uchar *p, int seed)
{
	int i;
	static uchar tricky[] = {0x00, 0x80, 0xff, 0x7f, 0x01, 0x81, 0xfe};

	for(i = 0; i < Max+2*Pad+8; i++)
		p[i] = (i*seed + (i>>3)) & 0xff;
	for(i = seed % 5; i < Max+2*Pad+8; i += 5)
		p[i] = tricky[(i+seed) % sizeof tricky];
}

static void
chrtest(void)
{
	int off, n, pos, ch;
	uchar *s, *g, *w;
	static int chs[] = {0x00, 0x01, 0x80, 0xff, 0x7f, 0x41};
	int k;

	for(off = 0; off < 8; off++)
	for(n = 0; n <= Max; n++){
		for(k = 0; k < nelem(chs); k++){
			ch = chs[k];
			fill(a, 3+k);
			s = a + Pad + off;
			/* scrub the byte so the match, if any, is where we put it */
			for(pos = 0; pos < n; pos++)
				if(s[pos] == ch)
					s[pos] = ch ^ 0x55;
			for(pos = -1; pos < n; pos++){
				if(pos >= 0)
					s[pos] = ch;
				g = memchr(s, ch, n);
				w = refchr(s, ch, n);
				if(g != w)
					testfail("memchr off %d n %d ch %#ux pos %d: got %p want %p",
						off, n, ch, pos, g, w);
				if(pos >= 0)
					s[pos] = ch ^ 0x55;
			}
		}
	}
}

static void
cmptest(void)
{
	int off1, off2, n, pos, d;
	uchar *s1, *s2;

	for(off1 = 0; off1 < 8; off1 += 3)
	for(off2 = 0; off2 < 8; off2++)
	for(n = 0; n <= Max; n++)
	for(d = 0; d < 3; d++){
		fill(a, 7);
		fill(b, 7);
		s1 = a + Pad + off1;
		s2 = b + Pad + off2;
		memmove(s2, s1, n);
		for(pos = -1; pos < n; pos++){
			if(pos >= 0)		/* differ at pos, in several ways */
				switch(d){
				case 0: s2[pos] = s1[pos] + 1; break;
				case 1: s2[pos] = s1[pos] ^ 0x80; break;
				case 2: s2[pos] = s1[pos] - 1; break;
				}
			if(sgn(memcmp(s1, s2, n)) != sgn(refcmp(s1, s2, n)))
				testfail("memcmp off %d,%d n %d pos %d d %d: got %d want %d",
					off1, off2, n, pos, d,
					sgn(memcmp(s1, s2, n)), sgn(refcmp(s1, s2, n)));
			if(pos >= 0)
				s2[pos] = s1[pos];
		}
	}
}

/* memccpy: stops after copying the first byte equal to ch */
static void
ccpytest(void)
{
	int off1, off2, n, pos, ch, i;
	uchar *s, *d, *r, *want;
	ulong cnt;

	for(off1 = 0; off1 < 8; off1 += 3)
	for(off2 = 0; off2 < 8; off2++)
	for(n = 0; n <= Max; n++)
	for(pos = -1; pos < n; pos++){
		ch = 0x80;
		fill(a, 11);
		s = a + Pad + off1;
		for(i = 0; i < n; i++)
			if(s[i] == ch)
				s[i] = 0x01;
		if(pos >= 0)
			s[pos] = ch;
		memset(b, 0xaa, sizeof b);
		memset(c, 0xaa, sizeof c);
		d = b + Pad + off2;
		want = c + Pad + off2;

		r = memccpy(d, s, ch, n);

		cnt = pos >= 0? pos+1: n;
		memmove(want, s, cnt);
		if(pos >= 0 && r != d + cnt)
			testfail("memccpy off %d,%d n %d pos %d: ret %p want %p",
				off1, off2, n, pos, r, d+cnt);
		if(pos < 0 && r != nil)
			testfail("memccpy off %d,%d n %d no match: ret %p want nil",
				off1, off2, n, r);
		if(memcmp(b, c, sizeof b) != 0)
			testfail("memccpy off %d,%d n %d pos %d: dest or canary differs",
				off1, off2, n, pos);
	}
}

static void
movetest(void)
{
	int off1, off2, n, i;
	uchar *s, *d;

	for(off1 = 0; off1 < 8; off1++)
	for(off2 = 0; off2 < 8; off2++)
	for(n = 0; n <= Max; n++){
		/* overlapping, both directions, inside one buffer */
		fill(a, 13);
		memmove(c, a, sizeof c);
		s = a + Pad + off1;
		d = a + Pad + off2;
		memmove(d, s, n);
		for(i = 0; i < n; i++)
			if(d[i] != c[Pad+off1+i])
				break;
		if(i < n || memcmp(a, c, Pad) != 0)
			testfail("memmove overlap %d->%d n %d: byte %d", off1, off2, n, i);
		/* separate buffers + canaries */
		fill(a, 17);
		memset(b, 0x5a, sizeof b);
		memmove(b+Pad+off2, a+Pad+off1, n);
		if(memcmp(b+Pad+off2, a+Pad+off1, n) != 0
		|| b[Pad+off2-1] != 0x5a || b[Pad+off2+n] != 0x5a)
			testfail("memmove %d->%d n %d: data or canary", off1, off2, n);
		memset(b, 0x5a, sizeof b);
		memset(b+Pad+off2, 0xc3, n);
		for(i = 0; i < n; i++)
			if(b[Pad+off2+i] != 0xc3)
				break;
		if(i < n || b[Pad+off2-1] != 0x5a || b[Pad+off2+n] != 0x5a)
			testfail("memset off %d n %d", off2, n);
	}
}

void
main(void)
{
	teststart();
	chrtest();
	cmptest();
	ccpytest();
	movetest();
	testend();
}
