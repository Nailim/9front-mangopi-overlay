#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "io.h"



enum {
	/* 1280x720@60, 74.25 MHz, hsync+ vsync+ (CEA VIC 4) */
	Hactive	= 1280,
	Hfp	= 110,
	Hsw	= 40,
	Hbp	= 220,
	Vactive	= 720,
	Vfp	= 5,
	Vsw	= 5,
	Vbp	= 20,
	Htotal	= Hactive+Hfp+Hsw+Hbp,				/* 1650 */
	Vtotal	= Vactive+Vfp+Vsw+Vbp,				/* 750 */
	Outsize	= (Vactive-1)<<16 | (Hactive-1),	/* mixer: height high */
	Tconsize = (Hactive-1)<<16 | (Vactive-1),	/* TCON: width high */


	Black	= 0xff000000,			/* opaque, XRGB8888 */
};

enum {
	/* CCU word indexes */
	PllVideo0	= 0x040/4,
	DeClk		= 0x600/4,
	DeBus		= 0x60c/4,
	DpssBus		= 0xabc/4,
	TconTvClk	= 0xb80/4,
	TconTvBus	= 0xb9c/4,

	PllEn		= 1<<31,
	PllLdo		= 1<<30,
	PllLockEn	= 1<<29,
	PllLock		= 1<<28,
	PllOut		= 1<<27,
	VideoN		= 98<<8,	/* x99 */
	VideoM		= 1<<1,		/* input /2: 24*99/2 = 1188 MHz */
};

enum {
	/* mixer0 registers */
	GlbCtl		= 0x0000,
	GlbDbuff	= 0x0008,
	GlbSize		= 0x000c,
	BldPipe		= 0x1000,
	BldFcolor	= 0x1004,
	BldInsize	= 0x1008,
	BldCoord	= 0x100c,
	BldRoute	= 0x1080,
	BldBkcolor	= 0x1088,
	BldOutsize	= 0x108c,
	BldMode		= 0x1090,
	BldOutctl	= 0x10fc,
	UiAttr		= 0x3000,	/* channel 1, layer 0 */
	UiSize		= 0x3004,
	UiCoord		= 0x3008,
	UiPitch		= 0x300c,
	UiLaddr		= 0x3010,
	UiOvlsize	= 0x3088,

	PipeFc		= 1<<0,		/* pipe 0 fill colour */
	PipeEn		= 1<<8,		/* pipe 0 enable */
	UiEn		= 1<<0,
	UiXrgb8888	= 4<<8,
	UiAlpha		= 0xff<<24,

	/* TCON TOP / TCON TV0 registers */
	TopPortSel	= 0x1c,
	TopGateSrc	= 0x20,
	TvGctl		= 0x00,
	TvInt0		= 0x04,
	TvInt1		= 0x08,
	TvPol		= 0x88,
	TvIotri		= 0x8c,
	TvCtl		= 0x90,
	TvBasic0	= 0x94,		/* BASIC1, BASIC2 follow */
	TvBasic3	= 0xa0,
	TvBasic4	= 0xa4,
	TvBasic5	= 0xa8,
	TvDatatri0	= 0x338,
	TvDatatri1	= 0x33c,

	TconEn		= 1<<31,
	TvDelay		= (Vtotal-Vactive-2)<<4,	/* start delay, 28 here, max 30 */
};

static void
wr(ulong *b, ulong off, ulong v)
{
	b[off/4] = v;
}

static int
waitlock(ulong *ccu)
{
	int i;

	for(i = 0; i < 100000; i++)
		if(ccu[PllVideo0] & PllLock)
			return 0;
	return -1;
}

static int
clockinit(void)
{
	ulong *ccu, *de;

	ccu = KADDR(PHYSCCU);
	ccu[PllVideo0] = PllEn|PllLdo|VideoN|VideoM;
	ccu[PllVideo0] &= ~PllLockEn;
	ccu[PllVideo0] |= PllLockEn;
	if(waitlock(ccu) < 0){
		print("hdmi: PLL_VIDEO0 no lock, %#.8lux\n", ccu[PllVideo0]);
		ccu[PllVideo0] &= ~PllEn;
		return -1;
	}
	ccu[PllVideo0] |= PllOut;

	ccu[DeClk]     = 0x81000003;	/* gate, PLL_VIDEO0 4x, /4 = 297 MHz */
	ccu[DeBus]     = 0x00010001;	/* gate + reset released */
	ccu[DpssBus]   = 0x00010001;
	ccu[TconTvClk] = 0x80000003;	/* gate, PLL_VIDEO0 1x, /4 = 74.25 MHz */
	ccu[TconTvBus] = 0x00010001;

	/* DE clock block: mixer0 sclk gate, hclk gate, ahb reset */
	de = KADDR(PHYSDE);
	de[0] = 1;
	de[1] = 1;
	de[2] = 1;

	return 0;
}

static void
mixerinit(void)
{
	ulong *m;
	int i;

	m = KADDR(PHYSMIXER0);
	for(i = 0; i < 0x6000/4; i++)	/* zero GLB, BLD, channel pages: writes only */
		m[i] = 0;
	/* post-processing enables (0xa0000...) skipped on purpose */

	wr(m, GlbCtl, 1);				/* RT_EN */
	wr(m, GlbSize, Outsize);
	wr(m, BldBkcolor, Black);
	wr(m, BldFcolor, Black);
	wr(m, BldMode, 0x03010301);		/* BLEND_MODE, two pipes */
	wr(m, BldMode+4, 0x03010301);
	wr(m, BldRoute, 1);				/* pipe 0 <- channel 1 */
	wr(m, BldPipe, PipeEn|PipeFc);
	wr(m, BldOutsize, Outsize);
	wr(m, BldOutctl, 0);			/* progressive */
	wr(m, GlbDbuff, 1);				/* commit */
}

static void
tconinit(void)
{
	ulong *top, *tv;

	top = KADDR(PHYSTCONTOP);
	wr(top, TopPortSel, 0);
	wr(top, TopGateSrc, 0);
	wr(top, TopPortSel, 2);				/* mixer0 -> TCON TV0 */
	wr(top, TopGateSrc, 0x10100000);	/* HDMI src = TV0, TV0 gate */

	tv = KADDR(PHYSTCONTV0);
	wr(tv, TvGctl, 0);
	wr(tv, TvInt0, 0);
	wr(tv, TvInt1, 0);
	wr(tv, TvIotri, ~0);
	wr(tv, 0xf4, ~0);					/* second tri-state word, as in the capture */
	wr(tv, TvCtl, TvDelay);
	wr(tv, TvBasic0,   Tconsize);		/* BASIC0..2: source, layer, output size */
	wr(tv, TvBasic0+4, Tconsize);
	wr(tv, TvBasic0+8, Tconsize);
	wr(tv, TvBasic3, (Htotal-1)<<16 | (Hsw+Hbp-1));
	wr(tv, TvBasic4, (Vtotal*2)<<16 | (Vsw+Vbp-1));
	wr(tv, TvBasic5, (Hsw-1)<<16 | (Vsw-1));
	wr(tv, TvPol, 0x03000000);			/* hsync+ vsync+ */
	wr(tv, TvDatatri0, 0);				/* enable R/Cb and G/Y data outputs */
	wr(tv, TvDatatri1, 0);				/* enable B/Cr data output */

	wr(tv, TvGctl, 1);					/* IOMAP: TCON1 */
	wr(tv, TvGctl, TconEn|1);
	wr(tv, TvCtl, TconEn|TvDelay);		/* channel 1 enable */
}

enum {
	HdmiClk	= 0xb04/4,
	HdmiBus	= 0xb1c/4,

	/* PHY registers (byte offsets) */
	PhyRext	= 0x04,
	PhyCtl0	= 0x40,
	PhyCtl1	= 0x44,
	PhyCtl2	= 0x48,
	PhyCtl3	= 0x4c,
	PhyCtl4	= 0x50,
	PhyCtl5	= 0x54,
	PllCtl0	= 0x58,
	PllCtl1	= 0x5c,
	PhySts	= 0x80,
	PhyCtl6	= 0x9c,
	PhyCtl7	= 0xa0,
	PhyDbg	= 0x6c,

	StsRcal	= 1<<16,
	StsLock	= 1<<4,
	StsTx	= 1<<0,
};

/* set bits hi..lo of register at byte offset off to v */
static void
fld(ulong *b, ulong off, int hi, int lo, ulong v)
{
	ulong m, r;

	m = ((1UL<<(hi-lo+1)) - 1) << lo;
	r = b[off/4];
	b[off/4] = (r & ~m) | ((v << lo) & m);
}

static int
phywait(ulong *phy, ulong bit, char *what)
{
	int i;

	for(i = 0; i < 2000; i++){
		microdelay(5);
		if(phy[PhySts/4] & bit)
			return 0;
	}
	print("hdmi: phy %s TIMEOUT sts %#.8lux\n", what, phy[PhySts/4]);
	return -1;
}

static int
phyinit(ulong *p)
{
	/* REXT calibration resistor enable, then the upper half = 0x80c0 */
	p[PhyRext/4] |= 1<<31;
	p[PhyRext/4] = p[PhyRext/4] & 0xffff | 0x80c00000;

	fld(p, PhyCtl5, 19, 16, 0xf);		/* p1opt: all channels */

	/* reset everything off */
	fld(p, PhyCtl0, 27, 24, 0);		/* entx */
	fld(p, PhyCtl5, 10, 10, 0);		/* enresck */
	fld(p, PhyCtl5, 9, 9, 0);		/* enres */
	fld(p, PhyCtl5, 7, 4, 0);		/* enp2s */
	fld(p, PhyCtl0, 16, 16, 0);		/* enck */
	fld(p, PhyCtl0, 23, 20, 0);		/* enbi */
	fld(p, PhyCtl5, 0, 0, 0);		/* encalog */
	fld(p, PhyCtl5, 8, 8, 0);		/* enrcal */
	fld(p, PhyCtl0, 17, 17, 0);		/* enldo_fs */
	fld(p, PhyCtl0, 18, 18, 0);		/* enldo */
	fld(p, PhyCtl5, 1, 1, 0);		/* enib */
	fld(p, PllCtl1, 11, 11, 1);		/* reset */
	fld(p, PllCtl1, 12, 12, 0);		/* pwron */
	fld(p, PllCtl0, 28, 28, 0);		/* envbs */

	/* PLL setup */
	fld(p, PllCtl0, 31, 30, 3);		/* cko_sel */
	fld(p, PllCtl0, 29, 29, 1);		/* bypass_ppll */
	fld(p, PllCtl1, 8, 8, 1);		/* drv_ana */
	fld(p, PllCtl1, 31, 31, 0);		/* clock source: PLL_video */
	fld(p, PllCtl1, 18, 18, 0);		/* sdm_en */
	fld(p, PllCtl1, 10, 10, 0);		/* sckref */
	fld(p, PllCtl0, 26, 24, 4);		/* slv */
	fld(p, PllCtl0, 2, 0, 7);		/* prop_cntrl */
	fld(p, PllCtl0, 5, 4, 3);		/* gmp_cntrl */
	fld(p, PllCtl1, 5, 4, 0);		/* ref_cntrl */
	fld(p, PllCtl0, 8, 8, 1);		/* vcorange */

	/* dividers */
	fld(p, PllCtl0, 15, 12, 0);		/* div_pre */
	fld(p, PllCtl1, 19, 19, 0);		/* pcnt_en */
	fld(p, PllCtl1, 27, 20, 1);		/* pcnt_n */
	fld(p, PllCtl1, 17, 16, 0);		/* pixel_rep */
	fld(p, PllCtl0, 22, 22, 0);		/* bypass_clrdpth */
	fld(p, PllCtl0, 21, 20, 0);		/* clr_dpth */
	fld(p, PllCtl0, 7, 6, 1);		/* n_cntrl */
	fld(p, PllCtl0, 17, 17, 0);		/* div2_ckbit */
	fld(p, PllCtl0, 16, 16, 0);		/* div2_cktmds */
	fld(p, PllCtl0, 23, 23, 0);		/* bcr */
	fld(p, PllCtl1, 12, 12, 1);		/* pwron */
	fld(p, PllCtl1, 11, 11, 0);		/* reset released */

	/* analog configuration words */
	p[PhyCtl1/4] &= 0xffc0ffff;
	p[PhyCtl2/4] &= 0xff000000;
	p[PhyCtl3/4] = p[PhyCtl3/4] & 0xffff0000 | 0xffff;
	p[PhyCtl4/4] = p[PhyCtl4/4] & 0xe0000000 | 0x0c0d0d0d;

	fld(p, PhyCtl6, 31, 31, 0);		/* switch_clkch_data */
	fld(p, PhyCtl6, 9, 0, 0x3ff);	/* clk_greate0_340m */
	fld(p, PhyCtl6, 19, 10, 0x3ff);	/* clk_greate1_340m */
	fld(p, PhyCtl6, 29, 20, 0);		/* clk_greate2_340m */
	fld(p, PhyCtl7, 9, 0, 0);		/* clk_greate3_340m */
	fld(p, PhyCtl7, 21, 12, 0x3e0);	/* clk_low_340m */
	fld(p, PhyCtl6, 30, 30, 1);		/* en_ckdat */
	fld(p, PhyCtl1, 27, 27, 0);		/* res_scktmds */
	fld(p, PhyCtl0, 7, 6, 2);		/* reg_csmps */
	fld(p, PhyCtl0, 5, 5, 0);		/* reg_ck_test_sel */
	fld(p, PhyCtl0, 4, 4, 1);		/* reg_ck_sel */
	fld(p, PhyDbg, 1, 0, 0);		/* txdata_debugmode */

	/* enable, in this order, waiting where the status says to */
	fld(p, PhyCtl4, 31, 29, 4);		/* reg_slv */
	fld(p, PhyCtl5, 1, 1, 1);		/* enib */
	fld(p, PhyCtl0, 18, 18, 1);		/* enldo */
	fld(p, PhyCtl0, 17, 17, 1);		/* enldo_fs */
	fld(p, PhyCtl5, 8, 8, 1);		/* enrcal */
	fld(p, PhyCtl5, 0, 0, 1);		/* encalog */
	if(phywait(p, StsRcal, "rcal") < 0)
		return -1;
	fld(p, PhyCtl0, 23, 20, 0xf);		/* enbi */
	if(phywait(p, StsLock, "pll lock") < 0)
		return -1;
	fld(p, PhyCtl0, 16, 16, 1);		/* enck */
	fld(p, PhyCtl5, 7, 4, 0xf);		/* enp2s */
	fld(p, PhyCtl5, 9, 9, 1);		/* enres */
	fld(p, PhyCtl5, 10, 10, 1);		/* enresck */
	fld(p, PhyCtl0, 27, 24, 0xf);	/* entx */
	if(phywait(p, StsTx, "tx ready") < 0)
		return -1;

	fld(p, PhyCtl0, 0, 0, 1);		/* sda_en */
	fld(p, PhyCtl0, 1, 1, 1);		/* scl_en */
	fld(p, PhyCtl0, 2, 2, 1);		/* hpd_en */
	fld(p, PhyCtl0, 11, 8, 0xf);	/* reg_den */
	fld(p, PllCtl0, 28, 28, 1);		/* envbs */
	return 0;
}

typedef struct Hw Hw;
struct Hw {
	ushort	off;
	uchar	v;
};

static uchar *hcore;

static void
cw(ulong off, int v)
{
	hcore[off] = v;
}

static void
cw2(ulong off, int v)		/* 16-bit field, low byte first */
{
	hcore[off] = v;
	hcore[off+1] = v>>8;
}

static void
fcinit(void)
{
	cw(0x1000, 0x78);		/* INVIDCONF: hsync+ vsync+, DE high, HDMI mode */
	cw2(0x1001, Hactive);
	cw2(0x1003, Htotal-Hactive);	/* HBLANK */
	cw2(0x1005, Vactive);
	cw(0x1007, Vtotal-Vactive);	/* VBLANK */
	cw2(0x1008, Hfp);		/* HSYNC delay */
	cw2(0x100a, Hsw);		/* HSYNC width */
	cw(0x100c, Vfp);		/* VSYNC delay */
	cw(0x100d, Vsw);		/* VSYNC width */
}

static void
cwtab(Hw *t, int n)
{
	int i;

	for(i = 0; i < n; i++)
		hcore[t[i].off] = t[i].v;
}

/* interrupt mutes and masks: polling only */
static Hw hmute[] = {
	{0x0180, 0xff}, {0x0181, 0xff}, {0x0182, 0x03}, {0x0183, 0x0f},
	{0x0184, 0x02}, {0x0185, 0x03}, {0x0186, 0x7f}, {0x0187, 0xff},
	{0x0188, 0x03},
	{0x0807, 0xff},					/* VP_MASK */
	{0x10d2, 0xff}, {0x10d6, 0xff}, {0x10da, 0x13},	/* FC_MASK0..2 */
	{0x3006, 0x01},					/* PHY_MASK0 */
	{0x3102, 0x0c},					/* AUD_INT */
};

/* dw_hdmi_enable_video_path, first half: control periods and preambles */
static Hw hpath[] = {
	{0x1011, 0x0c}, {0x1012, 0x20}, {0x1013, 0x01},
	{0x1014, 0x0b}, {0x1015, 0x16}, {0x1016, 0x21},
};

/* AVI infoframe (RGB, 16:9, VIC 4), packet scheduling, packetizer, TX input */
static Hw hinfo[] = {
	{0x1019, 0x60}, {0x101a, 0x28}, {0x101b, 0x04}, {0x101c, 0x04},
	{0x102a, 0x1b},				    /* VSDSIZE */
	{0x1073, 0x0f}, {0x1074, 0x03},	/* CTRLQHIGH/LOW */
	{0x10b7, 0x7b},				    /* DATAUTO3 */
	{0x10e0, 0x10},				    /* PRCONF */
	{0x1103, 0x10},				    /* GMD_CONF */
	{0x10e3, 0x1f},				    /* PACKET_TX_EN */
	{0x0801, 0x40}, {0x0802, 0x27}, {0x0804, 0x47},	/* video packetizer */
	{0x4101, 0x01},				    /* CSC scale */
	{0x4102, 0x1b}, {0x4103, 0x7c}, {0x4109, 0x20},	/* limited range RGB: */
	{0x410c, 0x1b}, {0x410d, 0x7c}, {0x4111, 0x20},	/* diag 0x1b7c, off 0x20 */
	{0x4116, 0x1b}, {0x4117, 0x7c}, {0x4119, 0x20},
	{0x411a, 0xff}, {0x411b, 0xff},
	{0x0200, 0x01}, {0x0201, 0x07},	/* TX_INVID0: RGB888, stuffing */
};



static int
coreinit(ulong *phy)
{
	hcore = KADDR(PHYSHDMI);

	cwtab(hmute, nelem(hmute));
	fcinit();			/* Linux order: frame composer, ... */
	if(phyinit(phy) < 0)		/* ... the PHY (it prints why), ... */
		return -1;
	cwtab(hpath, nelem(hpath));	/* ... then the video path */
	cw(0x4001, 0x7f);		/* MC_CLKDIS: all gated */
	cw(0x4001, 0x7e);		/* pixel clock on */
	cw(0x4001, 0x7c);		/* tmds clock on */
	cw(0x4001, 0x6c);		/* csc clock on (0x20 cec stays off) */
	cw(0x4004, 0x01);		/* MC_FLOWCTRL: csc in path */
	cwtab(hinfo, nelem(hinfo));
	/* overflow workaround: tmds reset, then one FC write */
	cw(0x4002, 0xdf);		/* MC_SWRSTZ: observed value */
	cw(0x1000, 0x78);
	return 0;
}

static int
hdmitxinit(void)
{
	long *ccu;

	ccu = KADDR(PHYSCCU);
	ccu[HdmiClk] = 0x80000000;
	ccu[HdmiBus] = 0x00030001;	/* gate + both resets released */
	microdelay(100);
	return coreinit(KADDR(PHYSHDMIPHY));
}


static void
mixerlayer(void *fb)
{
	ulong *m;

	m = KADDR(PHYSMIXER0);
	wr(m, UiSize, Outsize);
	wr(m, UiCoord, 0);
	wr(m, UiPitch, Hactive*4);
	wr(m, UiLaddr, PADDR(fb));		/* physical address, IOMMU off */
	wr(m, UiOvlsize, Outsize);
	wr(m, UiAttr, UiEn|UiXrgb8888|UiAlpha);
	wr(m, BldInsize, Outsize);		/* mandatory, else the layer has no area */
	wr(m, BldCoord, 0);
	wr(m, GlbDbuff, 1);				/* commit */
}



void
blankscreen(int blank)
{
	USED(blank);
}


void
hdmiinit(void)
{
	void *fb;

	if(clockinit() < 0)
		return;

	mixerinit();
	tconinit();
	if(hdmitxinit() < 0)
		print("hdmi: no video link\n");

	fb = screeninit(Hactive, Vactive, 32);
	if(fb == nil){
		print("hdmi: screeninit failed\n");
		return;
	}

	mixerlayer(fb);
}
