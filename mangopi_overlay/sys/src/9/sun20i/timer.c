#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "io.h"

/*
 * Allwinner SOC SPECIFIC 
 * timer0 only for now while testing 
 */


enum {
	ModeContinuous	= 0<<7,
	ModeSingle	= 1<<7,		/* count down once and stop, vs auto-reload */
	Div1		= 0<<4,
	SrcHosc		= 1<<2,	/* 24MHz - matches TIMEBASEFREQ */
	Reload		= 1<<1,
	Enable		= 1<<0,

	Timer0Irq	= 1<<0,
};

enum {
	MinPeriod	= TIMEBASEFREQ/(100*HZ),	/* 100us - don't livelock */
	MaxPeriod	= TIMEBASEFREQ/HZ,			/* one tick, 10ms */
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
	Timerregs *tr = (Timerregs*)KADDR(PHYSTIMER);

	tr->intv0 = ticks;
	tr->ctl0 = ModeContinuous|Div1|SrcHosc|Reload|Enable;
	tr->irqen |= Timer0Irq;

    plicenable(TIMER0IRQ, 1);
}

void
timer0ack(void)
{
	Timerregs *tr = (Timerregs*)KADDR(PHYSTIMER);
    ulong sta;

	tr->irqsta = Timer0Irq;
	sta = tr->irqsta;	/* readback - forces the clear to fully take effect */
    USED(sta);
}

void
timer0set(ulong ticks)
{
	Timerregs *tr = (Timerregs*)KADDR(PHYSTIMER);

	tr->intv0 = ticks;
	tr->ctl0 = ModeSingle|Div1|SrcHosc|Reload|Enable;
}

void
timerset(uvlong when)
{
	vlong period;

	period = when - fastticks(nil);
	if(period < MinPeriod)
		period = MinPeriod;
	else if(period > MaxPeriod)
		period = MaxPeriod;

	timer0set(period);
}

ulong
perfticks(void)
{
	return rdtime();
}
