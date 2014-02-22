/* arch/arm/mach-msm/board-flip-camera.c
 * Copyright (C) 2010 LGE, Inc.
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
#include <linux/types.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/platform_device.h>
#include <asm/setup.h>
#include <mach/gpio.h>
#include <mach/board.h>
#include <mach/msm_iomap.h>
#include <mach/vreg.h>
#include <mach/camera.h>
#include <mach/board.h>
#include <linux/regulator/consumer.h>

#include "devices.h"
#include <mach/board_lge.h>
/*====================================================================================
            RESET/PWDN
 =====================================================================================*/
//[LGE_UPDATE_S] jeonghoon.cho@lge.com 2011.06.22
#define CAM_MAIN_I2C_SLAVE_ADDR         (0x36)     
//[LGE_UPDATE_E]
#define CAM_VGA_I2C_SLAVE_ADDR          (0x7C >> 1)

#define CAM_MAIN_GPIO_RESET_N           (0)
#define CAM_VGA_GPIO_RESET_N            (163)
#define CAM_VGA_GPIO_PWDN               (164)


/*====================================================================================
            MSM VPE Device
 =====================================================================================*/
#ifdef CONFIG_MSM_VPE
static struct resource msm_vpe_resources[] = {
	{
		.start	= 0xAD200000,
		.end	= 0xAD200000 + SZ_1M - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= INT_VPE,
		.end	= INT_VPE,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device msm_vpe_device = {
       .name = "msm_vpe",
       .id   = 0,
       .num_resources = ARRAY_SIZE(msm_vpe_resources),
       .resource = msm_vpe_resources,
};
#endif

/*====================================================================================
            Devices
 =====================================================================================*/

static struct platform_device *victor_camera_msm_devices[] __initdata = {
 #ifdef CONFIG_MSM_VPE
    &msm_vpe_device,
#endif
// BEGIN soojeong.hwang@lge.com  Move to Board-univaq.c 
//#ifdef CONFIG_MSM_ROTATOR
//    &msm_rotator_device,
//#endif
// END soojeong.hwang@lge.com  Move to Board-univaq.c 

};


/*====================================================================================
            Flash
 =====================================================================================*/
#if defined(CONFIG_MT9P017) || defined(CONFIG_TCM9000MD)

#define CAM_FLASH_I2C_SLAVE_ADDR		0x53

static struct msm_camera_sensor_flash_src msm_flash_src = {
	.flash_sr_type = MSM_CAMERA_FLASH_SRC_PMIC,
};

static struct msm_camera_sensor_flash_data flash_mt9p017 = {
	.flash_type = MSM_CAMERA_FLASH_LED,
	.flash_src  = &msm_flash_src
};
#endif

/*====================================================================================
                                  Camera Sensor  ( Main Camera : ISX006 , VT Camera : MT9M113)
  ====================================================================================*/
//[LGE_UPDATE_S] jeonghoon.cho@lge.com 2011.06.22
static struct i2c_board_info camera_i2c_devices[] = {
#ifdef CONFIG_MT9P017
        [0]= {
            I2C_BOARD_INFO("mt9p017", CAM_MAIN_I2C_SLAVE_ADDR),
        },
#endif
#ifdef CONFIG_TCM9000MD   
    [2] = {
        I2C_BOARD_INFO("tcm9000md", CAM_VGA_I2C_SLAVE_ADDR),
    },
#endif
#ifdef CONFIG_LM2759_FLASH
	[1] = {
		I2C_BOARD_INFO("lm2759", CAM_FLASH_I2C_SLAVE_ADDR),
	},
#endif	
};
//[LGE_UPDATE_E]



#ifdef CONFIG_MSM_CAMERA
static uint32_t camera_off_gpio_table[] = {
        /* parallel CAMERA interfaces */
        GPIO_CFG(2,  0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT0 */
        GPIO_CFG(3,  0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT1 */
        GPIO_CFG(4,  0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT2 */
        GPIO_CFG(5,  0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT3 */
        GPIO_CFG(6,  0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT4 */
        GPIO_CFG(7,  0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT5 */
        GPIO_CFG(8, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT6 */
        GPIO_CFG(9, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT7 */
        GPIO_CFG(10, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT8 */
        GPIO_CFG(11, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT9 */
        GPIO_CFG(12, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* PCLK */
        GPIO_CFG(13, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* HSYNC_IN */
        GPIO_CFG(14, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* VSYNC_IN */
        GPIO_CFG(15, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), /* MCLK */
};

static uint32_t camera_on_gpio_table[] = {
        /* parallel CAMERA interfaces */
        GPIO_CFG(2,  1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT0 */
        GPIO_CFG(3,  1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT1 */
        GPIO_CFG(4,  1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT2 */
        GPIO_CFG(5,  1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT3 */
        GPIO_CFG(6,  1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT4 */
        GPIO_CFG(7,  1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT5 */
        GPIO_CFG(8, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT6 */
        GPIO_CFG(9, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT7 */
        GPIO_CFG(10, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT8 */
        GPIO_CFG(11, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT9 */
        GPIO_CFG(12, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_16MA), /* PCLK */
        GPIO_CFG(13, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* HSYNC_IN */
        GPIO_CFG(14, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* VSYNC_IN */
        GPIO_CFG(15, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_6MA), /* MCLK */
};

static void config_gpio_table(uint32_t *table, int len)
{
        int n, rc;
        for (n = 0; n < len; n++) {
                rc = gpio_tlmm_config(table[n], GPIO_CFG_ENABLE);
                if (rc) {
                        printk(KERN_ERR "%s: gpio_tlmm_config(%#x)=%d\n",
                                __func__, table[n], rc);
                        break;
                }
        }
}
int config_camera_on_gpios(void)
{
    config_gpio_table(camera_on_gpio_table,
            ARRAY_SIZE(camera_on_gpio_table));

    return 0;
}

void config_camera_off_gpios(void)
{
        config_gpio_table(camera_off_gpio_table,
                ARRAY_SIZE(camera_off_gpio_table));
}


/*====================================================================================
            Main Camera Power On
 =====================================================================================*/
static struct regulator_bulk_data regs_main1[] = {
  { .supply = "lvsw0" },
  { .supply = "gp2",  .min_uV = 2800000, .max_uV = 2800000  }
};

static struct regulator_bulk_data regs_main2[] = {
  { .supply = "gp9",  .min_uV = 2800000, .max_uV = 2800000 }
};

static int reg_count_main1;
static int reg_count_main2;



static struct platform_device msm_camera_sensor_mt9p017;

int main_camera_power_off (void)
{
    printk(KERN_ERR "%s: main_camera_power_off \n",__func__);

    gpio_set_value(CAM_MAIN_GPIO_RESET_N, 0);
    mdelay(1);
    
    msm_camio_clk_disable(CAMIO_CAM_MCLK_CLK);
    mdelay(1);

		regulator_bulk_disable(reg_count_main2, regs_main2);
		regulator_bulk_disable(reg_count_main1, regs_main1);
		regulator_bulk_free(reg_count_main2, regs_main2);
		regulator_bulk_free(reg_count_main1, regs_main1);
		reg_count_main1 = 0;
		reg_count_main2 = 0;

    return 0;
}


int main_camera_power_on (struct platform_device *pdev)
{
		int count1, count2, rc;
		struct device *dev = &pdev->dev;
				
    printk(KERN_ERR "%s: main_camera_power_on \n",__func__);

    gpio_set_value(CAM_MAIN_GPIO_RESET_N, 0);

		// Turn On Main Camera Power Part1
		count1 = ARRAY_SIZE(regs_main1);
		rc = regulator_bulk_get(dev, count1, regs_main1);
		if (rc) {
			dev_err(dev, "%s: regs_main1 could not get regulators: %d\n", __func__, rc);
			return 0;
		}
		
		rc = regulator_bulk_set_voltage(count1, regs_main1);
		if (rc) {
			dev_err(dev, "%s: regs_main1 could not set voltages: %d\n", __func__, rc);
			goto reg_free1;
		}
		
		rc = regulator_bulk_enable(count1, regs_main1);
		if (rc) {
			dev_err(dev, "%s: regs_main1 could not enable regulators: %d\n", __func__, rc);
			goto reg_free1;
		}
		
		reg_count_main1 = count1;

    mdelay(20);
		// Supply Clock and Make high on reset
		msm_camio_clk_enable(CAMIO_CAM_MCLK_CLK);
	mdelay(10);

    gpio_set_value(CAM_MAIN_GPIO_RESET_N, 1);
    mdelay(10);
		
		// Turn On Main Camera Power Part2
		count2 = ARRAY_SIZE(regs_main2);
		rc = regulator_bulk_get(dev, count2, regs_main2);
		if (rc) {
			dev_err(dev, "%s: regs_main2 could not get regulators: %d\n", __func__, rc);
			goto reg_free1;;
		}
		
		rc = regulator_bulk_set_voltage(count2, regs_main2);
		if (rc) {
			dev_err(dev, "%s: regs_main2 could not set voltages: %d\n", __func__, rc);
			goto reg_free2;
		}
		
		rc = regulator_bulk_enable(count2, regs_main2);
		if (rc) {
			dev_err(dev, "%s: regs_main2 could not enable regulators: %d\n", __func__, rc);
			goto reg_free2;
		}
		
		reg_count_main2 = count2;

    mdelay(3);

    return 0;
    
reg_free2:
    regulator_bulk_free(count2, regs_main2);
    reg_count_main2 = 0;
    regulator_bulk_disable(reg_count_main1, regs_main1);
reg_free1:
    regulator_bulk_free(count1, regs_main1);
    reg_count_main1 = 0;

    return 0;
}
#if defined (CONFIG_TCM9000MD)

/*====================================================================================
            VGA Camera Power On
 =====================================================================================*/
static struct regulator_bulk_data regs_vga1[] = {
  { .supply = "lvsw0"},
};

static struct regulator_bulk_data regs_vga2[] = {
  { .supply = "gp13",  .min_uV = 1800000, .max_uV = 1800000  },
  { .supply = "gp9",  .min_uV = 2800000, .max_uV = 2800000  }
};

static int reg_count_vga1;
static int reg_count_vga2;


int vga_camera_power_off (void)
{
    gpio_set_value(CAM_VGA_GPIO_RESET_N, 0);
    mdelay(1);

    gpio_set_value(CAM_VGA_GPIO_PWDN, 1);
    mdelay(1);
    msm_camio_clk_disable(CAMIO_CAM_MCLK_CLK);

    regulator_bulk_disable(reg_count_vga2, regs_vga2);
    regulator_bulk_free(reg_count_vga2, regs_vga2);
    reg_count_vga2 = 0;

    gpio_set_value(CAM_VGA_GPIO_PWDN, 0);
    mdelay(1);

    regulator_bulk_disable(reg_count_vga1, regs_vga1);
    regulator_bulk_free(reg_count_vga1, regs_vga1);
    reg_count_vga1 = 0;
    
    printk(KERN_ERR "vga_camera_power_off\n");
    
    return 0;        
}

int vga_camera_power_on (struct platform_device *pdev)
{
    int count1, count2, rc;
		struct device *dev = &pdev->dev;

    printk(KERN_ERR "vga_camera_power_on\n");

    gpio_set_value(CAM_VGA_GPIO_RESET_N, 0);
    mdelay(1);

    gpio_set_value(CAM_VGA_GPIO_PWDN, 0);
    mdelay(1);


		// Turn On Main Camera Power Part1
		count1 = ARRAY_SIZE(regs_vga1);
		rc = regulator_bulk_get(dev, count1, regs_vga1);
		if (rc) {
			dev_err(dev, "%s: regs_vga1 could not get regulators: %d\n", __func__, rc);
			return 0;
		}
		
		rc = regulator_bulk_set_voltage(count1, regs_vga1);
		if (rc) {
			dev_err(dev, "%s: regs_vga1 could not set voltages: %d\n", __func__, rc);
			goto reg_free1;
		}
		
		rc = regulator_bulk_enable(count1, regs_vga1);
		if (rc) {
			dev_err(dev, "%s: regs_vga1 could not enable regulators: %d\n", __func__, rc);
			goto reg_free1;
		}
		
		reg_count_vga1 = count1;

		// Turn On Main Camera Power Part2
		count2 = ARRAY_SIZE(regs_vga2);
		rc = regulator_bulk_get(dev, count2, regs_vga2);
		if (rc) {
			dev_err(dev, "%s: regs_vga2 could not get regulators: %d\n", __func__, rc);
			goto reg_free1;;
		}
		
		rc = regulator_bulk_set_voltage(count2, regs_vga2);
		if (rc) {
			dev_err(dev, "%s: regs_vga2 could not set voltages: %d\n", __func__, rc);
			goto reg_free2;
		}
		
		rc = regulator_bulk_enable(count2, regs_vga2);
		if (rc) {
			dev_err(dev, "%s: regs_vga2 could not enable regulators: %d\n", __func__, rc);
			goto reg_free2;
		}
		
		reg_count_vga2 = count2;

    gpio_set_value(CAM_VGA_GPIO_PWDN, 1);
    mdelay(1);

    msm_camio_clk_enable(CAMIO_CAM_MCLK_CLK);
    mdelay(1);
    
    gpio_set_value(CAM_VGA_GPIO_PWDN, 0);
    mdelay(1);
    
    gpio_set_value(CAM_VGA_GPIO_RESET_N, 1);
    mdelay(10);

    return 0;
    
reg_free2:
    regulator_bulk_free(count2, regs_vga2);
    reg_count_vga2 = 0;
    regulator_bulk_disable(reg_count_vga1, regs_vga1);
reg_free1:
    regulator_bulk_free(count1, regs_vga1);
    reg_count_vga1 = 0;

    return 0;
}

#elif defined (CONFIG_MT9V113)

int vga_camera_power_off (void)
{
    struct vreg *vreg_cam_iovdd_1_8v;
    struct vreg *vreg_cam_dvdd_1_8v;
    struct vreg *vreg_cam_avdd_2_8v;

    gpio_set_value(CAM_VGA_GPIO_RESET_N, 0);
    mdelay(1);

    gpio_set_value(CAM_VGA_GPIO_PWDN, 0);
    mdelay(5);

    msm_camio_clk_disable(CAMIO_CAM_MCLK_CLK);
    mdelay(5);


    vreg_cam_dvdd_1_8v = vreg_get(NULL, "gp13");
    vreg_disable(vreg_cam_dvdd_1_8v);

    vreg_cam_avdd_2_8v = vreg_get(NULL, "gp9");
    vreg_disable(vreg_cam_avdd_2_8v);

    vreg_cam_iovdd_1_8v = vreg_get(NULL, "lvsw0");
    vreg_disable(vreg_cam_iovdd_1_8v);

    mdelay(10);
	
    printk(KERN_ERR "vga_camera_power_off\n");
    
    return 0;        
}

int vga_camera_power_on (struct platform_device *pdev)
{
    int rc;

    struct vreg *vreg_cam_iovdd_1_8v;
    struct vreg *vreg_cam_dvdd_1_8v;
    struct vreg *vreg_cam_avdd_2_8v;

    printk(KERN_ERR "vga_camera_power_on\n");

    gpio_set_value(CAM_VGA_GPIO_RESET_N, 1);
    mdelay(1);

    gpio_set_value(CAM_VGA_GPIO_PWDN, 1);
    mdelay(1);

    vreg_cam_iovdd_1_8v = vreg_get(NULL, "lvsw0");
    vreg_enable(vreg_cam_iovdd_1_8v);
    mdelay(1);

    vreg_cam_avdd_2_8v = vreg_get(NULL, "gp9");
    rc = vreg_set_level(vreg_cam_avdd_2_8v, 2800);
    vreg_enable(vreg_cam_avdd_2_8v);
    mdelay(1);
	
    vreg_cam_dvdd_1_8v = vreg_get(NULL, "gp13");
    rc = vreg_set_level(vreg_cam_dvdd_1_8v, 1800);
    vreg_enable(vreg_cam_dvdd_1_8v);
    mdelay(10);

    /* Input MCLK = 24MHz */
    msm_camio_clk_enable(CAMIO_CAM_MCLK_CLK);
    mdelay(10);

    gpio_set_value(CAM_VGA_GPIO_PWDN, 0);
    mdelay(5);

    gpio_set_value(CAM_VGA_GPIO_RESET_N, 0);
    mdelay(10);
    
    gpio_set_value(CAM_VGA_GPIO_RESET_N, 1);
    mdelay(10);

    return 0;
}

#endif


struct resource msm_camera_resources[] = {
        {
                .start  = 0xA6000000,
                .end    = 0xA6000000 + SZ_1M - 1,
                .flags  = IORESOURCE_MEM,
        },
        {
                .start  = INT_VFE,
                .end    = INT_VFE,
                .flags  = IORESOURCE_IRQ,
        },
};

static struct msm_camera_device_platform_data msm_main_camera_device_data = {
        .camera_power_on   = main_camera_power_on,
        .camera_power_off  = main_camera_power_off,
        .camera_gpio_on    = config_camera_on_gpios,
        .camera_gpio_off   = config_camera_off_gpios,
        .ioext.camifpadphy = 0xAB000000,
        .ioext.camifpadsz  = 0x00000400,
        .ioext.csiphy      = 0xA6100000,
        .ioext.csisz       = 0x00000400,
        .ioext.csiirq      = INT_CSI,
        .ioclk.mclk_clk_rate = 24000000,
        .ioclk.vfe_clk_rate  = 153600000,       //122880000,            //QCT에서 변경 122880000->153600000  : CONFIG_MT9P017용
};

#ifdef CONFIG_TCM9000MD
static struct msm_camera_device_platform_data msm_vga_camera_device_data = {
        .camera_power_on                = vga_camera_power_on,
        .camera_power_off               = vga_camera_power_off,
        .camera_gpio_on                 = config_camera_on_gpios,
        .camera_gpio_off                = config_camera_off_gpios,
        .ioext.camifpadphy              = 0xAB000000,
        .ioext.camifpadsz               = 0x00000400,
        .ioext.csiphy                   = 0xA6100000,
        .ioext.csisz                    = 0x00000400,
        .ioext.csiirq                   = INT_CSI,
        .ioclk.mclk_clk_rate = 24000000,
        .ioclk.vfe_clk_rate  = 122880000,
};
#endif

#ifdef CONFIG_MT9P017
static struct msm_camera_sensor_info msm_camera_sensor_mt9p017_data = {
        .sensor_name    = "mt9p017",
        .sensor_reset   = CAM_MAIN_GPIO_RESET_N,
        .sensor_pwd     = CAM_MAIN_GPIO_RESET_N,
        .vcm_pwd        = 1,
        .vcm_enable     = 1,
        .pdata          = &msm_main_camera_device_data,
        .resource       = msm_camera_resources,
        .num_resources  = ARRAY_SIZE(msm_camera_resources),
        .flash_data     = &flash_mt9p017,
        .csi_if         = 1
};

static struct platform_device msm_camera_sensor_mt9p017 = {
        .name      = "msm_camera_mt9p017",
        .dev       = {
                .platform_data = &msm_camera_sensor_mt9p017_data,
        },
};
#endif

#ifdef CONFIG_TCM9000MD
static struct msm_camera_sensor_info msm_camera_sensor_tcm9000md_data = {
        .sensor_name      = "tcm9000md",
        .sensor_reset     = CAM_VGA_GPIO_RESET_N,
        .sensor_pwd       = CAM_VGA_GPIO_PWDN,
        .vcm_pwd          = 0,
        .vcm_enable       = 0,
        .pdata            = &msm_vga_camera_device_data,
        .resource         = msm_camera_resources,
        .num_resources    = ARRAY_SIZE(msm_camera_resources),
        .flash_data       = NULL,
        .csi_if           = 0,  
};

static struct platform_device msm_camera_sensor_tcm9000md = {
        .name      = "msm_camera_tcm9000md",
        .dev       = {
                .platform_data = &msm_camera_sensor_tcm9000md_data,
        },
};
#endif // CONFIG_TCM9000MD
#endif // CONFIG_MSM_CAMERA

static struct platform_device *victor_camera_devices[] __initdata = {
#ifdef CONFIG_MT9P017
    &msm_camera_sensor_mt9p017,
#endif
#ifdef CONFIG_TCM9000MD
    &msm_camera_sensor_tcm9000md,
#endif
};

void __init lge_add_camera_devices(void)
{
    int rc;
    // Initialize Main Camera Ctrl GPIO
    rc = gpio_tlmm_config( GPIO_CFG(CAM_MAIN_GPIO_RESET_N, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
                            GPIO_CFG_ENABLE);
                            
    rc = gpio_tlmm_config( GPIO_CFG(CAM_VGA_GPIO_RESET_N, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
                            GPIO_CFG_ENABLE);
                                
    rc = gpio_tlmm_config( GPIO_CFG(CAM_VGA_GPIO_PWDN, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
                            GPIO_CFG_ENABLE);

    printk(KERN_ERR "Set Main Camera GPIO : %d\n", rc);

    i2c_register_board_info(4 /* QUP ID */, camera_i2c_devices, ARRAY_SIZE(camera_i2c_devices));
    printk(KERN_ERR "i2c_register_board_info : %d\n", ARRAY_SIZE(camera_i2c_devices));
    
    platform_add_devices(victor_camera_msm_devices, ARRAY_SIZE(victor_camera_msm_devices));
    printk(KERN_ERR "platform_add_devices(camera_msm) : %d\n", ARRAY_SIZE(victor_camera_msm_devices));

    platform_add_devices(victor_camera_devices, ARRAY_SIZE(victor_camera_devices));
    printk(KERN_ERR "platform_add_devices(camera) : %d\n", ARRAY_SIZE(victor_camera_devices));
}

