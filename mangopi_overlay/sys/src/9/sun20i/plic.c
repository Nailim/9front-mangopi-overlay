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
