/* Copyright (c) 2008-2009, LG Electronics Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

#include "msm_fb.h"

#include <linux/memory.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include "linux/proc_fs.h"

#include <linux/delay.h>

#include <mach/hardware.h>
#include <mach/board_lge.h>
#include <linux/io.h>
#include <linux/gpio.h>

#include <asm/system.h>
#include <asm/mach-types.h>

#define DEBUG 1
#define EPRINTK(fmt, args...) printk(fmt, ##args)
#if DEBUG
#define DPRINTK(fmt, args...) printk(fmt, ##args)
#else
#define DPRINTK(fmt, args...) do { } while (0)
#endif

//Enable SWAP_SCANLINE if the panel is to be used in 240x320 orientation rather than native 320x240.
//#define SWAP_SCANLINE

#ifdef SWAP_SCANLINE
#define QVGA_WIDTH        240
#define QVGA_HEIGHT       320
#else
#define QVGA_WIDTH        320
#define QVGA_HEIGHT       240
#endif

#define DISP_QVGA_18BPP(x)  (x)
#define DISP_REG(name)  uint16 register_##name;
#define OUTPORT(x, y)  outpw(x, y)
#define INPORT(x)   inpw(x)


static void *DISP_CMD_PORT;
static void *DISP_DATA_PORT;

#define DISP_RNTI         0x10

#define DISP_CMD_OUT(cmd) OUTPORT(DISP_CMD_PORT, DISP_QVGA_18BPP(cmd))
#define DISP_DATA_OUT(data) OUTPORT(DISP_DATA_PORT, data)
#define DISP_DATA_IN() INPORT(DISP_DATA_PORT)

#if (defined(LH240QVGA_LCD_18BPP))
#define DISP_DATA_OUT_16TO18BPP(x) \
	DISP_DATA_OUT((((x)&0xf800)<<2|((x)&0x80000)>>3) \
		     | (((x)&0x7e0)<<1) \
		     | (((x)&0x1F)<<1|((x)&0x10)>>4))
#else
#define DISP_DATA_OUT_16TO18BPP(x) \
	DISP_DATA_OUT(x)
#endif


#define DISP_SET_RECT(start_row, end_row, start_col, end_col) \
   { \
       DISP_CMD_OUT(0x2B); \
       DISP_DATA_OUT(((start_row)&0xff00)>>8);\
       DISP_DATA_OUT((start_row)&0xff);\
       DISP_DATA_OUT(((end_row)&0xff00)>>8);\
       DISP_DATA_OUT((end_row)&0xff);\
       DISP_CMD_OUT(0x2A); \
       DISP_DATA_OUT(((start_col)&0xff00)>>8);\
       DISP_DATA_OUT((start_col)&0xff);\
       DISP_DATA_OUT(((end_col)&0xff00)>>8);\
       DISP_DATA_OUT((end_col)&0xff);\
   }

#define WAIT_MSEC(msec) mdelay(msec)

//static struct msm_panel_common_pdata *ebi2_lgit_pdata;
static struct msm_panel_ebi2lcd_pdata *ebi2_lgit_pdata;

static boolean disp_initialized = FALSE;
static boolean display_on = FALSE;

static uint32 auox04qvga_lcd_rev;
uint16 auox04qvga_panel_offset;

static void auox04qvga_disp_set_rect(int x, int xres, int y, int yres);
static void auox04qvga_disp_init(struct platform_device *pdev);
static int auox04qvga_disp_off(struct platform_device *pdev);
static int auox04qvga_disp_on(struct platform_device *pdev);
static void auox04qvga_set_revId(int);
static void auox04qvga_disp_exec_init_sequence(void);

/* future use */
void auox04qvga_disp_clear_screen_area(word start_row, word end_row,
				      word start_column, word end_column);

/* future use */
static void auox04qvga_set_revId(int id)
{

	auox04qvga_lcd_rev = id;

}

static void auox04qvga_disp_reset(void)
{

	printk("ebi2_gpio = %d\n", ebi2_lgit_pdata->gpio);
	gpio_request (ebi2_lgit_pdata->gpio, "2p0_sublcd_rst_n");
	gpio_direction_output (ebi2_lgit_pdata->gpio, 1);
	gpio_set_value (ebi2_lgit_pdata->gpio, 0);
	mdelay (10);
	gpio_set_value (ebi2_lgit_pdata->gpio, 1);
	mdelay (20);

}

static void auox04qvga_disp_release(struct platform_device *pdev){
	DISP_CMD_OUT(0x28);	//Display OFF
	DISP_CMD_OUT(0x10);	//Sleep IN
	mdelay(120);	//delay time
	gpio_free(ebi2_lgit_pdata->gpio);
}

static void auox04qvga_disp_sleep(struct platform_device *pdev){
	DISP_CMD_OUT(0x28);	//Display OFF
	DISP_CMD_OUT(0x10);	//Sleep IN
	//mdelay(120);		//delay time
}

static void auox04qvga_disp_wakeup(struct platform_device *pdev){
	DISP_CMD_OUT(0x11);	//Sleep OUT
	mdelay(120);		//delay time	
	DISP_CMD_OUT(0x29);	//Display ON
}

static void auox04qvga_disp_init(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;

	printk("%s+\n", __func__);
	if (disp_initialized)
		return;

	mfd = platform_get_drvdata(pdev);

	DISP_CMD_PORT = mfd->cmd_port;
	DISP_DATA_PORT = mfd->data_port;
	printk("%s cmd=%p, data=%p\n", __func__, DISP_CMD_PORT, DISP_DATA_PORT  );

	disp_initialized = TRUE;
	if (ebi2_lgit_pdata->initialized){
	/*pre-initialized by modem; do only bare minninum*/
		DISP_CMD_OUT(0x28);	//Display OFF
#ifdef SWAP_SCANLINE
		DISP_CMD_OUT(0x36);	//scan-lining [MADCTL]
		DISP_DATA_OUT(0x20);	//Portrait scan-lining
#else
		DISP_CMD_OUT(0x36);	//scan-lining [MADCTL]
		DISP_DATA_OUT(0x90);	//Landscape scan-lining
#endif
//ebi2 camp. TE enable.
	DISP_CMD_OUT(0x35);
	DISP_DATA_OUT(0x0); 		//TE

		auox04qvga_disp_set_rect(0, QVGA_HEIGHT-1, 0, QVGA_WIDTH-1);
		DISP_CMD_OUT(0x29);	//Display ON
		gpio_request (ebi2_lgit_pdata->gpio, "2p0_sublcd_rst_n");
		printk("%s pre-initialized -\n", __func__);
		return;
	}

	auox04qvga_set_revId(1);

	auox04qvga_disp_reset();
	auox04qvga_disp_exec_init_sequence();

	auox04qvga_disp_set_rect(0, QVGA_HEIGHT-1, 0, QVGA_WIDTH-1);
	printk("%s-\n", __func__);
	
}

static void auox04qvga_disp_exec_init_sequence()
{
// EBI2 AOU LCD INITIAL CODE (2011.06.30)
	clk_busy_wait(50000);
	DISP_CMD_OUT(0x11);	//SLPOUT
	clk_busy_wait(120000);
			
	DISP_CMD_OUT(0xF0);	//PASSWD1
	DISP_DATA_OUT(0x5A);
	DISP_DATA_OUT(0x5A);
	
	DISP_CMD_OUT(0xF1);	//PASSWD2
	DISP_DATA_OUT(0x5A);
	DISP_DATA_OUT(0x5A);
			
	DISP_CMD_OUT(0xFC);     //VCOM OFFSET
	DISP_DATA_OUT(0xA5);
	DISP_DATA_OUT(0xA5);
	
	DISP_CMD_OUT(0xF2);	//DISCTL
	DISP_DATA_OUT(0x1E);
	DISP_DATA_OUT(0x75);
	DISP_DATA_OUT(0x03);
	DISP_DATA_OUT(0x78);
	DISP_DATA_OUT(0x78);
	DISP_DATA_OUT(0x10);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x3D);
	DISP_DATA_OUT(0xF8);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x07);
	DISP_DATA_OUT(0x00);  //Normally Black=0 /Normally white=1
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x75);
	DISP_DATA_OUT(0x78);
	DISP_DATA_OUT(0x78);
	
	DISP_CMD_OUT(0xF4);	//PWRCTL
	DISP_DATA_OUT(0x07);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x77);
	DISP_DATA_OUT(0x7F);
	DISP_DATA_OUT(0x07);
	DISP_DATA_OUT(0x22); 
	DISP_DATA_OUT(0x2A);
	DISP_DATA_OUT(0x4B);
	DISP_DATA_OUT(0x03);
	DISP_DATA_OUT(0x2A);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x03);
	
	DISP_CMD_OUT(0xF5);	//VCMCTL
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x5D);
	DISP_DATA_OUT(0x2B);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x03);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x00);
	
	DISP_CMD_OUT(0xF6);	//SRCCTL
	DISP_DATA_OUT(0x02);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x07);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x04);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x04);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x07);
	
	DISP_CMD_OUT(0xF7);	//IFCTL
	DISP_DATA_OUT(0x02);
	DISP_DATA_OUT(0x00);	//0x00 ->0x02 VSYNC ON
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x00);
	
	DISP_CMD_OUT(0xF9);	//GAMMASEL RED
	DISP_DATA_OUT(0x04);
	
	DISP_CMD_OUT(0xFA);
	DISP_DATA_OUT(0x08);
	DISP_DATA_OUT(0x2D);
	DISP_DATA_OUT(0x02);
	DISP_DATA_OUT(0x07);
	DISP_DATA_OUT(0x07);
	DISP_DATA_OUT(0x1D);
	DISP_DATA_OUT(0x1E);
	DISP_DATA_OUT(0x21);
	DISP_DATA_OUT(0x18);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x01);                                         
	                           
	DISP_CMD_OUT(0xFB);
	DISP_DATA_OUT(0x08);
	DISP_DATA_OUT(0x2D);
	DISP_DATA_OUT(0x02);
	DISP_DATA_OUT(0x07);
	DISP_DATA_OUT(0x07);
	DISP_DATA_OUT(0x1D);
	DISP_DATA_OUT(0x1E);
	DISP_DATA_OUT(0x21);
	DISP_DATA_OUT(0x18);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x00);		
	
	DISP_CMD_OUT(0xF9);	//GAMMASEL GREEN
	DISP_DATA_OUT(0x02);
	
	DISP_CMD_OUT(0xFA);
	DISP_DATA_OUT(0x08);
	DISP_DATA_OUT(0x0F);
	DISP_DATA_OUT(0x0C);
	DISP_DATA_OUT(0x13);
	DISP_DATA_OUT(0x1D);
	DISP_DATA_OUT(0x2A);
	DISP_DATA_OUT(0x26);
	DISP_DATA_OUT(0x23);
	DISP_DATA_OUT(0x21);
	DISP_DATA_OUT(0x06);
	DISP_DATA_OUT(0x0D);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x01);                                         
	                           
	DISP_CMD_OUT(0xFB);
	DISP_DATA_OUT(0x08);
	DISP_DATA_OUT(0x0F);
	DISP_DATA_OUT(0x0C);
	DISP_DATA_OUT(0x13);
	DISP_DATA_OUT(0x1D);
	DISP_DATA_OUT(0x2A);
	DISP_DATA_OUT(0x26);
	DISP_DATA_OUT(0x23);
	DISP_DATA_OUT(0x21);
	DISP_DATA_OUT(0x06);
	DISP_DATA_OUT(0x0D);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x00);
	                         
	DISP_CMD_OUT(0xF9);	//GAMMASEL BLUE 
	DISP_DATA_OUT(0x01);
	
	DISP_CMD_OUT(0xFA);
	DISP_DATA_OUT(0x08);
	DISP_DATA_OUT(0x0F);
	DISP_DATA_OUT(0x1C);
	DISP_DATA_OUT(0x20);
	DISP_DATA_OUT(0x21);
	DISP_DATA_OUT(0x2E);
	DISP_DATA_OUT(0x26);
	DISP_DATA_OUT(0x23);
	DISP_DATA_OUT(0x23);
	DISP_DATA_OUT(0x11);
	DISP_DATA_OUT(0x1C);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x01);
	                         
	DISP_CMD_OUT(0xFB);
	DISP_DATA_OUT(0x08);
	DISP_DATA_OUT(0x0F);
	DISP_DATA_OUT(0x1C);
	DISP_DATA_OUT(0x20);
	DISP_DATA_OUT(0x21);
	DISP_DATA_OUT(0x2E);
	DISP_DATA_OUT(0x26);
	DISP_DATA_OUT(0x23);
	DISP_DATA_OUT(0x23);
	DISP_DATA_OUT(0x11);
	DISP_DATA_OUT(0x1C);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x00);
	        
	DISP_CMD_OUT(0x3A);
	DISP_DATA_OUT(0x55);
#ifdef SWAP_SCANLINE
	DISP_CMD_OUT(0x36);
	DISP_DATA_OUT(0x20); 		//Portrait scan-lining
#else
	DISP_CMD_OUT(0x36);
	DISP_DATA_OUT(0x90); 		//Landscape scan-lining
#endif
//ebi2 camp. TE enable.
	DISP_CMD_OUT(0x35);
	DISP_DATA_OUT(0x0); 		//TE
	
	DISP_CMD_OUT(0x29);		//Display On
	
	DISP_CMD_OUT(0x2A);   // x size
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x01);
	DISP_DATA_OUT(0x3F);
	
	DISP_CMD_OUT(0x2B);	// y size
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0x00);
	DISP_DATA_OUT(0xEF);
	
	DISP_CMD_OUT(0x2C); // gram write

}

static void auox04qvga_disp_set_rect_exported(int x, int y, int xres, int yres)
{
	if (!disp_initialized)
		return;

#ifdef SWAP_SCANLINE
	//DISP_SET_RECT(x,xres,y,yres);
	DISP_SET_RECT(y,yres,x,xres);
#else
	DISP_SET_RECT(y,yres,x,xres);
	//DISP_SET_RECT(x,xres,y,yres);
	//DISP_SET_RECT(0,239,0,319);
#endif
	DISP_CMD_OUT(0x2C);

}



static void auox04qvga_disp_set_rect(int x, int xres, int y, int yres)
{
	if (!disp_initialized)
		return;

	DISP_SET_RECT(x,xres,y,yres);
	DISP_CMD_OUT(0x2C);

}


static int auox04qvga_disp_off(struct platform_device *pdev)
{
	if (!disp_initialized)
	{
		printk("%s: disp_initialized=FALSE!\n", __func__);
		return 0;
	}
	auox04qvga_disp_sleep(pdev);
	display_on = FALSE;

	return 0;
}

void put_test_pattern(void)
{

	int i,count, BARS=4;
	int start_row, end_row, start_col, end_col;
	uint16 value[BARS];
	static int pass;
	uint16 values[]={0xffff, 0x001f, 0x07e0, 0xf800};

	start_row		=	0;
	end_row 		= 	QVGA_HEIGHT - 1;
	start_col		= 	0;
	end_col		= 	QVGA_WIDTH - 1;

	count		= 	QVGA_WIDTH * QVGA_HEIGHT;


	for (i=0; i<BARS; i++)
	{
		value[i] = values[i%(sizeof(values)/sizeof(uint16))];
	}


//	auox04qvga_disp_set_rect(start_row, end_row, start_col, end_col );
	DISP_CMD_OUT(0x2C);	//memory address

	for ( i = 0 ; i < count ; i ++ )
	{
#if defined(LGE_MODEL_C729_REV_EVB)
		if( i < count/4)
		{
			DISP_DATA_OUT(value[(pass + 0)%BARS]>>8);
			DISP_DATA_OUT(value[(pass + 0)%BARS]& 0xFF);
		}
		else if( i < count*2/4)
		{
			DISP_DATA_OUT(value[(pass + 1)%BARS]>>8);
			DISP_DATA_OUT(value[(pass + 1)%BARS]& 0xFF);
		}
		else if( i < count*3/4)
		{
			DISP_DATA_OUT(value[(pass + 2)%BARS]>>8);
			DISP_DATA_OUT(value[(pass + 2)%BARS]& 0xFF);
		}
		else
		{
			DISP_DATA_OUT(value[(pass + 3)%BARS]>>8);
			DISP_DATA_OUT(value[(pass + 3)%BARS]& 0xFF);
		}	
#elif defined(LGE_MODEL_C729_REV_A) || defined(LGE_MODEL_C729_REV_B)|| defined(LGE_MODEL_C729_REV_C)
		if( i < count/4)
		{
			DISP_DATA_OUT(value[(pass + 0)%BARS]);
		}
		else if( i < count*2/4)
		{
			DISP_DATA_OUT(value[(pass + 1)%BARS]);
		}
		else if( i < count*3/4)
		{
			DISP_DATA_OUT(value[(pass + 2)%BARS]);
		}
		else
		{
			DISP_DATA_OUT(value[(pass + 3)%BARS]);
		}	

#else
#error "SubLCD: Device not supported"
#endif
			
	}

	mdelay(1000);	
	pass++;
	printk("TEST PATTERN %d \n", pass);
}

static int auox04qvga_disp_on(struct platform_device *pdev)
{

	if (!disp_initialized)
		auox04qvga_disp_init(pdev);

	auox04qvga_disp_wakeup(pdev);
	display_on = TRUE;
	//put_test_pattern();
	//put_test_pattern();

	return 0;
}

void auox04qvga_disp_clear_screen_area
    (word start_row, word end_row, word start_column, word end_column) {
	int32 i;

	/* Clear the display screen */
	DISP_SET_RECT(start_row, end_row, start_column, end_column);
	DISP_CMD_OUT(0x2C);
	i = (end_row - start_row + 1) * (end_column - start_column + 1);
	for (; i > 0; i--)
		DISP_DATA_OUT_16TO18BPP(0x0);
}


static int auox04qvga_remove(struct platform_device *pdev)
{
	if (disp_initialized){
		auox04qvga_disp_release(pdev);
	}
	return 0;
}

static int auox04qvga_probe(struct platform_device *pdev)
{
	int ret = 0;
	DPRINTK("%s: started.\n", __func__);

	if (pdev->id == 0) {
		ebi2_lgit_pdata = pdev->dev.platform_data;
		return 0;
	}
	msm_fb_add_device(pdev);

	return ret;
}
/*
Generic SubLCD display device's driver, for the device registered by platform (board file).
*/
static struct platform_driver ebi2_qvga_driver = {	
	.probe  = auox04qvga_probe,
	.remove = auox04qvga_remove,
	.driver = {
		.name   = "ebi2_sublcd_qvga",
	},
};

static struct platform_driver this_driver = {	
	.probe  = auox04qvga_probe,
	.driver = {
		.name   = "ebi2_auoX04_qvga",
	},
};

static struct msm_fb_panel_data auox04qvga_panel_data = {
	.on = auox04qvga_disp_on,
	.off = auox04qvga_disp_off,
	.set_rect = auox04qvga_disp_set_rect_exported, //camp debug auox04qvga_disp_set_rect,
	.set_vsync_notifier = NULL, //Apparently, EBI2 doesnot have any vsync mechanism.
};

static struct platform_device this_device = {
	.name   = "ebi2_auoX04_qvga",
	.id	= 1,
	.dev	= {
		.platform_data = &auox04qvga_panel_data,
	}
};

static int __init auox04qvga_init(void)
{
	int ret;
	struct msm_panel_info *pinfo;

	ret = platform_driver_register(&ebi2_qvga_driver);
	ret = platform_driver_register(&this_driver);
	if (!ret) {
		pinfo = &auox04qvga_panel_data.panel_info;
#ifdef SWAP_SCANLINE
		pinfo->xres = 240;
		pinfo->yres = 320;
#else
		pinfo->xres = 320;
		pinfo->yres = 240;
#endif
		pinfo->type = EBI2_PANEL;
		//sumeet: DISPLAY_2, coz it is on secondary EBI2 channel - CS5...
		pinfo->pdest = DISPLAY_2;

		//sumeet TBD: check implications. This is copied as it is to EBI2_LCD_CFG0 in ebi2_lcd.c; 
		pinfo->wait_cycle = 0x4210504; //0x4210882;//0x8821104; //0x808000;
		pinfo->bpp = 16;
#ifdef CONFIG_FB_MSM_DPSV
		pinfo->fb_num = 1;	//Temp. 1, Until support for Panning in DPSV is implemented.
#else
		pinfo->fb_num = 2;
#endif
		pinfo->lcd.vsync_enable = FALSE; //TRUE;;
		pinfo->lcd.refx100 = 6000;
		pinfo->lcd.v_back_porch = 16;	//sumeet TBD: Tune the porch values for Vsync2.
		pinfo->lcd.v_front_porch = 4;
		pinfo->lcd.v_pulse_width = 0;
		pinfo->lcd.hw_vsync_mode = FALSE; //TRUE;;	
		pinfo->lcd.vsync_notifier_period = 0;

		ret = platform_device_register(&this_device);
		if (ret){
			platform_driver_unregister(&this_driver);
			platform_driver_unregister(&ebi2_qvga_driver);
		}
	}

	return ret;
}
//#ifdef CONFIG_FB_MSM_DPSV
//module_init(auox04qvga_init);
//#else
//this module should go after main LCD's module, to give fb0 to main lcd, and fb1 to this.
late_initcall(auox04qvga_init);
//#endif
