#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "io.h"

enum {
	Ccu		= 0x02001000,
	PllCpuCtl	= 0x000/4,	/* ulong index */
	RiscvClk	= 0xd00/4,

	PllEn		= 1<<31,
	PllLockEn	= 1<<29,
	PllLock		= 1<<28,
	PllNshift	= 8,
	PllNmask	= 0xff<<8,

	Muxshift	= 24,
	Muxmask		= 7<<24,
	MuxPllPeri	= 4,		/* PLL_PERI(1X) - datasheet's bypass */
	MuxPllCpu	= 5,
};


/*
 * Change PLL_CPU fro default 408 MHz
 */
void
cpuclockinit(int mhz)
{
	ulong *ccu, v;
	int n;

	n = mhz/24;			/* PLL_CPU = 24MHz * N */
	if(n < 12 || n > 255)
		return;
	ccu = KADDR(Ccu);

	/* 1. run from PLL_PERI(1X) while PLL_CPU changes */
	ccu[RiscvClk] = ccu[RiscvClk] & ~Muxmask | MuxPllPeri<<Muxshift;
	coherence();
	microdelay(10);

	/* 2. set N */
	v = ccu[PllCpuCtl] & ~PllNmask | (n-1)<<PllNshift | PllEn;
	ccu[PllCpuCtl] = v;
	coherence();

	/* 3. LOCK_ENABLE low then high */
	ccu[PllCpuCtl] = v & ~PllLockEn;
	coherence();
	ccu[PllCpuCtl] = v | PllLockEn;
	coherence();

	/* 4. wait for lock */
	while((ccu[PllCpuCtl] & PllLock) == 0)
		;

	/* 5. back to PLL_CPU */
	ccu[RiscvClk] = ccu[RiscvClk] & ~Muxmask | MuxPllCpu<<Muxshift;
	coherence();
	microdelay(10);
}

