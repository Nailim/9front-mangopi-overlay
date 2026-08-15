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
#define TIMEBASEFREQ	24000000
#define TIMERHZ		1
#define TICKINTERVAL	(TIMEBASEFREQ/TIMERHZ)


/*
 * Sv39 MMU
 */
#define	PTSHIFT		(PGSHIFT-3)		/* log2(entries per page-table page): 9 */
#define	PTLEVELS	3			    /* Sv39 is always exactly 3 levels */
#define	PTLX(v, l)	(((v) >> (PGSHIFT + (l)*PTSHIFT)) & ((1<<PTSHIFT)-1))	/* VPN[l] */
#define	PGLSZ(l)	(1ULL << (PGSHIFT + (l)*PTSHIFT))	/* bytes mapped by one level-l leaf */

/* PTE bits - RISC-V Sv39, bits 0-7, spec-defined */
#define	PTEVALID	(1<<0)
#define	PTEREAD		(1<<1)
#define	PTEWRITE	(1<<2)
#define	PTEEXEC		(1<<3)
#define	PTEUSER		(1<<4)
#define	PTEGLOBAL	(1<<5)
#define	PTEACCESSED	(1<<6)
#define	PTEDIRTY	(1<<7)

#define	PTEPTR		(PTEVALID)				/* V=1, R=W=X=0: points to next level */
#define	PTELEAF		(PTEVALID|PTEREAD|PTEWRITE|PTEEXEC|PTEACCESSED|PTEDIRTY|PTEGLOBAL)	/* TODO while testing: full RWX, no real permission enforcement yet */

/* PPN occupies PTE bits 53:10 */
#define	PTEPPNSHIFT	10
#define	PPN(pte)	(((uintptr)(pte) >> PTEPPNSHIFT) << PGSHIFT)	/* PTE -> the physical addr it points to */
#define	PAPTE(pa)	(((uintptr)(pa) >> PGSHIFT) << PTEPPNSHIFT)	/* physical addr -> its PPN field, OR with flags */

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

