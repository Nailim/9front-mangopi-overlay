#include "../port/portfns.h"

#define	cycles(ip)	(*(ip) = rdcycle())

#define	userureg(ur)	(((ur)->status & SSTATUS_SPP) == 0)

/*
 * Port-specific function prototypes.
 */

/* l.s */
void	setstvec(void*);
void	trapvec(void);			/* address taken only */
void	highstart(void);		/* address taken only */
void	intrenable(void);
uintptr	rdtime(void);
void	sfencevma(void);
int	    tas(void*);
int	    cmpswap(long*, long, long);
void	coherence(void);
int	    setlabel(Label*);
void	idlehands(void);
uintptr	getcallerpc(void*);
void	dummy(unsigned int);
void satpset(uintptr);
void fencei(void);
void forkret(void);

/* main.c */
void	main(void);
void	uart_putc(int);
void	uart_puts(char*);
void	uart_puthex64(unsigned long long);
void	uarthigh(void);

/* trap.c */
void	trapinit(void);
void	trap(Ureg*);

/* mmu.c */
uintptr	mmubootstrap(void);
void*	mmuhighentry(void);
void	mmulowdrop(void);
uintptr dramsize(void);

/* timer.c */
void	timer0init(ulong);
void	timer0ack(void);

/* plic.c */
void	plicinit(void);
void	plicenable(int, int);
int     plicclaim(void);
void	pliccomplete(int);

/* wdt.c */
void	wdt_main_enable(unsigned int);
void	wdt_main_disable(void);
void	wdt_main_feed(void);
void	wdt_riscv_enable(unsigned int);
void	wdt_riscv_disable(void);
void	wdt_riscv_feed(void);

/* ccu.c */
void cpuclockinit(int);

/* things called from port*/
#define	getpgcolor(a)	0
#define	kmapinval()

/* DRAM mapped by KZERO - kmap is linear-map address */
#define	kmap(p)		((KMap*)KADDR((p)->pa))
#define	kunmap(k)	USED(k)
#define	VA(k)		((uintptr)(k))

/* stub.c */
char*	getconf(char*);
int     isaconfig(char*, int, ISAConf*);
uintptr cankaddr(uintptr pa);
void    evenaddr(uintptr);
void    procfork(Proc*);
void    procsetup(Proc *);
void    procsave(Proc*);
void    procrestore(Proc*);
void    dumpstack(void);
void    exit(int);
void	rebootcmd(int, char**);
uvlong	fastticks(uvlong*);
int		mntversion(Chan*, char*, int, int);
Chan*	mntauth(Chan*, char*);
void	srvrenameuser(char*, char*);
void	shrrenameuser(char*, char*);
void    mntrahinit(Mntrah *rah);
long    mntrahread(Mntrah *rah, Chan *c, uchar *buf, long len, vlong off);
int		needpages(void*);
void	muxclose(Mnt*);
ulong	µs(void);
void	putswap(Page*);
int		swapcount(uintptr);
void	putmmu(uintptr, uintptr, Page*);
uintptr	userpc(void);
void    checkmmu(uintptr, uintptr);
void	kickpager(void);
void	timerset(Tval);
void	flushmmu(void);
void	mmurelease(Proc*);
void	mmuswitch(Proc*);
ulong	perfticks(void);
void	closeegrp(Egrp*);
uintptr	dbgpc(Proc*);
void	kprocchild(Proc*, void (*)(void));
void	dupswap(Page*);
Chan*	mntattach(Chan*, Chan*, char*, int);
Egrp*	newegrp(void);
void	envcpy(Egrp*, Egrp*);
void	forkchild(Proc*, Ureg*);
uintptr	execregs(uintptr, int, char**, Tos*);
void	fpunotify(Proc*);
void	fpunoted(Proc*);
void	bootlinks(void);
void	links(void);
void	callwithureg(void(*)(Ureg*));
void    touser(uintptr);

void    syscall(Ureg*);
void    setregisters(Ureg*, char*, char*, int);
void    uartconsinit(void);
void    uartpoll(void);
 uintptr rdcycle(void);
 
