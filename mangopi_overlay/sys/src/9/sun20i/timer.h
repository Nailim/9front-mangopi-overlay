#ifndef TIMER_H
#define TIMER_H

/*
 * Allwinner SOC SPECIFIC 
 * timer0 only for now while testing 
 */

#define TIMER0IRQ 75

void timer0init(ulong ticks);
void timer0ack(void);


#endif
