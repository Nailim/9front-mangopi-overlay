#include "mem.h"


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
    RET                 // Empty function - cheap busy-wait


GLOBL stack(SB), $16384
GLOBL mach0(SB), $MACHSIZE  // storage for the one Mach struct
GLOBL m(SB), $8             // storage for the `m` pointer itself (8 bytes on riscv64)
