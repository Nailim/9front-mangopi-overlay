#include "u.h"
#include "plic.h"

/*
 * D1 SOC SPECIFIC - interrupt-controller@10000000 in sun20i-d1.dtsi,
 * "allwinner,sun20i-d1-plic"/"thead,c900-plic" compatible - claims
 * RISC-V PLIC spec compliance, so the standard memory map applies.
 * riscv,ndev=176 real interrupt sources. Context ordering confirmed
 * from the dtsi's own interrupts-extended = <&cpu0_intc 11>, <&cpu0_intc
 * 9> (parent IRQ 11 = M-mode external, 9 = S-mode external), not
 * assumed: context 0 = hart0 M-mode, context 1 = hart0 S-mode - this
 * port only ever uses context 1.
 */
enum {
	PlicBase	= 0x10000000,
	Context		= 1,	/* hart0 S-mode - the only context this port uses */

	EnableBase	= PlicBase + 0x002000 + 0x80*Context,
	CtlBase		= PlicBase + 0x200000 + 0x1000*Context,
};

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
	Plicctl *ctl = (Plicctl*)CtlBase;

	ctl->threshold = 0;	/* accept any nonzero-priority IRQ */
}

void
plicenable(int irq, int priority)
{
	Plicpriority *pri = (Plicpriority*)PlicBase;
	ulong *en;

	pri->pri[irq] = priority;

	en = (ulong*)EnableBase;
	en[irq/32] |= 1 << (irq%32);
}

int
plicclaim(void)
{
	Plicctl *ctl = (Plicctl*)CtlBase;

	return ctl->claim;
}

void
pliccomplete(int irq)
{
	Plicctl *ctl = (Plicctl*)CtlBase;

	ctl->claim = irq;
}
