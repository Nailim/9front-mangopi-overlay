/*
 * Allwinner D1 (sun20i) architecture-specific stuff
 */

#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "io.h"


enum {
	Wdogtime	= 16,		/* seconds, the maximum (wdt_timeout_map) */
	Wdogfeedms	= 1000,		/* feed once a second */
};

/*
 * Hard reset: OpenSBI lists no srst, so let the main watchdog reset the SoC.
 */
void
archreboot(void)
{
    splhi();
	wdt_main_enable(1);     // with 1 s to drain the uart
	for(;;)
		idlehands();
}

void
wdogfeed(void)
{
	wdt_riscv_feed();
}

void
wdogoff(void)
{
	wdt_riscv_disable();
}

/* after timersinit() and xinit(): addclock0link mallocs */
void
wdogstart(void)
{
	wdt_riscv_enable(Wdogtime);
	addclock0link(wdogfeed, Wdogfeedms);
}
