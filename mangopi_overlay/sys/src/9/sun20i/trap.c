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
void clockintr(Ureg*, void*);


void
trapinit(void)
{
	setstvec(trapvec);

    plicinit();
	timer0init(TICKINTERVAL);
	intrenable(TIMER0IRQ, clockintr, nil, 0, "clock");
	intrinit();
}


void
clockintr(Ureg *ureg, void*)
{
	wdt_riscv_feed();		/* the old feed loop went away with schedinit() */
	timer0ack();			/* dismiss the hardware */
	timerintr(ureg, 0);		/* portable clock: m->ticks, timer list, re-arm */
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


static char *excname[16] = {
[0]	"sys: trap: instruction address misaligned",
[1]	"sys: trap: instruction access fault",
[2]	"sys: trap: illegal instruction",
[3]	"sys: breakpoint",
[4]	"sys: trap: load address misaligned",
[5]	"sys: trap: load access fault",
[6]	"sys: trap: store/AMO address misaligned",
[7]	"sys: trap: store/AMO access fault",
[8]	"sys: trap: environment call from U-mode",
[9]	"sys: trap: environment call from S-mode",
[12]	"sys: trap: instruction page fault",
[13]	"sys: trap: load page fault",
[15]	"sys: trap: store/AMO page fault",
};

static char*
excstr(uintptr cause)
{
	char *s;

	if(cause < nelem(excname) && (s = excname[cause]) != nil)
		return s;
	return "sys: trap: unknown";
}



static void
dumptrap(Ureg *ureg, uintptr cause)
{
	uart_puts("trap: exception, cause ");
	uart_puthex64(cause);
	uart_puts(" (");
	uart_puts(excstr(cause));
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
	char buf[ERRMAX];

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
			preempted(plicintr(ureg) == TIMER0IRQ);
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
			if(user && up != nil){
				snprint(buf, sizeof buf, "%s pc=%#p tval=%#p",
					excstr(cause), ureg->pc, ureg->tval);
				postnote(up, 1, buf, NDebug);
				break;
			}
			dumptrap(ureg, cause);	/* kernel bug; never returns */
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


Ureg*
notify(Ureg *ureg, char *msg)
{
	Ureg *nureg;
	uintptr sp;

	sp = ureg->sp;
	sp -= 256;			/* leave the faulting context for debugging */
	sp -= sizeof(Ureg);

	if(!okaddr(sp-ERRMAX-4*BY2WD, sizeof(Ureg)+ERRMAX+4*BY2WD, 1)
	|| ((uintptr)up->notify & 1) != 0
	|| (sp & 7) != 0)
		return nil;

	nureg = (Ureg*)sp;
	memmove(nureg, ureg, sizeof(Ureg));

	sp -= BY2WD+ERRMAX;
	memmove((char*)sp, msg, ERRMAX);

	sp -= 3*BY2WD;
	*(uintptr*)(sp+2*BY2WD) = sp+3*BY2WD;	/* msg   -> 8(FP) */
	*(uintptr*)(sp+1*BY2WD) = (uintptr)nureg;	/* ureg  -> 0(FP) */
	ureg->arg = (uintptr)nureg;			/* also in R8 */
	ureg->sp = sp;
	ureg->pc = (uintptr)up->notify;
	ureg->r1 = 0;					/* link */

	return nureg;
}

int
noted(Ureg *ureg, Ureg *nureg, int arg0)
{
	uintptr oureg, sp;

	oureg = (uintptr)nureg;
	if((oureg & 7) != 0)
		return -1;

	setregisters(ureg, (char*)ureg, (char*)nureg, sizeof(Ureg));

	switch(arg0){
	case NCONT:
	case NRSTR:
		if(!okaddr(ureg->pc, BY2WD, 0)
		|| !okaddr(ureg->sp, BY2WD, 0)
		|| (ureg->pc & 1) != 0 || (ureg->sp & 7) != 0)
			return -1;
		break;

	case NSAVE:
		sp = oureg - 4*BY2WD - ERRMAX;
		if(!okaddr(ureg->pc, BY2WD, 0)
		|| !okaddr(sp, 4*BY2WD, 1)
		|| (ureg->pc & 1) != 0 || (sp & 7) != 0)
			return -1;
		ureg->sp = sp;
		ureg->arg = oureg;
		((uintptr*)sp)[1] = oureg;
		((uintptr*)sp)[0] = 0;
		break;
	}
	return 0;
}

