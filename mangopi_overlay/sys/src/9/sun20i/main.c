#include "u.h"
#include "dat.h"
#include "wdt.h"

void PUT32(unsigned int, unsigned int);
unsigned int GET32(unsigned int);
void dummy(unsigned int);

void trapinit(void);

// for testing
void trapself(void);
void resumetest(void);
extern uintptr resumebuf[27];


// D1 SOC SPECIFIC: UART0_BASE/UART_THR/UART_LSR below
// to be replaced in a structured way later
#define UART0_BASE 0x02500000
#define UART_THR (UART0_BASE + (0 * 4))
#define UART_LSR (UART0_BASE + (5 * 4))
#define UART_LSR_THRE (1 << 5)

void uart_putc(int c)
{
	while(!(GET32(UART_LSR) & UART_LSR_THRE));
	PUT32(UART_THR, c & 0xFF);
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
	uart_puts("m->machno = ");
	uart_puthex64((unsigned long long)m->machno);
	uart_puts("\n");

    trapinit();

	wdt_riscv_feed();
	while(1){
		delay(20000000);
		wdt_riscv_feed();
	}
}
