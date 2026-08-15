#include "mem.h"

#define UREGSIZE 296	/* sizeof(Ureg): 37 fields * 8 bytes, see /riscv64/include/ureg.h */

#define CSR_SSTATUS 0x100
#define CSR_SIE     0x104
#define CSR_STVEC   0x105
#define CSR_SEPC    0x141
#define CSR_SCAUSE  0x142
#define CSR_STVAL   0x143


#define CSR_TIME 0xC01


#define SIE_STIE     0x20	// sie bit 5 - supervisor timer interrupt enable
#define SIE_SEIE     0x200  // sie bit 9 - external PLIC interrupt enable
#define SSTATUS_SIE  0x2	// sstatus bit 1 - global S-mode interrupt enable


#define CSR_SATP 0x180


TEXT _start(SB), $0
    MOVW $setSB(SB), R3		// PC-relative -> PA while running physically

    // BSS isn't zeroed by anything in the u-boot fatload/go boot path
    MOVW $edata(SB), R9
    MOVW $end(SB), R11
zerobss:
    BGE R9, R11, zerodone
    MOV R0, 0(R9)
    ADD $8, R9
    JMP zerobss
zerodone:

    MOVW $stack+16368(SB), R2	// 16 below the top: leaves the argument shadow
				                // slot a called C function may write to

    JAL R1, mmubootstrap(SB)	// R8 = satp value; tables built with PAs
    MOVW R8, CSR(CSR_SATP)
    SFENCEVMA
    // identity and high mappings both live now; PC is still physical

    JAL R1, mmuhighentry(SB)	// R8 = VA of highstart
    JMP (R8)


// Everything from here on executes at a high VA, so PC-relative symbol
// references yield VAs and the physical aliases are no longer needed.
TEXT highstart(SB), $-8
    MOVW $setSB(SB), R3		// now resolves high
    MOVW $stack+16368(SB), R2	// high stack

    MOVW $mach0(SB), R9
    MOVW $m(SB), R8
    MOV R9, 0(R8)		// m = &mach0, both VAs

    JAL R1, uarthigh(SB)	// console -> high VA before the identity map goes
    JAL R1, mmulowdrop(SB)	// retire the identity mapping

    MOVW $main(SB), R10
    JMP (R10)


TEXT setstvec(SB), $0
    MOVW R8, CSR(CSR_STVEC)
    RET

TEXT trapvec(SB), $-8   // negative frame = no auto RA-save prologue (any call inside would trigger one otherwise)
    ADD $-UREGSIZE, R2	// R2 = new Ureg frame base; old sp = R2+UREGSIZE (not yet saved anywhere)

    MOV R1, 8(R2)		// save true r1 first - it's about to become scratch
    MOV R2, R1
    ADD $UREGSIZE, R1	// R1 = old sp
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
    ADD $-16, R2		// leave a shadow-slot gap below ureg for trap()'s own prologue to write into
    JAL R1, trap(SB)
    ADD $16, R2			// undo - R2 is back at the ureg frame base for the restore sequence below

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


TEXT rdtime(SB), $0
    MOVW CSR(CSR_TIME), R8
    RET


TEXT intrenable(SB), $0
    MOVW CSR(CSR_SIE), R8
    MOVW $SIE_SEIE, R9
    OR R9, R8
    MOVW R8, CSR(CSR_SIE)

    MOVW CSR(CSR_SSTATUS), R8
    MOVW $SSTATUS_SIE, R9
    OR R9, R8
    MOVW R8, CSR(CSR_SSTATUS)
    RET


TEXT sfencevma(SB), $0
    SFENCEVMA
    RET


// Here for testing purpuses
TEXT trapself(SB), $0
    SYS $0x001		// EBREAK
    RET

// Here for testing purpuses
TEXT resumetest(SB), $0
    MOVW $-5, R5
    MOVW $-6, R6
    MOVW $-7, R7
    MOVW $-8, R8
    MOVW $-9, R9
    MOVW $-10, R10
    MOVW $-11, R11
    MOVW $-12, R12
    MOVW $-13, R13
    MOVW $-14, R14
    MOVW $-15, R15
    MOVW $-16, R16
    MOVW $-17, R17
    MOVW $-18, R18
    MOVW $-19, R19
    MOVW $-20, R20
    MOVW $-21, R21
    MOVW $-22, R22
    MOVW $-23, R23
    MOVW $-24, R24
    MOVW $-25, R25
    MOVW $-26, R26
    MOVW $-27, R27
    MOVW $-28, R28
    MOVW $-29, R29
    MOVW $-30, R30
    MOVW $-31, R31

    SYS $0x001		// EBREAK - trap() decodes, prints, and now returns

    MOVW $resumebuf(SB), R4
    MOV R5, 0(R4)
    MOV R6, 8(R4)
    MOV R7, 16(R4)
    MOV R8, 24(R4)
    MOV R9, 32(R4)
    MOV R10, 40(R4)
    MOV R11, 48(R4)
    MOV R12, 56(R4)
    MOV R13, 64(R4)
    MOV R14, 72(R4)
    MOV R15, 80(R4)
    MOV R16, 88(R4)
    MOV R17, 96(R4)
    MOV R18, 104(R4)
    MOV R19, 112(R4)
    MOV R20, 120(R4)
    MOV R21, 128(R4)
    MOV R22, 136(R4)
    MOV R23, 144(R4)
    MOV R24, 152(R4)
    MOV R25, 160(R4)
    MOV R26, 168(R4)
    MOV R27, 176(R4)
    MOV R28, 184(R4)
    MOV R29, 192(R4)
    MOV R30, 200(R4)
    MOV R31, 208(R4)

    RET


TEXT intrdisable(SB), $0
    MOVW CSR(CSR_SSTATUS), R8
    MOVW $-3, R9		// ~SSTATUS_SIE - same trick as $-1 elsewhere, two's-complement
    AND R9, R8
    MOVW R8, CSR(CSR_SSTATUS)
    RET


TEXT dummy(SB), $0
    RET                 // empty function - cheap busy-wait


GLOBL stack(SB), $16384
GLOBL mach0(SB), $MACHSIZE  // storage for the one Mach struct
GLOBL m(SB), $8             // storage for the `m` pointer itself (8 bytes on riscv64)

GLOBL resumebuf(SB), $216	// 27 * 8 bytes, R5..R31 - for testing purpuses
