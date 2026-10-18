/* NaN comparisons: only != is true when either operand is a NaN, for
 * double and float, with the operands in both orders.  (jc: OGT/OGE swap,
 * boolgen FP default.)  Self-checking: 6c is not IEEE on NaN. */
#include "../test.h"
static double zero = 0.0, big = 1e300, c15 = 1.5, c25 = 2.5;

static void
chk(char *what, char *nm, int v, int b, int s, int want)
{
	if(v != want || b != want || s != want){
		testfail("%s %s: value %d branch %d swapped %d want %d",
			what, nm, v, b, s, want);
	}
}

#define CHECK(T, what, nm, A, N, want) { \
	T a, n; int v, b, s; \
	a = A; n = N; \
	v = (a==n) | (a!=n)<<1 | (a<n)<<2 | (a<=n)<<3 | (a>n)<<4 | (a>=n)<<5; \
	b = 0; \
	if(a == n) b |= 1; \
	if(a != n) b |= 2; \
	if(a < n) b |= 4; \
	if(a <= n) b |= 8; \
	if(a > n) b |= 16; \
	if(a >= n) b |= 32; \
	s = 0;			/* the same relations, operands swapped */ \
	if(n == a) s |= 1; \
	if(n != a) s |= 2; \
	if(n > a) s |= 4; \
	if(n >= a) s |= 8; \
	if(n < a) s |= 16; \
	if(n <= a) s |= 32; \
	chk(what, nm, v, b, s, want); }

void
main(void)
{
	double inf, nan;

	teststart();
	inf = big * big;
	nan = inf - inf;

	/* unordered: only != is true */
	CHECK(double, "double", "1.5 v nan", c15, nan, 2)
	CHECK(double, "double", "0 v nan", zero, nan, 2)
	CHECK(double, "double", "inf v nan", inf, nan, 2)
	CHECK(double, "double", "nan v nan", nan, nan, 2)
	CHECK(float, "float", "1.5 v nan", c15, nan, 2)
	CHECK(float, "float", "0 v nan", zero, nan, 2)
	CHECK(float, "float", "inf v nan", inf, nan, 2)
	CHECK(float, "float", "nan v nan", nan, nan, 2)

	/* ordered, must not regress */
	CHECK(double, "double", "1.5 v 2.5", c15, c25, 2|4|8)
	CHECK(double, "double", "2.5 v 1.5", c25, c15, 2|16|32)
	CHECK(double, "double", "1.5 v 1.5", c15, c15, 1|8|32)
	CHECK(float, "float", "1.5 v 2.5", c15, c25, 2|4|8)
	CHECK(float, "float", "2.5 v 1.5", c25, c15, 2|16|32)
	CHECK(float, "float", "1.5 v 1.5", c15, c15, 1|8|32)

	testend();
}
