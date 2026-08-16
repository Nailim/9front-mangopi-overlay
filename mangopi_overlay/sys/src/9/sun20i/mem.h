/*
 * Memory and machine-specific definitions. Used in C and assembler.
 */

#define MIN(a, b)	((a) < (b)? (a): (b))
#define MAX(a, b)	((a) > (b)? (a): (b))

/*
 * Sizes
 */
#define	BI2BY		8			/* bits per byte */
#define	BY2PG		4096		/* bytes per page */
#define	BY2WD		8			/* bytes per word */
#define	BY2V		8			/* bytes per double word (xalloc.c) */
#define	BLOCKALIGN	64			/* C906 L1 line size (allocb.c) */

#define	PTEMAPMEM	(1024*1024)
#define	PTEPERTAB	(PTEMAPMEM/BY2PG)
#define	SEGMAPSIZE	8192
#define	SSEGMAPSIZE	16

#define	PGSHIFT		12			/* log(BY2PG) */
#define	ROUND(s, sz)	(((s)+((sz)-1))&~((sz)-1))
#define	PGROUND(s)	ROUND(s, BY2PG)

#define MAXMACH 1
#define KSTACK 4096

/*
 * Physical layout
 */
#define	PHYSDRAM	0x40000000		/* DRAM base */
#define	PHYSTEXT	0x41000000		/* where u-boot loads the kernel */

/*
 * Virtual layout
 */
#define	KZERO		(0xFFFFFFFF80000000ULL)	/* PA 0x00000000 */
#define	VDRAM		(KZERO + PHYSDRAM)	/* 0xFFFFFFFFC0000000 */
#define	KTZERO		(KZERO + PHYSTEXT)	/* 0xFFFFFFFFC1000000 - must match mkfile */

#define	KADDR(pa)	((void*)((uintptr)(pa) + KZERO))
#define	PADDR(va)	((uintptr)(va) - KZERO)


#define MACHSIZE 8192

/*
 * Set by SBI, make it work now, scale later
 */
#define	HZ		    100			    /* clock frequency */
#define	MS2HZ		(1000/HZ)		/* millisec per clock tick */
#define	TK2SEC(t)	((t)/HZ)		/* ticks to seconds */
#define TIMEBASEFREQ	24000000
#define	TICKINTERVAL	(TIMEBASEFREQ/HZ)	/* 24MHz / 100 = 240000 */


/*
 * Sv39 MMU
 */
#define	PTSHIFT		(PGSHIFT-3)		/* log2(entries per page-table page): 9 */
#define	PTLEVELS	3			    /* Sv39 is always exactly 3 levels */
#define	PTLX(v, l)	(((v) >> (PGSHIFT + (l)*PTSHIFT)) & ((1<<PTSHIFT)-1))	/* VPN[l] */
#define	PGLSZ(l)	(1ULL << (PGSHIFT + (l)*PTSHIFT))	/* bytes mapped by one level-l leaf */

/*
 * User address space: the Sv39 low half, 0 - 0x3FFFFFFFFF (2^38, since bit 38 is the sign bit).
 */
#define	UZERO		0ULL			    /* user segment base */
#define	UTZERO		(UZERO+BY2PG)		/* user text start */
#define	USTKTOP		0x0000003FFFFF0000ULL	/* user segment end +1 */
#define	USTKSIZE	(16*1024*1024)		/* user stack size */

/* PTE bits - RISC-V Sv39, bits 0-7, spec-defined */
#define	PTEVALID	(1<<0)
#define	PTEREAD		(1<<1)
#define	PTEWRITE	(1<<2)
#define	PTEEXEC		(1<<3)
#define	PTEUSER		(1<<4)
#define	PTEGLOBAL	(1<<5)
#define	PTEACCESSED	(1<<6)
#define	PTEDIRTY	(1<<7)

#define	PTEPTR		(PTEVALID)			/* V=1, R=W=X=0: points to next level */
#define	PTELEAF		(PTEVALID|PTEREAD|PTEWRITE|PTEEXEC|PTEACCESSED|PTEDIRTY|PTEGLOBAL)	/* TODO while testing: full RWX, no real permission enforcement yet */

/* PPN occupies PTE bits 53:10 */
#define	PPN(x)		((x)&~(BY2PG-1))	/* portable: page-align a PA */

#define	PTEPPNSHIFT	10
#define	PTEPA(pte)	(((uintptr)(pte) >> PTEPPNSHIFT) << PGSHIFT)	/* PTE -> PA it points to */
#define	PAPTE(pa)	(((uintptr)(pa) >> PGSHIFT) << PTEPPNSHIFT)	    /* PA -> PPN field */

/*
 * Attribute flags fault.c ORs into the page-aligned PA passed to
 * putmmu(). Decoded by putmmu(), never written to a PTE as-is.
 * PTEVALID and PTEWRITE double as the RISC-V V and W bits; PTERONLY and
 * PTECACHED are the zero-valued defaults, so putmmu tests their opposites.
 */
#define	PTERONLY	0		/* absence of PTEWRITE */
#define	PTECACHED	0		/* default; putmmu adds T-HEAD C|SH */
#define	PTEUNCACHED	(1<<8)	/* above the RISC-V bits, below BY2PG */

/* satp: mode in bits 63:60 (8=Sv39), root table PPN in bits 43:0 */
#define	SATPMODE_SV39	(8ULL<<60)
#define	MKSATP(rootpa)	(SATPMODE_SV39 | ((uintptr)(rootpa) >> PGSHIFT))

#define	SFENCEVMA	WORD $0x12000073	/* sfence.vma x0, x0 - flush all */

/* T-HEAD C906 custom memory-type PTE bits (63:60) - NOT standard RISC-V */
#define PTE_THEAD_SO	(1ULL<<63)	/* strong order */
#define PTE_THEAD_C	    (1ULL<<62)	/* cacheable */
#define PTE_THEAD_B	    (1ULL<<61)	/* bufferable */
#define PTE_THEAD_SH	(1ULL<<60)	/* shareable */


#define PTELEAFMEM	(PTELEAF|PTE_THEAD_C|PTE_THEAD_SH)	/* MT_PMA - normal RAM */
#define PTELEAFDEV	(PTELEAF|PTE_THEAD_SO)			/* MT_IO - device/MMIO */

