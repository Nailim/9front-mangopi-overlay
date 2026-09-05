#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "io.h"

/*
 * D1 SOC SPECIFIC - RISC-V PLIC spec compliance.
 * Note: context 0 = hart0 M-mode, context 1 = hart0 S-mode
 * - this port only ever uses context 1.
 */

typedef struct Plicpriority Plicpriority;
struct Plicpriority
{
	ulong	pri[176];	/* pri[0] unused - IRQ 0 doesn't exist */
};

typedef struct Plicctl Plicctl;
struct Plicctl
{
	ulong	threshold;
	ulong	claim;		/* read = claim, write = complete */
};

typedef struct Vctl Vctl;
struct Vctl
{
	void	(*f)(Ureg*, void*);
	void	*a;
	char	*name;
};

static Vctl vctl[176];		/* same bound as the priority array */

void
intrenable(int irq, void (*f)(Ureg*, void*), void *a, int, char *name)
{
	if(irq <= 0 || irq >= nelem(vctl))
		panic("intrenable: irq %d out of range", irq);
	vctl[irq].f = f;
	vctl[irq].a = a;
	vctl[irq].name = name;
	plicenable(irq, 1);
}

int
plicintr(Ureg *ureg)
{
	Vctl *v;
	int irq;

	irq = plicclaim();
	if(irq > 0 && irq < nelem(vctl) && (v = &vctl[irq])->f != nil)
		v->f(ureg, v->a);
	else if(irq != 0)
		iprint("plic: unhandled irq %d\n", irq);
	pliccomplete(irq);
	return irq;
}



void
plicinit(void)
{
	Plicctl *ctl = (Plicctl*)KADDR(PHYSPLICCTL);

	ctl->threshold = 0;	/* accept any nonzero-priority IRQ */
}

void
plicenable(int irq, int priority)
{
	Plicpriority *pri = (Plicpriority*)KADDR(PHYSPLIC);
	ulong *en;

	pri->pri[irq] = priority;

	en = (ulong*)KADDR(PHYSPLICEN);
	en[irq/32] |= 1 << (irq%32);
}

int
plicclaim(void)
{
	Plicctl *ctl = (Plicctl*)KADDR(PHYSPLICCTL);

	return ctl->claim;
}

void
pliccomplete(int irq)
{
	Plicctl *ctl = (Plicctl*)KADDR(PHYSPLICCTL);

	ctl->claim = irq;
}
