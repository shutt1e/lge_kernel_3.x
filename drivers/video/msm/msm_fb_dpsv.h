/* Copyright (c) 2008-2010, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of Code Aurora Forum, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef MSM_DPSV_FB_H
#define MSM_DPSV_FB_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include "linux/proc_fs.h"

#include <mach/hardware.h>
#include <linux/io.h>
#include <mach/board.h>

#include <asm/system.h>
#include <asm/mach-types.h>
#include <mach/memory.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/hrtimer.h>

#include <linux/fb.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#include "msm_fb_panel.h"
#include "mdp.h"

#if 0
#define DPSV_DBG(msg...)    \
		printk(KERN_DEBUG "[DPSV]:" msg);

#define DPSV_ERR(msg...)    \
		printk(KERN_ERR "[DPSV]:" msg);

#define DPSV_DBG_ENTER \
		DPSV_DBG("%s +\n", __FUNCTION__);

#define DPSV_DBG_EXIT \
		DPSV_DBG("%s -\n", __FUNCTION__);

#else
#define DPSV_DBG(msg...)
#define DPSV_ERR(msg...)
#define DPSV_DBG_ENTER
#define DPSV_DBG_EXIT
#endif

/* This define does the ordering of Panel 1 vis-a-vis Panel 2.
 * To inverse the ordering, undefine this.
 */
#define P1_WIDER_THAN_P2

/* The structure to hold data for DPSV implementation.
 *
 */
struct msm_fb_dpsv_info
{
	struct fb_info* fbi_p1;
	struct fb_info* fbi_p2;

};

extern int register_dpsv_framebuffer(struct fb_info *fb_info);
extern int unregister_dpsv_framebuffer(struct fb_info *fb_info);
extern void fb_dpsv_set_suspend(struct fb_info *info, int state);

//DPSV variants of these fbmem functions is not yet required.
/*
extern int fb_prepare_logo(struct fb_info *fb_info, int rotate);
extern int fb_show_logo(struct fb_info *fb_info, int rotate);
extern char* fb_get_buffer_offset(struct fb_info *info, struct fb_pixmap *buf, u32 size);
extern void fb_pad_unaligned_buffer(u8 *dst, u32 d_pitch, u8 *src, u32 idx,
				u32 height, u32 shift_high, u32 shift_low, u32 mod);
extern void fb_pad_aligned_buffer(u8 *dst, u32 d_pitch, u8 *src, u32 s_pitch, u32 height);
extern int fb_get_color_depth(struct fb_var_screeninfo *var,
			      struct fb_fix_screeninfo *fix);
extern int fb_get_options(char *name, char **option);
extern int fb_new_modelist(struct fb_info *info);
*/


#endif /* MSM_DPSV_FB_H */
