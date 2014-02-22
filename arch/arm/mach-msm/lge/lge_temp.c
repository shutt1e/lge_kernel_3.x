/*
 *
 *
 *
 */

#include <linux/platform_device.h>
#include <asm/io.h>
#include <linux/slab.h>
#include <mach/board_lge.h>
#include <linux/wakelock.h>

int early_suspend_ondemand;
// LGE_UPDATE for preventing halt of system server restart 
struct wake_lock system_server_crash_wake_lock;

static int get_qem_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	unsigned value;

	value = lge_get_nv_qem();
	printk("%s: qem=%d\n",__func__,value);
	
	return sprintf(buf, "%d\n", value);
}
static DEVICE_ATTR(qem, 0664, get_qem_show, NULL);


static int msm_nv_frststatus_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	unsigned char frststatus;

	frststatus = (unsigned char)lge_get_nv_frststatus();
	
	return sprintf(buf, "%d\n", frststatus);
}

static int msm_nv_frststatus_store(struct device *dev,
					struct device_attribute *attr, const char *buf, size_t count)
{
    unsigned char frststatus = (unsigned char)simple_strtoul(buf, NULL, 10);

	lge_set_nv_frststatus(frststatus);

	return 0;
}

static DEVICE_ATTR(frststatus, 0660, msm_nv_frststatus_show, msm_nv_frststatus_store);


static int msm_cpu_early_suspend_ondemand_show(struct device *dev, struct device_attribute *attr, char *buf)
{	
	return sprintf(buf, "%d\n", early_suspend_ondemand);
}

static int msm_cpu_early_suspend_ondemand_store(struct device *dev,
					struct device_attribute *attr, const char *buf, size_t count)
{
    early_suspend_ondemand = (unsigned char)simple_strtoul(buf, NULL, 10);
    
	return 0;
}

static DEVICE_ATTR(cpu_early_suspend_ondemand, 0660, msm_cpu_early_suspend_ondemand_show, msm_cpu_early_suspend_ondemand_store);

// LGE_UPDATE for preventing halt of system server restart 
static int system_server_crash_report(struct device *dev, struct device_attribute *attr, char *buf)
{
	printk("system_server_crash_report\n");

	wake_lock_timeout(&system_server_crash_wake_lock, 60*HZ);
	
	return sprintf(buf, "%d\n", 1);
}

static DEVICE_ATTR(system_server_crash, 0444, system_server_crash_report, NULL);


static int __init lge_tempdevice_probe(struct platform_device *pdev)
{
	int err;
	err = device_create_file(&pdev->dev, &dev_attr_qem);
	if (err < 0) 
		printk("%s : Cannot create the sysfs QEM\n", __func__);

	err = device_create_file(&pdev->dev, &dev_attr_frststatus);
	if (err < 0) 
		printk("%s : Cannot create the sysfs FRSTSTATUS\n", __func__);

	err = device_create_file(&pdev->dev, &dev_attr_cpu_early_suspend_ondemand);
	if (err < 0) 
		printk("%s : Cannot create the sysfs cpu_early_suspend_ondemand\n", __func__);


	// LGE_UPDATE for preventing halt of system server restart 
	wake_lock_init(&system_server_crash_wake_lock, WAKE_LOCK_SUSPEND, "system_server_crash");

	err = device_create_file(&pdev->dev, &dev_attr_system_server_crash);
	if (err < 0) 
		printk("%s : Cannot create the sysfs system_server_crash\n", __func__);

	return err;
}

static struct platform_device lgetemp_device = {
	.name = "autoall",
	.id		= -1,
};

static struct platform_driver this_driver __refdata = {
	.probe = lge_tempdevice_probe,
	.driver = {
		.name = "autoall",
	},
};

int __init lge_tempdevice_init(void)
{
	printk("%s\n", __func__);
	platform_device_register(&lgetemp_device);

	return platform_driver_register(&this_driver);
}

module_init(lge_tempdevice_init);
MODULE_DESCRIPTION("Just temporal code for PV");
MODULE_AUTHOR("MoonCheol Kang <neo.kang@lge.com>");
MODULE_LICENSE("GPL");
