#include "u.h"
#include "mem.h"
#include "plic.h"
#include "timer.h"

/*
 * Allwinner SOC SPECIFIC 
 * timer0 only for now while testing 
 */

enum {
	TimerBase	= 0x02050000,

	ModeContinuous	= 0<<7,
	Div1		= 0<<4,
	SrcHosc		= 1<<2,	/* 24MHz - matches TIMEBASEFREQ */
	Reload		= 1<<1,
	Enable		= 1<<0,

	Timer0Irq	= 1<<0,
};

typedef struct Timerregs Timerregs;
struct Timerregs
{
	ulong	irqen;
	ulong	irqsta;
	uchar	pad[8];
	ulong	ctl0;
	ulong	intv0;
	ulong	cur0;
	uchar	pad0[4];
	ulong	ctl1;
	ulong	intv1;
	ulong	cur1;
};

void
timer0init(ulong ticks)
{
	Timerregs *tr = (Timerregs*)KADDR(TimerBase);

	tr->intv0 = ticks;
	tr->ctl0 = ModeContinuous|Div1|SrcHosc|Reload|Enable;
	tr->irqen |= Timer0Irq;

    plicenable(TIMER0IRQ, 1);
}

void
timer0ack(void)
{
	Timerregs *tr = (Timerregs*)KADDR(TimerBase);
    ulong sta;

	tr->irqsta = Timer0Irq;
	sta = tr->irqsta;	/* readback - forces the clear to fully take effect */
    USED(sta);
}
