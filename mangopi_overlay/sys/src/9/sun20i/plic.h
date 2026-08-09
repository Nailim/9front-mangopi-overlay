#ifndef PLIC_H
#define PLIC_H

/*
 * RISC-V Platform-Level Interrupt Controller
 */

void plicinit(void);
void plicenable(int irq, int priority);
int plicclaim(void);
void pliccomplete(int irq);

#endif
