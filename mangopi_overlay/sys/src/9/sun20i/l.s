#include "mem.h"

#define UREGSIZE 296	/* sizeof(Ureg): 37 fields * 8 bytes, see /riscv64/include/ureg.h */

#define CSR_SSTATUS 0x100
#define CSR_SIE     0x104
#define CSR_STVEC   0x105
#define CSR_SEPC    0x141
#define CSR_SCAUSE  0x142
#define CSR_STVAL   0x143

TEXT _start(SB), $0
    MOVW $setSB(SB), R3

    // BSS isn't zeroed by anything in the u-boot fatload/go boot path
    MOVW $edata(SB), R9
    MOVW $end(SB), R11
zerobss:
    BGE R9, R11, zerodone
    MOV R0, 0(R9)
    ADD $8, R9
    JMP zerobss
zerodone:

    MOVW $stack+16384(SB), R2

    MOVW $mach0(SB), R9 // address of the Mach struct storage
    MOVW $m(SB), R8     // address of the `m` pointer variable itself
    MOV R9, 0(R8)       // m = &mach0

    MOVW $main(SB), R10	// load address of main into R10.
    JMP (R10)			// plain jump into main


TEXT setstvec(SB), $0
    MOVW R8, CSR(CSR_STVEC)
    RET

TEXT trapvec(SB), $0
    ADD $-UREGSIZE, R2		// R2 = new Ureg frame base; old sp = R2+UREGSIZE (not yet saved anywhere)

    MOV R1, 8(R2)		// save true r1 first - it's about to become scratch
    MOV R2, R1
    ADD $UREGSIZE, R1		// R1 = old sp
    MOV R1, 16(R2)		// ureg->sp

    MOV R3, 24(R2)
    MOV R4, 32(R2)
    MOV R5, 40(R2)
    MOV R6, 48(R2)
    MOV R7, 56(R2)
    MOV R8, 64(R2)
    MOV R9, 72(R2)
    MOV R10, 80(R2)
    MOV R11, 88(R2)
    MOV R12, 96(R2)
    MOV R13, 104(R2)
    MOV R14, 112(R2)
    MOV R15, 120(R2)
    MOV R16, 128(R2)
    MOV R17, 136(R2)
    MOV R18, 144(R2)
    MOV R19, 152(R2)
    MOV R20, 160(R2)
    MOV R21, 168(R2)
    MOV R22, 176(R2)
    MOV R23, 184(R2)
    MOV R24, 192(R2)
    MOV R25, 200(R2)
    MOV R26, 208(R2)
    MOV R27, 216(R2)
    MOV R28, 224(R2)
    MOV R29, 232(R2)
    MOV R30, 240(R2)
    MOV R31, 248(R2)

    MOVW CSR(CSR_SEPC), R1
    MOV R1, 0(R2)		// ureg->pc

    MOVW CSR(CSR_SSTATUS), R1
    MOV R1, 256(R2)		// ureg->status

    MOVW CSR(CSR_SIE), R1
    MOV R1, 264(R2)		// ureg->ie

    MOVW CSR(CSR_SCAUSE), R1
    MOV R1, 272(R2)		// ureg->cause

    MOVW CSR(CSR_STVAL), R1
    MOV R1, 280(R2)		// ureg->tval

    MOVW $0, R1
    MOV R1, 288(R2)		// ureg->curmode - debugger-only field (libmach j.c), no hardware source

    MOV R2, R8			// arg0 = ureg
    JAL R1, trap(SB)

    MOV 0(R2), R1
    MOVW R1, CSR(CSR_SEPC)	// sepc = ureg->pc, in case trap() advanced it

    MOV 8(R2), R1
    MOV 24(R2), R3
    MOV 32(R2), R4
    MOV 40(R2), R5
    MOV 48(R2), R6
    MOV 56(R2), R7
    MOV 64(R2), R8
    MOV 72(R2), R9
    MOV 80(R2), R10
    MOV 88(R2), R11
    MOV 96(R2), R12
    MOV 104(R2), R13
    MOV 112(R2), R14
    MOV 120(R2), R15
    MOV 128(R2), R16
    MOV 136(R2), R17
    MOV 144(R2), R18
    MOV 152(R2), R19
    MOV 160(R2), R20
    MOV 168(R2), R21
    MOV 176(R2), R22
    MOV 184(R2), R23
    MOV 192(R2), R24
    MOV 200(R2), R25
    MOV 208(R2), R26
    MOV 216(R2), R27
    MOV 224(R2), R28
    MOV 232(R2), R29
    MOV 240(R2), R30
    MOV 248(R2), R31

    MOV 16(R2), R2		// restore sp last - it's still needed as the base for this very load

    SYS $0x102			// SRET


// Here for testing purpuses
TEXT trapself(SB), $0
    SYS $0x001		// EBREAK
    RET

// PUT32/GET32/dummy: the proven MMIO/delay primitives
// Revisit and test `volatile` pointer codegen later
TEXT PUT32(SB), $0
    MOVW val+4(FP), R11	// second argument (value) - passed on the stack
    MOVW R11, 0(R8)		// first argument (address) - passed in R8
    RET
TEXT GET32(SB), $0
    MOVW 0(R8), R8		// first (and only) argument - passed in R8,
				        // result returned in the same register
    RET
TEXT dummy(SB), $0
    RET                 // empty function - cheap busy-wait


GLOBL stack(SB), $16384
GLOBL mach0(SB), $MACHSIZE  // storage for the one Mach struct
GLOBL m(SB), $8             // storage for the `m` pointer itself (8 bytes on riscv64)
