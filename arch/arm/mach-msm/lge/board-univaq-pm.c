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
#include <linux/i2c.h>
#include <linux/mfd/pmic8058.h>
/* #include <linux/input/pmic8058-keypad.h> */
/* #include <linux/pwm.h> */
/* #include <linux/pmic8058-pwm.h> */
#include <linux/leds-pmic8058.h>
#include <linux/regulator/consumer.h>
#include <asm/mach-types.h>
#include "../pm.h"
#include "../pm-boot.h"
#include "board-univaq.h"
#include <linux/msm_ssbi.h> /* LGE_UPDATE kideok.kim@lge.com 20111216 kernel 3.0.x Adaptation */
#include "devices.h"  /* LGE_UPDATE kideok.kim@lge.com 20111216 kernel 3.0.x Adaptation */


/* LGE_UPDATE_S 20111216 kideok.kim@lge.com kernel 3.0.x Adaptation */

struct pm8xxx_gpio_init_info {
	unsigned			gpio;
	struct pm_gpio			config;
};

/* static int pm8058_gpios_init(void) */
int pm8058_gpios_init(void)
{
	int rc;
	struct pm8xxx_gpio_init_info sdc4_en = {
		PM8058_GPIO_PM_TO_SYS(PMIC_GPIO_SDC4_EN_N),
		{
			.direction      = PM_GPIO_DIR_OUT,
			.pull           = PM_GPIO_PULL_UP_1P5,
			.vin_sel        = 2,
			.function       = PM_GPIO_FUNC_NORMAL,
			.inv_int_pol    = 0,
			.out_strength   = PM_GPIO_STRENGTH_LOW,
			.output_value   = 0,
		},
	};

	struct pm8xxx_gpio_init_info sdc4_pwr_en = {
		PM8058_GPIO_PM_TO_SYS(PMIC_GPIO_SDC4_PWR_EN_N),
		{
			.direction      = PM_GPIO_DIR_OUT,
			.pull           = PM_GPIO_PULL_NO,
			.vin_sel        = PM8058_GPIO_VIN_L5,
			.function       = PM_GPIO_FUNC_NORMAL,
			.inv_int_pol    = 0,
			.out_strength   = PM_GPIO_STRENGTH_LOW,
			.output_value   = 0,
		},
	};

#ifdef CONFIG_MMC_MSM_CARD_HW_DETECTION
	struct pm8xxx_gpio_init_info sdcc_det = {
		PM8058_GPIO_PM_TO_SYS(PMIC_GPIO_SD_DET - 1),
		{
			.direction      = PM_GPIO_DIR_IN,
			.pull           = PM_GPIO_PULL_UP_1P5,
			.vin_sel        = 2,
			.function       = PM_GPIO_FUNC_NORMAL,
			.inv_int_pol    = 0,
		},
	};

	if (machine_is_msm7x30_fluid())
		sdcc_det.config.inv_int_pol = 1;

	rc = pm8xxx_gpio_config(sdcc_det.gpio, &sdcc_det.config);
	if (rc) {
		pr_err("%s PMIC_GPIO_SD_DET config failed\n", __func__);
		return rc;
	}
#endif

	if (machine_is_msm7x30_fluid()) {
		rc = pm8xxx_gpio_config(sdc4_en.gpio, &sdc4_en.config);
		if (rc) {
			pr_err("%s PMIC_GPIO_SDC4_EN_N config failed\n",
								 __func__);
			return rc;
		}
		gpio_set_value_cansleep(sdc4_en.gpio, 0);
	}
	/* FFA -> gpio_25 controls vdd of sdcc4 */
	else {
		/* SCD4 gpio_25 */
		rc = pm8xxx_gpio_config(sdc4_pwr_en.gpio, &sdc4_pwr_en.config);
		if (rc) {
			pr_err("%s PMIC_GPIO_SDC4_PWR_EN_N config failed: %d\n",
			       __func__, rc);
			return rc;
		}

		rc = gpio_request(sdc4_pwr_en.gpio, "sdc4_pwr_en");
		if (rc) {
			pr_err("PMIC_GPIO_SDC4_PWR_EN_N gpio_req failed: %d\n",
			       rc);
			return rc;
		}
	}

	return 0;
}
/* LGE_UPDATE_E 20111216 kideok.kim@lge.com kernel 3.0.x Adaptation */


static int pm8058_pwm_config(struct pwm_device *pwm, int ch, int on)
{
	struct pm_gpio pwm_gpio_config = {
		.direction      = PM_GPIO_DIR_OUT,
		.output_buffer  = PM_GPIO_OUT_BUF_CMOS,
		.output_value   = 0,
		.pull           = PM_GPIO_PULL_NO,
		.vin_sel        = PM8058_GPIO_VIN_S3,
		.out_strength   = PM_GPIO_STRENGTH_HIGH,
		.function       = PM_GPIO_FUNC_2,
	};
	int	rc = -EINVAL;
	int	id, mode, max_mA;

	id = mode = max_mA = 0;
	switch (ch) {
	case 0:
	case 1:
	case 2:
		if (on) {
			id = 24 + ch;
			rc = pm8xxx_gpio_config(PM8058_GPIO_PM_TO_SYS(id - 1),
							&pwm_gpio_config);
			if (rc)
				pr_err("%s: pm8xxx_gpio_config(%d): rc=%d\n",
				       __func__, id, rc);
		}
		break;

	case 3:
		id = PM_PWM_LED_KPD;
		mode = PM_PWM_CONF_DTEST3;
		max_mA = 200;
		break;

	case 4:
		id = PM_PWM_LED_0;
		mode = PM_PWM_CONF_PWM1;
		max_mA = 40;
		break;

	case 5:
		id = PM_PWM_LED_2;
		mode = PM_PWM_CONF_PWM2;
		max_mA = 40;
		break;

	case 6:
		id = PM_PWM_LED_FLASH;
		mode = PM_PWM_CONF_DTEST3;
		max_mA = 200;
		break;

	default:
		break;
	}

	if (ch >= 3 && ch <= 6) {
		if (!on) {
			mode = PM_PWM_CONF_NONE;
			max_mA = 0;
		}
		rc = pm8058_pwm_config_led(pwm, id, mode, max_mA);
		if (rc)
			pr_err("%s: pm8058_pwm_config_led(ch=%d): rc=%d\n",
			       __func__, ch, rc);
	}

	return rc;
}

static int pm8058_pwm_enable(struct pwm_device *pwm, int ch, int on)
{
	int	rc;

	switch (ch) {
	case 7:
		rc = pm8058_pwm_set_dtest(pwm, on);
		if (rc)
			pr_err("%s: pwm_set_dtest(%d): rc=%d\n",
			       __func__, on, rc);
		break;
	default:
		rc = -EINVAL;
		break;
	}
	return rc;
}

#ifdef CONFIG_MACH_MSM8X55_UNIVA_Q
/* LGE_UPDATE_S 20110215 kideok.kim@lge.com Univa-Q Board bring-up */
static const unsigned int univaq_pm_keymap[] = {
#if defined(LGE_MODEL_C800_REV_EVB)
/// KEY(DRV,SNS, keys) on PMIC Keypad.
	KEY(0, 0, KEY_OPTION),
	KEY(0, 1, KEY_Q),
	KEY(0, 2, KEY_W),
	KEY(0, 3, KEY_E),
	KEY(0, 4, KEY_R),
	KEY(0, 5, KEY_T),

	KEY(1, 0, KEY_Y),
	KEY(1, 1, KEY_U),
	KEY(1, 2, KEY_I),
	KEY(1, 3, KEY_O),
	KEY(1, 4, KEY_P),
	KEY(1, 5, KEY_DELETE),

	KEY(2, 0, KEY_LEFTCTRL),
	KEY(2, 1, KEY_A),
	KEY(2, 2, KEY_S),
	KEY(2, 3, KEY_D),
	KEY(2, 4, KEY_F),
	KEY(2, 5, KEY_G),

	KEY(3, 0, KEY_H),
	KEY(3, 1, KEY_J),
	KEY(3, 2, KEY_K),
	KEY(3, 3, KEY_L),
	KEY(3, 4, KEY_ENTER),
	KEY(3, 5, KEY_UNKNOWN),

	KEY(4, 0, KEY_CAPSLOCK),
	KEY(4, 1, KEY_Z),
	KEY(4, 2, KEY_X),
	KEY(4, 3, KEY_C),
	KEY(4, 4, KEY_V),
	KEY(4, 5, KEY_B),

	KEY(5, 0, KEY_N),
	KEY(5, 1, KEY_M),
	KEY(5, 2, KEY_QUESTION),
	KEY(5, 3, KEY_UP),
	KEY(5, 4, KEY_RIGHTSHIFT),
	KEY(5, 5, KEY_UNKNOWN),

	KEY(6, 0, KEY_LEFTALT),
	KEY(6, 1, KEY_COMMA),
	KEY(6, 2, KEY_DOT),
	KEY(6, 3, KEY_COMMA),    // Need to be chaned to SYM key
	KEY(6, 4, KEY_SPACE),
	KEY(6, 5, KEY_LANGUAGE),

	KEY(7, 0, KEY_MAIL),
	KEY(7, 1, KEY_LEFT),
	KEY(7, 2, KEY_DOWN),
	KEY(7, 3, KEY_RIGHT),
	KEY(7, 4, KEY_UNKNOWN),
	KEY(7, 5, KEY_UNKNOWN),
#elif 1 //defined(LGE_MODEL_C800_REV_A)
/// KEY(DRV,SNS, keys) on PMIC Keypad.	// for rev.A

	KEY(0, 0, KEY_MENU), // ok
	KEY(0, 1, KEY_Q), // ok
	KEY(0, 2, KEY_W), // ok
	KEY(0, 3, KEY_E), // ok
	KEY(0, 4, KEY_UNKNOWN),
	KEY(0, 5, KEY_UNKNOWN),

	KEY(1, 0, KEY_R), // ok
	KEY(1, 1, KEY_T), // ok
	KEY(1, 2, KEY_Y), // ok
	KEY(1, 3, KEY_U), // ok 	// Need to be chaned to SYM key
	KEY(1, 4, KEY_UNKNOWN),
	KEY(1, 5, KEY_UNKNOWN),

	KEY(2, 0, KEY_I),  // ok
	KEY(2, 1, KEY_O),  // ok
	KEY(2, 2, KEY_P),  // ok
	KEY(2, 3, KEY_BACKSPACE), // ok
	KEY(2, 4, KEY_MAIL),  // ok
	KEY(2, 5, KEY_A),	// ok

	KEY(3, 0, KEY_S),  // ok
	KEY(3, 1, KEY_D),  // ok
	KEY(3, 2, KEY_F),  // ok
	KEY(3, 3, KEY_G),  // SYM
	KEY(3, 4, KEY_H), // ok
	KEY(3, 5, KEY_J), // ok

	KEY(4, 0, KEY_K), // ok
	KEY(4, 1, KEY_L), // ok
	KEY(4, 2, KEY_ENTER), //ok
	KEY(4, 3, KEY_CAPSLOCK), // nok
	KEY(4, 4, KEY_Z),	// ok
	KEY(4, 5, KEY_X),	// ok

	KEY(5, 0, KEY_C), // ok
	KEY(5, 1, KEY_V), //ok
	KEY(5, 2, KEY_B), // ok
	KEY(5, 3, KEY_N), // ok
	KEY(5, 4, KEY_M), //ok
	KEY(5, 5, KEY_QUESTION), //ok

	KEY(6, 0, KEY_RIGHT),	// ok
	KEY(6, 1, KEY_RIGHTSHIFT), // ok
	KEY(6, 2, KEY_LEFTALT),
	KEY(6, 3, KEY_COMMA),	 // 
	KEY(6, 4, KEY_DOT),  // ok
	KEY(6, 5, KEY_F15),  // SYM

	KEY(7, 0, KEY_SPACE), // ok
	KEY(7, 1, KEY_LANGUAGE),  // ok
	KEY(7, 2, KEY_F14), //ok
	KEY(7, 3, KEY_UP),  // ok
	KEY(7, 4, KEY_LEFT), //ok
	KEY(7, 5, KEY_DOWN), //ok

#else
 /// KEY(DRV,SNS, keys) on PMIC Keypad.  // for rev.A
 
	 KEY(0, 0, KEY_MENU), // ok
	 KEY(0, 1, KEY_Q), // ok
	 KEY(0, 2, KEY_W), // ok
	 KEY(0, 3, KEY_E), // ok
	 KEY(0, 4, KEY_UNKNOWN),
	 KEY(0, 5, KEY_UNKNOWN),
 
	 KEY(1, 0, KEY_R), // ok
	 KEY(1, 1, KEY_T), // ok
	 KEY(1, 2, KEY_Y), // ok
	 KEY(1, 3, KEY_U), // ok	 // Need to be chaned to SYM key
	 KEY(1, 4, KEY_UNKNOWN),
	 KEY(1, 5, KEY_UNKNOWN),
 
	 KEY(2, 0, KEY_I),  // ok
	 KEY(2, 1, KEY_O),  // ok
	 KEY(2, 2, KEY_P),  // ok
	 KEY(2, 3, KEY_SEARCH), // ok
	 KEY(2, 4, KEY_MAIL),  // ok
	 KEY(2, 5, KEY_A),   // ok
 
	 KEY(3, 0, KEY_S),  // ok
	 KEY(3, 1, KEY_D),  // ok
	 KEY(3, 2, KEY_F),  // ok
	 KEY(3, 3, KEY_G),  // SYM
	 KEY(3, 4, KEY_H), // ok
	 KEY(3, 5, KEY_J), // ok
 
	 KEY(4, 0, KEY_K), // ok
	 KEY(4, 1, KEY_L), // ok
	 KEY(4, 2, KEY_BACKSPACE), //ok
	 KEY(4, 3, KEY_LEFTSHIFT), // nok
	 KEY(4, 4, KEY_Z),   // ok
	 KEY(4, 5, KEY_X),   // ok
 
	 KEY(5, 0, KEY_C), // ok
	 KEY(5, 1, KEY_V), //ok
	 KEY(5, 2, KEY_B), // ok
	 KEY(5, 3, KEY_N), // ok
	 KEY(5, 4, KEY_M), //ok
	 KEY(5, 5, KEY_APOSTROPHE), //ok
 
	 KEY(6, 0, KEY_RIGHT),  // ok
	 KEY(6, 1, KEY_ENTER), // ok
	 KEY(6, 2, KEY_LEFTALT),
	 KEY(6, 3, KEY_F13),	  // Need to be chaned to SYM key
	 KEY(6, 4, KEY_WWW),  // ok
	 KEY(6, 5, KEY_COMMA),
 
	 KEY(7, 0, KEY_SPACE), // ok
	 KEY(7, 1, KEY_DOT),  // ok
	 KEY(7, 2, KEY_F14), //ok
	 KEY(7, 3, KEY_UP),  // ok
	 KEY(7, 4, KEY_LEFT), //ok
	 KEY(7, 5, KEY_DOWN), //ok
 
#endif

};
/* LGE_UPDATE_E 20110215 kideok.kim@lge.com Univa-Q Board bring-up */

#else
static const unsigned int victor_keymap[] = {
	KEY(1, 1, KEY_VOLUMEUP),
	KEY(1, 2, KEY_VOLUMEDOWN),
};
#endif

/* LGE_UPDATE_S kideok.kim@lge.com 20111216 kernel 3.0.x Adaptation */
#if 0
static struct resource resources_keypad[] = {
	{
		.start	= PM8058_KEYPAD_IRQ(PMIC8058_IRQ_BASE),
		.end	= PM8058_KEYPAD_IRQ(PMIC8058_IRQ_BASE),
		.flags	= IORESOURCE_IRQ,
	},
	{
		.start	= PM8058_KEYSTUCK_IRQ(PMIC8058_IRQ_BASE),
		.end	= PM8058_KEYSTUCK_IRQ(PMIC8058_IRQ_BASE),
		.flags	= IORESOURCE_IRQ,
	},
};
#endif

static struct matrix_keymap_data univaq_keymap_data = {
	.keymap_size   = ARRAY_SIZE(univaq_pm_keymap),
	.keymap    = univaq_pm_keymap,
};

static struct pm8xxx_keypad_platform_data univaq_keypad_data = {
	.input_name		= "univa_q-keypad",
	.input_phys_device	= "univa_q-keypad/input0",
	.num_rows		= 8,
	.num_cols		= 6,
	.rows_gpio_start	= PM8058_GPIO_PM_TO_SYS(8),
	.cols_gpio_start	= PM8058_GPIO_PM_TO_SYS(0),
	.debounce_ms		= 15,
	.scan_delay_ms		= 32,
	.row_hold_ns		= 91500,
	.wakeup			= 1,
	.keymap_data		= &univaq_keymap_data,
};
/* LGE_UPDATE_E kideok.kim@lge.com 20111216 kernel 3.0.x Adaptation */

static struct pm8058_pwm_pdata pm8058_pwm_data = {
	.config		= pm8058_pwm_config,
	.enable		= pm8058_pwm_enable,
};

/* LGE_UPDATE_S kideok.kim@lge.com 20111216 kernel 3.0.x Adaptation */
#if 0
/* Put sub devices with fixed location first in sub_devices array */
#define	PM8058_SUBDEV_KPD	0
#define	PM8058_SUBDEV_LED	1

static struct pm8058_gpio_platform_data pm8058_gpio_data = {
	.gpio_base	= PM8058_GPIO_PM_TO_SYS(0),
	.irq_base	= PM8058_GPIO_IRQ(PMIC8058_IRQ_BASE, 0),
	.init		= pm8058_gpios_init,
};

static struct pm8058_gpio_platform_data pm8058_mpp_data = {
	.gpio_base	= PM8058_GPIO_PM_TO_SYS(PM8058_GPIOS),
	.irq_base	= PM8058_MPP_IRQ(PMIC8058_IRQ_BASE, 0),
};
#endif
/* LGE_UPDATE_E kideok.kim@lge.com 20111216 kernel 3.0.x Adaptation */


/* LGE_UPDATE_S 20110215 kideok.kim@lge.com Univa-Q Board bring-up */
#ifdef CONFIG_MACH_MSM8X55_UNIVA_Q
static struct pmic8058_led pmic8058_univaq_leds[] = {
	[0] = {
		.name		= "keyboard-backlight",   // kideok.kim@lge.com Univaq adaptation
		.max_brightness = 6,
		.id		= PMIC8058_ID_LED_KB_LIGHT,
	},
	[1] = {
		.name		= "button-backlight",  // kideok.kim@lge.com Univaq adaptation
		.max_brightness = 6,
		.id		= PMIC8058_ID_LED_0,
	},
};

static struct pmic8058_leds_platform_data pm8058_univaq_leds_data = {
	.num_leds = ARRAY_SIZE(pmic8058_univaq_leds),
	.leds	= pmic8058_univaq_leds,
};
#endif


/* LGE_UPDATE_S kideok.kim@lge.com 20111216 kernel 3.0.x Adaptation */
#if 0

/* LGE_UPDATE_E 20110215 kideok.kim@lge.com Univa-Q Board bring-up */
static struct mfd_cell pm8058_subdevs[] = {
	{	.name = "pm8058-keypad",
		.id		= -1,
		.num_resources	= ARRAY_SIZE(resources_keypad),
		.resources	= resources_keypad,
	},
/* LGE_UPDATE_S 20110215 kideok.kim@lge.com Univa-Q Board bring-up */
#ifdef CONFIG_MACH_MSM8X55_UNIVA_Q
	{	.name = "pm8058-led",
		.id		= -1,
	},
#endif
/* LGE_UPDATE_E 20110215 kideok.kim@lge.com Univa-Q Board bring-up */
	{	.name = "pm8058-gpio",
		.id		= -1,
		.platform_data	= &pm8058_gpio_data,
		.data_size	= sizeof(pm8058_gpio_data),
	},
	{	.name = "pm8058-mpp",
		.id		= -1,
		.platform_data	= &pm8058_mpp_data,
		.data_size	= sizeof(pm8058_mpp_data),
	},
	{	.name = "pm8058-pwm",
		.id		= -1,
		.platform_data	= &pm8058_pwm_data,
		.data_size	= sizeof(pm8058_pwm_data),
	},
};
#endif
/* LGE_UPDATE_E kideok.kim@lge.com 20111216 kernel 3.0.x Adaptation */

/* LGE_UPDATE_S kideok.kim@lge.com 20111216 kernel 3.0.x Adaptation */
#if 0
static struct pm8058_platform_data pm8058_7x30_data = {
	.irq_base = PMIC8058_IRQ_BASE,

	.num_subdevs = ARRAY_SIZE(pm8058_subdevs),
	.sub_devices = pm8058_subdevs,
	.irq_trigger_flags = IRQF_TRIGGER_LOW,
};

static struct i2c_board_info pm8058_boardinfo[] __initdata = {
	{
		I2C_BOARD_INFO("pm8058-core", 0x55),
		.irq = MSM_GPIO_TO_INT(PMIC_GPIO_INT),
		.platform_data = &pm8058_7x30_data,
	},
};
#else

static struct pm8xxx_irq_platform_data pm8xxx_irq_pdata = {
	.irq_base		= PMIC8058_IRQ_BASE,
	.devirq			= MSM_GPIO_TO_INT(PMIC_GPIO_INT),
	.irq_trigger_flag       = IRQF_TRIGGER_LOW,
};

static struct pm8xxx_gpio_platform_data pm8xxx_gpio_pdata = {
	.gpio_base		= PM8058_GPIO_PM_TO_SYS(0),
};

static struct pm8xxx_mpp_platform_data pm8xxx_mpp_pdata = {
	.mpp_base	= PM8058_MPP_PM_TO_SYS(0),
};

static struct pm8058_platform_data pm8058_7x30_data = {
	.irq_pdata		= &pm8xxx_irq_pdata,
	.gpio_pdata		= &pm8xxx_gpio_pdata,
	.mpp_pdata		= &pm8xxx_mpp_pdata,
	.pwm_pdata		= &pm8058_pwm_data,
};

#ifdef CONFIG_MSM_SSBI
static struct msm_ssbi_platform_data msm7x30_ssbi_pm8058_pdata = {
	.rsl_id = "D:PMIC_SSBI",
	.controller_type = MSM_SBI_CTRL_SSBI2,
	.slave	= {
		.name			= "pm8058-core",
		.platform_data		= &pm8058_7x30_data,
	},
};
#endif


#endif
/* LGE_UPDATE_E kideok.kim@lge.com 20111216 kernel 3.0.x Adaptation */



int __init pmic8058_buses_init(void)
{
	if (gpio_tlmm_config(GPIO_CFG(PMIC_GPIO_INT, 1, GPIO_CFG_INPUT,
				  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE))
		pr_err("%s: gpio_tlmm_config (gpio=%d) failed\n",
		       __func__, PMIC_GPIO_INT);

/* LGE_UPDATE_S kideok.kim@lge.com 20111216 kernel 3.0.x Adaptation */
#if 0
	pm8058_7x30_data.sub_devices[PM8058_SUBDEV_KPD].platform_data
		= &univaq_keypad_data;
	pm8058_7x30_data.sub_devices[PM8058_SUBDEV_KPD].data_size
		= sizeof(univaq_keypad_data);

	i2c_register_board_info(6 /* I2C_SSBI ID */, pm8058_boardinfo,
				ARRAY_SIZE(pm8058_boardinfo));
#else

	pm8058_7x30_data.keypad_pdata = &univaq_keypad_data;

#ifdef CONFIG_MSM_SSBI
	msm_device_ssbi_pmic1.dev.platform_data =
				&msm7x30_ssbi_pm8058_pdata;
#endif


#endif
/* LGE_UPDATE_E kideok.kim@lge.com 20111216 kernel 3.0.x Adaptation */

	return 0;
}

/* LGE_UPDATE_S kideok.kim@lge.com 20111216 kernel 3.0.x Adaptation */
static struct msm_pm_platform_data msm_pm_data[MSM_PM_SLEEP_MODE_NR] = {
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE] = {
		.idle_supported = 1,
		.suspend_supported = 1,
		.idle_enabled = 1,
		.suspend_enabled = 1,
		.latency = 8594,
		.residency = 23740,
	},
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_NO_XO_SHUTDOWN] = {
		.idle_supported = 1,
		.suspend_supported = 1,
		.idle_enabled = 1,
		.suspend_enabled = 1,
		.latency = 4594,
		.residency = 23740,
	},
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_STANDALONE] = {
#ifdef CONFIG_MSM_STANDALONE_POWER_COLLAPSE
		.idle_supported = 1,
		.suspend_supported = 1,
		.idle_enabled = 1,
		.suspend_enabled = 0,
#else /*CONFIG_MSM_STANDALONE_POWER_COLLAPSE*/
		.idle_supported = 0,
		.suspend_supported = 0,
		.idle_enabled = 0,
		.suspend_enabled = 0,
#endif /*CONFIG_MSM_STANDALONE_POWER_COLLAPSE*/
		.latency = 500,
		.residency = 6000,
	},
	[MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT] = {
		.idle_supported = 1,
		.suspend_supported = 1,
		.idle_enabled = 0,
		.suspend_enabled = 1,
		.latency = 443,
		.residency = 1098,
	},
	[MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT] = {
		.idle_supported = 1,
		.suspend_supported = 1,
		.idle_enabled = 1,
		.suspend_enabled = 1,
		.latency = 2,
		.residency = 0,
	},
};

u32 msm7x30_power_collapse_latency(enum msm_pm_sleep_mode mode)
{
	switch (mode) {
	case MSM_PM_SLEEP_MODE_POWER_COLLAPSE:
		return msm_pm_data
		[MSM_PM_SLEEP_MODE_POWER_COLLAPSE].latency;
	case MSM_PM_SLEEP_MODE_POWER_COLLAPSE_NO_XO_SHUTDOWN:
		return msm_pm_data
		[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_NO_XO_SHUTDOWN].latency;
	case MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT:
		return msm_pm_data
		[MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT].latency;
	case MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT:
		return msm_pm_data
		[MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT].latency;
	default:
	return 0;
	}
}

static struct msm_pm_boot_platform_data msm_pm_boot_pdata __initdata = {
	.mode = MSM_PM_BOOT_CONFIG_RESET_VECTOR_VIRT,
	.v_addr = (uint32_t *)PAGE_OFFSET,
};

/* LGE_UPDATE_E kideok.kim@lge.com 20111216 kernel 3.0.x Adaptation */


void __init lge_pm_set_platform_data(void)
{
	msm_pm_set_platform_data(msm_pm_data, ARRAY_SIZE(msm_pm_data));
	BUG_ON(msm_pm_boot_init(&msm_pm_boot_pdata));   /* Temporal */
}
/* LGE_UPDATE_S 20110215 kideok.kim@lge.com Univa-Q Board bring-up */
#ifdef CONFIG_MACH_MSM8X55_UNIVA_Q
void __init pmic8058_leds_init(void)
{

/* LGE_UPDATE_S kideok.kim@lge.com 20111216 kernel 3.0.x Adaptation */
#if 0
		pm8058_7x30_data.sub_devices[PM8058_SUBDEV_LED].platform_data
			= &pm8058_univaq_leds_data;
		pm8058_7x30_data.sub_devices[PM8058_SUBDEV_LED].data_size
			= sizeof(pm8058_univaq_leds_data);
#else
		pm8058_7x30_data.leds_pdata = &pm8058_univaq_leds_data;

#endif
/* LGE_UPDATE_E kideok.kim@lge.com 20111216 kernel 3.0.x Adaptation */

}
#endif
/* LGE_UPDATE_E 20110215 kideok.kim@lge.com Univa-Q Board bring-up */
