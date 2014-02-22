/*
 *  apds990x.c - Linux kernel modules for ambient light + proximity sensor
 *
 *  Copyright (C) 2010 Lee Kai Koon <kai-koon.lee@avagotech.com>
 *  Copyright (C) 2010 Avago Technologies
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/spinlock_types.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/input.h>
#include <mach/gpio.h>
#include <mach/board_lge.h>

/* apds990x Debug mask value
 * usage: echo [mask_value] > debug_mask
 * All		: 127
 * No msg	: 0
 * default	: 24
 */
enum {
	APDS990x_DEBUG_ERR_CHECK		= 1U << 0,
	APDS990x_DEBUG_DEV_STATUS		= 1U << 1,
	APDS990x_DEBUG_FUNC_TRACE		= 1U << 2,
	APDS990x_DEBUG_REG_INFO			= 1U << 3,
	APDS990x_DEBUG_DEV_DEBOUNCE		= 1U << 4,
	APDS990x_DEBUG_GEN_INFO			= 1U << 5,
	APDS990x_DEBUG_INTR_INFO		= 1U << 6,
	APDS990x_DEBUG_AT_COMMAND		= 1U << 7,
};

#define APDS990x_DEBUG_PRINT	(1)
#define APDS990x_ERROR_PRINT	(1)

#if defined(APDS990x_DEBUG_PRINT)
#define APDS990xD(fmt, args...) \
			printk(KERN_INFO "D[%-18s:%5d]" \
				fmt, __FUNCTION__, __LINE__, ##args);
#else
#define APDS990xD(fmt, args...)	{};
#endif

#if defined(APDS990x_ERROR_PRINT)
#define APDS990xE(fmt, args...) \
			printk(KERN_ERR "E[%-18s:%5d]" \
				fmt, __FUNCTION__, __LINE__, ##args);
#else
#define APDS990xE(fmt, args...)	{};
#endif


#define APDS990x_DRV_NAME	/*"apds990x"*/"apds9900"
#define DRIVER_VERSION		"1.0.4"

#define APDS990x_INT		IRQ_EINT20

#define APDS990x_PS_DETECTION_THRESHOLD		300//400//600	//gooni.shim@lge.com 26-SEP-2011 Proximity & ALS Sensor tunning for Rev1.1
#define APDS990x_PS_HSYTERESIS_THRESHOLD	200//300//500	//gooni.shim@lge.com 30-JUN-2011 Proximity & ALS Sensor tunning for RevE

// gooni.shim@lge.com 21-Jul-2011 Change value for ALC Tunning. 
//#define APDS990x_ALS_THRESHOLD_HSYTERESIS	20	/* 20 = 20% */
#define APDS990x_ALS_THRESHOLD_HSYTERESIS	40	/* 40 = 40% */


/* Change History 
 *
 * 1.0.1	Functions apds990x_show_rev(), apds990x_show_id() and apds990x_show_status()
 *			have missing CMD_BYTE in the i2c_smbus_read_byte_data(). APDS-990x needs
 *			CMD_BYTE for i2c write/read byte transaction.
 *
 *
 * 1.0.2	Include PS switching threshold level when interrupt occurred
 *
 *
 * 1.0.3	Implemented ISR and delay_work, correct PS threshold storing
 *
 * 1.0.4	Added Input Report Event
 */

/*
 * Defines
 */

#define APDS990x_ENABLE_REG	0x00
#define APDS990x_ATIME_REG	0x01
#define APDS990x_PTIME_REG	0x02
#define APDS990x_WTIME_REG	0x03
#define APDS990x_AILTL_REG	0x04
#define APDS990x_AILTH_REG	0x05
#define APDS990x_AIHTL_REG	0x06
#define APDS990x_AIHTH_REG	0x07
#define APDS990x_PILTL_REG	0x08
#define APDS990x_PILTH_REG	0x09
#define APDS990x_PIHTL_REG	0x0A
#define APDS990x_PIHTH_REG	0x0B
#define APDS990x_PERS_REG	0x0C
#define APDS990x_CONFIG_REG	0x0D
#define APDS990x_PPCOUNT_REG	0x0E
#define APDS990x_CONTROL_REG	0x0F
#define APDS990x_REV_REG	0x11
#define APDS990x_ID_REG		0x12
#define APDS990x_STATUS_REG	0x13
#define APDS990x_CDATAL_REG	0x14
#define APDS990x_CDATAH_REG	0x15
#define APDS990x_IRDATAL_REG	0x16
#define APDS990x_IRDATAH_REG	0x17
#define APDS990x_PDATAL_REG	0x18
#define APDS990x_PDATAH_REG	0x19

#define CMD_BYTE	0x80
#define CMD_WORD	0xA0
#define CMD_SPECIAL	0xE0

#define CMD_CLR_PS_INT	0xE5
#define CMD_CLR_ALS_INT	0xE6
#define CMD_CLR_PS_ALS_INT	0xE7

#define SUPPORT_FOLDER_TEST		0
#define SUPPORT_ALS_POLLING		0

/*
 * Structs
 */
#if SUPPORT_FOLDER_TEST
#if defined(CONFIG_HAS_EARLYSUSPEND)
#include <linux/earlysuspend.h>
	
struct early_suspend apds990x_sensor_early_suspend;
static void apds990x_early_suspend(struct early_suspend *h);
static void apds990x_late_resume(struct early_suspend *h);
#endif
#endif

struct apds990x_data {
	struct i2c_client *client;
	// protect device state (in the registers and in pdata)
	// - always take this lock if you modify registers or pdata
	// - reading registers is already synchronized on the i2c bus
	// - reading from pdata migth yield inconsistent results. This might
	//   be acceptable to not always block android until a current reading is
	//   through. *** Alternatively the members of pdata could be made atomic_t ***
	//
	// functions with a "_locked" prefix require the caller to already hold the lock
	struct mutex device_lock;
	// lock the workqueu for updates of scheduled work
	struct spinlock wq_lock;
	struct delayed_work	dwork;	/* for PS interrupt */
#if SUPPORT_ALS_POLLING
	struct delayed_work    als_dwork; /* for ALS polling */
#endif
	struct input_dev *input_dev_als;
	struct input_dev *input_dev_ps;

	unsigned int enable;
	unsigned int atime;
	unsigned int ptime;
	unsigned int wtime;
	unsigned int ailt;
	unsigned int aiht;
	unsigned int pilt;
	unsigned int piht;
	unsigned int pers;
	unsigned int config;
	unsigned int ppcount;
	unsigned int control;

	/* control flag from HAL */
	unsigned int enable_ps_sensor;
	unsigned int enable_als_sensor;

	/* PS parameters */
	unsigned int ps_threshold;
	unsigned int ps_hysteresis_threshold; /* always lower than ps_threshold */
	unsigned int ps_detection;		/* 0 = near-to-far; 1 = far-to-near */
	unsigned int ps_detection_for_atcmd;
	unsigned int ps_data;			/* to store PS data */

	/* ALS parameters */
	unsigned int als_threshold_l;	/* low threshold */
	unsigned int als_threshold_h;	/* high threshold */
	unsigned int als_data;			/* to store ALS data */

	unsigned int als_gain;			/* needed for Lux calculation */
	unsigned int als_poll_delay;	/* needed for light sensor polling : micro-second (us) */
	unsigned int als_atime;			/* storage for als integratiion time */

	unsigned int irq;				/* Terminal out irq number */
};


/*
 * Global data
 */
static unsigned int apds990x_debug_mask = APDS990x_DEBUG_DEV_STATUS;

#if SUPPORT_FOLDER_TEST
static struct i2c_client *apds990x_i2c_client = NULL;
#endif
/*
 * Management functions
 */

static s32 apds990x_rapper_i2c_write_byte(struct i2c_client *client, u8 value)
{
	int cnt;
	s32 ret;
	
	for (ret = 0, cnt = 0; cnt <= 3; cnt++) {
		ret = i2c_smbus_write_byte(client, value);

		if(APDS990x_DEBUG_FUNC_TRACE & apds990x_debug_mask)
			APDS990xD("writing to i2c_smbus_write_byte (%d), cnt (%d)\n", ret, cnt);
		
		if (ret != -110)
			break;

		if (ret < 0)
			APDS990xE("re-writing to i2c_smbus_write_byte\n");
	}

	return ret;
}

static s32 apds990x_rapper_i2c_write_byte_data(struct i2c_client *client, u8 command, u8 value)
{
	int cnt;
	s32 ret;
	
	for (ret = 0, cnt = 0; cnt <= 3; cnt++) {
		ret = i2c_smbus_write_byte_data(client, command, value);

		if(APDS990x_DEBUG_FUNC_TRACE & apds990x_debug_mask)
			APDS990xD("writing to i2c_write_byte_data (%d), cnt (%d)\n", ret, cnt);
		
		if (!ret)
			break;
		
		if (ret < 0)
			APDS990xE("re-writing to i2c_write_byte_data\n");
	}

	return ret;
}

static s32 apds990x_rapper_i2c_write_word_data(struct i2c_client *client, u8 command, u16 value)
{
	int cnt;
	s32 ret;
	
	for (ret = 0, cnt = 0; cnt <= 3; cnt++) {
		ret = i2c_smbus_write_word_data(client, command, value);

		if(APDS990x_DEBUG_FUNC_TRACE & apds990x_debug_mask)
			APDS990xD("writing to i2c_write_word_data (%d), cnt (%d)\n", ret, cnt);
		
		if (!ret)
			break;

		if (ret < 0)
			APDS990xE("re-writing to i2c_write_word_data\n");
	}

	return ret;
}

static int apds990x_set_command_locked(struct i2c_client *client, int command)
{
	int ret;
	int clearInt;

	if (command == 0)
		clearInt = CMD_CLR_PS_INT;
	else if (command == 1)
		clearInt = CMD_CLR_ALS_INT;
	else
		clearInt = CMD_CLR_PS_ALS_INT;
		
	//ret = i2c_smbus_write_byte(client, clearInt);
	ret = apds990x_rapper_i2c_write_byte(client, clearInt);

	return ret;
}

static int apds990x_set_enable_locked(struct i2c_client *client, int enable)
{
	struct apds990x_data *data = i2c_get_clientdata(client);
	int ret;

	if(APDS990x_DEBUG_DEV_STATUS & apds990x_debug_mask)
		APDS990xD("enable = %x\n", enable);
	
	//ret = i2c_smbus_write_byte_data(client, CMD_BYTE|APDS990x_ENABLE_REG, enable);
	ret = apds990x_rapper_i2c_write_byte_data(client, CMD_BYTE|APDS990x_ENABLE_REG, enable);

	data->enable = enable;

	return ret;
}

static int apds990x_set_atime_locked(struct i2c_client *client, int atime)
{
	struct apds990x_data *data = i2c_get_clientdata(client);
	int ret;
	
	//ret = i2c_smbus_write_byte_data(client, CMD_BYTE|APDS990x_ATIME_REG, atime);
	ret = apds990x_rapper_i2c_write_byte_data(client, CMD_BYTE|APDS990x_ATIME_REG, atime);

	data->atime = atime;

	return ret;
}

static int apds990x_set_ptime_locked(struct i2c_client *client, int ptime)
{
	struct apds990x_data *data = i2c_get_clientdata(client);
	int ret;
	
	//ret = i2c_smbus_write_byte_data(client, CMD_BYTE|APDS990x_PTIME_REG, ptime);
	ret = apds990x_rapper_i2c_write_byte_data(client, CMD_BYTE|APDS990x_PTIME_REG, ptime);

	data->ptime = ptime;

	return ret;
}

static int apds990x_set_wtime_locked(struct i2c_client *client, int wtime)
{
	struct apds990x_data *data = i2c_get_clientdata(client);
	int ret;
	
	//ret = i2c_smbus_write_byte_data(client, CMD_BYTE|APDS990x_WTIME_REG, wtime);
	ret = apds990x_rapper_i2c_write_byte_data(client, CMD_BYTE|APDS990x_WTIME_REG, wtime);

	data->wtime = wtime;

	return ret;
}

static int apds990x_set_ailt_locked(struct i2c_client *client, int threshold)
{
	struct apds990x_data *data = i2c_get_clientdata(client);
	int ret;

	//ret = i2c_smbus_write_word_data(client, CMD_WORD|APDS990x_AILTL_REG, threshold);
	ret = apds990x_rapper_i2c_write_word_data(client, CMD_WORD|APDS990x_AILTL_REG, threshold);
	
	data->ailt = threshold;

	return ret;
}

static int apds990x_set_aiht_locked(struct i2c_client *client, int threshold)
{
	struct apds990x_data *data = i2c_get_clientdata(client);
	int ret;
	
	//ret = i2c_smbus_write_word_data(client, CMD_WORD|APDS990x_AIHTL_REG, threshold);
	ret = apds990x_rapper_i2c_write_word_data(client, CMD_WORD|APDS990x_AIHTL_REG, threshold);
	
	data->aiht = threshold;

	return ret;
}

static int apds990x_set_pilt_locked(struct i2c_client *client, int threshold)
{
	struct apds990x_data *data = i2c_get_clientdata(client);
	int ret;
	
	//ret = i2c_smbus_write_word_data(client, CMD_WORD|APDS990x_PILTL_REG, threshold);
	ret = apds990x_rapper_i2c_write_word_data(client, CMD_WORD|APDS990x_PILTL_REG, threshold);
	
	data->pilt = threshold;

	return ret;
}

static int apds990x_set_piht_locked(struct i2c_client *client, int threshold)
{
	struct apds990x_data *data = i2c_get_clientdata(client);
	int ret;
	
	//ret = i2c_smbus_write_word_data(client, CMD_WORD|APDS990x_PIHTL_REG, threshold);
	ret = apds990x_rapper_i2c_write_word_data(client, CMD_WORD|APDS990x_PIHTL_REG, threshold);
	
	data->piht = threshold;

	return ret;
}

static int apds990x_set_pers_locked(struct i2c_client *client, int pers)
{
	struct apds990x_data *data = i2c_get_clientdata(client);
	int ret;
	
	//ret = i2c_smbus_write_byte_data(client, CMD_BYTE|APDS990x_PERS_REG, pers);
	ret = apds990x_rapper_i2c_write_byte_data(client, CMD_BYTE|APDS990x_PERS_REG, pers);

	data->pers = pers;

	return ret;
}

static int apds990x_set_config_locked(struct i2c_client *client, int config)
{
	struct apds990x_data *data = i2c_get_clientdata(client);
	int ret;
	
	//ret = i2c_smbus_write_byte_data(client, CMD_BYTE|APDS990x_CONFIG_REG, config);
	ret = apds990x_rapper_i2c_write_byte_data(client, CMD_BYTE|APDS990x_CONFIG_REG, config);

	data->config = config;

	return ret;
}

static int apds990x_set_ppcount_locked(struct i2c_client *client, int ppcount)
{
	struct apds990x_data *data = i2c_get_clientdata(client);
	int ret;
	
	//ret = i2c_smbus_write_byte_data(client, CMD_BYTE|APDS990x_PPCOUNT_REG, ppcount);
	ret = apds990x_rapper_i2c_write_byte_data(client, CMD_BYTE|APDS990x_PPCOUNT_REG, ppcount);
	
	data->ppcount = ppcount;

	return ret;
}

static int apds990x_set_control_locked(struct i2c_client *client, int control)
{
	struct apds990x_data *data = i2c_get_clientdata(client);
	int ret;
	
	//ret = i2c_smbus_write_byte_data(client, CMD_BYTE|APDS990x_CONTROL_REG, control);
	ret = apds990x_rapper_i2c_write_byte_data(client, CMD_BYTE|APDS990x_CONTROL_REG, control);
	
	data->control = control;

	/* obtain ALS gain value */
	if ((data->control&0x03) == 0x00) /* 1X Gain */
		data->als_gain = 1;
	else if ((data->control&0x03) == 0x01) /* 8X Gain */
		data->als_gain = 8;
	else if ((data->control&0x03) == 0x02) /* 16X Gain */
		data->als_gain = 16;
	else  /* 120X Gain */
		data->als_gain = 120;

	return ret;
}

static int LuxCalculation_locked(struct i2c_client *client, int cdata, int irdata)
{
	struct apds990x_data *data = i2c_get_clientdata(client);
	int luxValue=0;

	int IAC1=0;
	int IAC2=0;
	int IAC=0;
	
	// gooni.shim@lge.com 31-AUG-2011 Change value for ALC Tunning(Rev 1.1). 
	//int GA=48;			/* 0.48 without glass window */
	//int COE_B=223;		/* 2.23 without glass window */
	//int COE_C=70;		/* 0.70 without glass window */
	//int COE_D=142;		/* 1.42 without glass window */
	int GA=238;			/* 2.380 without glass window for flip(*100)*/
	int COE_B=203;		/* 2.028 without glass window for flip(*100)*/
	int COE_C=63;		/* 0.628 without glass window for flip(*100)*/
	int COE_D=121;		/* 1.212 without glass window for flip(*100)*/
	
	int DF=52;

	IAC1 = (cdata - (COE_B*irdata)/100);	// re-adjust COE_B to avoid 2 decimal point
	IAC2 = ((COE_C*cdata)/100 - (COE_D*irdata)/100); // re-adjust COE_C and COE_D to void 2 decimal point

	if (IAC1 > IAC2)
		IAC = IAC1;
	else if (IAC1 <= IAC2)
		IAC = IAC2;
	else
		IAC = 0;

	luxValue = ((IAC*GA*DF)/100)/(((272*(256-data->atime))/100)*data->als_gain);

	return luxValue;
}

static void apds990x_change_ps_threshold_locked(struct i2c_client *client)
{
	struct apds990x_data *data = i2c_get_clientdata(client);
	int irdata=0;

	data->ps_data =	i2c_smbus_read_word_data(client, CMD_WORD|APDS990x_PDATAL_REG);
	irdata = i2c_smbus_read_word_data(client, CMD_WORD|APDS990x_IRDATAL_REG);

	// gooni.shim@lge.com 13-Apr-2011 Change condition for Proximtiy Sensor Tunning.
	//if (data->ps_data >= data->ps_threshold){
	if ( (data->ps_data > data->pilt) && (data->ps_data >= data->piht) && (irdata != (100*(1024*(256-data->atime)))/100)) {
		/* far-to-near detected */
		data->ps_detection = 1;
		data->ps_detection_for_atcmd = 1;
		
		if (APDS990x_DEBUG_AT_COMMAND & apds990x_debug_mask)
			APDS990xD("case1 data->ps_detection_for_atcmd = %d\n", data->ps_detection_for_atcmd);
		
		input_report_abs(data->input_dev_ps, ABS_DISTANCE, 0);/* FAR-to-NEAR detection */
		input_sync(data->input_dev_ps);

		// gooni.shim@lge.com 13-Apr-2011 Add PS Threshold hysteresis for Proximtiy Sensor Tunning.
		//apds990x_set_pilt_locked(client, data->ps_threshold - 100);
		apds990x_set_pilt_locked(client, data->ps_hysteresis_threshold);
		apds990x_set_piht_locked(client, 1023);

		if (APDS990x_DEBUG_DEV_STATUS & apds990x_debug_mask)
			APDS990xD("far-to-near detected\n");
		
	}
	// gooni.shim@lge.com 13-Apr-2011 Change condition for Proximtiy Sensor Tunning.
	//else if (data->ps_data < (data->ps_threshold - 100)){
	else if ( (data->ps_data <= data->pilt) && (data->ps_data < data->piht) ) {
		/* near-to-far detected */
		data->ps_detection = 0;
		data->ps_detection_for_atcmd = 0;

		if (APDS990x_DEBUG_AT_COMMAND & apds990x_debug_mask)
			APDS990xD("case2 data->ps_detection_for_atcmd = %d\n", data->ps_detection_for_atcmd);
		
		input_report_abs(data->input_dev_ps, ABS_DISTANCE, 1);/* NEAR-to-FAR detection */
		input_sync(data->input_dev_ps);

		apds990x_set_pilt_locked(client, 0);
		apds990x_set_piht_locked(client, data->ps_threshold);

		if (APDS990x_DEBUG_DEV_STATUS & apds990x_debug_mask)
			APDS990xD("near-to-far detected(case1)\n");
		
	}
	else if ( (irdata == (100*(1024*(256-data->atime)))/100) && (data->ps_detection == 1) ) {
		/* under strong ambient light*/
		/*NEAR-to-FAR */
		data->ps_detection = 0;
		data->ps_detection_for_atcmd = 0;

		if (APDS990x_DEBUG_AT_COMMAND & apds990x_debug_mask)
			APDS990xD("case3 data->ps_detection_for_atcmd = %d\n", data->ps_detection_for_atcmd);
		
		input_report_abs(data->input_dev_ps, ABS_DISTANCE, 1);/* NEAR-to-FAR detection */	
		input_sync(data->input_dev_ps);

		/*Keep the threshold NEAR condition to prevent the frequent Interrupt under strong ambient light*/
		apds990x_set_pilt_locked(client, data->ps_hysteresis_threshold);
		apds990x_set_piht_locked(client, 1023);

		if (APDS990x_DEBUG_DEV_STATUS & apds990x_debug_mask){
			APDS990xD("Set PS Threshold NEAR condition to prevent the frequent Interrupt under strong ambient light\n");
			APDS990xD("near-to-far detected(case2)\n");
		}
	}
	else if ( (data->pilt == 1023) && (data->piht == 0) ){
		/* this is the first near-to-far forced interrupt */
		data->ps_detection = 0;
		data->ps_detection_for_atcmd = 0;

		if (APDS990x_DEBUG_AT_COMMAND & apds990x_debug_mask)
			APDS990xD("case4 data->ps_detection_for_atcmd = %d\n", data->ps_detection_for_atcmd);
		
		input_report_abs(data->input_dev_ps, ABS_DISTANCE, 1);/* NEAR-to-FAR detection */	
		input_sync(data->input_dev_ps);
		
		apds990x_set_pilt_locked(client, 0);
		apds990x_set_piht_locked(client, data->ps_threshold);

		if (APDS990x_DEBUG_DEV_STATUS & apds990x_debug_mask)
			APDS990xD("near-to-far detected(case3)\n");
	}
}

static void apds990x_change_als_threshold_locked(struct i2c_client *client)
{
	struct apds990x_data *data = i2c_get_clientdata(client);
	int cdata, irdata;
	int luxValue=0;

	cdata = i2c_smbus_read_word_data(client, CMD_WORD|APDS990x_CDATAL_REG);
	irdata = i2c_smbus_read_word_data(client, CMD_WORD|APDS990x_IRDATAL_REG);

	luxValue = LuxCalculation_locked(client, cdata, irdata);

	luxValue = luxValue>0 ? luxValue : 0;
	luxValue = luxValue<10000 ? luxValue : 10000;

	//23.Aug.2011, sh.kim, under the direct sunlight
	if(cdata == (1024 * (256 - data->atime)))
		luxValue = 10000;
	
	if (APDS990x_DEBUG_DEV_STATUS & apds990x_debug_mask)
		APDS990xD("luxValue : %d\n", luxValue);
	
	// check PS under sunlight
#if 0
	if ( (data->ps_detection == 1) && (cdata > (95*(1024*(256-data->atime)))/100))	// PS was previously in far-to-near condition
	{
		// need to inform input event as there will be no interrupt from the PS
		input_report_abs(data->input_dev_ps, ABS_DISTANCE, 1);/* NEAR-to-FAR detection */	
		input_sync(data->input_dev_ps);

		apds990x_set_pilt_locked(client, 0);
		apds990x_set_piht_locked(client, data->ps_threshold);
		
		data->ps_detection = 0;	/* near-to-far detected */
		data->ps_detection_for_atcmd = 0;

		if (APDS990x_DEBUG_DEV_STATUS & apds990x_debug_mask)
			APDS990xD("apds_990x_proximity_handler = FAR\n");	
	}
#endif

	input_report_abs(data->input_dev_als, ABS_MISC, luxValue); // report the lux level
	input_sync(data->input_dev_als);

	data->als_data = cdata;

	data->als_threshold_l = (data->als_data * (100-APDS990x_ALS_THRESHOLD_HSYTERESIS) ) /100;
	data->als_threshold_h = (data->als_data * (100+APDS990x_ALS_THRESHOLD_HSYTERESIS) ) /100;

	if (data->als_threshold_h >= 65535) 
		data->als_threshold_h = 65535;

	apds990x_set_ailt_locked(client, data->als_threshold_l);
	apds990x_set_aiht_locked(client, data->als_threshold_h);

}

static void apds990x_reschedule_work(struct apds990x_data *data,
					  unsigned long delay)
{
	unsigned long flags;

	spin_lock_irqsave(&data->wq_lock, flags);

	/*
	 * If work is already scheduled then subsequent schedules will not
	 * change the scheduled time that's why we have to cancel it first.
	 */
	__cancel_delayed_work(&data->dwork);
	schedule_delayed_work(&data->dwork, delay);

	spin_unlock_irqrestore(&data->wq_lock, flags);
}

#if SUPPORT_ALS_POLLING
static void apds990x_als_polling_work_handler(struct work_struct *work)
{
	struct apds990x_data *data = container_of(work, struct apds990x_data, als_dwork.work);
	struct i2c_client *client=data->client;
	int cdata, irdata, pdata;
	int luxValue=0;

	mutex_lock(&data->device_lock);
	cdata = i2c_smbus_read_word_data(client, CMD_WORD|APDS990x_CDATAL_REG);
	irdata = i2c_smbus_read_word_data(client, CMD_WORD|APDS990x_IRDATAL_REG);
	pdata = i2c_smbus_read_word_data(client, CMD_WORD|APDS990x_PDATAL_REG);
	
	luxValue = LuxCalculation_locked(client, cdata, irdata);
	
	luxValue = luxValue>0 ? luxValue : 0;
	luxValue = luxValue<10000 ? luxValue : 10000;

	if(APDS990x_DEBUG_REG_INFO & apds990x_debug_mask)
		APDS990xD("lux = %d, cdata = %x, irdata = %x, pdata = %x\n", luxValue, cdata, irdata, pdata);

	// check PS under sunlight
	if ( (data->ps_detection == 1) && (cdata > (75*(1024*(256-data->atime)))/100))	// PS was previously in far-to-near condition
	{
		// need to inform input event as there will be no interrupt from the PS
		input_report_abs(data->input_dev_ps, ABS_DISTANCE, 0);/* NEAR-to-FAR detection */	
		input_sync(data->input_dev_ps);
		
		apds990x_set_pilt_locked(client, 0);
		apds990x_set_piht_locked(client, data->ps_threshold);

		data->pilt = 0;
		data->piht = data->ps_threshold;

		data->ps_detection = 0;	/* near-to-far detected */

		if(APDS990x_DEBUG_DEV_STATUS & apds990x_debug_mask)
			APDS990xD("apds_990x_proximity_handler = FAR\n");	
	}

	input_report_abs(data->input_dev_als, ABS_MISC, luxValue); // report the lux level
	input_sync(data->input_dev_als);
	
	mutex_unlock(&data->device_lock);
	
	schedule_delayed_work(&data->als_dwork, msecs_to_jiffies(data->als_poll_delay));	// restart timer
}
#endif

/* PS interrupt routine */
static void apds990x_work_handler(struct work_struct *work)
{
	struct apds990x_data *data = container_of(work, struct apds990x_data, dwork.work);
	struct i2c_client *client=data->client;
	int	status=0;
	int cdata=0;
	int pdata=0;
	int irdata=0;
	int ailt=0,aiht=0;
	int enable_value = 0;
	int ret=0;	// kk 06 Aug 2011

	if(APDS990x_DEBUG_FUNC_TRACE & apds990x_debug_mask)
		APDS990xD("*** start ***\n");

	mutex_lock(&data->device_lock);
	status = i2c_smbus_read_byte_data(client, CMD_BYTE|APDS990x_STATUS_REG);
	if(APDS990x_DEBUG_REG_INFO & apds990x_debug_mask){
		enable_value = i2c_smbus_read_byte_data(client, CMD_BYTE|APDS990x_ENABLE_REG);
		APDS990xD("[before] APDS990x_ENABLE_REG =  %x\n", enable_value);
	}

	//ret = i2c_smbus_write_byte_data(client, CMD_BYTE|APDS990x_ENABLE_REG, 1);	/* disable 990x's ADC first */
	ret = apds990x_rapper_i2c_write_byte_data(client, CMD_BYTE|APDS990x_ENABLE_REG, 1);	/* disable 990x's ADC first */
	///if (ret<0) {
	///	APDS990xD("[middle1] error writing to enable register (%d) data->enable = %x\n", ret, data->enable);
	///	//ret = i2c_smbus_write_byte_data(client, CMD_BYTE|APDS990x_ENABLE_REG, data->enable);
	///	ret = apds990x_rapper_i2c_write_byte_data(client, CMD_BYTE|APDS990x_ENABLE_REG, data->enable);
	///	APDS990xD("[middle1] re-writing to enable register (%d)\n", ret);		
	///}

	if(APDS990x_DEBUG_REG_INFO & apds990x_debug_mask)
		APDS990xD("status = %x, data->enable = %x\n", status, data->enable);
	
	if ((status & data->enable & 0x30) == 0x30) {
		/* both PS and ALS are interrupted */
		apds990x_change_als_threshold_locked(client);
		
		/* check if this is triggered by background ambient noise */	
		irdata = i2c_smbus_read_word_data(client, CMD_WORD|APDS990x_IRDATAL_REG);
		
		if (irdata != (100*(1024*(256-data->atime)))/100){
			apds990x_change_ps_threshold_locked(client);
		}
		else {
			if (data->ps_detection == 1) {
				apds990x_change_ps_threshold_locked(client);			

				if(APDS990x_DEBUG_FUNC_TRACE & apds990x_debug_mask)
					APDS990xD("Triggered by background ambient noise. near-to-FAR.\n");	
			}
			else {
				
				/*Keep the threshold NEAR condition to prevent the frequent Interrup under strong ambient light*/
				apds990x_set_pilt_locked(client, data->ps_hysteresis_threshold);
				apds990x_set_piht_locked(client, 1023);

				if(APDS990x_DEBUG_FUNC_TRACE & apds990x_debug_mask){
					APDS990xD("Set PS Threshold NEAR condition to prevent the frequent Interrup under strong ambient light.\n");
					APDS990xD("Triggered by background ambient noise. maintain FAR.\n");
				}
			}
		}

		apds990x_set_command_locked(client, 2);	/* 2 = CMD_CLR_PS_ALS_INT */
	}
	else if ((status & data->enable & 0x20) == 0x20) {
		/* only PS is interrupted */
		
		/* check if this is triggered by background ambient noise */
		irdata = i2c_smbus_read_word_data(client, CMD_WORD|APDS990x_IRDATAL_REG);
		if (irdata != (100*(1024*(256-data->atime)))/100){
			apds990x_change_ps_threshold_locked(client);
		}
		else {
			if (data->ps_detection == 1) {
				apds990x_change_ps_threshold_locked(client);			

				if(APDS990x_DEBUG_FUNC_TRACE & apds990x_debug_mask)
		        	APDS990xD("* Triggered by background ambient noise. near-to-FAR.\n");
			}
			else {
				/*Keep the threshold NEAR condition to prevent the frequent Interrup under strong ambient light*/
				apds990x_set_pilt_locked(client, data->ps_hysteresis_threshold);
				apds990x_set_piht_locked(client, 1023);

				if(APDS990x_DEBUG_FUNC_TRACE & apds990x_debug_mask){
					APDS990xD("* Set PS Threshold NEAR condition to prevent the frequent Interrup under strong ambient light.\n");
					APDS990xD("* Triggered by background ambient noise. maintain FAR.\n");
				}
			}
		}

		apds990x_set_command_locked(client, 0);	/* 0 = CMD_CLR_PS_INT */
	}
	else if ((status & data->enable & 0x10) == 0x10) {
		/* only ALS is interrupted */	
		apds990x_change_als_threshold_locked(client);

		/* check if this is triggered by background ambient noise */
		if(data->enable_ps_sensor == 1){
			irdata = i2c_smbus_read_word_data(client, CMD_WORD|APDS990x_IRDATAL_REG);
			if (irdata != (100*(1024*(256-data->atime)))/100){
				apds990x_change_ps_threshold_locked(client);
			}
			else {							 
				if (data->ps_detection == 1) {
					apds990x_change_ps_threshold_locked(client);			
			
					if(APDS990x_DEBUG_FUNC_TRACE & apds990x_debug_mask)
						APDS990xD("* Triggered by background ambient noise. near-to-FAR.\n");
				}
				else {
			
					/*Keep the threshold NEAR condition to prevent the frequent Interrup under strong ambient light*/
					apds990x_set_pilt_locked(client, data->ps_hysteresis_threshold);
					apds990x_set_piht_locked(client, 1023);

					if(APDS990x_DEBUG_FUNC_TRACE & apds990x_debug_mask){
						APDS990xD("* Set PS Threshold NEAR condition to prevent the frequent Interrup under strong ambient light.\n");
						APDS990xD("* Triggered by background ambient noise. maintain FAR.\n");
					}
				}
			}
		}

		apds990x_set_command_locked(client, 1);	/* 1 = CMD_CLR_ALS_INT */

	}

	if(APDS990x_DEBUG_REG_INFO & apds990x_debug_mask){
		pdata = i2c_smbus_read_word_data(client, CMD_WORD|APDS990x_PDATAL_REG);
		cdata = i2c_smbus_read_word_data(client, CMD_WORD|APDS990x_CDATAL_REG);
		irdata = i2c_smbus_read_word_data(client, CMD_WORD|APDS990x_IRDATAL_REG);
		
		ailt = i2c_smbus_read_word_data(client, CMD_WORD|APDS990x_AILTL_REG);
		aiht = i2c_smbus_read_word_data(client, CMD_WORD|APDS990x_AIHTL_REG);
		
		APDS990xD("pdata = %d, cdata = %d, irdata = %d\n", pdata, cdata, irdata);
		APDS990xD("ailt = %d, aiht = %d\n", ailt, aiht);
	
		enable_value = i2c_smbus_read_byte_data(client, CMD_BYTE|APDS990x_ENABLE_REG);
		
		APDS990xD("[middle] APDS990x_ENABLE_REG =  %x, data->enable = %x\n", enable_value, data->enable);
	}

	//ret = i2c_smbus_write_byte_data(client, CMD_BYTE|APDS990x_ENABLE_REG, data->enable);
	ret = apds990x_rapper_i2c_write_byte_data(client, CMD_BYTE|APDS990x_ENABLE_REG, data->enable);
	///if (ret<0) {
	///	APDS990xD("[middle2] error writing to enable register (%d) data->enable = %x\n", ret, data->enable);
	///	//ret = i2c_smbus_write_byte_data(client, CMD_BYTE|APDS990x_ENABLE_REG, data->enable);
	///	ret = apds990x_rapper_i2c_write_byte_data(client, CMD_BYTE|APDS990x_ENABLE_REG, data->enable);
	///	APDS990xD("[middle2] re-writing to enable register (%d)\n", ret);		
	///}
	
	if(APDS990x_DEBUG_REG_INFO & apds990x_debug_mask)
		APDS990xD("data->enable_ps_sensor = %d, data->enable_als_sensor = %d\n", data->enable_ps_sensor, data->enable_als_sensor);
	

	if(APDS990x_DEBUG_REG_INFO & apds990x_debug_mask){
		enable_value = i2c_smbus_read_byte_data(client, CMD_BYTE|APDS990x_ENABLE_REG);
		
		APDS990xD("[after] APDS990x_ENABLE_REG =  %x\n", enable_value);
	}

	mutex_unlock(&data->device_lock);
	
	if(APDS990x_DEBUG_FUNC_TRACE & apds990x_debug_mask)
		APDS990xD("*** end ***\n");
}

/* assume this is ISR */
static irqreturn_t apds990x_interrupt(int vec, void *info)
{
	struct i2c_client *client=(struct i2c_client *)info;
	struct apds990x_data *data = i2c_get_clientdata(client);

	if(APDS990x_DEBUG_FUNC_TRACE & apds990x_debug_mask)
		APDS990xD("==> apds990x_interrupt\n");
	
	apds990x_reschedule_work(data, 0);	

	return IRQ_HANDLED;
}

/*
 * SysFS support
 */
static ssize_t apds990x_show_enable_ps_sensor(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds990x_data *data = i2c_get_clientdata(client);
	
	return sprintf(buf, "%d\n", data->enable_ps_sensor);
}

static ssize_t apds990x_store_enable_ps_sensor(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = NULL;
	struct apds990x_data *data = NULL;
	unsigned long val = simple_strtoul(buf, NULL, 10);
#if SUPPORT_ALS_POLLING
	unsigned long flags;
#endif
	int hw_rev_num = lge_get_hw_rev();
	int enable_value = 0;
	
	client = to_i2c_client(dev);
	data = i2c_get_clientdata(client);
	
	// Null pointer check.
	if(!client)
		return count;
	if(!data)
		return count;

	if(APDS990x_DEBUG_FUNC_TRACE & apds990x_debug_mask)
		APDS990xD("hw_rev_num : %d\n", hw_rev_num);
	
	if ((val != 0) && (val != 1)) {
		if(APDS990x_DEBUG_ERR_CHECK & apds990x_debug_mask)
			APDS990xE("store unvalid value=%lx\n", val);
		return count;
	}
	
	if(APDS990x_DEBUG_DEV_STATUS & apds990x_debug_mask)
		APDS990xD("enable ps senosr ( %lx)\n", val);

	mutex_lock(&data->device_lock);
		
	if(APDS990x_DEBUG_REG_INFO & apds990x_debug_mask){
		enable_value = i2c_smbus_read_byte_data(client, CMD_BYTE|APDS990x_ENABLE_REG);
	
		APDS990xD("[before] APDS990x_ENABLE_REG =  %x\n", enable_value);
		APDS990xD("[before] data->enable_als_sensor ( %x), data->enable_ps_sensor (%x)\n", data->enable_als_sensor, data->enable_ps_sensor );
	}
	
	if(val == 1) {
		//turn on p sensor
		if (data->enable_ps_sensor==0) {

			data->enable_ps_sensor= 1;

			apds990x_set_enable_locked(client,0); /* Power Off */
			apds990x_set_atime_locked(client, 0xED/*0xdb*/); /* 100.64ms */
			apds990x_set_ptime_locked(client, 0xFF); /* 2.72ms */

			//gooni.shim@lge.com 03-AUG-2011 Proximity & ALS Sensor tunning for Rev1.1
			if(hw_rev_num <= LGE_REV_F)
				apds990x_set_ppcount_locked(client, 0xFA);	/* 250-pulse */
			else if(hw_rev_num == LGE_REV_10)
				apds990x_set_ppcount_locked(client, 0x1E);	/* 30 pulse(4cm) */
			else if(hw_rev_num >= LGE_REV_11)
				apds990x_set_ppcount_locked(client, 0x23);	/* 35 pulse(4cm)...gooni.shim@lge.com 28-OCT-2011*/

			//gooni.shim@lge.com 23-SEP-2011 Proximity & ALS Sensor tunning for Rev1.1_final
			if(hw_rev_num <= LGE_REV_10)
				apds990x_set_control_locked(client, 0xE0); /* 12.5mA*/
			else if(hw_rev_num >= LGE_REV_11)
				apds990x_set_control_locked(client, 0x60); /* 50mA*/
			
			apds990x_set_pilt_locked(client, 1023);	// to force first Near-to-Far interrupt
			apds990x_set_piht_locked(client, 0);
			
			data->ps_threshold = APDS990x_PS_DETECTION_THRESHOLD;
			data->ps_hysteresis_threshold = APDS990x_PS_HSYTERESIS_THRESHOLD;
			data->ps_detection = 1;			// we are forcing Near-to-Far interrupt, so this is defaulted to 1
			data->ps_detection_for_atcmd = 0;

			if(APDS990x_DEBUG_AT_COMMAND & apds990x_debug_mask)
				APDS990xD("case1 data->ps_detection_for_atcmd = %d\n", data->ps_detection_for_atcmd);
			
			//gooni.shim@lge.com 20-JUN-2011
			/*apds990x_set_ailt_locked( client, 0);
			apds990x_set_aiht_locked( client, (95*(1024*(256-data->atime)))/100);*/
			apds990x_set_ailt_locked(client, (100*(1024*(256-data->atime)))/100);
			apds990x_set_aiht_locked(client, 0);
			
			apds990x_set_pers_locked(client, 0x33); /* 3 persistence */
			
		#if SUPPORT_ALS_POLLING
			if (data->enable_als_sensor==0) {

				/* we need this polling timer routine for sunlight canellation */
				spin_lock_irqsave(&data->wq_lock, flags);
			
				/*
				 * If work is already scheduled then subsequent schedules will not
				 * change the scheduled time that's why we have to cancel it first.
				 */
				__cancel_delayed_work(&data->als_dwork);
				schedule_delayed_work(&data->als_dwork, msecs_to_jiffies(data->als_poll_delay));	// 100ms
			
				spin_unlock_irqrestore(&data->wq_lock, flags);
			}
		#endif

			apds990x_set_enable_locked(client, 0x3F);	 /* enable PS and ALS interrupt and WAIT */
		}
	} 
	else {
		//turn off p sensor - kk 25 Apr 2011 we can't turn off the entire sensor, the light sensor may be needed by HAL
		data->enable_ps_sensor = 0;
		if (data->enable_als_sensor) {
			
			// reconfigute light sensor setting		
			apds990x_set_enable_locked(client,0); /* Power Off */
			
			apds990x_set_atime_locked(client, data->als_atime);  /* previous als poll delay */
			
			apds990x_set_ailt_locked(client, 0xFFFF);	// to force first ALS interrupt
			apds990x_set_aiht_locked(client, 0);		// in order get current ALS reading
			
			//gooni.shim@lge.com 03-AUG-2011 Proximity & ALS Sensor tunning for Rev1.1
			//gooni.shim@lge.com 23-SEP-2011 Proximity & ALS Sensor tunning for Rev1.1_final
			if(hw_rev_num <= LGE_REV_10)
				apds990x_set_control_locked(client, 0xE0); /* 12.5mA*/
			else if(hw_rev_num >= LGE_REV_11)
				apds990x_set_control_locked(client, 0x60); /* 50mA*/
			//apds990x_set_control_locked(client, 0xE0); /* 12.5mA*/
			
			apds990x_set_pers_locked(client, 0x33); /* 3 persistence */

			apds990x_set_enable_locked(client, 0x1B);	 /* only enable light sensor and WAIT */
		
		#if SUPPORT_ALS_POLLING	
			spin_lock_irqsave(&data->wq_lock, flags);
			
			/*
			 * If work is already scheduled then subsequent schedules will not
			 * change the scheduled time that's why we have to cancel it first.
			 */
			__cancel_delayed_work(&data->als_dwork);
			schedule_delayed_work(&data->als_dwork, msecs_to_jiffies(data->als_poll_delay));	// 100ms
			
			spin_unlock_irqrestore(&data->wq_lock, flags);
		#endif
		}
		else {
			apds990x_set_enable_locked(client, 0);

		#if SUPPORT_ALS_POLLING
			spin_lock_irqsave(&data->wq_lock, flags);
			
			/*
			 * If work is already scheduled then subsequent schedules will not
			 * change the scheduled time that's why we have to cancel it first.
			 */
			__cancel_delayed_work(&data->als_dwork);
		
			spin_unlock_irqrestore(&data->wq_lock, flags);
		#endif
		}
	}

	if(APDS990x_DEBUG_REG_INFO & apds990x_debug_mask){
		enable_value  = 0;
		enable_value = i2c_smbus_read_byte_data(client, CMD_BYTE|APDS990x_ENABLE_REG);
	
		APDS990xD("[after] APDS990x_ENABLE_REG =  %x\n", enable_value);
		APDS990xD("[after] data->enable_als_sensor ( %x), data->enable_ps_sensor (%x)\n", data->enable_als_sensor, data->enable_ps_sensor );
	}

	mutex_unlock(&data->device_lock);
	
	return count;
}
static DEVICE_ATTR(enable_ps_sensor, S_IRUGO | S_IWUSR | S_IWGRP, apds990x_show_enable_ps_sensor, apds990x_store_enable_ps_sensor);

static ssize_t apds990x_show_enable_als_sensor(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds990x_data *data = i2c_get_clientdata(client);
	
	return sprintf(buf, "%d\n", data->enable_als_sensor);
}

static ssize_t apds990x_store_enable_als_sensor(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = NULL;
	struct apds990x_data *data = NULL;
	unsigned long val = simple_strtoul(buf, NULL, 10);
#if SUPPORT_ALS_POLLING
	unsigned long flags;
#endif
	int hw_rev_num = lge_get_hw_rev();
	int enable_value = 0;
	
	client = to_i2c_client(dev);
	data = i2c_get_clientdata(client);

	// Null pointer check.
	if(!client)
		return count;
	if(!data)
		return count;

	if(APDS990x_DEBUG_FUNC_TRACE & apds990x_debug_mask)
		APDS990xD("*** start ***\n");
	
	if ((val != 0) && (val != 1))
	{
		if(APDS990x_DEBUG_ERR_CHECK & apds990x_debug_mask)
			APDS990xE("store unvalid value = %lx\n", val);
		return count;
	}
	
	if(APDS990x_DEBUG_DEV_STATUS & apds990x_debug_mask)
		APDS990xD("enable als sensor ( %lx)\n", val);

	mutex_lock(&data->device_lock);
	
	if(APDS990x_DEBUG_REG_INFO & apds990x_debug_mask){
		enable_value = i2c_smbus_read_byte_data(client, CMD_BYTE|APDS990x_ENABLE_REG);
	
		APDS990xD("[before] APDS990x_ENABLE_REG =  %x\n", enable_value);
		APDS990xD("[before] data->enable_als_sensor ( %x), data->enable_ps_sensor (%x)\n", data->enable_als_sensor, data->enable_ps_sensor );
	}
	
	if(val == 1) {
		//turn on light  sensor
		if (data->enable_als_sensor==0) {

			data->enable_als_sensor = 1;

			apds990x_set_enable_locked(client,0); /* Power Off */
		
			apds990x_set_atime_locked(client, data->als_atime);  
		
			apds990x_set_ailt_locked(client, 0xFFFF);	// to force first ALS interrupt
			apds990x_set_aiht_locked(client, 0);		// in order get current ALS reading
		
			//gooni.shim@lge.com 03-AUG-2011 Proximity & ALS Sensor tunning for Rev1.1
			//gooni.shim@lge.com 23-SEP-2011 Proximity & ALS Sensor tunning for Rev1.1_final
			if(hw_rev_num <= LGE_REV_10)
				apds990x_set_control_locked(client, 0xE0); /* 12.5mA*/
			else if(hw_rev_num >= LGE_REV_11)
				apds990x_set_control_locked(client, 0x60); /* 50mA*/
			//apds990x_set_control_locked(client, 0xE0); /* 12.5mA*/
			
			apds990x_set_pers_locked(client, 0x33); /* 3 persistence */
		
			if (data->enable_ps_sensor) {
				//gooni.shim@lge.com 20-JUN-2011
				/*apds990x_set_ailt_locked( client, 0);
				apds990x_set_aiht_locked( client, (95*(1024*(256-data->atime)))/100);*/
				apds990x_set_ailt_locked(client, (100*(1024*(256-data->atime)))/100);
				apds990x_set_aiht_locked(client, 0);

				apds990x_set_enable_locked(client, 0x3F);	 /* if prox sensor was activated previously */
			}
			else {
				apds990x_set_enable_locked(client, 0x1B);	 /* only enable light sensor and WAIT*/
			}
		
		#if SUPPORT_ALS_POLLING	
			spin_lock_irqsave(&data->wq_lock, flags);
		
			/*
			 * If work is already scheduled then subsequent schedules will not
			 * change the scheduled time that's why we have to cancel it first.
			 */
			__cancel_delayed_work(&data->als_dwork);
			schedule_delayed_work(&data->als_dwork, msecs_to_jiffies(data->als_poll_delay));

			spin_unlock_irqrestore(&data->wq_lock, flags);
		#endif
		}
	}
	else {
		// turn off light sensor
		// what if the p sensor is active?
		data->enable_als_sensor = 0;
		if (data->enable_ps_sensor) {
			apds990x_set_enable_locked(client,0); /* Power Off */
			apds990x_set_atime_locked(client, 0xED/*0xdb*/);  /* 100.64ms */
			apds990x_set_ptime_locked(client, 0xFF); /* 2.72ms */
			
			//gooni.shim@lge.com 03-AUG-2011 Proximity & ALS Sensor tunning for Rev1.1
			if(hw_rev_num <= LGE_REV_F)
				apds990x_set_ppcount_locked(client, 0xFA);	/* 250-pulse */
			else if(hw_rev_num == LGE_REV_10)
				apds990x_set_ppcount_locked(client, 0x1E);	/* 30 pulse(4cm) */
			else if(hw_rev_num >= LGE_REV_11)
				apds990x_set_ppcount_locked(client, 0X23);	/* 35 pulse(4cm)...gooni.shim@lge.com 28-OCT-2011*/

			//gooni.shim@lge.com 23-SEP-2011 Proximity & ALS Sensor tunning for Rev1.1_final
			if(hw_rev_num <= LGE_REV_10)
				apds990x_set_control_locked(client, 0xE0); /* 12.5mA*/
			else if(hw_rev_num >= LGE_REV_11)
				apds990x_set_control_locked(client, 0x60); /* 50mA*/

			
			//gooni.shim@lge.com 20-JUN-2011
			/*apds990x_set_ailt_locked( client, 0);
			apds990x_set_aiht_locked( client, (95*(1024*(256-data->atime)))/100);*/
			apds990x_set_ailt_locked(client, (100*(1024*(256-data->atime)))/100);
			apds990x_set_aiht_locked( client, 0);
			
			apds990x_set_pers_locked(client, 0x33); /* 3 persistence */

			apds990x_set_enable_locked(client, 0x3F);	 /* enable als + prox sensor with interrupt */			
		}
		else {
			apds990x_set_enable_locked(client, 0);
		}
		
	#if SUPPORT_ALS_POLLING
		spin_lock_irqsave(&data->wq_lock, flags);
		
		/*
		 * If work is already scheduled then subsequent schedules will not
		 * change the scheduled time that's why we have to cancel it first.
		 */
		__cancel_delayed_work(&data->als_dwork);
		
		spin_unlock_irqrestore(&data->wq_lock, flags);
	#endif
	}

	if(APDS990x_DEBUG_REG_INFO & apds990x_debug_mask){
		enable_value = 0;
		enable_value = i2c_smbus_read_byte_data(client, CMD_BYTE|APDS990x_ENABLE_REG);
	
		APDS990xD("[after] APDS990x_ENABLE_REG =  %x\n", enable_value);
		APDS990xD("[after] data->enable_als_sensor ( %x), data->enable_ps_sensor (%x)\n", data->enable_als_sensor, data->enable_ps_sensor );
	}

	mutex_unlock(&data->device_lock);
	
	if(APDS990x_DEBUG_FUNC_TRACE & apds990x_debug_mask)
		APDS990xD("*** end ***\n");
	
	return count;
}
static DEVICE_ATTR(enable_als_sensor, S_IRUGO | S_IWUSR | S_IWGRP, apds990x_show_enable_als_sensor, apds990x_store_enable_als_sensor);

static ssize_t apds990x_show_als_poll_delay(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds990x_data *data = i2c_get_clientdata(client);
	
	return sprintf(buf, "%d\n", data->als_poll_delay*1000);	// return in micro-second
}

static ssize_t apds990x_store_als_poll_delay(struct device *dev,
					struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds990x_data *data = i2c_get_clientdata(client);
	unsigned long val = simple_strtoul(buf, NULL, 10);
	int ret;
	int poll_delay=0;
#if SUPPORT_ALS_POLLING
	unsigned long flags;
#endif
	
	if (val<5000)
		val = 5000;	// minimum 5ms

	mutex_lock(&data->device_lock);
	
	data->als_poll_delay = val/1000;	// convert us => ms
	
	poll_delay = 256 - (val/2720);	// the minimum is 2.72ms = 2720 us, maximum is 696.32ms
	if (poll_delay >= 256)
		data->als_atime = 255;
	else if (poll_delay < 0)
		data->als_atime = 0;
	else
		data->als_atime = poll_delay;
	
	ret = apds990x_set_atime_locked(client, data->als_atime);

	mutex_unlock(&data->device_lock);
	
	if (ret < 0)
		return ret;

#if SUPPORT_ALS_POLLING
	/* we need this polling timer routine for sunlight canellation */
	spin_lock_irqsave(&data->wq_lock, flags);
		
	/*
	 * If work is already scheduled then subsequent schedules will not
	 * change the scheduled time that's why we have to cancel it first.
	 */
	__cancel_delayed_work(&data->als_dwork);
	schedule_delayed_work(&data->als_dwork, msecs_to_jiffies(data->als_poll_delay));	// 100ms

	spin_unlock_irqrestore(&data->wq_lock, flags);
#endif
	
	return count;
}
static DEVICE_ATTR(als_poll_delay, S_IRUGO | S_IWUSR | S_IWGRP, apds990x_show_als_poll_delay, apds990x_store_als_poll_delay);

static ssize_t apds990x_show_als_lux_value(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds990x_data *data = i2c_get_clientdata(client);
	int cdata, irdata;
	int luxValue=0;
	
	// we want all data to be from the same measurement. So better make sure
	// we have no one else use the bus inbetween.
	mutex_lock(&data->device_lock);

	mdelay(100);
	
	cdata = i2c_smbus_read_word_data(client, CMD_WORD|APDS990x_CDATAL_REG);
	irdata = i2c_smbus_read_word_data(client, CMD_WORD|APDS990x_IRDATAL_REG);

	luxValue = LuxCalculation_locked(client, cdata, irdata);

	mutex_unlock(&data->device_lock);

	luxValue = luxValue>0 ? luxValue : 0;
	luxValue = luxValue<10000 ? luxValue : 10000;

	return sprintf(buf, "%d\n", luxValue);
}
static DEVICE_ATTR(als_lux_value, S_IRUGO, apds990x_show_als_lux_value, NULL);

static ssize_t apds990x_show_ps_detection(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct apds990x_data *data = i2c_get_clientdata(client);

    return sprintf(buf, "%x\n", data->ps_detection);    
}
static DEVICE_ATTR(ps_detection, S_IRUGO, apds990x_show_ps_detection, NULL);

static ssize_t apds990x_show_ps_detection_for_atcmd(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct apds990x_data *data = i2c_get_clientdata(client);
    int pdata;
    pdata = data->ps_detection_for_atcmd;

	if(APDS990x_DEBUG_AT_COMMAND & apds990x_debug_mask)
		APDS990xD("data->ps_detection_for_atcmd= %d, !pdata = %d\n", pdata, !pdata);

    return sprintf(buf, "%d\n", !data->ps_detection_for_atcmd);
}
static DEVICE_ATTR(ps_detection_for_atcmd, S_IRUGO, apds990x_show_ps_detection_for_atcmd, NULL);

static ssize_t apds990x_show_enable(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds990x_data *data = i2c_get_clientdata(client);
	int enable_value = 0;

	// Make sure, we don't read a transitional value while the worker thread is
	// in action
	mutex_lock(&data->device_lock);
	enable_value = i2c_smbus_read_byte_data(client, CMD_BYTE|APDS990x_ENABLE_REG);
	mutex_unlock(&data->device_lock);

	return sprintf(buf, "%x\n", enable_value);
}

static ssize_t apds990x_store_enable(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds990x_data *data = i2c_get_clientdata(client);
	unsigned long val = simple_strtoul(buf, NULL, 10);
	int ret = 0;

	mutex_lock(&data->device_lock);
	ret = apds990x_set_enable_locked(client, val);
	mutex_unlock(&data->device_lock);
	
	if (ret < 0)
		return ret;

	return count;
}

static DEVICE_ATTR(enable, S_IRUGO | S_IWUSR | S_IWGRP, apds990x_show_enable, apds990x_store_enable);

static ssize_t apds990x_show_pdata(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	int pdata = 0;

	pdata = i2c_smbus_read_word_data(client, CMD_WORD | APDS990x_PDATAL_REG);
	
	return sprintf(buf, "%x\n", pdata);
}
static DEVICE_ATTR(pdata, S_IRUGO, apds990x_show_pdata, NULL);

static ssize_t apds990x_show_cdata(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	int cdata = 0;

	cdata = i2c_smbus_read_word_data(client, CMD_WORD | APDS990x_CDATAL_REG);
	
	return sprintf(buf, "%x\n", cdata);
}
static DEVICE_ATTR(cdata, S_IRUGO, apds990x_show_cdata, NULL);

static ssize_t apds990x_show_irdata(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	int irdata = 0;

	irdata = i2c_smbus_read_word_data(client, CMD_WORD|APDS990x_IRDATAL_REG);

	return sprintf(buf, "%x\n", irdata);
}
static DEVICE_ATTR(irdata, S_IRUGO, apds990x_show_irdata, NULL);

static ssize_t apds990x_show_ppcount(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct apds990x_data *data = i2c_get_clientdata(client);

    return sprintf(buf, "%x\n", data->ppcount);
}

static ssize_t apds990x_store_ppcount(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct i2c_client *client = to_i2c_client(dev);
	struct apds990x_data *data = i2c_get_clientdata(client);
    unsigned long val = simple_strtoul(buf, NULL, 10);
    int ret = 0;

	mutex_lock(&data->device_lock);
    ret = apds990x_set_ppcount_locked(client, val);
	mutex_unlock(&data->device_lock);
	
    if (ret < 0)
        return ret;

    return count;
}
static DEVICE_ATTR(ppcount,  S_IRUGO | S_IWUSR | S_IWGRP, apds990x_show_ppcount, apds990x_store_ppcount);

static ssize_t apds990x_show_control(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct apds990x_data *data = i2c_get_clientdata(client);
    int control = 0;

	// Make sure, we don't read a transitional value while the worker thread is
	// in action
	mutex_lock(&data->device_lock);
    control = i2c_smbus_read_byte_data(client, CMD_BYTE|APDS990x_CONTROL_REG);
	mutex_unlock(&data->device_lock);

    //return sprintf(buf, "%x\n", data->control);
    return sprintf(buf, "reg(%x),buf(%x)\n", control, data->control);
}

static ssize_t apds990x_store_control(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct i2c_client *client = to_i2c_client(dev);
	struct apds990x_data *data = i2c_get_clientdata(client);
    unsigned long val = simple_strtoul(buf, NULL, 10);
    int ret = 0;

	mutex_lock(&data->device_lock);
    ret = apds990x_set_control_locked(client, val);
	mutex_unlock(&data->device_lock);
	
    if (ret < 0)
        return ret;

    return count;
}
static DEVICE_ATTR(control,  S_IRUGO | S_IWUSR | S_IWGRP, apds990x_show_control, apds990x_store_control);

static ssize_t apds990x_show_pers(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct apds990x_data *data = i2c_get_clientdata(client);

    return sprintf(buf, "%x\n", data->pers);
}

static ssize_t apds990x_store_pers(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct i2c_client *client = to_i2c_client(dev);
	struct apds990x_data *data = i2c_get_clientdata(client);
    unsigned long val = simple_strtoul(buf, NULL, 10);
    int ret = 0;

	mutex_lock(&data->device_lock);
    ret = apds990x_set_pers_locked(client, val);
	mutex_unlock(&data->device_lock);
	
    if (ret < 0)
        return ret;

    return count;
}
static DEVICE_ATTR(pers, S_IRUGO | S_IWUSR | S_IWGRP, apds990x_show_pers, apds990x_store_pers);

static ssize_t apds990x_show_ps_threshold(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct apds990x_data *data = i2c_get_clientdata(client);

    return sprintf(buf, "%x\n", data->ps_threshold);
}

static ssize_t apds990x_store_ps_threshold(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct i2c_client *client = to_i2c_client(dev);
	struct apds990x_data *data = i2c_get_clientdata(client);
	
    unsigned long val = simple_strtoul(buf, NULL, 10);

    mutex_lock(&data->device_lock);
	data->ps_threshold = val;
	mutex_unlock(&data->device_lock);
	
    return count;
}
static DEVICE_ATTR(ps_threshold, S_IRUGO | S_IWUSR | S_IWGRP, apds990x_show_ps_threshold, apds990x_store_ps_threshold);

static ssize_t apds990x_show_ps_hysteresis_threshold(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct apds990x_data *data = i2c_get_clientdata(client);

    return sprintf(buf, "%x\n", data->ps_hysteresis_threshold);
}

static ssize_t apds990x_store_ps_hysteresis_threshold(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct i2c_client *client = to_i2c_client(dev);
	struct apds990x_data *data = i2c_get_clientdata(client);
	
    unsigned long val = simple_strtoul(buf, NULL, 10);

    mutex_lock(&data->device_lock);
	data->ps_hysteresis_threshold = val;
	mutex_unlock(&data->device_lock);
	
    return count;
}
static DEVICE_ATTR(ps_hysteresis_threshold, S_IRUGO | S_IWUSR | S_IWGRP, apds990x_show_ps_hysteresis_threshold, apds990x_store_ps_hysteresis_threshold);

static ssize_t apds990x_store_command(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{    
	struct i2c_client *client = to_i2c_client(dev);
	struct apds990x_data *data = i2c_get_clientdata(client);
	unsigned long val = simple_strtoul(buf, NULL, 10);
	int ret = 0;

	if (val < 0 || val > 2)
		return -EINVAL;

	mutex_lock(&data->device_lock);
	ret = apds990x_set_command_locked(client, val);
	mutex_unlock(&data->device_lock);
	
	if (ret < 0)
		return ret;

	return count;
}
static DEVICE_ATTR(command, S_IRUGO | S_IWUSR | S_IWGRP, NULL, apds990x_store_command);

static ssize_t apds990x_show_status(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	int status = 0;

	status = i2c_smbus_read_byte_data(client, CMD_BYTE|APDS990x_STATUS_REG);

	return sprintf(buf, "%x\n", status);
}
static DEVICE_ATTR(status, S_IRUGO, apds990x_show_status, NULL);

static ssize_t apds990x_show_debug_mask(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", apds990x_debug_mask);
}

static ssize_t apds990x_store_debug_mask(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    unsigned long val = simple_strtoul(buf, NULL, 10);

    apds990x_debug_mask = val;

    return count;
}
static DEVICE_ATTR(debug_mask,  S_IRUGO | S_IWUSR | S_IWGRP, apds990x_show_debug_mask, apds990x_store_debug_mask);


static struct attribute *apds990x_attributes[] = {
	&dev_attr_enable_ps_sensor.attr,
	&dev_attr_enable_als_sensor.attr,
	&dev_attr_als_poll_delay.attr,
	&dev_attr_als_lux_value.attr,
	&dev_attr_ps_detection.attr,
	&dev_attr_ps_detection_for_atcmd.attr,
	&dev_attr_enable.attr,
	&dev_attr_pdata.attr,
	&dev_attr_cdata.attr,
	&dev_attr_irdata.attr,
	&dev_attr_ppcount.attr,
	&dev_attr_control.attr,
	&dev_attr_pers.attr,
	&dev_attr_ps_threshold.attr,
	&dev_attr_ps_hysteresis_threshold.attr,
	&dev_attr_command.attr,
	&dev_attr_status.attr,
	&dev_attr_debug_mask.attr,
	NULL
};


static const struct attribute_group apds990x_attr_group = {
	.attrs = apds990x_attributes,
};

/*
 * Initialization function
 */

static int apds990x_init_client(struct i2c_client *client)
{
	struct apds990x_data *data = i2c_get_clientdata(client);
	int err;
	int id;
	int hw_rev_num = lge_get_hw_rev();
	
	err = apds990x_set_enable_locked(client, 0);

	if (err < 0)
		return err;
	
	id = i2c_smbus_read_byte_data(client, CMD_BYTE|APDS990x_ID_REG);
	if (id == 0x20) {
		if (APDS990x_DEBUG_REG_INFO & apds990x_debug_mask)
			APDS990xD("APDS-9901\n");
	}
	else if (id == 0x29) {
		if (APDS990x_DEBUG_REG_INFO & apds990x_debug_mask)
			APDS990xD("APDS-990x\n");
	}
	else {
		if (APDS990x_DEBUG_ERR_CHECK & apds990x_debug_mask)
			APDS990xE("Neither APDS-9901 nor APDS-990x\n");
		return -EIO;
	}

	apds990x_set_atime_locked(client, data->als_atime);	// 100.64ms ALS integration time
	apds990x_set_ptime_locked(client, 0xFF);	// 2.72ms Prox integration time
	apds990x_set_wtime_locked(client, 0xFF);	// 2.72ms Wait time

	//gooni.shim@lge.com 03-AUG-2011 Proximity & ALS Sensor tunning for Rev1.1
	if(hw_rev_num <= LGE_REV_F)
		apds990x_set_ppcount_locked(client, 0xFA);	/* 250-pulse */
	else if(hw_rev_num == LGE_REV_10)
		apds990x_set_ppcount_locked(client, 0x1E);	/* 30 pulse(4cm)*/
	else if(hw_rev_num >= LGE_REV_11)
		apds990x_set_ppcount_locked(client, 0x23);	/* 35 pulse(4cm)...gooni.shim@lge.com 28-OCT-2011*/
	apds990x_set_config_locked(client, 0);		// no long wait

	//gooni.shim@lge.com 23-SEP-2011 Proximity & ALS Sensor tunning for Rev1.1_final
	if(hw_rev_num <= LGE_REV_10)
		apds990x_set_control_locked(client, 0xE0); /* 12.5mA*/
	else if(hw_rev_num >= LGE_REV_11)
		apds990x_set_control_locked(client, 0x60); /* 50mA*/
	
	apds990x_set_pilt_locked(client, 1023);	// to force first Near-to-Far interrupt
	apds990x_set_piht_locked(client, 0);

	data->ps_threshold = APDS990x_PS_DETECTION_THRESHOLD;
	data->ps_hysteresis_threshold = APDS990x_PS_HSYTERESIS_THRESHOLD;
	data->ps_detection = 1;			// we are forcing Near-to-Far interrupt, so this is defaulted to 1
	data->ps_detection_for_atcmd = 0;
	if (APDS990x_DEBUG_AT_COMMAND & apds990x_debug_mask)
		APDS990xD("data->ps_detection_for_atcmd= %d\n", data->ps_detection_for_atcmd);
	
	apds990x_set_ailt_locked(client, 0xFFFF);	// to force first ALS interrupt
	apds990x_set_aiht_locked(client, 0);		// in order get current ALS reading

	apds990x_set_pers_locked(client, 0x33);	// 3 consecutive Interrupt persistence
	
	// sensor is in disabled mode but all the configurations are preset

	return 0;
}

/*
 * I2C init/probing/exit functions
 */

static struct i2c_driver apds990x_driver;
static int __devinit apds990x_probe(struct i2c_client *client,
				   const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct apds990x_data *data;
	int err = 0;
	int ret = 0;
	
#if SUPPORT_FOLDER_TEST
	struct apds9900_platform_data* pboarddata = NULL;
#else
	struct apds9900_platform_data* pboarddata = client->dev.platform_data;
#endif
	
	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE)) {
		err = -EIO;
		goto exit;
	}

	data = kzalloc(sizeof(struct apds990x_data), GFP_KERNEL);
	if (!data) {
		err = -ENOMEM;
		goto exit;
	}
	
	data->client = client;
	i2c_set_clientdata(client, data);
#if SUPPORT_FOLDER_TEST
	apds990x_i2c_client = client;
#endif
	data->enable = 0;	/* default mode is standard */
	data->ps_threshold = 0;
	data->ps_hysteresis_threshold = 0;
	data->ps_detection = 0;	/* default to no detection */
	data->ps_detection_for_atcmd = 0;
	if (APDS990x_DEBUG_AT_COMMAND & apds990x_debug_mask)
		APDS990xD("data->ps_detection_for_atcmd= %d\n", data->ps_detection_for_atcmd);
	data->enable_als_sensor = 0;	// default to 0
	data->enable_ps_sensor = 0;	// default to 0
	data->als_poll_delay = 100;	// default to 100ms
	data->als_atime	= 0xED/*0xdb*/;			// work in conjuction with als_poll_delay

#if SUPPORT_FOLDER_TEST
	pboarddata = apds990x_i2c_client->dev.platform_data;
#endif
	data->irq = gpio_to_irq(pboarddata->irq_num);
	
	mutex_init(&data->device_lock);

	spin_lock_init(&data->wq_lock);
	
	pboarddata->power(1);
	
	mdelay(10);//udelay(60);
	
	INIT_DELAYED_WORK(&data->dwork, apds990x_work_handler);
#if SUPPORT_ALS_POLLING
	INIT_DELAYED_WORK(&data->als_dwork, apds990x_als_polling_work_handler); 
#endif
	if (APDS990x_DEBUG_INTR_INFO & apds990x_debug_mask)
		APDS990xD("interrupt is hooked.\n");
	
	/* Initialize the APDS990x chip */
	err = apds990x_init_client(client);
	if (err)
		goto exit_kfree;

	/* Register to Input Device */
	data->input_dev_als = input_allocate_device();
	if (!data->input_dev_als) {
		err = -ENOMEM;

		if (APDS990x_DEBUG_ERR_CHECK & apds990x_debug_mask)
			APDS990xE("Failed to allocate input device als.\n");
		goto exit_free_irq;
	}

	data->input_dev_ps = input_allocate_device();
	if (!data->input_dev_ps) {
		err = -ENOMEM;

		if (APDS990x_DEBUG_ERR_CHECK & apds990x_debug_mask)
			APDS990xE("Failed to allocate input device ps.\n");
		goto exit_free_dev_als;
	}
	
	set_bit(EV_ABS, data->input_dev_als->evbit);
	set_bit(EV_ABS, data->input_dev_ps->evbit);

	input_set_abs_params(data->input_dev_als, ABS_MISC, 0, 10000, 0, 0);
	input_set_abs_params(data->input_dev_ps, ABS_DISTANCE, 0, 1, 0, 0);

	data->input_dev_als->name = "Avago light sensor";
	data->input_dev_ps->name = "Avago proximity sensor";

	if (request_irq(/*APDS990x_INT*/data->irq, apds990x_interrupt, IRQF_DISABLED|IRQ_TYPE_EDGE_FALLING,
		APDS990x_DRV_NAME, (void *)client)) {
		dev_info(&client->dev, "apds990x.c: Could not allocate APDS990x_INT !\n");
	
		goto exit_kfree;
	}
	
	err = input_register_device(data->input_dev_als);
	if (err) {
		err = -ENOMEM;

		if (APDS990x_DEBUG_ERR_CHECK & apds990x_debug_mask)
			APDS990xE("Unable to register input device als: %s\n", data->input_dev_als->name);
		goto exit_free_dev_ps;
	}

	err = input_register_device(data->input_dev_ps);
	if (err) {
		err = -ENOMEM;

		if (APDS990x_DEBUG_ERR_CHECK & apds990x_debug_mask)
			APDS990xE("Unable to register input device ps: %s\n", data->input_dev_ps->name);
		goto exit_unregister_dev_als;
	}

	ret = irq_set_irq_wake(data->irq, 1); /* LGE_UPDATE kideok.kim@lge.com 20111216 kernel 3.0.x Adaptation */
	if (ret)
		irq_set_irq_wake(data->irq, 0); /* LGE_UPDATE kideok.kim@lge.com 20111216 kernel 3.0.x Adaptation */
		
	/* Register sysfs hooks */
	err = sysfs_create_group(&client->dev.kobj, &apds990x_attr_group);
	if (err)
		goto exit_unregister_dev_ps;

#if SUPPORT_FOLDER_TEST
#if defined(CONFIG_HAS_EARLYSUSPEND)
	apds990x_sensor_early_suspend.suspend = apds990x_early_suspend;
	apds990x_sensor_early_suspend.resume = apds990x_late_resume;
	register_early_suspend(&apds990x_sensor_early_suspend);
#endif
#endif

	dev_info(&client->dev, "enable = %s\n",	data->enable ? "1" : "0");

	dev_info(&client->dev, "interrupt is hooked\n");

	dev_info(&client->dev, "support ver. %s enabled\n", DRIVER_VERSION);

	return 0;

exit_unregister_dev_ps:
	input_unregister_device(data->input_dev_ps);	
exit_unregister_dev_als:
	input_unregister_device(data->input_dev_als);
exit_free_dev_ps:
	input_free_device(data->input_dev_ps);
exit_free_dev_als:
	input_free_device(data->input_dev_als);
exit_free_irq:
	free_irq(/*APDS990x_INT*/data->irq, client);	
exit_kfree:
	kfree(data);
exit:
	return err;
}

static int __devexit apds990x_remove(struct i2c_client *client)
{
	struct apds990x_data *data = i2c_get_clientdata(client);
	
	input_unregister_device(data->input_dev_als);
	input_unregister_device(data->input_dev_ps);
	
	input_free_device(data->input_dev_als);
	input_free_device(data->input_dev_ps);

	irq_set_irq_wake(data->irq, 0); /* LGE_UPDATE kideok.kim@lge.com 20111216 kernel 3.0.x Adaptation */
	
	free_irq(/*APDS990x_INT*/data->irq, client);

	sysfs_remove_group(&client->dev.kobj, &apds990x_attr_group);

	/* Power down the device */
	apds990x_set_enable_locked(client, 0);

#if SUPPORT_FOLDER_TEST
	apds990x_i2c_client = NULL;
#endif

	kfree(data);

#if SUPPORT_FOLDER_TEST
#if defined(CONFIG_HAS_EARLYSUSPEND)
	unregister_early_suspend(&apds990x_sensor_early_suspend);
#endif
#endif

	return 0;
}

#if SUPPORT_FOLDER_TEST
#if defined(CONFIG_HAS_EARLYSUSPEND)
#define ADPS_HAS_EARLY_SUSPEND
static void apds990x_early_suspend(struct early_suspend *h)
{
	struct apds990x_data *pdev = i2c_get_clientdata(apds990x_i2c_client);
	struct apds9900_platform_data *pboarddata = pdev->client->dev.platform_data;

	disable_irq(pdev->irq);

	cancel_delayed_work_sync(&pdev->dwork);

	mutex_lock(&pdev->device_lock);
	apds990x_set_enable_locked(apds990x_i2c_client, 0);
	mutex_unlock(&pdev->device_lock);
	
	pboarddata->power(0);
}

static void apds990x_late_resume(struct early_suspend *h)
{
	struct apds990x_data *pdev = i2c_get_clientdata(apds990x_i2c_client);
	struct apds9900_platform_data *pboarddata = pdev->client->dev.platform_data;
	
	pboarddata->power(1);
	udelay(60);

	mutex_lock(&pdev->device_lock);
	apds990x_init_client(apds990x_i2c_client);
	
	enable_irq(pdev->irq);
	
	apds990x_set_enable_locked(apds990x_i2c_client, 0x3F);

	mutex_unlock(&pdev->device_lock);
}
#endif
#endif


#if  defined(CONFIG_PM) && !defined(ADPS_HAS_EARLY_SUSPEND)	//#ifdef CONFIG_PM
static int apds990x_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct apds990x_data *pdev = i2c_get_clientdata(client);
	
	if (APDS990x_DEBUG_FUNC_TRACE & apds990x_debug_mask)
		APDS990xD("apds990x_suspend()\n");

	// Prevent the work queue from accessing the chip while the gpio subsystem
	// and i2c bus are suspending or more likely, while they are not yet resumed.
	disable_irq(pdev->irq);
	flush_delayed_work(&pdev->dwork);
	
	return 0;//apds990x_set_enable_locked(client, 0);
}

static int apds990x_resume(struct i2c_client *client)
{
	struct apds990x_data *pdev = i2c_get_clientdata(client);
	
	if (APDS990x_DEBUG_FUNC_TRACE & apds990x_debug_mask)
		APDS990xD("apds990x_resume()\n");

	enable_irq(pdev->irq);
	
	return 0;//apds990x_set_enable_locked(client, 0);
}

#else

#define apds990x_suspend	NULL
#define apds990x_resume		NULL

#endif /* CONFIG_PM */

static const struct i2c_device_id apds990x_id[] = {
	{ /*"apds990x"*/"apds9900", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, apds990x_id);

static struct i2c_driver apds990x_driver = {
	.driver = {
		.name	= APDS990x_DRV_NAME,
		.owner	= THIS_MODULE,
	},
	.suspend = apds990x_suspend,
	.resume	= apds990x_resume,
	.probe	= apds990x_probe,
	.remove	= __devexit_p(apds990x_remove),
	.id_table = apds990x_id,
};

static int __init apds990x_init(void)
{
	return i2c_add_driver(&apds990x_driver);
}

static void __exit apds990x_exit(void)
{
	i2c_del_driver(&apds990x_driver);
}

MODULE_AUTHOR("Lee Kai Koon <kai-koon.lee@avagotech.com>");
MODULE_DESCRIPTION("APDS990x ambient light + proximity sensor driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);

module_init(apds990x_init);
module_exit(apds990x_exit);


