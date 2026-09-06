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


    PllPeriCtl	= 0x020/4,
	BusClkGate	= 0x84c/4,	/* mmc0/mmc1 gate and reset share this */
	Mmc0Clk		= 0x830/4,

	GateMmc0	= 1<<0,		/* clk_d1.c: CLK_BUS_MMC0 */
	ResetMmc0	= 1<<16,	/* clk_d1.c: RST_BUS_MMC0, 1 = released */

	MmcClkGate	= 1<<31,
	MmcMshift	= 0,		/* M: bits 3:0, divide by M+1 */
	MmcMmask	= 0xf,
	MmcPshift	= 8,		/* P: bits 9:8, divide by 1<<P */
	MmcPmask	= 3<<8,
	MmcMuxshift	= 24,		/* mux: bits 26:24 */
	MmcMuxmask	= 7<<24,
	MmcSrcHosc	= 0,
	MmcSrcPeri1x	= 1,
	MmcPostdiv	= 2,		/* fixed /2 after M and P */

	Hoscfreq	= 24*1000*1000,
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


/*
 * PLL_PERI(1X), computed rather than assumed:
 *   4x = 24MHz * (N+1) / (M+1)	reg 0x020, N bits 15:8, M bit 1
 *   2x = 4x / (P+1)		reg 0x020, P bits 18:16
 *   1x = 2x / 2		fixed factor
 */
ulong
peri1xfreq(void)
{
	ulong *ccu, v;
	int n, m, p;

	ccu = KADDR(Ccu);
	v = ccu[PllPeriCtl];
	n = ((v >> 8) & 0xff) + 1;
	m = ((v >> 1) & 1) + 1;
	p = ((v >> 16) & 7) + 1;
	return (24*1000*1000ULL * n) / m / p / 2;
}

/*
 * Bring MMC0 out of reset and enable its AHB clock. 
 */
void
mmc0enable(void)
{
    // Gate and reset share one register; the reset bit is released by writing 1.
	
    ulong *ccu;

	ccu = KADDR(Ccu);
	ccu[BusClkGate] &= ~ResetMmc0;		/* assert */
	coherence();
	microdelay(10);
	ccu[BusClkGate] |= ResetMmc0;		/* release */
	coherence();
	ccu[BusClkGate] |= GateMmc0;
	coherence();
	microdelay(10);
}

/*
 * Card clock. Rate = parent / (M+1) / (1<<P) / 2.
 * Identification runs at 400kHz from the 24MHz oscillator; 
 * anything above 12MHz needs PLL_PERI(1X).
 * The divider is rounded up, so the card is never clocked faster than asked.
 */
void
mmc0clock(int hz)
{
	ulong *ccu, v, freq;
	int m, p, div, src;

	if(hz <= 0)
		return;
	ccu = KADDR(Ccu);

	if(hz <= Hoscfreq/MmcPostdiv){
		src = MmcSrcHosc;
		freq = Hoscfreq;
	}else{
		src = MmcSrcPeri1x;
		freq = peri1xfreq();
	}

	div = freq / MmcPostdiv / hz;
	if(div < 1)
		div = 1;
	for(p = 0; p < 3 && div > (16<<p); p++)
		;
	m = (div + (1<<p) - 1) >> p;		/* round up */
	if(m > 16)
		m = 16;
	if(m < 1)
		m = 1;

	v = src<<MmcMuxshift | p<<MmcPshift | (m-1)<<MmcMshift;
	ccu[Mmc0Clk] = v;			/* gate off while changing */
	coherence();
	ccu[Mmc0Clk] = v | MmcClkGate;
	coherence();
	microdelay(10);
}

