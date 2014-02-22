/* drivers/video/msm/src/drv/fb/msm_fb.c
 *
 * Core MSM framebuffer driver.
 *
 * Copyright (C) 2007 Google Incorporated
 * Copyright (c) 2008-2010, Code Aurora Forum. All rights reserved.
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

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/mm.h>
#include <linux/fb.h>
#include <linux/msm_mdp.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <mach/board.h>
#include <linux/uaccess.h>

#include <linux/workqueue.h>
#include <linux/string.h>
#include <linux/version.h>
#include <linux/proc_fs.h>
#include <linux/vmalloc.h>
#include <linux/debugfs.h>
#include <linux/console.h>
#include <linux/android_pmem.h>
#include <linux/leds.h>
#include <linux/pm_runtime.h>

//#define SLIDE
#ifdef SLIDE
#include <linux/input.h>
#include <linux/workqueue.h>
#endif
	
#include "msm_fb_dpsv.h"
#include "msm_fb.h"
#include "mdp.h"
#include "mdp4.h"

extern unsigned char *fbram;
extern unsigned char *fbram_phys;
extern int fbram_size;

static struct msm_fb_dpsv_info mdi;
static struct fb_info dpsv_fbi;

static int msm_fb_dpsv_open(struct fb_info *info, int user);
static int msm_fb_dpsv_release(struct fb_info *info, int user);
static int msm_fb_dpsv_pan_display(struct fb_var_screeninfo *var,
			      struct fb_info *info);
static int msm_fb_dpsv_check_var(struct fb_var_screeninfo *var,
			    struct fb_info *info);
static int msm_fb_dpsv_set_par(struct fb_info *info);
static int msm_fb_dpsv_blank(int blank_mode, struct fb_info *info);
static int msm_fb_dpsv_ioctl(struct fb_info *info, unsigned int cmd,
			unsigned long arg);
//static int msm_fb_dpsv_mmap(struct fb_info *info, struct vm_area_struct * vma);
static void msm_fb_dpsv_imageblit(struct fb_info *info, const struct fb_image *image);
static void msm_fb_dpsv_copyarea(struct fb_info *info,
			    const struct fb_copyarea *area);
static void msm_fb_dpsv_fillrect(struct fb_info *info,
			    const struct fb_fillrect *rect);


#ifdef SLIDE
static int device_state = 1;
#endif

static struct fb_ops msm_fb_dpsv_ops = {
	.owner = THIS_MODULE,
	.fb_open = msm_fb_dpsv_open,
	.fb_release = msm_fb_dpsv_release,
	.fb_read = NULL,
	.fb_write = NULL,
	.fb_cursor = NULL,
	.fb_check_var = msm_fb_dpsv_check_var,	/* vinfo check */
	.fb_set_par = msm_fb_dpsv_set_par,	/* set the video mode according to info->var */
	.fb_setcolreg = NULL,	/* set color register */
	.fb_blank = msm_fb_dpsv_blank,	/* blank display */
	.fb_pan_display = msm_fb_dpsv_pan_display,	/* pan display */
	.fb_fillrect = msm_fb_dpsv_fillrect,	/* Draws a rectangle */
	.fb_copyarea = msm_fb_dpsv_copyarea,	/* Copy data from area to another */
	.fb_imageblit = msm_fb_dpsv_imageblit,	/* Draws a image to the display */
	.fb_rotate = NULL,
	.fb_sync = NULL,	/* wait for blit idle, optional */
	.fb_ioctl = msm_fb_dpsv_ioctl,	/* perform fb specific ioctl (optional) */

	.fb_mmap = NULL,	//Sumeet TBD: Using Default mmap of fbmem. msm_fb_mmap does additional pageprotections; handle them later.
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

	DPSV_DBG("+ \n" );
	handle = kzalloc(sizeof(struct input_handle), GFP_KERNEL);
	if (!handle)
		return -ENOMEM;

	handle->dev = dev;
	handle->handler = handler;
	handle->name = "slider-dpsv";

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
	
	if (type == EV_SW && code == SW_LID && value == 1) //closed.
	{
		DPSV_DBG("slide close detected\n");
		device_state = 0;
	}
	else
	if (type == EV_SW && code == SW_LID && value == 0) //opened.
	{
		DPSV_DBG("slide open detected\n" );
		device_state = 1;
	}
}

static struct input_handler sld_input_handler = {
	.event		= sld_input_event,
	.connect	= sld_input_connect,
	.disconnect	= sld_input_disconnect,
	.name		= "slider_dpsv",
	.id_table	= sld_ids,
};
#endif


static int verify_fbinfo(struct fb_info* fbi)
{
	int ret=0;
	struct msm_fb_data_type* mfd = fbi->par;
	if (mfd->panel_info.type != EBI2_PANEL && mfd->panel_info.type != MDDI_PANEL)
		return EINVAL;

	ret = register_framebuffer (fbi);
	unregister_framebuffer (fbi);
	if (ret < 0)
	{
		printk("DPSV: Error: Failed to register Framebuffer: %d\n", ret);
	}
	return ret;
};

static int verify_and_order_p1_p2(void)
{
	struct fb_info* t;
#ifdef P1_WIDER_THAN_P2
	if (mdi.fbi_p1->var.xres < mdi.fbi_p2->var.xres)
#else
	if (mdi.fbi_p1->var.xres > mdi.fbi_p2->var.xres)
#endif
	{
		t = mdi.fbi_p1;
		mdi.fbi_p1 = mdi.fbi_p2;
		mdi.fbi_p2 = t;
	}

	//verify compatibility of p1 and p2.
	//TBD.
/* Sumeet TBD: The calculations in initialise_dpsv_fbi take into consideration physically stitched aspect of DPSV buffers.
 * However, panning needs to be very carefully handled with this.
 * For now, implement just single buffer; later, implement panning and enable back-buffer.
*/
	if (mdi.fbi_p1->var.yres != mdi.fbi_p1->var.yres_virtual)
	{
		DPSV_ERR ("Back Buffering requested in P1, but in DPSV NOT yet supported\n");
		//panic("Back buffering not yet supported in dpsv");
		return -1;
	}
	if (mdi.fbi_p2->var.yres != mdi.fbi_p2->var.yres_virtual)
	{
		DPSV_ERR ("Back Buffering requested in P2, but in DPSV NOT yet supported\n");
		//panic("Back buffering not yet supported in dpsv");
		return -1;
	}

	return 0;
};

static int initialise_dpsv_fbi(void)
{
	int fbram_offset;
	int ret = 0;
	struct fb_fix_screeninfo* fix = &dpsv_fbi.fix;
	struct fb_var_screeninfo* var = &dpsv_fbi.var;

	if (!mdi.fbi_p1 || !mdi.fbi_p2)
	{
		DPSV_ERR ("Invalid State of DPSV\n");
		//panic("Invalid State of DPSV");
		return -1;
	}
	snprintf(fix->id, sizeof(fix->id), "msmfb40_dpsv");
	//TBD calculate fix.smem_length

	//Copy fix and var params from P1's, by default.
	//TBD: later, just do memcopy instead of elementary copy.

	fix->type				= mdi.fbi_p1->fix.type;
	fix->xpanstep			= mdi.fbi_p1->fix.xpanstep;
	fix->ypanstep			= mdi.fbi_p1->fix.ypanstep;
	fix->line_length		= mdi.fbi_p1->fix.line_length;

	var->vmode				= mdi.fbi_p1->var.vmode;
	var->blue.offset		= mdi.fbi_p1->var.blue.offset;
	var->green.offset		= mdi.fbi_p1->var.green.offset;
	var->red.offset			= mdi.fbi_p1->var.red.offset;
	var->blue.length		= mdi.fbi_p1->var.blue.length;
	var->green.length		= mdi.fbi_p1->var.green.length;
	var->red.length			= mdi.fbi_p1->var.red.length;
	var->blue.msb_right		= mdi.fbi_p1->var.blue.msb_right;
	var->green.msb_right	= mdi.fbi_p1->var.green.msb_right;
	var->red.msb_right		= mdi.fbi_p1->var.red.msb_right;
	var->transp.offset		= mdi.fbi_p1->var.transp.offset;
	var->transp.length		= mdi.fbi_p1->var.transp.length;

	var->pixclock			= mdi.fbi_p1->var.pixclock;

	var->xres				= mdi.fbi_p1->var.xres;		//xres is greater of the two, which is in P1.
	var->yres				= mdi.fbi_p1->var.yres + mdi.fbi_p2->var.yres;
	var->xres_virtual		= var->xres;
	var->yres_virtual		= mdi.fbi_p1->var.yres_virtual + mdi.fbi_p2->var.yres_virtual;
	var->yres_virtual		*= 2; /******** TEMP ONLY, to allow GRALLOC to function *********/
	var->bits_per_pixel		= mdi.fbi_p1->var.bits_per_pixel;
	
	fix->smem_len			= fix->line_length * var->yres_virtual;

	dpsv_fbi.fbops			= &msm_fb_dpsv_ops;
	dpsv_fbi.flags			= FBINFO_FLAG_DEFAULT;
	dpsv_fbi.pseudo_palette = mdi.fbi_p1->pseudo_palette;
	dpsv_fbi.par			= NULL; //no parameter currently needed for DPSV.


	fix->type_aux			= 0;	/* if type == FB_TYPE_INTERLEAVED_PLANES */
	fix->visual				= FB_VISUAL_TRUECOLOR;	/* True Color */
	fix->ywrapstep			= 0;	/* No support */
	fix->mmio_start			= 0;	/* No MMIO Address */
	fix->mmio_len			= 0;	/* No MMIO Address */
	fix->accel				= FB_ACCEL_NONE;/* FB_ACCEL_MSM needes to be added in fb.h */

	var->xoffset			= 0,	/* Offset from virtual to visible */
	var->yoffset			= 0,	/* resolution */
	var->grayscale			= 0,	/* No graylevels */
	var->nonstd				= 0,	/* standard pixel format */
	var->activate			= FB_ACTIVATE_VBL,	/* activate it at vsync */
	var->height =			-1,	/* height of picture in mm */
	var->width =			-1,	/* width of picture in mm */
	var->accel_flags =		0,	/* acceleration flags */
	var->sync =				0,	/* see FB_SYNC_* */
	var->rotate =			0,	/* angle we rotate counter clockwise */


	fbram_offset = PAGE_ALIGN((int)fbram)-(int)fbram;
	fbram += fbram_offset;
	fbram_phys += fbram_offset;
	fbram_size -= fbram_offset;

	if (fbram_size < fix->smem_len) {
		DPSV_ERR ("error: no more framebuffer memory!\n");
		return -ENOMEM;
	}

	dpsv_fbi.screen_base		= fbram;
	dpsv_fbi.fix.smem_start		= (unsigned long)fbram_phys;

	mdi.fbi_p1->screen_base		= fbram;
	mdi.fbi_p1->fix.smem_start	= (unsigned long)fbram_phys;

	mdi.fbi_p2->screen_base		= fbram + mdi.fbi_p1->fix.line_length * mdi.fbi_p1->var.yres;
	mdi.fbi_p2->fix.smem_start	= (unsigned long)fbram_phys + mdi.fbi_p1->fix.line_length * mdi.fbi_p1->var.yres;

	//Set yres_virtual of both panels'f fb_info, according to pysically stitched DPSV buffer design.
	mdi.fbi_p1->var.yres_virtual = ( mdi.fbi_p1->var.yres ) * 2 + mdi.fbi_p2->var.yres;	//sumeet TBD: 2 is for fb_page; generalise it.
	mdi.fbi_p2->var.yres_virtual = mdi.fbi_p1->var.yres + ( mdi.fbi_p2->var.yres * 2 );

	//fixup xres_virtual, line_length and smem_len of panel2.
	mdi.fbi_p2->var.xres_virtual = mdi.fbi_p1->var.xres ;
	mdi.fbi_p2->var.bits_per_pixel = mdi.fbi_p1->var.bits_per_pixel;
	mdi.fbi_p2->fix.line_length = mdi.fbi_p2->var.xres_virtual * mdi.fbi_p2->var.bits_per_pixel/8;
	mdi.fbi_p2->fix.smem_len	= fix->line_length * mdi.fbi_p2->var.yres_virtual;

	DPSV_DBG ("FrameBuffer[DPSV.P1] %dx%d size=%d bytes screen_base = %#X\n",
	     mdi.fbi_p1->var.xres, mdi.fbi_p1->var.yres, mdi.fbi_p1->fix.smem_len, (unsigned int)mdi.fbi_p1->screen_base);

	DPSV_DBG ("FrameBuffer[DPSV.P2] %dx%d size=%d bytes screen_base = %#X\n",
	     mdi.fbi_p2->var.xres, mdi.fbi_p2->var.yres, mdi.fbi_p2->fix.smem_len, (unsigned int)mdi.fbi_p2->screen_base);


	memset(dpsv_fbi.screen_base, 0x0, fix->smem_len);

	ret = register_framebuffer (&dpsv_fbi);
	if (ret < 0)
	{
		printk("ERROR registering DPSV framebuffer: %d\n", ret);
		return ret;
	}
	DPSV_DBG ("FrameBuffer[DPSV] %dx%d size=%d bytes is registered successfully!\n",
	     dpsv_fbi.var.xres, dpsv_fbi.var.yres, dpsv_fbi.fix.smem_len);

#ifdef SLIDE
	ret = input_register_handler(&sld_input_handler);
	DPSV_DBG("input_register_handler: %d.\n", ret );
#endif



	fbram += fix->smem_len;
	fbram_phys += fix->smem_len;
	fbram_size -= fix->smem_len;

	return ret;
};

int register_dpsv_framebuffer(struct fb_info *fbi)
{
	int ret = 0;

	//this is the first panel registration.
	if (!mdi.fbi_p1)
	{
		DPSV_DBG ("Registering First DPSV FBI\n");
		//Let fbmem do the verifications.
		ret = verify_fbinfo(fbi);
		if (ret < 0)
		{
			printk("DPSV: Error: Failed to register 1st Framebuffer: %d\n", ret);
			return ret;
		}
		mdi.fbi_p1 = fbi;
		DPSV_DBG ("Registered First DPSV FBI\n");
		return 0;
	}
	if (mdi.fbi_p2)
	{
		//P2 is also already registered! Cannot accept more FB requests for DPSV; so, 
		//return error, so that this is handled in msm_fb.c
		return -EBUSY;
	}
	//this is the second panel registration.
	DPSV_DBG ("Registering Second DPSV FBI\n");
	ret = verify_fbinfo(fbi);
	if (ret < 0)
	{
		printk("DPSV: Error: Failed to register 2nd Framebuffer: %d\n", ret);
		return ret;
	}
	
	mdi.fbi_p2 = fbi;
	ret = verify_and_order_p1_p2();
	if (ret < 0)
	{
		printk("DPSV: Error: Panels are not compatible: %d\n", ret);
		return ret;
	}	
	DPSV_DBG ("Registered Second DPSV FBI\n");
	initialise_dpsv_fbi();
	//register DPSV framebuffer.
//	ret = register_framebuffer(&dpsv_fbi);
	if (ret < 0)
	{
		printk("DPSV: Error: Failed to register DPSV Framebuffer: %d\n", ret);
	}
	return ret;
}

#if 1

static int msm_fb_dpsv_open(struct fb_info *info, int user)
{
	int ret;

	DPSV_DBG_ENTER;
	ret = mdi.fbi_p1->fbops->fb_open(mdi.fbi_p1, user);

	if (ret < 0) {
		printk(KERN_ERR "P1 Open Failed\n");
		return ret;
	}

	ret = mdi.fbi_p2->fbops->fb_open(mdi.fbi_p2, user);

	if (ret < 0) {
		printk(KERN_ERR "P2 Open Failed\n");
		mdi.fbi_p1->fbops->fb_release(mdi.fbi_p1, user);
		return ret;
	}

	DPSV_DBG_EXIT;
	return 0;
}

static int msm_fb_dpsv_release(struct fb_info *info, int user)
{
	int ret;

	DPSV_DBG_ENTER;
	ret = mdi.fbi_p1->fbops->fb_release (mdi.fbi_p1, user);
	if (ret < 0) {
		printk(KERN_ERR "P1 Release Failed\n");
	}

	ret = mdi.fbi_p2->fbops->fb_release (mdi.fbi_p2, user);
	if (ret < 0) {
		printk(KERN_ERR "P2 Release Failed\n");
	}

	DPSV_DBG_EXIT;
	return ret;
}

static int msm_fb_dpsv_check_var(struct fb_var_screeninfo *var, struct fb_info *info)
{
	int ret = 0;
	//DPSV_DBG_ENTER;

/*	if (mdi.fbi_p1->fbops->fb_check_var)
	{
		ret = mdi.fbi_p1->fbops->fb_check_var(var, mdi.fbi_p1);
		if (ret < 0)
		{
			 printk (KERN_ERR "DPSV: p1's check_var failed: %d\n", ret);
			 return ret;
		}
	}
	if (mdi.fbi_p2->fbops->fb_check_var)
	{
		ret = mdi.fbi_p2->fbops->fb_check_var(var, mdi.fbi_p2);
		if (ret < 0)
		{
			 printk (KERN_ERR "DPSV: p2's check_var failed: %d\n", ret);
			 return ret;
		}
	}
*/

	struct msm_fb_data_type *mfd_p1 = (struct msm_fb_data_type *)mdi.fbi_p1->par;
	struct msm_fb_data_type *mfd_p2 = (struct msm_fb_data_type *)mdi.fbi_p2->par;

// Unmodifiable parts verification
	if (var->rotate != FB_ROTATE_UR)
		return -EINVAL;
	if (var->grayscale != info->var.grayscale)
		return -EINVAL;
	if (var->bits_per_pixel != info->var.bits_per_pixel)	//Sumeet TBD: pixel format is frozen for now. Check with platform whether more fmts are needed.
		return -EINVAL;

//boundary checks
	if ((var->xres_virtual <= 0) || (var->yres_virtual <= 0))
		return -EINVAL;

	if (info->fix.smem_len <
		(var->xres_virtual*var->yres_virtual*(var->bits_per_pixel/8)))
		return -EINVAL;

	if ((var->xres == 0) || (var->yres == 0))
		return -EINVAL;

	if ((var->xres != mfd_p1->panel_info.xres) ||
		(var->yres != mfd_p1->panel_info.yres + mfd_p2->panel_info.yres)){
		DPSV_ERR ("changing xres/yres in dpsv not yet defined\n");
		return -EINVAL;
	}

	if ((var->xres > mfd_p1->panel_info.xres) ||
		(var->yres > mfd_p1->panel_info.yres + mfd_p2->panel_info.yres))
		return -EINVAL;

	if (var->xoffset > (var->xres_virtual - var->xres))
		return -EINVAL;

	if (var->yoffset > (var->yres_virtual - var->yres))
		return -EINVAL;

	//DPSV_DBG_EXIT;

	return ret;
}

static int msm_fb_dpsv_blank(int blank_mode, struct fb_info *info)
{
	int ret;
	DPSV_DBG_ENTER;

	ret = mdi.fbi_p1->fbops->fb_blank(blank_mode, mdi.fbi_p1);
	if (ret < 0) {
		printk(KERN_ERR "P1 BLANK %d Failed: %d\n", blank_mode, ret);
		return ret;
	}

	ret = mdi.fbi_p2->fbops->fb_blank(blank_mode, mdi.fbi_p2);
	if (ret < 0) {
		printk(KERN_ERR "P2 BLANK %d Failed: %d\n", blank_mode, ret);
	}

	DPSV_DBG_EXIT;
	return ret;
}

static void msm_fb_dpsv_fillrect(struct fb_info *info,
			    const struct fb_fillrect *rect)
{
	DPSV_DBG_ENTER;
	printk (KERN_ERR "fillrect: Not Yet Implemented in DPSV; who is calling me??\n");
	DPSV_DBG_EXIT;
}

static void msm_fb_dpsv_copyarea(struct fb_info *info,
			    const struct fb_copyarea *area)
{
	DPSV_DBG_ENTER;
	printk (KERN_ERR "copyarea: Not Yet Implemented in DPSV; who is calling me??\n");
	DPSV_DBG_EXIT;
}

static void msm_fb_dpsv_imageblit(struct fb_info *info, const struct fb_image *image)
{
	DPSV_DBG_ENTER;
	printk (KERN_ERR "imageblit: Not Yet Implemented in DPSV; who is calling me??\n");
	DPSV_DBG_EXIT;
}

static int msm_fb_dpsv_ioctl(struct fb_info *info, unsigned int cmd,
			unsigned long arg)
{
	DPSV_DBG_ENTER;
	mdi.fbi_p1->fbops->fb_ioctl(mdi.fbi_p1, cmd, arg);
	DPSV_DBG_EXIT;
	return 0;
}

static int msm_fb_dpsv_set_par(struct fb_info *info)
{
	//DPSV_DBG_ENTER;

	//sumeet TBD: Copy from info->var to fbi_p1.var and fbi_p2.var, whatever is relevant and changeable.
	mdi.fbi_p1->var.pixclock = info->var.pixclock;
	mdi.fbi_p1->fbops->fb_set_par(mdi.fbi_p1);

	mdi.fbi_p2->var.pixclock = info->var.pixclock;
	mdi.fbi_p2->fbops->fb_set_par(mdi.fbi_p2);

	//DPSV_DBG_EXIT;
	return 0;
}

static int msm_fb_dpsv_pan_display(struct fb_var_screeninfo *var, struct fb_info *info)
{
	//DPSV_DBG_ENTER;

	struct fb_var_screeninfo var1, var2;

	int ret;
	var1 = mdi.fbi_p1->var;
	var2 = mdi.fbi_p2->var;
	
	var1.activate = var->activate;
	var2.activate = var->activate;

	if (var->reserved[0] == 0x54445055) {
		DPSV_DBG("pan UPDT x=%d, y=%d\n", var->reserved[1] & 0xffff, (var->reserved[1] >> 16) & 0xffff);
		DPSV_DBG("pan UPDT w=%d, h=%d\n", var->reserved[2] & 0xffff, (var->reserved[2] >> 16) & 0xffff);
	}

	if (var->yoffset == 0)
	{
		var1.yoffset = var2.yoffset = 0;
	}else 
	if (var->yoffset == info->var.yres)
	{
		var1.yoffset = info->var.yres;
		var2.yoffset = info->var.yres;
	}else 
	{
		DPSV_ERR("yoffset invalid %d\n", var->yoffset);
		//return 0;
	}	

	//DPSV_DBG ("1.yo = %d, 2.yo = %d\n", var1.yoffset, var2.yoffset);

	//NOTE: MDDI panel's rendering is now non-blocking (MDP4_NONBLOCKING); thus, the below sequential code renders both panels paralely.
	ret = mdi.fbi_p1->fbops->fb_pan_display(&var1, mdi.fbi_p1);
	//DPSV_DBG ("PANned 1: %X, ret %d\n", mdi.fbi_p1, ret);
#ifdef SLIDE
	if (device_state){
		ret = mdi.fbi_p2->fbops->fb_pan_display(&var2, mdi.fbi_p2);
		DPSV_DBG ("PANned 2: %d\n", ret);
	}
#else
	ret = mdi.fbi_p2->fbops->fb_pan_display(&var2, mdi.fbi_p2);
	//DPSV_DBG ("PANned 2: %d\n", ret);
#endif

	//DPSV_DBG_EXIT;
	return ret;
}
#endif
