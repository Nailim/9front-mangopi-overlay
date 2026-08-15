#include "u.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "io.h"



/*
 * D1 SOC SPECIFIC - register offsets, the key value, the timeout
 * encoding table, and both base addresses below are Allwinner D1
 * hardware (from U-Boot's sunxi_wdt driver / arch/riscv/dts/
 * sun20i-d1.dtsi). Porting to a different board means replacing this
 * whole file with that SoC's own watchdog register layout - nothing
 * here generalizes. What DOES generalize: the pattern of "check
 * whether a watchdog is already armed by the boot chain before you get
 * control, and feed or explicitly disable it early" - some form of
 * this will very likely be needed on any real board, which is why this
 * is in the skeleton at all rather than left as a driver you'd only
 * find by porting rv_baremetal-boot-plan9-riscv64-wdt separately.
 *
 * D1 has multiple watchdog instances sharing the same IP block/register
 * layout: CTRL/CFG/MODE at +0x10/+0x14/+0x18 from each watchdog's base,
 * gated by a fixed key that must be present in CFG/MODE writes.
 *
 *  - "wdt" (main system-reset watchdog): 0x020500a0
 *  - "riscv_wdt" (the C906 core's own watchdog - left enabled, no
 *    status override, in the dtsi - this is the one that will bite you
 *    if you don't feed or disable it): 0x06011000
 */

#define WDT_KEY 0x16aa0000

#define WDT_CTRL_RELOAD ((1 << 0) | (0x0a57 << 1)) /* kick/reload */
#define WDT_MODE_EN (1 << 0)
#define WDT_RESET_MASK 0x03
#define WDT_RESET_VAL  0x01
#define WDT_TIMEOUT_SHIFT 4
#define WDT_TIMEOUT_MASK  0xf
#define WDT_MAX_TIMEOUT 16 /* seconds */

typedef struct Wdtregs Wdtregs;
struct Wdtregs
{
	uchar	pad[0x10];
	ulong	ctrl;		/* +0x10 */
	ulong	cfg;		/* +0x14 */
	ulong	mode;		/* +0x18 */
};

#define MAINWDT		((Wdtregs*)KADDR(PHYSWDTMAIN))
#define RISCVWDT	KADDR(PHYSWDTRISCV)

/* seconds -> register value, same table as wdt_timeout_map in
 * drivers/watchdog/sunxi_wdt.c, indices 0..16 in order. Written as a
 * plain positional array, not C99 designated initializers - ic (an
 * ANSI/K&R-era compiler) doesn't support those. */
static unsigned char wdt_timeout_map[1 + WDT_MAX_TIMEOUT] = {
	0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x7, 0x8,
	0x8, 0x9, 0x9, 0xa, 0xa, 0xb, 0xb,
};

static void
wdt_feed_at(Wdtregs *w)
{
	w->ctrl = WDT_CTRL_RELOAD;
}

static void
wdt_disable_at(Wdtregs *w)
{
	w->mode = WDT_KEY;
}

static void
wdt_enable_at(Wdtregs *w, unsigned int timeout_seconds)
{
	ulong val;

	if(timeout_seconds > WDT_MAX_TIMEOUT)
		timeout_seconds = WDT_MAX_TIMEOUT;

	val = w->cfg;
	val &= ~WDT_RESET_MASK;
	val |= WDT_RESET_VAL | WDT_KEY;
	w->cfg = val;

	val = w->mode;
	val &= ~(WDT_TIMEOUT_MASK << WDT_TIMEOUT_SHIFT);
	val |= wdt_timeout_map[timeout_seconds] << WDT_TIMEOUT_SHIFT;
	val |= WDT_MODE_EN | WDT_KEY;
	w->mode = val;

	wdt_feed_at(w);
}

void
wdt_main_enable(unsigned int timeout_seconds)
{
	wdt_enable_at(MAINWDT, timeout_seconds);
}

void
wdt_main_disable(void)
{
	wdt_disable_at(MAINWDT);
}

void
wdt_main_feed(void)
{
	wdt_feed_at(MAINWDT);
}

void
wdt_riscv_enable(unsigned int timeout_seconds)
{
	wdt_enable_at(RISCVWDT, timeout_seconds);
}

void
wdt_riscv_disable(void)
{
	wdt_disable_at(RISCVWDT);
}

void
wdt_riscv_feed(void)
{
	 wdt_feed_at(RISCVWDT);
}
