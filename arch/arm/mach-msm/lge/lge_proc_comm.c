/*
 * arch/arm/mach-msm/lge/lge_proc_comm.c
 *
 * Copyright (C) 2010 LGE, Inc
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
#include <linux/device.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <mach/board_lge.h>
#include "../proc_comm.h"

/* 2011-01-26 by baborobo@lge.com
 * match with
 * /modem/AMSS/products/7x30/modem/rfa/pmic/common/app/src/pmapp_pcil.c
 */
enum {
	CUSTOMER_CMD1_SET_SPECIAL_CLOCK0,
	CUSTOMER_CMD1_GET_BOOT_ON_TYPE,
	CUSTOMER_CMD1_HW_RESET_IMM,
	CUSTOMER_CMD1_GET_QEM,
	CUSTOMER_CMD1_GET_SIM_STATE,
	CUSTOMER_CMD1_GET_FRSTSTATUS,
	CUSTOMER_CMD1_SET_FRSTSTATUS,
	CUSTOMER_CMD1_HW_POWEROFF_IMM
};

unsigned lge_get_pif_info(void)
{
	int err;
	unsigned pif_value = -1;
	unsigned cmd_pif = CUSTOMER_CMD1_GET_BOOT_ON_TYPE;

	err = msm_proc_comm(PCOM_CUSTOMER_CMD1, &pif_value, &cmd_pif);
	if (err < 0) {
		pr_err("%s: msm_proc_comm(PCOM_CUSTOMER_CMD1) failed\n",
		       __func__);
		return err;
	}

	/* 2011-01-26 by baborobo@lge.com
	 * 0 : Boot-on by PMIC VBUS
	       ( it is connected the USB Cable or the TA Charger )
	 * 1 : Boot-on by Normal Power long key or reboot
	 * 2 : Boot-on by PIF or PMIC remote-power-on Port
	 */
	return pif_value;
}

EXPORT_SYMBOL(lge_get_pif_info);

unsigned lge_get_nv_qem(void)
{
	int err;
	unsigned ret = 0;
	unsigned cmd = CUSTOMER_CMD1_GET_QEM;
	
	err = msm_proc_comm(PCOM_CUSTOMER_CMD1, &ret, &cmd);
	if (err < 0) {
		pr_err("%s: msm_proc_comm(PCOM_CUSTOMER_CMD1) failed. cmd(%d)\n",
		       __func__, cmd);
		return err;
	}

	return ret;
}
EXPORT_SYMBOL(lge_get_nv_qem);

unsigned lge_get_nv_frststatus(void)
{
	int err;
	unsigned ret = 0;
	unsigned cmd = CUSTOMER_CMD1_GET_FRSTSTATUS;
	
	err = msm_proc_comm(PCOM_CUSTOMER_CMD1, &ret, &cmd);
	if (err < 0) {
		pr_err("%s: msm_proc_comm(PCOM_CUSTOMER_CMD1) failed. cmd(%d)\n",
		       __func__, cmd);
	}

	return ret;
}
EXPORT_SYMBOL(lge_get_nv_frststatus);


void lge_set_nv_frststatus(unsigned char flag)
{
	int err;
	static unsigned req;
	unsigned cmd = CUSTOMER_CMD1_SET_FRSTSTATUS;
	
	req = (unsigned)flag;

	err = msm_proc_comm(PCOM_CUSTOMER_CMD1, &req, &cmd);
	if (err < 0) {
		pr_err("%s: msm_proc_comm(PCOM_CUSTOMER_CMD1) failed. cmd(%d)\n",
		       __func__, cmd);
	}
	return;
}
EXPORT_SYMBOL(lge_set_nv_frststatus);


unsigned lge_get_sim_info(void)
{
	int err;
	unsigned value = 0;
	unsigned cmd = CUSTOMER_CMD1_GET_SIM_STATE;

	err = msm_proc_comm(PCOM_CUSTOMER_CMD1, &value, &cmd);
	if (err < 0) {
		pr_err("%s: msm_proc_comm(PCOM_CUSTOMER_CMD1) failed\n",
		       __func__);
		return err;
	}
	return value;
}
EXPORT_SYMBOL(lge_get_sim_info);
