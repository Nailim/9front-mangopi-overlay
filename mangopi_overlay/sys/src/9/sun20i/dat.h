typedef struct Mach     Mach;
typedef struct Label	Label;
typedef struct Proc     Proc;
typedef struct Ureg     Ureg;

#pragma incomplete Proc		/* defined by ../port/portdat.h - to be included later */
#pragma incomplete Ureg

struct Mach
{
	int	machno;		/* physical id of processor */
};

struct Label
{
	uintptr	sp;
	uintptr	pc;
};

extern register Mach* m;	/* R7 */
extern register Proc* up;	/* R6 */

