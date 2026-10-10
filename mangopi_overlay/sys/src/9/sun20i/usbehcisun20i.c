#include	"u.h"
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"io.h"
#include	"../port/error.h"
#include	"../port/usb.h"
#include	"usbehci.h"


enum {
	Pmu1		= PHYSEHCI1 + 0x800,	/* USB1 PHY "pmu" registers */
	Pmucfg		= 0x000/4,		/* passby / AHB burst control */
	Hciphyctl	= 0x010/4,

	Ulpibypass	= 1<<0,		/* phy-sun4i-usb.c SUNXI_ULPI_BYPASS_EN */
	Incrxalign	= 1<<8,		/* SUNXI_AHB_INCRX_ALIGN_EN */
	Incr4burst	= 1<<9,		/* SUNXI_AHB_INCR4_BURST_EN */
	Ichr8		= 1<<10,	/* SUNXI_AHB_ICHR8_EN */

	Siddq		= 1<<3,		/* PHY_CTL_SIDDQ: 1 = PHY powered down */
};

/*
 * Bring USB1's PHY out of power-down and put the controller on the AHB passby path.
 * For the D1 this is all PHY1 needs: phy-sun4i-usb.c's
 * sun20i_d1_cfg is type sun50i_h6_phy, whose bit-banged PHY_CTL
 * sequence applies only to PHY0.  usb1enable() has already ungated the
 * clocks and released RST_USB_PHY1.
 */
static void
phy1enable(void)
{
	ulong *pmu;

	pmu = KADDR(Pmu1);
	pmu[Hciphyctl] &= ~Siddq;
	coherence();
	pmu[Pmucfg] |= Ichr8|Incr4burst|Incrxalign|Ulpibypass;
	coherence();
	microdelay(100);
}

/*
 * Reset the controller to a known state and pick up the frame-list
 * size.  Without this ctlr->nframes stays 0 and ehcimeminit() has
 * nothing to allocate.
 * Modelled on zynq/usbehcizynq.c omap/usbehciomap.c
 */
static void
ehcireset(Ctlr *ctlr)
{
	Eopio *opio;
	int i;

	ilock(ctlr);
	opio = ctlr->opio;

	ehcirun(ctlr, 0);			/* stop before resetting */

	/* clear the high 32 bits of address signals if 64-bit capable */
	if((ctlr->capio->capparms & C64) != 0)
		opio->seg = 0;

	opio->cmd |= Chcreset;
	coherence();
	for(i = 0; i < 100; i++){
		if((opio->cmd & Chcreset) == 0)
			break;
		delay(1);
	}
	if(i == 100)
		print("usbehci: controller reset timed out\n");

	opio->cmd = (opio->cmd & ~Citcmask) | Citc8;
	coherence();

	switch(opio->cmd & Cflsmask){
	case Cfls1024:
		ctlr->nframes = 1024;
		break;
	case Cfls512:
		ctlr->nframes = 512;
		break;
	case Cfls256:
		ctlr->nframes = 256;
		break;
	default:
		panic("usbehci: unknown fls %#lux", opio->cmd & Cflsmask);
	}
	dprint("usbehci: %d frames\n", ctlr->nframes);
	iunlock(ctlr);
}


static void
setdebug(Hci*, int d)
{
	ehcidebug = d;
}

static void
shutdown(Hci *hp)
{
	int i;
	Ctlr *ctlr;
	Eopio *opio;

	ctlr = hp->aux;
	ilock(ctlr);
	opio = ctlr->opio;
	opio->cmd |= Chcreset;
	coherence();
	for(i = 0; i < 100; i++){
		if((opio->cmd & Chcreset) == 0)
			break;
		delay(1);
	}
	if(i == 100)
		print("usbehci: reset timed out\n");
	iunlock(ctlr);
}

static int
reset(Hci *hp)
{
	Ctlr *ctlr;
	Ecapio *capio;
	static int beenhere;

	if(beenhere)
		return -1;
	beenhere = 1;

	if(getconf("*nousbehci") != nil)
		return -1;

	usb1enable();			/* CCU gates and resets */
	phy1enable();			/* PHY out of power-down */

	ctlr = smalloc(sizeof(Ctlr));
	ctlr->capio = capio = KADDR(PHYSEHCI1);
	ctlr->opio = (Eopio*)((uintptr)capio + (capio->cap & 0xff));

	hp->aux = ctlr;
	hp->port = PHYSEHCI1;
	hp->irq = EHCI1IRQ;
	hp->nports = capio->parms & Cnports;

	ctlr->tdalloc = ucallocalign;
	ctlr->dmaalloc = ucalloc;
	ctlr->dmafree = ucfree;

	ehcireset(ctlr);        /* <- HCRESET to a known state */
	ehcimeminit(ctlr);      /* <- frame list, QH ring, periodic tree */

	ehcilinkage(hp);
	hp->shutdown = shutdown;
	hp->debug = setdebug;

	intrenable(hp->irq, hp->interrupt, hp, 0, hp->type);
	return 0;
}

void
usbehcilink(void)
{
	addhcitype("ehci", reset);
}
