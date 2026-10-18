#!/usr/bin/env python3
# convgen.py > convtest.c
# Differential test of integer codegen: build with 6c and with jc,
# run both, outputs must be identical.

T = [("char",8,1), ("uchar",8,0), ("short",16,1), ("ushort",16,0),
     ("int",32,1), ("uint",32,0), ("long",32,1), ("ulong",32,0),
     ("vlong",64,1), ("uvlong",64,0)]

V = ["0", "1", "-1", "2", "-2", "127", "128", "255", "256", "-128", "-129",
     "32767", "32768", "65535", "65536", "-32768", "-32769",
     "0x7fffffffLL", "0x80000000LL", "0xffffffffLL", "0x100000000LL",
     "-0x80000000LL", "-0x80000001LL",
     "0x7fffffffffffffffLL", "(-0x7fffffffffffffffLL-1)"]

SH = [0, 1, 7, 8, 15, 16, 31, 32, 63]

out = []
p = out.append

p("#include \"../test.h\"")
p("")
p("static vlong vals[] = {%s};" % ", ".join(V))
p("#define NV %d" % len(V))
p("#define H0 0xcbf29ce484222325ULL")
p("")
p("/* through a call, so the shared front end cannot constant-fold */")
p("static vlong get(int i) { return vals[i]; }")
p("static uvlong mix(uvlong h, uvlong x) { return (h ^ x) * 0x100000001b3ULL; }")
p("")

def cell(label):
    p('\tprint("%s %%llux\\n", h);' % label)

for (t, w, sg) in T:
    p("static void")
    p("t_%s(void)" % t)
    p("{")
    p("\tint i, j, k, c;")
    p("\t%s a, b, r;" % t)
    p("\tuvlong h, bits;")
    p("\tdouble d;")
    p("\tstatic int sh[] = {%s};" % ", ".join(str(s) for s in SH if s < w))
    p("")

    for op in ["+", "-", "*", "&", "|", "^", "/", "%"]:
        p("\th = H0;")
        p("\tfor(i = 0; i < NV; i++) for(j = 0; j < NV; j++){")
        p("\t\ta = get(i); b = get(j);")
        if op in "/%":
            p("\t\tif(b == 0) continue;")
            if sg:
                p("\t\tif(b == -1) continue;\t/* MIN/-1 traps on amd64 */")
        p("\t\tr = a %s b; h = mix(h, r);" % op)
        p("\t}")
        cell("%s %s" % (t, op))

    for op in ["+=", "-=", "*=", "&=", "|=", "^="]:
        p("\th = H0;")
        p("\tfor(i = 0; i < NV; i++) for(j = 0; j < NV; j++){")
        p("\t\ta = get(i); b = get(j); r = a; r %s b; h = mix(h, r);" % op)
        p("\t}")
        cell("%s %s" % (t, op))

    p("\th = H0;")
    p("\tfor(i = 0; i < NV; i++){")
    p("\t\tr = get(i); r++; h = mix(h, r); r--; r--; h = mix(h, r);")
    p("\t}")
    cell("%s ++--" % t)

    for op in ["==", "!=", "<", "<=", ">", ">="]:
        p("\th = H0;")
        p("\tfor(i = 0; i < NV; i++) for(j = 0; j < NV; j++){")
        p("\t\ta = get(i); b = get(j);")
        p("\t\tc = a %s b; h = mix(h, c);" % op)
        p("\t\tif(a %s b) h = mix(h, 1); else h = mix(h, 2);" % op)
        p("\t}")
        cell("%s %s" % (t, op))

    for op in ["<<", ">>"]:
        p("\th = H0;")
        p("\tfor(i = 0; i < NV; i++) for(k = 0; k < nelem(sh); k++){")
        p("\t\ta = get(i); r = a %s sh[k]; h = mix(h, r);" % op)
        p("\t}")
        cell("%s %s" % (t, op))

    for (t2, _, _) in T:
        p("\th = H0;")
        p("\tfor(i = 0; i < NV; i++){")
        p("\t\t%s x, y[1];" % t2)
        p("\t\ta = get(i);")
        p("\t\tx = (%s)a; h = mix(h, x);" % t2)
        p("\t\ty[0] = (%s)a;" % t2)
        p("\t\tif(y[0] != (%s)a) h = mix(h, 0xdead);" % t2)
        p("\t\tif((%s)a == y[0]) h = mix(h, 1); else h = mix(h, 2);" % t2)
        p("\t}")
        cell("%s->%s" % (t, t2))

    p("\th = H0;")
    p("\tfor(i = 0; i < NV; i++){")
    p("\t\ta = get(i); d = a;")
    p("\t\tmemmove(&bits, &d, sizeof bits); h = mix(h, bits);")
    p("\t\tif(d > -9.0e18 && d < 9.0e18){ r = d; h = mix(h, r); }")
    p("\t}")
    cell("%s double" % t)
    p("}")
    p("")

p("void")
p("main(void)")
p("{")
p("\tteststart();")
p('\tprint("sizes c%d s%d i%d l%d v%d p%d charsigned %d\\n",')
p("\t\t(int)sizeof(char), (int)sizeof(short), (int)sizeof(int),")
p("\t\t(int)sizeof(long), (int)sizeof(vlong), (int)sizeof(void*),")
p("\t\t(char)get(7) < 0);")
for (t, _, _) in T:
    p("\tt_%s();" % t)
p("\texits(nil);")
p("}")

print("\n".join(out))
