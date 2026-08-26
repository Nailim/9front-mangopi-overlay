/*
 * traps, exceptions, interrupts, system calls.
 */

#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "io.h"
#include "ureg.h"

// for now to get things together
void uart_puts(char*);
void uart_puthex64(unsigned long long);


extern void trapvec(void);
void setstvec(void*);
void intrenable(void);


void
trapinit(void)
{
	setstvec(trapvec);

    plicinit();
	timer0init(TICKINTERVAL);
	intrenable();
}


static void
clockintr(Ureg *ureg)
{
	timer0ack();			/* dismiss the hardware */
	timerintr(ureg, 0);		/* portable clock: m->ticks, timer list, re-arm */
}

static void
plicintr(Ureg *ureg)
{
	int irq;

	irq = plicclaim();

	switch(irq){
	case TIMER0IRQ:
		clockintr(ureg);
		break;
	default:
		iprint("trap: unhandled PLIC irq %d\n", irq);
		break;
	}
	pliccomplete(irq);
}


void
trap(Ureg *ureg)
{
	uintptr cause;

	cause = ureg->cause;

	/* interrupt switch - bit 63 set */
    if(cause & ((uintptr)1<<63)){
		switch(cause & ~((uintptr)1<<63)){
		case 9:
			plicintr(ureg);
			return;
		default:
			uart_puts("trap: unhandled interrupt, code ");
			uart_puthex64(cause & ~((uintptr)1<<63));
			uart_puts("\n");
			uart_puts("trap: halting\n");
			for(;;);
		}
	}

	/* exception switch - bit 63 clear */
    uart_puts("trap: exception, cause ");
	uart_puthex64(cause);
	uart_puts(" (");
	switch(cause){
	case 0:	uart_puts("instruction address misaligned"); break;
	case 1:	uart_puts("instruction access fault"); break;
	case 2:	uart_puts("illegal instruction"); break;
	case 3:	uart_puts("breakpoint"); break;
	case 4:	uart_puts("load address misaligned"); break;
	case 5:	uart_puts("load access fault"); break;
	case 6:	uart_puts("store/AMO address misaligned"); break;
	case 7:	uart_puts("store/AMO access fault"); break;
	case 8:	uart_puts("environment call from U-mode"); break;
	case 9:	plicintr(ureg); return;
	case 12:	uart_puts("instruction page fault"); break;
	case 13:	uart_puts("load page fault"); break;
	case 15:	uart_puts("store/AMO page fault"); break;
	default:	uart_puts("unknown"); break;
	}
	uart_puts(")\n");

	uart_puts("  sepc    = "); uart_puthex64(ureg->pc); uart_puts("\n");
	uart_puts("  stval   = "); uart_puthex64(ureg->tval); uart_puts("\n");
	uart_puts("  sstatus = "); uart_puthex64(ureg->status); uart_puts("\n");

    // not ment to be usefull yet, just testing
    if(cause == 3){
		uart_puts("trap: returning\n");
        ureg->pc += 4;  // skip past the ebreak - sepc points AT it, not after it
		return;
	}

	uart_puts("trap: halting\n");
	for(;;);
}
