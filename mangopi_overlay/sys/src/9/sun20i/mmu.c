#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "io.h"

/*
 * Sv39 page tables. Built while still running physically identity.
 * Every address stored in a PTE or table pointer here is a PA.
 */

void sfencevma(void);
void uart_puts(char*);
void uart_puthex64(unsigned long long);

extern void highstart(void);	/* l.s - first instruction that runs at a high VA */

enum {
	NTABLES = 24,	/* 19 needed: 1 root + 2 high kernel + 7 high MMIO + 2 identity kernel + 7 identity MMIO */
};

enum {
	Dram512	= 512*1024*1024,
	Probeoff = 0x100000,		/* 1MB into DRAM: above OpenSBI's
								 * reserved 0x40000000-0x4005ffff,
								 * below the kernel at 0x41000000 */
};

enum {
	Nuserslot = 256,	/* root slots 0-255 are the Sv39 low half */
};


static uintptr tablestore[(NTABLES+1)*512]; /* +1 spare page so a page-aligned base always fits */
static uintptr *tablebase;
static int ntable;
static uintptr *root;   /* PA of the root table */

static uintptr kernelsatp;

static uintptr*
newtable(void)
{
	uintptr *t;

    if(ntable >= NTABLES){
		uart_puts("mmu: out of page tables\n");
		for(;;);
	}

	t = tablebase + ntable*512;
	ntable++;

	return t;
}

static void
mapleaf(uintptr va, uintptr pa, uintptr attr)
{
	uintptr *l1, *l0;
	int x2, x1, x0;

	x2 = PTLX(va, 2);
	if(!(root[x2] & PTEVALID))
		root[x2] = PAPTE((uintptr)newtable()) | PTEPTR;
	l1 = (uintptr*)PTEPA(root[x2]);

	x1 = PTLX(va, 1);
	if(!(l1[x1] & PTEVALID))
		l1[x1] = PAPTE((uintptr)newtable()) | PTEPTR;
	l0 = (uintptr*)PTEPA(l1[x1]);

	x0 = PTLX(va, 0);
	l0[x0] = PAPTE(pa) | attr;
}

static void
maprange(uintptr va, uintptr pa, uintptr size, uintptr attr)
{
	uintptr o;

	for(o = 0; o < size; o += BY2PG)
		mapleaf(va+o, pa+o, attr);
}

static void
maphigh(uintptr pa, uintptr size, uintptr attr)
{
	maprange((uintptr)KADDR(pa), pa, size, attr);
}

static void
mapident(uintptr pa, uintptr size, uintptr attr)
{
	maprange(pa, pa, size, attr);
}

/*
 * Map a range with level-1 (2MB) leaves rather than 4KB pages.
 * Used only for the kernel's linear map of DRAM.
 * User mappings come from putmmu() with 4KB leaves.
 */
static void
mapblock(uintptr va, uintptr pa, uintptr size, uintptr attr)
{
	uintptr *l1, o;
	int x2, x1;

	for(o = 0; o < size; o += PGLSZ(1)){		/* PGLSZ(1) == 2MB */
		x2 = PTLX(va+o, 2);
		if(!(root[x2] & PTEVALID))
			root[x2] = PAPTE((uintptr)newtable()) | PTEPTR;
		l1 = (uintptr*)PTEPA(root[x2]);
		x1 = PTLX(va+o, 1);
		l1[x1] = PAPTE(pa+o) | attr;
	}
}

/*
 * Build the tables and hand back the satp value.
 * Does NOT write satp - l.s does that, so the PA->VA transition happens
 * at a defined point in assembly rather than deep in a C call chain.
 */
uintptr
mmubootstrap(void)
{
    uintptr satp;

	tablebase = (uintptr*)PGROUND((uintptr)tablestore);
	root = newtable();

	/* the permanent high-half mapping */
	mapblock((uintptr)KADDR(PHYSDRAM), PHYSDRAM, DRAMMAX, PTELEAFMEM);	/* kernel text/data/bss/stack/tables */
	maphigh(PHYSPIO, PHYSPIOSIZE, PTELEAFDEV);	/* PIO, CCU, timer, wdt, UARTs, i2c, ... */
	maphigh(PHYSWDTRISCV, BY2PG, PTELEAFDEV);	/* riscv watchdog */
	maphigh(PHYSPLIC, BY2PG, PTELEAFDEV);		/* PLIC priority */
	maphigh(PHYSPLICEN, BY2PG, PTELEAFDEV);     /* PLIC enable, context 1 */
	maphigh(PHYSPLICCTL, BY2PG, PTELEAFDEV);	/* PLIC threshold/claim, context 1 */

	/* identity: mirrors for the trampoline */
	mapblock(PHYSDRAM, PHYSDRAM, DRAMMAX, PTELEAFMEM);
	mapident(PHYSPIO, PHYSPIOSIZE, PTELEAFDEV);
	mapident(PHYSWDTRISCV, BY2PG, PTELEAFDEV);
	mapident(PHYSPLIC, BY2PG, PTELEAFDEV);
	mapident(PHYSPLICEN, BY2PG, PTELEAFDEV);
	mapident(PHYSPLICCTL, BY2PG, PTELEAFDEV);

    satp = MKSATP((uintptr)root);
    uart_puts("mmu: satp="); uart_puthex64(satp); uart_puts("\n");  // debug output

	kernelsatp = satp;

	return satp;
}

/*
 * Virtual address of l.s's highstart.
 */
void*
mmuhighentry(void)
{
    void *va;

	va = KADDR((uintptr)highstart);

    uart_puts("mmu: highstart="); uart_puthex64((uintptr)va); uart_puts("\n");  // debug output
	
    return va;
}

/*
 * Called once we're running high: retire the identity mapping so the low
 * half is free for user space later.
 */
void
mmulowdrop(void)
{
	uintptr *r;

	r = KADDR((uintptr)root);	/* root holds a PA; we run high now */
	r[PTLX(PHYSTEXT, 2)] = 0;
	r[PTLX(PHYSPIO, 2)] = 0;
	sfencevma();
    uart_puts("mmu: identity map dropped\n");   // debug output
}


/*
 * D1 boards ship with either 512MB or 1GB, detect it: 
 * on a 512MB part the DDR controller aliases, so a write 512MB up lands on the same cell as the one below.
 * Probes 1MB into DRAM - below the kernel at PHYSTEXT, and above
 * OpenSBI's PMP-protected 0x40000000-0x4005ffff, which would fault.
 */
uintptr
dramsize(void)
{
	ulong *lo, *hi, save;
	uintptr size;

	lo = KADDR(PHYSDRAM + Probeoff);
	hi = KADDR(PHYSDRAM + Probeoff + Dram512);

	save = *lo;
	*lo = 0x55aa55aa;
	*hi = 0xaa55aa55;
	coherence();
	size = (*lo == 0x55aa55aa)? 2*Dram512: Dram512;
	*lo = save;
	coherence();

	return size;
}

/*
 * Return the number of bytes that can be accessed via KADDR(pa).
 * If pa is not a valid argument to KADDR, return 0.
 */
uintptr
cankaddr(uintptr pa)
{
	// if(pa < (uintptr)-KZERO)
	// 	return -KZERO - pa;
	// return 0;
	if(pa >= PHYSDRAM && pa < PHYSDRAM+DRAMMAX)
		return PHYSDRAM+DRAMMAX - pa;
	return 0;
}


static uintptr*
kernelroot(void)
{
	return KADDR((uintptr)root);	/* root holds a PA; we run high now */
}


/*
 * Give p its own root table: user half empty, kernel half copied from
 * the boot root. The copy is safe only because mmubootstrap() creates
 * every kernel level-2 entry up front and nothing adds more later.
 */
static void
mmurootalloc(Proc *p)
{
	uintptr *r;
	Page *pg;

	pg = newpage(0, nil);
	r = KADDR(pg->pa);
	memset(r, 0, Nuserslot*BY2WD);
	memmove(&r[Nuserslot], &kernelroot()[Nuserslot], BY2PG - Nuserslot*BY2WD);
	coherence();
	p->mmuroot = pg;
	p->satp = MKSATP(pg->pa);
}

/*
 * PTE for va at the given level in up's tables, allocating intermediate
 * tables from up->mmufree. nil when that list is empty; the caller
 * refills it and retries.
 */
static uintptr*
mmuwalk(uintptr va, int level)
{
	uintptr *table, pte;
	Page *pg;
	int i, x;

	table = KADDR(up->mmuroot->pa);
	for(i = PTLEVELS-1; i > level; i--){
		x = PTLX(va, i);
		pte = table[x];
		if(pte & PTEVALID){
			table = KADDR(PTEPA(pte));
			continue;
		}
		pg = up->mmufree;
		if(pg == nil)
			return nil;
		up->mmufree = pg->next;
		pg->next = up->mmuused;
		up->mmuused = pg;
		memset(KADDR(pg->pa), 0, BY2PG);
		coherence();
		table[x] = PAPTE(pg->pa) | PTEPTR;
		table = KADDR(pg->pa);
	}
	return &table[PTLX(va, level)];
}

/*
 * Recycle p's table pages onto its free list and empty the user half of
 * its root - the pager has invalidated its mappings, or it is exiting.
 */
static void
mmufree(Proc *p)
{
	Page *pg, *next;

	if(p->mmuroot != nil)
		memset(KADDR(p->mmuroot->pa), 0, Nuserslot*BY2WD);
	for(pg = p->mmuused; pg != nil; pg = next){
		next = pg->next;
		pg->next = p->mmufree;
		p->mmufree = pg;
	}
	p->mmuused = nil;
}

void
putmmu(uintptr va, uintptr pa, Page *pg)
{
	uintptr *pte, attr;
	Page *new;
	int s;

	if(up->mmuroot == nil){
		mmurootalloc(up);
		s = splhi();
		satpset(up->satp);	/* sched may not run before we return to user */
		splx(s);
	}

	s = splhi();
	while((pte = mmuwalk(va, 0)) == nil){
		spllo();
		new = newpage(0, nil);
		splhi();
		new->next = up->mmufree;
		up->mmufree = new;
	}

	/*
	 * PTERONLY and PTECACHED are 0, so they are tested by their opposites.
	 * R is always present: W without R is a reserved encoding. 
	 * A and D are always present: C906 has no hardware A/D
	 * update, and a leaf without them faults on every access
	 */
	attr = PTEVALID|PTEREAD|PTEUSER|PTEACCESSED|PTEDIRTY;
	if(pa & PTEWRITE)
		attr |= PTEWRITE;
	if((pa & PTENOEXEC) == 0)
		attr |= PTEEXEC;
	if(pa & (PTEUNCACHED|PTEDEVICE))
		attr |= PTE_THEAD_SO;
	else
		attr |= PTE_THEAD_C|PTE_THEAD_SH;

	*pte = PAPTE(PPN(pa)) | attr;
	coherence();
	sfencevma();

	if(needtxtflush(pg)){
		fencei();
		donetxtflush(pg);
	}
	splx(s);
}
void
checkmmu(uintptr, uintptr)
{
    /* nothing for now */
}
void
flushmmu(void)
{
    int s;

	s = splhi();
	up->newtlb = 1;
	mmuswitch(up);
	splx(s);
}
void
mmurelease(Proc* p)
{
    mmuswitch(nil);
	mmufree(p);
	freepages(p->mmufree, nil, 0);
	p->mmufree = nil;
	if(p->mmuroot != nil){
		putpage(p->mmuroot);
		p->mmuroot = nil;
	}
	p->satp = 0;
}
void
mmuswitch(Proc* p)
{
    if(p == nil || p->mmuroot == nil){
		satpset(kernelsatp);	/* kernel proc: the boot tables suffice */
		return;
	}
	if(p->newtlb){
		mmufree(p);
		p->newtlb = 0;
	}
	satpset(p->satp);
}
