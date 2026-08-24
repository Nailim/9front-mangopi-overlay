#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "pool.h"
#include "io.h"

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
	while(n-- > 0)
		uart_putc(*s++);
}

void
confinit(void)
{
	uintptr pa, memsize;
	ulong kpages;
	int i;

	memsize = dramsize();
	print("dram: %lludMB\n", (uvlong)memsize/(1024*1024));
	// uart_puts("dram: ");
	// uart_puthex64((unsigned long long)memsize/(1024*1024));
	// uart_puts("\n");

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


void main(void)
{
	uart_puts("9sun20i: Plan9 riscv64 D1 kernel skeleton starting\n");

	uart_puts("main at ");
	uart_puthex64((unsigned long long)main);
	uart_puts("\n");

	uart_puts("m->machno = ");
	uart_puthex64((unsigned long long)m->machno);
	uart_puts("\n");

	trapinit();

	confinit();

	wdt_riscv_feed();
	while(1){
		delay(10000);
		wdt_riscv_feed();
	}
}
