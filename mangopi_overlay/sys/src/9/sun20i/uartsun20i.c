/*
 * Allwinner D1 UART: a DesignWare 8250, adapted from cycv/uartcycv.c.
 */

 /*
 * TODO: update later when real intrenable() is available:
 * now input is polled from clockintr rather than interrupt-driven -
 * plicintr is still a hardcoded switch on TIMER0IRQ.
 */

#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "io.h"


enum {
	RBR = 0,
	IER,
	FCR,    /* IIR on read, FCR on write */
	LCR,
	MCR,
	LSR,
	MSR,
	SCR,
	
	IIR = FCR,
};

enum {
	LSR_DR = 1<<0,      /* LSR: data ready */
	LSR_THRE = 1<<5,    /* LSR: transmit holding empty */
};

typedef struct Ctlr {
	Lock;
	ulong *r;
} Ctlr;

extern PhysUart sun20iphysuart;

static Ctlr uctlr[1] = {
	{ .r = (ulong*)KADDR(PHYSUART0) },
};

static Uart suart[1] = {
	{
		.regs	= &uctlr[0],
		.name	= "UART0",
		.freq	= 24000000,
		.phys	= &sun20iphysuart,
		.console = 1,
		.baud	= 115200,
	},
};

void
uartconsinit(void)
{
	consuart = suart;
}

static Uart*
suartpnp(void)
{
	return suart;
}


static int
suartgetc(Uart *uart)
{
	Ctlr *ct;

	ct = uart->regs;
	while((ct->r[LSR] & LSR_DR) == 0)
        ;
	return ct->r[RBR];
}

static void
suartputc(Uart *uart, int ch)
{
	Ctlr *ct;

	ct = uart->regs;
	while((ct->r[LSR] & LSR_THRE) == 0)
		;
	ct->r[RBR] = ch;
}

/*
 * Drain all staged output synchronously - TODO update when intrenable() is available.
 * With no interrupts we must finish here or output stalls after one FIFO.
 */
static void
suartkick(Uart *uart)
{
	Ctlr *ct;
    int ch;

	ct = uart->regs;
	USED(ct);
	for(;;){
		if(uart->op >= uart->oe && uartstageoutput(uart) == 0)
			break;
		ch = *uart->op++;
		if(ch == '\n')		/* TODO: remove later - kbdfs's job */
			suartputc(uart, '\r');
		suartputc(uart, ch);
	}
}

/*
 * Called from clockintr. Moves received characters into the Uart's stage ring,
 * devuart's own uartclock() pushes them to the queue.
 */
void
uartpoll(void)
{
	Ctlr *ct;
    int ch;

	ct = uctlr;
	while(ct->r[LSR] & LSR_DR){
		ch = ct->r[RBR];
		if(ch == '\r')			/* TODO: remove later - kbdfs's job */
			ch = '\n';
		uartrecv(suart, ch);
	}
}

static void
suartenable(Uart *uart, int)
{
	Ctlr *ct;

	ct = uart->regs;
	ilock(ct);
	while((ct->r[LSR] & LSR_THRE) == 0)
		;
	ct->r[LCR] = 0x03;	/* 8 bits, no parity, 1 stop */
	ct->r[IIR] = 0x01;	/* FCR: enable FIFOs */
	ct->r[IER] = 0;		/* no interrupts - we poll */
	iunlock(ct);
}

static int
suartbits(Uart *uart, int n)
{
	Ctlr *ct;

	ct = uart->regs;
	switch(n){
	case 5: ct->r[LCR] = ct->r[LCR] & ~3 | 0; return 0;
	case 6: ct->r[LCR] = ct->r[LCR] & ~3 | 1; return 0;
	case 7: ct->r[LCR] = ct->r[LCR] & ~3 | 2; return 0;
	case 8: ct->r[LCR] = ct->r[LCR] & ~3 | 3; return 0;
	}
	return -1;
}

/* U-Boot already set rate at 115200*/
static int
suartbaud(Uart*, int n)
{
    print("uart baud %d\n", n);
	return 0;
}

static int
suartparity(Uart *uart, int p)
{
	Ctlr *ct;

	ct = uart->regs;
	switch(p){
	case 'n': ct->r[LCR] = ct->r[LCR] & ~0x38; return 0;
	case 'o': ct->r[LCR] = ct->r[LCR] & ~0x38 | 0x08; return 0;
	case 'e': ct->r[LCR] = ct->r[LCR] & ~0x38 | 0x18; return 0;
	}
	return -1;
}

static void
suartnop(Uart*, int)
{
}

static int
suartnope(Uart*, int)
{
	return -1;
}

PhysUart sun20iphysuart = {
	.pnp		= suartpnp,
	.enable		= suartenable,
	.kick		= suartkick,
	.getc		= suartgetc,
	.putc		= suartputc,
	.bits		= suartbits,
	.baud		= suartbaud,
	.parity		= suartparity,

	.stop		= suartnope,
	.rts		= suartnop,
	.dtr		= suartnop,
	.dobreak	= suartnop,
	.fifo		= suartnop,
	.power		= suartnop,
	.modemctl	= suartnop,
};
