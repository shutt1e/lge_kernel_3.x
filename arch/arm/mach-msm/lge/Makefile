EXTRA_CFLAGS += -Iarch/arm/mach-msm

#LGE_UPDATE_S kideok.kim@lge.com 20110323 For lge_M7630QWERTY_gb (C800, C729)
ifeq ($(TARGET_PRODUCT),univaq_tmo_us)
obj-$(CONFIG_MACH_MSM8X55_UNIVA_Q) += board-univaq.o
obj-$(CONFIG_MACH_MSM8X55_UNIVA_Q) += board-univaq-pm.o
obj-$(CONFIG_MACH_MSM8X55_UNIVA_Q) += board-univaq-sound.o
obj-$(CONFIG_MACH_MSM8X55_UNIVA_Q) += board-univaq-misc.o
obj-$(CONFIG_MACH_MSM8X55_UNIVA_Q) += board-univaq-mmc.o
obj-$(CONFIG_MACH_MSM8X55_UNIVA_Q) += board-univaq-input.o

# LGE_UPDATE_S 20110215 kideok.kim@lge.com Univa-Q Board bring-up
# obj-$(CONFIG_MACH_MSM8X55_UNIVA_Q) += board-victor-panel.o
obj-$(CONFIG_MACH_MSM8X55_UNIVA_Q) += board-univaq-panel.o
# LGE_UPDATE_E 20110215 kideok.kim@lge.com Univa-Q Board bring-up

obj-$(CONFIG_MACH_MSM8X55_UNIVA_Q) += board-univaq-bt.o
obj-$(CONFIG_MACH_MSM8X55_UNIVA_Q) += board-univaq-camera.o
obj-$(CONFIG_MACH_MSM8X55_UNIVA_Q) += board-univaq-regulator.o

obj-$(CONFIG_MACH_LGE) += devices_lge.o

obj-$(CONFIG_LGE_HEADSET_MAX14579) += lge_headset_MAX14579.o
obj-$(CONFIG_LGE_PROC_COMM) += lge_proc_comm.o
obj-y += lge_audio_misc_ctrl.o

obj-$(CONFIG_LGE_SUPPORT_AT_CMD) += lge_ats.o
obj-$(CONFIG_LGE_SUPPORT_AT_CMD) += lge_ats_cmd.o
obj-$(CONFIG_LGE_SUPPORT_AT_CMD) += lge_ats_eta.o
obj-$(CONFIG_LGE_SUPPORT_AT_CMD) += lge_ats_mmc.o
obj-$(CONFIG_LGE_SUPPORT_AT_CMD) += lge_ats_event_log.o
obj-$(CONFIG_LGE_SUPPORT_AT_CMD) += lge_ats_flex.o # LGE_CHANGE [irene.park@lge.com] 2010-06-04 - to get flex value from ARM9 

obj-$(CONFIG_LGE_HANDLE_PANIC) += lge_handle_panic.o
obj-$(CONFIG_LGE_ATCMD_TTY) += lge_tty_atcmd.o
obj-$(CONFIG_LGE_FACTORY_PATCH) += lge_temp.o

endif


ifeq ($(TARGET_PRODUCT),victor_tmo_us)
obj-$(CONFIG_MACH_MSM8X55_VICTOR) += board-victor.o
obj-$(CONFIG_MACH_MSM8X55_VICTOR) += board-victor-pm.o
obj-$(CONFIG_MACH_MSM8X55_VICTOR) += board-victor-sound.o
obj-$(CONFIG_MACH_MSM8X55_VICTOR) += board-victor-misc.o
obj-$(CONFIG_MACH_MSM8X55_VICTOR) += board-victor-mmc.o
obj-$(CONFIG_MACH_MSM8X55_VICTOR) += board-victor-input.o
obj-$(CONFIG_MACH_MSM8X55_VICTOR) += board-victor-panel.o
obj-$(CONFIG_MACH_MSM8X55_VICTOR) += board-victor-bt.o
obj-$(CONFIG_MACH_MSM8X55_VICTOR) += board-victor-camera.o

obj-$(CONFIG_MACH_LGE) += devices_lge.o

obj-$(CONFIG_LGE_HEADSET_MAX14579) += lge_headset_MAX14579.o
obj-$(CONFIG_LGE_PROC_COMM) += lge_proc_comm.o
obj-$(CONFIG_MACH_MSM8X55_VICTOR) += lge_audio_misc_ctrl.o

obj-$(CONFIG_LGE_SUPPORT_AT_CMD) += lge_ats.o
obj-$(CONFIG_LGE_SUPPORT_AT_CMD) += lge_ats_cmd.o
obj-$(CONFIG_LGE_SUPPORT_AT_CMD) += lge_ats_eta.o
obj-$(CONFIG_LGE_SUPPORT_AT_CMD) += lge_ats_mmc.o
obj-$(CONFIG_LGE_SUPPORT_AT_CMD) += lge_ats_event_log.o
obj-$(CONFIG_LGE_SUPPORT_AT_CMD) += lge_ats_flex.o # LGE_CHANGE [irene.park@lge.com] 2010-06-04 - to get flex value from ARM9 

obj-$(CONFIG_LGE_HANDLE_PANIC) += lge_handle_panic.o
obj-$(CONFIG_LGE_ATCMD_TTY) += lge_tty_atcmd.o
obj-$(CONFIG_LGE_FACTORY_PATCH) += lge_temp.o

endif
#LGE_UPDATE_E kideok.kim@lge.com 20110323

ifeq ($(TARGET_PRODUCT),victor_open_com)
obj-$(CONFIG_MACH_MSM8X55_VICTOR) += board-victor.o
obj-$(CONFIG_MACH_MSM8X55_VICTOR) += board-victor-pm.o
obj-$(CONFIG_MACH_MSM8X55_VICTOR) += board-victor-sound-open.o
obj-$(CONFIG_MACH_MSM8X55_VICTOR) += board-victor-misc.o
obj-$(CONFIG_MACH_MSM8X55_VICTOR) += board-victor-mmc.o
obj-$(CONFIG_MACH_MSM8X55_VICTOR) += board-victor-input.o
obj-$(CONFIG_MACH_MSM8X55_VICTOR) += board-victor-panel.o
obj-$(CONFIG_MACH_MSM8X55_VICTOR) += board-victor-bt.o
obj-$(CONFIG_MACH_MSM8X55_VICTOR) += board-victor-camera.o

obj-$(CONFIG_MACH_LGE) += devices_lge.o

obj-$(CONFIG_LGE_HEADSET_MAX14579) += lge_headset_MAX14579.o
obj-$(CONFIG_LGE_PROC_COMM) += lge_proc_comm.o
obj-$(CONFIG_MACH_MSM8X55_VICTOR) += lge_audio_misc_ctrl_open.o

obj-$(CONFIG_LGE_SUPPORT_AT_CMD) += lge_ats.o
obj-$(CONFIG_LGE_SUPPORT_AT_CMD) += lge_ats_cmd.o
obj-$(CONFIG_LGE_SUPPORT_AT_CMD) += lge_ats_eta.o
obj-$(CONFIG_LGE_SUPPORT_AT_CMD) += lge_ats_mmc.o
obj-$(CONFIG_LGE_SUPPORT_AT_CMD) += lge_ats_event_log.o
obj-$(CONFIG_LGE_SUPPORT_AT_CMD) += lge_ats_flex.o # LGE_CHANGE [irene.park@lge.com] 2010-06-04 - to get flex value from ARM9 

obj-$(CONFIG_LGE_HANDLE_PANIC) += lge_handle_panic.o
obj-$(CONFIG_LGE_ATCMD_TTY) += lge_tty_atcmd.o
obj-$(CONFIG_LGE_FACTORY_PATCH) += lge_temp.o

endif
