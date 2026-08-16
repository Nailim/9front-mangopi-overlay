typedef struct Conf	    Conf;
typedef struct Confmem	Confmem;
typedef struct FPsave	FPsave;
typedef struct PFPU	    PFPU;
typedef struct ISAConf	ISAConf;
typedef struct Label	Label;
typedef struct Mach	    Mach;
typedef struct Page	    Page;
typedef struct PMMU	    PMMU;
typedef struct Proc	    Proc;
typedef struct Ureg	    Ureg;
typedef uvlong		    Tval;
typedef void		    KMap;		/* KZERO maps all of physical - KMap is just KADDR */

#pragma incomplete Ureg

#define MAXSYSARG	5		    /* for mount(fd, mpt, flag, arg, srv) */
#define AOUT_MAGIC	(Y_MAGIC)	/* riscv64 */

struct Label
{
	uintptr	sp;
	uintptr	pc;
};

/*
 * rv64imafdc - F and D extensions present, so 32 double-precision registers plus fcsr.
 * Nothing saves/restores these yet; TODO for latr.
 */
struct FPsave
{
	uvlong	regs[32];
	ulong	fcsr;
};

struct PFPU
{
	int	fpstate;
	FPsave	fpsave[1];
};

enum
{
	FPinit,
	FPactive,
	FPinactive,
	FPnotify = 0x100
};

struct Confmem
{
	uintptr	base;
	ulong	npage;
	uintptr	kbase;
	uintptr	klimit;
};

struct Conf
{
	ulong	nmach;		/* processors */
	ulong	nproc;		/* processes */
	Confmem	mem[1];		/* physical memory - D1 has one DRAM bank */
	ulong	npage;		/* total physical pages of memory */
	ulong	upages;		/* user page pool */
	ulong	copymode;	/* 0 is copy on write, 1 is copy on reference */
	ulong	ialloc;		/* max interrupt time allocation in bytes */
	ulong	pipeqsize;	/* size in bytes of pipe queues */
	ulong	nimage;		/* number of page cache image headers */
	ulong	nswap;		/* number of swap pages */
	int	nswppo;		    /* max # of pageouts per segment pass */
	int	monitor;
};

/*
 *  MMU state carried per process.
 */
#define NCOLOR	1

struct PMMU
{
	uintptr	satp;
	Page	*mmuused, *mmufree;
};

#include "../port/portdat.h"

struct Mach
{
	int	machno;			/* physical id of processor */
	uintptr	splpc;		/* pc of last caller to splhi */
	Proc*	proc;		/* current process */
	/* end of known offsets to assembly */

	PMach;

	int	cpumhz;
	uvlong	cpuhz;		/* speed of cpu */

	uintptr	stack[1];
};

extern Mach mach0;
#define MACHP(n)	(&mach0)	/* MAXMACH == 1 */

#define NISAOPT		8
struct ISAConf
{
	char	*type;
	ulong	port;
	int	irq;
	int	nopt;
	char	*opt[NISAOPT];
};
#define BUSUNKNOWN -1

struct
{
	char	machs[MAXMACH];	/* active CPUs */
	int exiting;	        /* shutdown */
}active;

extern register Mach* m;	/* R7 */
extern register Proc* up;	/* R6 */
