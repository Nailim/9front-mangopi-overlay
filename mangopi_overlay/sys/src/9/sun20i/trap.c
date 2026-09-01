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
	wdt_riscv_feed();		/* the old feed loop went away with schedinit() */
	uartpoll();				/* handle uart for now */
	timer0ack();			/* dismiss the hardware */
	timerintr(ureg, 0);		/* portable clock: m->ticks, timer list, re-arm */
}

static int
plicintr(Ureg *ureg)
{
	int irq, clock;

	irq = plicclaim();
	clock = 0;
	switch(irq){
	case TIMER0IRQ:
		clockintr(ureg);
		clock = 1;
		break;
	default:
		iprint("trap: unhandled PLIC irq %d\n", irq);
		break;
	}
	pliccomplete(irq);
	return clock;
}


static void
faultriscv(Ureg *ureg, int read)
{
	uintptr addr;
	int user, insyscall;
	char buf[ERRMAX];

	addr = ureg->tval;
	user = userureg(ureg);
	if(up == nil)
		panic("fault before process context: addr=%#p pc=%#p", addr, ureg->pc);
	if(!user){
		if(addr >= USTKTOP)
			panic("kernel fault: addr=%#p pc=%#p", addr, ureg->pc);
		if(up->nlocks)
			panic("fault holding locks: addr=%#p pc=%#p", addr, ureg->pc);
	}
	insyscall = up->insyscall;
	up->insyscall = 1;
	if(fault(addr, ureg->pc, read) < 0){
		if(!user)
			panic("fault: %s addr=%#p pc=%#p", read? "read": "write", addr, ureg->pc);
		snprint(buf, sizeof buf, "sys: trap: fault %s addr=%#p",
			read? "read": "write", addr);
		postnote(up, 1, buf, NDebug);
	}
	up->insyscall = insyscall;
}


static void
dumptrap(Ureg *ureg, uintptr cause)
{
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
	case 7:	uart_puts("store/AMO address misaligned"); break;
	case 8:	uart_puts("environment call from U-mode"); break;
	case 9:	uart_puts("environment call from S-mode"); break;
	case 12:	uart_puts("instruction page fault"); break;
	case 13:	uart_puts("load page fault"); break;
	case 15:	uart_puts("store/AMO page fault"); break;
	default:	uart_puts("unknown"); break;
	}
	uart_puts(")\n");

	uart_puts("  sepc    = "); uart_puthex64(ureg->pc); uart_puts("\n");
	uart_puts("  stval   = "); uart_puthex64(ureg->tval); uart_puts("\n");
	uart_puts("  sstatus = "); uart_puthex64(ureg->status); uart_puts("\n");

	uart_puts("trap: halting\n");
	for(;;);
}


void
trap(Ureg *ureg)
{
	uintptr cause;
	int user;

	cause = ureg->cause;

	if(cause == 8){			/* environment call from U-mode */
		syscall(ureg);
		return;
	}

	user = userureg(ureg);

	if(user)
		kenter(ureg);		/* user branch only */


	/* Handled exceptions - bit 63 clear */
	if(cause & ((uintptr)1<<63)){
		switch(cause & ~((uintptr)1<<63)){
		case 9:
			preempted(plicintr(ureg));
			break;
		default:
			uart_puts("trap: unhandled interrupt, code ");
			uart_puthex64(cause & ~((uintptr)1<<63));
			uart_puts("\ntrap: halting\n");
			for(;;);
		}
	}else{
		switch(cause){
		case 12:	/* instruction page fault */
		case 13:	/* load page fault */
			faultriscv(ureg, 1);
			break;
		case 15:	/* store/AMO page fault */
			faultriscv(ureg, 0);
			break;
		default:
			dumptrap(ureg, cause);
		}
	}

	splhi();

	if(user){
		if(up->procctl || up->nnote)
			donotify(ureg);
		kexit(ureg);
	}
}


void
syscall(Ureg *ureg)
{
	ulong scallnr;

	if(!kenter(ureg))
		panic("syscall from kernel: pc=%#p", ureg->pc);

	/*
	 * sepc points AT the ecall, not past it - unlike ARM, where the
	 * hardware has already advanced. Before dosyscall: a note delivered
	 * while the syscall sleeps would otherwise resume by re-executing it.
	 */
	ureg->pc += 4;

	scallnr = ureg->arg;			/* R8 */

	dosyscall(scallnr, (Sargs*)(ureg->sp + BY2WD), &ureg->ret);

	if(up->procctl || up->nnote)
		donotify(ureg);
	if(up->delaysched)
		sched();
	
	kexit(ureg);
}

uintptr
userpc(void)
{
    return up->dbgreg->pc;
}

uintptr
dbgpc(Proc*)
{
    Ureg *ur;

	ur = up->dbgreg;
	if(ur == nil)
		return 0;
	return ur->pc;
}

void
setkernur(Ureg *ureg, Proc *p)
{
	ureg->pc = p->sched.pc;
	ureg->sp = p->sched.sp + BY2WD;
	ureg->r1 = (uintptr)sched;	/* link */
}

/*
 * A debugger writing /proc/n/regs must not be able to set sstatus -
 * SPP there would return to S-mode with user-controlled registers.
 */
void
setregisters(Ureg *ureg, char *pureg, char *uva, int n)
{
	uintptr status;

	status = ureg->status;
	memmove(pureg, uva, n);
	ureg->status = status;
}


uintptr
execregs(uintptr entry, int argc, char *argv[], Tos *tos)
{
	uintptr *sp = (void*)argv;
	Ureg *ureg;

	*--sp = argc;			/* SP+0 = argc, SP+8 = argv[0] */

	ureg = up->dbgreg;
	ureg->sp = (uintptr)sp;
	ureg->pc = entry;
	ureg->r1 = 0;			/* link */

	return (uintptr)tos;		/* arrives in R8 for _main */
}

void
forkchild(Proc *p, Ureg *ureg)
{
	Ureg *cureg;

	p->sched.pc = (uintptr)forkret;
	p->sched.sp = (uintptr)p - sizeof(Ureg);

	cureg = (Ureg*)p->sched.sp;
	memmove(cureg, ureg, sizeof(Ureg));
	cureg->ret = 0;			/* child's fork() returns 0 */
}

