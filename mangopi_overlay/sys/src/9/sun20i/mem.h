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

#define KZERO 0x41000000
#define KTZERO KZERO
#define MACHSIZE 8192

/*
 * Set by SBI, make it work now, scale later
 */
#define TIMEBASEFREQ	24000000
#define TIMERHZ		1
#define TICKINTERVAL	(TIMEBASEFREQ/TIMERHZ)
