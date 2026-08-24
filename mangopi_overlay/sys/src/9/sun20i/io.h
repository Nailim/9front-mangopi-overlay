/*
 * D1 physical MMIO addresses and IRQ numbers.
 * All physical - drivers reach them through KADDR().
 */

 
#define DRAMMAX (1024*1024*1024)    /* max expected ram on these boards */

/* Low-speed peripheral block: pinctrl, CCU, timer, watchdog, UARTs,
 * i2c, ... - mapped as one range rather than page by page. */
#define	PHYSPIO		0x02000000
#define	PHYSPIOSIZE	(6*1024*1024)

#define	PHYSTIMER	0x02050000	    /* within PHYSPIO */
#define	PHYSUART0	0x02500000	    /* within PHYSPIO */
#define	PHYSWDTMAIN	0x020500a0	    /* within PHYSPIO */
#define	PHYSWDTRISCV	0x06011000	/* the C906's own watchdog - separate */

/*
 * PLIC: context 0 = hart0 M-mode, context 1 = hart0 S-mode
 * - this port only ever uses context 1.
 * (dtsi: interrupts-extended = <&cpu0_intc 11>, <&cpu0_intc 9>).*/

/* Note: PHYSPLICEN is not page-aligned (PLIC base + 0x2080). mapleaf
 * truncates to the containing page, which is the one we want, and
 * BY2PG is a single iteration so it can't spill into the next page.
 * Don't "fix" this by rounding the size up. */

#define	PHYSPLIC	0x10000000
#define	PLICCONTEXT	1
#define	PHYSPLICEN	(PHYSPLIC + 0x002000 + 0x80*PLICCONTEXT)
#define	PHYSPLICCTL	(PHYSPLIC + 0x200000 + 0x1000*PLICCONTEXT)

#define	TIMER0IRQ	75
