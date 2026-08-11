#include "u.h"
#include "mem.h"

/*
 * TODO testing, KZERO stays == physical
 */

void satpset(uintptr);
void sfencevma(void);

void uart_puts(char*);
void uart_puthex64(unsigned long long);

void intrdisable(void);

enum {
	NTABLES = 12,	/* root + L1/L0 for the kernel region + L1/L0 for UART0 - generous */
};

static uintptr tablestore[(NTABLES+1)*512];	/* +1 spare page so a page-aligned base always fits */
static uintptr *tablebase;
static int ntable;

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
mapleaf(uintptr *root, uintptr va, uintptr pa, uintptr attr)
{
	uintptr *l1, *l0;
	int x2, x1, x0;

	x2 = PTLX(va, 2);
	if(!(root[x2] & PTEVALID))
		root[x2] = PAPTE((uintptr)newtable()) | PTEPTR;
	l1 = (uintptr*)PPN(root[x2]);

	x1 = PTLX(va, 1);
	if(!(l1[x1] & PTEVALID))
		l1[x1] = PAPTE((uintptr)newtable()) | PTEPTR;
	l0 = (uintptr*)PPN(l1[x1]);

	x0 = PTLX(va, 0);
	l0[x0] = PAPTE(pa) | attr;
}

static void
maprange(uintptr *root, uintptr base, uintptr size, uintptr attr)
{
	uintptr va;

	for(va = base; va < base+size; va += BY2PG)
		mapleaf(root, va, va, attr);
}

void
mmuinit(void)
{
	uintptr *root, satp;

	tablebase = (uintptr*)PGROUND((uintptr)tablestore);
	root = newtable();

	maprange(root, KZERO, 256*1024, PTELEAFMEM);

	maprange(root, 0x02000000, BY2PG, PTELEAFDEV);	/* PIO (GPIO) */
	maprange(root, 0x02050000, BY2PG, PTELEAFDEV);	/* timer */
	maprange(root, 0x02500000, BY2PG, PTELEAFDEV);	/* UART0 */
	maprange(root, 0x06011000, BY2PG, PTELEAFDEV);	/* riscv watchdog */
	maprange(root, 0x10000000, BY2PG, PTELEAFDEV);	/* PLIC priority */
	maprange(root, 0x10002000, BY2PG, PTELEAFDEV);	/* PLIC enable, context 1 */
	maprange(root, 0x10201000, BY2PG, PTELEAFDEV);	/* PLIC threshold/claim, context 1 */

	satp = MKSATP((uintptr)root);
	uart_puts("mmu: satp="); uart_puthex64(satp); uart_puts("\n");

    sfencevma();
	satpset(satp);
	uart_puts("mmu: sv39 enabled, still alive\n");
}
