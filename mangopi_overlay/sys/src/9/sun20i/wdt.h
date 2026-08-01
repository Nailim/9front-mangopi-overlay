#ifndef WDT_H
#define WDT_H

/*
 * D1 SOC SPECIFIC - see wdt.c. Everything in this header/its
 * implementation is Allwinner D1 hardware, not a generic riscv64/Plan9
 * pattern. Ported here anyway (rather than left out of the skeleton)
 * because SOME watchdog will almost certainly need enabling/feeding/
 * disabling on any real board bring-up, and getting bit by an
 * unexpectedly-armed watchdog during early boot is a near-universal
 * gotcha worth having a worked example of, even one that needs
 * rewriting for a different SoC.
 */

void wdt_main_enable(unsigned int timeout_seconds);
void wdt_main_disable(void);
void wdt_main_feed(void);

void wdt_riscv_enable(unsigned int timeout_seconds);
void wdt_riscv_disable(void);
void wdt_riscv_feed(void);

#endif
