/* drivers/video/backlight/aat33xx_bl.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/backlight.h>
#include <linux/fb.h>
#include <linux/delay.h>
#include <linux/gpio.h>
//#include <mach/board-flip.h>
#include <mach/vreg.h>

#define MODULE_NAME  "aat3369_bl"
#define CONFIG_BACKLIGHT_LEDS_CLASS

#ifdef CONFIG_BACKLIGHT_LEDS_CLASS
#include <linux/leds.h>
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#define SLIDE
#ifdef SLIDE
#include <linux/input.h>
#endif

static int debug = 1;

#define dprintk(fmt, args...) \
	do { \
		if (debug) \
			printk(KERN_INFO "%s:%s: " fmt, MODULE_NAME, __func__, ## args); \
	} while(0);

#define eprintk(fmt, args...)   printk(KERN_ERR "%s:%s: " fmt, MODULE_NAME, __func__, ## args)

int aat3369_step = 30;

int aat3369_level[30]=		
{
30,
35,
40,
45,
50,
56,
63,
72,
80,
87,
100,
109,
114,
120,
130,
138,
146,
154,
162,
170,
178,
186,
194,
202,
210,
218,
226,
234,
242,
256	};

int aat3369_brightness[32]=		
{	
2,
3,
4,
5,
6,
7,
8,
9,
10,
11,
12,
13,
14,
15,
16,
17,
18,
19,
20,
21,
22,
23,
24,
25,
26,
27,
28,
29,
30,
31 };

/********************************************
 * Definition
 ********************************************/

#define LEDS_BACKLIGHT_NAME "sublcd-backlight"

/*
 *The GPIO pin working as S2C pin for Backlight control.
 * 
*/
static int s2c_gpio_num;

/* @brightness : The current brightness level of backlight.
*/
static int brightness;

/* @backup_brightness: save the brightness value if turning off due to slider.
*/
static int backup_brightness;

/* @device_state : 0 => slider closed; 1 => slider opened.
*/
static int device_state;

/* Mutex needed to protect brightness from concurrent access
 * by LED classdev and inputdev (slider open/close) 
*/
//mutex_lock is not permitted during isr context. Replace mutex with spin_lock_irqsave
//DEFINE_MUTEX (brightness_lock);
spinlock_t brightness_lock;

#define MAX_STEPS 32

static void aat33xx_set_brightness (int level)
{
	int j;
	if (!device_state) {
		dprintk(" @off\n");
		brightness = level;
		return;
	}
	//dprintk("set brightness: %d\n", level);
	if (level == 0){
		gpio_set_value (s2c_gpio_num, 0);
		brightness = level;
		return;
	}
	for (j=MAX_STEPS; j>=level; j--)
	{
		if (!j) break;
		gpio_set_value (s2c_gpio_num, 0);
		udelay(2);
		gpio_set_value (s2c_gpio_num, 1);
		udelay(2);
	}
	mdelay(10);
	brightness = level;
}

static void subleds_brightness_set(struct led_classdev *led_cdev, enum led_brightness value)
{
	int next, i;
	unsigned long irqflags;

#if 0
	next = value *  32 / LED_FULL;
#else
#if 0
	if (value <= 56)
		next = value * 2 / 56;
	else if(value > 56 && value <= 102)
		next = (value - 56) * (18 - 2) / (102 - 56) + 2;
	else if(value >102)
		next = (value - 102) * (31 - 18) / (LED_FULL - 102) + 18;
#endif
#if 0
	if (value <= 56)
		next = value * 2 / 56;
	else if(value > 56 && value <= 102)
		next = (value - 56) * (14 - 2) / (102 - 56) + 2;
	else if(value >102)
		next = (value - 102) * (31 - 14) / (LED_FULL - 102) + 14;
//	printk("really_android input brightness =%d]\n", 32-next);
#else
	if(value < aat3369_level[0])		
	{
		next = 0; 
	}
	else
	{
		for(i = 0; i < aat3369_step - 1; i++)
		{
			if(value >= aat3369_level[i] && value < aat3369_level[i + 1])
				break;
		}
		
		next = aat3369_brightness[i];
	}
//	printk("[set %d scaled to %d current is %d]\n", value, next, brightness);
#endif

#endif	
	if (value != 0 && next == 0) next++;	//lower boundary case

	dprintk("[set %d scaled to %d current is %d]\n", value, next, brightness);

	if (brightness != next) {
/*[sumeet.gupta 240111 temporary reducing sub-lcd backlight level */
		//mutex_lock is not permitted during isr context. Replace mutex with spin_lock_irqsave
		//mutex_lock(&brightness_lock);
		spin_lock_irqsave(&brightness_lock, irqflags);
		aat33xx_set_brightness(next);
		//aat33xx_set_brightness(next/3);	//temp: to reduce power wastage.
		//mutex_unlock(&brightness_lock);
		spin_unlock_irqrestore(&brightness_lock, irqflags);
/*]sumeet.gupta 240111 temporary reducing sub-lcd backlight level */
	}
}


static struct led_classdev aat33xx_led_dev = {
	.name = LEDS_BACKLIGHT_NAME,
	.brightness_set = subleds_brightness_set,
};

#ifdef SLIDE
static const struct input_device_id sld_ids[] = {
	{.flags = INPUT_DEVICE_ID_MATCH_EVBIT | INPUT_DEVICE_ID_MATCH_SWBIT,
	 .evbit = { BIT_MASK(EV_SW) },
	 .swbit = { BIT_MASK(SW_LID) },
	},
	{ },
};

static int sld_input_connect(struct input_handler *handler,
		struct input_dev *dev, const struct input_device_id *id)
{
	struct input_handle *handle;
	int error;

	dprintk("+ \n" );
	handle = kzalloc(sizeof(struct input_handle), GFP_KERNEL);
	if (!handle)
		return -ENOMEM;

	handle->dev = dev;
	handle->handler = handler;
	handle->name = "slider-sublcd";

	error = input_register_handle(handle);
	if (error)
		goto err2;

	error = input_open_device(handle);
	if (error)
		goto err1;

	return 0;
err1:
	input_unregister_handle(handle);
err2:
	kfree(handle);
	return error;
}

static void sld_input_disconnect(struct input_handle *handle)
{
	input_close_device(handle);
	input_unregister_handle(handle);
	kfree(handle);
}

static void sld_input_event(struct input_handle *handle, unsigned int type,
		unsigned int code, int value)
{
	unsigned long irqflags;

	//mutex_lock is not permitted during isr context. Replace mutex with spin_lock_irqsave
	//mutex_lock(&brightness_lock);
	spin_lock_irqsave(&brightness_lock, irqflags);

	if (type == EV_SW && code == SW_LID && value == 1) //closed.
	{
		dprintk("slide close detected\n");
		backup_brightness = brightness;
		aat33xx_set_brightness (0);
		device_state = 0;
	}
	else
	if (type == EV_SW && code == SW_LID && value == 0) //opened.
	{
		dprintk("slide open detected\n" );
		device_state = 1;
		aat33xx_set_brightness (brightness? brightness : backup_brightness);
	}
	//mutex_unlock(&brightness_lock);
	spin_unlock_irqrestore(&brightness_lock, irqflags);

}

static struct input_handler sld_input_handler = {
	.event		= sld_input_event,
	.connect	= sld_input_connect,
	.disconnect	= sld_input_disconnect,
	.name		= "slider_subbl",
	.id_table	= sld_ids,
};
#endif

static int aat33xx_probe(struct platform_device *pdev)
{
	int ret;
	int i;
	eprintk("%s: started.\n", __func__);
	if (!pdev->num_resources){
		eprintk("GPIO resource not specified by device, Bailing out\n" );
		return ENODEV;
	}
	s2c_gpio_num = pdev->resource[0].start;
	
	ret = gpio_request (s2c_gpio_num, "sublcd_bl");
	if (ret){
		eprintk("Failed to Aquire GPIO %d, Bailing out\n", s2c_gpio_num);
		return ret;
	}
	gpio_direction_output (s2c_gpio_num, 1);

	gpio_set_value (s2c_gpio_num, 1);
	mdelay (10);
	
	device_state = 1;

	for (i=0; i<MAX_STEPS; i++)
	{
//		aat33xx_set_brightness (i);
//		mdelay(100);
	}

	//Not registering as a backlight device for now; only LED device is what is needed.
//#ifdef CONFIG_FB_MSM_DPSV
#if 0
	extern int dpsv_led_register(struct device*, struct led_classdev*);
	if (dpsv_led_register(&pdev->dev, &aat33xx_led_dev) == 0) {
#else
	if (led_classdev_register(&pdev->dev, &aat33xx_led_dev) == 0) {
#endif
		dprintk("Registered led class dev successfully.\n" );
	}

	spin_lock_init(&brightness_lock);
	
#ifdef SLIDE
	ret = input_register_handler(&sld_input_handler);
	dprintk("input_register_handler: %d.\n", ret );
	
#endif

	return 0;
}

static struct platform_driver this_driver = {	
	.probe  = aat33xx_probe,
	.driver = {
		.name   = MODULE_NAME,
	},
};

static int __init aat33xx_init(void)
{
	int ret;
	dprintk("AAT33XX init start\n");
	ret = platform_driver_register(&this_driver);
	return 0;
}

static void __exit aat33xx_exit(void)
{
	dprintk("AAT33XX stop\n");
}

module_init(aat33xx_init);
module_exit(aat33xx_exit);

MODULE_DESCRIPTION("Backlight driver for ANALOGIC TECH AAT3369");
MODULE_AUTHOR("Sumeet Gupta <sumeet.gupta@lge.com>");
MODULE_LICENSE("GPL");

