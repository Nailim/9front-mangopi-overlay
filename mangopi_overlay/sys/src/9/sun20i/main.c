#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "pool.h"
#include "io.h"

#include "tos.h"	// maybe remove later

void dummy(unsigned int);

void trapinit(void);
/* no mmuinit - the MMU switch now happens in l.s before main() */



// Defines to get port included and compiled
Conf	conf;
Image*	swapimage;
Uart*	consuart;


#define UART_LSR_THRE	(1<<5)

static volatile u32int *uart = (volatile u32int*)PHYSUART0;

void
uarthigh(void)
{
	uart = (volatile u32int*)KADDR(PHYSUART0);
}

void
uart_putc(int c)
{
	while((uart[5] & UART_LSR_THRE) == 0);	/* LSR at +0x14 */
	uart[0] = c & 0xFF;			            /* THR at +0x00 */
}

void uart_puts(char *s)
{
	while(*s){
		if(*s == '\n')
			uart_putc('\r');
		uart_putc(*s++);
	}
}

void uart_puthex64(unsigned long long v)
{
	static char digits[] = "0123456789abcdef";
	int i;

	uart_puts("0x");
	for(i = 60; i >= 0; i -= 4)
		uart_putc(digits[(v >> i) & 0xf]);
}

void
microdelay(int us)
{
	uintptr t0;

	t0 = rdtime();
	while(rdtime() - t0 < (uintptr)us * (TIMEBASEFREQ/1000000));
}

void
delay(int ms)
{
	while(--ms >= 0)
		microdelay(1000);
}



/* Actual kernel functions - todo remove commnet later */
void
uartputc(int c)
{
	uart_putc(c);
}

void
uartputs(char *s, int n)
{
	while(n-- > 0){
		if(*s == '\n')
			uart_putc('\r');
		uart_putc(*s++);
	}
}

void
confinit(void)
{
	uintptr pa, memsize;
	ulong kpages;
	int i;

	memsize = dramsize();
	print("dram: %lludMB\n", (uvlong)memsize/(1024*1024));

	conf.nmach = 1;

	pa = PADDR(PGROUND((uintptr)end));
	conf.mem[0].base = pa;
	conf.mem[0].npage = (PHYSDRAM + memsize - pa)/BY2PG;
	conf.mem[0].kbase = (uintptr)KADDR(pa);
	conf.mem[0].klimit = (uintptr)KADDR(PHYSDRAM + memsize);

	conf.npage = 0;
	for(i = 0; i < nelem(conf.mem); i++)
		conf.npage += conf.mem[i].npage;

	conf.upages = (conf.npage*80)/100;
	conf.ialloc = ((conf.npage - conf.upages)/2)*BY2PG;

	conf.nproc = 100 + ((conf.npage*BY2PG)/(1024*1024))*5;
	if(conf.nproc > 2000)
		conf.nproc = 2000;
	conf.nimage = 200;
	conf.copymode = 0;		/* copy on write */

	kpages = (conf.npage - conf.upages) * BY2PG;
	kpages -= conf.upages*sizeof(Page)
		+ conf.nproc*sizeof(Proc*)
		+ conf.nimage*sizeof(Image);
	mainmem->maxsize = kpages;
	imagmem->maxsize = kpages - (kpages/10);
}


static void
footask(void*)
{
	for(;;){
		print("%s: ticks %lud\n", up->text, m->ticks);
		tsleep(&up->sleep, return0, nil, 1000);
	}
}

static void
mmutask(void*)
{
	enum { Testva = 0x100000 };
	Page *pg;
	ulong *p;

	pg = newpage(0, nil);
	memset(KADDR(pg->pa), 0, BY2PG);

	putmmu(Testva, PPN(pg->pa) | PTEVALID | PTEWRITE, pg);

	p = (ulong*)Testva;
	*p = 0xcafebabe;
	print("mmutest: satp=%#p pa=%#p\n", up->satp, pg->pa);
	print("mmutest: via user va %#lux\n", *p);
	print("mmutest: via kzero   %#lux\n", *(ulong*)KADDR(pg->pa));

	flushmmu();
	print("mmutest: after flushmmu, kzero %#lux\n", *(ulong*)KADDR(pg->pa));

	print("mmutest: freecount before release %lud\n", palloc.freecount);
	mmurelease(up);
	print("mmutest: after mmurelease        %lud (want +3)\n", palloc.freecount);
	putpage(pg);
	print("mmutest: after putpage           %lud (want +4)\n", palloc.freecount);

	for(;;)
		tsleep(&up->sleep, return0, nil, 1000);
}

static ulong usercode[] = {
	// 0x00000073,	/* ecall */
	// 0x0000006f,	/* j . - spin, so the timer keeps trapping from U-mode */

	// 0x00600413,	/* addi x8, x0, 6	- ALARM, the syscall number */
	// 0x00000073,	/* ecall */
	// 0xff9ff06f,	/* j -8			- back to the addi, loop forever */

	0x00000413,	/* addi x8, x0, 0	- the argument */
	0x00813423,	/* sd   x8, 8(x2)	- 0(FP): where libc's stub spills it */
	0x00600413,	/* addi x8, x0, 6	- ALARM, the syscall number */
	0x00000073,	/* ecall */
	0xff1ff06f,	/* j -16		- back to the first addi */
};

static void
usertask(void*)
{
	KMap *k;
	Page *p;

	up->seg[SSEG] = newseg(SG_STACK | SG_NOEXEC, USTKTOP-USTKSIZE, USTKSIZE/BY2PG);
	up->seg[TSEG] = newseg(SG_TEXT | SG_RONLY, UTZERO, 1);
	up->seg[TSEG]->flushme = 1;

	p = newpage(UTZERO, nil);
	k = kmap(p);
	memset((uchar*)VA(k), 0, BY2PG);
	memmove((uchar*)VA(k)+32, usercode, sizeof usercode);	/* +32: where the a.out header would be */
	kunmap(k);
	segpage(up->seg[TSEG], p);

	up->kp = 0;
	up->noswap = 0;
	up->privatemem = 0;
	procpriority(up, PriNormal, 0);
	procsetup(up);

	flushmmu();

	print("usertask: entering user mode at %#p\n", (uintptr)UENTRY);
	touser(USTKTOP - sizeof(Tos) - 64);
}


void main(void)
{
	active.machs[m->machno] = 1;

	uart_puts("9sun20i: Plan9 riscv64 D1 kernel skeleton starting\n");

	uart_puts("main at ");
	uart_puthex64((unsigned long long)main);
	uart_puts("\n");

	uart_puts("m->machno = ");
	uart_puthex64((unsigned long long)m->machno);
	uart_puts("\n");

	trapinit();

	confinit();

	xinit();

	printinit();

	timersinit();

	initseg();
	links();
	chandevreset();
	pageinit();

	procinit0();

	// userinit();
	/* remove when initcode is in place and userinit can run */
	up = nil;
	kstrdup(&eve, "");

	kproc("footask1", footask, nil);
	kproc("footask2", footask, nil);
	print("mmutest: freecount before start %lud\n", palloc.freecount);
	kproc("mmutask", mmutask, nil);
	kproc("usertask", usertask, nil);
	schedinit();		/* never returns */


	while(1){
		wdt_riscv_feed();
		print("main ticks %lud\n", m->ticks);
		delay(1000);
	}
}
