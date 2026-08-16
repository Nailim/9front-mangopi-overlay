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

static uintptr tablestore[(NTABLES+1)*512]; /* +1 spare page so a page-aligned base always fits */
static uintptr *tablebase;
static int ntable;
static uintptr *root;   /* PA of the root table */

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
	maphigh(PHYSTEXT, 512*1024, PTELEAFMEM);	/* kernel text/data/bss/stack/tables */
	maphigh(PHYSPIO, PHYSPIOSIZE, PTELEAFDEV);	/* PIO, CCU, timer, wdt, UARTs, i2c, ... */
	maphigh(PHYSWDTRISCV, BY2PG, PTELEAFDEV);	/* riscv watchdog */
	maphigh(PHYSPLIC, BY2PG, PTELEAFDEV);		/* PLIC priority */
	maphigh(PHYSPLICEN, BY2PG, PTELEAFDEV);     /* PLIC enable, context 1 */
	maphigh(PHYSPLICCTL, BY2PG, PTELEAFDEV);	/* PLIC threshold/claim, context 1 */

	/* identity: mirrors the high mapping so drivers can keep using
	 * physical addresses untill jump to highstart. */
	mapident(PHYSTEXT, 512*1024, PTELEAFMEM);
	mapident(PHYSPIO, PHYSPIOSIZE, PTELEAFDEV);
	mapident(PHYSWDTRISCV, BY2PG, PTELEAFDEV);
	mapident(PHYSPLIC, BY2PG, PTELEAFDEV);
	mapident(PHYSPLICEN, BY2PG, PTELEAFDEV);
	mapident(PHYSPLICCTL, BY2PG, PTELEAFDEV);

    satp = MKSATP((uintptr)root);
    uart_puts("mmu: satp="); uart_puthex64(satp); uart_puts("\n");  // debug output

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
