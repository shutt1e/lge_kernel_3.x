/* Copyright (c) 2009-2010, Code Aurora Forum. All rights reserved.
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

#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <mach/vreg.h>
#include <mach/rpc_server_handset.h>
#include <mach/board.h>
#include <mach/board_lge.h>

/* LGE_UPDATE_S 20110215 kideok.kim@lge.com Univa-Q Board bring-up */

#include "board-flip.h"
#include <linux/gpio_keys.h>
#include <linux/gpio_event.h>

/* LGE_UPDATE_E 20110215 kideok.kim@lge.com Univa-Q Board bring-up */
/* atcmd virtual device : AT%FKPD */
static unsigned int atcmd_virtual_keycode[ATCMD_VIRTUAL_KEYPAD_ROW][ATCMD_VIRTUAL_KEYPAD_COL] = {
	{KEY_1,   KEY_9,           KEY_A,   KEY_I,   KEY_Q,   KEY_Y,       KEY_UP,          KEY_COMMA},
	{KEY_2,   KEY_0,           KEY_B,   KEY_J,   KEY_R,   KEY_Z,       KEY_LEFT,        KEY_DOT},
	{KEY_3,   KEY_BACK,        KEY_C,   KEY_K,   KEY_S,   KEY_Z,       KEY_RIGHT,       KEY_SPACE},
	{KEY_4,   KEY_SEARCH,      KEY_D,   KEY_L,   KEY_T,   KEY_SEND,    KEY_CAMERA,      KEY_F23},
	{KEY_5,   KEY_HOME,        KEY_E,   KEY_M,   KEY_U,   KEY_END,     KEY_LEFTALT,     KEY_F24},
	{KEY_6,   KEY_MENU,        KEY_F,   KEY_N,   KEY_V,   KEY_ENTER,   KEY_RIGHTALT,    KEY_NUMERIC_STAR},
	{KEY_7,   KEY_VOLUMEUP,    KEY_G,   KEY_O,   KEY_W,   KEY_DELETE,  KEY_LEFTSHIFT,   KEY_NUMERIC_POUND},
	{KEY_8,   KEY_VOLUMEDOWN,  KEY_H,   KEY_P,   KEY_X,   KEY_DOWN,    KEY_RIGHTSHIFT,  KEY_UNKNOWN},

};

static struct atcmd_virtual_platform_data atcmd_virtual_pdata = {
	.keypad_row = ATCMD_VIRTUAL_KEYPAD_ROW,
	.keypad_col = ATCMD_VIRTUAL_KEYPAD_COL,
	.keycode = (unsigned int *)atcmd_virtual_keycode,
};

static struct platform_device atcmd_virtual_device = {
	.name = "atcmd_virtual_kbd",
	.id = -1,
	.dev = {
		.platform_data = &atcmd_virtual_pdata,
	},
};


/* head set device */
static struct msm_handset_platform_data hs_platform_data = {
	.hs_name = "7k_handset",
	.pwr_key_delay_ms = 500, /* 0 will disable end key */
};

static struct platform_device hs_device = {
	.name   = "msm-handset",
	.id     = -1,
	.dev    = {
		.platform_data = &hs_platform_data,
	},
};

static int main_ts_set_vreg(unsigned char onoff)
{
	struct vreg *vreg_touch_avdd; //, *vreg_touch_vdd;
	int rc;

	printk("[MainTouch] %s() onoff:%d\n",__FUNCTION__, onoff);

	vreg_touch_avdd = vreg_get(0, "wlan");
	//vreg_touch_vdd = vreg_get(0, "wlan2");

	if(IS_ERR(vreg_touch_avdd)/* || IS_ERR(vreg_touch_vdd)*/) {
		printk("[MainTouch] vreg_get fail : touch\n");
		return -1;
	}

	if (onoff) {
		rc = vreg_set_level(vreg_touch_avdd, 2850);
		if (rc != 0) {
			printk("[MainTouch] avdd vreg_set_level failed\n");
			return -1;
		}

		rc = vreg_enable(vreg_touch_avdd);
		if (rc != 0) {
			printk("[MainTouch] avdd vreg_enable failed\n");
		}
#ifndef CONFIG_MACH_MSM8X55_FLIP

		rc = vreg_set_level(vreg_touch_vdd, 1800);
		if (rc != 0) {
			printk("[MainTouch] vdd vreg_set_level failed\n");
			return -1;
		}

		rc = vreg_enable(vreg_touch_vdd);

		if (rc != 0) {
			printk("[MainTouch] vdd vreg_enable failed\n");
		}
#endif		
	} else {
		rc = vreg_disable(vreg_touch_avdd);
		if (rc != 0) {
			printk("[MainTouch] avdd vreg_disable failed\n");
		}

#ifndef CONFIG_MACH_MSM8X55_FLIP
		rc = vreg_disable(vreg_touch_vdd);
		if (rc != 0) {
			printk("[MainTouch] vdd vreg_disable failed\n");
		}
#endif

	}

	return 0;
}
/* LGE_UPDATE_E 20110215 kideok.kim@lge.com Univa-Q Board bring-up */

#ifdef CONFIG_TOUCHSCREEN_QT602240
static struct qt602240_platform_data main_ts_pdata = {
	.x_line		= 19,
	.y_line		= 11,
	.x_size		= 800,
	.y_size		= 480,
	.blen		= 32,
	.threshold	= 40,
	.voltage	= 2700000,
	.orient		= 7,
	.power		= main_ts_set_vreg,
	.gpio_int	= MAIN_TS_GPIO_INT,
	.irq		= MAIN_TS_GPIO_IRQ,
	.scl		= MAIN_TS_GPIO_I2C_SCL,
	.sda		= MAIN_TS_GPIO_I2C_SDA,
};
#else   // UNIVA_Q uses here
static struct touch_platform_data main_ts_pdata = {
	.ts_x_min   = MAIN_TS_X_MIN,
	.ts_x_max   = MAIN_TS_X_MAX,
	.ts_x_scrn_max = MAIN_TS_X_SCRN_MAX,
	.ts_y_min   = MAIN_TS_Y_MIN,
	.ts_y_max   = MAIN_TS_Y_MAX,
    .ts_y_start = MAIN_TS_Y_START,
	.ts_y_scrn_max = MAIN_TS_Y_SCRN_MAX,
	.power      = main_ts_set_vreg,
	.gpio_int   = MAIN_TS_GPIO_INT,
	.irq        = MAIN_TS_GPIO_IRQ,
	.scl        = MAIN_TS_GPIO_I2C_SCL,
	.sda        = MAIN_TS_GPIO_I2C_SDA,
 	.hw_i2c     = 1,
    .ce         = MAIN_TS_GPIO_CE,
};
#endif

#ifndef CONFIG_MACH_MSM8X55_FLIP

static struct gpio_i2c_pin touch_panel_i2c_pin[] = {
	[0] = {
		.sda_pin    = MAIN_TS_GPIO_I2C_SDA,
		.scl_pin    = MAIN_TS_GPIO_I2C_SCL,
		.reset_pin  = MAIN_TS_GPIO_RESET,
		.irq_pin    = MAIN_TS_GPIO_INT,
	},
};


static struct i2c_gpio_platform_data touch_panel_i2c_pdata = {
	.sda_is_open_drain = 0,
	.scl_is_open_drain = 0,
	.udelay = 2,
};



static struct platform_device touch_panel_i2c_device = {
	.name = "i2c-gpio",
	.dev.platform_data = &touch_panel_i2c_pdata,
};
#endif


static struct i2c_board_info touch_panel_i2c_bdinfo[] = {
	[0] = {
		I2C_BOARD_INFO("qt602240_ts", MXT224_TS_I2C_SLAVE_ADDR),
		.type = "qt602240_ts",
		.platform_data = &main_ts_pdata,
		.irq = MAIN_TS_GPIO_IRQ,
	},
};
/*
 *  Sub Touch
 */

static int sub_ts_set_vreg(unsigned char onoff)
{
#if 0

	struct vreg *vreg_touch;
	int rc;

	printk("[SubTouch] %s() onoff:%d\n",__FUNCTION__, onoff);

	vreg_touch = vreg_get(0, "wlan");

	if(IS_ERR(vreg_touch)) {
		printk("[SubTouch] vreg_get fail : touch\n");
		return -1;
	}

	if (onoff) {
		rc = vreg_set_level(vreg_touch, 2800);
		if (rc != 0) {
			printk("[SubTouch] vreg_set_level failed\n");
			return -1;
		}
		vreg_enable(vreg_touch);
	} else
		vreg_disable(vreg_touch);
#endif
	return 0;
}

static struct gpio_i2c_pin sub_ts_i2c_pin[] = {
	[0] = {
		.sda_pin	= SUB_TS_GPIO_I2C_SDA,
		.scl_pin	= SUB_TS_GPIO_I2C_SCL,
		.reset_pin	= SUB_TS_GPIO_CE,
		.irq_pin	= SUB_TS_GPIO_INT,
	},
};

static struct i2c_gpio_platform_data sub_ts_i2c_pdata = {
	.sda_is_open_drain	= 0,
	.scl_is_open_drain	= 0,
	.udelay				= 2,
};

static struct platform_device sub_ts_i2c_device = {
	.name	= "i2c-gpio",
	.dev.platform_data = &sub_ts_i2c_pdata,
};


static struct touch_platform_data sub_ts_pdata = {
	.ts_x_min   = SUB_TS_X_MIN,
	.ts_x_max   = SUB_TS_X_MAX,
	.ts_x_scrn_max = SUB_TS_X_SCRN_MAX,
	.ts_y_min   = SUB_TS_Y_MIN,
	.ts_y_max   = SUB_TS_Y_MAX,
    .ts_y_start = SUB_TS_Y_START,
	.ts_y_scrn_max = SUB_TS_Y_SCRN_MAX,
	.power 	    = sub_ts_set_vreg,
	.gpio_int   = SUB_TS_GPIO_INT,
	.irq 	    = SUB_TS_GPIO_IRQ,
	.scl        = SUB_TS_GPIO_I2C_SCL,
	.sda        = SUB_TS_GPIO_I2C_SDA,
 	.hw_i2c     = 0,
	.ce			= SUB_TS_GPIO_CE,
};

static struct i2c_board_info sub_ts_i2c_bdinfo[] = {
	[0] = {
		I2C_BOARD_INFO("touch_mcs6000_sub", MCS6000_TS_I2C_SLAVE_ADDR),
		.type = "touch_mcs6000_sub",
		.platform_data = &sub_ts_pdata,
		.irq = SUB_TS_GPIO_IRQ,
	},
};

static void __init flip_init_i2c_sub_touch(int bus_num)
{	
	printk("[Touch] flip_init_i2c_sub_touch\n");

	sub_ts_i2c_device.id = bus_num;

	init_gpio_i2c_pin(&sub_ts_i2c_pdata, sub_ts_i2c_pin[0],	&sub_ts_i2c_bdinfo[0]);
	i2c_register_board_info(bus_num, &sub_ts_i2c_bdinfo[0], 1);
	platform_device_register(&sub_ts_i2c_device);
}


/* LGE_UPDATE_S 20110215 kideok.kim@lge.com Univa-Q Board bring-up */

/* Hall IC input */

static struct gpio_event_direct_entry flip_slide_switch_map[] = {
	{ 18,		   SW_LID		   }, 
};

static int flip_gpio_slide_power(
		const struct gpio_event_platform_data *pdata, bool on)
{
	return 0;
}

static int flip_gpio_slide_input_func(struct gpio_event_input_devs *input_devs,
		struct gpio_event_info *info, void **data, int func)
{

	if (func == GPIO_EVENT_FUNC_INIT)
		gpio_tlmm_config(GPIO_CFG(18, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP,
					GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	return gpio_event_input_func(input_devs, info, data, func);
}

static struct gpio_event_input_info flip_slide_switch_info = {
	.info.func = flip_gpio_slide_input_func,
	.debounce_time.tv64 = (80 * NSEC_PER_MSEC),
#if defined(LGE_MODEL_C729_REV_EVB) || defined(LGE_MODEL_C729_REV_A) || defined(LGE_MODEL_C729_REV_B)
	.flags = 0,
#else
	.flags = GPIOEDF_ACTIVE_HIGH,
#endif
	.type = EV_SW,
	.keymap = flip_slide_switch_map,
	.keymap_size = ARRAY_SIZE(flip_slide_switch_map)
};


static struct gpio_event_info *flip_gpio_slide_info[] = {
	&flip_slide_switch_info.info,
};

static struct gpio_event_platform_data flip_gpio_slide_data = {
	.name = "gpio-slide-detect",
	.info = flip_gpio_slide_info,
	.info_count = ARRAY_SIZE(flip_gpio_slide_info),
	.power = flip_gpio_slide_power,
};

static struct platform_device flip_gpio_slide_device = {
	.name = "gpio-event",
	.id = 0,
	.dev        = {
		.platform_data  = &flip_gpio_slide_data,
	},
};




/* Direct GPIO Side Keys */
//LGE_UPDATE_S kideok.kim@lge.com 20110309 Rev.A Adaptation

static struct gpio_keys_button flip_sidekey_buttons[] = {
	{	
		.code		= KEY_VOLUMEUP,
		.gpio		= 145,
		.active_low	= 1,
		.desc		= "volume_up_sidekey",
		.wakeup		= 1,
		.type       = EV_KEY,
	},
	{	
		.code		= KEY_VOLUMEDOWN,
		.gpio		= 147,
		.active_low	= 1,
		.desc		= "volume_down_sidekey",
		.wakeup		= 1,
		.type       = EV_KEY,
	},
};

static struct gpio_keys_platform_data flip_sidekey_button_data = {
	.buttons	= flip_sidekey_buttons,
	.nbuttons	= ARRAY_SIZE(flip_sidekey_buttons),
};


static struct platform_device flip_gpio_direct_sidekey_device = {
	.name = "gpio-sidekey-device",
	.id = -1,
	.dev		= {
		.platform_data =  &flip_sidekey_button_data,
	},
};


/* LGE_UPDATE_E 20110215 kideok.kim@lge.com Univa-Q Board bring-up */


/* accelerometer */
static int kr3dh_config_gpio(int config)
{
	int err = 0;

	/*GPIO setting*/
#if 0
    err = gpio_request(ACCEL_GPIO_INT, "kr3dh");
	if (err != 0)
		printk("GPIO_TOUCH_ATTN request failed.\n");

	gpio_direction_input(ACCEL_GPIO_INT);
	err = gpio_tlmm_config(GPIO_CFG(ACCEL_GPIO_INT, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
#endif
	if(config){	/* for wake state */
	}
	else{	/* for sleep state */
		gpio_tlmm_config(GPIO_CFG(ACCEL_GPIO_INT, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	}
		
	return err;
}

static int accel_power_set(unsigned char enable)
{
	int err = 0;
	int hw_rev_num = lge_get_hw_rev();
	struct vreg *vreg_power_1;
	struct vreg *vreg_power_2;

	printk("### accel_power_set enable : %d\n", enable);

	if(hw_rev_num < LGE_REV_11)
		vreg_power_1 = vreg_get(0, VREG_PERI_26V);
	else if(hw_rev_num >= LGE_REV_11)
		vreg_power_1 = vreg_get(0, VREG_PROXI_VDD_26V);
    vreg_power_2 = vreg_get(0, VREG_SENSOR_IO_18V);

	if (enable) {
		vreg_enable(vreg_power_1);
		err = vreg_set_level(vreg_power_1, 2600);
		if (err != 0) {
			printk("### accel_power_set vreg_power_1 failed.\n");
			return -1;
		}

		vreg_enable(vreg_power_2);
		err = vreg_set_level(vreg_power_2, 1800);
		if (err != 0) {
			printk("### accel_power_set vreg_power_2 failed.\n");
			return -1;
		}
		printk("### accel sensor power OK\n");
	}
	else {
		vreg_disable(vreg_power_1);
		vreg_disable(vreg_power_2);
	}

	return err;
}

static int kr_init(void){return 0;}
static void kr_exit(void){}
static int power_on(void){return accel_power_set(1);}
static int power_off(void){return accel_power_set(0);}

struct kr3dh_platform_data accel_pdata = {
	.poll_interval = 100,
	.min_interval = 0,
	.g_range = 0x00,
	.axis_map_x = 0,
	.axis_map_y = 1,
	.axis_map_z = 2,
	.negate_x = 0,
	.negate_y = 0,
	.negate_z = 0,
	.power_on = power_on,
	.power_off = power_off,
	.kr_init = kr_init,
	.kr_exit = kr_exit,
	.gpio_config = kr3dh_config_gpio,
};


/* ecompass */
static int ecom_power_set(unsigned char enable)
{
	int err = 0;
	int hw_rev_num = lge_get_hw_rev();
	struct vreg *vreg_power_1;
	struct vreg *vreg_power_2;

	printk("### ecom_power_set enable : %d\n", enable);

	
	if(hw_rev_num < LGE_REV_11)
		vreg_power_1 = vreg_get(0, VREG_PERI_26V);
	else if(hw_rev_num >= LGE_REV_11)
		vreg_power_1 = vreg_get(0, VREG_PROXI_VDD_26V);
    vreg_power_2 = vreg_get(0, VREG_SENSOR_IO_18V);

	if (enable) {
		vreg_enable(vreg_power_1);
		err = vreg_set_level(vreg_power_1, 2600);
		if (err != 0) {
			printk("### vreg_power_1 failed.\n");
			return -1;
		}

		vreg_enable(vreg_power_2);
		err = vreg_set_level(vreg_power_2, 1800);
		if (err != 0) {
			printk("### vreg_power_2 failed.\n");
			return -1;
		}
		printk("### sensor power OK\n");
	}
	else {
		vreg_disable(vreg_power_1);
		vreg_disable(vreg_power_2);
	}

	return err;
}

static struct akm8975_platform_data ecom8975_pdata = {
	//.gpio_DRDY		= ECOM_GPIO_DRDY,
	.power          = ecom_power_set, // TODO : power off implement
	.accelerator_name = "kr3dh",
	.fdata_sign_x = 1,
	.fdata_sign_y = -1,
	.fdata_sign_z = 1,
	.fdata_order0 = 1,
	.fdata_order1 = 0,
	.fdata_order2 = 2,
	.sensitivity1g = 1024,
};


/* proximity */
static int apds_power_set(unsigned char enable)
{
	int err = 0;
	int hw_rev_num = lge_get_hw_rev();
	struct vreg *vreg_power_1;

	printk("### apds_power_set enable : %d\n", enable);

	
	if(hw_rev_num < LGE_REV_11)
		vreg_power_1 = vreg_get(0, VREG_PROXI_VDD_26V);
	else if(hw_rev_num >= LGE_REV_11)
		vreg_power_1 = vreg_get(0, VREG_PERI_26V);

	if (enable) {
		vreg_enable(vreg_power_1);

		// LGE_UPDATE_S... gooni.shim@lge.com 13-Apr-2011
		// Change Vreg_power value for Proximtiy Sensor Tunning.(2.6V --> 2.8V)
		// gooni.shim@lge.com 30-JUN-2011 Change Vreg_power value for Proximtiy Sensor Tunning.(2.8V --> 3.0V)
		err = vreg_set_level(vreg_power_1, 3000/*2800*/);
		// LGE_UPDATE_E... 

		if (err != 0) {
			printk("### vreg_power_1 failed.\n");
			return -1;
		}

		printk("### adps sensor power OK\n");
	}
	else {
		vreg_disable(vreg_power_1);
	}

	return err; 
}

static struct apds9900_platform_data apds9900_pdata = {
	.power = apds_power_set,
	.irq_num= PROXI_GPIO_DOUT,
};

/* sensor i2c board info */
static struct i2c_board_info ecom_accel_proxi_i2c_bdinfo[] = {
	[0] = {
		I2C_BOARD_INFO("akm8975", ECOM_I2C_ADDRESS),
		.platform_data = &ecom8975_pdata,		
	},
	[1] = {
		I2C_BOARD_INFO("kr3dh", ACCEL_I2C_ADDRESS),
		.platform_data = &accel_pdata,
	},
	[2] = {
		I2C_BOARD_INFO("apds9900", PROXI_I2C_ADDRESS),
		.platform_data = &apds9900_pdata,
	},
};

static struct gpio_i2c_pin ecom_accel_proxi_i2c_pin[] = {
	[0] = {
		.sda_pin	= ECOM_GPIO_I2C_SDA,
		.scl_pin	= ECOM_GPIO_I2C_SCL,
		.reset_pin	= 0,
		.irq_pin	= 0,
	},
	[1] = {
		.sda_pin	= ACCEL_GPIO_I2C_SDA,
		.scl_pin	= ACCEL_GPIO_I2C_SCL,
		.reset_pin	= 0,
		.irq_pin	= ACCEL_GPIO_INT,//0,
	},
	[2] = {
		.sda_pin	= PROXI_GPIO_I2C_SDA,
		.scl_pin	= PROXI_GPIO_I2C_SCL,
		.reset_pin	= 0,
		.irq_pin	= PROXI_GPIO_DOUT,//0,
	},
};

static struct i2c_gpio_platform_data ecom_accel_proxi_i2c_pdata = {
	.sda_pin			= SENSORS_GPIO_I2C_SDA,
	.scl_pin			= SENSORS_GPIO_I2C_SCL,
	.sda_is_open_drain	= 0,
	.scl_is_open_drain	= 0,
	.udelay = 2,
};

static struct platform_device ecom_accel_proxi_i2c_device = {
        .name 	= "i2c-gpio",
        .dev.platform_data = &ecom_accel_proxi_i2c_pdata,
};

static void __init victor_init_i2c_ecom_accel_proxi(int bus_num)
{
	ecom_accel_proxi_i2c_device.id = 15;

	//init_gpio_i2c_pin(&ecom_accel_proxi_i2c_pdata, ecom_accel_proxi_i2c_pin[0], &ecom_accel_proxi_i2c_bdinfo[1]);
	init_gpio_i2c_pin(&ecom_accel_proxi_i2c_pdata, ecom_accel_proxi_i2c_pin[0], &ecom_accel_proxi_i2c_bdinfo[0]);
	init_gpio_i2c_pin(&ecom_accel_proxi_i2c_pdata, ecom_accel_proxi_i2c_pin[1], &ecom_accel_proxi_i2c_bdinfo[1]);
	init_gpio_i2c_pin(&ecom_accel_proxi_i2c_pdata, ecom_accel_proxi_i2c_pin[2], &ecom_accel_proxi_i2c_bdinfo[2]);
	
	//i2c_register_board_info(15, ecom_accel_proxi_i2c_bdinfo, ARRAY_SIZE(ecom_accel_proxi_i2c_bdinfo));
	i2c_register_board_info(15, &ecom_accel_proxi_i2c_bdinfo[0], 3/*ARRAY_SIZE(ecom_accel_proxi_i2c_bdinfo)*/);
	
	platform_device_register(&ecom_accel_proxi_i2c_device);
}

static void __init touch_panel(int bus_num)
{
	i2c_register_board_info(bus_num, &touch_panel_i2c_bdinfo[0], 1);
}

/* input platform device */
static struct platform_device *flip_input_devices[] __initdata = {
	&hs_device,
	&flip_gpio_slide_device,
	&flip_gpio_direct_sidekey_device,
	&atcmd_virtual_device,
};

/* common function */
void __init lge_add_input_devices(void)
{
    platform_add_devices(flip_input_devices, ARRAY_SIZE(flip_input_devices));

	touch_panel(0);
	lge_add_gpio_i2c_device(flip_init_i2c_sub_touch); // GPIO_I2C

	lge_add_gpio_i2c_device(victor_init_i2c_ecom_accel_proxi);
}
