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
needpages(void*)
{
    panic("needpages");
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


void	(*proctrace)(Proc*, int, vlong);    /* Delete this when 'proc' is added to the CONF file */

void
callwithureg(void(*)(Ureg*))
{
    panic("callwithureg");
}


FPsave*
notefpsave(Proc*)
{
	return nil;		/* no FP state carried across notes yet */
}
void
fpunotify(Proc *p)
{
    p->fpstate |= FPnotify;
}
void
fpunoted(Proc *p)
{
    p->fpstate &= ~FPnotify;
}


