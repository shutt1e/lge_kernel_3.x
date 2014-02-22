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
#ifdef CONFIG_MSM_SSBI
#include <linux/msm_ssbi.h>
#endif
#include <linux/mfd/pmic8058.h>
#include <linux/leds-pmic8058.h>
#include <linux/regulator/consumer.h>
#include <asm/mach-types.h>
#include "../pm.h"
#include "../pm-boot.h"
#include "board-victor.h"
#include "devices.h"

struct pm8xxx_gpio_init_info {
	unsigned			gpio;
	struct pm_gpio			config;
};

/* static int pm8058_gpios_init(void) */
int pm8058_gpios_init(void)
{
/* LGE_UPDATE_S 20120201 johny.kim@lge.com not used in victor */
#if 0
	int rc;
	struct pm8xxx_gpio_init_info sdc4_en = {
		PM8058_GPIO_PM_TO_SYS(PMIC_GPIO_SDC4_EN_N),
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
		/* SCD4 gpio */
		rc = pm8xxx_gpio_config(sdc4_en.gpio, &sdc4_en.config);
		if (rc) {
			pr_err("%s PMIC_GPIO_SDC4_EN_N config failed\n",
								 __func__);
			return rc;
		}
		rc = gpio_request(sdc4_en.gpio, "sdc4_en");
		if (rc) {
			pr_err("%s PMIC_GPIO_SDC4_EN_N gpio_request failed\n",
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
#endif
/* LGE_UPDATE_E 20120201 johny.kim@lge.com not used in victor */
	return 0;
}

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

static const unsigned int victor_keymap[] = {
	KEY(1, 1, KEY_VOLUMEUP),
	KEY(1, 2, KEY_VOLUMEDOWN),
};

static struct matrix_keymap_data victor_keymap_data = {
	.keymap_size   = ARRAY_SIZE(victor_keymap),
	.keymap    = victor_keymap,
};

static struct pm8xxx_keypad_platform_data victor_keypad_data = {
	.input_name		= "victor-keypad",
	.input_phys_device	= "victor-keypad/input0",
	.num_rows		= 5,  //1,
	.num_cols		= 5,  //2,
	.rows_gpio_start	= PM8058_GPIO_PM_TO_SYS(8),
	.cols_gpio_start	= PM8058_GPIO_PM_TO_SYS(0),
	.debounce_ms		= 15,
	.scan_delay_ms		= 32,
	.row_hold_ns		= 91500,
	.wakeup			= 1,
	.rep         = 0,
	.keymap_data		= &victor_keymap_data,
};

static struct pm8058_pwm_pdata pm8058_pwm_data = {
	.config		= pm8058_pwm_config,
	.enable		= pm8058_pwm_enable,
};

static struct pmic8058_led pmic8058_victor_leds[] = {
	[0] = {
		.name		= "button-backlight",
		.max_brightness = 20,
		.id		= PMIC8058_ID_LED_0,
	},
};

static struct pmic8058_leds_platform_data pm8058_victor_leds_data = {
	.num_leds = ARRAY_SIZE(pmic8058_victor_leds),
	.leds	= pmic8058_victor_leds,
};

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

int __init pmic8058_buses_init(void)
{
	if (gpio_tlmm_config(GPIO_CFG(PMIC_GPIO_INT, 1, GPIO_CFG_INPUT,
				  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE))
		pr_err("%s: gpio_tlmm_config (gpio=%d) failed\n",
		       __func__, PMIC_GPIO_INT);

	pm8058_7x30_data.keypad_pdata = &victor_keypad_data;

/* LGE_UPDATE_S kideok.kim@lge.com 20111216 kernel 3.0.x Adaptation */
#ifdef CONFIG_MSM_SSBI
	msm_device_ssbi_pmic1.dev.platform_data =
				&msm7x30_ssbi_pm8058_pdata;
#endif
/* LGE_UPDATE_E kideok.kim@lge.com 20111216 kernel 3.0.x Adaptation */
	return 0;
}

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

void __init lge_pm_set_platform_data(void)
{
	msm_pm_set_platform_data(msm_pm_data, ARRAY_SIZE(msm_pm_data));
	BUG_ON(msm_pm_boot_init(&msm_pm_boot_pdata));
}

void __init pmic8058_leds_init(void)
{
	pm8058_7x30_data.leds_pdata = &pm8058_victor_leds_data;
}
