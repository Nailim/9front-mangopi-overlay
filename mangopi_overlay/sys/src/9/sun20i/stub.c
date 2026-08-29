#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "io.h"
#include "../port/error.h"

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


/* ../port/sysfile.c */
void
evenaddr(uintptr addr)
{
    if(addr & 3){
		postnote(up, 1, "sys: odd address", NDebug);
		error(Ebadarg);
	}
}

/* ../port/sysproc.c */
void
procfork(Proc*)
{
    /* replace later */
}
void
procsetup(Proc* p)
{
    p->fpstate = FPinit;    // update later
}

/* ../port/proc.c */
void
procsave(Proc*)
{
    /* no FP state in use - replace later */
}
void
procrestore(Proc*)
{
    /* nothing touches F/D registers yet - replace later */
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
kickpager(void)
{
    panic("kickpager");
}
void
closeegrp(Egrp*)
{
    panic("closeegrp");
}

void
kprocchild(Proc *p, void (*entry)(void))
{
    p->sched.pc = (uintptr)entry;
	p->sched.sp = (uintptr)p - 16;		/* 16 clear bytes: a callee may spill its first arg to sp+8 */
	*(void**)p->sched.sp = kprocchild;	/* fake saved pc, for getcallerpc */
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
	/*
	 * No bootdir section in the CONF file yet, so no boot filesystem to register.
	 */
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







