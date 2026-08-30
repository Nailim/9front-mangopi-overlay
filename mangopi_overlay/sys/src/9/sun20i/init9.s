TEXT main(SB), 1, $-8
	MOVW	$setSB(SB), R3		/* SB */
	MOVW	$boot(SB), R8		/* arg0 - startboot ignores it */
	JAL     R0, startboot(SB)	/* tail jump: link to R0 discards it */

