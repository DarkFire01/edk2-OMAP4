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

	select_bus(1, OMAP_I2C_STANDARD);

	data_for_lcd	=	0x05;

	i2c_write(0x38, 0x10, 1, &data_for_lcd, 1); 	
	
	data_for_lcd	=		0x42;
	i2c_write(0x38, 0x30, 1, &data_for_lcd, 1); 	
	
	data_for_lcd	=		0x7F;
	i2c_write(0x38, 0xA0, 1, &data_for_lcd, 1); 	

	select_bus(0, OMAP_I2C_STANDARD);			

	omap_set_gpio_direction(GPIO_LCD_RESET_N, 0);	
	omap_set_gpio_dataout(GPIO_LCD_RESET_N, 1); 
	omap_set_gpio_dataout(GPIO_LCD_RESET_N, 0);
	omap_set_gpio_dataout(GPIO_LCD_RESET_N, 1);

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

	lcd_init();
#if 0
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

	///dsi2_init();
}

UINT32* Draw = (UINT32*)0x87000000;
void loaddisplay()
{

    init_panel();
    configure_dispc();



	dispc_set_control2_reg();
	dispc_set_lcd_size(OMAP_DSS_CHANNEL_LCD2, 480, 801);
    dispc_go(OMAP_DSS_CHANNEL_LCD2);
	dispc_set_lcd_size(OMAP_DSS_CHANNEL_LCD, 480, 801);
      dispc_go(OMAP_DSS_CHANNEL_LCD);
	dispc_enable_lcd_out(1);
	 *Draw = 0xFF00FF;
  
}
