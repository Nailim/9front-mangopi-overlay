#include "mem.h"

#define CSR_SATP 0x180
#define FENCEI		WORD $0x0000100F
#define DCACHECIPA(rs1)	WORD $(0x02b0000b | ((rs1)<<15))
#define SYNCI		WORD $0x01a0000b

/* R8 -> { dest PA, source PA, size } */
TEXT _reboot(SB), 1, $-4
	MOV	0(R8), R10
	MOV	8(R8), R11
	MOV	16(R8), R12
	MOV	R10, R14		/* start of the new kernel */
	ADD	R10, R12, R13		/* end */

	/* leave translation: this page is identity mapped */
	MOVW	R0, CSR(CSR_SATP)
	SFENCEVMA

copy:
	MOV	0(R11), R15
	MOV	R15, 0(R10)
	ADD	$8, R11
	ADD	$8, R10
	SLTU	R13, R10, R15
	BNE	R15, copy

	MOV	R14, R10
flush:
	DCACHECIPA(10)
	ADD	$CACHELINE, R10
	SLTU	R13, R10, R15
	BNE	R15, flush
	SYNCI
	FENCEI
	JMP	(R14)
