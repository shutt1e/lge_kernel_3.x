/* Copyright (c) 2008-2009, Code Aurora Forum. All rights reserved.
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/hrtimer.h>

#include <mach/hardware.h>
#include <linux/io.h>

#include <asm/system.h>
#include <asm/mach-types.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h>

#include <linux/fb.h>

//sumeet.gupta 040411 ONE_CODE: Changing whole file, as it is used only in FLIP.
#include "mdp.h"
#include "mdp4.h"
#include "msm_fb.h"

//sumeet 301110: DMA doesn't show data on the panel, while memcopy does.
//#define EBI2_DMA_WORKAROUND

#ifndef EBI2_DMA_WORKAROUND
static void mdp_dma_s_update_lcd(struct msm_fb_data_type *mfd)
{
	MDPIBUF *iBuf = &mfd->ibuf;
	int mddi_dest = FALSE;
	uint32 outBpp = iBuf->bpp;
	uint32 dma_s_cfg_reg;
	uint8 *src;
	struct msm_fb_panel_data *pdata =
	    (struct msm_fb_panel_data *)mfd->pdev->dev.platform_data;

	//printk(KERN_ERR "DMA_S kickoff\n");
	dma_s_cfg_reg = DMA_PACK_TIGHT | DMA_PACK_ALIGN_LSB |
	    DMA_OUT_SEL_AHB | DMA_IBUF_NONCONTIGUOUS;

	if (mfd->fb_imgType == MDP_BGR_565)
		dma_s_cfg_reg |= DMA_PACK_PATTERN_BGR;
	else
		dma_s_cfg_reg |= DMA_PACK_PATTERN_BGR;

	if (outBpp == 4)
		dma_s_cfg_reg |= DMA_IBUF_C3ALPHA_EN;

	if (outBpp == 2)
		dma_s_cfg_reg |= DMA_IBUF_FORMAT_RGB565;

	dma_s_cfg_reg |= 5<<24; //qct ebi2 dma_s patch merge.

	//ebi2 camp: TEAR_CHECK_EN register setting for sub-panel.
	if (mfd->panel_info.lcd.hw_vsync_mode){
		MDP_OUTP(MDP_BASE + 0x20c, 0x3);
		MDP_OUTP(MDP_BASE + 0x124, 0x24);
		//MDP_OUTP(MDP_BASE + 0x214, 235); //for 240x320 orientation.
		MDP_OUTP(MDP_BASE + 0x214, 200);
	}

	if (mfd->panel_info.pdest != DISPLAY_2) {
		printk(KERN_ERR "error: non-secondary type through dma_s!\n");
		return;
	}

	if (mfd->panel_info.type == MDDI_PANEL ||
		mfd->panel_info.type == EXT_MDDI_PANEL) {
		dma_s_cfg_reg |= DMA_OUT_SEL_MDDI;
		mddi_dest = TRUE;
	} else {
		outp32(MDP_EBI2_LCD1, mfd->data_port_phys);
	}

	src = (uint8 *) iBuf->buf;
	/* starting input address */
	src += (iBuf->dma_x + iBuf->dma_y * iBuf->ibuf_width) * outBpp;

	/* MDP cmd block enable */
	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_ON, FALSE);
	if (mfd->panel_info.type == MDDI_PANEL ||
		mfd->panel_info.type == EBI2_PANEL) {
		MDP_OUTP(MDP_BASE + 0xa0004,
			(iBuf->dma_h << 16 | (iBuf->dma_w)));
			//(320 << 16 | (iBuf->dma_w)));
		MDP_OUTP(MDP_BASE + 0xa0008, src);	/* ibuf address */
		MDP_OUTP(MDP_BASE + 0xa000c,
			iBuf->ibuf_width * outBpp);/* ystride */
	} else {
		MDP_OUTP(MDP_BASE + 0xb0004,
			(iBuf->dma_h << 16 | iBuf->dma_w));
		MDP_OUTP(MDP_BASE + 0xb0008, src);	/* ibuf address */
		MDP_OUTP(MDP_BASE + 0xb000c,
			iBuf->ibuf_width * outBpp);/* ystride */
	}

	/* PIXELSIZE */
	if (mfd->panel_info.bpp == 18) {
		dma_s_cfg_reg |= DMA_DSTC0G_6BITS |	/* 666 18BPP */
		    DMA_DSTC1B_6BITS | DMA_DSTC2R_6BITS;
	} else {
		dma_s_cfg_reg |= DMA_DSTC0G_6BITS |	/* 565 16BPP */
		    DMA_DSTC1B_5BITS | DMA_DSTC2R_5BITS;
	}

	if (mddi_dest) {
		if (mfd->panel_info.type == MDDI_PANEL) {
			MDP_OUTP(MDP_BASE + 0xa0010,
				(iBuf->dma_y << 16) | iBuf->dma_x);
			MDP_OUTP(MDP_BASE + 0x00090, 1);
		} else {
			MDP_OUTP(MDP_BASE + 0xb0010,
				(iBuf->dma_y << 16) | iBuf->dma_x);
			MDP_OUTP(MDP_BASE + 0x00090, 2);
		}
		MDP_OUTP(MDP_BASE + 0x00094,
				(MDDI_VDO_PACKET_DESC << 16) |
				mfd->panel_info.mddi.vdopkt);
	} else {
		/* setting LCDC write window */
		pdata->set_rect(iBuf->dma_x, iBuf->dma_y, iBuf->dma_w,
				iBuf->dma_h);
	}

	if (mfd->panel_info.type == EBI2_PANEL || mfd->panel_info.type == MDDI_PANEL)
		MDP_OUTP(MDP_BASE + 0xa0000, dma_s_cfg_reg);
	else
		MDP_OUTP(MDP_BASE + 0xb0000, dma_s_cfg_reg);

	/* MDP cmd block disable */
	outp32(msm_mdp_base + 0x38, 0xE);  //qct ebi2 dma_s patch merge after modify.
	outp32(msm_mdp_base + 0x70, 0x3);  //camp debug.
	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_OFF, FALSE);
	if (mfd->panel_info.type == EBI2_PANEL || mfd->panel_info.type == MDDI_PANEL)
		mdp_pipe_kickoff(MDP_DMA_S_TERM, mfd);
	else
		mdp_pipe_kickoff(MDP_DMA_E_TERM, mfd);

}

#else //WORKAROUND

void copy_from_mfd_rgba(struct msm_fb_data_type *mfd)
{
	MDPIBUF *iBuf = &mfd->ibuf;
	//uint32 outBpp = iBuf->bpp;
	uint32* src;
	uint16 dma_w = iBuf->dma_w;							//RoI width
	uint16 dma_h = iBuf->dma_h;							//RoI Height
	uint16 ystr = (iBuf->ibuf_width - dma_w);			//extra pixels to be skipped.
	int offset = ((uint32)(iBuf->buf)) - mfd->fbi->fix.smem_start;
	int i, stride = 0;
	
	uint16 rgb565;
	uint16 r,g,b;

	/* starting input address */
	src =  (uint32*)(mfd->fbi->screen_base + offset);
	src += (iBuf->dma_x + iBuf->dma_y * iBuf->ibuf_width) ;// * outBpp;
	outpw(mfd->cmd_port, 0x2C);

	b = (*src >> 19) & 0x1F; g = (*src >> 10) & 0x3F; r = (*src >> 03) & 0x1F;
	rgb565 = (r<<11) | (g<<5) | b;
	outpw(mfd->data_port, rgb565);

	for (i=1; i<dma_w*dma_h; i++)
	{
		if (!(i%dma_w))
		{
			stride+=ystr;		//stride.
		}
		b = (*(src+i+stride) >> 19) & 0x1F; g = (*(src+i+stride) >> 10) & 0x3F; r = (*(src+i+stride) >> 03) & 0x1F;
		rgb565 = (r<<11) | (g<<5) | b;
		outpw(mfd->data_port, rgb565);
	}
};

#endif

void mdp_dma_s_update(struct msm_fb_data_type *mfd)
{
	static int dbg = 0;
	if (!dbg)
	mdelay(10);
	//if (dbg)
	//	printk("D2+\n");
	down(&mfd->dma->mutex);
	if ((mfd) && (!mfd->dma->busy) && (mfd->panel_power_on)) {
		down(&mfd->sem);
		if (mfd->panel_info.type == EBI2_PANEL || mfd->panel_info.type == MDDI_PANEL)
			mdp_enable_irq(MDP_DMA_S_TERM);

		else
			mdp_enable_irq(MDP_DMA_E_TERM);
		mfd->dma->busy = TRUE;
		INIT_COMPLETION(mfd->dma->comp);
		mfd->ibuf_flushed = TRUE;
#ifndef EBI2_DMA_WORKAROUND
		mdp_dma_s_update_lcd(mfd);
		up(&mfd->sem);

		/* wait until DMA finishes the current job */
		//if (dbg)
	//		printk("D2W+\n");
		dbg=1;
		wait_for_completion_killable(&mfd->dma->comp);
	//	printk("D2W-\n");
#else
		copy_from_mfd_rgba(mfd);
		mfd->dma->busy = FALSE;
		up(&mfd->sem);
#endif
		if (mfd->panel_info.type == EBI2_PANEL || mfd->panel_info.type == MDDI_PANEL)
			mdp_disable_irq(MDP_DMA_S_TERM);
		else
			mdp_disable_irq(MDP_DMA_E_TERM);

	/* signal if pan function is waiting for the update completion */
		if (mfd->pan_waiting) {
			mfd->pan_waiting = FALSE;
			complete(&mfd->pan_comp);
		}
	}
	up(&mfd->dma->mutex);
	//printk("D2-\n");
}
