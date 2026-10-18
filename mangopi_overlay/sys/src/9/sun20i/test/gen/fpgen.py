#!/usr/bin/env python3
# fpgen.py > fptest.c
# Floating-point / calls / structs differential test: build with 6c and jc,
# run both, outputs must be identical.  Companion to convgen.py.

DV = ["0.0", "1.0", "-1.0", "0.5", "-0.5", "1.5", "-2.5", "3.0", "0.1",
      "-0.3", "1e-10", "1e10", "123456789.0", "2147483648.0",
      "4294967296.0", "1e300", "-1e300", "1e-300", "4.9e-324",
      "1.7976931348623157e308", "16777217.0", "0.3333333333333333"]
IBIG, IMBIG, IM1 = DV.index("1e300"), DV.index("-1e300"), DV.index("-1.0")
NB = len(DV)

V = ["0", "1", "-1", "2", "-2", "127", "128", "255", "256", "-128", "-129",
     "32767", "32768", "65535", "65536", "-32768", "-32769",
     "0x7fffffffLL", "0x80000000LL", "0xffffffffLL", "0x100000000LL",
     "-0x80000000LL", "-0x80000001LL",
     "0x7fffffffffffffffLL", "(-0x7fffffffffffffffLL-1)"]

FP = [("float", "gf"), ("double", "gd")]
# name, signed, exclusive low bound, exclusive high bound for FP->int in range.
# uvlong stops at 9.0e18 on purpose: >= 2^63 goes through the signed convert
# (known open item) and x86/RISC-V disagree out of range anyway.
INTS = [("char", 1, "-129.0", "128.0"), ("uchar", 0, "-1.0", "256.0"),
        ("short", 1, "-32769.0", "32768.0"), ("ushort", 0, "-1.0", "65536.0"),
        ("int", 1, "-2147483649.0", "2147483648.0"),
        ("uint", 0, "-1.0", "4294967296.0"),
        ("long", 1, "-2147483649.0", "2147483648.0"),
        ("ulong", 0, "-1.0", "4294967296.0"),
        ("vlong", 1, "-9.0e18", "9.0e18"), ("uvlong", 0, "-1.0", "9.0e18")]
ALL = [(n, "gv", "i") for (n, _, _, _) in INTS] + \
      [(n, g, "f") for (n, g) in FP]

out = []
p = out.append

def T(s, **kw):
    for k, v in kw.items():
        s = s.replace("@" + k + "@", str(v))
    return s

import os
# 6c is not IEEE on NaN, so the nan cells cannot be compared with its
# reference (cc/nan.c checks NaN semantics self-consistently instead).
# KEEPNAN=1 python3 fpgen.py keeps them, for jc vs jc -R comparison.
def cell(label):
    if label.startswith("nan ") and not os.environ.get("KEEPNAN"):
        p('\t(void)h;\t/* %s: not compared with 6c */' % label)
        return
    p('\tprint("%s %%llux\\n", h);' % label)

p("#include \"../test.h\"")
p("")
p("static double base[] = {%s};" % ", ".join(DV))
p("static vlong ivals[] = {%s};" % ", ".join(V))
p("#define NV %d" % len(V))
p("#define NB %d" % NB)
p("#define NDV (NB+3)\t/* + inf, -inf, -0.0 */")
p("#define IINF NB")
p("#define H0 0xcbf29ce484222325ULL")
p("")
p("static double dv[NDV];")
p("static float fv[NDV];")
p("static double nanv;")
p("")
p("/* all values arrive through calls so the shared front end cannot fold */")
p("static vlong gv(int i) { return ivals[i]; }")
p("static double gd(int i) { return dv[i]; }")
p("static float gf(int i) { return fv[i]; }")
p("")
p("static void")
p("init(void)")
p("{")
p("\tint i;")
p("")
p("\tfor(i = 0; i < NB; i++)")
p("\t\tdv[i] = base[i];")
p("\tdv[NB] = dv[%d] * dv[%d];\t/* +inf */" % (IBIG, IBIG))
p("\tdv[NB+1] = dv[%d] * dv[%d];\t/* -inf */" % (IBIG, IMBIG))
p("\tdv[NB+2] = dv[0] * dv[%d];\t/* -0.0, without relying on unary minus */" % IM1)
p("\tnanv = dv[NB] - dv[NB];")
p("\tfor(i = 0; i < NDV; i++)")
p("\t\tfv[i] = dv[i];")
p("}")
p("")
p("static uvlong mix(uvlong h, uvlong x) { return (h ^ x) * 0x100000001b3ULL; }")
p("")
p("/* NaN payload and sign differ legitimately between x86 and RISC-V */")
p("static uvlong")
p("hd(uvlong h, double d)")
p("{")
p("\tuvlong b;")
p("")
p("\tmemmove(&b, &d, sizeof b);")
p("\tif((b & 0x7ff0000000000000ULL) == 0x7ff0000000000000ULL")
p("\t&& (b & 0x000fffffffffffffULL) != 0)")
p("\t\tb = 0x7ff8000000000000ULL;	/* any NaN */")
p("\treturn mix(h, b);")
p("}")
p("")

# ---- per-FP-type cells ----------------------------------------------------
for (t, g) in FP:
    p("static void")
    p("t_%s(void)" % t)
    p("{")
    p("\tint i, j, c;")
    p("\t%s a, b, r;" % t)
    p("\tvlong w;")
    p("\tuvlong h;")
    p("")
    PAIR = "\tfor(i = 0; i < NDV; i++) for(j = 0; j < NDV; j++){\n" \
           "\t\ta = %s(i); b = %s(j);" % (g, g)

    for op in ["+", "-", "*", "/"]:
        p("\th = H0;")
        p(PAIR)
        p("\t\tr = a %s b; h = hd(h, r);" % op)
        p("\t}")
        cell("%s %s" % (t, op))

    for op in ["==", "!=", "<", "<=", ">", ">="]:
        p("\th = H0;")
        p(PAIR)
        p("\t\tc = a %s b; h = mix(h, c);" % op)
        p("\t\tif(a %s b) h = mix(h, 1); else h = mix(h, 2);" % op)
        p("\t}")
        cell("%s %s" % (t, op))

    p("\th = H0;")
    p(PAIR)
    p("\t\tc = a && b; h = mix(h, c); c = a || b; h = mix(h, c);")
    p("\t\tif(a && b) h = mix(h, 1); else h = mix(h, 2);")
    p("\t\tif(a || b) h = mix(h, 3); else h = mix(h, 4);")
    p("\t}")
    cell("%s &&||" % t)

    p("\th = H0;")
    p("\tfor(i = 0; i < NDV; i++){")
    p("\t\ta = %s(i); r = -a; h = hd(h, r); c = !a; h = mix(h, c);" % g)
    p("\t\tif(a) h = mix(h, 1); else h = mix(h, 2);")
    p("\t\tr = a; r++; h = hd(h, r); r--; r--; h = hd(h, r);")
    p("\t}")
    cell("%s neg!++--" % t)

    for op in ["+=", "-=", "*=", "/="]:
        p("\th = H0;")
        p(PAIR)
        p("\t\tr = a; r %s b; h = hd(h, r);" % op)
        p("\t}")
        cell("%s %s" % (t, op))

    # FP -> FP
    for (t2, _) in FP:
        if t2 == t:
            continue
        p("\th = H0;")
        p("\tfor(i = 0; i < NDV; i++){")
        p("\t\t%s x; x = %s(i); h = hd(h, x);" % (t2, g))
        p("\t}")
        cell("%s->%s" % (t, t2))

    # FP -> int, in range only
    for (u, sg, lo, hi) in INTS:
        p("\th = H0;")
        p("\tfor(i = 0; i < NDV; i++){")
        p("\t\t%s x; a = %s(i);" % (u, g))
        p("\t\tif(a > %s && a < %s){ x = a; h = mix(h, (vlong)x); }" % (lo, hi))
        p("\t}")
        cell("%s->%s" % (t, u))

    # int -> FP
    for (u, _, _, _) in INTS:
        p("\th = H0;")
        p("\tfor(i = 0; i < NV; i++){")
        p("\t\t%s x; x = gv(i); r = x; h = hd(h, r);" % u)
        p("\t}")
        cell("%s->%s" % (u, t))

    # mixed binary / comparison / ?:
    for (u, _, _, _) in INTS:
        p("\th = H0;")
        p("\tfor(i = 0; i < NDV; i++) for(j = 0; j < NV; j++){")
        p("\t\t%s x; a = %s(i); x = gv(j);" % (u, g))
        p("\t\tr = a + x; h = hd(h, r); r = a - x; h = hd(h, r);")
        p("\t\tr = a * x; h = hd(h, r); r = a / x; h = hd(h, r);")
        p("\t\tc = a < x; h = mix(h, c); c = a == x; h = mix(h, c);")
        p("\t\tif(a > x) h = mix(h, 1); else h = mix(h, 2);")
        p("\t\tw = gv(j); r = (i & 1) ? a : w; h = hd(h, r);")
        p("\t}")
        cell("%s mixed %s" % (t, u))

    # Literal constants.  jc keeps 0.0/0.5/1.0/2.0 in F28-F31 as 64-bit double
    # patterns; a single-precision instruction reading one of those sees a
    # non-NaN-boxed value, i.e. NaN.  Nothing else in this file uses float
    # literals, so without this cell that failure is invisible.
    sfx = "f" if t == "float" else ""
    lits = ["0.0", "0.5", "1.0", "2.0", "1.5", "2.5", "3.0",
            "-0.5", "-1.0", "-2.0", "0.25", "100.0"]
    p("\th = H0;")
    p("\t{")
    p("\t\tdouble d;")
    p("")
    for lit in lits:
        p("\t\tr = %s%s; h = hd(h, r);" % (lit, sfx))
    p("\t\tfor(i = 0; i < NDV; i++){")
    p("\t\t\ta = %s(i);" % g)
    p("\t\t\tr = a * 0.5%s; h = hd(h, r);" % sfx)
    p("\t\t\tr = a + 1.0%s; h = hd(h, r);" % sfx)
    p("\t\t\tr = a - 2.0%s; h = hd(h, r);" % sfx)
    p("\t\t\tr = a * 2.0%s; h = hd(h, r);" % sfx)
    p("\t\t\tr = a / 2.0%s; h = hd(h, r);" % sfx)
    p("\t\t\tr = 1.0%s - a; h = hd(h, r);" % sfx)
    p("\t\t\tr = 0.0%s - a; h = hd(h, r);" % sfx)
    p("\t\t\tc = a < 0.5%s; h = mix(h, c);" % sfx)
    p("\t\t\tc = a == 0.0%s; h = mix(h, c);" % sfx)
    p("\t\t\tc = a > 2.0%s; h = mix(h, c);" % sfx)
    p("\t\t\td = a; d = d * 0.5 + 1.0; h = hd(h, d);")
    p("\t\t}")
    p("\t}")
    cell("%s literals" % t)

    # NaN: separate cells, judged separately (see notes)
    p("\th = H0;")
    p("\tfor(i = 0; i < NDV; i++){")
    p("\t\t%s n; a = %s(i); n = nanv;" % (t, g))
    for op in ["==", "!=", "<", "<=", ">", ">="]:
        p("\t\tc = a %s n; h = mix(h, c); c = n %s a; h = mix(h, c);" % (op, op))
        p("\t\tif(a %s n) h = mix(h, 1); else h = mix(h, 2);" % op)
    p("\t\tr = a + n; h = hd(h, r); r = n * a; h = hd(h, r);")
    p("\t\tif(n) h = mix(h, 1); else h = mix(h, 2);")
    p("\t\tc = !n; h = mix(h, c);")
    p("\t}")
    cell("nan %s" % t)
    p("}")
    p("")

# ---- mixed-type compound assignment: the OASADD/OASMUL neighbourhood -----
p("static int pi[] = {4, 5, 6, 7}, si[] = {-7, -5, 4, 6};")
p("static double pf[] = {0.5, 1.5, 2.25, 3.75}, sf[] = {-3.75, -1.5, 0.5, 2.25};")
p("")
p("static void")
p("t_compound(void)")
p("{")
p("\tint i, j;")
p("\tuvlong h;")
p("")
for (t1, _, k1) in ALL:
    u1 = dict((n, s) for (n, s, _, _) in INTS).get(t1, 1)
    for (t2, _, k2) in ALL:
        u2 = dict((n, s) for (n, s, _, _) in INTS).get(t2, 1)
        pos = (not u1) or (not u2)
        sr = ("pi" if pos else "si") if k1 == "i" else ("pf" if pos else "sf")
        sb = ("pi" if pos else "si") if k2 == "i" else ("pf" if pos else "sf")
        hs = (lambda v: "h = mix(h, (vlong)%s);" % v) if k1 == "i" \
             else (lambda v: "h = hd(h, %s);" % v)
        p("\th = H0;")
        p("\tfor(i = 0; i < 4; i++) for(j = 0; j < 4; j++){")
        p("\t\t%s r; %s b; b = %s[j];" % (t1, t2, sb))
        for op in ["+=", "-=", "*=", "/="]:
            p("\t\tr = %s[i]; r %s b; %s" % (sr, op, hs("r")))
        p("\t}")
        cell("%s %s=" % (t1, t2) if False else "cmp %s <- %s" % (t1, t2))
p("}")
p("")

# ---- calls, varargs, structs ----------------------------------------------
p(T('''
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
	print("calls argsum %llux\\n", h);

	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NDV; j++){
		k = (i + j) % NDV;
		h = hd(h, fneg(gf(i)));
		h = hd(h, f3(gf(i), gf(j), gf(k)));
		h = hd(h, fnp[0](gf(i), gf(j), gf(k)));
		h = hd(h, d3(gd(i), gf(j), gv(k % NV)));
	}
	print("calls float %llux\\n", h);

	h = H0;
	for(i = 0; i < NDV; i++) for(j = 0; j < NDV; j++){
		k = (i + j) % NDV;
		h = hd(h, vsumd(3, gd(i), (double)gf(j), gd(k)));
		h = mix(h, vmix(2, (int)gv(i % NV), gd(j), gv(k % NV),
			(int)gv(j % NV), gd(i), gv((k+1) % NV)));
	}
	print("calls varargs %llux\\n", h);
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
	print("struct byvalue %llux\\n", h);
}
'''))

p("void")
p("main(void)")
p("{")
p("\t/* amd64 Plan 9 traps on invalid/zdiv/overflow; init() already makes NaN/inf,")
p("\t * so the masks must be set before it.  RISC-V never traps: a no-op there. */")
p("\tsetfcr(getfcr() & ~(FPINVAL|FPZDIV|FPOVFL|FPUNFL|FPINEX));")
p("\tinit();")
p("\tteststart();")
p('\tprint("sizes f%d d%d S%d\\n", (int)sizeof(float), (int)sizeof(double),')
p("\t\t(int)sizeof(S));")
p("\tt_float();")
p("\tt_double();")
p("\tt_compound();")
p("\tt_calls();")
p("\tt_struct();")
p("\texits(nil);")
p("}")

print("\n".join(out))
