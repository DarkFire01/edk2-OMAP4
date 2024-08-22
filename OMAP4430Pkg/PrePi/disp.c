// Pi.c: Entry point for SEC(Security).

#include "Pi.h"

#include <Pi/PiBootMode.h>
#include <Pi/PiHob.h>
#include <PiDxe.h>
#include <PiPei.h>

#include <Guid/LzmaDecompress.h>
#include <Ppi/GuidedSectionExtraction.h>

#include <Library/ArmLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/DebugAgentLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/PeCoffGetEntryPointLib.h>
#include <Library/PerformanceLib.h>
#include <Library/PrePiHobListPointerLib.h>
#include <Library/PrePiLib.h>
#include <Library/SerialPortLib.h>
 typedef UINT16 u16;
#define max(a,b) 	(((a)>(b)) ? (a) : (b))
#define min(a,b) 	(((a)<(b)) ? (a) : (b))
#define CONFIG_ARCH_OMAP4 1
#define abs(x) ({				\
		int __x = (x);			\
		(__x < 0) ? -__x : __x;		\
	})

enum omap_plane {
	OMAP_DSS_GFX	= 0,
	OMAP_DSS_VIDEO1	= 1,
	OMAP_DSS_VIDEO2	= 2,
	OMAP_DSS_VIDEO3 = 3		
};
enum omap_channel {
	OMAP_DSS_CHANNEL_LCD	= 0,
	OMAP_DSS_CHANNEL_DIGIT	= 1,
	OMAP_DSS_CHANNEL_LCD2	= 2,	
};
enum omap_color_mode {
	OMAP_DSS_COLOR_CLUT1	= 1 << 0,  
	OMAP_DSS_COLOR_CLUT2	= 1 << 1,  
	OMAP_DSS_COLOR_CLUT4	= 1 << 2,  
	OMAP_DSS_COLOR_CLUT8	= 1 << 3,  
	OMAP_DSS_COLOR_RGB12U	= 1 << 4,  
	OMAP_DSS_COLOR_ARGB16	= 1 << 5,  
	OMAP_DSS_COLOR_RGB16	= 1 << 6,  
	OMAP_DSS_COLOR_RGB24U	= 1 << 7,  
	OMAP_DSS_COLOR_RGB24P	= 1 << 8,  
	OMAP_DSS_COLOR_YUV2	= 1 << 9,  
	OMAP_DSS_COLOR_UYVY	= 1 << 10, 
	OMAP_DSS_COLOR_ARGB32	= 1 << 11, 
	OMAP_DSS_COLOR_RGBA32	= 1 << 12, 
	OMAP_DSS_COLOR_RGBX32	= 1 << 13, 
	OMAP_DSS_COLOR_NV12     = 1 << 14, 
	OMAP_DSS_COLOR_RGBA12	= 1 << 15, 
	OMAP_DSS_COLOR_XRGB12	= 1 << 16, 
	OMAP_DSS_COLOR_ARGB16_1555	= 1 << 17, 
	OMAP_DSS_COLOR_RGBX24_32_ALGN	= 1 << 18, 
	OMAP_DSS_COLOR_XRGB15	= 1 << 19, 
	OMAP_DSS_COLOR_GFX_OMAP2 =
		OMAP_DSS_COLOR_CLUT1 | OMAP_DSS_COLOR_CLUT2 |
		OMAP_DSS_COLOR_CLUT4 | OMAP_DSS_COLOR_CLUT8 |
		OMAP_DSS_COLOR_RGB12U | OMAP_DSS_COLOR_RGB16 |
		OMAP_DSS_COLOR_RGB24U | OMAP_DSS_COLOR_RGB24P,
	OMAP_DSS_COLOR_VID_OMAP2 =
		OMAP_DSS_COLOR_RGB16 | OMAP_DSS_COLOR_RGB24U |
		OMAP_DSS_COLOR_RGB24P | OMAP_DSS_COLOR_YUV2 |
		OMAP_DSS_COLOR_UYVY,
	OMAP_DSS_COLOR_GFX_OMAP3 =
		OMAP_DSS_COLOR_CLUT1 | OMAP_DSS_COLOR_CLUT2 |
		OMAP_DSS_COLOR_CLUT4 | OMAP_DSS_COLOR_CLUT8 |
		OMAP_DSS_COLOR_RGB12U | OMAP_DSS_COLOR_ARGB16 |
		OMAP_DSS_COLOR_RGB16 | OMAP_DSS_COLOR_RGB24U |
		OMAP_DSS_COLOR_RGB24P | OMAP_DSS_COLOR_ARGB32 |
		OMAP_DSS_COLOR_RGBA32 | OMAP_DSS_COLOR_RGBX32,
	OMAP_DSS_COLOR_VID1_OMAP3 =
#ifdef CONFIG_ARCH_OMAP4
		OMAP_DSS_COLOR_NV12 | OMAP_DSS_COLOR_RGBA12 |
		OMAP_DSS_COLOR_XRGB12 | OMAP_DSS_COLOR_ARGB16_1555 |
		OMAP_DSS_COLOR_RGBX24_32_ALGN | OMAP_DSS_COLOR_XRGB15 |
#endif
		OMAP_DSS_COLOR_RGB12U | OMAP_DSS_COLOR_ARGB16 |
		OMAP_DSS_COLOR_RGB16 | OMAP_DSS_COLOR_RGB24U |
		OMAP_DSS_COLOR_RGB24P | OMAP_DSS_COLOR_YUV2 |
		OMAP_DSS_COLOR_UYVY | OMAP_DSS_COLOR_ARGB32 |
		OMAP_DSS_COLOR_RGBA32 | OMAP_DSS_COLOR_RGBX32,
	OMAP_DSS_COLOR_VID2_OMAP3 = OMAP_DSS_COLOR_VID1_OMAP3,
	OMAP_DSS_COLOR_VID3_OMAP3 = OMAP_DSS_COLOR_VID2_OMAP3,
};
enum omap_dss_load_mode {
	OMAP_DSS_LOAD_CLUT_AND_FRAME	= 0,
	OMAP_DSS_LOAD_CLUT_ONLY		= 1,
	OMAP_DSS_LOAD_FRAME_ONLY	= 2,
	OMAP_DSS_LOAD_CLUT_ONCE_FRAME	= 3,
};
enum omap_dss_trans_key_type {
	OMAP_DSS_COLOR_KEY_GFX_DST = 0,
	OMAP_DSS_COLOR_KEY_VID_SRC = 1,
};
enum omap_panel_config {
	OMAP_DSS_LCD_IVS		= 1<<0,
	OMAP_DSS_LCD_IHS		= 1<<1,
	OMAP_DSS_LCD_IPC		= 1<<2,
	OMAP_DSS_LCD_IEO		= 1<<3,
	OMAP_DSS_LCD_RF			= 1<<4,
	OMAP_DSS_LCD_ONOFF		= 1<<5,
	OMAP_DSS_LCD_TFT		= 1<<20,
};
enum omap_display_caps {
	OMAP_DSS_DISPLAY_CAP_MANUAL_UPDATE	= 1 << 0,
	OMAP_DSS_DISPLAY_CAP_TEAR_ELIM		= 1 << 1,
};
enum omap_dss_update_mode {
	OMAP_DSS_UPDATE_DISABLED = 0,
	OMAP_DSS_UPDATE_AUTO,
	OMAP_DSS_UPDATE_MANUAL,
};
enum omap_dss_display_state {
	OMAP_DSS_DISPLAY_DISABLED = 0,
	OMAP_DSS_DISPLAY_ACTIVE,
	OMAP_DSS_DISPLAY_SUSPENDED,
};
enum omap_dss_rotation_type {
	OMAP_DSS_ROT_DMA = 0,
	OMAP_DSS_ROT_VRFB = 1,
	OMAP_DSS_ROT_TILER = 2,	
};
enum omap_dss_rotation_angle {
	OMAP_DSS_ROT_0   = 0,
	OMAP_DSS_ROT_90  = 1,
	OMAP_DSS_ROT_180 = 2,
	OMAP_DSS_ROT_270 = 3,
};
enum dsi {
			dsi1 = 0,
			dsi2 = 1,
			};
#define FLD_MASK(start, end)	(((1 << ((start) - (end) + 1)) - 1) << (end))
#define FLD_VAL(val, start, end) (((val) << (end)) & FLD_MASK(start, end))
#define FLD_GET(val, start, end) (((val) & FLD_MASK(start, end)) >> (end))
#define FLD_MOD(orig, val, start, end) \
	(((orig) & ~FLD_MASK(start, end)) | FLD_VAL(val, start, end))
#define DISPC_MAX_FCK 173000000
enum omap_burst_size {
	OMAP_DSS_BURST_4x32	= 0, 
	OMAP_DSS_BURST_8x32	= 1, 
	OMAP_DSS_BURST_16x32	= 2, 
};
enum dss_clock {
	DSS_CLK_ICK	= 1 << 0,
	DSS_CLK_FCK1	= 1 << 1,
	DSS_CLK_FCK2	= 1 << 2,
	DSS_CLK_54M	= 1 << 3,
	DSS_CLK_96M	= 1 << 4,
};
struct dss_clock_info {
	unsigned long fck;
	UINT16 fck_div;
};
struct dispc_clock_info {
	unsigned long lck;
	unsigned long pck;
	UINT16 lck_div;
	UINT16 pck_div;
};
struct dsi_clock_info {
	unsigned long fint;
	unsigned long clkin4ddr;
	unsigned long clkin;
	unsigned long dsi1_pll_fclk;
	unsigned long dsi2_pll_fclk;
	unsigned long lp_clk;
	UINT16 regn;
	UINT16 regm;
	UINT16 regm3;
	UINT16 regm4;
	UINT16 lp_clk_div;
	char highfreq;
	BOOLEAN use_dss2_fck;
	BOOLEAN use_dss2_sys_clk;
};
struct omap_video_timings {
	UINT16 x_res;
	UINT16 y_res;
	UINT32 pixel_clock;
	UINT16 hsw;	
	UINT16 hfp;	
	UINT16 hbp;	
	UINT16 vsw;	
	UINT16 vfp;	
	UINT16 vbp;	
};
struct overlay_cache_data {
	BOOLEAN dirty;
	BOOLEAN shadow_dirty;
	BOOLEAN enabled;
	UINT32 paddr;
	void volatile *vaddr;
	UINT32 screen_width;
	UINT32 width;
	UINT32 height;
	enum omap_color_mode color_mode;
	char rotation;
	enum omap_dss_rotation_type rotation_type;
	BOOLEAN mirror;
	UINT32 pos_x;
	UINT32 pos_y;
	UINT32 out_width;	
	UINT32 out_height;	
	char global_alpha;
	char pre_alpha_mult;
	enum omap_channel channel;
	BOOLEAN replication;
	BOOLEAN ilace;
	enum omap_burst_size burst_size;
	UINT32 fifo_low;
	UINT32 fifo_high;
	BOOLEAN manual_update;
	UINT32 p_uv_addr; 
};
static struct overlay_cache_data cosmo_overlay = {
	.screen_width = 480,
	.width = 480,
	.height = 800,
	.color_mode = OMAP_DSS_COLOR_ARGB32,	
	.ilace = 0,
	.rotation = 0,
	.rotation_type = OMAP_DSS_ROT_DMA,
	.mirror = 0,
	.pos_x = 0,
	.pos_y = 0,
	.out_width = 480,
	.out_height = 800,
	.global_alpha = 255,
	.channel = OMAP_DSS_GFX,
	.replication = 0,
	.burst_size = OMAP_DSS_BURST_16x32,
	.fifo_low = 0,
	.fifo_high = 960,
	.manual_update = 0,
};

   	/* DSS */
#define DSS_BASE                0x58000000
   	/* DISPLAY CONTROLLER */
#define DISPC_BASE              0x58001000

#define DSS_SZ_REGS				0x00000200
struct dss_reg {
	UINT16 idx;
};
#define DSS_REG(idx)			((const struct dss_reg) { idx })
struct dispc_reg { UINT32 idx; };
#define DISPC_REG(idx)			((const struct dispc_reg) { idx })
#define DSS_REVISION			DSS_REG(0x0000)
#define DSS_SYSCONFIG			DSS_REG(0x0010)
#define DSS_SYSSTATUS			DSS_REG(0x0014)
#define DSS_CONTROL				DSS_REG(0x0040)
#define DSS_SDI_CONTROL			DSS_REG(0x0044)
#define DSS_PLL_CONTROL			DSS_REG(0x0048)
#define DSS_SDI_STATUS			DSS_REG(0x005C)
#ifdef CONFIG_ARCH_OMAP4
#define DSS_STATUS				DSS_REG(0x005C)
#endif
#define DISPC_SZ_REGS			SZ_1K
#define DISPC_REVISION			DISPC_REG(0x0000)
#define DISPC_SYSCONFIG			DISPC_REG(0x0010)
#define DISPC_SYSSTATUS			DISPC_REG(0x0014)
#define DISPC_IRQSTATUS			DISPC_REG(0x0018)
#define DISPC_IRQENABLE			DISPC_REG(0x001C)
#define DISPC_CONTROL			DISPC_REG(0x0040)
#define DISPC_CONFIG			DISPC_REG(0x0044)
#define DISPC_CAPABLE			DISPC_REG(0x0048)
#define DISPC_DEFAULT_COLOR0		DISPC_REG(0x004C)
#define DISPC_DEFAULT_COLOR1		DISPC_REG(0x0050)
#define DISPC_TRANS_COLOR0		DISPC_REG(0x0054)
#define DISPC_TRANS_COLOR1		DISPC_REG(0x0058)
#define DISPC_LINE_STATUS		DISPC_REG(0x005C)
#define DISPC_LINE_NUMBER		DISPC_REG(0x0060)
#define DISPC_TIMING_H			DISPC_REG(0x0064)
#define DISPC_TIMING_V			DISPC_REG(0x0068)
#define DISPC_POL_FREQ			DISPC_REG(0x006C)
#ifndef CONFIG_ARCH_OMAP4
#define DISPC_DIVISOR			DISPC_REG(0x0070)
#else
#define DISPC_DIVISOR			DISPC_REG(0x0804)
#define DISPC_DIVISOR1			DISPC_REG(0x0070)
#endif
#define DISPC_GLOBAL_ALPHA		DISPC_REG(0x0074)
#define DISPC_SIZE_DIG			DISPC_REG(0x0078)
#define DISPC_SIZE_LCD			DISPC_REG(0x007C)
#ifdef CONFIG_ARCH_OMAP4
#define DISPC_GLOBAL_BUFFER		DISPC_REG(0x0800)
#endif
#define DISPC_GFX_BA0			DISPC_REG(0x0080)
#define DISPC_GFX_BA1			DISPC_REG(0x0084)
#define DISPC_GFX_POSITION		DISPC_REG(0x0088)
#define DISPC_GFX_SIZE			DISPC_REG(0x008C)
#define DISPC_GFX_ATTRIBUTES		DISPC_REG(0x00A0)
#define DISPC_GFX_FIFO_THRESHOLD	DISPC_REG(0x00A4)
#define DISPC_GFX_FIFO_SIZE_STATUS	DISPC_REG(0x00A8)
#define DISPC_GFX_ROW_INC		DISPC_REG(0x00AC)
#define DISPC_GFX_PIXEL_INC		DISPC_REG(0x00B0)
#define DISPC_GFX_WINDOW_SKIP	DISPC_REG(0x00B4)
#define DISPC_GFX_TABLE_BA		DISPC_REG(0x00B8)
#define DISPC_DATA_CYCLE1		DISPC_REG(0x01D4)
#define DISPC_DATA_CYCLE2		DISPC_REG(0x01D8)
#define DISPC_DATA_CYCLE3		DISPC_REG(0x01DC)
#define DISPC_CPR_COEF_R		DISPC_REG(0x0220)
#define DISPC_CPR_COEF_G		DISPC_REG(0x0224)
#define DISPC_CPR_COEF_B		DISPC_REG(0x0228)
#define DISPC_GFX_PRELOAD		DISPC_REG(0x022C)
#define DISPC_VID_FIR_COEF_H(n, i)	DISPC_REG(0x00F0 + (n)*0x90 + (i)*0x8)
#define DISPC_VID_FIR_COEF_HV(n, i)	DISPC_REG(0x00F4 + (n)*0x90 + (i)*0x8)
#define DISPC_VID_CONV_COEF(n, i)	DISPC_REG(0x0130 + (n)*0x90 + (i)*0x4)
#define DISPC_VID_FIR_COEF_V(n, i)	DISPC_REG(0x01E0 + (n)*0x20 + (i)*0x4)
#define DISPC_VID_PRELOAD(n)		DISPC_REG(0x230 + (n)*0x04)
#define DISPC_CONTROL2			DISPC_REG(0x0238)
#define DISPC_CONFIG2			DISPC_REG(0x0620)
#ifdef CONFIG_ARCH_OMAP4
#define DISPC_DEFAULT_COLOR2		DISPC_REG(0x03AC)
#define DISPC_TRANS_COLOR2			DISPC_REG(0x03B0)
#define DISPC_CPR2_COEF_B			DISPC_REG(0x03B4)
#define DISPC_CPR2_COEF_G			DISPC_REG(0x03B8)
#define DISPC_CPR2_COEF_R			DISPC_REG(0x03BC)
#define DISPC_DATA2_CYCLE1			DISPC_REG(0x03C0)
#define DISPC_DATA2_CYCLE2			DISPC_REG(0x03C4)
#define DISPC_DATA2_CYCLE3			DISPC_REG(0x03C8)
#define DISPC_SIZE_LCD2				DISPC_REG(0x03CC)
#define DISPC_TIMING_H2				DISPC_REG(0x0400)
#define DISPC_TIMING_V2				DISPC_REG(0x0404)
#define DISPC_POL_FREQ2				DISPC_REG(0x0408)
#define DISPC_DIVISOR2				DISPC_REG(0x040C)
#endif
#define TCH 0
#define DCS_READ_NUM_ERRORS		0x05
#define DCS_READ_POWER_MODE		0x0a
#define DCS_READ_MADCTL			0x0b
#define DCS_READ_PIXEL_FORMAT	0x0c
#define DCS_RDDSDR				0x0f
#define DCS_SLEEP_IN			0x10
#define DCS_SLEEP_OUT			0x11
#define DCS_DISPLAY_OFF			0x28
#define DCS_DISPLAY_ON			0x29
#define DCS_COLUMN_ADDR			0x2a
#define DCS_PAGE_ADDR			0x2b
#define DCS_MEMORY_WRITE		0x2c
#define DCS_TEAR_OFF			0x34
#define DCS_TEAR_ON				0x35
#define DCS_MEM_ACC_CTRL		0x36
#define DCS_PIXEL_FORMAT		0x3a
#define DCS_BRIGHTNESS			0x51
#define DCS_CTRL_DISPLAY		0x53
#define DCS_WRITE_CABC			0x55
#define DCS_READ_CABC			0x56
#define DCS_GET_ID				0xf8
#define END_OF_COMMAND	0xFF
static inline void dispc_write_reg(const struct dispc_reg idx, UINT32 val)
{
	MmioWrite32(DISPC_BASE + idx.idx, val);
}

static inline UINT32 dispc_read_reg(const struct dispc_reg idx)
{
	return MmioRead32(DISPC_BASE + idx.idx);
}


#define REG_GET(idx, start, end) \
	FLD_GET(dispc_read_reg(idx), start, end)

#define REG_FLD_MOD(idx, val, start, end)				\
	dispc_write_reg(idx, FLD_MOD(dispc_read_reg(idx), val, start, end))

void dispc_go(enum omap_channel channel)
{
	if (REG_GET(DISPC_CONTROL2, 0, 0) == 0)
		return;

	if (REG_GET(DISPC_CONTROL2, 5, 5) == 1) {
		DEBUG((EFI_D_INFO, "GO bit not down for \n"));
		return;
	}

	DEBUG((EFI_D_INFO, "GO LCD2"));

	REG_FLD_MOD(DISPC_CONTROL2, 1, 5, 5);	
}

void dispc_set_lcd_size(enum omap_channel channel, UINT32 width, UINT32 height)
{
	UINT32 val;
//.	BUG_ON((width > (1 << 11)) || (height > (1 << 11)));
	val = FLD_VAL(height - 1, 26, 16) | FLD_VAL(width - 1, 10, 0);
#ifdef CONFIG_ARCH_OMAP4
	if (OMAP_DSS_CHANNEL_LCD2 == channel)
		dispc_write_reg(DISPC_SIZE_LCD2, val);
	else
#endif
	dispc_write_reg(DISPC_SIZE_LCD, val);
}

void dispc_enable_lcd_out(BOOLEAN enable)
{
	BOOLEAN is_on;

	/* When we disable LCD output, we need to wait until frame is done.
	 * Otherwise the DSS is still working, and turning off the clocks
	 * prevents DSS from going to OFF mode */
	is_on = REG_GET(DISPC_CONTROL2, 0, 0);

	if (!enable && is_on) {
        for(int i = 0; i < 100000; i++){}
		//mdelay(100);
	}

	REG_FLD_MOD(DISPC_CONTROL2, enable ? 1 : 0, 0, 0);

	if (!enable && is_on) {
        for(int i = 0; i < 100000; i++){}
		//(100);
	}

}


void dispc_pck_free_enable(BOOLEAN enable)
{
	REG_FLD_MOD(DISPC_CONTROL, enable ? 1 : 0, 27, 27);
}

void dispc_set_control2_reg()
{
	UINT32 l;
	int stallmode = 0; //OMAP_DSS_PARALLELMODE_BYPASS
	int lcd_mode = 1; //SET TFT
	int fifo_hancheck = 0;
	int tftline_code = 3; // tft line = 24 -> code 3
	
	l = dispc_read_reg(DISPC_CONTROL2);

	l = FLD_MOD(l, stallmode, 11, 11);
	l = FLD_MOD(l, lcd_mode, 3, 3);
	l = FLD_MOD(l, fifo_hancheck, 16, 16);
	l = FLD_MOD(l, tftline_code, 9, 8);

	dispc_write_reg(DISPC_CONTROL2, l);
}

void dispc_set_lcd_timings(enum omap_channel channel, struct omap_video_timings *timings)
{
	UINT32 timing_h, timing_v;

	timing_h = FLD_VAL(timings->hsw-1, 7, 0) | FLD_VAL(timings->hfp-1, 19, 8) |
				FLD_VAL(timings->hbp-1, 31, 20);

	timing_v = FLD_VAL(timings->vsw-1, 7, 0) | FLD_VAL(timings->vfp, 19, 8) |
				FLD_VAL(timings->vbp, 31, 20);	 

#ifdef CONFIG_ARCH_OMAP4
	dispc_write_reg(DISPC_TIMING_H2, timing_h);
	dispc_write_reg(DISPC_TIMING_V2, timing_v);
#endif
	//printf("DOLCOM : timing %x %x\n", timing_h, timing_v);
}

 void dispc_get_lcd_divisor(enum omap_channel channel,
					int *lck_div, int *pck_div)
{
	UINT32 l;

#ifdef CONFIG_ARCH_OMAP4
	if (OMAP_DSS_CHANNEL_LCD2 == channel)
		l = dispc_read_reg(DISPC_DIVISOR2);
	else
		l = dispc_read_reg(DISPC_DIVISOR1);
#else
		l = dispc_read_reg(DISPC_DIVISOR);
#endif
	*lck_div = FLD_GET(l, 23, 16);
	*pck_div = FLD_GET(l, 7, 0);
}
/* TODO: Check with Senthil on handling of clocks */

static inline UINT32 dss_read_reg(const struct dss_reg idx)
{
	return MmioRead32(DSS_BASE + idx.idx);
}


unsigned long dss_clk_get_rate(enum dss_clock clk)
{
	switch (clk) {
		case DSS_CLK_ICK:
			return 166000000;
		case DSS_CLK_FCK1:
			return 153600000;
		case DSS_CLK_FCK2:
			return 0;
		case DSS_CLK_54M:
			return 54000000;
		case DSS_CLK_96M:
			return 96000000;
		}

	return 0;

}

int dss_get_dispc_clk_source(void)
{
	return FLD_GET(dss_read_reg(DSS_CONTROL), 0, 0);  
}

unsigned long dispc_fclk_rate(void)
{
	unsigned long r = 0;

	if (dss_get_dispc_clk_source() == 0)
		r = dss_clk_get_rate(DSS_CLK_FCK1);
	return r;
}

unsigned long dispc_lclk_rate(enum omap_channel channel)
{
	int lcd;
	unsigned long r;
	UINT32 l;
#ifdef CONFIG_ARCH_OMAP4
	if (OMAP_DSS_CHANNEL_LCD2 == channel)
		l = dispc_read_reg(DISPC_DIVISOR2);
	else
		l = dispc_read_reg(DISPC_DIVISOR1);
#else
	l = dispc_read_reg(DISPC_DIVISOR);
#endif
	lcd = FLD_GET(l, 23, 16);

	r = dispc_fclk_rate();

	return r / lcd;
}

unsigned long dispc_pclk_rate(enum omap_channel channel)
{
	int lcd, pcd;
	unsigned long r;
	UINT32 l;
#ifdef CONFIG_ARCH_OMAP4
	if (OMAP_DSS_CHANNEL_LCD2 == channel)
		l = dispc_read_reg(DISPC_DIVISOR2);
	else
		l = dispc_read_reg(DISPC_DIVISOR1);
#else
	l = dispc_read_reg(DISPC_DIVISOR);
#endif
	lcd = FLD_GET(l, 23, 16);
	pcd = FLD_GET(l, 7, 0);

	r = dispc_fclk_rate();

	return r / lcd / pcd;
}


void dispc_set_pol_freq(enum omap_panel_config config,
							UINT8 acbi, UINT8 acb)
{
	UINT32 l = 0;
	
	l |= FLD_VAL((config & OMAP_DSS_LCD_ONOFF) != 0, 17, 17);
	l |= FLD_VAL((config & OMAP_DSS_LCD_RF) != 0, 16, 16);
	l |= FLD_VAL((config & OMAP_DSS_LCD_IEO) != 0, 15, 15);
	l |= FLD_VAL((config & OMAP_DSS_LCD_IPC) != 0, 14, 14);
	l |= FLD_VAL((config & OMAP_DSS_LCD_IHS) != 0, 13, 13);
	l |= FLD_VAL((config & OMAP_DSS_LCD_IVS) != 0, 12, 12);
	l |= FLD_VAL(acbi, 11, 8);
	l |= FLD_VAL(acb, 7, 0);
	dispc_write_reg(DISPC_POL_FREQ2, l);
}

/* with fck as input clock rate, find dispc dividers that produce req_pck */
void dispc_find_clk_divs(BOOLEAN is_tft, unsigned long req_pck, unsigned long fck,
		struct dispc_clock_info *cinfo)
{
	UINT16 pcd_min = is_tft ? 2 : 3;
	unsigned long best_pck;
	UINT16 best_ld, cur_ld;
	UINT16 best_pd, cur_pd;

	best_pck = 0;
	best_ld = 0;
	best_pd = 0;

	for (cur_ld = 1; cur_ld <= 255; ++cur_ld) {
		unsigned long lck = fck / cur_ld;

		for (cur_pd = pcd_min; cur_pd <= 255; ++cur_pd) {
			unsigned long pck = lck / cur_pd;
			long old_delta = abs(best_pck - req_pck);
			long new_delta = abs(pck - req_pck);

			if (best_pck == 0 || new_delta < old_delta) {
				best_pck = pck;
				best_ld = cur_ld;
				best_pd = cur_pd;

				if (pck == req_pck)
					goto found;
			}

			if (pck < req_pck)
				break;
		}

		if (lck / pcd_min < req_pck)
			break;
	}

found:
	cinfo->lck_div = best_ld;
	cinfo->pck_div = best_pd;
	cinfo->lck = fck / cinfo->lck_div;
	cinfo->pck = cinfo->lck / cinfo->pck_div;
}

/* calculate clock rates using dividers in cinfo */
int dispc_calc_clock_rates(unsigned long dispc_fclk_rate,
		struct dispc_clock_info *cinfo)
{
	if (cinfo->lck_div > 255 || cinfo->lck_div == 0)
		return -1;
	if (cinfo->pck_div < 2 || cinfo->pck_div > 255)
		return -1;

	cinfo->lck = dispc_fclk_rate / cinfo->lck_div;
	cinfo->pck = cinfo->lck / cinfo->pck_div;

	return 0;
}

int dispc_set_clock_div(enum omap_channel channel,
		struct dispc_clock_info *cinfo)
{

	 
	dispc_write_reg(DISPC_DIVISOR2,	FLD_VAL(cinfo->lck_div, 23, 16) | FLD_VAL(cinfo->pck_div, 7, 0));

	return 0;
}

int configure_dispc(void)
{
// == config overlay
	struct overlay_cache_data *c = &cosmo_overlay;
	UINT32 paddr;

	UINT32 val;

	paddr = 0x87000000;

	dispc_write_reg(DISPC_GFX_ATTRIBUTES, 0x42000011); //	dispc_set_channel_out(plane, channel);
	
	dispc_write_reg(DISPC_GFX_BA0,	paddr );
	dispc_write_reg(DISPC_GFX_BA1, paddr );
	dispc_write_reg(DISPC_GFX_ROW_INC, 1);
	dispc_write_reg(DISPC_GFX_PIXEL_INC, 1);
	dispc_write_reg(DISPC_GFX_POSITION, FLD_VAL(c->pos_y, 26, 16) | FLD_VAL(c->pos_x, 10, 0));
	dispc_write_reg(DISPC_GFX_SIZE, FLD_VAL(c->height - 1, 26, 16) | FLD_VAL(c->width - 1, 10, 0));
	val =dispc_read_reg(DISPC_GLOBAL_ALPHA);
	val = FLD_MOD(val,c->global_alpha, 7, 0);	
	dispc_write_reg(DISPC_GLOBAL_ALPHA,val);
	REG_FLD_MOD(DISPC_GLOBAL_ALPHA, c->global_alpha, 7, 0);	
	
    dispc_write_reg(DISPC_GFX_FIFO_THRESHOLD, 0x03fc03bc);//dispc_setup_plane_fifo(plane, c->fifo_low, c->fifo_high);
    
// == manager config
	val = dispc_read_reg(DISPC_CONFIG2);
	val = FLD_MOD(val,OMAP_DSS_COLOR_KEY_GFX_DST, 11, 11); // dispc_set_trans_key
	val = FLD_MOD(val,0, 10, 10); //dispc_enable_trans_key	

	dispc_write_reg(DISPC_CONFIG2, val);	
	dispc_write_reg(DISPC_TRANS_COLOR2, 0);

	return 0;
}




































#define OMAP_GPIO_MODULE	6

/* omap4430 specific GPIO registers */
#define OMAP44XX_GPIO1_BASE	0x4A310000
#define OMAP44XX_GPIO2_BASE	0x48055000
#define OMAP44XX_GPIO3_BASE	0x48057000
#define OMAP44XX_GPIO4_BASE	0x48059000
#define OMAP44XX_GPIO5_BASE	0x4805B000
#define OMAP44XX_GPIO6_BASE	0x4805D000

#define OMAP44XX_GPIO_REVISION			0x000
#define OMAP44XX_GPIO_SYSCONFIG			0x010
#define OMAP44XX_GPIO_SYSSTATUS			0x114
#define OMAP44XX_GPIO_IRQSTATUS1		0x118
#define OMAP44XX_GPIO_IRQENABLE1		0x11C
#define OMAP44XX_GPIO_WAKEUPENABLE		0x120
#define OMAP44XX_GPIO_IRQSTATUS2		0x128
#define OMAP44XX_GPIO_IRQENABLE2		0x12C
#define OMAP44XX_GPIO_CTRL				0x130
#define OMAP44XX_GPIO_OE				0x134
#define OMAP44XX_GPIO_DATAIN			0x138
#define OMAP44XX_GPIO_DATAOUT			0x13C
#define OMAP44XX_GPIO_LEVELDETECT0		0x140
#define OMAP44XX_GPIO_LEVELDETECT1		0x144
#define OMAP44XX_GPIO_RISINGDETECT		0x148
#define OMAP44XX_GPIO_FALLINGDETECT		0x14C
#define OMAP44XX_GPIO_DEBOUNCENABLE		0x150
#define OMAP44XX_GPIO_DEBOUNCINGTIME	0x154
#define OMAP44XX_GPIO_CLEARIRQENABLE1	0x160
#define OMAP44XX_GPIO_SETIRQENABLE1		0x164
#define OMAP44XX_GPIO_CLEARIRQENABLE2	0x170
#define OMAP44XX_GPIO_SETIRQENABLE2		0x174
#define OMAP44XX_GPIO_CLEARWKUENA		0x180
#define OMAP44XX_GPIO_SETWKUENA			0x184
#define OMAP44XX_GPIO_CLEARDATAOUT		0x190
#define OMAP44XX_GPIO_SETDATAOUT		0x194

/* for swtching gpio bank */
struct gpio_bank {
	unsigned int base;
};


static struct gpio_bank omap44xx_gpio_bank[OMAP_GPIO_MODULE] = {
	{OMAP44XX_GPIO1_BASE},
	{OMAP44XX_GPIO2_BASE},
	{OMAP44XX_GPIO3_BASE},
	{OMAP44XX_GPIO4_BASE},
	{OMAP44XX_GPIO5_BASE},
	{OMAP44XX_GPIO6_BASE},
};


static inline struct gpio_bank *get_gpio_bank(int gpio)
{
	return &omap44xx_gpio_bank[gpio >> 5];
}


static inline int get_gpio_index(int gpio)
{
	return gpio & 0x1f;
}


static inline int gpio_valid(int gpio)
{
	if (gpio < (32 * OMAP_GPIO_MODULE))
		return 0;

	//printf("omap-gpio: invalid GPIO %d\n", gpio);
	return -1;
}


void omap_set_gpio_direction(int gpio, int is_input)
{
	unsigned int val;
	struct gpio_bank *bank;

	if (gpio_valid(gpio) < 0)
		return;

	bank = get_gpio_bank(gpio);
	
	val = MmioRead32(bank->base + OMAP44XX_GPIO_OE);
	if (is_input)
		val |= 1 << get_gpio_index(gpio);
	else
		val &= ~(1 << get_gpio_index(gpio));
	MmioWrite32( bank->base + OMAP44XX_GPIO_OE, val);
}


void omap_set_gpio_dataout(int gpio, int enable)
{
	unsigned int val;
	struct gpio_bank *bank;

	if (gpio_valid(gpio) < 0)
		return;

	bank = get_gpio_bank(gpio);
	
	val = 1 << get_gpio_index(gpio);
	if (enable) {
		MmioWrite32(bank->base + OMAP44XX_GPIO_SETDATAOUT, val);
	} else {
		MmioWrite32(bank->base + OMAP44XX_GPIO_CLEARDATAOUT, val);
	}
}


int omap_get_gpio_datain(int gpio)
{
	struct gpio_bank *bank;

	bank = get_gpio_bank(gpio);

	return (MmioRead32(bank->base + OMAP44XX_GPIO_DATAIN) & (1 << get_gpio_index(gpio))) != 0;
}


#define	GPIO_FLASH_EN		190
#define GPIO_MOVIE_MODE_EN	28
#define GPIO_LCD_RESET_N	30
#if defined (CONFIG_LGE_CX2) || defined (CONFIG_LGE_P2)
#define GPIO_LCD_MAKER_ID 158
#endif

int             DownloadValid = 0;  

#define GPIO_LCD_CP_EN		98

#define GPIO_LCD_EN		27


























/* Stuff on L3 Interconnect */
#define SMX_APE_BASE			0x68000000

/* L3 Firewall */
#define A_REQINFOPERM0		(SMX_APE_BASE + 0x05048)
#define A_READPERM0		(SMX_APE_BASE + 0x05050)
#define A_WRITEPERM0		(SMX_APE_BASE + 0x05058)

/* GPMC */
#define OMAP44XX_GPMC_BASE		(0x50000000)

/* DMM */
#define OMAP44XX_DMM_BASE		0x4E000000

/* SMS */
#define OMAP44XX_SMS_BASE               0x6C000000

/* SDRC */
#define OMAP44XX_SDRC_BASE              0x6D000000








#define OMAP44XX_CORE_L4_IO_BASE	0x4A000000

#define OMAP44XX_WAKEUP_L4_IO_BASE	0x4A300000

#define OMAP44XX_L4_PER			0x48000000

#define OMAP44XX_L4_IO_BASE		OMAP44XX_CORE_L4_IO_BASE

/* CONTROL */
#define OMAP44XX_CTRL_BASE		0x4A100000
#define OMAP44XX_CTRL_GEN_BASE		(OMAP44XX_L4_IO_BASE+0x2000)

/* TAP information  dont know for 3430*/
#define OMAP44XX_TAP_BASE	(0x49000000) /*giving some junk for virtio */

/* UART */
#define OMAP44XX_UART1			(OMAP44XX_L4_PER+0x6a000)
#define OMAP44XX_UART2			(OMAP44XX_L4_PER+0x6c000)
#define OMAP44XX_UART3			(OMAP44XX_L4_PER+0x20000)
#define OMAP44XX_UART4			(OMAP44XX_L4_PER+0x6e000)

/* General Purpose Timers */
#define OMAP44XX_GPT1			0x48318000
#define OMAP44XX_GPT2			0x48032000
#define OMAP44XX_GPT3			0x48034000
#define OMAP44XX_GPT4			0x48036000
#define OMAP44XX_GPT5			0x40138000
#define OMAP44XX_GPT6			0x4013A000
#define OMAP44XX_GPT7			0x4013C000
#define OMAP44XX_GPT8			0x4013E000
#define OMAP44XX_GPT9			0x48040000
#define OMAP44XX_GPT10			0x48086000
#define OMAP44XX_GPT11			0x48088000
#define OMAP44XX_GPT12			0x48304000

/* WatchDog Timers (1 secure, 3 GP) */
#define WD1_BASE			(0x4A322000)
#define WD2_BASE			(0x4A314000)
#define WD3_BASE			(0x40130000)

/* 32KTIMER */
#define SYNC_32KTIMER_BASE		(0x48320000)
#define S32K_CR				(SYNC_32KTIMER_BASE+0x10)

/* GPIO */
/* OMAP3 GPIO registers */
#define I2C_BASE1		(OMAP44XX_L4_PER + 0x70000)
#define I2C_BASE2		(OMAP44XX_L4_PER + 0x72000)
#define I2C_BASE3		(OMAP44XX_L4_PER + 0x60000)
#define I2C_BASE4		(OMAP44XX_L4_PER + 0x350000)


enum {
	I2C1	=	0,
	I2C2,
	I2C3,
	I2C4,
	I2C_MAX
};

#define I2C_DEFAULT_BASE I2C_BASE1

#define I2C_REV                 (0x00)
#define I2C_IE                  (0x2C)
#define I2C_IE1                 (0x84)
#define I2C_IE_CLR              (0x30)
#define I2C_STAT1               (0x88)
#define I2C_STAT                (0x28)
#define I2C_IV                  (0x34)
#define I2C_BUF                 (0x94)
#define I2C_CNT                 (0x98)
#define I2C_DATA                (0x9c)
#define I2C_SYSC                (0x10)
#define I2C_CON                 (0xA4)
#define I2C_OA                  (0xA8)
#define I2C_SA                  (0xAc)
#define I2C_PSC                 (0xB0)
#define I2C_SCLL                (0xB4)
#define I2C_SCLH                (0xB8)
#define I2C_SYSTEST             (0xBc)
#define I2C_SYSS		(0x90)

#define UCHAR   unsigned char
/* I2C masks */

/* I2C Interrupt Enable Register (I2C_IE): */
#define I2C_IE_GC_IE    (1 << 5)
#define I2C_IE_XRDY_IE  (1 << 4)        /* Transmit data ready interrupt enable */
#define I2C_IE_RRDY_IE  (1 << 3)        /* Receive data ready interrupt enable */
#define I2C_IE_ARDY_IE  (1 << 2)        /* Register access ready interrupt enable */
#define I2C_IE_NACK_IE  (1 << 1)        /* No acknowledgment interrupt enable */
#define I2C_IE_AL_IE    (1 << 0)        /* Arbitration lost interrupt enable */

/* I2C Status Register (I2C_STAT): */

#define I2C_STAT_SBD    (1 << 15)       /* Single byte data */
#define I2C_STAT_BB     (1 << 12)       /* Bus busy */
#define I2C_STAT_ROVR   (1 << 11)       /* Receive overrun */
#define I2C_STAT_XUDF   (1 << 10)       /* Transmit underflow */
#define I2C_STAT_AAS    (1 << 9)        /* Address as slave */
#define I2C_STAT_GC     (1 << 5)
#define I2C_STAT_XRDY   (1 << 4)        /* Transmit data ready */
#define I2C_STAT_RRDY   (1 << 3)        /* Receive data ready */
#define I2C_STAT_ARDY   (1 << 2)        /* Register access ready */
#define I2C_STAT_NACK   (1 << 1)        /* No acknowledgment interrupt enable */
#define I2C_STAT_AL     (1 << 0)        /* Arbitration lost interrupt enable */
void i2c_init(int speed, int slaveadd);

/* I2C Interrupt Code Register (I2C_INTCODE): */

#define I2C_INTCODE_MASK        7
#define I2C_INTCODE_NONE        0
#define I2C_INTCODE_AL          1       /* Arbitration lost */
#define I2C_INTCODE_NAK         2       /* No acknowledgement/general call */
#define I2C_INTCODE_ARDY        3       /* Register access ready */
#define I2C_INTCODE_RRDY        4       /* Rcv data ready */
#define I2C_INTCODE_XRDY        5       /* Xmit data ready */

/* I2C Buffer Configuration Register (I2C_BUF): */

#define I2C_BUF_RDMA_EN         (1 << 15)       /* Receive DMA channel enable */
#define I2C_BUF_XDMA_EN         (1 << 7)        /* Transmit DMA channel enable */

/* I2C Configuration Register (I2C_CON): */

#define I2C_CON_EN      (1 << 15)       /* I2C module enable */
#define I2C_CON_BE      (1 << 14)       /* Big endian mode */
#define I2C_CON_STB     (1 << 11)       /* Start byte mode (master mode only) */
#define I2C_CON_MST     (1 << 10)       /* Master/slave mode */
#define I2C_CON_TRX     (1 << 9)        /* Transmitter/receiver mode (master mode only) */
#define I2C_CON_XA      (1 << 8)        /* Expand address */
#define I2C_CON_STP     (1 << 1)        /* Stop condition (master mode only) */
#define I2C_CON_STT     (1 << 0)        /* Start condition (master mode only) */

/* I2C System Test Register (I2C_SYSTEST): */

#define I2C_SYSTEST_ST_EN       (1 << 15)       /* System test enable */
#define I2C_SYSTEST_FREE        (1 << 14)       /* Free running mode (on breakpoint) */
#define I2C_SYSTEST_TMODE_MASK  (3 << 12)       /* Test mode select */
#define I2C_SYSTEST_TMODE_SHIFT (12)            /* Test mode select */
#define I2C_SYSTEST_SCL_I       (1 << 3)        /* SCL line sense input value */
#define I2C_SYSTEST_SCL_O       (1 << 2)        /* SCL line drive output value */
#define I2C_SYSTEST_SDA_I       (1 << 1)        /* SDA line sense input value */
#define I2C_SYSTEST_SDA_O       (1 << 0)        /* SDA line drive output value */
#define CONFIG_OMAP44XX 1
/* I2C System Control Register (I2C_SYSC): */
#define I2C_SYSC_SRST           (1 << 1)        /* Software Reset */

/* I2C System Status Register (I2C_SYSS): */
#define I2C_SYSS_RDONE          (1 << 0)        /* Internel reset monitoring */

#define I2C_SCLL_SCLL        (0)
#define I2C_SCLL_SCLL_M      (0xFF)
#define I2C_SCLL_HSSCLL      (8)
#define I2C_SCLH_HSSCLL_M    (0xFF)
#define I2C_SCLH_SCLH        (0)
#define I2C_SCLH_SCLH_M      (0xFF)
#define I2C_SCLH_HSSCLH      (8)
#define I2C_SCLH_HSSCLH_M    (0xFF)

#define OMAP_I2C_STANDARD          100
#define OMAP_I2C_FAST_MODE         400
#define OMAP_I2C_HIGH_SPEED        3400

#define SYSTEM_CLOCK_12       12000
#define SYSTEM_CLOCK_13       13000
#define SYSTEM_CLOCK_192      19200
#define SYSTEM_CLOCK_96       96000

#define I2C_IP_CLK SYSTEM_CLOCK_96
#define I2C_PSC_MAX          (0x0f)
#define I2C_PSC_MIN          (0x00)

#define CFG_I2C_SPEED            100
#define CFG_I2C_SLAVE            1
#define CFG_I2C_BUS              0
#define CFG_I2C_BUS_SELECT       1

static UINT32 i2c_base = I2C_DEFAULT_BASE;
static UINT32 i2c_speed = CFG_I2C_SPEED;

#define DBG(ARGS...)
#define inb(a) MmioRead16(i2c_base + (a))
#define outb(v,a) MmioWrite16((i2c_base + (a)), (v))
#define inw(a) MmioRead16(i2c_base +(a))
#define outw(v,a) MmioWrite16((i2c_base + (a)), (v))


static void wait_for_bb(void);
static UINT16 wait_for_pin(void);
static void flush_fifo(void);

#if defined(CONFIG_OMAP34XX) || defined(CONFIG_OMAP44XX)
#define I2C_NUM_IF 4
#else
#define I2C_NUM_IF 2
#endif

int select_bus(int bus, int speed)
{
	if ((bus < 0) || (bus >= I2C_NUM_IF)) {
		//printf("Bad bus ID-%d\n", bus);
		return -1;
	}

#if defined(CONFIG_OMAP243X) || defined(CONFIG_OMAP34XX) || defined(CONFIG_OMAP44XX)
	
	if ((speed != OMAP_I2C_STANDARD) && (speed != OMAP_I2C_FAST_MODE)
	    && (speed != OMAP_I2C_HIGH_SPEED)) {
		//printf("Invalid Speed for i2c init-%d\n", speed);
		return -1;
	}
#else
	if ((speed != OMAP_I2C_STANDARD) && (speed != OMAP_I2C_FAST_MODE)) {
		//printf("Invalid Speed for i2c init-%d\n", speed);
		return -1;
	}
#endif

#if defined(CONFIG_OMAP34XX) || defined(CONFIG_OMAP44XX)
	if (bus == 3)
		i2c_base = I2C_BASE4;
	else if (bus == 2)
		i2c_base = I2C_BASE3;
	else 
#endif
	if (bus == 1)
		i2c_base = I2C_BASE2;
	else
		i2c_base = I2C_BASE1;

	i2c_init(speed, CFG_I2C_SLAVE);
	return 0;
}

void i2c_init(int speed, int slaveadd)
{
	int scl_lh = 0;
	int psc = 0;
	int iclk = 0;
	int reset_timeout = 10;
		

	if (inw(I2C_CON) & I2C_CON_EN) {
		outw(0, I2C_CON);
		for(int i = 0; i < 100000; i++){}
	}
	outw(I2C_SYSC_SRST, I2C_SYSC);	
		for(int i = 0; i < 100000; i++){}
	
	psc = I2C_PSC_MAX;
	while (psc >= I2C_PSC_MIN) {
		iclk = I2C_IP_CLK / (psc + 1);
		switch (speed) {
		case OMAP_I2C_STANDARD:
			scl_lh = (iclk * 10 / (OMAP_I2C_STANDARD * 2));
			break;
		case OMAP_I2C_HIGH_SPEED:
			
		case OMAP_I2C_FAST_MODE:
			scl_lh = (iclk * 10 / (OMAP_I2C_FAST_MODE * 2));
			break;
			
		}
	
		
		if (scl_lh % 10) {
			scl_lh = -1;
		} else {
			scl_lh /= 10;
			scl_lh -= 7;
		}
		if (scl_lh >= 0) {
			break;
		}
		psc--;
	}
	
	if (psc < I2C_PSC_MIN) {
		psc = 0;
		return;

	}
	iclk = I2C_IP_CLK / (psc + 1);

	switch (speed) {
	case OMAP_I2C_STANDARD:
		scl_lh =
		    (((iclk / (OMAP_I2C_STANDARD * 2)) - 7) &
		     I2C_SCLL_SCLL_M) << I2C_SCLL_SCLL;
		break;
	case OMAP_I2C_HIGH_SPEED:
		scl_lh =
		    (((I2C_IP_CLK / (OMAP_I2C_HIGH_SPEED * 2)) - 7) &
		     I2C_SCLH_HSSCLL_M) << I2C_SCLL_HSSCLL;
		
	case OMAP_I2C_FAST_MODE:
		scl_lh |=
		    (((iclk / (OMAP_I2C_FAST_MODE * 2)) - 7) &
		     I2C_SCLL_SCLL_M) << I2C_SCLL_SCLL;
		break;
		
	}

	
	outw(I2C_CON_EN, I2C_CON);
	while (!(inw(I2C_SYSS) & I2C_SYSS_RDONE) && reset_timeout--) {
		if (reset_timeout <= 0)
		
		for(int i = 0; i < 100000; i++){}
	}

	outw(0, I2C_CON);  

	outw(psc, I2C_PSC);
	outw(scl_lh, I2C_SCLL);
	outw(scl_lh, I2C_SCLH);
	
	outw(slaveadd, I2C_OA);
	outw(I2C_CON_EN, I2C_CON);

	outw(I2C_IE_XRDY_IE | I2C_IE_RRDY_IE | I2C_IE_ARDY_IE |
	     I2C_IE_NACK_IE | I2C_IE_AL_IE, I2C_IE);
		for(int i = 0; i < 100000; i++){}
	flush_fifo();
	outw(0xFFFF, I2C_STAT);
	outw(0, I2C_CNT);
	i2c_speed = speed;
}

static int i2c_read_byte(UINT8 devaddr, UINT8 regoffset, UINT8 * value)
{
	int err;
	int i2c_error = 0;
	UINT16 status;

	wait_for_bb();

	outw(1, I2C_CNT);
	
	outw(devaddr, I2C_SA);
	
	outw(I2C_CON_EN | ((i2c_speed == OMAP_I2C_HIGH_SPEED) ? 0x1 << 12 : 0) |
	     I2C_CON_MST | I2C_CON_STT | I2C_CON_TRX, I2C_CON);

	status = wait_for_pin();

	if (status & I2C_STAT_XRDY) {
		
		outb(regoffset, I2C_DATA);

		err = 2000;
		while (!(inw(I2C_STAT) & I2C_STAT_ARDY) && err--)
			;
		if (err <= 0)
			i2c_error = 1;

		if (inw(I2C_STAT) & I2C_STAT_NACK) {
			i2c_error = 1;
		}
	} else {
		i2c_error = 1;
	}

	if (!i2c_error) {
		err = 2000;
		outw(I2C_CON_EN, I2C_CON);
		while (inw(I2C_STAT) || (inw(I2C_CON) & I2C_CON_MST)) {
			
			outw(0xFFFF, I2C_STAT);
			if (!err--) {
				break;
			}
		}

		outw(devaddr, I2C_SA);
		
		outw(1, I2C_CNT);
		
		outw(I2C_CON_EN |
		     ((i2c_speed ==
		       OMAP_I2C_HIGH_SPEED) ? 0x1 << 12 : 0) | I2C_CON_MST |
		     I2C_CON_STT | I2C_CON_STP, I2C_CON);

		status = wait_for_pin();
		if (status & I2C_STAT_RRDY) {
#if defined(CONFIG_OMAP243X) || defined(CONFIG_OMAP34XX) || defined(CONFIG_OMAP44XX)
			*value = inb(I2C_DATA);
#else
			*value = inw(I2C_DATA);
#endif
			
			err = 20000;
			while (!(inw(I2C_STAT) & I2C_STAT_ARDY) && err--)
				;
			if (err <= 0){
				//printf("i2c_read_byte -- I2C_STAT_ARDY error\n");
				i2c_error = 1;
			}
		} else {
			i2c_error = 1;
		}

		if (!i2c_error) {
			int err = 1000;
			outw(I2C_CON_EN, I2C_CON);
			while (inw(I2C_STAT)
			       || (inw(I2C_CON) & I2C_CON_MST)) {
				outw(0xFFFF, I2C_STAT);
				if (!err--) {
					break;
				}
			}
		}
	}
	flush_fifo();
	outw(0xFFFF, I2C_STAT);
	outw(0, I2C_CNT);
	return i2c_error;
}

static int i2c_write_byte(UINT8 devaddr, UINT8 regoffset, UINT8 value)
{
	int eout;
	int i2c_error = 0;
	UINT16 status, stat;

	wait_for_bb();

	outw(2, I2C_CNT);
	
	outw(devaddr, I2C_SA);
	
	outw(I2C_CON_EN | ((i2c_speed == OMAP_I2C_HIGH_SPEED) ? 0x1 << 12 : 0) |
	     I2C_CON_MST | I2C_CON_STT | I2C_CON_TRX | I2C_CON_STP, I2C_CON);

	status = wait_for_pin();

	if (status & I2C_STAT_XRDY) {
#if defined(CONFIG_OMAP243X) || defined(CONFIG_OMAP34XX) || defined(CONFIG_OMAP44XX)
		
		outb(regoffset, I2C_DATA);
		outw(I2C_STAT_XRDY, I2C_STAT);
		status = wait_for_pin();
		if ((status & I2C_STAT_XRDY)) {
			
			outb(value, I2C_DATA);
			outw(I2C_STAT_XRDY, I2C_STAT);
		} else {
			i2c_error = 1;
		}
#else
		
		outw((value << 8) | regoffset, I2C_DATA);
#endif
		
		eout= 20000;
		while (!(inw(I2C_STAT) & I2C_STAT_ARDY) && eout--)
			;
	
		if (inw(I2C_STAT) & I2C_STAT_NACK) {
			i2c_error = 1;
		}
	} else {
		i2c_error = 1;
	}
	if (!i2c_error) {
		eout = 2000;

		outw(I2C_CON_EN, I2C_CON);
		while ((stat = inw(I2C_STAT)) || (inw(I2C_CON) & I2C_CON_MST)) {
			
			outw(0xFFFF, I2C_STAT);
			if (--eout == 0)	
				break;
		}
	}
	flush_fifo();
	outw(0xFFFF, I2C_STAT);
	outw(0, I2C_CNT);
	return i2c_error;
}

int i2c_write_bytes(UINT8 devaddr, UINT8 regoffset, UINT8* data, UINT8 len)
{
	int eout;
	int i2c_error = 0;
	UINT16 status, stat;

	wait_for_bb();

	outw(3, I2C_CNT);
	
	outw(devaddr, I2C_SA);
	
	outw(I2C_CON_EN | ((i2c_speed == OMAP_I2C_HIGH_SPEED) ? 0x1 << 12 : 0) |
			I2C_CON_MST | I2C_CON_STT | I2C_CON_TRX | I2C_CON_STP, I2C_CON);

	status = wait_for_pin();

	if (status & I2C_STAT_XRDY) {
#if defined(CONFIG_OMAP243X) || defined(CONFIG_OMAP34XX) || defined(CONFIG_OMAP44XX)
		if (status & I2C_STAT_XRDY) {
			
			outb(regoffset, I2C_DATA);
			outw(I2C_STAT_XRDY, I2C_STAT);
			while(len--)
			{
				status = wait_for_pin();
				if ((status & I2C_STAT_XRDY)) {
					
					outb(*data, I2C_DATA);
					outw(I2C_STAT_XRDY, I2C_STAT);
				} else {
					i2c_error = 1;
				}
				data++;
			}
		}
#else
		
		outw((value << 8) | regoffset, I2C_DATA);
#endif
		
		eout= 20000;
		while (!(inw(I2C_STAT) & I2C_STAT_ARDY) && eout--)
			;

		if (inw(I2C_STAT) & I2C_STAT_NACK) {
			i2c_error = 1;
		}
	} else {
		i2c_error = 1;
	}
	if (!i2c_error) {
		eout = 2000;

		outw(I2C_CON_EN, I2C_CON);
		while ((stat = inw(I2C_STAT)) || (inw(I2C_CON) & I2C_CON_MST)) {
			
			outw(0xFFFF, I2C_STAT);
			if (--eout == 0)	
				break;
		}
	}
	flush_fifo();
	outw(0xFFFF, I2C_STAT);
	outw(0, I2C_CNT);
	return i2c_error;
}

static void flush_fifo(void)
{
	UINT32 stat;

	while (1) {
		stat = inw(I2C_STAT);
		if (stat == I2C_STAT_RRDY) {
#if defined(CONFIG_OMAP243X) || defined(CONFIG_OMAP34XX) || defined(CONFIG_OMAP44XX)
			inb(I2C_DATA);
#else
			inw(I2C_DATA);
#endif
			outw(I2C_STAT_RRDY, I2C_STAT);
		} else
			break;
	}
}

int i2c_probe(UCHAR chip)
{
	int res = 1;		

	if (chip == inw(I2C_OA)) {
		return res;
	}

	wait_for_bb();

	outw(1, I2C_CNT);
	
	outw(chip, I2C_SA);
	
	outw(I2C_CON_EN | ((i2c_speed == OMAP_I2C_HIGH_SPEED) ? 0x1 << 12 : 0) |
	     I2C_CON_MST | I2C_CON_STT | I2C_CON_STP, I2C_CON);
	
	for(int i = 0; i < 100000; i++){}

	if (!(inw(I2C_STAT) & I2C_STAT_NACK)) {
		res = 0;	
		flush_fifo();
		outw(0xFFFF, I2C_STAT);
	} else {
		outw(0xFFFF, I2C_STAT);	
		outw(inw(I2C_CON) | I2C_CON_STP, I2C_CON);	
		for(int i = 0; i < 100000; i++){}
		wait_for_bb();
	}
	flush_fifo();
	outw(0, I2C_CNT);	
	outw(0xFFFF, I2C_STAT);
	return res;
}

int i2c_read(UCHAR chip, UINT32 addr, int alen, UCHAR * buffer, int len)
{
	int i;

	if (alen > 1) {
		//printf("I2C read: addr len %d not supported\n", alen);
		return 1;
	}

	if (addr + len > 256) {
		//printf("I2C read: address out of range\n");
		return 1;
	}

	for (i = 0; i < len; i++) {
		if (i2c_read_byte(chip, addr + i, &buffer[i])) {
		//	printf("I2C read: I/O error %x\n", chip);
			i2c_init(i2c_speed, CFG_I2C_SLAVE);
			return 1;
		}
	}

	return 0;
}

int i2c_write(UCHAR chip, UINT32 addr, int alen, UCHAR * buffer, int len)
{
	int i;

	if (alen > 1) {
		///printf("I2C write: addr len %d not supported\n", alen);
		return 1;
	}

	if (addr + len > 256) {
		//printf("I2C write: address out of range\n");
		return 1;
	}

	for (i = 0; i < len; i++) {
		if (i2c_write_byte(chip, addr + i, buffer[i])) {
			//printf("I2C write: I/O error %x\n", chip);
			i2c_init(i2c_speed, CFG_I2C_SLAVE);
			return 1;
		}
	}

	return 0;
}

static void wait_for_bb(void)
{
	int timeout = 5000;
	UINT16 stat;

	outw(0xFFFF, I2C_STAT);	
	while ((stat = inw(I2C_STAT) & I2C_STAT_BB) && timeout--) {
		outw(stat, I2C_STAT);
	}

	if (timeout <= 0) {
		
	}
	outw(0xFFFF, I2C_STAT);	
}

static UINT16 wait_for_pin(void)
{
	UINT16 status;
	int timeout = 9000;

	do {
		status = inw(I2C_STAT);
	} while (!(status &
		   (I2C_STAT_ROVR | I2C_STAT_XUDF | I2C_STAT_XRDY |
		    I2C_STAT_RRDY | I2C_STAT_ARDY | I2C_STAT_NACK |
		    I2C_STAT_AL)) && timeout--);

	if (timeout <= 0) {
	
		outw(0xFFFF, I2C_STAT);
	}
	return status;
}





void	lcd_init()
{
	unsigned char	data_for_lcd;

	omap_set_gpio_direction(GPIO_LCD_EN, 0);		
	omap_set_gpio_dataout(GPIO_LCD_EN, 1);

	omap_set_gpio_direction(GPIO_LCD_CP_EN, 0);		
	omap_set_gpio_dataout(GPIO_LCD_CP_EN, 1);

	i2c_init(OMAP_I2C_STANDARD, 0x36);
	select_bus(I2C2, OMAP_I2C_STANDARD);

#define	LM3528_UNISON	0

#ifdef	LM3528_UNISON
	data_for_lcd	=	0xc5;	
#else
	data_for_lcd	=	0xc3;	
#endif
	i2c_write(0x36, 0x10, 1, &data_for_lcd, 1);	

	omap_set_gpio_dataout(GPIO_LCD_EN, 0);	


	data_for_lcd	=	0x00;	
	i2c_write(0x36, 0x80, 1, &data_for_lcd, 1);	

	omap_set_gpio_dataout(GPIO_LCD_EN, 1);

	data_for_lcd	=	0x7A;

	i2c_write(0x36, 0xA0, 1, &data_for_lcd, 1);	

#ifndef	LM3528_UNISON	
	data_for_lcd	=	0x7f;
	i2c_write(0x36, 0xB0, 1, &data_for_lcd, 1);	
#endif

	select_bus(0, 400);			

	omap_set_gpio_direction(GPIO_LCD_RESET_N, 0);	
	omap_set_gpio_dataout(GPIO_LCD_RESET_N, 0);	
	omap_set_gpio_dataout(GPIO_LCD_RESET_N, 1);

}

typedef UINT32 u32;
/*****************************************************************
 * sr32 - clear & set a value in a bit range for a 32 bit address
 *****************************************************************/
void sr32(u32 addr, u32 start_bit, u32 num_bits, u32 value)
{
	u32 tmp, msk = 0;
	msk = 1 << num_bits;
	--msk;
	tmp = MmioRead32(addr) & ~(msk << start_bit);
	tmp |=  value << start_bit;
	MmioWrite32(addr, tmp);
}

#define LCD_XRES		480
#define LCD_YRES		800

#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))

   	
#define DSS_BASE                0x58000000
   	
#define DISPC_BASE              0x58001000

#define DSS_SZ_REGS				0x00000200

#define DSI_CLOCK_POLARITY  0   
#define DSI_DATA1_POLARITY  0   
#define DSI_DATA2_POLARITY  0   
#define DSI_CLOCK_LANE      1   
#define DSI_DATA1_LANE      3   
#define DSI_DATA2_LANE      2   
#define DSI_CTRL_PIXEL_SIZE 24  
#define DSI_DIV_REGN        20
#define DSI_DIV_REGM        110
#define DSI_DIV_REGM3       3
#define DSI_DIV_REGM4       10
#define DSI_DIV_LCK_DIV     2
#define DSI_DIV_PCK_DIV     4
#define DSI_DIV_LP_CLK_DIV 12

#if 1	
enum omap_dsi_mode {
	OMAP_DSI_MODE_CMD = 0,
	OMAP_DSI_MODE_VIDEO = 1,
};
#endif

#define DSI_BASE		0x58004000
#define DSI2_BASE		0x58005000
#define CONFIG_OMAP2_DSS_MIN_FCK_PER_PCK 0

static int dss_state = OMAP_DSS_DISPLAY_DISABLED;

struct dsi_reg { u16 idx; };

#define DSI_REG(idx)		((const struct dsi_reg) { idx })

#define DSI_SZ_REGS		SZ_1K

#define DSI_REVISION			DSI_REG(0x0000)
#define DSI_SYSCONFIG			DSI_REG(0x0010)
#define DSI_SYSSTATUS			DSI_REG(0x0014)
#define DSI_IRQSTATUS			DSI_REG(0x0018)
#define DSI_IRQENABLE			DSI_REG(0x001C)
#define DSI_CTRL					DSI_REG(0x0040)
#define DSI_GNQ					DSI_REG(0x0044)
#define DSI_COMPLEXIO_CFG1		DSI_REG(0x0048)
#define DSI_COMPLEXIO_IRQ_STATUS	DSI_REG(0x004C)
#define DSI_COMPLEXIO_IRQ_ENABLE	DSI_REG(0x0050)
#define DSI_CLK_CTRL			DSI_REG(0x0054)
#define DSI_TIMING1				DSI_REG(0x0058)
#define DSI_TIMING2				DSI_REG(0x005C)
#define DSI_VM_TIMING1			DSI_REG(0x0060)
#define DSI_VM_TIMING2			DSI_REG(0x0064)
#define DSI_VM_TIMING3			DSI_REG(0x0068)
#define DSI_CLK_TIMING			DSI_REG(0x006C)
#define DSI_TX_FIFO_VC_SIZE		DSI_REG(0x0070)
#define DSI_RX_FIFO_VC_SIZE		DSI_REG(0x0074)
#define DSI_COMPLEXIO_CFG2		DSI_REG(0x0078)
#define DSI_RX_FIFO_VC_FULLNESS		DSI_REG(0x007C)
#define DSI_VM_TIMING4			DSI_REG(0x0080)
#define DSI_TX_FIFO_VC_EMPTINESS	DSI_REG(0x0084)
#define DSI_VM_TIMING5			DSI_REG(0x0088)
#define DSI_VM_TIMING6			DSI_REG(0x008C)
#define DSI_VM_TIMING7			DSI_REG(0x0090)
#define DSI_STOPCLK_TIMING		DSI_REG(0x0094)
#ifdef CONFIG_ARCH_OMAP4
#define DSI_CTRL2			DSI_REG(0x0098)
#define DSI_VM_TIMING8			DSI_REG(0x009C)
#define DSI_TE_HSYNC_WIDTH(n)		DSI_REG(0x00A0 + (n * 0xC))
#define DSI_TE_VSYNC_WIDTH(n)		DSI_REG(0x00A4 + (n * 0xC))
#define DSI_TE_HSYNC_NUMBER(n)		DSI_REG(0x00A8 + (n * 0xC))
#endif
#define DSI_VC_CTRL(n)			DSI_REG(0x0100 + (n * 0x20))
#define DSI_VC_TE(n)			DSI_REG(0x0104 + (n * 0x20))
#define DSI_VC_LONG_PACKET_HEADER(n)	DSI_REG(0x0108 + (n * 0x20))
#define DSI_VC_LONG_PACKET_PAYLOAD(n)	DSI_REG(0x010C + (n * 0x20))
#define DSI_VC_SHORT_PACKET_HEADER(n)	DSI_REG(0x0110 + (n * 0x20))
#define DSI_VC_IRQSTATUS(n)		DSI_REG(0x0118 + (n * 0x20))
#define DSI_VC_IRQENABLE(n)		DSI_REG(0x011C + (n * 0x20))

#define DSI_DSIPHY_CFG0			DSI_REG(0x200 + 0x0000)
#define DSI_DSIPHY_CFG1			DSI_REG(0x200 + 0x0004)
#define DSI_DSIPHY_CFG2			DSI_REG(0x200 + 0x0008)
#define DSI_DSIPHY_CFG5			DSI_REG(0x200 + 0x0014)

#define DSI_DSIPHY_CFG12		DSI_REG(0x200 + 0x0030)
#define DSI_DSIPHY_CFG14		DSI_REG(0x200 + 0x0038)
#define DSI_DSIPHY_CFG8			DSI_REG(0x200 + 0x0020)
#define DSI_DSIPHY_CFG9			DSI_REG(0x200 + 0x0024)

#define DSI_PLL_CONTROL			DSI_REG(0x300 + 0x0000)
#define DSI_PLL_STATUS			DSI_REG(0x300 + 0x0004)
#define DSI_PLL_GO			DSI_REG(0x300 + 0x0008)
#define DSI_PLL_CONFIGURATION1		DSI_REG(0x300 + 0x000C)
#define DSI_PLL_CONFIGURATION2		DSI_REG(0x300 + 0x0010)
#ifdef CONFIG_ARCH_OMAP4
#define DSI_PLL_CONFIGURATION3		DSI_REG(0x300 + 0x0014)
#define DSI_SSC_CONFIGURATION1		DSI_REG(0x300 + 0x0018)
#define DSI_SSC_CONFIGURATION2		DSI_REG(0x300 + 0x001C)
#define DSI_SSC_CONFIGURATION4		DSI_REG(0x300 + 0x0020)
#endif

  
  
#define DSICLKCTRLLPCLKDIVISOR    		1<<0| 1<<1| 1<<2 | 1<<3 | 1<<4 | 1<<5 | 1<<6 | 1<<7 | 1<<8 | 1<<9 | 1<<10 | 1<<11 | 1<<12

 
#define DSI_DT_DCS_SHORT_WRITE_0	0x05
#define DSI_DT_DCS_SHORT_WRITE_1	0x15
#define DSI_DT_DCS_READ			0x06
#define DSI_DT_SET_MAX_RET_PKG_SIZE	0x37
#define DSI_DT_NULL_PACKET		0x09
#define DSI_DT_DCS_LONG_WRITE		0x39

#define DSI_DT_RX_ACK_WITH_ERR		0x02
#define DSI_DT_RX_DCS_LONG_READ		0x1c
#define DSI_DT_RX_SHORT_READ_1		0x21
#define DSI_DT_RX_SHORT_READ_2		0x22

#define DSI_PACKED_PIXEL_STREAM_16	0x0e
#define DSI_PACKED_PIXEL_STREAM_18	0x1e
#define DSI_PIXEL_STREAM_3BYTE_18	0x2e
#define DSI_PACKED_PIXEL_STREAM_24	0x3e

#if defined (CONFIG_LGE_P2) 
#define DSI_VC_CMD 0
#define DSI_VC_VIDEO 0
#else
#define DSI_VC_CMD 0
#define DSI_VC_VIDEO 1
#endif

#if CONFIG_ARCH_OMAP4
#define FINT_MAX 2500000
#define FINT_MIN 750000
#define REGN 8
#define REGM 12
#define REGM3 5
#else

#define FINT_MAX 2500000
#define FINT_MIN 500000
#define REGN 7
#define REGM 11
#define REGM3 4
#endif
#define REGN_MAX (1 << REGN)
#define REGM_MAX ((1 << REGM) - 1)
#define REGM3_MAX (1 << REGM3)
#define REGM4 REGM3
#define REGM4_MAX (1 << REGM4)

#define LP_DIV_MAX ((1 << 13) - 1)

extern void volatile  *dss_base;
extern void volatile  *dispc_base;
void volatile  *gpio_base;
void volatile  *dsi_base;
#ifdef CONFIG_ARCH_OMAP4
void volatile  *dsi2_base;
#endif

enum fifo_size {
	DSI_FIFO_SIZE_0		= 0,
	DSI_FIFO_SIZE_32	= 1,
	DSI_FIFO_SIZE_64	= 2,
	DSI_FIFO_SIZE_96	= 3,
	DSI_FIFO_SIZE_128	= 4,
};

enum dsi_vc_mode {
	DSI_VC_MODE_L4 = 0,
	DSI_VC_MODE_VP,
};

static u32 cosmo_config = (OMAP_DSS_LCD_TFT | OMAP_DSS_LCD_ONOFF | OMAP_DSS_LCD_RF);
static int cosmo_acbi = 0; 
static int cosmo_acb = 0; 

static struct omap_video_timings cosmo_panel_timings = {	
	.x_res          = LCD_XRES,
	.y_res          = LCD_YRES,
	.pixel_clock    = 26583,
	.hfp            = 14, 
	.hsw            = 51, 
	.hbp            = 19, 
	.vfp            = 0, 
	.vsw            = 8, 
	.vbp            = 22, 
};  
typedef UINT16 u16;
typedef BOOLEAN bool;
struct dsi_update_region {
	bool dirty;
	u16 x, y, w, h; 
};

static struct dsi_struct
{
	void volatile	*base;

	struct dsi_clock_info current_cinfo;

	struct {
		enum dsi_vc_mode mode;
		enum fifo_size fifo_size;
		int dest_per;	
	} vc[4];
 
	unsigned pll_locked; 

 	bool framedone_received;
	struct dsi_update_region update_region;
	struct dsi_update_region active_update_region;
 
	enum omap_dss_update_mode user_update_mode;
	enum omap_dss_update_mode update_mode;
	enum omap_dsi_mode dsi_mode;
	bool te_enabled;
	bool use_ext_te;

 
	unsigned long cache_req_pck;
	unsigned long cache_clk_freq;
	struct dsi_clock_info cache_cinfo;

	u32		errors;
	
 	int debug_read;
	int debug_write;
} dsi_2;




















static inline void dsi_write_reg(enum dsi lcd_ix,
	const struct dsi_reg idx, u32 val)
{
	(lcd_ix == dsi1) ? MmioWrite32( DSI_BASE + idx.idx, val) :
		MmioWrite32(DSI2_BASE + idx.idx, val);
}

static inline u32 dsi_read_reg(enum dsi lcd_ix, const struct dsi_reg idx)
{
	if (lcd_ix == dsi1)
		return MmioRead32(DSI_BASE + idx.idx);
	else
		return MmioRead32(DSI2_BASE + idx.idx);
}

 #undef REG_GET
 #undef REG_FLD_MOD
#define REG_GET(no, idx, start, end) \
	FLD_GET(dsi_read_reg(no, idx), start, end)

#define REG_FLD_MOD(no, idx, val, start, end) \
	dsi_write_reg(no, idx, FLD_MOD(dsi_read_reg(no, idx), val, start, end))


static inline int wait_for_bit_change_delay(enum dsi lcd_ix,
		const struct dsi_reg idx, int bitnum, int value, int delay)
{
	int t = 100000;

	while (REG_GET(lcd_ix, idx, bitnum, bitnum) != value) {
		//udelay(delay);
		if (--t == 0)
			return !value;
	}

	return value;
}
typedef UINT8 u8;
static inline int wait_for_bit_change(enum dsi lcd_ix,
	const struct dsi_reg idx, int bitnum, int value)
{
	int t = 100000;

	while (REG_GET(lcd_ix, idx, bitnum, bitnum) != value) {
		if (--t == 0)
			return !value;
	}

	return value;
}
 
 
static inline int dsi_if_enable(enum dsi lcd_ix, bool enable)
{
	//./DEBUG(("EFI_D_INFO, dsi_if_enable(%d, %d)\n", lcd_ix, enable));

	enable = enable ? 1 : 0;
	REG_FLD_MOD(lcd_ix, DSI_CTRL, enable, 0, 0); 

	if (wait_for_bit_change(lcd_ix, DSI_CTRL, 0, enable) != enable) {
		//	printf("Failed to set dsi_if_enable to %d\n", enable);
			return -1;
	}

	return 0;
}

unsigned long dsi_get_dsi1_pll_rate(enum dsi lcd_ix)
{
	return dsi_2.current_cinfo.dsi1_pll_fclk;
}

static unsigned long dsi_get_dsi2_pll_rate(enum dsi lcd_ix)
{
	return dsi_2.current_cinfo.dsi2_pll_fclk;
}

static unsigned long dsi_get_txbyteclkhs(enum dsi lcd_ix)
{
	return dsi_2.current_cinfo.clkin4ddr / 16;
}

int dss_get_dsi_clk_source(void)
{
	return FLD_GET(dss_read_reg(DSS_CONTROL), 1, 1);
}

static unsigned long dsi_fclk_rate(enum dsi lcd_ix)
{
	unsigned long r;

	if (dss_get_dsi_clk_source() == 2) {				
		
		r = dss_clk_get_rate(DSS_CLK_FCK1);
	} else {
		
		r = dsi_get_dsi2_pll_rate(lcd_ix);
	}

	return r;
}

static int dsi_set_lp_clk_divisor(void)
{
	unsigned long dsi_fclk;
	unsigned lp_clk_div;
	unsigned long lp_clk;
	enum dsi lcd_ix;
	lcd_ix = dsi2;

	lp_clk_div = DSI_DIV_LP_CLK_DIV;

	if (lp_clk_div == 0 || lp_clk_div > LP_DIV_MAX)
		return -1;

	dsi_fclk = dsi_fclk_rate(lcd_ix);

	lp_clk = dsi_fclk / 2 / lp_clk_div;

//	printf("LP_CLK_DIV %u, LP_CLK %lu\n", lp_clk_div, lp_clk);
	dsi_2.current_cinfo.lp_clk = lp_clk;
	dsi_2.current_cinfo.lp_clk_div = lp_clk_div;

	REG_FLD_MOD(lcd_ix, DSI_CLK_CTRL, lp_clk_div,
			12, 0);			

	REG_FLD_MOD(lcd_ix, DSI_CLK_CTRL, 0,
			21, 21);		

	return 0;
}

enum dsi_pll_power_state {
	DSI_PLL_POWER_OFF	= 0x0,
	DSI_PLL_POWER_ON_HSCLK	= 0x1,
	DSI_PLL_POWER_ON_ALL	= 0x2,
	DSI_PLL_POWER_ON_DIV	= 0x3,
};

static int dsi_pll_power(enum dsi lcd_ix, enum dsi_pll_power_state state)
{
 

	REG_FLD_MOD(lcd_ix, DSI_CLK_CTRL, state, 31, 30);	

	while (FLD_GET(dsi_read_reg(lcd_ix, DSI_CLK_CTRL), 29, 28) != state) {
		//udelay(1);
	 
	}

	return 0;
}

int dsi_calc_clock_rates(struct dsi_clock_info *cinfo)
{
	if (cinfo->regn == 0 || cinfo->regn > REGN_MAX)
		return -1;

	if (cinfo->regm == 0 || cinfo->regm > REGM_MAX)
		return -1;

	if (cinfo->regm3 > REGM3_MAX)
		return -1;

	if (cinfo->regm4 > REGM4_MAX)
		return -1;

	if (cinfo->use_dss2_sys_clk) {

		if(1)
			cinfo->clkin = 38400000;
		else
			cinfo->clkin = 26000000;
		cinfo->highfreq = 0;
	} else if (cinfo->use_dss2_fck) {
		
		cinfo->clkin = 38400000;

		cinfo->highfreq = 0;
	} else {
		cinfo->clkin = dispc_pclk_rate(OMAP_DSS_CHANNEL_LCD);

		if (cinfo->clkin < 32000000)
			cinfo->highfreq = 0;
		else
			cinfo->highfreq = 1;
	}
//	printf("here cinfo->clkin =%x\n",cinfo->clkin);

	cinfo->fint = cinfo->clkin / (cinfo->regn * (cinfo->highfreq ? 2 : 1));

	if (cinfo->fint > FINT_MAX || cinfo->fint < FINT_MIN)
		return -1;

	cinfo->clkin4ddr = 2 * cinfo->regm * cinfo->fint;

	if (cinfo->clkin4ddr > 1800 * 1000 * 1000)
		return -1;

	if (cinfo->regm3 > 0)
		cinfo->dsi1_pll_fclk = cinfo->clkin4ddr / cinfo->regm3;
	else
		cinfo->dsi1_pll_fclk = 0;

	if (cinfo->regm4 > 0)
		cinfo->dsi2_pll_fclk = cinfo->clkin4ddr / cinfo->regm4;
	else
		cinfo->dsi2_pll_fclk = 0;

	return 0;
}

int dsi_pll_calc_clock_div_pck(enum dsi lcd_ix, bool is_tft,
		unsigned long req_pck,	struct dsi_clock_info *dsi_cinfo,
		struct dispc_clock_info *dispc_cinfo)
{
	struct dsi_clock_info cur, best;
	struct dispc_clock_info best_dispc;
	int min_fck_per_pck;
	int match = 0;
	unsigned long dss_clk_fck2;

	dss_clk_fck2 = dss_clk_get_rate(DSS_CLK_FCK2);

	if (req_pck == dsi_2.cache_req_pck &&
			dsi_2.cache_cinfo.clkin == dss_clk_fck2) {
	//	printf("DSI clock info found from cache\n");
		*dsi_cinfo = dsi_2.cache_cinfo;
		dispc_find_clk_divs(is_tft, req_pck, dsi_cinfo->dsi1_pll_fclk,
				dispc_cinfo);
		return 0;
	}

	min_fck_per_pck = CONFIG_OMAP2_DSS_MIN_FCK_PER_PCK;

	if (min_fck_per_pck &&
		req_pck * min_fck_per_pck > DISPC_MAX_FCK) {
	
		min_fck_per_pck = 0;
	}

	//printf("dsi_pll_calc\n");

retry:
	SetMem32(&best, 0, sizeof(best));
	SetMem32(&best_dispc, 0, sizeof(best_dispc));

	SetMem32(&cur, 0, sizeof(cur));
	cur.clkin = dss_clk_fck2;
	cur.use_dss2_fck = 1;
	cur.highfreq = 0;

	for (cur.regn = 1; cur.regn < REGN_MAX; ++cur.regn) {
		if (cur.highfreq == 0)
			cur.fint = cur.clkin / cur.regn;
		else
			cur.fint = cur.clkin / (2 * cur.regn);

		if (cur.fint > FINT_MAX || cur.fint < FINT_MIN)
			continue;

		for (cur.regm = 1; cur.regm < REGM_MAX; ++cur.regm) {
			unsigned long a, b;

			a = 2 * cur.regm * (cur.clkin/1000);
			b = cur.regn * (cur.highfreq + 1);
			cur.clkin4ddr = a / b * 1000;

			if (cur.clkin4ddr > 1800 * 1000 * 1000)
				break;

			for (cur.regm3 = 1; cur.regm3 < REGM3_MAX;
					++cur.regm3) {
				struct dispc_clock_info cur_dispc;
				cur.dsi1_pll_fclk = cur.clkin4ddr / cur.regm3;

				if (cur.dsi1_pll_fclk  < req_pck)
					break;

				if (cur.dsi1_pll_fclk > DISPC_MAX_FCK)
					continue;

				if (min_fck_per_pck &&
					cur.dsi1_pll_fclk <
						req_pck * min_fck_per_pck)
					continue;

				match = 1;

				dispc_find_clk_divs(is_tft, req_pck,
						cur.dsi1_pll_fclk,
						&cur_dispc);

				if (abs(cur_dispc.pck - req_pck) <
						abs(best_dispc.pck - req_pck)) {
					best = cur;
					best_dispc = cur_dispc;

					if (cur_dispc.pck == req_pck)
						goto found;
				}
			}
		}
	}
found:
	if (!match) {
		if (min_fck_per_pck) {
 
			min_fck_per_pck = 0;
			goto retry;
		}

 
		return -1;
	}

	best.regm4 = best.clkin4ddr / 48000000;
	if (best.regm4 > REGM4_MAX)
		best.regm4 = REGM4_MAX;
	else if (best.regm4 == 0)
		best.regm4 = 1;
	best.dsi2_pll_fclk = best.clkin4ddr / best.regm4;

	if (dsi_cinfo)
		*dsi_cinfo = best;
	if (dispc_cinfo)
		*dispc_cinfo = best_dispc;

	dsi_2.cache_req_pck = req_pck;
	dsi_2.cache_clk_freq = 0;
	dsi_2.cache_cinfo = best;

	return 0;
}

int dsi_pll_set_clock_div(enum dsi lcd_ix, struct dsi_clock_info *cinfo)
{
	int r = 0;
	u32 l;
 
	dsi_2.current_cinfo.fint = cinfo->fint;
	dsi_2.current_cinfo.clkin4ddr = cinfo->clkin4ddr;
	dsi_2.current_cinfo.dsi1_pll_fclk = cinfo->dsi1_pll_fclk;
	dsi_2.current_cinfo.dsi2_pll_fclk = cinfo->dsi2_pll_fclk;

	dsi_2.current_cinfo.regn = cinfo->regn;
	dsi_2.current_cinfo.regm = cinfo->regm;
	dsi_2.current_cinfo.regm3 = cinfo->regm3;
	dsi_2.current_cinfo.regm4 = cinfo->regm4;
#if 0
	printf("DSI Fint %ld\n", cinfo->fint);

	printf("clkin (%s) rate %ld, highfreq %d\n",
			cinfo->use_dss2_fck ? "dss2_fck" : "pclkfree",
			cinfo->clkin,
			cinfo->highfreq);

	printf("CLKIN4DDR = 2 * %d / %d * %lu / %d = %lu\n",
			cinfo->regm,
			cinfo->regn,
			cinfo->clkin,
			cinfo->highfreq + 1,
			cinfo->clkin4ddr);

	printf("Data rate on 1 DSI lane %ld Mbps\n",
			cinfo->clkin4ddr / 1000 / 1000 / 2);

	printf("Clock lane freq %ld Hz\n", cinfo->clkin4ddr / 4);

	printf("regm3 = %d, dsi1_pll_fclk = %lu\n",
			cinfo->regm3, cinfo->dsi1_pll_fclk);
	printf("regm4 = %d, dsi2_pll_fclk = %lu\n",
			cinfo->regm4, cinfo->dsi2_pll_fclk);

	REG_FLD_MOD(lcd_ix, DSI_PLL_CONTROL, 0, 0,
			0); 
#endif
	l = dsi_read_reg(lcd_ix, DSI_PLL_CONFIGURATION1);
	l = FLD_MOD(l, 1, 0, 0);		
	l = FLD_MOD(l, cinfo->regn - 1, REGN, 1);	
	
	l = FLD_MOD(l, cinfo->regm, REGN + REGM, REGN + 1);
	l = FLD_MOD(l, cinfo->regm3 > 0 ? cinfo->regm3 - 1 : 0,
		REGN + REGM + REGM3, REGN + REGM + 1);	
	
	l = FLD_MOD(l, cinfo->regm4 > 0 ? cinfo->regm4 - 1 : 0,
		REGN + REGM + REGM3 + REGM4, REGN + REGM + REGM3 + 1);
	dsi_write_reg(lcd_ix, DSI_PLL_CONFIGURATION1, l);

	//BUG_ON(cinfo->fint < FINT_MIN || cinfo->fint > FINT_MAX);

	l = dsi_read_reg(lcd_ix, DSI_PLL_CONFIGURATION2);
	
	l = FLD_MOD(l, cinfo->use_dss2_fck ? 0 : 1,
			11, 11);		
	l = FLD_MOD(l, cinfo->highfreq,
			12, 12);		
	l = FLD_MOD(l, 1, 13, 13);		
	l = FLD_MOD(l, 0, 14, 14);		
	l = FLD_MOD(l, 1, 20, 20);		
	if (1)
		l = FLD_MOD(l, 3, 22, 21);	
	dsi_write_reg(lcd_ix, DSI_PLL_CONFIGURATION2, l);

	REG_FLD_MOD(lcd_ix, DSI_PLL_GO, 1, 0, 0); 

	if (wait_for_bit_change(lcd_ix, DSI_PLL_GO, 0, 0) != 0) {
		//printf("dsi pll go bit not going down.\n");
		r = -1;
		goto err;
	}
	
	if (wait_for_bit_change(lcd_ix, DSI_PLL_STATUS, 1, 1) != 1) {
		//printf("cannot lock PLL\n");
		r = -1;
		goto err;
	}

	dsi_2.pll_locked = 1;

	l = dsi_read_reg(lcd_ix, DSI_PLL_CONFIGURATION2);
	l = FLD_MOD(l, 0, 0, 0);	
	l = FLD_MOD(l, 0, 5, 5);	
	l = FLD_MOD(l, 0, 6, 6);	
	if (0)
		l = FLD_MOD(l, 0, 7, 7);
	l = FLD_MOD(l, 0, 8, 8);	
	l = FLD_MOD(l, 0, 10, 9);	
	l = FLD_MOD(l, 1, 13, 13);	
	l = FLD_MOD(l, 1, 14, 14);	
	l = FLD_MOD(l, 0, 15, 15);	
	l = FLD_MOD(l, 1, 16, 16);	
	l = FLD_MOD(l, 0, 17, 17);	
	l = FLD_MOD(l, 1, 18, 18);	
	l = FLD_MOD(l, 0, 19, 19);	
	l = FLD_MOD(l, 0, 20, 20);	
	if (1) {
		l = FLD_MOD(l, 0, 25, 25);	
		l = FLD_MOD(l, 0, 26, 26);	
	}
	dsi_write_reg(lcd_ix, DSI_PLL_CONFIGURATION2, l);

	DEBUG((EFI_D_INFO,"PLL config done\n"));
err:
	return r;
}

int dsi_pll_init(enum dsi lcd_ix, bool enable_hsclk, bool enable_hsdiv)
{
	int r = 0;
	enum dsi_pll_power_state pwstate;
 
	//printf("PLL init\n");

	REG_FLD_MOD(lcd_ix, DSI_CLK_CTRL, 1, 14, 14);
	 

		dispc_pck_free_enable(1);

	if (wait_for_bit_change(lcd_ix, DSI_PLL_STATUS, 0, 1) != 1) {
		//puts("PLL not coming out of reset.\n");
		r = -1;
		goto err1;
	}

	dispc_pck_free_enable(1);

	pwstate = DSI_PLL_POWER_ON_ALL;

	r = dsi_pll_power(lcd_ix, pwstate);

	if (r)
		goto err1;

	//printf("PLL init done\n");

	return 0;

err1:
#if 0
	if (!cpu_is_omap44xx())
		regulator_disable(p_dsi->vdds_dsi_reg);
#endif
 
	return r;
}

enum dsi_complexio_power_state {
	DSI_COMPLEXIO_POWER_OFF		= 0x0,
	DSI_COMPLEXIO_POWER_ON		= 0x1,
	DSI_COMPLEXIO_POWER_ULPS	= 0x2,
};

static int dsi_complexio_power(enum dsi lcd_ix,
		enum dsi_complexio_power_state state)
{
	 

	REG_FLD_MOD(lcd_ix, DSI_COMPLEXIO_CFG1, state, 28, 27);

 
	
	REG_FLD_MOD(lcd_ix, DSI_COMPLEXIO_CFG1, 1, 30, 30);
 
	return 0;
}

static void dsi_complexio_config(void)
{
	u32 r;
	enum dsi lcd_ix;

	int clk_lane   = DSI_CLOCK_LANE;
	int data1_lane = DSI_DATA1_LANE;
	int data2_lane = DSI_DATA2_LANE;
	int clk_pol    = DSI_CLOCK_POLARITY;
	int data1_pol  = DSI_DATA1_POLARITY;
	int data2_pol  = DSI_DATA2_POLARITY;

	lcd_ix = dsi2;
	r = dsi_read_reg(lcd_ix, DSI_COMPLEXIO_CFG1);
	r = FLD_MOD(r, clk_lane, 2, 0);
	r = FLD_MOD(r, clk_pol, 3, 3);
	r = FLD_MOD(r, data1_lane, 6, 4);
	r = FLD_MOD(r, data1_pol, 7, 7);
	r = FLD_MOD(r, data2_lane, 10, 8);
	r = FLD_MOD(r, data2_pol, 11, 11);
	dsi_write_reg(lcd_ix, DSI_COMPLEXIO_CFG1, r);

}

static inline unsigned ns2ddr(enum dsi lcd_ix, unsigned ns)
{
	unsigned long ddr_clk;
	
	ddr_clk = dsi_2.current_cinfo.clkin4ddr / 4;
	return (ns * (ddr_clk/1000/1000) + 999) / 1000;
}

static inline unsigned ddr2ns(enum dsi lcd_ix, unsigned ddr)
{
	unsigned long ddr_clk;

	ddr_clk = dsi_2.current_cinfo.clkin4ddr / 4;
	return ddr * 1000 * 1000 / (ddr_clk / 1000);
}

static void dsi_complexio_timings(enum dsi lcd_ix)
{
	u32 r;
	u32 ths_prepare, ths_prepare_ths_zero, ths_trail, ths_exit;
	u32 tlpx_half, tclk_trail, tclk_zero;
	u32 tclk_prepare;
	 
	ths_prepare = ns2ddr(lcd_ix, 70) + 2;

	ths_prepare_ths_zero = ns2ddr(lcd_ix, 175) + 2;

	ths_trail = ns2ddr(lcd_ix, 60) + 5;

	ths_exit = ns2ddr(lcd_ix, 145);

	tlpx_half = ns2ddr(lcd_ix, 25);

	tclk_trail = ns2ddr(lcd_ix, 60) + 2;

	tclk_prepare = ns2ddr(lcd_ix, 65);

	tclk_zero = ns2ddr(lcd_ix, 265);

	r = dsi_read_reg(lcd_ix, DSI_DSIPHY_CFG0);
	r = FLD_MOD(r, ths_prepare, 31, 24);
	r = FLD_MOD(r, ths_prepare_ths_zero, 23, 16);
	r = FLD_MOD(r, ths_trail, 15, 8);
	r = FLD_MOD(r, ths_exit, 7, 0);
	dsi_write_reg(lcd_ix, DSI_DSIPHY_CFG0, r);

	r = dsi_read_reg(lcd_ix, DSI_DSIPHY_CFG1);
	r = FLD_MOD(r, tlpx_half, 22, 16);
	r = FLD_MOD(r, tclk_trail, 15, 8);
	r = FLD_MOD(r, tclk_zero, 7, 0);
	dsi_write_reg(lcd_ix, DSI_DSIPHY_CFG1, r);

	r = dsi_read_reg(lcd_ix, DSI_DSIPHY_CFG2);
	r = FLD_MOD(r, tclk_prepare, 7, 0);
	dsi_write_reg(lcd_ix, DSI_DSIPHY_CFG2, r);
}

static int dsi_complexio_init(enum dsi lcd_ix)
{	
	int r = 0;

 
	//printf("dsi_complexio_init\n");

	if (1) {

		*((volatile unsigned int *)0x4A100618) |= 0xffffffff;

MmioRead32(0x4A100618);

		REG_FLD_MOD(lcd_ix, DSI_CLK_CTRL, 1, 13, 13);	
		REG_FLD_MOD(lcd_ix, DSI_CLK_CTRL, 1, 18, 18);	 
		REG_FLD_MOD(lcd_ix, DSI_CLK_CTRL, 1, 14, 14);
	}

	dsi_read_reg(lcd_ix, DSI_DSIPHY_CFG5);

	if (wait_for_bit_change(lcd_ix, DSI_DSIPHY_CFG5,
			30, 1) != 1) {
		//printf("ComplexIO PHY not coming out of reset.\n");
		r = -1;
		goto err;
	}

	dsi_complexio_config();
	r = dsi_complexio_power(lcd_ix, DSI_COMPLEXIO_POWER_ON);
	if (r)
		goto err;

	if (wait_for_bit_change(lcd_ix, DSI_COMPLEXIO_CFG1,
			29, 1) != 1) {
		//printf("ComplexIO not coming out of reset.\n");

		r = -1;

		goto err;

	}

	dsi_complexio_timings(lcd_ix);

	dsi_if_enable(lcd_ix, 1);

	dsi_if_enable(lcd_ix, 0);

	REG_FLD_MOD(lcd_ix, DSI_CLK_CTRL, 1,

			20, 20); 

	dsi_if_enable(lcd_ix, 1);

	dsi_if_enable(lcd_ix, 0);

	//printf("CIO init done\n");

err:

	return r;

}
 
static int _dsi_wait_reset(enum dsi lcd_ix)

{

	int i = 0;

	while (REG_GET(lcd_ix, DSI_SYSSTATUS, 0, 0) == 0) {

		if (i++ > 50) {

			return -1;

		}

		//udelay(1);

	}

	return 0;

}

 

static void dsi_config_tx_fifo(enum dsi lcd_ix, enum fifo_size size1,

		enum fifo_size size2, enum fifo_size size3,

		enum fifo_size size4)

{

	u32 r = 0;

	int add = 0;

	int i;

	dsi_2.vc[0].fifo_size = size1;

	dsi_2.vc[1].fifo_size = size2;

	dsi_2.vc[2].fifo_size = size3;

	dsi_2.vc[3].fifo_size = size4;

	for (i = 0; i < 4; i++) {

		u8 v;

		int size = dsi_2.vc[i].fifo_size;

		if (add + size > 4) {

	

		}

		v = FLD_VAL(add, 2, 0) | FLD_VAL(size, 7, 4);

		r |= v << (8 * i);

		add += size;

	}

	dsi_write_reg(lcd_ix, DSI_TX_FIFO_VC_SIZE, r);

}

static void dsi_config_rx_fifo(enum dsi lcd_ix, enum fifo_size size1,

		enum fifo_size size2, enum fifo_size size3,

		enum fifo_size size4)

{

	u32 r = 0;

	int add = 0;

	int i;

	dsi_2.vc[0].fifo_size = size1;

	dsi_2.vc[1].fifo_size = size2;

	dsi_2.vc[2].fifo_size = size3;

	dsi_2.vc[3].fifo_size = size4;

	for (i = 0; i < 4; i++) {

		u8 v;

		int size = dsi_2.vc[i].fifo_size;

		if (add + size > 4) {
 
		}

		v = FLD_VAL(add, 2, 0) | FLD_VAL(size, 7, 4);

		r |= v << (8 * i);

		add += size;

	}

		dsi_write_reg(lcd_ix, DSI_RX_FIFO_VC_SIZE, r);

}

static int dsi_force_tx_stop_mode_io(enum dsi lcd_ix)

{
	u32 r;

	r = dsi_read_reg(lcd_ix, DSI_TIMING1);

	r = FLD_MOD(r, 1, 15, 15);	

	dsi_write_reg(lcd_ix, DSI_TIMING1, r);

	if (wait_for_bit_change(lcd_ix, DSI_TIMING1, 15, 0) != 0) {

	//	printf("TX_STOP bit not going down\n");

		return -1;

	}

	return 0;

}

 static int dsi_vc_enable(enum dsi lcd_ix, int channel, bool enable)

{	

	if (dsi_2.update_mode != OMAP_DSS_UPDATE_AUTO)

	

	enable = enable ? 1 : 0;

	REG_FLD_MOD(lcd_ix, DSI_VC_CTRL(channel), enable, 0, 0);

	if (wait_for_bit_change(lcd_ix, DSI_VC_CTRL(channel),

			0, enable) != enable) {

		
			return -1;

	}

	return 0;

}

static void dsi_vc_initial_config(enum dsi lcd_ix, int channel)

{

	u32 r;

	//printf("%d", channel);

	r = dsi_read_reg(lcd_ix, DSI_VC_CTRL(channel));



	r = FLD_MOD(r, 0, 1, 1); 
	r = FLD_MOD(r, 0, 2, 2); 
	r = FLD_MOD(r, 0, 3, 3); 
	r = FLD_MOD(r, 0, 4, 4); 
	r = FLD_MOD(r, 1, 7, 7); 
	r = FLD_MOD(r, 1, 8, 8); 
	r = FLD_MOD(r, 0, 9, 9); 
	if (1) {
		r = FLD_MOD(r, 3, 11, 10);	
		r = FLD_MOD(r, 1, 12, 12);	
		}
	r = FLD_MOD(r, 4, 29, 27); 
	if (0)
	r = FLD_MOD(r, 4, 23, 21); 

	dsi_write_reg(lcd_ix, DSI_VC_CTRL(channel), r);
	dsi_2.vc[channel].mode = DSI_VC_MODE_L4;
}

static void dsi_vc_initial_config_vp(enum dsi lcd_ix, int channel, enum omap_dsi_mode dsi_mode)
{
	u32 r;

	//printf("%d", channel);

	r = dsi_read_reg(lcd_ix, DSI_VC_CTRL(channel));
	r = FLD_MOD(r, 1, 1, 1); 
	r = FLD_MOD(r, 0, 2, 2); 
	r = FLD_MOD(r, 0, 3, 3); 
	r = FLD_MOD(r, (OMAP_DSI_MODE_CMD == dsi_mode) ? 0 : 1, 4, 4); 
	r = FLD_MOD(r, 1, 7, 7); 
	r = FLD_MOD(r, 1, 8, 8); 
	r = FLD_MOD(r, 1, 9, 9); 
	r = FLD_MOD(r, 1, 12, 12);	
	r = FLD_MOD(r, 4, 29, 27); 
	r = FLD_MOD(r, 4, 23, 21); 
	r = FLD_MOD(r, (OMAP_DSI_MODE_CMD == dsi_mode) ? 1: 0, 30, 30);	
	r = FLD_MOD(r, 0, 31, 31);	
	dsi_write_reg(lcd_ix, DSI_VC_CTRL(channel), r);
}

static void dsi_vc_config_l4(enum dsi lcd_ix, int channel)
{	
	if (dsi_2.vc[channel].mode == DSI_VC_MODE_L4)
		return;

//	printf("%d", channel);

	dsi_vc_enable(lcd_ix, channel, 0);

#if defined (CONFIG_LGE_P2) 
	dsi_vc_initial_config(lcd_ix, channel);
#else
	REG_FLD_MOD(lcd_ix, DSI_VC_CTRL(channel), 0, 1, 1); 
#endif
	dsi_vc_enable(lcd_ix, channel, 1);

	dsi_2.vc[channel].mode = DSI_VC_MODE_L4;
}

 void dsi_vc_config_vp(enum dsi lcd_ix, int channel)
{
	
#ifdef CONFIG_ARCH_OMAP4
	if (dsi_2.vc[channel].mode == DSI_VC_MODE_VP)
			return;

	//printf("%d", channel);

	dsi_vc_enable(lcd_ix, channel, 0);


#if defined (CONFIG_LGE_P2) 
	dsi_vc_initial_config_vp(lcd_ix, channel, OMAP_DSI_MODE_VIDEO);
#else
	REG_FLD_MOD(lcd_ix, DSI_VC_CTRL(channel), 1,
			1, 1); 
#endif
	dsi_vc_enable(lcd_ix, channel, 1);

	dsi_2.vc[channel].mode = DSI_VC_MODE_VP;
#endif
}

 
static void dsi_vc_flush_long_data(enum dsi lcd_ix, int channel)
{
	while (REG_GET(lcd_ix, DSI_VC_CTRL(channel), 20, 20)) {
		u32 val;
		val = dsi_read_reg(lcd_ix, DSI_VC_SHORT_PACKET_HEADER(channel));

	}
}

static void dsi_show_rx_ack_with_err(u16 err)
{
#if 0
	printk("\tACK with ERROR (%#x):\n", err);
	if (err & (1 << 0))
		printk("\t\tSoT Error\n");
	if (err & (1 << 1))
		printk("\t\tSoT Sync Error\n");
	if (err & (1 << 2))
		printk("\t\tEoT Sync Error\n");
	if (err & (1 << 3))
		printk("\t\tEscape Mode Entry Command Error\n");
	if (err & (1 << 4))
		printk("\t\tLP Transmit Sync Error\n");
	if (err & (1 << 5))
		printk("\t\tHS Receive Timeout Error\n");
	if (err & (1 << 6))
		printk("\t\tFalse Control Error\n");
	if (err & (1 << 7))
		printk("\t\t(reserved7)\n");
	if (err & (1 << 8))
		printk("\t\tECC Error, single-bit (corrected)\n");
	if (err & (1 << 9))
		printk("\t\tECC Error, multi-bit (not corrected)\n");
	if (err & (1 << 10))
		printk("\t\tChecksum Error\n");
	if (err & (1 << 11))
		printk("\t\tData type not recognized\n");
	if (err & (1 << 12))
		printk("\t\tInvalid VC ID\n");
	if (err & (1 << 13))
		printk("\t\tInvalid Transmission Length\n");
	if (err & (1 << 14))
		printk("\t\t(reserved14)\n");
	if (err & (1 << 15))
		printk("\t\tDSI Protocol Violation\n");
#endif	
}

static u16 dsi_vc_flush_receive_data(enum dsi lcd_ix, int channel)
{	
	
	while (REG_GET(lcd_ix, DSI_VC_CTRL(channel), 20, 20)) {
		u32 val;
		u8 dt;
		val = dsi_read_reg(lcd_ix, DSI_VC_SHORT_PACKET_HEADER(channel));
		//printf("\trawval %#08x\n", val);
		dt = FLD_GET(val, 5, 0);
		if (dt == DSI_DT_RX_ACK_WITH_ERR) {
			u16 err = FLD_GET(val, 23, 8);
			if (0)
				dsi_show_rx_ack_with_err(err);
		} else if (dt == DSI_DT_RX_SHORT_READ_1) {
	 
		} else if (dt == DSI_DT_RX_SHORT_READ_2) {
		 
		} else if (dt == DSI_DT_RX_DCS_LONG_READ) {
		 
			dsi_vc_flush_long_data(lcd_ix, channel);
		} else {
			//printf("\tunknown datatype 0x%02x\n", dt);
		}
	}
	return 0;
}
 

static inline void dsi_vc_write_long_header(enum dsi lcd_ix, int channel,
	u8 data_type, u16 len, u8 ecc)
{
	u32 val;
	u8 data_id;

	ecc = 0; 

	if (1)
	data_id = data_type | dsi_2.vc[channel].dest_per << 6;
	else
		data_id = data_type | channel << 6;

	val = FLD_VAL(data_id, 7, 0) | FLD_VAL(len, 23, 8) |
		FLD_VAL(ecc, 31, 24);

	dsi_write_reg(lcd_ix, DSI_VC_LONG_PACKET_HEADER(channel), val);
}

static inline void dsi_vc_write_long_payload(enum dsi lcd_ix, int channel,
		u8 b1, u8 b2, u8 b3, u8 b4)
{
	u32 val;

	val = b4 << 24 | b3 << 16 | b2 << 8  | b1 << 0;
	dsi_write_reg(lcd_ix, DSI_VC_LONG_PACKET_PAYLOAD(channel), val);
}

static int dsi_vc_send_long(enum dsi lcd_ix,
	int channel, u8 data_type, u8 *data, u16 len, u8 ecc)
{
	int i;
	u8 *p;
	int r = 0;
	u8 b1, b2, b3, b4;

 
	if (dsi_2.vc[channel].fifo_size * 32 * 4 < len + 4) {
		//printf("unable to send long packet: packet too long. ch = %d , len =%d, fifo = %d\n",channel,len,dsi_2.vc[channel].fifo_size);
		return -1;
	}

	dsi_vc_config_l4(lcd_ix, channel);

	dsi_vc_write_long_header(lcd_ix, channel, data_type, len, ecc);

	p = data;
	for (i = 0; i < len >> 2; i++) {
	 
		b1 = *p++;
		b2 = *p++;
		b3 = *p++;
		b4 = *p++;

	//	mdelay(2+1);
		dsi_vc_write_long_payload(lcd_ix, channel, b1, b2, b3, b4);
	}

	i = len % 4;
	if (i) {
		b1 = 0; b2 = 0; b3 = 0;

 
		switch (i) {
		case 3:
			b1 = *p++;
			b2 = *p++;
			b3 = *p++;
			break;
		case 2:
			b1 = *p++;
			b2 = *p++;
			break;
		case 1:
			b1 = *p++;
			break;
		}

		dsi_vc_write_long_payload(lcd_ix, channel, b1, b2, b3, 0);
	}

	return r;
}

int send_short_packet(enum dsi lcd_ix, u8 data_type, u8 vc, u8 data0,
 u8 data1, bool mode, bool ecc)
{
	u32 val, header = 0, count = 10000;

	dsi_vc_enable(lcd_ix, vc, 0);
	
	val = dsi_read_reg(lcd_ix, DSI_VC_CTRL(vc));
	if (mode == 1) {
		val = val | (1<<9);
		}
	else if (mode == 0) {
		val = val & ~(1<<9);
		}
	dsi_write_reg(lcd_ix, DSI_VC_CTRL(vc), val);

	dsi_vc_enable(lcd_ix, vc, 1);
	
	header = (0<<24)|
			(data1<<16)|
			(data0<<8)|
			(0<<6) |
			(data_type<<0);
	dsi_write_reg(lcd_ix, DSI_VC_SHORT_PACKET_HEADER(0), header);

	//printf("Header = 0x%x", header);

	do {
		val = dsi_read_reg(lcd_ix, DSI_VC_IRQSTATUS(vc));
	} while ((!(val & 0x00000004)) && --count);
	if (count) {
		//printf("Short packet  success!!! \n\r");

		dsi_write_reg(lcd_ix, DSI_VC_IRQSTATUS(vc), 0x00000004);
		return 0;
		}
	else	{
		//printf("Failed to send Short packet !!! \n\r");
		return -1;
		}
}

static int dsi_vc_send_short(enum dsi lcd_ix, int channel, u8 data_type,
	u16 data, u8 ecc)
{	
	u32 r;
	u8 data_id;
 
	dsi_vc_config_l4(lcd_ix, channel);

	if (1) {
		if (FLD_GET(dsi_read_reg(lcd_ix, DSI_VC_CTRL(channel)),
			16, 16)) {
		//	printf("ERROR FIFO FULL, aborting transfer\n");
			return -1;
		}

		data_id = data_type | dsi_2.vc[channel].dest_per << 6;
	} else {
		data_id = data_type | 0 << 6;
	}

	r = (data_id << 0) | (data << 8) | (0 << 16) | (ecc << 24);

 
	dsi_write_reg(lcd_ix, DSI_VC_SHORT_PACKET_HEADER(channel), r);
	
	return 0;
}

int dsi_vc_dcs_write_nosync(enum dsi lcd_ix, int channel, u8 *data, int len)
{	
	int r = 0;
 
	if (len == 1) {
		r = dsi_vc_send_short(lcd_ix, channel, DSI_DT_DCS_SHORT_WRITE_0,
				data[0], 0);
	} else if (len == 2) {
		r = dsi_vc_send_short(lcd_ix, channel, DSI_DT_DCS_SHORT_WRITE_1,
				data[0] | (data[1] << 8), 0);		
	} else {
		
		r = dsi_vc_send_long(lcd_ix, channel, DSI_DT_DCS_LONG_WRITE,
				data, len, 0);
	}

	return r;
}

int dsi_vc_generic_write_short(enum dsi ix, int channel, u8 *data, int len)
{
	int r;
	if( len == 1) {
		r = dsi_vc_send_short(ix, channel, 0x23, data[0], 0);
	} else {
		r = dsi_vc_send_short(ix, channel, 0x23, data[0] | (data[1] << 8), 0);
	}
	
	return r;
}
 
int dsi_vc_dcs_write(enum dsi ix, int channel,
	u8 *data, int len)
{

	int r;

	r = dsi_vc_dcs_write_nosync(ix, channel, data, len);
	if (r)
		goto err;
	
	if (REG_GET(ix, DSI_VC_CTRL(channel), 20, 20)) {	
		//printf("rx fifo not empty after write, dumping data:\n");
		dsi_vc_flush_receive_data(ix, channel);
		r = -1;
		//printf("###  rx fifo not empty after write, dumping data. ###\n");
		goto err;
	}

	return 0;
err:

	return r;
}
static void dsi_set_lp_rx_timeout(enum dsi lcd_ix, unsigned long ns)
{
	u32 r;
	unsigned x4 = 0, x16 = 0;
	unsigned long fck = 0;
	unsigned long ticks = 0;
	lcd_ix = dsi2;

	fck = dsi_fclk_rate(lcd_ix);
	ticks = (fck / 1000 / 1000) * ns / 1000;
	x4 = 0;
	x16 = 0;

	if (ticks > 0x1fff) {
		ticks = (fck / 1000 / 1000) * ns / 1000 / 4;
		x4 = 1;
		x16 = 0;
	}

	if (ticks > 0x1fff) {
		ticks = (fck / 1000 / 1000) * ns / 1000 / 16;
		x4 = 0;
		x16 = 1;
	}

	if (ticks > 0x1fff) {
		ticks = (fck / 1000 / 1000) * ns / 1000 / (4 * 16);
		x4 = 1;
		x16 = 1;
	}

	if (ticks > 0x1fff) {
		//printf("LP_TX_TO over limit, setting it to max\n");
		ticks = 0x1fff;
		x4 = 1;
		x16 = 1;
	}

	r = dsi_read_reg(lcd_ix, DSI_TIMING2);
	r = FLD_MOD(r, 1, 15, 15);	
	r = FLD_MOD(r, x16, 14, 14);	
	r = FLD_MOD(r, x4, 13, 13);	
	r = FLD_MOD(r, ticks, 12, 0);	
	dsi_write_reg(lcd_ix, DSI_TIMING2, r);
 
}

static void dsi_set_ta_timeout(enum dsi lcd_ix, unsigned long ns)
{
	u32 r;
	unsigned x8 = 0, x16 = 0;
	unsigned long fck = 0;
	unsigned long ticks = 0;

	fck = dsi_fclk_rate(lcd_ix);
	ticks = (fck / 1000 / 1000) * ns / 1000;
	x8 = 0;
	x16 = 0;

	if (ticks > 0x1fff) {
		ticks = (fck / 1000 / 1000) * ns / 1000 / 8;
		x8 = 1;
		x16 = 0;
	}

	if (ticks > 0x1fff) {
		ticks = (fck / 1000 / 1000) * ns / 1000 / 16;
		x8 = 0;
		x16 = 1;
	}

	if (ticks > 0x1fff) {
		ticks = (fck / 1000 / 1000) * ns / 1000 / (8 * 16);
		x8 = 1;
		x16 = 1;
	}

	if (ticks > 0x1fff) {
		//printf("TA_TO over limit, setting it to max\n");
		ticks = 0x1fff;
		x8 = 1;
		x16 = 1;
	}

	r = dsi_read_reg(lcd_ix, DSI_TIMING1);

	r = FLD_MOD(r, 1, 31, 31);		
	
	r = FLD_MOD(r, 0, 31, 31);		
	r = FLD_MOD(r, x16, 30, 30);	
	r = FLD_MOD(r, x8, 29, 29);		
	r = FLD_MOD(r, ticks, 28, 16);	

	dsi_write_reg(lcd_ix, DSI_TIMING1, r);

 
}

static void dsi_set_stop_state_counter(enum dsi lcd_ix, unsigned long ns)
{
	u32 r;
	unsigned x4 = 0, x16 = 0;
	unsigned long fck = 0;
	unsigned long ticks = 0;

	fck = dsi_fclk_rate(lcd_ix);
	ticks = (fck / 1000 / 1000) * ns / 1000;
	x4 = 0;
	x16 = 0;

	if (ticks > 0x1fff) {
		ticks = (fck / 1000 / 1000) * ns / 1000 / 4;
		x4 = 1;
		x16 = 0;
	}

	if (ticks > 0x1fff) {
		ticks = (fck / 1000 / 1000) * ns / 1000 / 16;
		x4 = 0;
		x16 = 1;
	}

	if (ticks > 0x1fff) {
		ticks = (fck / 1000 / 1000) * ns / 1000 / (4 * 16);
		x4 = 1;
		x16 = 1;
	}

	if (ticks > 0x1fff) {
	 
		ticks = 0x1fff;
		x4 = 1;
		x16 = 1;
	}

	r = dsi_read_reg(lcd_ix, DSI_TIMING1);
	r = FLD_MOD(r, 1, 15, 15);		
	r = FLD_MOD(r, x16, 14, 14);	
	r = FLD_MOD(r, x4, 13, 13);		
	r = FLD_MOD(r, ticks, 12, 0);	
	dsi_write_reg(lcd_ix, DSI_TIMING1, r);

 
}

static void dsi_set_hs_tx_timeout(enum dsi lcd_ix, unsigned long ns)
{
	u32 r;
	unsigned x4 = 0, x16 = 0;
	unsigned long fck = 0;
	unsigned long ticks = 0;
	lcd_ix = dsi2;

	fck = dsi_get_txbyteclkhs(lcd_ix);
	ticks = (fck / 1000 / 1000) * ns / 1000;
	x4 = 0;
	x16 = 0;

	if (ticks > 0x1fff) {
		ticks = (fck / 1000 / 1000) * ns / 1000 / 4;
		x4 = 1;
		x16 = 0;
	}

	if (ticks > 0x1fff) {
		ticks = (fck / 1000 / 1000) * ns / 1000 / 16;
		x4 = 0;
		x16 = 1;
	}

	if (ticks > 0x1fff) {
		ticks = (fck / 1000 / 1000) * ns / 1000 / (4 * 16);
		x4 = 1;
		x16 = 1;
	}

	if (ticks > 0x1fff) {
		//printf("HS_TX_TO over limit, setting it to max\n");
		ticks = 0x1fff;
		x4 = 1;
		x16 = 1;
	}

	r = dsi_read_reg(lcd_ix, DSI_TIMING2);
	r = FLD_MOD(r, 1, 31, 31);	
	r = FLD_MOD(r, x16, 30, 30);	
	r = FLD_MOD(r, x4, 29, 29);	
	r = FLD_MOD(r, ticks, 28, 16);	
	dsi_write_reg(lcd_ix, DSI_TIMING2, r);

 
}

static int dsi_proto_config(void)
{
	u32 r;
	int buswidth = 0;
	enum dsi lcd_ix;

	enum omap_dsi_mode dsi_mode = OMAP_DSI_MODE_VIDEO;
	
	lcd_ix = dsi2;

	if (1) {
#if defined (CONFIG_LGE_P2) 
		enum fifo_size size1 = DSI_FIFO_SIZE_32;
		enum fifo_size size2 = DSI_FIFO_SIZE_32;
#else
		enum fifo_size size1 =  (OMAP_DSI_MODE_VIDEO == dsi_mode) ? ( (DSI_VC_VIDEO == 0) ? DSI_FIFO_SIZE_0 : DSI_FIFO_SIZE_64) : DSI_FIFO_SIZE_64;
		enum fifo_size size2 = (OMAP_DSI_MODE_VIDEO == dsi_mode) ? ( (DSI_VC_VIDEO == 1) ? DSI_FIFO_SIZE_0 : DSI_FIFO_SIZE_64) : DSI_FIFO_SIZE_64;
#endif
		dsi_config_tx_fifo(lcd_ix, size1,
			size2,
			DSI_FIFO_SIZE_0,
			DSI_FIFO_SIZE_0);

		dsi_config_rx_fifo(lcd_ix, size1,
			size2,
			DSI_FIFO_SIZE_0,
			DSI_FIFO_SIZE_0);
	}

	dsi_set_stop_state_counter(lcd_ix, 1000);
	dsi_set_ta_timeout(lcd_ix, 6400000);
	dsi_set_lp_rx_timeout(lcd_ix, 48000);
	dsi_set_hs_tx_timeout(lcd_ix, 1000000);

	switch (DSI_CTRL_PIXEL_SIZE) {
	case 16:
		buswidth = 0;
		break;
	case 18:
		buswidth = 1;
		break;
	case 24:
		buswidth = 2;
		break;
	default:
		break;
	}

	r = dsi_read_reg(lcd_ix, DSI_CTRL);
	r = FLD_MOD(r, (1) ? 0 : 1,
			1, 1);				
	r = FLD_MOD(r, (1) ? 0 : 1,
			2, 2);				
	r = FLD_MOD(r, 1, 3, 3);	
	r = FLD_MOD(r, 1, 4, 4);	
	r = FLD_MOD(r, buswidth, 7, 6); 
	r = FLD_MOD(r, 0, 8, 8);	
	r = FLD_MOD(r, 1, 14, 14);      
	
	if(OMAP_DSI_MODE_VIDEO == dsi_mode)
	{
		
		r = FLD_MOD(r, 2, 13, 12);      
		r = FLD_MOD(r, (1) ? 1 : 0,
			9, 9); 							
		r = FLD_MOD(r, 1, 15, 15);      
		r = FLD_MOD(r, 0, 16, 16);      
		r = FLD_MOD(r, 1, 17, 17);      
		r = FLD_MOD(r, 0, 18, 18);	
		r = FLD_MOD(r, 0, 20, 20);	
		r = FLD_MOD(r, 0, 21, 21);	
		r = FLD_MOD(r, 0, 22, 22);	
		r = FLD_MOD(r, 0, 23, 23);      		
		r = FLD_MOD(r, (1) ? 1 : 0,
			11, 11);					
		r = FLD_MOD(r, (1) ? 1 : 0,
			10, 10);			
		
		r = FLD_MOD(r, 0,19, 19);					
	}
	else
	{
		r = FLD_MOD(r, 2, 13, 12);      
		r = FLD_MOD(r, (1) ? 0 : 1,
			19, 19);							
	}	
	
	if (0) {
		r = FLD_MOD(r, 1, 24, 24);	
		
		r = FLD_MOD(r, 0, 25, 25);
	}
	dsi_write_reg(lcd_ix, DSI_CTRL, r);

	dsi_vc_initial_config(lcd_ix, DSI_VC_CMD);
#if ! defined (CONFIG_LGE_P2) 
	if (1)
		dsi_vc_initial_config_vp(lcd_ix, DSI_VC_VIDEO, dsi_mode);
#endif
	
	dsi_2.vc[0].dest_per = 0;
	dsi_2.vc[1].dest_per = 0;
	dsi_2.vc[2].dest_per = 0;
	dsi_2.vc[3].dest_per = 0;

	return 0;
}

static void dsi_proto_timings(void)
{
	unsigned tlpx, tclk_zero, tclk_prepare, tclk_trail;
	unsigned tclk_pre, tclk_post;
	unsigned ths_prepare, ths_prepare_ths_zero, ths_zero;
	unsigned ths_trail, ths_exit;
	unsigned ddr_clk_pre, ddr_clk_post;
	unsigned enter_hs_mode_lat, exit_hs_mode_lat;
	unsigned ths_eot;
	u32 r;
	enum dsi lcd_ix;
	lcd_ix = dsi2;

	r = dsi_read_reg(lcd_ix, DSI_DSIPHY_CFG0);
	ths_prepare = FLD_GET(r, 31, 24);
	ths_prepare_ths_zero = FLD_GET(r, 23, 16);
	ths_zero = ths_prepare_ths_zero - ths_prepare;
	ths_trail = FLD_GET(r, 15, 8);
	ths_exit = FLD_GET(r, 7, 0);

	r = dsi_read_reg(lcd_ix, DSI_DSIPHY_CFG1);
	tlpx = FLD_GET(r, 22, 16) * 2;
	tclk_trail = FLD_GET(r, 15, 8);
	tclk_zero = FLD_GET(r, 7, 0);

	r = dsi_read_reg(lcd_ix, DSI_DSIPHY_CFG2);
	tclk_prepare = FLD_GET(r, 7, 0);

	tclk_pre = 20;
	if(1)
		tclk_pre = 8;
	
	tclk_post = ns2ddr(lcd_ix, 60) + 26;

	if (DSI_DATA1_LANE != 0 && DSI_DATA2_LANE != 0)
		ths_eot = 2;
	else
		ths_eot = 4;

	if(OMAP_DSI_MODE_VIDEO == dsi_2.dsi_mode)
		ths_eot = 0;

	//printf("tclk_post=%d, tclk_trail=%d\n", tclk_post, tclk_trail);
	ddr_clk_pre = DIV_ROUND_UP(tclk_pre + tlpx + tclk_zero + tclk_prepare,
			4);
	ddr_clk_post = DIV_ROUND_UP(tclk_post + tclk_trail, 4) + ths_eot;

 
	r = dsi_read_reg(lcd_ix, DSI_CLK_TIMING);
	r = FLD_MOD(r, ddr_clk_pre, 15, 8);
	r = FLD_MOD(r, ddr_clk_post, 7, 0);
	dsi_write_reg(lcd_ix, DSI_CLK_TIMING, r);

	enter_hs_mode_lat = 1 + DIV_ROUND_UP(tlpx, 4) +
		DIV_ROUND_UP(ths_prepare, 4) +
		DIV_ROUND_UP(ths_zero + 3, 4);

	exit_hs_mode_lat = DIV_ROUND_UP(ths_trail + ths_exit, 4) + 1 + ths_eot;

	r = FLD_VAL(enter_hs_mode_lat, 31, 16) |
		FLD_VAL(exit_hs_mode_lat, 15, 0);
	dsi_write_reg(lcd_ix, DSI_VM_TIMING7, r);

}

#define DSI_DECL_VARS \
	int __dsi_cb = 0; u32 __dsi_cv = 0;

#define DSI_FLUSH(no, ch) \
	if (__dsi_cb > 0) { \
		 \
		dsi_write_reg(no, DSI_VC_LONG_PACKET_PAYLOAD(ch), __dsi_cv); \
		__dsi_cb = __dsi_cv = 0; \
	}

#define DSI_PUSH(no, ch, data) \
	do { \
		__dsi_cv |= (data) << (__dsi_cb * 8); \
		 \
		if (++__dsi_cb > 3) \
			DSI_FLUSH(no, ch); \
	} while (0)

 void dsi_set_update_region(enum dsi lcd_ix,
	u16 x, u16 y, 	u16 w, u16 h)
{
	if (dsi_2.update_region.dirty) {
		dsi_2.update_region.x = min(x, dsi_2.update_region.x);
		dsi_2.update_region.y = min(y, dsi_2.update_region.y);
		dsi_2.update_region.w = max(w, dsi_2.update_region.w);
		dsi_2.update_region.h = max(h, dsi_2.update_region.h);
	} else {
		dsi_2.update_region.x = x;
		dsi_2.update_region.y = y;
		dsi_2.update_region.w = w;
		dsi_2.update_region.h = h;
	}

	dsi_2.update_region.dirty = 1;
}

static int dsi_configure_dsi_clocks(void)
{
	struct dsi_clock_info cinfo;
	int r;
	enum dsi lcd_ix;
	lcd_ix = dsi2;

		cinfo.use_dss2_fck = 1;

	cinfo.regn  = 20;  

	cinfo.regm  = 177; 

	cinfo.regm3 = 5; 
	cinfo.regm4 = 7; 

	r = dsi_calc_clock_rates(&cinfo);
	if (r)
		return r;

	r = dsi_pll_set_clock_div(lcd_ix, &cinfo);
	if (r) {
		//printf("Failed to set dsi clocks\n");
		return r;
	}

	return 0;
}
static int dsi_configure_dispc_clocks(void)
{
	struct dispc_clock_info dispc_cinfo;
	int r;
	unsigned long long fck;
	enum dsi lcd_ix;

	lcd_ix = dsi2;
	
	fck = dsi_get_dsi1_pll_rate(lcd_ix);

	dispc_cinfo.lck_div = 1;
	dispc_cinfo.pck_div = 5;

	r = dispc_calc_clock_rates(fck, &dispc_cinfo);
	if (r) {
		//printf("Failed to calc dispc clocks\n");
		return r;
	}

	r = dispc_set_clock_div(OMAP_DSS_CHANNEL_LCD2, &dispc_cinfo);
	if (r) {
		//printf("Failed to set dispc clocks\n");
		return r;
	}

	return 0;
}

void dsi_enable_video_mode(void)
{
	u8 pixel_type;
	u16 n_bytes;
	enum dsi lcd_ix;
	
	lcd_ix = dsi2;
	dsi_if_enable(lcd_ix, 0);

	switch (DSI_CTRL_PIXEL_SIZE) {
	case 16:
		pixel_type = DSI_PACKED_PIXEL_STREAM_16;
		n_bytes = 480 * 2;
		break;
	case 18:
		pixel_type = DSI_PACKED_PIXEL_STREAM_18;
		n_bytes = (480 * 18) / 8;
		break;
	case 24:
	default:
		pixel_type = DSI_PACKED_PIXEL_STREAM_24;
		n_bytes = 480 * 3;
		break;	
	}
#if defined (CONFIG_LGE_P2) 
	dsi_vc_config_vp(lcd_ix, DSI_VC_VIDEO);
	dsi_vc_write_long_header(lcd_ix, DSI_VC_VIDEO, pixel_type, n_bytes, 0);
#else
	dsi_vc_enable(lcd_ix, DSI_VC_CMD, 0); 
	dsi_vc_write_long_header(lcd_ix, DSI_VC_VIDEO, pixel_type, n_bytes, 0);		
	dsi_vc_enable(lcd_ix, DSI_VC_VIDEO, 1);
#endif
	dsi_if_enable(lcd_ix, 1);	
}


static inline void dss_write_reg(const struct dss_reg idx, u32 val)
{
	MmioWrite32(DSS_BASE + idx.idx,val);
}


static void dsi_set_video_mode_timings(void)
{
	u32 r;
	u16 window_sync;
	u16 vm_tl;
	u16 n_lanes;
	enum dsi lcd_ix;	
	lcd_ix = dsi2;

	window_sync = 4;
	r = FLD_VAL(101, 11, 0) | FLD_VAL(20, 23, 12) | FLD_VAL(58, 31, 24);
	dsi_write_reg(lcd_ix, DSI_VM_TIMING1, r);

	r = FLD_VAL(22, 7, 0) | FLD_VAL(1, 15, 8) | FLD_VAL(8, 23, 16) | FLD_VAL(window_sync, 27, 24);
	dsi_write_reg(lcd_ix, DSI_VM_TIMING2, r);

	n_lanes = 2;	
	vm_tl = ((24) *
		(480 +
		 51 + 
		 19 +
		 14) )/ (8 * n_lanes);	

	r = FLD_VAL(800, 15, 0) | FLD_VAL(vm_tl, 31, 16);
	dsi_write_reg(lcd_ix, DSI_VM_TIMING3, r);
} 
int dsi2_init(void)
{
	u32 l;
	int r;
	enum dsi lcd_ix = dsi2;

	dsi_2.errors = 0;

	dsi_2.update_mode = OMAP_DSS_UPDATE_DISABLED;
	dsi_2.user_update_mode = OMAP_DSS_UPDATE_DISABLED;
	dsi_2.dsi_mode = OMAP_DSI_MODE_VIDEO;	 
	
#if 0
	rev = dsi_read_reg(lcd_ix, DSI_REVISION);
	printf("OMAP DSI2 rev %d.%d\n",
	FLD_GET(rev, 7, 4), FLD_GET(rev, 3, 0));
#endif

	DEBUG((EFI_D_INFO,"dsi_display_enable\n"));
	
	if (1)
		*((volatile unsigned int*) 0x4A307100)	|=	0x00030007;  

	REG_FLD_MOD(dsi2, DSI_SYSCONFIG, 1, 1, 1);
	r = _dsi_wait_reset(dsi2);
	if (r)
		return r;

	dispc_set_control2_reg();
	dispc_set_lcd_timings(OMAP_DSS_CHANNEL_LCD2, &cosmo_panel_timings); 
	dispc_set_lcd_size(OMAP_DSS_CHANNEL_LCD2, 480, 801);
	dispc_set_pol_freq(cosmo_config, cosmo_acbi, cosmo_acb);

	r = dsi_pll_init(lcd_ix, 1, 1);
	if (r)
		return r;

	r = dsi_configure_dsi_clocks();
	if (r)
		return r;

	r = dss_read_reg(DSS_CONTROL);	
	r = FLD_MOD(r, 1, 10, 10);	
	r = FLD_MOD(r, 1, 12, 12);	
	dss_write_reg(DSS_CONTROL, r);
	
	//puts("PLL OK\n");

	r = dsi_configure_dispc_clocks();
	if (r)
		return r;

	//printk("dsi_configure_dispc_clocks is ok\n");

	r = dsi_complexio_init(lcd_ix);
	if (r)
		return r;
	//printk("dsi_complexio_init is ok\n");

	dsi_proto_timings();
	dsi_set_lp_clk_divisor();

	if (OMAP_DSI_MODE_VIDEO == dsi_2.dsi_mode) {
		dsi_set_video_mode_timings();		
	}
	
	r = dsi_proto_config();
	if (r)
		return r;
	//printk("dsi_proto_config is ok\n");

	dsi_vc_enable(lcd_ix, DSI_VC_CMD, 1);
	dsi_if_enable(lcd_ix, 1);
	dsi_force_tx_stop_mode_io(lcd_ix);

#ifdef CONFIG_ARCH_OMAP4
	
	dsi_write_reg(lcd_ix, DSI_DSIPHY_CFG12, 0x58);

	l = dsi_read_reg(lcd_ix, DSI_DSIPHY_CFG14);
	l = FLD_MOD(l, 1, 31, 31);
	l = FLD_MOD(l, 0x54, 30, 23);
	l = FLD_MOD(l, 1, 19, 19);
	l = FLD_MOD(l, 1, 18, 18);
	l = FLD_MOD(l, 7, 17, 14);
	l = FLD_MOD(l, 1, 11, 11);
	dsi_write_reg(lcd_ix, DSI_DSIPHY_CFG14, l);

	l = dsi_read_reg(lcd_ix, DSI_DSIPHY_CFG8);
	l = FLD_MOD(l, 1, 11, 11);
	l = FLD_MOD(l, 0x10, 10, 6);
	l = FLD_MOD(l, 1, 5, 5);
	l = FLD_MOD(l, 0xE, 3, 0);
	dsi_write_reg(lcd_ix, DSI_DSIPHY_CFG8, l);
#endif

	dsi_2.use_ext_te = 1;

	dss_state = OMAP_DSS_DISPLAY_ACTIVE;

	return 0;
}

void init_panel()
{


#if defined (CONFIG_LGE_CX2)
	unsigned char maker_id;
#endif
	*((volatile unsigned int*) 0x4a100600) = 0x14700000;
	*((volatile unsigned int*) 0x4A100604) = 0x88888888;

	omap_set_gpio_direction(GPIO_FLASH_EN, 0);
	omap_set_gpio_dataout(GPIO_FLASH_EN, 0);

#define GPIO_AUD_PWRON		127
	omap_set_gpio_direction(GPIO_AUD_PWRON, 0);
	omap_set_gpio_dataout(GPIO_AUD_PWRON, 0);
#define CM_DSS_L3INIT_CLKCTRL		0x4a009300
#define CM_DSS_CLKSTCTRL                0x4a009100
#define CM_DSS_DSS_CLKCTRL              0x4a009120
#define CM_DSS_DEISS_CLKCTRL            0x4a009128
#define CM_CLKSEL_CORE				0x4a004100
	lcd_init();
#if 1
	sr32(CM_CLKSEL_CORE,0,32,0x118);

	sr32(CM_DSS_CLKSTCTRL,0,32,0x2);
	sr32(CM_DSS_DSS_CLKCTRL,0,32,0x00000f02);
	sr32(CM_DSS_L3INIT_CLKCTRL,0,32,0x00000002);
#endif
	*(volatile int*) 0x4A307100 = 0x00030007;


	UINT32 l;
	l = dispc_read_reg(DISPC_SYSCONFIG);
	l = FLD_MOD(l, 2, 13, 12);
	l = FLD_MOD(l, 2, 4, 3);
	l = FLD_MOD(l, 1, 2, 2);
	l = FLD_MOD(l, 1, 0, 0);
	dispc_write_reg(DISPC_SYSCONFIG, l);

	l = dispc_read_reg(DISPC_CONFIG);
	l = FLD_MOD(l, OMAP_DSS_LOAD_FRAME_ONLY, 2, 1);
	dispc_write_reg(DISPC_CONFIG, l);

	dsi2_init();
}
UINT32* Draw = (UINT32*)0x87000000;
void loaddisplay()
{

    init_panel();
  


	dispc_set_control2_reg();
	dispc_set_lcd_size(OMAP_DSS_CHANNEL_LCD2, 480, 801);


	  configure_dispc();

    u8 buf;
	buf = DCS_DISPLAY_ON;
	dsi_vc_dcs_write(1,TCH, &buf, 1);

	dsi_enable_video_mode();	
    dispc_go(OMAP_DSS_CHANNEL_LCD2);
	dispc_enable_lcd_out(1);
	
	SetMem32(Draw, 0x100, 0xFF00FF);
	  DEBUG((EFI_D_INFO, "\nLCD SETUP DONE\n"));
}
