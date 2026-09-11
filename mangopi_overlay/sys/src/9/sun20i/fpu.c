#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"

/*
 * NOTE: jc keeps four floating-point constants permanently in registers -
 * i.out.h names them FREGZERO(28), FREGHALF(29), FREGONE(30) and
 * FREGTWO(31) - and synthesises small constants by arithmetic on them
 * (2.5 is FREGTWO+FREGHALF, -1.0 is FREGZERO-FREGONE). Every process
 * must start with those four loaded or such constants come out NaN.
 */

static FPsave initfp;

void
fpuinit(void)
{
	memset(&initfp, 0, sizeof initfp);
	initfp.regs[28] = 0x0000000000000000ULL;	/* 0.0 */
	initfp.regs[29] = 0x3FE0000000000000ULL;	/* 0.5 */
	initfp.regs[30] = 0x3FF0000000000000ULL;	/* 1.0 */
	initfp.regs[31] = 0x4000000000000000ULL;	/* 2.0 */
	initfp.fcsr = 0;
}

void
procsetup(Proc *p)
{
	p->fpstate = FPinit;
}

/*
 * The kernel never uses floating point, so state only has to survive a
 * context switch - not every trap. These are the ../port/proc.c hooks.
 */
void
procsave(Proc *p)
{
	if((p->fpstate & ~FPnotify) != FPactive)
		return;
	if(p->state == Moribund){
		p->fpstate = FPinit;
		return;
	}
	fpsetfs(SSTATUS_FS_CLEAN);	/* stores below need FS != Off */
	fpsave(p->fpsave);
	p->fpstate = FPinactive | (p->fpstate & FPnotify);
}

void
procrestore(Proc *p)
{
	switch(p->fpstate & ~FPnotify){
	case FPinit:
		fpsetfs(SSTATUS_FS_CLEAN);
		fprestore(&initfp);
		break;
	case FPinactive:
		fpsetfs(SSTATUS_FS_CLEAN);
		fprestore(p->fpsave);
		break;
	default:
		return;
	}
	p->fpstate = FPactive | (p->fpstate & FPnotify);
}

void
fpunotify(Proc *p)
{
	if(p->fpstate == FPactive){
		fpsetfs(SSTATUS_FS_CLEAN);
		fpsave(p->fpsave);
		p->fpstate = FPinactive;
	}
	p->fpstate |= FPnotify;
}

void
fpunoted(Proc *p)
{
	p->fpstate &= ~FPnotify;
}

FPsave*
notefpsave(Proc*)
{
	return nil;
}
