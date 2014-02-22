/* arch/arm/mach-msm/lge/board-univaq-panel.c
 * Copyright (C) 2011 LGE Corporation.
 * Author: Kideok Kim <kideok.kim@lge.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/mfd/pmic8058.h>
#include <asm/mach-types.h>
#include <mach/mpp.h>
#include <mach/board.h>
#include <mach/vreg.h>
#include <mach/pmic.h>
#include <mach/rpc_pmapp.h>
#include <mach/board_lge.h>
#include "devices.h"
#include "board-flip.h"

/* LGE_UPDATE_S 20110215 kideok.kim@lge.com Univa-Q Board bring-up */
#define MSM_FB_LCDC_VREG_OP(name, op, level)			\
do { \
	vreg = vreg_get(0, name); \
	vreg_set_level(vreg, level); \
	if (vreg_##op(vreg)) \
		printk(KERN_ERR "%s: %s vreg operation failed \n", \
			(vreg_##op == vreg_enable) ? "vreg_enable" \
				: "vreg_disable", name); \
} while (0)

#if 0

static char *msm_fb_vreg[] = {
	"gp10",
	"gp6",
};

static int mddi_power_save_on;


static void msm_fb_mddi_power_save(int on)
{
	struct vreg *vreg;
	int flag_on = !!on;

	if (mddi_power_save_on == flag_on)
		return;

	mddi_power_save_on = flag_on;

	if (on) {
#if defined (LGE_MODEL_C729_REV_EVB) || defined (LGE_MODEL_C729_REV_A) || defined (LGE_MODEL_C729_REV_B)
		MSM_FB_LCDC_VREG_OP(msm_fb_vreg[0], enable, 1800);
#else		
		vreg = vreg_get(NULL, "lvsw0");
		vreg_enable(vreg);
#endif		
		MSM_FB_LCDC_VREG_OP(msm_fb_vreg[1], enable, 2800);
		
		
	} else{
#if defined (LGE_MODEL_C729_REV_EVB) || defined (LGE_MODEL_C729_REV_A) || defined (LGE_MODEL_C729_REV_B)	
		MSM_FB_LCDC_VREG_OP(msm_fb_vreg[0], disable, 0);
#else
		vreg = vreg_get(NULL, "lvsw0");
		vreg_disable(vreg);
#endif
		MSM_FB_LCDC_VREG_OP(msm_fb_vreg[1], disable, 0);		
	}
}
#endif

//LGE_UPDATE_S minhobb2.kim@lge.com for RevA LCD power & Camera preview tearing
static int msm_fb_mddi_sel_clk(u32 *clk_rate)
{
	*clk_rate *= 2;
	return 0;
}

static struct mddi_platform_data mddi_pdata = {

//	.mddi_power_save = msm_fb_mddi_power_save,
	.mddi_power_save = NULL,			// LCD power
	.mddi_sel_clk = msm_fb_mddi_sel_clk,		// Camera preview tearing	
};
//LGE_UPDATE_E minhobb2.kim@lge.com for RevA LCD power & Camera preview tearing


static struct msm_panel_common_pdata mdp_pdata = {
	.gpio = 30,
	.mdp_core_clk_rate = 122880000,
//[sumeet.gupta 040411 ONE_CODE
#ifdef LGE_MODEL_C729 //dual vsync
        .gpio_vsync2 = 24,
#endif

};




static void __init msm_fb_add_devices(void)
{
	msm_fb_register_device("mdp", &mdp_pdata);
	msm_fb_register_device("pmdh", &mddi_pdata);
	msm_fb_register_device("ebi2", NULL);
}
static int mddi_mainlcd_pmic_backlight(int level)
{
	/* TODO: Backlight control here */
	return 0;
}
static struct msm_panel_lgit_pdata mddi_mainlcd_panel_data = {
#if !defined (LGE_MODEL_C729_REV_EVB)
	.gpio = 94,				/* lcd reset_n */
#else
	.gpio = 95,				/* lcd reset_n */
#endif
	.pmic_backlight = mddi_mainlcd_pmic_backlight,
	.initialized = 1,

};
static struct platform_device mddi_mainlcd_panel_device = {
#if defined (CONFIG_FB_MSM_MDDI_NOVATEK_HVGA)
	.name   = "mddi_novatek_hvga",
#elif defined (CONFIG_FB_MSM_MDDI_AUO_HVGA)	
	.name   = "mddi_auo_hvga",
#endif	
	.id     = 0,
	.dev    = {
		.platform_data = &mddi_mainlcd_panel_data,
	}
};
////////////////////////////////////////////////////////
/* backlight device */
static struct gpio_i2c_pin bl_i2c_pin[] = {
	[0] = {
#if !defined (LGE_MODEL_C729_REV_EVB)
		.sda_pin	= 92,
		.scl_pin	= 93,
#else
		.sda_pin	= 1,
		.scl_pin	= 0,
#endif
		.reset_pin	= 25,
		.irq_pin	= 0,     /* need to be checked */
	},
};

static struct i2c_gpio_platform_data bl_i2c_pdata = {
	.sda_is_open_drain	= 0,
	.scl_is_open_drain	= 0,
	.udelay				= 2,
};

static struct platform_device bl_i2c_device = {
	.name	= "i2c-gpio",
	.dev.platform_data = &bl_i2c_pdata,
};

static struct i2c_board_info bl_i2c_bdinfo[] = {
	[0] = {
		I2C_BOARD_INFO("lm3530", 0x38),
		.type = "lm3530",
		.platform_data = NULL, //&lm3530bl_data[0],
	},
};
//LGE_UPDATE_S kideok.kim@lge.com 20110309 Rev.A Adaptation 
#if defined(CONFIG_BACKLIGHT_LM3530)
static struct backlight_platform_data lm3530bl_data[] = {
	[0] = {
		.gpio = 25,
	},
};
#else
static struct backlight_platform_data aat2870bl_data[] = {
	[0] = {
		.gpio = 25,
		.version = 2862,
	},
};
#endif
//LGE_UPDATE_E kideok.kim@lge.com 20110309 Rev.A Adaptation 

#if 0
#define GPIO_BL_I2C_SDA	75
#define GPIO_BL_I2C_SCL	74

static struct gpio_i2c_pin amp_i2c_pin[] = {
	[0] = {
    .sda_pin = GPIO_BL_I2C_SDA,
    .scl_pin = GPIO_BL_I2C_SCL,
	},
};

static struct i2c_gpio_platform_data amp_i2c_pdata = {
	.sda_is_open_drain = 0,
	.scl_is_open_drain = 0,
	.udelay = 2,
};

static struct platform_device amp_i2c_device = {
	.id = 14,
	.name = "i2c-gpio",
	.dev.platform_data = &amp_i2c_pdata,
};

static struct i2c_board_info amp_i2c_bdinfo[] = {
/* daniel.kang@lge.com ++ */
/* Sharing the GPIO I2C and add audience device here */
/* Change the order : daniel.kang 10.09.10 */

	[0] = {
	I2C_BOARD_INFO("lm3530", 0x38),
	.type = "lm3530",
	.platform_data = &lm3530bl_data[0],
	},


};
#endif

#if 0

int init_gpio_i2c_pin(struct i2c_gpio_platform_data *i2c_adap_pdata,
		struct gpio_i2c_pin gpio_i2c_pin,
		struct i2c_board_info *i2c_board_info_data)
{
	i2c_adap_pdata->sda_pin = gpio_i2c_pin.sda_pin;
	i2c_adap_pdata->scl_pin = gpio_i2c_pin.scl_pin;

	gpio_tlmm_config(GPIO_CFG(gpio_i2c_pin.sda_pin, 0, GPIO_CFG_OUTPUT,
				GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_tlmm_config(GPIO_CFG(gpio_i2c_pin.scl_pin, 0, GPIO_CFG_OUTPUT,
				GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_set_value(gpio_i2c_pin.sda_pin, 1);
	gpio_set_value(gpio_i2c_pin.scl_pin, 1);

	if (gpio_i2c_pin.reset_pin) {
		gpio_tlmm_config(GPIO_CFG(gpio_i2c_pin.reset_pin, 0, GPIO_CFG_OUTPUT,
					GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		gpio_set_value(gpio_i2c_pin.reset_pin, 1);
	}

	if (gpio_i2c_pin.irq_pin) {
		gpio_tlmm_config(GPIO_CFG(gpio_i2c_pin.irq_pin, 0, GPIO_CFG_INPUT,
					GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		i2c_board_info_data->irq =
			MSM_GPIO_TO_INT(gpio_i2c_pin.irq_pin);
	}

	return 0;
}
#endif

//[sumeet.gupta ONE_CODE 040411
static struct resource sub_bl_resource[] = {
   {
      .name = "bl_gpio",
      .start = 143,
      .end = 143,
      .flags = IORESOURCE_IO,
   }
};

static struct platform_device sub_bl_device = {
	.name	= "aat3369_bl",
    .resource = sub_bl_resource,
   .num_resources = ARRAY_SIZE(sub_bl_resource),
};

static int ebi2_sublcd_pmic_backlight(int level)
{
	/* TODO: Backlight control here */
	return 0;
}

static struct msm_panel_ebi2lcd_pdata ebi2_sublcd_panel_data = { 
	.gpio = 33,				/* lcd reset_n */
	.pmic_backlight = ebi2_sublcd_pmic_backlight,
	.initialized = 1,
};

static struct platform_device ebi2_sublcd_panel_device = {
	.name   = "ebi2_sublcd_qvga",
	.id     = 0,
	.dev    = {
		.platform_data = &ebi2_sublcd_panel_data,
	}
};

static int ebi2_sublcd_init (void){
	#define GPIO_EBI2_LCD_CS 89
	#define FUNC_EBI2_CS5_N 1
	int ret;
	ret = gpio_tlmm_config(GPIO_CFG(GPIO_EBI2_LCD_CS, FUNC_EBI2_CS5_N, GPIO_CFG_OUTPUT, 
					GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);

	#define GPIO_EBI2_ADR15_OUT 163
	#define FUNC_EBI2_ADR_OUT 2
	ret = gpio_tlmm_config(GPIO_CFG(GPIO_EBI2_ADR15_OUT, FUNC_EBI2_ADR_OUT, GPIO_CFG_OUTPUT, 
					GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);

	#define GPIO_LCD2_RESET 33
	ret = gpio_tlmm_config(GPIO_CFG(GPIO_LCD2_RESET, 0, GPIO_CFG_OUTPUT, 
					GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	return ret;
}


//]sumeet.gupta ONE_CODE 040411



void __init flip_init_i2c_backlight(int bus_num)
{
	bl_i2c_device.id = bus_num;
#if defined(CONFIG_BACKLIGHT_LM3530)
	bl_i2c_bdinfo[0].platform_data = &lm3530bl_data[0]; //[lge_bd_rev];
#else
	bl_i2c_bdinfo[0].platform_data = &aat2870bl_data[0]; //[lge_bd_rev];
#endif		
	init_gpio_i2c_pin(&bl_i2c_pdata, bl_i2c_pin[0],	&bl_i2c_bdinfo[0]);
	i2c_register_board_info(bus_num, &bl_i2c_bdinfo[0], 1);    // bus_num = 12 ? kideok.kim.
	platform_device_register(&bl_i2c_device);
}

void __init lge_add_lcd_devices(void)
{

	platform_device_register(&mddi_mainlcd_panel_device);
//LGE_UPDATE_S minhobb2.kim@lge.com for MainBL
#if defined (LGE_MODEL_C729_REV_EVB)
	lge_add_gpio_i2c_device(flip_init_i2c_backlight);
#endif
//[sumeet.gupta ONE_CODE 040411
	platform_device_register(&sub_bl_device);
	platform_device_register(&ebi2_sublcd_panel_device);
	printk("%d", ebi2_sublcd_panel_device.id);
	ebi2_sublcd_init ();
//]sumeet.gupta ONE_CODE 040411
	msm_fb_add_devices();
}
