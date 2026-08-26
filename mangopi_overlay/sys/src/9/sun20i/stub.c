#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "io.h"

/* ../port/devcons.c */
char*
getconf(char*)
{
	return nil;		/* no plan9.ini equivalent on this board yet */
}
int
isaconfig(char*, int, ISAConf*)
{
	return 0;
}

/* ../port/cache.c */
KMap*
kmap(Page*)
{
    panic("kmap");
}
void
kunmap(KMap*)
{
    panic("kunmap");
}
int
VA(KMap*)
{
    panic("VA");
}

/* ../port/portclock.c */
int
userureg(Ureg*)
{
    panic("userureg");
}

/* ../port/sysfile.c */
void
evenaddr(uintptr)
{
    panic("evenaddr");
}

/* ../port/sysproc.c */
void
procfork(Proc*)
{
    panic("procfork");
}
void
procsetup(Proc*)
{
    panic("procsetup");
}

/* ../port/proc.c */
void
cycles(uvlong *)
{
    panic("cycles");
}
void
procsave(Proc*)
{
    panic("procsave");
}
void
procrestore(Proc*)
{
    panic("procrestore");
}

/* kernel */
void
dumpstack(void)
{
    /* no stack trace yet - must not panic, panic() calls this */
    // panic("dumpstack");
}
void
exit(int)
{
    splhi();
	uart_puts("exit: halted\n");
	for(;;)
		idlehands();
}
void
rebootcmd(int, char**)
{
    panic("rebootcmd");
}
uvlong
fastticks(uvlong* hz)
{
    // panic("fastticks");
    if(hz != nil)
		*hz = TIMEBASEFREQ;
	return rdtime();
}
int
mntversion(Chan*, char*, int, int)
{
    panic("mntversion");
}
Chan*
mntauth(Chan*, char*)
{
    panic("mntauth");
}
void
srvrenameuser(char*, char*)
{
    panic("srvrenameuser");
}
void
shrrenameuser(char*, char*)
{
    panic("shrrenameuser");
}
void
mntrahinit(Mntrah *rah)
{
    panic("mntrahinit");
}
long
mntrahread(Mntrah *rah, Chan *c, uchar *buf, long len, vlong off)
{
    panic("mntrahread");
}
int
needpages(void*)
{
    panic("needpages");
}
void
muxclose(Mnt*)
{
    panic("muxclose");
}
ulong
µs(void)
{
    panic("µs");
}
void
putswap(Page*)
{
    panic("putswap");
}
int
swapcount(uintptr)
{
    panic("swapcount");
}
void
putmmu(uintptr, uintptr, Page*)
{
    panic("putmmu");
}
uintptr
userpc(void)
{
    panic("userpc");
}
void
checkmmu(uintptr, uintptr)
{
    panic("checkmmu");
}
void
kickpager(void)
{
    panic("kickpager");
}
void
flushmmu(void)
{
    panic("flushmmu");
}
void
mmurelease(Proc*)
{
    panic("mmurelease");
}
void
mmuswitch(Proc*)
{
    panic("mmuswitch");
}
void
closeegrp(Egrp*)
{
    panic("closeegrp");
}
uintptr
dbgpc(Proc*)
{
    panic("dbgpc");
}
void
kprocchild(Proc*, void (*)(void))
{
    panic("kprocchild");
}
void
dupswap(Page*)
{
    panic("dupswap");
}
Chan*
mntattach(Chan*, Chan*, char*, int)
{
    panic("mntattach");
}
Egrp*
newegrp(void)
{
    panic("newegrp");
}
void
envcpy(Egrp*, Egrp*)
{
    panic("envcpy");
}
void
forkchild(Proc*, Ureg*)
{
    panic("forkchild");
}
uintptr
execregs(uintptr, int, char**, Tos*)
{
    panic("execregs");
}
void
fpunotify(Proc*)
{
    panic("fpunotify");
}
void
fpunoted(Proc*)
{
    panic("fpunoted");
}
void
bootlinks(void)
{
    panic("bootlinks");
}
void	(*proctrace)(Proc*, int, vlong);    /* Delete this when 'proc' is added to the CONF file */
int
uartgetc(void)
{
    panic("uartgetc");
}
void
callwithureg(void(*)(Ureg*))
{
    panic("callwithureg");
}




