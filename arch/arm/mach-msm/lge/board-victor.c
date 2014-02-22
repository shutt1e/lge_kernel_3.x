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

#include <asm/mach-types.h>
#include <asm/mach/arch.h>

#include <mach/board.h>
#include <mach/dma.h>
#include <mach/msm_hsusb.h>
#include <mach/rpc_hsusb.h>
#ifdef CONFIG_USB_G_ANDROID
#include <linux/usb/android.h>
#include <mach/usbdiag.h>
#endif

#include <mach/socinfo.h>
#include <mach/board.h>
#include <mach/board_lge.h>

#include "devices.h"
#include "timer.h"
#include "spm.h"

#include "acpuclock.h"
#include <mach/msm_memtypes.h>
#include <linux/bootmem.h>
#include <linux/io.h>
#include <linux/android_pmem.h>

#include "board-victor.h"
#include "board-msm7x30-regulator.h"



/* Regulator API support */

#ifdef CONFIG_MSM_PROC_COMM_REGULATOR
static struct platform_device msm_proccomm_regulator_dev = {
	.name = PROCCOMM_REGULATOR_DEV_NAME,
	.id   = -1,
	.dev  = {
		.platform_data = &msm7x30_proccomm_regulator_data
	}
};
#endif

static struct platform_device *devices[] __initdata = {
#ifdef CONFIG_MSM_PROC_COMM_REGULATOR
	&msm_proccomm_regulator_dev,
#endif
	/* LGE_UPDATE_S */
	&asoc_msm_pcm,
	&asoc_msm_dai0,
	&asoc_msm_dai1,
		
#if defined (CONFIG_SND_MSM_MVS_DAI_SOC)
	&asoc_msm_mvs,
	&asoc_mvs_dai0,
	&asoc_mvs_dai1,
#endif
	/* LGE_UPDATE_E */

	&msm_device_smd,
	&msm_device_dmov,
#ifdef CONFIG_MSM_SSBI
	&msm_device_ssbi_pmic1,
#endif
#ifdef CONFIG_I2C_SSBI
	&msm_device_ssbi7,
#endif
//	&msm_device_i2c,
//	&msm_device_i2c_2,
	&msm_device_uart_dm1,
	&qup_device_i2c,
// BEGIN soojeong.hwang@lge.com Add device( rotator & 720P)
#ifdef CONFIG_MSM_ROTATOR
    &msm_rotator_device,
#endif
    &msm_device_vidc_720p,      // Video Codec
// END soojeong.hwang@lge.com Add device( rotator & 720P)
	&msm_ebi0_thermal,
	&msm_ebi1_thermal
};

static struct msm_gpio msm_i2c_gpios_hw[] = {
	{ GPIO_CFG(70, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), "i2c_scl" },
	{ GPIO_CFG(71, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), "i2c_sda" },
};

static struct msm_gpio msm_i2c_gpios_io[] = {
	{ GPIO_CFG(70, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), "i2c_scl" },
	{ GPIO_CFG(71, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), "i2c_sda" },
};

static struct msm_gpio qup_i2c_gpios_io[] = {
	{ GPIO_CFG(16, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), "qup_scl" },
	{ GPIO_CFG(17, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), "qup_sda" },
};
static struct msm_gpio qup_i2c_gpios_hw[] = {
	{ GPIO_CFG(16, 2, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), "qup_scl" },
	{ GPIO_CFG(17, 2, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), "qup_sda" },
};

static void
msm_i2c_gpio_config(int adap_id, int config_type)
{
	struct msm_gpio *msm_i2c_table;

	/* Each adapter gets 2 lines from the table */
	if (adap_id > 0)
		return;
	if (config_type)
		msm_i2c_table = &msm_i2c_gpios_hw[adap_id*2];
	else
		msm_i2c_table = &msm_i2c_gpios_io[adap_id*2];
	msm_gpios_enable(msm_i2c_table, 2);
}
/*This needs to be enabled only for OEMS*/
#ifndef CONFIG_QUP_EXCLUSIVE_TO_CAMERA
static struct vreg *qup_vreg;
#endif
static void
qup_i2c_gpio_config(int adap_id, int config_type)
{
	int rc = 0;
	struct msm_gpio *qup_i2c_table;
	/* Each adapter gets 2 lines from the table */
	if (adap_id != 4)
		return;
	if (config_type)
		qup_i2c_table = qup_i2c_gpios_hw;
	else
		qup_i2c_table = qup_i2c_gpios_io;
	rc = msm_gpios_enable(qup_i2c_table, 2);
	if (rc < 0)
		printk(KERN_ERR "QUP GPIO enable failed: %d\n", rc);
	/*This needs to be enabled only for OEMS*/
#ifndef CONFIG_QUP_EXCLUSIVE_TO_CAMERA
	if (qup_vreg) {
		int rc = vreg_set_level(qup_vreg, 1800);
		if (rc) {
			pr_err("%s: vreg LVS1 set level failed (%d)\n",
			__func__, rc);
		}
		rc = vreg_enable(qup_vreg);
		if (rc) {
			pr_err("%s: vreg_enable() = %d \n",
			__func__, rc);
		}
	}
#endif
}

static struct msm_i2c_platform_data msm_i2c_pdata = {
	.clk_freq = 100000,
	.pri_clk = 70,
	.pri_dat = 71,
	.rmutex  = 1,
	.rsl_id = "D:I2C02000021",
	.msm_i2c_config_gpio = msm_i2c_gpio_config,
};

static void __init msm_device_i2c_init(void)
{
	if (msm_gpios_request(msm_i2c_gpios_hw, ARRAY_SIZE(msm_i2c_gpios_hw)))
		pr_err("failed to request I2C gpios\n");

	msm_device_i2c.dev.platform_data = &msm_i2c_pdata;
}

static struct msm_i2c_platform_data msm_i2c_2_pdata = {
	.clk_freq = 100000,
	.rmutex  = 1,
	.rsl_id = "D:I2C02000022",
	.msm_i2c_config_gpio = msm_i2c_gpio_config,
};

static void __init msm_device_i2c_2_init(void)
{
	msm_device_i2c_2.dev.platform_data = &msm_i2c_2_pdata;
}

static struct msm_i2c_platform_data qup_i2c_pdata = {
	.clk_freq = 384000,
	.msm_i2c_config_gpio = qup_i2c_gpio_config,
};

static void __init qup_device_i2c_init(void)
{
	if (msm_gpios_request(qup_i2c_gpios_hw, ARRAY_SIZE(qup_i2c_gpios_hw)))
		pr_err("failed to request I2C gpios\n");

	qup_device_i2c.dev.platform_data = &qup_i2c_pdata;
	/*This needs to be enabled only for OEMS*/
#ifndef CONFIG_QUP_EXCLUSIVE_TO_CAMERA
	qup_vreg = regulator_get(&qup_device_i2c.dev, "lvsw1");
	if (IS_ERR(qup_vreg)) {
		dev_err(&qup_device_i2c.dev,
			"%s: regulator_get failed: %ld\n",
			__func__, PTR_ERR(qup_vreg));
	}
#endif
}

#ifdef CONFIG_I2C_SSBI
static struct msm_i2c_ssbi_platform_data msm_i2c_ssbi7_pdata = {
	.rsl_id = "D:CODEC_SSBI",
	.controller_type = MSM_SBI_CTRL_SSBI,
};
#endif

#ifdef CONFIG_SERIAL_MSM_CONSOLE
static struct msm_gpio uart2_config_data[] = {
	{ GPIO_CFG(51, 2, GPIO_CFG_INPUT,   GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), "UART2_Rx"},
	{ GPIO_CFG(52, 2, GPIO_CFG_OUTPUT,  GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), "UART2_Tx"},
};

static void msm7x30_init_uart2(void)
{
	msm_gpios_request_enable(uart2_config_data,
			ARRAY_SIZE(uart2_config_data));

}
#endif

static struct msm_spm_platform_data msm_spm_data __initdata = {
	.reg_base_addr = MSM_SAW_BASE,

	.reg_init_values[MSM_SPM_REG_SAW_CFG] = 0x05,
	.reg_init_values[MSM_SPM_REG_SAW_SPM_CTL] = 0x18,
	.reg_init_values[MSM_SPM_REG_SAW_SPM_SLP_TMR_DLY] = 0x00006666,
	.reg_init_values[MSM_SPM_REG_SAW_SPM_WAKE_TMR_DLY] = 0xFF000666,

	.reg_init_values[MSM_SPM_REG_SAW_SLP_CLK_EN] = 0x01,
	.reg_init_values[MSM_SPM_REG_SAW_SLP_HSFS_PRECLMP_EN] = 0x03,
	.reg_init_values[MSM_SPM_REG_SAW_SLP_HSFS_POSTCLMP_EN] = 0x00,

	.reg_init_values[MSM_SPM_REG_SAW_SLP_CLMP_EN] = 0x01,
	.reg_init_values[MSM_SPM_REG_SAW_SLP_RST_EN] = 0x00,
	.reg_init_values[MSM_SPM_REG_SAW_SPM_MPM_CFG] = 0x00,

	.awake_vlevel = 0xF2,
	.retention_vlevel = 0xE0,
	.collapse_vlevel = 0x72,
	.retention_mid_vlevel = 0xE0,
	.collapse_mid_vlevel = 0xE0,

	.vctl_timeout_us = 50,
};


extern int display_common_init(void);
static void __init msm7x30_init(void)
{
	uint32_t soc_version = 0;

	soc_version = socinfo_get_version();

	msm_clock_init(&msm7x30_clock_init_data);
#ifdef CONFIG_SERIAL_MSM_CONSOLE
	if (lge_get_uart_mode())
	msm7x30_init_uart2();
#endif
	msm_spm_init(&msm_spm_data, 1);
	acpuclk_init(&acpuclk_7x30_soc_data);

	msm_add_pmem_devices();
	msm_add_fb_device();
	msm_add_kgsl_device();

	msm_add_usb_devices();

#ifdef CONFIG_MSM7KV2_AUDIO
	lge_victor_audio_init();
	msm_snddev_init();
	aux_pcm_gpio_init();
#endif

	if (lge_get_uart_mode())
	  platform_device_register(&msm_device_uart2);

	msm_device_i2c_init();
	msm_device_i2c_2_init();
	qup_device_i2c_init();

#if defined(LGE_MODEL_E730)
//	msm7x30_init_marimba();
#else
//	register_board_info();
#endif
	/* initialize pm */
	pmic8058_leds_init();

	pmic8058_buses_init();

#ifdef CONFIG_I2C_SSBI
	msm_device_ssbi7.dev.platform_data = &msm_i2c_ssbi7_pdata;
#endif

	platform_add_devices(devices, ARRAY_SIZE(devices));

	pm8058_gpios_init();

/*********** LGE Adaptation *************/

	lge_pm_set_platform_data();

	/* add lcd devices */
	lge_add_lcd_devices();

	/* add mmc devices */
	lge_add_mmc_devices();

	/* add misc devices */
	lge_add_misc_devices();

	/* add input devices */
	lge_add_input_devices();

	/* init audio devices */	/* Do fix position */
	msm7x30_init_marimba();

	platform_device_register(&msm_device_i2c);
	platform_device_register(&msm_device_i2c_2);

	/* gpio i2c devices should be registered at latest point */
	lge_add_gpio_i2c_devices();

	/* add buletooth devices */
	lge_add_btpower_devices();

	/* add camera devices */
	lge_add_camera_devices();

	/* add ram console device */
#ifdef CONFIG_ANDROID_RAM_CONSOLE
	lge_add_ramconsole_devices();
#endif

#if defined(CONFIG_ANDROID_RAM_CONSOLE) && defined(CONFIG_LGE_HANDLE_PANIC)
	lge_add_panic_handler_devices();
#endif

	display_common_init();

}


/* Belows are moved from devices_lge.c */
static struct resource msm_fb_resources[] = {
	{
		.flags  = IORESOURCE_DMA,
	}
};

static int msm_fb_detect_panel(const char *name)
{
	if (!strncmp(name, "mddi_toshiba_wvga_pt", 20))
		return -EPERM;
	else if (!strncmp(name, "lcdc_toshiba_wvga_pt", 20))
		return 0;
	else if (!strcmp(name, "mddi_orise"))
		return -EPERM;
	else if (!strcmp(name, "mddi_quickvx"))
		return -EPERM;

	return -ENODEV;
}

static struct msm_fb_platform_data msm_fb_pdata = {
	.detect_client = msm_fb_detect_panel,
	.mddi_prescan = 1,
};

static struct platform_device msm_fb_device = {
	.name   = "msm_fb",
	.id     = 0,
	.num_resources  = ARRAY_SIZE(msm_fb_resources),
	.resource       = msm_fb_resources,
	.dev    = {
		.platform_data = &msm_fb_pdata,
	}
};

#ifdef CONFIG_LGE_HIDDEN_RESET_PATCH
static unsigned int fb_phys_addr;
static size_t fb_copy_phys;
static size_t fb_copy_size;
static size_t *fb_copy_virt;
static size_t *fb_copy_virt_rgb888;

unsigned int lge_get_fb_phys_addr(void)
{
	return fb_phys_addr;
}

void *lge_get_fb_addr(void)
{
	return (void *)__va(msm_fb_resources[0].start);
}

void *lge_get_fb_copy_phys_addr(void)
{
	return (void *)fb_copy_phys;
}

void *lge_get_fb_copy_virt_addr(void)
{
	return (void *)fb_copy_virt;
}

void *lge_get_fb_copy_virt_rgb888_addr(void)
{
	return (void *)fb_copy_virt_rgb888;
}

static void __init lge_make_fb_pmem(void)
{
	struct membank *bank = &meminfo.bank[2];

	fb_copy_phys = bank->start + bank->size + LGE_RAM_CONSOLE_SIZE + LGE_CRASH_LOG_SIZE;
	fb_copy_size = 800 * 480 * 2;
	fb_copy_virt = ioremap(fb_copy_phys, fb_copy_size);

	printk("FB START PHYS ADDR : %x\n", fb_copy_phys);
	printk("FB START VIRT ADDR : %x\n", (unsigned int)fb_copy_virt);
	printk("FB SIZE : %x\n", fb_copy_size);

	if (on_hidden_reset)
		fb_copy_virt_rgb888 = kmalloc(sizeof(unsigned int) * 800 * 480 * 2, GFP_KERNEL);

	return;
}
#endif

void __init msm_add_fb_device(void)
{
#if defined(CONFIG_LGE_HIDDEN_RESET_PATCH)
    lge_make_fb_pmem();
#endif

	platform_device_register(&msm_fb_device);
}

static struct android_pmem_platform_data android_pmem_pdata = {
	.name = "pmem",
	.allocator_type = PMEM_ALLOCATORTYPE_ALLORNOTHING,
	.cached = 1,
	.memory_type = MEMTYPE_EBI0,
};

static struct platform_device android_pmem_device = {
	.name = "android_pmem",
	.id = 0,
	.dev = { .platform_data = &android_pmem_pdata },
};

static struct android_pmem_platform_data android_pmem_adsp_pdata = {
       .name = "pmem_adsp",
       .allocator_type = PMEM_ALLOCATORTYPE_BITMAP,
       .cached = 0,
	.memory_type = MEMTYPE_EBI0,
};

static struct android_pmem_platform_data android_pmem_audio_pdata = {
       .name = "pmem_audio",
       .allocator_type = PMEM_ALLOCATORTYPE_BITMAP,
       .cached = 0,
	.memory_type = MEMTYPE_EBI0,
};

static struct platform_device android_pmem_adsp_device = {
       .name = "android_pmem",
       .id = 2,
       .dev = { .platform_data = &android_pmem_adsp_pdata },
};

static struct platform_device android_pmem_audio_device = {
       .name = "android_pmem",
       .id = 4,
       .dev = { .platform_data = &android_pmem_audio_pdata },
};

static struct platform_device *pmem_devices[] __initdata = {
/*	&android_pmem_kernel_ebi1_device,  */ /* LGE_UPDATE kideok.kim@lge.com 20111216 kernel 3.0.x Adaptation */
	&android_pmem_device,
	&android_pmem_adsp_device,
	&android_pmem_audio_device,
};

void __init msm_add_pmem_devices(void)
{
	platform_add_devices(pmem_devices, ARRAY_SIZE(pmem_devices));
}

static unsigned pmem_sf_size = MSM_PMEM_SF_SIZE;
static int __init pmem_sf_size_setup(char *p)
{
	pmem_sf_size = memparse(p, NULL);
	return 0;
}
early_param("pmem_sf_size", pmem_sf_size_setup);

static unsigned fb_size;
static int __init fb_size_setup(char *p)
{
	fb_size = memparse(p, NULL);
	return 0;
}
early_param("fb_size", fb_size_setup);

static unsigned pmem_adsp_size = MSM_PMEM_ADSP_SIZE;
static int __init pmem_adsp_size_setup(char *p)
{
	pmem_adsp_size = memparse(p, NULL);
	return 0;
}
early_param("pmem_adsp_size", pmem_adsp_size_setup);

static unsigned fluid_pmem_adsp_size = MSM_FLUID_PMEM_ADSP_SIZE;
static int __init fluid_pmem_adsp_size_setup(char *p)
{
	fluid_pmem_adsp_size = memparse(p, NULL);
	return 0;
}
early_param("fluid_pmem_adsp_size", fluid_pmem_adsp_size_setup);

static unsigned pmem_audio_size = MSM_PMEM_AUDIO_SIZE;
static int __init pmem_audio_size_setup(char *p)
{
	pmem_audio_size = memparse(p, NULL);
	return 0;
}
early_param("pmem_audio_size", pmem_audio_size_setup);

static unsigned pmem_kernel_ebi0_size = PMEM_KERNEL_EBI0_SIZE;
static int __init pmem_kernel_ebi0_size_setup(char *p)
{
	pmem_kernel_ebi0_size = memparse(p, NULL);
	return 0;
}
early_param("pmem_kernel_ebi0_size", pmem_kernel_ebi0_size_setup);

static struct memtype_reserve msm7x30_reserve_table[] __initdata = {
	[MEMTYPE_SMI] = {
	},
	[MEMTYPE_EBI0] = {
		.flags	=	MEMTYPE_FLAGS_1M_ALIGN,
	},
	[MEMTYPE_EBI1] = {
		.flags	=	MEMTYPE_FLAGS_1M_ALIGN,
	},
};

static void __init size_pmem_devices(void)
{
#ifdef CONFIG_ANDROID_PMEM
	unsigned long size;

	if machine_is_msm7x30_fluid()
		size = fluid_pmem_adsp_size;
	else
		size = pmem_adsp_size;
	android_pmem_adsp_pdata.size = size;
	android_pmem_audio_pdata.size = pmem_audio_size;
	android_pmem_pdata.size = pmem_sf_size;
#endif
}

static void __init reserve_memory_for(struct android_pmem_platform_data *p)
{
	msm7x30_reserve_table[p->memory_type].size += p->size;
}

static void __init reserve_pmem_memory(void)
{
#ifdef CONFIG_ANDROID_PMEM
	reserve_memory_for(&android_pmem_adsp_pdata);
	reserve_memory_for(&android_pmem_audio_pdata);
	reserve_memory_for(&android_pmem_pdata);
	msm7x30_reserve_table[MEMTYPE_EBI0].size += pmem_kernel_ebi0_size;
#endif
}

#ifdef CONFIG_FB_MSM_OVERLAY0_WRITEBACK
static void __init reserve_mdp_memory(void)
{
//	mdp_pdata.ov0_wb_size = MSM_FB_OVERLAY0_WRITEBACK_SIZE; 
	msm7x30_reserve_table[MEMTYPE_EBI0 /* mdp_pdata.mem_hid */].size += MSM_FB_OVERLAY0_WRITEBACK_SIZE;//mdp_pdata.ov0_wb_size;
}
#endif

static void __init msm7x30_calculate_reserve_sizes(void)
{
	size_pmem_devices();
	reserve_pmem_memory();
#ifdef CONFIG_FB_MSM_OVERLAY0_WRITEBACK	
  reserve_mdp_memory();
#endif
}


/* LGE_UPDATE_S youngrok.song@lge.com QCT404032I patch */
#if 1 //404032I
#define DDR1_BANK_BASE 0X20000000
#define DDR2_BANK_BASE 0X40000000

static unsigned int phys_add = DDR2_BANK_BASE;
unsigned long ebi1_phys_offset = DDR2_BANK_BASE;
EXPORT_SYMBOL(ebi1_phys_offset);

static int msm7x30_paddr_to_memtype(unsigned int paddr)
{
	if (paddr < phys_add)
		return MEMTYPE_EBI0;
	if (paddr >= phys_add && paddr < 0x80000000)
		return MEMTYPE_EBI1;
	return MEMTYPE_NONE;
}

static void __init msm7x30_fixup(struct machine_desc *desc, struct tag *tags,
				 char **cmdline, struct meminfo *mi)
{
	for (; tags->hdr.size; tags = tag_next(tags)) {
		if (tags->hdr.tag == ATAG_MEM && tags->u.mem.start ==
							DDR1_BANK_BASE) {
				ebi1_phys_offset = DDR1_BANK_BASE;
				phys_add = DDR1_BANK_BASE;
				break;
		}
	}
}
#else //404029I
static int msm7x30_paddr_to_memtype(unsigned int paddr)
{
	if (paddr < 0x10000000)
		return MEMTYPE_EBI0;
	if (paddr >= 0x20000000 && paddr < 0x30000000)
		return MEMTYPE_EBI1;
	return MEMTYPE_NONE;
}
#endif
/* LGE_UPDATE_E youngrok.song@lge.com QCT404032I patch */

static struct reserve_info msm7x30_reserve_info __initdata = {
	.memtype_reserve_table = msm7x30_reserve_table,
	.calculate_reserve_sizes = msm7x30_calculate_reserve_sizes,
	.paddr_to_memtype = msm7x30_paddr_to_memtype,
};
static void __init msm7x30_reserve(void)
{
	reserve_info = &msm7x30_reserve_info;
	msm_reserve();
}

void __init msm7x30_allocate_memory_regions(void)
{
	void *addr;
	unsigned long size;

	size = fb_size ? : MSM_FB_SIZE;
	addr = alloc_bootmem_align(size, 0x1000);
	msm_fb_resources[0].start = __pa(addr);
	msm_fb_resources[0].end = msm_fb_resources[0].start + size - 1;
	pr_info("allocating %lu bytes at %p (%lx physical) for fb\n",
		size, addr, __pa(addr));

#ifdef CONFIG_LGE_HIDDEN_RESET_PATCH
	fb_phys_addr = __pa(addr);
#endif

#ifdef CONFIG_FB_MSM_LCDC_LGDISPLAY_WVGA_OLED 
	/* LGE_CHANGE 
	* Copy the oled display screen to oled frame buffer
	* 2011-03-22, cheongil.hyun@lge.com
	*/
	memcpy(addr, __va(0x2FD00000), 480*800*2);
#endif
// LGE_UPDATE_S rpc_stuck
#ifdef CONFIG_LGE_RPC_DOG_CRASH_DEBUG
	size = ulScheduleLogBufSize;
	if (size) {
		pvScheduleLogBufV = alloc_bootmem(size);
		pvScheduleLogBufP = (void*)__pa((unsigned long)pvScheduleLogBufV);
		printk(KERN_INFO "[%s:%d] Allocating %lu bytes at %p (%lx physical) for kernel scheduling log buffer\n", __func__, __LINE__, size, pvScheduleLogBufV, (long unsigned int)pvScheduleLogBufP);
	}
	nNextLogIdx = 0;
#endif	
// LGE_UPDATE_E
}

static void __init msm7x30_init_irq(void)
{
	msm_init_irq();
}

static void __init msm7x30_map_io(void)
{
	msm_shared_ram_phys = 0x00100000;
	msm_map_msm7x30_io();
	if (socinfo_init() < 0)
		printk(KERN_ERR "%s: socinfo_init() failed!\n",
		       __func__);
}

static void __init msm7x30_init_early(void)
{
	msm7x30_allocate_memory_regions();
}

MACHINE_START(MSM8X55_VICTOR, "LGE MSM8X55 VICTOR")
	.boot_params = PLAT_PHYS_OFFSET + 0x100,
	.map_io = msm7x30_map_io,
	.reserve = msm7x30_reserve,
	.init_irq = msm7x30_init_irq,
	.init_machine = msm7x30_init,
	.timer = &msm_timer,
	.init_early = msm7x30_init_early,
	.handle_irq = vic_handle_irq,
	.fixup = msm7x30_fixup,
MACHINE_END
