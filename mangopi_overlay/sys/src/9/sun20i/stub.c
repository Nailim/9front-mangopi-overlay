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
    /* nothing cached to flush -replace later */
}
void
mmurelease(Proc*)
{
    /* no per-process tables to free yet - replace later */
}
void
mmuswitch(Proc* p)
{
    USED(p);
	/*
	 * Kernel processes only. They share the one page table built by mmubootstrap, so there is nothing to install.
     * Replaces later with satpset(p->satp) + sfencevma() once putmmu builds real per-process tables.
	 */
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




