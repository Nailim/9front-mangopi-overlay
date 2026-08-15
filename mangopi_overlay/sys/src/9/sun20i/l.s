#include "mem.h"

#define UREGSIZE 296	/* sizeof(Ureg): 37 fields * 8 bytes, see /riscv64/include/ureg.h */

#define CSR_SSTATUS 0x100
#define CSR_SIE     0x104
#define CSR_STVEC   0x105
#define CSR_SEPC    0x141
#define CSR_SCAUSE  0x142
#define CSR_STVAL   0x143


#define CSR_TIME 0xC01


#define SIE_SEIE     0x200  // sie bit 9 - external PLIC interrupt enable
#define SSTATUS_SIE  0x2	// sstatus bit 1 - global S-mode interrupt enable


#define CSR_SATP 0x180


/*
 * Defines from libc/riscv64/tas.s */
#define AQ	(1<<26)
#define RL	(1<<25)
#define LRW(rs1, rd) \
	WORD $((2<<27)|(0<<20)|((rs1)<<15)|(2<<12)|((rd)<<7)|057|AQ)
#define SCW(rs2, rs1, rd) \
	WORD $((3<<27)|((rs2)<<20)|((rs1)<<15)|(2<<12)|((rd)<<7)|057|AQ|RL)

#define FENCE	WORD $(0xf | 0xff<<20)		/* fence iorw,iorw */


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
    
    MOVW $mach0(SB), R7		// m (physical until the jump)
    MOV R0, R6			    // up = nil

    JAL R1, mmubootstrap(SB)	// R8 = satp value; tables built with PAs
    MOVW R8, CSR(CSR_SATP)
    SFENCEVMA
    // identity and high mappings both live now; PC is still physical

    JAL R1, mmuhighentry(SB)	// R8 = VA of highstart
    JMP (R8)


// Everything from here on executes at a high VA, so PC-relative symbol
// references yield VAs and the physical aliases are no longer needed.
TEXT highstart(SB), $-8
    MOVW $setSB(SB), R3		    // now resolves high
    MOVW $stack+16368(SB), R2	// high stack

    MOVW $mach0(SB), R7		// m = &mach0, pinned
    MOV R0, R6			    // up = nil

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
    // up/m: restoring these is only correct while traps come from kernel mode - later in implementation we must reload, not trust, them
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


// Interrupt masking ...
TEXT splhi(SB), $0
    CSRRC CSR(CSR_SSTATUS), $SSTATUS_SIE, R8	// clear SIE, old sstatus -> R8
    AND $SSTATUS_SIE, R8
    RET

TEXT spllo(SB), $0
    CSRRS CSR(CSR_SSTATUS), $SSTATUS_SIE, R8	// set SIE, old sstatus -> R8
    AND $SSTATUS_SIE, R8
    RET

TEXT splx(SB), $0
    BEQ R8, splxoff
    CSRRS CSR(CSR_SSTATUS), $SSTATUS_SIE, R8
    RET
splxoff:
    CSRRC CSR(CSR_SSTATUS), $SSTATUS_SIE, R8
    RET

TEXT islo(SB), $0
    MOVW CSR(CSR_SSTATUS), R8
    AND $SSTATUS_SIE, R8
    RET


// Scheduler primitives ...
TEXT setlabel(SB), $-8
    MOV R2, 0(R8)	// Label.sp
    MOV R1, 8(R8)	// Label.pc = our return address
    MOV R0, R8		// return 0
    RET

TEXT gotolabel(SB), $-8
    MOV 0(R8), R2	// restore sp
    MOV 8(R8), R1	// restore pc
    MOV $1, R8		// return 1 - reappears as setlabel's second return
    RET

// Atomics and barriers ...
TEXT tas(SB), $0
TEXT _tas(SB), $0
    MOV R8, R12		// address of the key
    MOV $1, R10
    FENCE
tas1:
    LRW(12, 8)		// R8 = *R12 (old value, returned)
    SCW(10, 12, 14)	// *R12 = 1 if reservation held; R14 = 0 on success
    BNE R14, tas1
    RET

TEXT cmpswap(SB), $0
    MOV R8, R12		// addr
    MOVW old+8(FP), R13	// long is 32-bit, slots are 8 bytes apart
    MOVW new+12(FP), R14
    FENCE
cas1:
    LRW(12, 15)
    BNE R15, R13, cas0	// mismatch: no reservation to clear, RISC-V has no CLREX
    SCW(14, 12, 16)
    BNE R16, cas1	// lost the reservation, retry
    MOV $1, R8
    RET
cas0:
    MOV R0, R8
    RET

TEXT coherence(SB), $0
    FENCE
    RET

TEXT idlehands(SB), $0
    SYS $0x105		// WFI
    RET

TEXT getcallerpc(SB), $0
    MOV 0(R2), R8	// caller's saved LINK at 0(SP)
    RET


TEXT dummy(SB), $0
    RET                 // empty function - cheap busy-wait


GLOBL stack(SB), $16384
GLOBL mach0(SB), $MACHSIZE  // storage for the one Mach struct
