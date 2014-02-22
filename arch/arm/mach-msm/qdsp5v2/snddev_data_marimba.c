/* Copyright (c) 2009-2011, Code Aurora Forum. All rights reserved.
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
 */
#include <linux/platform_device.h>
#include <linux/debugfs.h>
#include <linux/mfd/msm-adie-codec.h>
#include <linux/uaccess.h>
#include <mach/qdsp5v2/snddev_icodec.h>
#include <mach/qdsp5v2/marimba_profile.h>
#include <mach/qdsp5v2/aux_pcm.h>
#include <mach/qdsp5v2/snddev_ecodec.h>
#include <mach/qdsp5v2/audio_dev_ctl.h>
#include <mach/qdsp5v2/snddev_virtual.h>
#include <mach/board.h>
#include <asm/mach-types.h>
#include <mach/gpio.h>
#include <mach/qdsp5v2/snddev_mi2s.h>
#include <mach/qdsp5v2/mi2s.h>
#include <mach/qdsp5v2/audio_acdb_def.h>
//LGE_UPDATE_S	taeho.youn@lge.com
#include <mach/qdsp5v2/lge_tpa2055-amp.h>
//LGE_UPDATE_E	taeho.youn@lge.com

/*BEGIN: 0010359 daniel.kang@lge.com ++ Nov 1st*/
// LG_HW_REV6 Macro is changed to WITHOUT_AUDIENCE
/*END: 0010359 daniel.kang@lge.com ++ Nov 1st*/


/* CONFIG_MACH_LGE_FLIP	ehgrace.kim 10.05.03
	add LGE amp
*/
#include <mach/board_lge.h>

/* define the value for BT_SCO */
#define BT_SCO_PCM_CTL_VAL (PCM_CTL__RPCM_WIDTH__LINEAR_V |\
				PCM_CTL__TPCM_WIDTH__LINEAR_V)
#define BT_SCO_DATA_FORMAT_PADDING (DATA_FORMAT_PADDING_INFO__RPCM_FORMAT_V |\
				DATA_FORMAT_PADDING_INFO__TPCM_FORMAT_V)
#define BT_SCO_AUX_CODEC_INTF   AUX_CODEC_INTF_CTL__PCMINTF_DATA_EN_V

// SRI_START Adding new device 25th Aug,2010 (daniel.kang@lge.com)
// Audience device configuration
#if defined (CONFIG_MACH_LGE_FLIP)
#define HANDSET_PCM_CTL_VAL (PCM_CTL__RPCM_WIDTH__LINEAR_V |\
				PCM_CTL__TPCM_WIDTH__LINEAR_V)
#define HANDSET_DATA_FORMAT_PADDING (DATA_FORMAT_PADDING_INFO__RPCM_FORMAT_V |\
				DATA_FORMAT_PADDING_INFO__TPCM_FORMAT_V)
#define HANDSET_AUX_CODEC_INTF   AUX_CODEC_INTF_CTL__PCMINTF_DATA_EN_V
#endif
// SRI_END Adding new device 25th Aug,2010 (daniel.kang@lge.com)

// srinivas.mittapalli@lge.com 16Sept2010 baikal 0009676
#define LGE_CUST_CAL   // undef this macro after tuning Done 



//LGE_UPDATE_S	taeho.youn@lge.com
#if 0
#ifdef CONFIG_DEBUG_FS
static struct dentry *debugfs_hsed_config;
static void snddev_hsed_config_modify_setting(int type);
static void snddev_hsed_config_restore_setting(void);
#endif
/* srinivas.mittapalli@lge.com 16Sept2010 baikal 0009676 */
#ifdef  LGE_CUST_CAL
int mic_bias0_handset_mic = 1000;
int mic_bias1_handset_mic =1100;
int mic_bias0_aud_hmic = 1200;
int mic_bias1_aud_hmic =1300;
int mic_bias0_spkmic = 1400;
int mic_bias1_spkmic =1500;
int mic_bias0_aud_spkmic = 1600;
int mic_bias1_aud_spkmic=1700;
int mic_bias0_handset_endfire = 1800;
int mic_bias1_handset_endfire =1900;
int mic_bias0_handset_broadside = 2000;
int mic_bias1_handset_broadside =2100;
int mic_bias0_spk_endfire= 2200;
int mic_bias1_spk_endfire =2300;
int mic_bias0_spk_broadside = 2400;
int mic_bias1_spk_broadside =2500;
#endif //LGE_CUST_CAl
/* srinivas.mittapalli@lge.com 16Sept2010 */

/* daniel.kang@lge.com comments ++ */
/* To release static variable (struct adie_codec_action_unit XXXXXXXX) remove static in case of LGE_CUST_CAL */
/* daniel.kang@lge.com comments ++ */

/////////////////////////////////////////////////////////////////////////////////////////
// Handset mono RX (voice and sound)
/////////////////////////////////////////////////////////////////////////////////////////
#ifndef LGE_CUST_CAL
static 
#endif 
struct adie_codec_action_unit iearpiece_48KHz_osr256_actions[] =
	HANDSET_RX_48000_OSR_256;

static struct adie_codec_hwsetting_entry iearpiece_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = iearpiece_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(iearpiece_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile iearpiece_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = iearpiece_settings,
	.setting_sz = ARRAY_SIZE(iearpiece_settings),
};
/* baikal ID 0009676::srinivas.mittapalli@lge.com 11Oct2010 */
#ifndef LGE_CUST_CAL
static 
#endif
struct snddev_icodec_data snddev_iearpiece_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "handset_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HANDSET_SPKR,
	.profile = &iearpiece_profile,
	.channel_mode = 1,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.default_sample_rate = 48000,
	.pamp_on = NULL,
	.pamp_off = NULL,
	.property = SIDE_TONE_MASK,
	.max_voice_rx_vol[VOC_NB_INDEX] = -200,
	.min_voice_rx_vol[VOC_NB_INDEX] = -1700,
	.max_voice_rx_vol[VOC_WB_INDEX] = -200,
	.min_voice_rx_vol[VOC_WB_INDEX] = -1700
};

static struct platform_device msm_iearpiece_device = {
	.name = "snddev_icodec",
	.id = 0,
	.dev = { .platform_data = &snddev_iearpiece_data },
};

/////////////////////////////////////////////////////////////////////////////////////////
// Handset TX Dual Mic with main mic as primary
//              ( QCT default configuration is Single, MIC1 - in Bryce Back mic)
/////////////////////////////////////////////////////////////////////////////////////////

#if defined (CONFIG_MACH_LGE_FLIP)
/////////////////////////
#ifndef LGE_CUST_CAL
static 
#endif
struct adie_codec_action_unit imic_8KHz_osr256_actions[] =
	MIC1_RIGHT_AUX_IN_LEFT_8000_OSR_256;

//TODO: Need to be verified from QCOM.
/*QCOM Dual MIC using 8K config for 16 & 48 K.*/
static struct adie_codec_hwsetting_entry imic_settings[] = {
	{
		.freq_plan = 8000,
		.osr = 256,
		.actions = imic_8KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(imic_8KHz_osr256_actions),
	},/* 8KHz profile can be used for 16Khz */
	{
		.freq_plan = 16000,
		.osr = 256,
		.actions = imic_8KHz_osr256_actions, //imic_16KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(imic_8KHz_osr256_actions), //ARRAY_SIZE(imic_16KHz_osr256_actions),
	},/* 8KHz profile can be used for 48Khz */
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = imic_8KHz_osr256_actions,//imic_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(imic_8KHz_osr256_actions), //ARRAY_SIZE(imic_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile imic_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = imic_settings,
	.setting_sz = ARRAY_SIZE(imic_settings),
};
static enum hsed_controller imic_pmctl_id[] = {PM_HSED_CONTROLLER_0,PM_HSED_CONTROLLER_1};

static struct snddev_icodec_data snddev_imic_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "handset_tx",
	.copp_id = 0,
// daniel.kang@lge.com ++ 3rd Nov
// 0010463: Enable Fluence in handset, speaker TX
	.acdb_id = ACDB_ID_HANDSET_MIC_ENDFIRE,	// ACDB_ID_HANDSET_MIC,
// daniel.kang@lge.com -- 3rd Nov
	.profile = &imic_profile,
	.channel_mode = 2,
	.pmctl_id = imic_pmctl_id,
	.pmctl_id_sz = ARRAY_SIZE(imic_pmctl_id),
	.default_sample_rate = 48000,
/* CONFIG_MACH_LGE_BRYCE	ehgrace.kim 10.05.03
	add LGE amp*/
/* daniel.kang@lge.com ++ */
/* change pamp_on/off name : mic_amp_on controls the mic swith, not turn on amp */
/* MSM_mic_routing_config makes CAM_MIC_EN switch 0 for camcorder recording */
	.pamp_on = &lge_snddev_MSM_mic_route_config,
	.pamp_off = &lge_snddev_mic_route_deconfig,
/*IN Rev C Audience is not required to bypass Mic1. ++ kiran.kanneganti@lge.com 01-Oct-2010 baikal 0009680 */
#if defined (WITHOUT_AUDIENCE)
	.audience_ctrl_ON = NULL,
	.audience_ctrl_OFF = NULL, 
#else
	.audience_ctrl_ON = &lge_audience_ON,
	.audience_ctrl_OFF = &lge_audience_OFF,	
/* -- kiran.kanneganti@lge.com 01-Oct-2010*/
#endif	
// srinivas.mittapalli@lge.com 16Sept2010 baikal 0009676
#ifdef LGE_CUST_CAL
 	.micbias_current = {&mic_bias0_handset_mic,&mic_bias1_handset_mic},
 #else
	.micbias_current = {1700,1700}, // Need to keep the Tuned values
 #endif //LGE_CUST_CAL
// srinivas.mittapalli@lge.com 16Sept2010	
/* daniel.kang@lge.com -- */
};

static struct platform_device msm_imic_device = {
	.name = "snddev_icodec",
	.id = 1,
	.dev = { .platform_data = &snddev_imic_data },
};


/////////////////////////
#else	// Qualcomm Single MIC
/////////////////////////

static struct adie_codec_action_unit imic_8KHz_osr256_actions[] =
	HANDSET_TX_8000_OSR_256;

static struct adie_codec_action_unit imic_16KHz_osr256_actions[] =
	HANDSET_TX_16000_OSR_256;

static struct adie_codec_action_unit imic_48KHz_osr256_actions[] =
	HANDSET_TX_48000_OSR_256;

static struct adie_codec_hwsetting_entry imic_settings[] = {
	{
		.freq_plan = 8000,
		.osr = 256,
		.actions = imic_8KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(imic_8KHz_osr256_actions),
	},
	{
		.freq_plan = 16000,
		.osr = 256,
		.actions = imic_16KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(imic_16KHz_osr256_actions),
	},
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = imic_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(imic_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile imic_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = imic_settings,
	.setting_sz = ARRAY_SIZE(imic_settings),
};

static enum hsed_controller imic_pmctl_id[] = {PM_HSED_CONTROLLER_0};

static struct snddev_icodec_data snddev_imic_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "handset_tx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HANDSET_MIC,
	.profile = &imic_profile,
	.channel_mode = 1,
	.pmctl_id = imic_pmctl_id,
	.pmctl_id_sz = ARRAY_SIZE(imic_pmctl_id),
	.default_sample_rate = 48000,
	.pamp_on = NULL,
	.pamp_off = NULL,
// srinivas.mittapalli@lge.com 16Sept2010 baikal 0009676
#ifdef LGE_CUST_CAL
 	.micbias_current = {&mic_bias0_handset_mic,&mic_bias1_handset_mic},
 #else
	.micbias_current = {1700,1700}, // Need to keep the Tuned values
 #endif //LGE_CUST_CAL
// srinivas.mittapalli@lge.com 16Sept2010
};


static struct platform_device msm_imic_device = {
	.name = "snddev_icodec",
	.id = 1,
	.dev = { .platform_data = &snddev_imic_data },
};

/////////////////////////
#endif  // (CONFIG_MACH_LGE_FLIP)

/////////////////////////////////////////////////////////////////////////////////////////
// Headset Stereo RX : no change (voice? and sound)
/////////////////////////////////////////////////////////////////////////////////////////
#ifndef LGE_CUST_CAL
static 
#endif
 struct adie_codec_action_unit ihs_stereo_rx_48KHz_osr256_actions[] =
	HEADSET_STEREO_RX_LEGACY_48000_OSR_256;

static struct adie_codec_hwsetting_entry ihs_stereo_rx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = ihs_stereo_rx_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(ihs_stereo_rx_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile ihs_stereo_rx_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = ihs_stereo_rx_settings,
	.setting_sz = ARRAY_SIZE(ihs_stereo_rx_settings),
};

/* baikal ID 0009676::srinivas.mittapalli@lge.com 11Oct2010 */
#ifndef LGE_CUST_CAL
static 
#endif
struct snddev_icodec_data snddev_ihs_stereo_rx_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "headset_stereo_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HEADSET_SPKR_STEREO,
	.profile = &ihs_stereo_rx_profile,
	.channel_mode = 2,
	.default_sample_rate = 48000,
/* CONFIG_MACH_LGE_FLIP	ehgrace.kim 10.05.03
	add LGE amp
*/
#if defined (CONFIG_MACH_LGE_FLIP)
	.pamp_on = &lge_snddev_hs_amp_on,
	.pamp_off = &lge_snddev_amp_off,
#else
	.pamp_on = NULL,
	.pamp_off = NULL,
#endif
	.property = SIDE_TONE_MASK,
	.max_voice_rx_vol[VOC_NB_INDEX] = -700,
	.min_voice_rx_vol[VOC_NB_INDEX] = -2200,
	.max_voice_rx_vol[VOC_WB_INDEX] = -900,
	.min_voice_rx_vol[VOC_WB_INDEX] = -2400
};

static struct platform_device msm_ihs_stereo_rx_device = {
	.name = "snddev_icodec",
	.id = 2,
	.dev = { .platform_data = &snddev_ihs_stereo_rx_data },
};

/////////////////////////////////////////////////////////////////////////////////////////
// Headset mono RX : no change (voice?)
/////////////////////////////////////////////////////////////////////////////////////////
#ifndef LGE_CUST_CAL
static 
#endif
 struct adie_codec_action_unit ihs_mono_rx_48KHz_osr256_actions[] =
//HEADSET_RX_LEGACY_48000_OSR_256;
//Baikal ID 0009962 :: Headset Mono configuration for voice call ++kiran.kanneganti@lge.com
HEADSET_MONO_RX_LEGACY_48000_OSR_256;
//--kiran.kanneganti@lge.com
static struct adie_codec_hwsetting_entry ihs_mono_rx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = ihs_mono_rx_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(ihs_mono_rx_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile ihs_mono_rx_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = ihs_mono_rx_settings,
	.setting_sz = ARRAY_SIZE(ihs_mono_rx_settings),
};

/* baikal ID 0009676::srinivas.mittapalli@lge.com 11Oct2010 */
#ifndef LGE_CUST_CAL
static 
#endif
 struct snddev_icodec_data snddev_ihs_mono_rx_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "headset_mono_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HEADSET_SPKR_MONO,
	.profile = &ihs_mono_rx_profile,
	.channel_mode = 1,
	.default_sample_rate = 48000,
#if defined (CONFIG_MACH_LGE_FLIP) //Baikal ID 0009962 Headset Mono Configuration for call.
	.pamp_on = &lge_snddev_hs_amp_on, //kiran.kanneganti@lge.com
	.pamp_off = &lge_snddev_amp_off,
#else
	.pamp_on = NULL,
	.pamp_off = NULL,
#endif
	.property = SIDE_TONE_MASK,
	.max_voice_rx_vol[VOC_NB_INDEX] = -700,
	.min_voice_rx_vol[VOC_NB_INDEX] = -2200,
	.max_voice_rx_vol[VOC_WB_INDEX] = -900,
	.min_voice_rx_vol[VOC_WB_INDEX] = -2400,

};

static struct platform_device msm_ihs_mono_rx_device = {
	.name = "snddev_icodec",
	.id = 3,
	.dev = { .platform_data = &snddev_ihs_mono_rx_data },
};

/////////////////////////////////////////////////////////////////////////////////////////
// Headset stereo RX (FFA) : don't care
/////////////////////////////////////////////////////////////////////////////////////////

static struct adie_codec_action_unit ihs_ffa_stereo_rx_48KHz_osr256_actions[] =
	HEADSET_STEREO_RX_CAPLESS_48000_OSR_256;

static struct adie_codec_hwsetting_entry ihs_ffa_stereo_rx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = ihs_ffa_stereo_rx_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(ihs_ffa_stereo_rx_48KHz_osr256_actions),
	}
};

#ifdef CONFIG_DEBUG_FS
static struct adie_codec_action_unit
	ihs_ffa_stereo_rx_class_d_legacy_48KHz_osr256_actions[] =
	HEADSET_STEREO_RX_CLASS_D_LEGACY_48000_OSR_256;

static struct adie_codec_hwsetting_entry
	ihs_ffa_stereo_rx_class_d_legacy_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions =
		ihs_ffa_stereo_rx_class_d_legacy_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE
		(ihs_ffa_stereo_rx_class_d_legacy_48KHz_osr256_actions),
	}
};

static struct adie_codec_action_unit
	ihs_ffa_stereo_rx_class_ab_legacy_48KHz_osr256_actions[] =
	HEADSET_STEREO_RX_LEGACY_48000_OSR_256;

static struct adie_codec_hwsetting_entry
	ihs_ffa_stereo_rx_class_ab_legacy_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions =
		ihs_ffa_stereo_rx_class_ab_legacy_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE
		(ihs_ffa_stereo_rx_class_ab_legacy_48KHz_osr256_actions),
	}
};
#endif

static struct adie_codec_dev_profile ihs_ffa_stereo_rx_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = ihs_ffa_stereo_rx_settings,
	.setting_sz = ARRAY_SIZE(ihs_ffa_stereo_rx_settings),
};

static struct snddev_icodec_data snddev_ihs_ffa_stereo_rx_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "headset_stereo_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HEADSET_SPKR_STEREO,
	.profile = &ihs_ffa_stereo_rx_profile,
	.channel_mode = 2,
	.default_sample_rate = 48000,
	.voltage_on = msm_snddev_hsed_voltage_on,
	.voltage_off = msm_snddev_hsed_voltage_off,
	.max_voice_rx_vol[VOC_NB_INDEX] = -700,
	.min_voice_rx_vol[VOC_NB_INDEX] = -2200,
	.max_voice_rx_vol[VOC_WB_INDEX] = -900,
	.min_voice_rx_vol[VOC_WB_INDEX] = -2400,
};

static struct platform_device msm_ihs_ffa_stereo_rx_device = {
	.name = "snddev_icodec",
	.id = 4,
	.dev = { .platform_data = &snddev_ihs_ffa_stereo_rx_data },
};

/////////////////////////////////////////////////////////////////////////////////////////
// Headset mono RX (FFA) : don't care
/////////////////////////////////////////////////////////////////////////////////////////

static struct adie_codec_action_unit ihs_ffa_mono_rx_48KHz_osr256_actions[] =
	HEADSET_RX_CAPLESS_48000_OSR_256;

static struct adie_codec_hwsetting_entry ihs_ffa_mono_rx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = ihs_ffa_mono_rx_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(ihs_ffa_mono_rx_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile ihs_ffa_mono_rx_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = ihs_ffa_mono_rx_settings,
	.setting_sz = ARRAY_SIZE(ihs_ffa_mono_rx_settings),
};

static struct snddev_icodec_data snddev_ihs_ffa_mono_rx_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "headset_mono_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HEADSET_SPKR_MONO,
	.profile = &ihs_ffa_mono_rx_profile,
	.channel_mode = 1,
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_hsed_voltage_on,
	.pamp_off = msm_snddev_hsed_voltage_off,
	.max_voice_rx_vol[VOC_NB_INDEX] = -700,
	.min_voice_rx_vol[VOC_NB_INDEX] = -2200,
	.max_voice_rx_vol[VOC_WB_INDEX] = -900,
	.min_voice_rx_vol[VOC_WB_INDEX] = -2400,
};

static struct platform_device msm_ihs_ffa_mono_rx_device = {
	.name = "snddev_icodec",
	.id = 5,
	.dev = { .platform_data = &snddev_ihs_ffa_mono_rx_data },
};

/////////////////////////////////////////////////////////////////////////////////////////
// Headset mono TX : (voice and sound)
/////////////////////////////////////////////////////////////////////////////////////////

static struct adie_codec_action_unit ihs_mono_tx_8KHz_osr256_actions[] =
	HEADSET_MONO_TX_8000_OSR_256;

static struct adie_codec_action_unit ihs_mono_tx_16KHz_osr256_actions[] =
	HEADSET_MONO_TX_16000_OSR_256;
#ifndef LGE_CUST_CAL
static 
#endif
struct adie_codec_action_unit ihs_mono_tx_48KHz_osr256_actions[] =
	HEADSET_MONO_TX_48000_OSR_256;

static struct adie_codec_hwsetting_entry ihs_mono_tx_settings[] = {
	{
		.freq_plan = 8000,
		.osr = 256,
		.actions = ihs_mono_tx_8KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(ihs_mono_tx_8KHz_osr256_actions),
	},
	{
		.freq_plan = 16000,
		.osr = 256,
		.actions = ihs_mono_tx_16KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(ihs_mono_tx_16KHz_osr256_actions),
	},
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = ihs_mono_tx_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(ihs_mono_tx_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile ihs_mono_tx_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = ihs_mono_tx_settings,
	.setting_sz = ARRAY_SIZE(ihs_mono_tx_settings),
};

static struct snddev_icodec_data snddev_ihs_mono_tx_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "headset_mono_tx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HEADSET_MIC,
	.profile = &ihs_mono_tx_profile,
	.channel_mode = 1,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_tx_route_config,
	.pamp_off = msm_snddev_tx_route_deconfig,
};

static struct platform_device msm_ihs_mono_tx_device = {
	.name = "snddev_icodec",
	.id = 6,
	.dev = { .platform_data = &snddev_ihs_mono_tx_data },
};

/////////////////////////////////////////////////////////////////////////////////////////
// FM Radio : don't care
/////////////////////////////////////////////////////////////////////////////////////////

static struct adie_codec_action_unit ifmradio_handset_osr64_actions[] =
	FM_HANDSET_OSR_64;

static struct adie_codec_hwsetting_entry ifmradio_handset_settings[] = {
	{
		.freq_plan = 8000,
		.osr = 256,
		.actions = ifmradio_handset_osr64_actions,
		.action_sz = ARRAY_SIZE(ifmradio_handset_osr64_actions),
	}
};

static struct adie_codec_dev_profile ifmradio_handset_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = ifmradio_handset_settings,
	.setting_sz = ARRAY_SIZE(ifmradio_handset_settings),
};

static struct snddev_icodec_data snddev_ifmradio_handset_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_FM),
	.name = "fmradio_handset_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_LP_FM_SPKR_PHONE_STEREO_RX,
	.profile = &ifmradio_handset_profile,
	.channel_mode = 1,
	.default_sample_rate = 8000,
	.pamp_on = NULL,
	.pamp_off = NULL,
	.dev_vol_type = SNDDEV_DEV_VOL_DIGITAL,
};

static struct platform_device msm_ifmradio_handset_device = {
	.name = "snddev_icodec",
	.id = 7,
	.dev = { .platform_data = &snddev_ifmradio_handset_data },
};

/////////////////////////////////////////////////////////////////////////////////////////
// Speaker RX (sound)
/////////////////////////////////////////////////////////////////////////////////////////

/* CONFIG_MACH_LGE_FLIP	ehgrace.kim 10.05.03
	changed the speaker rx setting
*/
#if defined (CONFIG_MACH_LGE_FLIP)
#ifndef LGE_CUST_CAL
static 
#endif
struct adie_codec_action_unit ispeaker_rx_48KHz_osr256_actions[] =
   SPEAKER_RX_48000_OSR_256;
#else
static struct adie_codec_action_unit ispeaker_rx_48KHz_osr256_actions[] =
   SPEAKER_STEREO_RX_48000_OSR_256;
#endif

static struct adie_codec_hwsetting_entry ispeaker_rx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = ispeaker_rx_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(ispeaker_rx_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile ispeaker_rx_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = ispeaker_rx_settings,
	.setting_sz = ARRAY_SIZE(ispeaker_rx_settings),
};
//Bryce Speaker is Mono. kiran.kanneganti@lge.com
/* baikal ID 0009676::srinivas.mittapalli@lge.com 11Oct2010 */
#ifndef LGE_CUST_CAL
static 
#endif
 struct snddev_icodec_data snddev_ispeaker_rx_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "speaker_stereo_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_SPKR_PHONE_MONO,
	.profile = &ispeaker_rx_profile,
	.channel_mode = 1,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.default_sample_rate = 48000,
/* CONFIG_MACH_LGE_FLIP	ehgrace.kim 10.05.03
	add LGE amp
*/
#if defined (CONFIG_MACH_LGE_FLIP)
	.pamp_on = &lge_snddev_spk_amp_on,
	.pamp_off = &lge_snddev_amp_off,
#else
	.pamp_on = &msm_snddev_poweramp_on,
	.pamp_off = &msm_snddev_poweramp_off,
#endif
	.max_voice_rx_vol[VOC_NB_INDEX] = 1000,
	.min_voice_rx_vol[VOC_NB_INDEX] = -500,
	.max_voice_rx_vol[VOC_WB_INDEX] = 1000,
	.min_voice_rx_vol[VOC_WB_INDEX] = -500,
};

static struct platform_device msm_ispeaker_rx_device = {
	.name = "snddev_icodec",
	.id = 8,
	.dev = { .platform_data = &snddev_ispeaker_rx_data },

};

/////////////////////////////////////////////////////////////////////////////////////////
// FM Speaker : don't care
/////////////////////////////////////////////////////////////////////////////////////////

static struct adie_codec_action_unit ifmradio_speaker_osr64_actions[] =
	FM_SPEAKER_OSR_64;

static struct adie_codec_hwsetting_entry ifmradio_speaker_settings[] = {
	{
		.freq_plan = 8000,
		.osr = 256,
		.actions = ifmradio_speaker_osr64_actions,
		.action_sz = ARRAY_SIZE(ifmradio_speaker_osr64_actions),
	}
};

static struct adie_codec_dev_profile ifmradio_speaker_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = ifmradio_speaker_settings,
	.setting_sz = ARRAY_SIZE(ifmradio_speaker_settings),
};

static struct snddev_icodec_data snddev_ifmradio_speaker_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_FM),
	.name = "fmradio_speaker_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_LP_FM_SPKR_PHONE_STEREO_RX,
	.profile = &ifmradio_speaker_profile,
	.channel_mode = 1,
	.default_sample_rate = 8000,
	.pamp_on = &msm_snddev_poweramp_on,
	.pamp_off = &msm_snddev_poweramp_off,
	.dev_vol_type = SNDDEV_DEV_VOL_DIGITAL,
};

static struct platform_device msm_ifmradio_speaker_device = {
	.name = "snddev_icodec",
	.id = 9,
	.dev = { .platform_data = &snddev_ifmradio_speaker_data },
};

/////////////////////////////////////////////////////////////////////////////////////////
// FM headset : don't care
/////////////////////////////////////////////////////////////////////////////////////////

static struct adie_codec_action_unit ifmradio_headset_osr64_actions[] =
	FM_HEADSET_STEREO_CLASS_D_LEGACY_OSR_64;

static struct adie_codec_hwsetting_entry ifmradio_headset_settings[] = {
	{
		.freq_plan = 8000,
		.osr = 256,
		.actions = ifmradio_headset_osr64_actions,
		.action_sz = ARRAY_SIZE(ifmradio_headset_osr64_actions),
	}
};

static struct adie_codec_dev_profile ifmradio_headset_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = ifmradio_headset_settings,
	.setting_sz = ARRAY_SIZE(ifmradio_headset_settings),
};

static struct snddev_icodec_data snddev_ifmradio_headset_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_FM),
	.name = "fmradio_headset_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_LP_FM_HEADSET_SPKR_STEREO_RX,
	.profile = &ifmradio_headset_profile,
	.channel_mode = 1,
	.default_sample_rate = 8000,
	.pamp_on = NULL,
	.pamp_off = NULL,
	.dev_vol_type = SNDDEV_DEV_VOL_DIGITAL,
};

static struct platform_device msm_ifmradio_headset_device = {
	.name = "snddev_icodec",
	.id = 10,
	.dev = { .platform_data = &snddev_ifmradio_headset_data },
};

/////////////////////////////////////////////////////////////////////////////////////////
// FM headset (FFA) : don't care
/////////////////////////////////////////////////////////////////////////////////////////

static struct adie_codec_action_unit ifmradio_ffa_headset_osr64_actions[] =
	FM_HEADSET_CLASS_AB_STEREO_CAPLESS_OSR_64;

static struct adie_codec_hwsetting_entry ifmradio_ffa_headset_settings[] = {
	{
		.freq_plan = 8000,
		.osr = 256,
		.actions = ifmradio_ffa_headset_osr64_actions,
		.action_sz = ARRAY_SIZE(ifmradio_ffa_headset_osr64_actions),
	}
};

static struct adie_codec_dev_profile ifmradio_ffa_headset_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = ifmradio_ffa_headset_settings,
	.setting_sz = ARRAY_SIZE(ifmradio_ffa_headset_settings),
};

static struct snddev_icodec_data snddev_ifmradio_ffa_headset_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_FM),
	.name = "fmradio_headset_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_LP_FM_HEADSET_SPKR_STEREO_RX,
	.profile = &ifmradio_ffa_headset_profile,
	.channel_mode = 1,
	.default_sample_rate = 8000,
	.pamp_on = msm_snddev_hsed_voltage_on,
	.pamp_off = msm_snddev_hsed_voltage_off,
	.dev_vol_type = SNDDEV_DEV_VOL_DIGITAL,
};

static struct platform_device msm_ifmradio_ffa_headset_device = {
	.name = "snddev_icodec",
	.id = 11,
	.dev = { .platform_data = &snddev_ifmradio_ffa_headset_data },
};

/////////////////////////////////////////////////////////////////////////////////////////
// BT Voice RX
/////////////////////////////////////////////////////////////////////////////////////////

/* baikal ID 0009676::srinivas.mittapalli@lge.com 11Oct2010 */
#ifndef LGE_CUST_CAL
static 
#endif
struct snddev_ecodec_data snddev_bt_sco_earpiece_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "bt_sco_rx",
	.copp_id = 1,
	.acdb_id = ACDB_ID_BT_SCO_SPKR,
	.channel_mode = 1,
	.conf_pcm_ctl_val = BT_SCO_PCM_CTL_VAL,
	.conf_aux_codec_intf = BT_SCO_AUX_CODEC_INTF,
	.conf_data_format_padding_val = BT_SCO_DATA_FORMAT_PADDING,
	.max_voice_rx_vol[VOC_NB_INDEX] = 400,
	.min_voice_rx_vol[VOC_NB_INDEX] = -1100,
	.max_voice_rx_vol[VOC_WB_INDEX] = 400,
	.min_voice_rx_vol[VOC_WB_INDEX] = -1100,
/* daniel.kang@lge.com ++ */
/* Since we add the following element, we need to initialize them */
#if defined (CONFIG_MACH_LGE_FLIP)
//include pmic to add mic bias for external codec also. Kiran kanneganti. 26-aug-2010		
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.need_tx_rx_lb = 0,
	.pamp_on = NULL,
	.pamp_off = NULL,
	.audience_ctrl_ON = NULL,
	.audience_ctrl_OFF = NULL,
#endif
/* daniel.kang@lge.com -- */
};

/////////////////////////////////////////////////////////////////////////////////////////
// BT Voice TX
/////////////////////////////////////////////////////////////////////////////////////////

static struct snddev_ecodec_data snddev_bt_sco_mic_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "bt_sco_tx",
	.copp_id = 1,
	.acdb_id = ACDB_ID_BT_SCO_MIC,
	.channel_mode = 1,
	.conf_pcm_ctl_val = BT_SCO_PCM_CTL_VAL,
	.conf_aux_codec_intf = BT_SCO_AUX_CODEC_INTF,
	.conf_data_format_padding_val = BT_SCO_DATA_FORMAT_PADDING,
/* daniel.kang@lge.com ++ */
/* Since we add the following element, we need to initialize them */
#if defined (CONFIG_MACH_LGE_FLIP)
//include pmic to add mic bias for external codec also. Kiran kanneganti. 26-aug-2010		
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.need_tx_rx_lb = 0,
	.pamp_on = NULL,
	.pamp_off = NULL,
	.audience_ctrl_ON = NULL,
	.audience_ctrl_OFF = NULL,
#endif
/* daniel.kang@lge.com -- */
};

struct platform_device msm_bt_sco_earpiece_device = {
	.name = "msm_snddev_ecodec",
	.id = 0,
	.dev = { .platform_data = &snddev_bt_sco_earpiece_data },
};

struct platform_device msm_bt_sco_mic_device = {
	.name = "msm_snddev_ecodec",
	.id = 1,
	.dev = { .platform_data = &snddev_bt_sco_mic_data },
};

// SRI_START Adding new device 25th Aug ,2010 (daniel.kang@lge.com)
// Audience device configuration
#if defined (CONFIG_MACH_LGE_FLIP)

/////////////////////////////////////////////////////////////////////////////////////////
// Audience Handset RX
/////////////////////////////////////////////////////////////////////////////////////////
/* baikal ID 0009676::srinivas.mittapalli@lge.com 11Oct2010 */
#ifndef LGE_CUST_CAL
static 
#endif
struct snddev_ecodec_data snddev_iearpiece_audience_device = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "handset_audience_rx",
	.copp_id = 1,						 
	.acdb_id = ACDB_ID_AUD_HANDSET_RX, //Audience cal data added. kiran.kanneganti@lge.com 07-SEP-2010 
	.channel_mode = 1,
	.conf_pcm_ctl_val = HANDSET_PCM_CTL_VAL,
	.conf_aux_codec_intf = HANDSET_AUX_CODEC_INTF,
	.conf_data_format_padding_val = HANDSET_DATA_FORMAT_PADDING,
	.max_voice_rx_vol[VOC_NB_INDEX] = 400,
	.min_voice_rx_vol[VOC_NB_INDEX] = -1100,
	.max_voice_rx_vol[VOC_WB_INDEX] = 400,
	.min_voice_rx_vol[VOC_WB_INDEX] = -1100,
//	.max_voice_rx_vol[VOC_NB_INDEX] = 0,		// 400,		// set 0dB RX gain, TX gain is set in acdb.
//	.min_voice_rx_vol[VOC_NB_INDEX] = -1000,	// -1100,	// set -10dB RX gain
//	.max_voice_rx_vol[VOC_WB_INDEX] = 0,		// 400,		// Value changed for Audience Cal
//	.min_voice_rx_vol[VOC_WB_INDEX] = -1000,	// -1100,
//include pmic to add mic bias for external codec also. Kiran kanneganti. 26-aug-2010
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.need_tx_rx_lb = 1,
	.pamp_on = NULL,
	.pamp_off = NULL,
	.audience_ctrl_ON = NULL,
	.audience_ctrl_OFF = NULL,
};

/////////////////////////////////////////////////////////////////////////////////////////
// Audience Handset TX
/////////////////////////////////////////////////////////////////////////////////////////

static enum hsed_controller idual_mic_pmctl_id[] = {
	PM_HSED_CONTROLLER_0, PM_HSED_CONTROLLER_1
};

static struct snddev_ecodec_data snddev_imic_audience_device = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "handset_audience_tx",
	.copp_id = 1,						 
	.acdb_id = ACDB_ID_AUD_HANDSET_TX, //Audience cal data added. kiran.kanneganti@lge.com 07-SEP-2010 
	.channel_mode = 1,				 
	.conf_pcm_ctl_val = HANDSET_PCM_CTL_VAL,
	.conf_aux_codec_intf = HANDSET_AUX_CODEC_INTF,
	.conf_data_format_padding_val = HANDSET_DATA_FORMAT_PADDING,
//include pmic to add mic bias for external codec also. Kiran kanneganti. 26-aug-2010
	.pmctl_id = idual_mic_pmctl_id,
	.pmctl_id_sz = ARRAY_SIZE(idual_mic_pmctl_id),
	.need_tx_rx_lb = 0,
	.pamp_on = &lge_snddev_AUD_mic_route_config,
	.pamp_off = &lge_snddev_mic_route_deconfig,
	.audience_ctrl_ON = &lge_audience_ON,
	.audience_ctrl_OFF = &lge_audience_OFF,
// srinivas.mittapalli@lge.com 16Sept2010 baikal 0009676
#ifdef LGE_CUST_CAL
 	.micbias_current = {&mic_bias0_aud_hmic,&mic_bias1_aud_hmic},
 #else
	.micbias_current = {1700,1700}, // Need to keep the Tuned values
 #endif //LGE_CUST_CAL
// srinivas.mittapalli@lge.com 16Sept2010
};

struct platform_device msm_iearpiece_audience_device = {
	.name = "msm_snddev_ecodec",
	.id = 2,
	.dev = { .platform_data = &snddev_iearpiece_audience_device },
};

struct platform_device msm_imic_audience_device = {
	.name = "msm_snddev_ecodec",
	.id = 3,
	.dev = { .platform_data = &snddev_imic_audience_device },
};

/////////////////////////////////////////////////////////////////////////////////////////
// Audience Speaker RX
/////////////////////////////////////////////////////////////////////////////////////////
/* baikal ID 0009676::srinivas.mittapalli@lge.com 11Oct2010 */
#ifndef LGE_CUST_CAL
static 
#endif
 struct snddev_ecodec_data snddev_ispeaker_audience_rx_device = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "speaker_mono_audience_rx", //kiran.kanneganti@lge.com 03 Sep 2010
	.copp_id = 1,					 
	.acdb_id = ACDB_ID_AUD_SPKR_RX, //Audience cal data added. kiran.kanneganti@lge.com 07-SEP-2010 
	.channel_mode = 1,
	.conf_pcm_ctl_val = HANDSET_PCM_CTL_VAL,
	.conf_aux_codec_intf = HANDSET_AUX_CODEC_INTF,
	.conf_data_format_padding_val = HANDSET_DATA_FORMAT_PADDING,
/*BEGIN: 0010043 kiran.kanneganti@lge.com 2010-10-19*/
/*MOD: 0010043: Increased Rx voice volume for speaker & removed unused code*/	
	.max_voice_rx_vol[VOC_NB_INDEX] = 1500,
	.min_voice_rx_vol[VOC_NB_INDEX] = 0,
	.max_voice_rx_vol[VOC_WB_INDEX] = 1500,
	.min_voice_rx_vol[VOC_WB_INDEX] = 0,
/*END:0010043 kiran.kanneganti@lge.com 2010-10-19*/
//include pmic to add mic bias for external codec also. Kiran kanneganti. 26-aug-2010		
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.need_tx_rx_lb = 1,
	.pamp_on = &lge_snddev_spk_amp_on,
	.pamp_off = &lge_snddev_amp_off,
	.audience_ctrl_ON = NULL,
	.audience_ctrl_OFF = NULL,
};

/////////////////////////////////////////////////////////////////////////////////////////
// Audience Speaker TX
/////////////////////////////////////////////////////////////////////////////////////////

static struct snddev_ecodec_data snddev_ispeaker_audience_tx_device = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "speaker_mono_audience_tx",//kiran.kanneganti@lge.com 03 Sep 2010
	.copp_id = 1,						
	.acdb_id = ACDB_ID_AUD_SPKR_TX, //Audience cal data added. kiran.kanneganti@lge.com 07-SEP-2010 
	.channel_mode = 1,				
	.conf_pcm_ctl_val = HANDSET_PCM_CTL_VAL,
	.conf_aux_codec_intf = HANDSET_AUX_CODEC_INTF,
	.conf_data_format_padding_val = HANDSET_DATA_FORMAT_PADDING,
//include pmic to add mic bias for external codec also. Kiran kanneganti. 26-aug-2010		
	.pmctl_id = idual_mic_pmctl_id,
	.pmctl_id_sz = ARRAY_SIZE(idual_mic_pmctl_id),
	.need_tx_rx_lb = 0,
	.pamp_on = &lge_snddev_AUD_mic_route_config,
	.pamp_off = &lge_snddev_mic_route_deconfig,
//add function pointers to control audience. kiran.kanneganti 26-aug-2010	
	.audience_ctrl_ON = &lge_audience_ON,
	.audience_ctrl_OFF = &lge_audience_OFF,
// srinivas.mittapalli@lge.com 16Sept2010 baikal 0009676
#ifdef  LGE_CUST_CAL
	 .micbias_current = {&mic_bias0_aud_spkmic,&mic_bias0_aud_spkmic},
 #else
	.micbias_current = {1700,1700}, // Need to keep the Tuned values
 #endif //LGE_CUST_CAL
// srinivas.mittapalli@lge.com 16Sept2010	
};

struct platform_device msm_ispeaker_audience_rx_device = {
	.name = "msm_snddev_ecodec",
	.id = 4,
	.dev = { .platform_data = &snddev_ispeaker_audience_rx_device },
};

struct platform_device msm_ispeaker_audience_tx_device = {
	.name = "msm_snddev_ecodec",
	.id = 5,
	.dev = { .platform_data = &snddev_ispeaker_audience_tx_device },
};
#endif
// SRI_END Adding new device 25th Aug ,2010 (daniel.kang@lge.com)

/////////////////////////////////////////////////////////////////////////////////////////
// Dual Mic Handset TX (Back mic as primary) sound
/////////////////////////////////////////////////////////////////////////////////////////
#ifndef LGE_CUST_CAL
static 
#endif
struct adie_codec_action_unit idual_mic_endfire_8KHz_osr256_actions[] =
/* Kiran.kanneganti@lge.com 07-SEP-2010 ++ */
/* for QCOM dual Mic Support */
#if defined (CONFIG_MACH_LGE_FLIP)
	MIC1_LEFT_AUX_IN_RIGHT_8000_OSR_256;
#else /* This is the qualcomm default code */
	MIC1_LEFT_LINE_IN_RIGHT_8000_OSR_256;
#endif
/* Kiran.kanneganti@lge.com 07-SEP-2010 -- */
static struct adie_codec_hwsetting_entry idual_mic_endfire_settings[] = {
	{
		.freq_plan = 8000,
		.osr = 256,
		.actions = idual_mic_endfire_8KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(idual_mic_endfire_8KHz_osr256_actions),
	}, /* 8KHz profile can be used for 16KHz */
	{
		.freq_plan = 16000,
		.osr = 256,
		.actions = idual_mic_endfire_8KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(idual_mic_endfire_8KHz_osr256_actions),
	}, /* 8KHz profile can be used for 48KHz */
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = idual_mic_endfire_8KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(idual_mic_endfire_8KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile idual_mic_endfire_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = idual_mic_endfire_settings,
	.setting_sz = ARRAY_SIZE(idual_mic_endfire_settings),
};
/* Kiran.kanneganti@lge.com 07-SEP-2010 ++ */
/* for QCOM dual Mic Support */
#if defined (CONFIG_MACH_LGE_FLIP)
static enum hsed_controller idual_mic_endfire_pmctl_id[] = {
	PM_HSED_CONTROLLER_0, PM_HSED_CONTROLLER_1
};
#else /* This is the qualcomm default code */
static enum hsed_controller idual_mic_endfire_pmctl_id[] = {
	PM_HSED_CONTROLLER_0, PM_HSED_CONTROLLER_2
};
#endif
/* Kiran.kanneganti@lge.com 07-SEP-2010 -- */

static struct snddev_icodec_data snddev_idual_mic_endfire_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "handset_dual_mic_endfire_tx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HANDSET_MIC_ENDFIRE,
	.profile = &idual_mic_endfire_profile,
	.channel_mode = 2,
	.default_sample_rate = 48000,
	.pmctl_id = idual_mic_endfire_pmctl_id,
	.pmctl_id_sz = ARRAY_SIZE(idual_mic_endfire_pmctl_id),
/* Kiran.kanneganti@lge.com 07-SEP-2010 ++ */
#if defined (CONFIG_MACH_LGE_FLIP)	
	.pamp_on = &lge_snddev_MSM_mic_route_config,
	.pamp_off = &lge_snddev_mic_route_deconfig,
/*IN Rev C Audience is not required to bypass Mic1. ++ kiran.kanneganti@lge.com 01-Oct-2010 baikal 0009680 */
#if defined(WITHOUT_AUDIENCE) 
	.audience_ctrl_ON = NULL,
	.audience_ctrl_OFF = NULL,
#else
	.audience_ctrl_ON = &lge_audience_ON,
	.audience_ctrl_OFF = &lge_audience_OFF,
#endif	
/*  -- kiran.kanneganti@lge.com 01-Oct-2010*/	
#else /* This is the qualcomm default code */
	.pamp_on = NULL,
	.pamp_off = NULL,
#endif	
/* Kiran.kanneganti@lge.com 07-SEP-2010 -- */
// srinivas.mittapalli@lge.com 16Sept2010 baikal 0009676
#ifdef  LGE_CUST_CAL
 	.micbias_current = {&mic_bias0_handset_endfire,&mic_bias1_handset_endfire},
 #else
	.micbias_current = {1700,1700}, // Need to keep the Tuned values
 #endif //LGE_CUST_CAL
// srinivas.mittapalli@lge.com 16Sept2010
};

static struct platform_device msm_idual_mic_endfire_device = {
	.name = "snddev_icodec",
	.id = 12,
	.dev = { .platform_data = &snddev_idual_mic_endfire_data },
};


static struct snddev_icodec_data\
		snddev_idual_mic_endfire_real_stereo_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "handset_dual_mic_endfire_tx_real_stereo",
	.copp_id = 0,
	.acdb_id = PSEUDO_ACDB_ID,
	.profile = &idual_mic_endfire_profile,
	.channel_mode = REAL_STEREO_CHANNEL_MODE,
	.default_sample_rate = 48000,
	.pmctl_id = idual_mic_endfire_pmctl_id,
	.pmctl_id_sz = ARRAY_SIZE(idual_mic_endfire_pmctl_id),
	.pamp_on = NULL,
	.pamp_off = NULL,
};

static struct platform_device msm_real_stereo_tx_device = {
	.name = "snddev_icodec",
	.id = 26,
	.dev = { .platform_data =
			&snddev_idual_mic_endfire_real_stereo_data },
};

/////////////////////////////////////////////////////////////////////////////////////////
// Dual Mic Broadside : don't care
/////////////////////////////////////////////////////////////////////////////////////////

static struct adie_codec_action_unit idual_mic_bs_8KHz_osr256_actions[] =
	MIC1_LEFT_AUX_IN_RIGHT_8000_OSR_256;

static struct adie_codec_hwsetting_entry idual_mic_broadside_settings[] = {
	{
		.freq_plan = 8000,
		.osr = 256,
		.actions = idual_mic_bs_8KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(idual_mic_bs_8KHz_osr256_actions),
	}, /* 8KHz profile can be used for 16KHz */
	{
		.freq_plan = 16000,
		.osr = 256,
		.actions = idual_mic_bs_8KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(idual_mic_bs_8KHz_osr256_actions),
	}, /* 8KHz profile can be used for 16KHz */
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = idual_mic_bs_8KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(idual_mic_bs_8KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile idual_mic_broadside_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = idual_mic_broadside_settings,
	.setting_sz = ARRAY_SIZE(idual_mic_broadside_settings),
};

static enum hsed_controller idual_mic_broadside_pmctl_id[] = {
	PM_HSED_CONTROLLER_0, PM_HSED_CONTROLLER_2
};

static struct snddev_icodec_data snddev_idual_mic_broadside_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "handset_dual_mic_broadside_tx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HANDSET_MIC_BROADSIDE,
	.profile = &idual_mic_broadside_profile,
	.channel_mode = 2,
	.default_sample_rate = 48000,
	.pmctl_id = idual_mic_broadside_pmctl_id,
	.pmctl_id_sz = ARRAY_SIZE(idual_mic_broadside_pmctl_id),
	.pamp_on = NULL,
	.pamp_off = NULL,
// srinivas.mittapalli@lge.com 16Sept2010 baikal 0009676
#ifdef  LGE_CUST_CAL
 	.micbias_current = {&mic_bias0_handset_broadside,&mic_bias1_handset_broadside},
 #else
	.micbias_current = {1700,1700}, // Need to keep the Tuned values
 #endif //LGE_CUST_CAL
// srinivas.mittapalli@lge.com 16Sept2010	
};

static struct platform_device msm_idual_mic_broadside_device = {
	.name = "snddev_icodec",
	.id = 13,
	.dev = { .platform_data = &snddev_idual_mic_broadside_data },
};

/////////////////////////////////////////////////////////////////////////////////////////
// Dual Mic TX Speaker Mode : Back Mic is primary
/////////////////////////////////////////////////////////////////////////////////////////
#ifndef LGE_CUST_CAL
static 
#endif
struct adie_codec_action_unit ispk_dual_mic_ef_8KHz_osr256_actions[] =
#if defined (CONFIG_MACH_LGE_FLIP)//for QCOM dual Mic Support. Kiran.kanneganti@lge.com
	MIC1_LEFT_AUX_IN_RIGHT_8000_OSR_256;
#else
	SPEAKER_MIC1_LEFT_LINE_IN_RIGHT_8000_OSR_256;
#endif

static struct adie_codec_hwsetting_entry ispk_dual_mic_ef_settings[] = {
	{
		.freq_plan = 8000,
		.osr = 256,
		.actions = ispk_dual_mic_ef_8KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(ispk_dual_mic_ef_8KHz_osr256_actions),
	}, /* 8KHz profile can be used for 16Khz */
	{
		.freq_plan = 16000,
		.osr = 256,
		.actions = ispk_dual_mic_ef_8KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(ispk_dual_mic_ef_8KHz_osr256_actions),
	}, /* 8KHz profile can be used for 48KHz */
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = ispk_dual_mic_ef_8KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(ispk_dual_mic_ef_8KHz_osr256_actions),
	},
};

static struct adie_codec_dev_profile ispk_dual_mic_ef_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = ispk_dual_mic_ef_settings,
	.setting_sz = ARRAY_SIZE(ispk_dual_mic_ef_settings),
};

static struct snddev_icodec_data snddev_spk_idual_mic_endfire_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "speaker_dual_mic_endfire_tx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_SPKR_PHONE_MIC_ENDFIRE,
	.profile = &ispk_dual_mic_ef_profile,
	.channel_mode = 2,
	.default_sample_rate = 48000,
	.pmctl_id = idual_mic_endfire_pmctl_id,
	.pmctl_id_sz = ARRAY_SIZE(idual_mic_endfire_pmctl_id),
#if defined (CONFIG_MACH_LGE_FLIP)	
	.pamp_on = &lge_snddev_MSM_mic_route_config,
	.pamp_off = &lge_snddev_mic_route_deconfig,
/*IN Rev C Audience is not required to bypass Mic1. ++ kiran.kanneganti@lge.com 01-Oct-2010 baikal 0009680 */
#if defined(WITHOUT_AUDIENCE)	
	.audience_ctrl_ON = NULL,
	.audience_ctrl_OFF = NULL,
#else
	.audience_ctrl_ON = &lge_audience_ON,
	.audience_ctrl_OFF = &lge_audience_OFF,
#endif
/* -- kiran.kanneganti@lge.com 01-Oct-2010*/
#else
	.pamp_on = NULL,
	.pamp_off = NULL,
#endif	
// srinivas.mittapalli@lge.com 16Sept2010 baikal 0009676
#ifdef LGE_CUST_CAL
 	.micbias_current = {&mic_bias0_spk_endfire,&mic_bias1_spk_endfire},
 #else
	.micbias_current = {1700,1700}, // Need to keep the Tuned values
 #endif //LGE_CUST_CAL
// srinivas.mittapalli@lge.com 16Sept2010
};

static struct platform_device msm_spk_idual_mic_endfire_device = {
	.name = "snddev_icodec",
	.id = 14,
	.dev = { .platform_data = &snddev_spk_idual_mic_endfire_data },
};

/////////////////////////////////////////////////////////////////////////////////////////
// Dual Mic TX Broadside : don't care
/////////////////////////////////////////////////////////////////////////////////////////

static struct adie_codec_action_unit ispk_dual_mic_bs_8KHz_osr256_actions[] =
	SPEAKER_MIC1_LEFT_AUX_IN_RIGHT_8000_OSR_256;

static struct adie_codec_hwsetting_entry ispk_dual_mic_bs_settings[] = {
	{
		.freq_plan = 8000,
		.osr = 256,
		.actions = ispk_dual_mic_bs_8KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(ispk_dual_mic_bs_8KHz_osr256_actions),
	}, /* 8KHz profile can be used for 16Khz */
	{
		.freq_plan = 16000,
		.osr = 256,
		.actions = ispk_dual_mic_bs_8KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(ispk_dual_mic_bs_8KHz_osr256_actions),
	}, /* 8KHz profile can be used for 48KHz */
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = ispk_dual_mic_bs_8KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(ispk_dual_mic_bs_8KHz_osr256_actions),
	},
};

static struct adie_codec_dev_profile ispk_dual_mic_bs_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = ispk_dual_mic_bs_settings,
	.setting_sz = ARRAY_SIZE(ispk_dual_mic_bs_settings),
};
static struct snddev_icodec_data snddev_spk_idual_mic_broadside_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "speaker_dual_mic_broadside_tx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_SPKR_PHONE_MIC_BROADSIDE,
	.profile = &ispk_dual_mic_bs_profile,
	.channel_mode = 2,
	.default_sample_rate = 48000,
	.pmctl_id = idual_mic_broadside_pmctl_id,
	.pmctl_id_sz = ARRAY_SIZE(idual_mic_broadside_pmctl_id),
	.pamp_on = NULL,
	.pamp_off = NULL,
// srinivas.mittapalli@lge.com 16Sept2010 baikal 0009676
#ifdef  LGE_CUST_CAL
 	.micbias_current = {&mic_bias0_spk_broadside,&mic_bias1_spk_broadside},
 #else
	.micbias_current = {1700,1700}, // Need to keep the Tuned values
 #endif //LGE_CUST_CAL
// srinivas.mittapalli@lge.com 16Sept2010
};

static struct platform_device msm_spk_idual_mic_broadside_device = {
	.name = "snddev_icodec",
	.id = 15,
	.dev = { .platform_data = &snddev_spk_idual_mic_broadside_data },
};

/////////////////////////////////////////////////////////////////////////////////////////
// TTY headset TX
/////////////////////////////////////////////////////////////////////////////////////////

static struct adie_codec_action_unit itty_hs_mono_tx_8KHz_osr256_actions[] =
	TTY_HEADSET_MONO_TX_8000_OSR_256;

static struct adie_codec_hwsetting_entry itty_hs_mono_tx_settings[] = {
	/* 8KHz, 16KHz, 48KHz TTY Tx devices can shared same set of actions */
	{
		.freq_plan = 8000,
		.osr = 256,
		.actions = itty_hs_mono_tx_8KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(itty_hs_mono_tx_8KHz_osr256_actions),
	},
	{
		.freq_plan = 16000,
		.osr = 256,
		.actions = itty_hs_mono_tx_8KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(itty_hs_mono_tx_8KHz_osr256_actions),
	},
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = itty_hs_mono_tx_8KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(itty_hs_mono_tx_8KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile itty_hs_mono_tx_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = itty_hs_mono_tx_settings,
	.setting_sz = ARRAY_SIZE(itty_hs_mono_tx_settings),
};

static struct snddev_icodec_data snddev_itty_hs_mono_tx_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE | SNDDEV_CAP_TTY),
	.name = "tty_headset_mono_tx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_TTY_HEADSET_MIC,
	.profile = &itty_hs_mono_tx_profile,
	.channel_mode = 1,
	.default_sample_rate = 48000,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.pamp_on = NULL,
	.pamp_off = NULL,
};

static struct platform_device msm_itty_hs_mono_tx_device = {
	.name = "snddev_icodec",
	.id = 16,
	.dev = { .platform_data = &snddev_itty_hs_mono_tx_data },
};

/////////////////////////////////////////////////////////////////////////////////////////
// TTY headset RX
/////////////////////////////////////////////////////////////////////////////////////////

static struct adie_codec_action_unit itty_hs_mono_rx_8KHz_osr256_actions[] =
	TTY_HEADSET_MONO_RX_CLASS_D_8000_OSR_256;

static struct adie_codec_action_unit itty_hs_mono_rx_16KHz_osr256_actions[] =
	TTY_HEADSET_MONO_RX_CLASS_D_16000_OSR_256;

static struct adie_codec_action_unit itty_hs_mono_rx_48KHz_osr256_actions[] =
	TTY_HEADSET_MONO_RX_CLASS_D_48000_OSR_256;

static struct adie_codec_hwsetting_entry itty_hs_mono_rx_settings[] = {
	{
		.freq_plan = 8000,
		.osr = 256,
		.actions = itty_hs_mono_rx_8KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(itty_hs_mono_rx_8KHz_osr256_actions),
	},
	{
		.freq_plan = 16000,
		.osr = 256,
		.actions = itty_hs_mono_rx_16KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(itty_hs_mono_rx_16KHz_osr256_actions),
	},
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = itty_hs_mono_rx_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(itty_hs_mono_rx_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile itty_hs_mono_rx_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = itty_hs_mono_rx_settings,
	.setting_sz = ARRAY_SIZE(itty_hs_mono_rx_settings),
};

static struct snddev_icodec_data snddev_itty_hs_mono_rx_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE | SNDDEV_CAP_TTY),
	.name = "tty_headset_mono_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_TTY_HEADSET_SPKR,
	.profile = &itty_hs_mono_rx_profile,
	.channel_mode = 1,
	.default_sample_rate = 48000,

// daniel.kang@lge.com ++
// Baikal ID : 0010221, tty rx needs AMP
#if defined (CONFIG_MACH_LGE_FLIP) 
        .pamp_on = &lge_snddev_hs_amp_on,
        .pamp_off = &lge_snddev_amp_off,
#else
        .pamp_on = NULL,
        .pamp_off = NULL,
#endif
// daniel.kang@lge.com --
	.max_voice_rx_vol[VOC_NB_INDEX] = 0,
	.min_voice_rx_vol[VOC_NB_INDEX] = 0,
	.max_voice_rx_vol[VOC_WB_INDEX] = 0,
	.min_voice_rx_vol[VOC_WB_INDEX] = 0,
};

static struct platform_device msm_itty_hs_mono_rx_device = {
	.name = "snddev_icodec",
	.id = 17,
	.dev = { .platform_data = &snddev_itty_hs_mono_rx_data },
};

/////////////////////////////////////////////////////////////////////////////////////////
// Dual mic tx Speaker : Main mic as primary
//      (QCT default configuration is Single MIC)
/////////////////////////////////////////////////////////////////////////////////////////

#if defined (CONFIG_MACH_LGE_FLIP)  //speaker_tx supports dual Mic & Miain Mic as primary. Kiran kanneganti@lge.com 09 SEP 2010 

//////////////////////////
#ifndef LGE_CUST_CAL
static 
#endif
struct adie_codec_action_unit ispeaker_tx_8KHz_osr256_actions[] =
	MIC1_RIGHT_AUX_IN_LEFT_8000_OSR_256;

//TODO: Need to be verified from QCOM.
/*QCOM Dual MIC using 8K config for 16 & 48 K.*/
static struct adie_codec_hwsetting_entry ispeaker_tx_settings[] = {
	{
		.freq_plan = 8000,
		.osr = 256,
		.actions = ispeaker_tx_8KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(ispeaker_tx_8KHz_osr256_actions),
	},
	{ /* 8KHz profile is good for 16KHz */
		.freq_plan = 16000,
		.osr = 256,
		.actions = ispeaker_tx_8KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(ispeaker_tx_8KHz_osr256_actions),
	},/* 8KHz profile can be used for 48Khz */
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = ispeaker_tx_8KHz_osr256_actions,//ispeaker_tx_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(ispeaker_tx_8KHz_osr256_actions),//ARRAY_SIZE(ispeaker_tx_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile ispeaker_tx_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = ispeaker_tx_settings,
	.setting_sz = ARRAY_SIZE(ispeaker_tx_settings),
};

static enum hsed_controller ispk_pmctl_id[] = {PM_HSED_CONTROLLER_0,PM_HSED_CONTROLLER_1};

static struct snddev_icodec_data snddev_ispeaker_tx_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "speaker_mono_tx",
	.copp_id = 0,
// daniel.kang@lge.com ++ 3rd Nov
// 0010463: Enable Fluence in handset, speaker TX
	.acdb_id = ACDB_ID_SPKR_PHONE_MIC_ENDFIRE,	// ACDB_ID_SPKR_PHONE_MIC,
// daniel.kang@lge.com -- 3rd Nov
	.profile = &ispeaker_tx_profile,
	.channel_mode = 2,		// dual mic
	.pmctl_id = ispk_pmctl_id,
	.pmctl_id_sz = ARRAY_SIZE(ispk_pmctl_id),
	.default_sample_rate = 48000,
/* CONFIG_MACH_LGE_FLIP	ehgrace.kim 10.05.03
	add LGE amp
*/
/* daniel.kang@lge.com ++ */
	.pamp_on = &lge_snddev_MSM_mic_route_config,
	.pamp_off = &lge_snddev_mic_route_deconfig,
/*IN Rev C Audience is not required to bypass Mic1. ++ kiran.kanneganti@lge.com 01-Oct-2010 baikal 0009680 */
#if defined(WITHOUT_AUDIENCE)
	.audience_ctrl_ON = NULL,
	.audience_ctrl_OFF = NULL, 
#else
	.audience_ctrl_ON = &lge_audience_ON,
	.audience_ctrl_OFF = &lge_audience_OFF,	
#endif	
/* -- kiran.kanneganti@lge.com 01-Oct-2010*/
// srinivas.mittapalli@lge.com 16Sept2010 baikal 0009676
#ifdef LGE_CUST_CAL
 	.micbias_current = {&mic_bias0_spkmic,&mic_bias1_spkmic},
 #else
	.micbias_current = {1700,1700}, // Need to keep the Tuned values
 #endif //LGE_CUST_CAL
// srinivas.mittapalli@lge.com 16Sept2010	
/* daniel.kang@lge.com -- */
};

static struct platform_device msm_ispeaker_tx_device = {
	.name = "snddev_icodec",
	.id = 18,
	.dev = { .platform_data = &snddev_ispeaker_tx_data },
};

///////////////////////////
#else // QCT default (Single MIC)
///////////////////////////

static struct adie_codec_action_unit ispeaker_tx_8KHz_osr256_actions[] =
	SPEAKER_TX_8000_OSR_256;

static struct adie_codec_action_unit ispeaker_tx_48KHz_osr256_actions[] =
	SPEAKER_TX_48000_OSR_256;

static struct adie_codec_hwsetting_entry ispeaker_tx_settings[] = {
	{
		.freq_plan = 8000,
		.osr = 256,
		.actions = ispeaker_tx_8KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(ispeaker_tx_8KHz_osr256_actions),
	},
	{ /* 8KHz profile is good for 16KHz */
		.freq_plan = 16000,
		.osr = 256,
		.actions = ispeaker_tx_8KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(ispeaker_tx_8KHz_osr256_actions),
	},
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = ispeaker_tx_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(ispeaker_tx_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile ispeaker_tx_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = ispeaker_tx_settings,
	.setting_sz = ARRAY_SIZE(ispeaker_tx_settings),
};

static enum hsed_controller ispk_pmctl_id[] = {PM_HSED_CONTROLLER_0};

static struct snddev_icodec_data snddev_ispeaker_tx_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "speaker_mono_tx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_SPKR_PHONE_MIC,
	.profile = &ispeaker_tx_profile,
	.channel_mode = 1,
	.pmctl_id = ispk_pmctl_id,
	.pmctl_id_sz = ARRAY_SIZE(ispk_pmctl_id),
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_tx_route_config,
	.pamp_off = msm_snddev_tx_route_deconfig,
// srinivas.mittapalli@lge.com 16Sept2010 baikal 0009676
#ifdef  LGE_CUST_CAL
 	.micbias_current = {&mic_bias0_spkmic,&mic_bias1_spkmic},
 #else
	.micbias_current = {1700,1700}, // Need to keep the Tuned values
 #endif //LGE_CUST_CAL
// srinivas.mittapalli@lge.com 16Sept2010	
};

static struct platform_device msm_ispeaker_tx_device = {
	.name = "snddev_icodec",
	.id = 18,
	.dev = { .platform_data = &snddev_ispeaker_tx_data },
};

#endif	// (CONFIG_MACH_LGE_FLIP)

/////////////////////////////////////////////////////////////////////////////////////////
// Handset RX FFA : don't care
/////////////////////////////////////////////////////////////////////////////////////////

static struct adie_codec_action_unit iearpiece_ffa_48KHz_osr256_actions[] =
	HANDSET_RX_48000_OSR_256_FFA;

static struct adie_codec_hwsetting_entry iearpiece_ffa_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = iearpiece_ffa_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(iearpiece_ffa_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile iearpiece_ffa_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = iearpiece_ffa_settings,
	.setting_sz = ARRAY_SIZE(iearpiece_ffa_settings),
};

static struct snddev_icodec_data snddev_iearpiece_ffa_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "handset_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HANDSET_SPKR,
	.profile = &iearpiece_ffa_profile,
	.channel_mode = 1,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.default_sample_rate = 48000,
	.pamp_on = NULL,
	.pamp_off = NULL,
	.max_voice_rx_vol[VOC_NB_INDEX] = -700,
	.min_voice_rx_vol[VOC_NB_INDEX] = -2200,
	.max_voice_rx_vol[VOC_WB_INDEX] = -1400,
	.min_voice_rx_vol[VOC_WB_INDEX] = -2900,
};

static struct platform_device msm_iearpiece_ffa_device = {
	.name = "snddev_icodec",
	.id = 19,
	.dev = { .platform_data = &snddev_iearpiece_ffa_data },
};

/////////////////////////////////////////////////////////////////////////////////////////
// Handset TX FFA : don't care
/////////////////////////////////////////////////////////////////////////////////////////

static struct adie_codec_action_unit imic_ffa_8KHz_osr256_actions[] =
	HANDSET_TX_8000_OSR_256_FFA;

static struct adie_codec_action_unit imic_ffa_16KHz_osr256_actions[] =
	HANDSET_TX_16000_OSR_256_FFA;

static struct adie_codec_action_unit imic_ffa_48KHz_osr256_actions[] =
	HANDSET_TX_48000_OSR_256_FFA;

static struct adie_codec_hwsetting_entry imic_ffa_settings[] = {
	{
		.freq_plan = 8000,
		.osr = 256,
		.actions = imic_ffa_8KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(imic_ffa_8KHz_osr256_actions),
	},
	{
		.freq_plan = 16000,
		.osr = 256,
		.actions = imic_ffa_16KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(imic_ffa_16KHz_osr256_actions),
	},
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = imic_ffa_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(imic_ffa_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile imic_ffa_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = imic_ffa_settings,
	.setting_sz = ARRAY_SIZE(imic_ffa_settings),
};

static struct snddev_icodec_data snddev_imic_ffa_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "handset_tx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HANDSET_MIC,
	.profile = &imic_ffa_profile,
	.channel_mode = 1,
	.pmctl_id = imic_pmctl_id,
	.pmctl_id_sz = ARRAY_SIZE(imic_pmctl_id),
	.default_sample_rate = 48000,
	.pamp_on = NULL,
	.pamp_off = NULL,
};

static struct platform_device msm_imic_ffa_device = {
	.name = "snddev_icodec",
	.id = 20,
	.dev = { .platform_data = &snddev_imic_ffa_data },
};

/////////////////////////////////////////////////////////////////////////////////////////
// Handset and Speaker RX : Notification
/////////////////////////////////////////////////////////////////////////////////////////
#ifndef LGE_CUST_CAL
static 
#endif 
struct adie_codec_action_unit
	ihs_stereo_speaker_stereo_rx_48KHz_osr256_actions[] =
	HEADSET_STEREO_SPEAKER_STEREO_RX_CAPLESS_48000_OSR_256;


static struct adie_codec_hwsetting_entry
	ihs_stereo_speaker_stereo_rx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = ihs_stereo_speaker_stereo_rx_48KHz_osr256_actions,
		.action_sz =
		ARRAY_SIZE(ihs_stereo_speaker_stereo_rx_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile ihs_stereo_speaker_stereo_rx_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = ihs_stereo_speaker_stereo_rx_settings,
	.setting_sz = ARRAY_SIZE(ihs_stereo_speaker_stereo_rx_settings),
};
/* baikal ID 0009676::srinivas.mittapalli@lge.com 11Oct2010 */
#ifndef LGE_CUST_CAL
static 
#endif
 struct snddev_icodec_data snddev_ihs_stereo_speaker_stereo_rx_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "headset_stereo_speaker_stereo_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HEADSET_STEREO_PLUS_SPKR_STEREO_RX,
	.profile = &ihs_stereo_speaker_stereo_rx_profile,
	.channel_mode = 2,
	.default_sample_rate = 48000,
//for sonification stream need to turn on both headset & speaker. 
// ++ kiran.kanneganti	
#if defined (CONFIG_MACH_LGE_FLIP)
	.pamp_on = &lge_snddev_spk_hs_amp_on,
	.pamp_off = &lge_snddev_amp_off,
#else
	.pamp_on = &msm_snddev_poweramp_on,
	.pamp_off = &msm_snddev_poweramp_off,
#endif
#if defined (CONFIG_MACH_LGE_FLIP)
	.voltage_on = NULL,
	.voltage_off = NULL,
#else
	.voltage_on = msm_snddev_hsed_voltage_on,
	.voltage_off = msm_snddev_hsed_voltage_off,
#endif	
// -- kiran.kanneganti
	.max_voice_rx_vol[VOC_NB_INDEX] = -500,
	.min_voice_rx_vol[VOC_NB_INDEX] = -2000,
	.max_voice_rx_vol[VOC_WB_INDEX] = -500,
	.min_voice_rx_vol[VOC_WB_INDEX] = -2000,
};

static struct platform_device msm_ihs_stereo_speaker_stereo_rx_device = {
	.name = "snddev_icodec",
	.id = 21,
	.dev = { .platform_data = &snddev_ihs_stereo_speaker_stereo_rx_data },
};

/////////////////////////////////////////////////////////////////////////////////////////
// HDMI RX
/////////////////////////////////////////////////////////////////////////////////////////

static struct snddev_mi2s_data snddev_mi2s_stereo_rx_data = {
	.capability = SNDDEV_CAP_RX ,
	.name = "hdmi_stereo_rx",
	.copp_id = 3,
	.acdb_id = ACDB_ID_HDMI,
	.channel_mode = 2,
	.sd_lines = MI2S_SD_0,
	.route = msm_snddev_tx_route_config,
	.deroute = msm_snddev_tx_route_deconfig,
	.default_sample_rate = 48000,
};

static struct platform_device msm_snddev_mi2s_stereo_rx_device = {
	.name = "snddev_mi2s",
	.id = 0,
	.dev = { .platform_data = &snddev_mi2s_stereo_rx_data },
};

/////////////////////////////////////////////////////////////////////////////////////////
// FM TX : don't care
/////////////////////////////////////////////////////////////////////////////////////////

static struct snddev_mi2s_data snddev_mi2s_fm_tx_data = {
	.capability = SNDDEV_CAP_TX ,
	.name = "fmradio_stereo_tx",
	.copp_id = 2,
	.acdb_id = ACDB_ID_FM_TX,
	.channel_mode = 2,
	.sd_lines = MI2S_SD_3,
	.route = NULL,
	.deroute = NULL,
	.default_sample_rate = 48000,
};

static struct platform_device  msm_snddev_mi2s_fm_tx_device = {
	.name = "snddev_mi2s",
	.id = 1,
	.dev = { .platform_data = &snddev_mi2s_fm_tx_data},
};

static struct snddev_icodec_data snddev_fluid_imic_tx_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "handset_tx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_SPKR_PHONE_MIC,
	.profile = &ispeaker_tx_profile,
	.channel_mode = 1,
	.pmctl_id = ispk_pmctl_id,
	.pmctl_id_sz = ARRAY_SIZE(ispk_pmctl_id),
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_tx_route_config,
	.pamp_off = msm_snddev_tx_route_deconfig,
};

static struct platform_device msm_fluid_imic_tx_device = {
	.name = "snddev_icodec",
	.id = 22,
	.dev = { .platform_data = &snddev_fluid_imic_tx_data },
};

/////////////////////////////////////////////////////////////////////////////////////////
// fluid RX : don't care
/////////////////////////////////////////////////////////////////////////////////////////

static struct snddev_icodec_data snddev_fluid_iearpiece_rx_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "handset_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_SPKR_PHONE_STEREO,
	.profile = &ispeaker_rx_profile,
	.channel_mode = 2,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.default_sample_rate = 48000,
	.pamp_on = &msm_snddev_poweramp_on,
	.pamp_off = &msm_snddev_poweramp_off,
	.max_voice_rx_vol[VOC_NB_INDEX] = -500,
	.min_voice_rx_vol[VOC_NB_INDEX] = -1000,
	.max_voice_rx_vol[VOC_WB_INDEX] = -500,
	.min_voice_rx_vol[VOC_WB_INDEX] = -1000,
};

static struct platform_device msm_fluid_iearpeice_rx_device = {
	.name = "snddev_icodec",
	.id = 23,
	.dev = { .platform_data = &snddev_fluid_iearpiece_rx_data },
};

/////////////////////////////////////////////////////////////////////////////////////////
// Fluid dual mic
/////////////////////////////////////////////////////////////////////////////////////////

static struct adie_codec_action_unit fluid_idual_mic_ef_8KHz_osr256_actions[] =
	MIC1_LEFT_AUX_IN_RIGHT_8000_OSR_256;

static struct adie_codec_hwsetting_entry fluid_idual_mic_endfire_settings[] = {
	{
		.freq_plan = 8000,
		.osr = 256,
		.actions = fluid_idual_mic_ef_8KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(fluid_idual_mic_ef_8KHz_osr256_actions),
	}, /* 8KHz profile can be used for 16KHz */
	{
		.freq_plan = 16000,
		.osr = 256,
		.actions = fluid_idual_mic_ef_8KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(fluid_idual_mic_ef_8KHz_osr256_actions),
	}, /* 8KHz profile can also be used for 48KHz */
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = fluid_idual_mic_ef_8KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(fluid_idual_mic_ef_8KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile fluid_idual_mic_endfire_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = fluid_idual_mic_endfire_settings,
	.setting_sz = ARRAY_SIZE(fluid_idual_mic_endfire_settings),
};

static enum hsed_controller fluid_idual_mic_endfire_pmctl_id[] = {
	PM_HSED_CONTROLLER_0, PM_HSED_CONTROLLER_2
};

static struct snddev_icodec_data snddev_fluid_idual_mic_endfire_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "handset_dual_mic_endfire_tx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_SPKR_PHONE_MIC_ENDFIRE,
	.profile = &fluid_idual_mic_endfire_profile,
	.channel_mode = 2,
	.default_sample_rate = 48000,
	.pmctl_id = fluid_idual_mic_endfire_pmctl_id,
	.pmctl_id_sz = ARRAY_SIZE(fluid_idual_mic_endfire_pmctl_id),
	.pamp_on = msm_snddev_tx_route_config,
	.pamp_off = msm_snddev_tx_route_deconfig,
};

static struct platform_device msm_fluid_idual_mic_endfire_device = {
	.name = "snddev_icodec",
	.id = 24,
	.dev = { .platform_data = &snddev_fluid_idual_mic_endfire_data },
};

/////////////////////////////////////////////////////////////////////////////////////////
// Fluid dual mic speaker
/////////////////////////////////////////////////////////////////////////////////////////

static struct snddev_icodec_data snddev_fluid_spk_idual_mic_endfire_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "speaker_dual_mic_endfire_tx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_SPKR_PHONE_MIC_ENDFIRE,
	.profile = &fluid_idual_mic_endfire_profile,
	.channel_mode = 2,
	.default_sample_rate = 48000,
	.pmctl_id = fluid_idual_mic_endfire_pmctl_id,
	.pmctl_id_sz = ARRAY_SIZE(fluid_idual_mic_endfire_pmctl_id),
	.pamp_on = msm_snddev_tx_route_config,
	.pamp_off = msm_snddev_tx_route_deconfig,
};

static struct platform_device msm_fluid_spk_idual_mic_endfire_device = {
	.name = "snddev_icodec",
	.id = 25,
	.dev = { .platform_data = &snddev_fluid_spk_idual_mic_endfire_data },
};

/////////////////////////////////////////////////////////////////////////////////////////
// BT A2DP TX/RX
/////////////////////////////////////////////////////////////////////////////////////////

static struct snddev_virtual_data snddev_a2dp_tx_data = {
	.capability = SNDDEV_CAP_TX,
	.name = "a2dp_tx",
	.copp_id = 5,
	.acdb_id = PSEUDO_ACDB_ID,
};

static struct snddev_virtual_data snddev_a2dp_rx_data = {
	.capability = SNDDEV_CAP_RX,
	.name = "a2dp_rx",
	.copp_id = 2,
	.acdb_id = PSEUDO_ACDB_ID,
};

static struct platform_device msm_a2dp_rx_device = {
	.name = "snddev_virtual",
	.id = 0,
	.dev = { .platform_data = &snddev_a2dp_rx_data },
};

static struct platform_device msm_a2dp_tx_device = {
	.name = "snddev_virtual",
	.id = 1,
	.dev = { .platform_data = &snddev_a2dp_tx_data },
};

/////////////////////////////////////////////////////////////////////////////////////////
// Virtual : not used : don't care
/////////////////////////////////////////////////////////////////////////////////////////

static struct snddev_virtual_data snddev_uplink_rx_data = {
	.capability = SNDDEV_CAP_RX,
	.name = "uplink_rx",
	.copp_id = 5,
	.acdb_id = PSEUDO_ACDB_ID,
};

static struct platform_device msm_uplink_rx_device = {
	.name = "snddev_virtual",
	.id = 2,
	.dev = { .platform_data = &snddev_uplink_rx_data },
};

/////////////////////////////////////////////////////////////////////////////////////////
/* Configurations list */
/////////////////////////////////////////////////////////////////////////////////////////

static struct platform_device *snd_devices_ffa[] __initdata = {
	&msm_iearpiece_ffa_device,
	&msm_imic_ffa_device,
	&msm_ifmradio_handset_device,
	&msm_ihs_ffa_stereo_rx_device,
	&msm_ihs_ffa_mono_rx_device,
	&msm_ihs_mono_tx_device,
	&msm_bt_sco_earpiece_device,
	&msm_bt_sco_mic_device,
	&msm_ispeaker_rx_device,
	&msm_ifmradio_speaker_device,
	&msm_ifmradio_ffa_headset_device,
	&msm_idual_mic_endfire_device,
	&msm_idual_mic_broadside_device,
	&msm_spk_idual_mic_endfire_device,
	&msm_spk_idual_mic_broadside_device,
	&msm_itty_hs_mono_tx_device,
	&msm_itty_hs_mono_rx_device,
	&msm_ispeaker_tx_device,
	&msm_ihs_stereo_speaker_stereo_rx_device,
	&msm_a2dp_rx_device,
	&msm_a2dp_tx_device,
	&msm_snddev_mi2s_stereo_rx_device,
	&msm_snddev_mi2s_fm_tx_device,
	&msm_uplink_rx_device,
	&msm_real_stereo_tx_device,
};

// Bryce uses this config
static struct platform_device *snd_devices_surf[] __initdata = {
	&msm_iearpiece_device,
	&msm_imic_device,
	&msm_ihs_stereo_rx_device,
	&msm_ihs_mono_rx_device,
	&msm_ihs_mono_tx_device,
	&msm_bt_sco_earpiece_device,
	&msm_bt_sco_mic_device,
	&msm_ifmradio_handset_device,
	&msm_ispeaker_rx_device,
	&msm_ifmradio_speaker_device,
	&msm_ifmradio_headset_device,
	&msm_itty_hs_mono_tx_device,
	&msm_itty_hs_mono_rx_device,
	&msm_ispeaker_tx_device,
	&msm_ihs_stereo_speaker_stereo_rx_device,
	&msm_a2dp_rx_device,
	&msm_a2dp_tx_device,
	&msm_snddev_mi2s_stereo_rx_device,
	&msm_snddev_mi2s_fm_tx_device,
	&msm_uplink_rx_device,
// SRI_START Adding new device 25th Aug ,2010
// Audience device configuration
#if defined (CONFIG_MACH_LGE_FLIP)
	&msm_iearpiece_audience_device,
	&msm_imic_audience_device,
	&msm_ispeaker_audience_rx_device,
	&msm_ispeaker_audience_tx_device,
#endif
// SRI_END Adding new device 25th Aug,2010
/* kiran.kanneganti@lge.com 03 Sep 2010 ++ */
#if defined (CONFIG_MACH_LGE_FLIP)
/* Aux in mic Right & mic 1 left QCOM DUAL MIC device */
	&msm_idual_mic_endfire_device,
	&msm_spk_idual_mic_endfire_device,
#endif
/* kiran.kanneganti@lge.com 03 Sep 2010 -- */
};

static struct platform_device *snd_devices_fluid[] __initdata = {
	&msm_ihs_stereo_rx_device,
	&msm_ihs_mono_rx_device,
	&msm_ihs_mono_tx_device,
	&msm_ispeaker_rx_device,
	&msm_ispeaker_tx_device,
	&msm_fluid_imic_tx_device,
	&msm_fluid_iearpeice_rx_device,
	&msm_fluid_idual_mic_endfire_device,
	&msm_fluid_spk_idual_mic_endfire_device,
	&msm_a2dp_rx_device,
	&msm_a2dp_tx_device,
	&msm_snddev_mi2s_stereo_rx_device,
	&msm_uplink_rx_device,
	&msm_ifmradio_speaker_device,
	&msm_ifmradio_headset_device,
};

#ifdef CONFIG_DEBUG_FS
static void snddev_hsed_config_modify_setting(int type)
{
	struct platform_device *device;
	struct snddev_icodec_data *icodec_data;

	device = &msm_ihs_ffa_stereo_rx_device;
	icodec_data = (struct snddev_icodec_data *)device->dev.platform_data;

	if (icodec_data) {
		if (type == 1) {
			icodec_data->voltage_on = NULL;
			icodec_data->voltage_off = NULL;
			icodec_data->profile->settings =
				ihs_ffa_stereo_rx_class_d_legacy_settings;
			icodec_data->profile->setting_sz =
			ARRAY_SIZE(ihs_ffa_stereo_rx_class_d_legacy_settings);
		} else if (type == 2) {
			icodec_data->voltage_on = NULL;
			icodec_data->voltage_off = NULL;
			icodec_data->profile->settings =
				ihs_ffa_stereo_rx_class_ab_legacy_settings;
			icodec_data->profile->setting_sz =
			ARRAY_SIZE(ihs_ffa_stereo_rx_class_ab_legacy_settings);
		}
	}
}

static void snddev_hsed_config_restore_setting(void)
{
	struct platform_device *device;
	struct snddev_icodec_data *icodec_data;

	device = &msm_ihs_ffa_stereo_rx_device;
	icodec_data = (struct snddev_icodec_data *)device->dev.platform_data;

	if (icodec_data) {
		icodec_data->voltage_on = msm_snddev_hsed_voltage_on;
		icodec_data->voltage_off = msm_snddev_hsed_voltage_off;
		icodec_data->profile->settings = ihs_ffa_stereo_rx_settings;
		icodec_data->profile->setting_sz =
			ARRAY_SIZE(ihs_ffa_stereo_rx_settings);
	}
}

static ssize_t snddev_hsed_config_debug_write(struct file *filp,
	const char __user *ubuf, size_t cnt, loff_t *ppos)
{
	char *lb_str = filp->private_data;
	char cmd;

	if (get_user(cmd, ubuf))
		return -EFAULT;

	if (!strcmp(lb_str, "msm_hsed_config")) {
		switch (cmd) {
		case '0':
			snddev_hsed_config_restore_setting();
			break;

		case '1':
			snddev_hsed_config_modify_setting(1);
			break;

		case '2':
			snddev_hsed_config_modify_setting(2);
			break;

		default:
			break;
		}
	}
	return cnt;
}

static int snddev_hsed_config_debug_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static const struct file_operations snddev_hsed_config_debug_fops = {
	.open = snddev_hsed_config_debug_open,
	.write = snddev_hsed_config_debug_write
};
#endif
//Baikal Id:0009963. Rx voice volume change support from testmode. kiran.kanneganti@lge.com
/*******************************************************************************
*	Function Name :  lge_get_audio_device_data_address
*	Args : inetrger variable address to get number of devices
*	dependencies : None. 
*	Note: check Audio_misc_ctrl.c regarding the usage.
********************************************************************************/
//Use to manipulate or read the device data.
//++kiran.kanneganti@lge.com
struct platform_device** lge_get_audio_device_data_address( int* Num_of_Dev)
{
	if (machine_is_lge_flip())
	{
		*Num_of_Dev = ARRAY_SIZE(snd_devices_surf);
		return snd_devices_surf;
	}
	else
		return NULL;
}
EXPORT_SYMBOL(lge_get_audio_device_data_address);
//--kiran.kanneganti@lge.com
/**********************************************************************/

void __init msm_snddev_init(void)
{
	if (machine_is_msm7x30_ffa() || machine_is_msm8x55_ffa() ||
		machine_is_msm8x55_svlte_ffa()) {
		platform_add_devices(snd_devices_ffa,
		ARRAY_SIZE(snd_devices_ffa));
#ifdef CONFIG_DEBUG_FS
		debugfs_hsed_config = debugfs_create_file("msm_hsed_config",
					S_IFREG | S_IRUGO, NULL,
		(void *) "msm_hsed_config", &snddev_hsed_config_debug_fops);
#endif
	} else if (machine_is_msm7x30_surf() || machine_is_msm8x55_surf() ||
		machine_is_msm8x55_svlte_surf())
		platform_add_devices(snd_devices_surf,
		ARRAY_SIZE(snd_devices_surf));
/* CONFIG_MACH_LGE_FLIP	ehgrace.kim 10.05.03
	add LGE amp
*/
#ifdef CONFIG_MACH_LGE_FLIP
	else if (machine_is_lge_flip())
		platform_add_devices(snd_devices_surf,
		ARRAY_SIZE(snd_devices_surf));
#endif
	else if (machine_is_msm7x30_fluid())
		platform_add_devices(snd_devices_fluid,
		ARRAY_SIZE(snd_devices_fluid));
	else
		pr_err("%s: Unknown machine type\n", __func__);
}
#else
// mic input to AUX
// 0x0D, 0xFF, 0xC1
#define AUX_TX_48000_OSR_256 \
	{{ADIE_CODEC_ACTION_STAGE_REACHED, ADIE_CODEC_FLASH_IMAGE}, \
	{ADIE_CODEC_ACTION_ENTRY, ADIE_CODEC_PACK_ENTRY(0x80, 0x01, 0x01)}, \
	{ADIE_CODEC_ACTION_ENTRY, ADIE_CODEC_PACK_ENTRY(0x80, 0x01, 0x00)}, \
	{ADIE_CODEC_ACTION_ENTRY, ADIE_CODEC_PACK_ENTRY(0x8A, 0x30, 0x30)}, \
	{ADIE_CODEC_ACTION_ENTRY, ADIE_CODEC_PACK_ENTRY(0x11, 0xfc, 0xfc)}, \
	{ADIE_CODEC_ACTION_ENTRY, ADIE_CODEC_PACK_ENTRY(0x13, 0xfc, 0x58)}, \
	{ADIE_CODEC_ACTION_ENTRY, ADIE_CODEC_PACK_ENTRY(0x14, 0xff, 0x65)}, \
	{ADIE_CODEC_ACTION_ENTRY, ADIE_CODEC_PACK_ENTRY(0x15, 0xff, 0x64)}, \
	{ADIE_CODEC_ACTION_ENTRY, ADIE_CODEC_PACK_ENTRY(0x82, 0xff, 0x5A)}, \
	{ADIE_CODEC_ACTION_ENTRY, ADIE_CODEC_PACK_ENTRY(0x10, 0xFF, 0x68)}, \
	{ADIE_CODEC_ACTION_STAGE_REACHED, ADIE_CODEC_DIGITAL_READY}, \
	{ADIE_CODEC_ACTION_ENTRY, ADIE_CODEC_PACK_ENTRY(0x0D, 0xFF, 0xC1)}, \
	{ADIE_CODEC_ACTION_DELAY_WAIT, 0xbb8}, \
	{ADIE_CODEC_ACTION_ENTRY, ADIE_CODEC_PACK_ENTRY(0x83, 0x14, 0x14)}, \
	{ADIE_CODEC_ACTION_ENTRY, ADIE_CODEC_PACK_ENTRY(0x8b, 0xff, 0xAC)}, \
	{ADIE_CODEC_ACTION_ENTRY, ADIE_CODEC_PACK_ENTRY(0x8c, 0x03, 0x02)}, \
	{ADIE_CODEC_ACTION_ENTRY, ADIE_CODEC_PACK_ENTRY(0x86, 0xff, 0x0A)}, \
	{ADIE_CODEC_ACTION_ENTRY, ADIE_CODEC_PACK_ENTRY(0x8A, 0x50, 0x40)}, \
	{ADIE_CODEC_ACTION_STAGE_REACHED, ADIE_CODEC_DIGITAL_ANALOG_READY}, \
	{ADIE_CODEC_ACTION_ENTRY, ADIE_CODEC_PACK_ENTRY(0x8A, 0x10, 0x30)}, \
	{ADIE_CODEC_ACTION_ENTRY, ADIE_CODEC_PACK_ENTRY(0x0D, 0xFF, 0x00)}, \
	{ADIE_CODEC_ACTION_ENTRY, ADIE_CODEC_PACK_ENTRY(0x83, 0x14, 0x00)}, \
	{ADIE_CODEC_ACTION_STAGE_REACHED, ADIE_CODEC_ANALOG_OFF}, \
	{ADIE_CODEC_ACTION_ENTRY, ADIE_CODEC_PACK_ENTRY(0x11, 0xff, 0x00)}, \
	{ADIE_CODEC_ACTION_STAGE_REACHED, ADIE_CODEC_DIGITAL_OFF} }

#define CHANNEL_MODE_MONO 1
#define CHANNEL_MODE_STEREO 2

//***************************************************************************************
//      RX VOICE PATH
//***************************************************************************************

//=======================================================================================
// Handset mono RX (voice call through receiver)    DEVICE_HANDSET_VOICE_RX 
//=======================================================================================
static struct adie_codec_action_unit earpiece_voice_rx_actions[] =
	HANDSET_RX_48000_OSR_256;

static struct adie_codec_hwsetting_entry earpiece_voice_rx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = earpiece_voice_rx_actions,
		.action_sz = ARRAY_SIZE(earpiece_voice_rx_actions),
	}
};

static struct adie_codec_dev_profile earpiece_voice_rx_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = earpiece_voice_rx_settings,
	.setting_sz = ARRAY_SIZE(earpiece_voice_rx_settings),
};

struct snddev_icodec_data earpiece_voice_rx_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "handset_voice_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HANDSET_SPKR,
	.profile = &earpiece_voice_rx_profile,
	.channel_mode = CHANNEL_MODE_MONO,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.default_sample_rate = 48000,
	.pamp_on = NULL,
	.pamp_off = NULL,
	.max_voice_rx_vol[VOC_NB_INDEX] = 0,
	.min_voice_rx_vol[VOC_NB_INDEX] = -2400,
	.max_voice_rx_vol[VOC_WB_INDEX] = 0,
	.min_voice_rx_vol[VOC_WB_INDEX] = -2400
};

static struct platform_device lge_device_earpiece_voice_rx = {
	.name = "snddev_icodec",
	.id = DEVICE_ID_INTERNAL_HANDSET_VOICE_RX,
	.dev = { .platform_data = &earpiece_voice_rx_data },
};
#if 0
//=======================================================================================
// Headset mono RX : (voice call through headset)    DEVICE_HEADSET_MONO_VOICE_RX
//=======================================================================================
static struct adie_codec_action_unit headset_mono_voice_rx_actions[] =
	HEADSET_RX_LEGACY_48000_OSR_256;

static struct adie_codec_hwsetting_entry headset_mono_voice_rx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = headset_mono_voice_rx_actions,
		.action_sz = ARRAY_SIZE(headset_mono_voice_rx_actions),
	}
};

static struct adie_codec_dev_profile headset_mono_voice_rx_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = headset_mono_voice_rx_settings,
	.setting_sz = ARRAY_SIZE(headset_mono_voice_rx_settings),
};

static struct snddev_icodec_data headset_mono_voice_rx_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "headset_mono_voice_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HEADSET_SPKR_MONO,
	.profile = &headset_mono_voice_rx_profile,
	.channel_mode = 1,
	.default_sample_rate = 48000,
	.pamp_on = &set_amp_headset_stereo_voice,
	.pamp_off = &set_amp_PowerDown,
	.max_voice_rx_vol[VOC_NB_INDEX] = -1200,
	.min_voice_rx_vol[VOC_NB_INDEX] = -2700,
	.max_voice_rx_vol[VOC_WB_INDEX] = -1200,
	.min_voice_rx_vol[VOC_WB_INDEX] = -2700,
};

static struct platform_device lge_device_headset_mono_voice_rx = {
	.name = "snddev_icodec",
	.id = 1,
	.dev = { .platform_data = &headset_mono_voice_rx_data },
};
#endif

//=======================================================================================
// Headset stereo RX : (voice call through headset)    DEVICE_HEADSET_STEREO_VOICE_RX
//=======================================================================================
static struct adie_codec_action_unit headset_mono_voice_rx_actions[] =
	HEADSET_MONO_RX_LEGACY_48000_OSR_256;

static struct adie_codec_hwsetting_entry headset_mono_voice_rx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = headset_mono_voice_rx_actions,
		.action_sz = ARRAY_SIZE(headset_mono_voice_rx_actions),
	}
};

static struct adie_codec_dev_profile headset_mono_voice_rx_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = headset_mono_voice_rx_settings,
	.setting_sz = ARRAY_SIZE(headset_mono_voice_rx_settings),
};

struct snddev_icodec_data headset_mono_voice_rx_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "headset_stereo_voice_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HEADSET_SPKR_MONO,
	.profile = &headset_mono_voice_rx_profile,
	.channel_mode = CHANNEL_MODE_MONO,
	.default_sample_rate = 48000,
	.pamp_on = &set_amp_headset_mono_voice,
	.pamp_off = &set_amp_PowerDown,
	.max_voice_rx_vol[VOC_NB_INDEX] = -100,
	.min_voice_rx_vol[VOC_NB_INDEX] = -1600,
	.max_voice_rx_vol[VOC_WB_INDEX] = -100,
	.min_voice_rx_vol[VOC_WB_INDEX] = -1600
};

static struct platform_device lge_device_headset_mono_voice_rx = {
	.name = "snddev_icodec",
	.id = DEVICE_ID_INTERNAL_HEADSET_VOICE_RX,
	.dev = { .platform_data = &headset_mono_voice_rx_data },
};
#if 0
//=======================================================================================
// Speaker mono RX : (voice call through speaker)    DEVICE_SPEAKER_MONO_VOICE_RX
//=======================================================================================
static struct adie_codec_action_unit speaker_mono_voice_rx_actions[] =
   SPEAKER_RX_48000_OSR_256;

static struct adie_codec_hwsetting_entry speaker_mono_voice_rx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = speaker_mono_voice_rx_actions,
		.action_sz = ARRAY_SIZE(speaker_mono_voice_rx_actions),
	}
};

static struct adie_codec_dev_profile speaker_mono_voice_rx_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = speaker_mono_voice_rx_settings,
	.setting_sz = ARRAY_SIZE(speaker_mono_voice_rx_settings),
};

static struct snddev_icodec_data speaker_mono_voice_rx_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "speaker_mono_voice_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_SPKR_PHONE_STEREO,
	.profile = &speaker_mono_voice_rx_profile,
	.channel_mode = 2,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.default_sample_rate = 48000,
	.pamp_on = &set_amp_speaker_mono_voice,
	.pamp_off = &set_amp_PowerDown,
	.max_voice_rx_vol[VOC_NB_INDEX] = 1000,
	.min_voice_rx_vol[VOC_NB_INDEX] = -500,
	.max_voice_rx_vol[VOC_WB_INDEX] = 1000,
	.min_voice_rx_vol[VOC_WB_INDEX] = -500,
};

static struct platform_device lge_device_speaker_mono_voice_rx = {
	.name = "snddev_icodec",
	.id = 3,
	.dev = { .platform_data = &speaker_mono_voice_rx_data },
};
#endif
//=======================================================================================
// Speaker stereo RX : (voice call through speaker)    DEVICE_SPEAKER_STEREO_VOICE_RX
//=======================================================================================
static struct adie_codec_action_unit speaker_stereo_voice_rx_actions[] =
   SPEAKER_STEREO_RX_48000_OSR_256;

static struct adie_codec_hwsetting_entry speaker_stereo_voice_rx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = speaker_stereo_voice_rx_actions,
		.action_sz = ARRAY_SIZE(speaker_stereo_voice_rx_actions),
	}
};

static struct adie_codec_dev_profile speaker_stereo_voice_rx_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = speaker_stereo_voice_rx_settings,
	.setting_sz = ARRAY_SIZE(speaker_stereo_voice_rx_settings),
};

struct snddev_icodec_data speaker_stereo_voice_rx_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "speaker_stereo_voice_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_LGE_SPEAKER_STEREO_VOICE_RX,
	.profile = &speaker_stereo_voice_rx_profile,
	.channel_mode = CHANNEL_MODE_STEREO,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.default_sample_rate = 48000,
	.pamp_on = &set_amp_speaker_stereo_voice,
	.pamp_off = &set_amp_PowerDown,
	.max_voice_rx_vol[VOC_NB_INDEX] = 1000,
	.min_voice_rx_vol[VOC_NB_INDEX] = -1000,
	.max_voice_rx_vol[VOC_WB_INDEX] = 1000,
	.min_voice_rx_vol[VOC_WB_INDEX] = -1000,
};

static struct platform_device lge_device_speaker_stereo_voice_rx = {
	.name = "snddev_icodec",
	.id = DEVICE_ID_INTERNAL_SPEAKER_VOICE_RX,
	.dev = { .platform_data = &speaker_stereo_voice_rx_data },
};

struct snddev_icodec_data speaker_GAN_voice_rx_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "speaker_stereo_GAN_voice_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_LGE_SPEAKER_STEREO_VOICE_RX,
	.profile = &speaker_stereo_voice_rx_profile,
	.channel_mode = CHANNEL_MODE_STEREO,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.default_sample_rate = 48000,
	.pamp_on = &set_amp_speaker_GAN_voice,
	.pamp_off = &set_amp_PowerDown,
	.max_voice_rx_vol[VOC_NB_INDEX] = 1000,
	.min_voice_rx_vol[VOC_NB_INDEX] = -1000,
	.max_voice_rx_vol[VOC_WB_INDEX] = 1000,
	.min_voice_rx_vol[VOC_WB_INDEX] = -1000,
};

static struct platform_device lge_device_speaker_GAN_voice_rx = {
	.name = "snddev_icodec",
	.id = DEVICE_ID_INTERNAL_SPEAKER_GAN_VOICE_RX,
	.dev = { .platform_data = &speaker_GAN_voice_rx_data },
};


//=======================================================================================
// TTY RX : (TTY call)    DEVICE_TTY_RX
//=======================================================================================
static struct adie_codec_action_unit tty_rx_actions[] =
	HEADSET_MONO_RX_LEGACY_48000_OSR_256;

static struct adie_codec_hwsetting_entry tty_rx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = tty_rx_actions,
		.action_sz = ARRAY_SIZE(tty_rx_actions),
	}
};

static struct adie_codec_dev_profile tty_rx_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = tty_rx_settings,
	.setting_sz = ARRAY_SIZE(tty_rx_settings),
};

static struct snddev_icodec_data tty_rx_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE | SNDDEV_CAP_TTY),
	.name = "tty_headset_mono_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_TTY_HEADSET_SPKR,
	.profile = &tty_rx_profile,
	.channel_mode = CHANNEL_MODE_MONO,
	.default_sample_rate = 48000,
	.pamp_on = &set_amp_tty,
	.pamp_off = &set_amp_PowerDown,
	.max_voice_rx_vol[VOC_NB_INDEX] = -700,
	.min_voice_rx_vol[VOC_NB_INDEX] = -800,
	.max_voice_rx_vol[VOC_WB_INDEX] = -700,
	.min_voice_rx_vol[VOC_WB_INDEX] = -800
};

static struct platform_device lge_device_tty_rx = {
	.name = "snddev_icodec",
	.id = DEVICE_ID_INTERNAL_TTY_RX,
	.dev = { .platform_data = &tty_rx_data },
};
//=======================================================================================
// BT sco RX : (voice call through BT)    DEVICE_BT_SCO_VOICE_RX
//=======================================================================================
struct snddev_ecodec_data bt_sco_voice_rx_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "bt_sco_voice_rx",
	.copp_id = 1,
	.acdb_id = ACDB_ID_BT_SCO_SPKR,
	.channel_mode = CHANNEL_MODE_MONO,
	.conf_pcm_ctl_val = BT_SCO_PCM_CTL_VAL,
	.conf_aux_codec_intf = BT_SCO_AUX_CODEC_INTF,
	.conf_data_format_padding_val = BT_SCO_DATA_FORMAT_PADDING,
	.max_voice_rx_vol[VOC_NB_INDEX] = 400,
	.min_voice_rx_vol[VOC_NB_INDEX] = -1100,
	.max_voice_rx_vol[VOC_WB_INDEX] = 400,
	.min_voice_rx_vol[VOC_WB_INDEX] = -1100,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
};

struct platform_device lge_device_bt_sco_voice_rx = {
	.name = "msm_snddev_ecodec",
	.id = DEVICE_ID_EXTERNAL_BT_SCO_VOICE_RX,
	.dev = { .platform_data = &bt_sco_voice_rx_data },
};



//***************************************************************************************
//      RX AUDIO PATH
//***************************************************************************************

//=======================================================================================
// Handset mono RX (audio through receiver)    DEVICE_HANDSET_AUDIO_RX 
//=======================================================================================
static struct adie_codec_action_unit earpiece_audio_rx_actions[] =
	HANDSET_RX_48000_OSR_256_AUDIO;

static struct adie_codec_hwsetting_entry earpiece_audio_rx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = earpiece_audio_rx_actions,
		.action_sz = ARRAY_SIZE(earpiece_audio_rx_actions),
	}
};

static struct adie_codec_dev_profile earpiece_audio_rx_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = earpiece_audio_rx_settings,
	.setting_sz = ARRAY_SIZE(earpiece_audio_rx_settings),
};

static struct snddev_icodec_data earpiece_audio_rx_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "handset_audio_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_LGE_HANDSET_AUDIO_RX,
	.profile = &earpiece_audio_rx_profile,
	.channel_mode = CHANNEL_MODE_MONO,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.default_sample_rate = 48000,
	.pamp_on = NULL,
	.pamp_off = NULL,
	.max_voice_rx_vol[VOC_NB_INDEX] = -200,
	.min_voice_rx_vol[VOC_NB_INDEX] = -200,
	.max_voice_rx_vol[VOC_WB_INDEX] = -200,
	.min_voice_rx_vol[VOC_WB_INDEX] = -200,
};

static struct platform_device lge_device_earpiece_audio_rx = {
	.name = "snddev_icodec",
	.id = DEVICE_ID_INTERNAL_HANDSET_AUDIO_RX,
	.dev = { .platform_data = &earpiece_audio_rx_data },
};
#if 0
//=======================================================================================
// Headset mono RX : (audio through headset)    DEVICE_HEADSET_MONO_AUDIO_RX
//=======================================================================================
static struct adie_codec_action_unit headset_mono_audio_rx_actions[] =
	HEADSET_RX_LEGACY_48000_OSR_256;

static struct adie_codec_hwsetting_entry headset_mono_audio_rx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = headset_mono_audio_rx_actions,
		.action_sz = ARRAY_SIZE(headset_mono_audio_rx_actions),
	}
};

static struct adie_codec_dev_profile headset_mono_audio_rx_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = headset_mono_audio_rx_settings,
	.setting_sz = ARRAY_SIZE(headset_mono_audio_rx_settings),
};

static struct snddev_icodec_data headset_mono_audio_rx_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "headset_mono_audio_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HEADSET_SPKR_MONO,
	.profile = &headset_mono_audio_rx_profile,
	.channel_mode = 1,
	.default_sample_rate = 48000,
	.pamp_on = &set_amp_headset_mono_audio,
	.pamp_off = &set_amp_PowerDown,
	.max_voice_rx_vol[VOC_NB_INDEX] = -700,
	.min_voice_rx_vol[VOC_NB_INDEX] = -2200,
	.max_voice_rx_vol[VOC_WB_INDEX] = -900,
	.min_voice_rx_vol[VOC_WB_INDEX] = -2400,
};

static struct platform_device lge_device_headset_mono_audio_rx = {
	.name = "snddev_icodec",
	.id = 7,
	.dev = { .platform_data = &headset_mono_audio_rx_data },
};
#endif
//=======================================================================================
// Headset stereo RX : (audio through headset)    DEVICE_HEADSET_STEREO_AUDIO_RX
//=======================================================================================
static struct adie_codec_action_unit headset_stereo_audio_rx_actions[] =
	HEADSET_STEREO_RX_LEGACY_48000_OSR_256_AUDIO;

static struct adie_codec_hwsetting_entry headset_stereo_audio_rx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = headset_stereo_audio_rx_actions,
		.action_sz = ARRAY_SIZE(headset_stereo_audio_rx_actions),
	}
};

static struct adie_codec_dev_profile headset_stereo_audio_rx_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = headset_stereo_audio_rx_settings,
	.setting_sz = ARRAY_SIZE(headset_stereo_audio_rx_settings),
};

static struct snddev_icodec_data headset_stereo_audio_rx_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "headset_stereo_audio_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HEADSET_SPKR_STEREO,
	.profile = &headset_stereo_audio_rx_profile,
	.channel_mode = CHANNEL_MODE_STEREO,
	.default_sample_rate = 48000,
	.pamp_on = &set_amp_headset_stereo_audio,
	.pamp_off = &set_amp_PowerDown,
	.max_voice_rx_vol[VOC_NB_INDEX] = -700,
	.min_voice_rx_vol[VOC_NB_INDEX] = -700,
	.max_voice_rx_vol[VOC_WB_INDEX] = -700,
	.min_voice_rx_vol[VOC_WB_INDEX] = -700
};

static struct platform_device lge_device_headset_stereo_audio_rx = {
	.name = "snddev_icodec",
	.id = DEVICE_ID_INTERNAL_HEADSET_AUDIO_RX,
	.dev = { .platform_data = &headset_stereo_audio_rx_data },
};

#if 0
//=======================================================================================
// Speaker mono RX : (audio through speaker)    DEVICE_SPEAKER_MONO_AUDIO_RX
//=======================================================================================
static struct adie_codec_action_unit speaker_mono_audio_rx_actions[] =
   SPEAKER_RX_48000_OSR_256;

static struct adie_codec_hwsetting_entry speaker_mono_audio_rx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = speaker_mono_audio_rx_actions,
		.action_sz = ARRAY_SIZE(speaker_mono_audio_rx_actions),
	}
};

static struct adie_codec_dev_profile speaker_mono_audio_rx_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = speaker_mono_audio_rx_settings,
	.setting_sz = ARRAY_SIZE(speaker_mono_audio_rx_settings),
};

static struct snddev_icodec_data speaker_mono_audio_rx_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "speaker_mono_audio_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_SPKR_PHONE_STEREO,
	.profile = &speaker_mono_voice_rx_profile,
	.channel_mode = 2,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.default_sample_rate = 48000,
	.pamp_on = &set_amp_speaker_mono_audio,
	.pamp_off = &set_amp_PowerDown,
	.max_voice_rx_vol[VOC_NB_INDEX] = 1000,
	.min_voice_rx_vol[VOC_NB_INDEX] = -500,
	.max_voice_rx_vol[VOC_WB_INDEX] = 1000,
	.min_voice_rx_vol[VOC_WB_INDEX] = -500,
};

static struct platform_device lge_device_speaker_mono_audio_rx = {
	.name = "snddev_icodec",
	.id = 9,
	.dev = { .platform_data = &speaker_mono_audio_rx_data },
};
#endif

//=======================================================================================
// Speaker stereo RX : (audio call through speaker)    DEVICE_SPEAKER_STEREO_AUDIO_RX
//=======================================================================================
static struct adie_codec_action_unit speaker_stereo_audio_rx_actions[] =
   SPEAKER_STEREO_RX_48000_OSR_256_AUDIO;

static struct adie_codec_hwsetting_entry speaker_stereo_audio_rx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = speaker_stereo_audio_rx_actions,
		.action_sz = ARRAY_SIZE(speaker_stereo_audio_rx_actions),
	}
};

static struct adie_codec_dev_profile speaker_stereo_audio_rx_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = speaker_stereo_audio_rx_settings,
	.setting_sz = ARRAY_SIZE(speaker_stereo_audio_rx_settings),
};

static struct snddev_icodec_data speaker_stereo_audio_rx_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "speaker_stereo_audio_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_SPKR_PHONE_STEREO,
	.profile = &speaker_stereo_audio_rx_profile,
	.channel_mode = CHANNEL_MODE_STEREO,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.default_sample_rate = 48000,
	.pamp_on = &set_amp_speaker_stereo_audio,
	.pamp_off = &set_amp_PowerDown,
	.max_voice_rx_vol[VOC_NB_INDEX] = 500,
	.min_voice_rx_vol[VOC_NB_INDEX] = 500,
	.max_voice_rx_vol[VOC_WB_INDEX] = 500,
	.min_voice_rx_vol[VOC_WB_INDEX] =  500,
};

static struct platform_device lge_device_speaker_stereo_audio_rx = {
	.name = "snddev_icodec",
	.id = DEVICE_ID_INTERNAL_SPEAKER_AUDIO_RX,
	.dev = { .platform_data = &speaker_stereo_audio_rx_data },
};

//=========================================================================================================
// Headset / Speaker dual RX : (audio through headset and speaker)    DEVICE_HEADSET_SPEAKER_AUDIO_RX
//=========================================================================================================
static struct adie_codec_action_unit headset_speaker_audio_rx_actions[] =
	HEADSET_SPEAKER_STEREO_RX_LEGACY_48000_OSR_256_AUDIO;

static struct adie_codec_hwsetting_entry headset_speaker_audio_rx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = headset_speaker_audio_rx_actions,
		.action_sz =
		ARRAY_SIZE(headset_speaker_audio_rx_actions),
	}
};

static struct adie_codec_dev_profile headset_speaker_audio_rx_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = headset_speaker_audio_rx_settings,
	.setting_sz = ARRAY_SIZE(headset_speaker_audio_rx_settings),
};

static struct snddev_icodec_data headset_speaker_audio_rx_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "headset_speaker_stereo_rx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_LGE_HEADSET_SPEAKER_AUDIO_RX,
	.profile = &headset_speaker_audio_rx_profile,
	.channel_mode = CHANNEL_MODE_STEREO,
	.default_sample_rate = 48000,
	.pamp_on = &set_amp_headset_speaker_audio,
	.pamp_off = &set_amp_PowerDown,
	.voltage_on = NULL,
	.voltage_off = NULL,
	.max_voice_rx_vol[VOC_NB_INDEX] = -200,
	.min_voice_rx_vol[VOC_NB_INDEX] = -200,
	.max_voice_rx_vol[VOC_WB_INDEX] = -200,
	.min_voice_rx_vol[VOC_WB_INDEX] = -200,
};

static struct platform_device lge_device_headset_speaker_audio_rx = {
	.name = "snddev_icodec",
	.id = DEVICE_ID_INTERNAL_HEADSET_SPEAKER_RX,
	.dev = { .platform_data = &headset_speaker_audio_rx_data },
};

//=======================================================================================
// BT sco RX : (audio through BT)    DEVICE_BT_SCO_AUDIO_RX
//=======================================================================================
static struct snddev_ecodec_data bt_sco_audio_rx_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "bt_sco_audio_rx",
	.copp_id = 1,
	.acdb_id = ACDB_ID_LGE_BT_SCO_AUDIO_RX,
	.channel_mode = CHANNEL_MODE_MONO,
	.conf_pcm_ctl_val = BT_SCO_PCM_CTL_VAL,
	.conf_aux_codec_intf = BT_SCO_AUX_CODEC_INTF,
	.conf_data_format_padding_val = BT_SCO_DATA_FORMAT_PADDING,
	.max_voice_rx_vol[VOC_NB_INDEX] = 400,
	.min_voice_rx_vol[VOC_NB_INDEX] = -1100,
	.max_voice_rx_vol[VOC_WB_INDEX] = 400,
	.min_voice_rx_vol[VOC_WB_INDEX] = -1100,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
};

struct platform_device lge_device_bt_sco_audio_rx = {
	.name = "msm_snddev_ecodec",
	.id = DEVICE_ID_EXTERNAL_BT_SCO_AUDIO_RX,
	.dev = { .platform_data = &bt_sco_audio_rx_data },
};

//=======================================================================================
// BT A2DP RX : (audio through A2DP)    DEVICE_BT_A2DP_RX
//=======================================================================================
static struct snddev_virtual_data bt_a2dp_rx_data = {
	.capability = SNDDEV_CAP_RX,
	.name = "a2dp_rx",
	.copp_id = 2,
	.acdb_id = PSEUDO_ACDB_ID,
};

static struct platform_device lge_device_bt_a2dp_rx = {
	.name = "snddev_virtual",
	.id = 0,
	.dev = { .platform_data = &bt_a2dp_rx_data },
};

//***************************************************************************************
//      TX VOICE PATH
//***************************************************************************************

static enum hsed_controller handset_mic1_aux_pmctl_id[] = {PM_HSED_CONTROLLER_0};

//=======================================================================================
// Handset voice TX (voice call through main mic)    DEVICE_HANDSET_VOICE_TX 
//=======================================================================================
static struct adie_codec_action_unit handset_voice_tx_actions[] =
	HANDSET_TX_48000_OSR_256;

//HANDSET_TX_48000_OSR_256

static struct adie_codec_hwsetting_entry handset_voice_tx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = handset_voice_tx_actions,
		.action_sz = ARRAY_SIZE(handset_voice_tx_actions),
	}
};

static struct adie_codec_dev_profile handset_voice_tx_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = handset_voice_tx_settings,
	.setting_sz = ARRAY_SIZE(handset_voice_tx_settings),
};

static struct snddev_icodec_data handset_voice_tx_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "handset_voice_tx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HANDSET_MIC,  //ACDB_ID_HANDSET_MIC
	.profile = &handset_voice_tx_profile,
	.channel_mode = CHANNEL_MODE_MONO,
	.pmctl_id = handset_mic1_aux_pmctl_id,
	.pmctl_id_sz = ARRAY_SIZE(handset_mic1_aux_pmctl_id),
	.default_sample_rate = 48000,
};

static struct platform_device lge_device_handset_voice_tx = {
	.name = "snddev_icodec",
	.id = DEVICE_ID_INTERNAL_HANDSET_VOICE_TX,
	.dev = { .platform_data = &handset_voice_tx_data },
};

//=======================================================================================
// Headset voice TX (voice call through headset mic)    DEVICE_HEADSET_VOICE_TX 
//=======================================================================================
static struct adie_codec_action_unit headset_voice_tx_actions[] =
	HEADSET_MONO_TX_48000_OSR_256;

static struct adie_codec_hwsetting_entry headset_voice_tx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = headset_voice_tx_actions,
		.action_sz = ARRAY_SIZE(headset_voice_tx_actions),
	}
};

static struct adie_codec_dev_profile headset_voice_tx_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = headset_voice_tx_settings,
	.setting_sz = ARRAY_SIZE(headset_voice_tx_settings),
};

static struct snddev_icodec_data headset_voice_tx_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "headset_voice_tx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HEADSET_MIC,
	.profile = &headset_voice_tx_profile,
	.channel_mode = CHANNEL_MODE_MONO,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_tx_route_config,
	.pamp_off = msm_snddev_tx_route_deconfig,
};

static struct platform_device lge_device_headset_voice_tx = {
	.name = "snddev_icodec",
	.id = DEVICE_ID_INTERNAL_HEADSET_VOICE_TX,
	.dev = { .platform_data = &headset_voice_tx_data },
};

//=======================================================================================
// Speaker voice TX (voice call through aux mic)    DEVICE_SPEAKER_VOICE_TX 
//=======================================================================================
static struct adie_codec_action_unit speaker_voice_tx_actions[] =
	AUX_TX_48000_OSR_256;

static struct adie_codec_hwsetting_entry speaker_voice_tx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = speaker_voice_tx_actions,
		.action_sz = ARRAY_SIZE(speaker_voice_tx_actions),
	}
};

static struct adie_codec_dev_profile speaker_voice_tx_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = speaker_voice_tx_settings,
	.setting_sz = ARRAY_SIZE(speaker_voice_tx_settings),
};

static struct snddev_icodec_data speaker_voice_tx_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "speaker_voice_tx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_SPKR_PHONE_MIC,
	.profile = &speaker_voice_tx_profile,
	.channel_mode = CHANNEL_MODE_MONO,
	.pmctl_id = handset_mic1_aux_pmctl_id,
	.pmctl_id_sz = ARRAY_SIZE(handset_mic1_aux_pmctl_id),
	.default_sample_rate = 48000,
};

static struct platform_device lge_device_speaker_tx = {
	.name = "snddev_icodec",
	.id = DEVICE_ID_INTERNAL_SPEAKER_VOICE_TX,
	.dev = { .platform_data = &speaker_voice_tx_data },
};

//=======================================================================================
// TTY TX : (TTY call)    DEVICE_TTY_TX
//=======================================================================================
static struct adie_codec_action_unit tty_tx_actions[] =
	HEADSET_MONO_TX_48000_OSR_256;

static struct adie_codec_hwsetting_entry tty_tx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = tty_tx_actions,
		.action_sz = ARRAY_SIZE(tty_tx_actions),
	}
};

static struct adie_codec_dev_profile tty_tx_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = tty_tx_settings,
	.setting_sz = ARRAY_SIZE(tty_tx_settings),
};

static struct snddev_icodec_data tty_tx_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE | SNDDEV_CAP_TTY),
	.name = "tty_tx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_TTY_HEADSET_MIC,
	.profile = &tty_tx_profile,
	.channel_mode = CHANNEL_MODE_MONO,
	.default_sample_rate = 48000,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.pamp_on = NULL,
	.pamp_off = NULL,
};

static struct platform_device lge_device_tty_tx = {
	.name = "snddev_icodec",
	.id = DEVICE_ID_INTERNAL_TTY_TX,
	.dev = { .platform_data = &tty_tx_data },
};

//=======================================================================================
// BT sco TX : (voice call through bt sco)    DEVICE_BT_SCO_VOICE_TX
//=======================================================================================
static struct snddev_ecodec_data bt_sco_voice_tx_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "bt_sco_tx",
	.copp_id = 1,
	.acdb_id = ACDB_ID_BT_SCO_MIC,
	.channel_mode = CHANNEL_MODE_MONO,
	.conf_pcm_ctl_val = BT_SCO_PCM_CTL_VAL,
	.conf_aux_codec_intf = BT_SCO_AUX_CODEC_INTF,
	.conf_data_format_padding_val = BT_SCO_DATA_FORMAT_PADDING,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
};

struct platform_device lge_device_bt_sco_voice_tx = {
	.name = "msm_snddev_ecodec",
	.id = DEVICE_ID_EXTERNAL_BT_SCO_VOICE_TX,
	.dev = { .platform_data = &bt_sco_voice_tx_data },
};



//***************************************************************************************
//      TX AUDIO PATH
//***************************************************************************************

//=======================================================================================
// Handset rec TX (recording through main mic)    DEVICE_MIC1_REC_TX 
//=======================================================================================
static struct adie_codec_action_unit mic1_rec_tx_actions[] =
	SPEAKER_TX_48000_OSR_256_REC;

static struct adie_codec_hwsetting_entry mic1_rec_tx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = mic1_rec_tx_actions,
		.action_sz = ARRAY_SIZE(mic1_rec_tx_actions),
	}
};

static struct adie_codec_dev_profile mic1_rec_tx_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = mic1_rec_tx_settings,
	.setting_sz = ARRAY_SIZE(mic1_rec_tx_settings),
};

static struct snddev_icodec_data mic1_rec_tx_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "mic1_rec_tx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_LGE_MIC1_REC_TX,
	.profile = &mic1_rec_tx_profile,
	.channel_mode = CHANNEL_MODE_MONO,
	.pmctl_id = handset_mic1_aux_pmctl_id,
	.pmctl_id_sz = ARRAY_SIZE(handset_mic1_aux_pmctl_id),
	.default_sample_rate = 48000,
};

static struct platform_device lge_device_mic1_rec_tx = {
	.name = "snddev_icodec",
	.id = DEVICE_ID_INTERNAL_MIC1_REC,
	.dev = { .platform_data = &mic1_rec_tx_data },
};

//===========================================================================

static struct adie_codec_action_unit cam_rec_tx_actions[] =
	SPEAKER_TX_48000_OSR_256_CAM_REC;

static struct adie_codec_hwsetting_entry cam_rec_tx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = cam_rec_tx_actions,
		.action_sz = ARRAY_SIZE(cam_rec_tx_actions),
	}
};

static struct adie_codec_dev_profile cam_rec_tx_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = cam_rec_tx_settings,
	.setting_sz = ARRAY_SIZE(cam_rec_tx_settings),
};

static struct snddev_icodec_data cam_rec_tx_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "cam_rec_tx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_LGE_MIC1_REC_TX,
	.profile = &cam_rec_tx_profile,
	.channel_mode = CHANNEL_MODE_MONO,
	.pmctl_id = handset_mic1_aux_pmctl_id,
	.pmctl_id_sz = ARRAY_SIZE(handset_mic1_aux_pmctl_id),
	.default_sample_rate = 48000,
};

static struct platform_device lge_device_cam_rec_tx = {
	.name = "snddev_icodec",
	.id = DEVICE_ID_INTERNAL_CAM_REC,
	.dev = { .platform_data = &cam_rec_tx_data },
};

//===========================================================================

static struct adie_codec_action_unit voice_rec_tx_actions[] =
	SPEAKER_TX_48000_OSR_256_VOICE_REC;

static struct adie_codec_hwsetting_entry voice_rec_tx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = voice_rec_tx_actions,
		.action_sz = ARRAY_SIZE(voice_rec_tx_actions),
	}
};

static struct adie_codec_dev_profile voice_rec_tx_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = voice_rec_tx_settings,
	.setting_sz = ARRAY_SIZE(voice_rec_tx_settings),
};

static struct snddev_icodec_data voice_rec_tx_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "voice_rec_tx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_LGE_MIC1_REC_TX,
	.profile = &voice_rec_tx_profile,
	.channel_mode = CHANNEL_MODE_MONO,
	.pmctl_id = handset_mic1_aux_pmctl_id,
	.pmctl_id_sz = ARRAY_SIZE(handset_mic1_aux_pmctl_id),
	.default_sample_rate = 48000,
};

static struct platform_device lge_device_voice_rec_tx = {
	.name = "snddev_icodec",
	.id = DEVICE_ID_INTERNAL_VOICE_REC,
	.dev = { .platform_data = &voice_rec_tx_data },
};

/*
//=======================================================================================
// Handset rec TX (recording through aux mic)    DEVICE_AUX_REC_TX
//=======================================================================================
static struct adie_codec_action_unit auxmic_rec_tx_actions[] =
	AUX_TX_48000_OSR_256;

static struct adie_codec_hwsetting_entry auxmic_rec_tx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = auxmic_rec_tx_actions,
		.action_sz = ARRAY_SIZE(auxmic_rec_tx_actions),
	}
};

static struct adie_codec_dev_profile auxmic_rec_tx_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = auxmic_rec_tx_settings,
	.setting_sz = ARRAY_SIZE(auxmic_rec_tx_settings),
};

static struct snddev_icodec_data auxmic_rec_tx_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "aux_rec_tx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_LGE_AUX_REC_TX,
	.profile = &auxmic_rec_tx_profile,
	.channel_mode = 1,
	.pmctl_id = handset_aux_pmctl_id,
	.pmctl_id_sz = ARRAY_SIZE(handset_aux_pmctl_id),
	.default_sample_rate = 48000,
};

static struct platform_device lge_device_auxmic_rec_tx = {
	.name = "snddev_icodec",
	.id = 17,
	.dev = { .platform_data = &auxmic_rec_tx_data },
};

//=======================================================================================
// Headset audio TX (recording through headset mic)    DEVICE_HEADSET_REC_TX 
//=======================================================================================
static struct adie_codec_action_unit headset_rec_tx_actions[] =
	HEADSET_MONO_TX_48000_OSR_256;

static struct adie_codec_hwsetting_entry headset_rec_tx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = headset_rec_tx_actions,
		.action_sz = ARRAY_SIZE(headset_rec_tx_actions),
	}
};

static struct adie_codec_dev_profile headset_rec_tx_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = headset_rec_tx_settings,
	.setting_sz = ARRAY_SIZE(headset_rec_tx_settings),
};

static struct snddev_icodec_data headset_rec_tx_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "headset_rec_tx",
	.copp_id = 0,
	.acdb_id = ACDB_ID_HEADSET_MIC,
	.profile = &headset_rec_tx_profile,
	.channel_mode = 1,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_tx_route_config,
	.pamp_off = msm_snddev_tx_route_deconfig,
};

static struct platform_device lge_device_headset_rec_tx = {
	.name = "snddev_icodec",
	.id = 18,
	.dev = { .platform_data = &headset_rec_tx_data },
};
//=======================================================================================
// BT sco audio TX : (recording through bt sco)    DEVICE_BT_SCO_REC_TX
//=======================================================================================
static struct snddev_ecodec_data bt_sco_rec_tx_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "bt_sco_rec_tx",
	.copp_id = 1,
	.acdb_id = ACDB_ID_BT_SCO_MIC,
	.channel_mode = 1,
	.conf_pcm_ctl_val = BT_SCO_PCM_CTL_VAL,
	.conf_aux_codec_intf = BT_SCO_AUX_CODEC_INTF,
	.conf_data_format_padding_val = BT_SCO_DATA_FORMAT_PADDING,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
};

struct platform_device lge_device_bt_sco_rec_tx = {
	.name = "msm_snddev_ecodec",
	.id = 3,
	.dev = { .platform_data = &bt_sco_rec_tx_data },
};
*/

/////////////////////////////////////////////////////////////////////////////////////////
/* Configurations list */
/////////////////////////////////////////////////////////////////////////////////////////
// Bryce uses this config
static struct platform_device *lge_snd_devices[] __initdata = {
  &lge_device_earpiece_voice_rx,
  &lge_device_headset_mono_voice_rx,
  &lge_device_speaker_stereo_voice_rx,
  &lge_device_tty_rx,
  &lge_device_bt_sco_voice_rx,
  &lge_device_earpiece_audio_rx,
  &lge_device_headset_stereo_audio_rx,
  &lge_device_speaker_stereo_audio_rx,
  &lge_device_headset_speaker_audio_rx,
  &lge_device_bt_sco_audio_rx,
  &lge_device_bt_a2dp_rx,
  &lge_device_handset_voice_tx,
  &lge_device_headset_voice_tx,
  &lge_device_speaker_tx,
  &lge_device_tty_tx,
  &lge_device_bt_sco_voice_tx,
  &lge_device_mic1_rec_tx,
  &lge_device_cam_rec_tx,
  &lge_device_voice_rec_tx,
  &lge_device_speaker_GAN_voice_rx
}; 

struct adie_codec_action_unit *codec_cal[] = {
  earpiece_voice_rx_actions,
  headset_mono_voice_rx_actions,
  speaker_stereo_voice_rx_actions,
  tty_rx_actions,
  headset_stereo_audio_rx_actions,
  headset_speaker_audio_rx_actions,
  speaker_stereo_audio_rx_actions,
  handset_voice_tx_actions,
  headset_voice_tx_actions,
  speaker_voice_tx_actions,
  tty_tx_actions,
  mic1_rec_tx_actions,
  cam_rec_tx_actions,
  voice_rec_tx_actions
};

void /*__init*/__refdata msm_snddev_init(void)
{
		platform_add_devices(lge_snd_devices, ARRAY_SIZE(lge_snd_devices));
//        lge_snddev_MSM_mic_route_config();
}
#endif
//LGE_UPDATE_E	taeho.youn@lge.com
