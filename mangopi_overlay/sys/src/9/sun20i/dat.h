typedef struct Mach	Mach;
typedef struct Label	Label;
typedef struct Ureg	Ureg;

#pragma incomplete Ureg

struct Mach
{
	int	machno;		/* physical id of processor */
};

extern Mach* m;     /* just point to the single core */

struct Label
{
	uintptr	sp;
	uintptr	pc;
};
