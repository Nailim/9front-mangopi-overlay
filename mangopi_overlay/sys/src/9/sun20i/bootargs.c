#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "io.h"

enum { MAXCONF = 64 };

static char *confname[MAXCONF];
static char *confval[MAXCONF];
static int nconf;

static int
findconf(char *k)
{
	int i;

	for(i = 0; i < nconf; i++)
		if(cistrcmp(confname[i], k) == 0)
			return i;
	return -1;
}

static void
addconf(char *k, char *v)
{
	int i;

	i = findconf(k);
	if(i < 0){
		if(nconf >= MAXCONF)
			return;
		i = nconf++;
		confname[i] = k;
	}
	confval[i] = v;
}

/*
 * plan9.ini is a text file U-Boot has put at BOOTARGS, in a zero-filled
 * buffer.  A missing file leaves the first byte 0.  No malloc here:
 * this runs before xinit; the name/value pointers point into the buffer.
 */
void
bootargsinit(void)
{
	char *cp, *line[MAXCONF], *p, *q, *v;
	int i, n;

	cp = BOOTARGS;
	cp[BOOTARGSLEN-1] = 0;
	if(*cp == 0 || (uchar)*cp >= 0x80)
		return;

	/* strip '\r' (file edited elsewhere), tab -> space */
	p = cp;
	for(q = cp; *q; q++){
		if(*q == '\r')
			continue;
		if(*q == '\t')
			*q = ' ';
		*p++ = *q;
	}
	*p = 0;

	n = getfields(cp, line, MAXCONF, 1, "\n");
	for(i = 0; i < n; i++){
		if(*line[i] == '#')
			continue;
		v = strchr(line[i], '=');
		if(v == nil)
			continue;
		*v++ = '\0';
		addconf(line[i], v);
	}
}

char*
getconf(char *name)
{
	int i;

	if((i = findconf(name)) < 0)
		return nil;
	return confval[i];
}

/* from init0, in process context: ksetenv needs up */
void
setconfenv(void)
{
	int i;

	for(i = 0; i < nconf; i++){
		if(confname[i][0] != '*')
			ksetenv(confname[i], confval[i], 0);
		ksetenv(confname[i], confval[i], 1);
	}
}
