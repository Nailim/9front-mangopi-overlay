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


void
init0(void)
{
	char buf[2*KNAMELEN], **sp;

	uart_puts("init0: entered\n");
	chandevinit();
	uart_puts("init0: chandevinit done\n");

	if(!waserror()){
		snprint(buf, sizeof buf, "riscv64 %s", conffile);
		ksetenv("terminal", buf, 0);
		ksetenv("cputype", "riscv64", 0);
		if(cpuserver)
			ksetenv("service", "cpu", 0);
		else
			ksetenv("service", "terminal", 0);
		poperror();
	}
	kproc("alarm", alarmkproc, 0);
	uart_puts("init0: alarm kproc started\n");

	/*
	 * initcode's stack. 0(SP) is the callee's saved-LINK slot and
	 * arguments start at 0(FP) = SP+BY2WD, so startboot's second
	 * argument, argv, is read from SP+2*BY2WD.
	 */
	sp = (char**)(USTKTOP - sizeof(Tos) - 8 - sizeof(sp[0])*6);
	sp[0] = nil;			/* saved LINK */
	sp[1] = nil;			/* arg0 slot; init9.s also sets R8 */
	sp[2] = (char*)&sp[3];		/* argv */
	sp[3] = (char*)&sp[5];		/* argv[0] */
	sp[4] = nil;			/* argv terminator */
	strcpy((char*)&sp[5], "boot");

	uart_puts("init0: touser sp=");
	uart_puthex64((uintptr)sp);
	uart_puts("\n");
	splhi();

	{
		ulong *t;
		int i;

		t = (ulong*)UTZERO;		/* SUM is set; the read faults the page in */
		uart_puts("code");
		for(i = 8; i < 14; i++){	/* t[8] is offset 32 = UENTRY */
			uart_puts(" ");
			uart_puthex64(t[i]);
		}
		uart_puts("\n");
	}

	touser((uintptr)sp);
}

// static void
// pstask(void*)
// {
// 	Proc *p;
// 	int i;

// 	for(;;){
// 		tsleep(&up->sleep, return0, nil, 10000);
// 		for(i = 0; (p = proctab(i)) != nil; i++){
// 			if(p->state == Dead)
// 				continue;
// 			print("ps: %lud %s %s %s\n", p->pid, p->text,
// 				statename[p->state], p->psstate != nil? p->psstate: "");
// 		}
// 		print("ps: ---\n");
// 	}
// }


void main(void)
{
	active.machs[m->machno] = 1;

	uartconsinit();

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

	quotefmtinstall();

	timersinit();

	initseg();
	links();
	chandevreset();
	pageinit();

	procinit0();

	userinit();

	// kproc("ps", pstask, nil);
	schedinit();		/* never returns */


	while(1){
		wdt_riscv_feed();
		print("main ticks %lud\n", m->ticks);
		delay(1000);
	}
}


void
setupwatchpts(Proc *, Watchpt *, int n)
{
	if(n > 0)
		error("no watchpoints");
}

