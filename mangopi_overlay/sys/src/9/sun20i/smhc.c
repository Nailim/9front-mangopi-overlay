/*
 * Allwinner SMHC - mmc0 - SD card slot (mmc1 - WiFi SDIO).
 *
 * Structure inspired by Richard Miller's bcm/emmc.c, registers
 * based on U-Boot's drivers/mmc/sunxi_mmc.{c,h}.
 */

#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "io.h"
#include "../port/error.h"
#include "../port/sd.h"


enum {					/* registers, u32int indices */
	Gctrl		= 0x000/4,
	Clkcr		= 0x004/4,
	Timeout		= 0x008/4,
	Width		= 0x00c/4,
	Blksz		= 0x010/4,
	Bytecnt		= 0x014/4,
	Cmd		    = 0x018/4,
	Arg		    = 0x01c/4,
	Resp0		= 0x020/4,
	Resp1		= 0x024/4,
	Resp2		= 0x028/4,
	Resp3		= 0x02c/4,
	Imask		= 0x030/4,
	Rint		= 0x038/4,
	Status		= 0x03c/4,
	Sampdl		= 0x144/4,
	Fifo		= 0x200/4,
};

enum {					/* Gctrl */
	Softreset	= 1<<0,
	Fiforeset	= 1<<1,
	Dmareset	= 1<<2,
	Ctrlreset	= Softreset|Fiforeset|Dmareset,
	Accessbyahb	= 1<<31,
};

enum {					/* Clkcr */
	Clkdivmask	= 0xff,
	Clkenable	= 1<<16,
};

enum {					/* Cmd */
	Respexpire	= 1<<6,
	Longresp	= 1<<7,
	Chkrespcrc	= 1<<8,
	Dataexpire	= 1<<9,
	Cmdwrite	= 1<<10,
	Autostop	= 1<<12,
	Waitpreover	= 1<<13,
	Sendinitseq	= 1<<15,
	Upclkonly	= 1<<21,
	Cmdstart	= 1<<31,
};

enum {					/* Rint */
	Commanddone	= 1<<2,
	Dataover	= 1<<3,
	Autocmddone	= 1<<14,
	Cardinsert	= 1<<30,

	Interr		= 1<<1 | 1<<6 | 1<<7 | 1<<8 | 1<<9
			| 1<<10 | 1<<11 | 1<<12 | 1<<13 | 1<<15,
};

enum {					/* Status */
	Fifoempty	= 1<<2,
	Fifofull	= 1<<3,
	Cardpresent	= 1<<8,
	Carddatabusy	= 1<<9,
};

enum {
	Caldlswen	= 1<<7,		/* Sampdl, D1 calibration */
};

static u32int *regs;

static int
updateclk(void)
{
	ulong now;

	regs[Cmd] = Cmdstart|Upclkonly|Waitpreover;
	coherence();
	for(now = m->ticks; regs[Cmd] & Cmdstart; )
		if((long)(m->ticks - now) > HZ)
			return -1;
	regs[Rint] = regs[Rint];	/* the update sets spurious irq bits */
	return 0;
}

static void
smhcclock(int hz)
{
	u32int v;

	v = regs[Clkcr] & ~Clkdivmask;
	regs[Clkcr] = v & ~Clkenable;
	coherence();
	updateclk();

	mmc0clock(hz);			/* CCU side */

	regs[Clkcr] = v & ~Clkenable;	/* internal divider 0 */
	regs[Sampdl] = Caldlswen;	    /* D1 uses delay calibration */
	coherence();
	regs[Clkcr] = v | Clkenable;
	coherence();
	updateclk();
}

static int
smhcinit(SDio*)
{
	regs = KADDR(PHYSMMC0);
	mmc0enable();			/* AHB gate on, out of reset */

	regs[Gctrl] = Ctrlreset;
	coherence();
	delay(1);
	if(regs[Gctrl] & Ctrlreset)	/* reset bits self-clear */
		return -1;

	regs[Gctrl] = Accessbyahb;	/* FIFO access, not DMA */
	regs[Imask] = 0;
	regs[Rint] = ~0;
	regs[Timeout] = ~0;
	regs[Width] = 0;
	coherence();
	return 0;
}

static void
smhcenable(SDio*)
{
	smhcclock(400000);		/* identification speed */
}

static int
smhcinquiry(SDio*, char *inquiry, int inqlen)
{
	return snprint(inquiry, inqlen, "Allwinner SMHC");
}

static int
smhccmd(SDio*, SDiocmd *cmd, u32int arg, u32int *resp)
{
	u32int c, i;
	ulong now;

	/*
	 * Autostop issues CMD12 for us on multiblock transfers; sending
	 * it again confuses the controller (sunxi_mmc.c does the same).
	 */
	if(cmd->index == 12)
		return 0;

	c = Cmdstart | cmd->index;
	if(cmd->index == 0)
		c |= Sendinitseq;
	switch(cmd->resp){
	case 0:
		break;
	case 2:
		c |= Respexpire|Longresp|Chkrespcrc;
		break;
	case 3:			/* R3 carries no CRC */
		c |= Respexpire;
		break;
	default:
		c |= Respexpire|Chkrespcrc;
		break;
	}
	if(cmd->data){
		c |= Dataexpire|Waitpreover;
		if((cmd->data & 1) == 0)
			c |= Cmdwrite;
		if(cmd->data > 2)
			c |= Autostop;
	}

    /* the card may still be programming a previous write */
	for(now = m->ticks; regs[Status] & Carddatabusy; ){
		if((long)(m->ticks - now) > 5*HZ){
			print("smhc: %s card busy, status %ux\n", cmd->name, regs[Status]);
			error(Eio);
		}
    }

	regs[Rint] = ~0;
	regs[Arg] = arg;
	coherence();
	regs[Cmd] = c;
	coherence();

	for(now = m->ticks;;){
		i = regs[Rint];
		if(i & (Commanddone|Interr))
			break;
		if((long)(m->ticks - now) > HZ){
			print("smhc: %s timeout rint %ux status %ux\n",
				cmd->name, i, regs[Status]);
			error(Eio);
		}
	}
	if(i & Interr){
		print("smhc: %s error rint %ux status %ux\n",
			cmd->name, i, regs[Status]);
		error(Eio);
	}

	switch(cmd->resp){
	case 0:
		resp[0] = 0;
		break;
	case 2:				/* no shifting - see file comment */
		resp[0] = regs[Resp0];
		resp[1] = regs[Resp1];
		resp[2] = regs[Resp2];
		resp[3] = regs[Resp3];
		break;
	default:
		resp[0] = regs[Resp0];
		break;
	}
	if(cmd->busy)
		for(now = m->ticks; regs[Status] & Carddatabusy; )
			if((long)(m->ticks - now) > 3*HZ)
				error(Eio);
	return 0;
}

static void
smhciosetup(SDio*, int, void *buf, int bsize, int bcount)
{
	if((uintptr)buf & 3)
		error(Eio);		/* controller needs word alignment */
	regs[Blksz] = bsize;
	regs[Bytecnt] = bsize * bcount;
	coherence();
}

static void
smhcio(SDio*, int write, uchar *buf, int len)
{
	u32int *p, *e;
	ulong now;

	p = (u32int*)buf;
	e = p + len/4;
	for(now = m->ticks; p < e; ){
		if((long)(m->ticks - now) > 5*HZ)
			error(Eio);
		if(regs[Rint] & Interr)
			error(Eio);
		if(write){
			if((regs[Status] & Fifofull) == 0)
				regs[Fifo] = *p++;
		}else{
			if((regs[Status] & Fifoempty) == 0)
				*p++ = regs[Fifo];
		}
	}
	for(now = m->ticks;;){
		if(regs[Rint] & (Dataover|Autocmddone))
			break;
		if(regs[Rint] & Interr)
			error(Eio);
		if((long)(m->ticks - now) > 5*HZ)
			error(Eio);
	}
}

static void
smhcbus(SDio*, int width, int speed)
{
	switch(width){
	case 1:
		regs[Width] = 0;
		break;
	case 4:
		regs[Width] = 1;
		break;
	case 8:
		regs[Width] = 2;
		break;
	}
	if(speed)
		smhcclock(speed);
}

void
smhclink(void)
{
	static SDio io = {
		"smhc",
		smhcinit,
		smhcenable,
		smhcinquiry,
		smhccmd,
		smhciosetup,
		smhcio,
		smhcbus,
	};
	addmmcio(&io);
}
