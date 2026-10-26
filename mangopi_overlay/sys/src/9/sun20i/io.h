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
#define	PHYSMMC0	0x04020000	    /* SD card slot */
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

#define UART0IRQ    18
#define	MMC0IRQ		56
#define	TIMER0IRQ	75

#define	PHYSCCU		0x02001000	    /* within PHYSPIO */

/*
 * USB.  The MQ-Pro wires USB1 as the host port (dts: ehci1/ohci1
 * "okay", usb_otg "peripheral"), so USB0 is never a host here and
 * PHYSEHCI1 is the only block this port drives.
 *
 * PHYSUSB0 is recorded but deliberately not mapped.  It covers the OTG
 * block and the PHYs' shared control register at +0x400; for the D1,
 * phy-sun4i-usb.c reaches that register only on the PHY0 path, and
 * PHY1 needs nothing but the two writes in phy1enable().  Map it if
 * USB0 is ever driven.
 */
#define	PHYSUSB0	0x04100000	/* OTG block; PHYs' shared ctl at +0x400 */
#define	PHYSEHCI1	0x04200000	/* EHCI1; OHCI1 at +0x400, PHY1 pmu at +0x800 */

#define	EHCI1IRQ	49
#define	OHCI1IRQ	50		    /* 9front has no OHCI driver */

#define	PHYSPLAN9INI	0x40800000	/* U-Boot loads plan9.ini here: above OpenSBI, below the kernel */
#define	BOOTARGS	((char*)KADDR(PHYSPLAN9INI))
#define	BOOTARGSLEN	0x2000
