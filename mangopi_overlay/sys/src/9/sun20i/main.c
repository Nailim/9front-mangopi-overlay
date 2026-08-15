#include "u.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "io.h"

void dummy(unsigned int);

void trapinit(void);
/* no mmuinit - the MMU switch now happens in l.s before main() */


#define UART_LSR_THRE	(1<<5)

static volatile u32int *uart = (volatile u32int*)PHYSUART0;

void
uarthigh(void)
{
	uart = (volatile u32int*)KADDR(PHYSUART0);
}

void
uart_putc(int c)
{
	while((uart[5] & UART_LSR_THRE) == 0);	/* LSR at +0x14 */
	uart[0] = c & 0xFF;			            /* THR at +0x00 */
}

void uart_puts(char *s)
{
	while(*s){
		if(*s == '\n')
			uart_putc('\r');
		uart_putc(*s++);
	}
}

void uart_puthex64(unsigned long long v)
{
	static char digits[] = "0123456789abcdef";
	int i;

	uart_puts("0x");
	for(i = 60; i >= 0; i -= 4)
		uart_putc(digits[(v >> i) & 0xf]);
}

void delay(unsigned int count)
{
	unsigned int i;

	for(i = 0; i < count; i++)
		dummy(i);
}


void main(void)
{
	uart_puts("9sun20i: Plan9 riscv64 D1 kernel skeleton starting\n");

	uart_puts("main at ");
	uart_puthex64((unsigned long long)main);
	uart_puts("\n");

	uart_puts("m->machno = ");
	uart_puthex64((unsigned long long)m->machno);
	uart_puts("\n");

	trapinit();

    // testing 
    Label l;
	long v;
	int n;

	n = setlabel(&l);
	uart_puts("setlabel returned ");
	uart_puthex64(n);
	uart_puts("\n");
	if(n == 0)
		gotolabel(&l);		/* should reappear above with n == 1 */

	v = 0;
	uart_puts("tas first  = "); uart_puthex64(tas(&v)); uart_puts("\n");	/* 0 */
	uart_puts("tas second = "); uart_puthex64(tas(&v)); uart_puts("\n");	/* 1 */

	v = 5;
	uart_puts("cas match  = "); uart_puthex64(cmpswap(&v, 5, 9)); uart_puts("\n");	/* 1, v=9 */
	uart_puts("cas miss   = "); uart_puthex64(cmpswap(&v, 5, 7)); uart_puts("\n");	/* 0, v=9 */
	uart_puts("v = "); uart_puthex64(v); uart_puts("\n");				/* 9 */
    

	wdt_riscv_feed();
	while(1){
		delay(20000000);
		wdt_riscv_feed();
	}
}
