/*
 *  For rk3066 - rk3188
 *  Author: olegk0 <olegvedi@gmail.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "rk_layers_priv.h"

#ifdef DEBUGRGA
void DdgPrintRGA( struct rga_req *RGA_req)
{



    OVLDBG("src.format:%d\n",RGA_req->src.format);
    OVLDBG("src.act_w:%d\n",RGA_req->src.act_w);
    OVLDBG("src.act_h:%d\n",RGA_req->src.act_h);

    OVLDBG("src.yrgb_addr:0x%X\n",RGA_req->src.yrgb_addr);
    OVLDBG("src.uv_addr:0x%X\n",RGA_req->src.uv_addr);
    OVLDBG("src.v_addr:0x%X\n",RGA_req->src.v_addr);

    OVLDBG("src.vir_w:%d\n",RGA_req->src.vir_w);
    OVLDBG("src.vir_h:%d\n",RGA_req->src.vir_h);
//Dst
    OVLDBG("dst.vir_w:%d\n",RGA_req->dst.vir_w);
    OVLDBG("dst.vir_h:%d\n",RGA_req->dst.vir_h);
    OVLDBG("dst.x_offset:%d\n",RGA_req->dst.x_offset);
    OVLDBG("dst.y_offset:%d\n",RGA_req->dst.y_offset);
    OVLDBG("dst.act_w:%d\n",RGA_req->dst.act_w);
    OVLDBG("dst.act_h:%d\n",RGA_req->dst.act_h);//1/2

    OVLDBG("dst.format:%d\n",RGA_req->dst.format);
    OVLDBG("dst.yrgb_addr:0x%X\n",RGA_req->dst.yrgb_addr);

    OVLDBG("clip.xmax:%d\n",RGA_req->clip.xmax);
    OVLDBG("clip.ymax:%d\n",RGA_req->clip.ymax);
}
#endif


//++++++++++++++++++++++++++++++++++++RGA++++++++++++++++++++++++++++++++++++++++++
int ovlInitRGAHW()
{
    return open(FB_DEV_RGA, O_RDWR);
}
//-------------------------------------------------------------
int ovlRgaBlit(int syncmode)
{


    int ret, timeout = 0;

    while(pthread_mutex_trylock(&pOvl_priv->rgamutex) ==  EBUSY){
    	timeout++;
    	if(timeout > HW_TIMEOUT){
    		OVLDBG("Timeout rga");
    		return -EBUSY;
    	}
    	usleep(1);
    }
    ret = ioctl(pOvl_priv->OvlFb[EMU2Layer_IPP].fd, syncmode, &pOvl_priv->RGA_req);
    pthread_mutex_unlock(&pOvl_priv->rgamutex);
#ifdef DEBUG
    if(ret)
    	OVLDBG("ret:%d",ret);
#endif
    return ret;
}
//------------------------------------------------------------------------------
void ovlRgaInitReg(uint32_t SrcYAddr, int SrcFrmt, int DstFrmt,
		uint32_t DstYAddr, int Src_w, int Src_h, int Src_vir, int Dst_vir, Bool PhyAdr)
{
    memset(&pOvl_priv->RGA_req, 0, sizeof(struct rga_req));
//Src
    pOvl_priv->RGA_req.src.format = SrcFrmt;//    = 0x1,;
    pOvl_priv->RGA_req.src.act_w = Src_w;
    pOvl_priv->RGA_req.src.act_h = Src_h;

    pOvl_priv->RGA_req.src.yrgb_addr = SrcYAddr;
//    RGA_req.src.uv_addr  = SrcUVAddr;
//    RGA_req.src.v_addr   = SrcVAddr;

    pOvl_priv->RGA_req.src.vir_w = Src_vir;
    pOvl_priv->RGA_req.src.vir_h = Src_h;
/*    pOvl_priv->RGA_req.src.x_offset = Src_x;
    pOvl_priv->RGA_req.src.y_offset = Src_y;
    */
//Dst
    pOvl_priv->RGA_req.dst.vir_w = pOvl_priv->OvlLay[UILayer].var.xres_virtual;
    pOvl_priv->RGA_req.dst.vir_h = pOvl_priv->OvlLay[UILayer].var.yres_virtual;
//    RGA_req.dst.x_offset = Drw_x;
//    RGA_req.dst.y_offset = Drw_y;
    pOvl_priv->RGA_req.dst.act_w = pOvl_priv->RGA_req.src.act_w;
    pOvl_priv->RGA_req.dst.act_h = pOvl_priv->RGA_req.src.act_h;//1/2

    pOvl_priv->RGA_req.dst.format = DstFrmt;
    pOvl_priv->RGA_req.dst.yrgb_addr = DstYAddr;

    pOvl_priv->RGA_req.clip.xmax = pOvl_priv->RGA_req.dst.vir_w;
    pOvl_priv->RGA_req.clip.ymax = pOvl_priv->RGA_req.dst.vir_h;

//    RGA_req.src_trans_mode = 1;

    if(!PhyAdr){
    	pOvl_priv->RGA_req.mmu_info.mmu_en = 1;
    	pOvl_priv->RGA_req.mmu_info.mmu_flag = 0b100001;  /* [0] mmu enable [1] src_flush [2] dst_flush [3] CMD_flush [4~5] page size*/
    }
}
//---------------------------------------------------------------
void ovlRGASetFormats(OvlLayoutFormatType format, RGAUpdModeType UpMode)
{
    uint8_t		RGA_mode = RK_FORMAT_RGBX_8888;

    switch(format) {
    case RKL_FORMAT_RGB_888:
    	RGA_mode = RK_FORMAT_RGB_888;
    	break;
    case RKL_FORMAT_RGB_565:
    	RGA_mode = RK_FORMAT_RGB_565;
    	break;
    case RKL_FORMAT_YCrCb_NV12_SP:
    	RGA_mode = RK_FORMAT_YCbCr_420_SP;
        break;
    case RKL_FORMAT_YCbCr_422_SP:
    	RGA_mode = RK_FORMAT_YCrCb_422_SP;
    	break;
/*
    case RKL_FORMAT_YCrCb_NV12_P:
    	RGA_mode = RK_FORMAT_YCbCr_420_P;
        break;
    case RKL_FORMAT_YCbCr_422_P:
    	RGA_mode = RK_FORMAT_YCrCb_422_P;
    	break;
    	*/
/*    case RK_FORMAT_YCrCb_444:
    	break;*/
//    case RKL_FORMAT_RGBX_8888:
//    case RKL_FORMAT_RGBA_8888:
    default:
    	RGA_mode = RK_FORMAT_RGBX_8888;
    }

    if(UpMode == SRC_MODE || UpMode == BOTH_MODE){
    	pOvl_priv->RGA_req.src.format = RGA_mode;
	}
    if(UpMode == DST_MODE || UpMode == BOTH_MODE){
		pOvl_priv->RGA_req.dst.format = RGA_mode;
	}
}
//--------------------------------------------------------------------------------
void ovlRGASetDrw( int Drw_w, int Drw_h, int Drw_x, int Drw_y)
{

/*
	pOvl_priv->RGA_req.src.act_w = Drw_w;
	pOvl_priv->RGA_req.src.act_h = Drw_h;
*/
	pOvl_priv->RGA_req.dst.x_offset = Drw_x;
	pOvl_priv->RGA_req.dst.y_offset = Drw_y;
	pOvl_priv->RGA_req.dst.act_w = Drw_w;
	pOvl_priv->RGA_req.dst.act_h = Drw_h;//1/2
//    RGA_req.clip.xmax = overlay.cur_var.xres-1;
//    RGA_req.clip.ymax = overlay.cur_var.yres-1;
}
//---------------------------------------------------------------
void ovlRGASetSrc(uint32_t SrcYAddr)
{
    pOvl_priv->RGA_req.src.yrgb_addr = SrcYAddr;
//    RGA_req.src.uv_addr  = SrcUVAddr;
//    RGA_req.src.v_addr   = SrcVAddr;
}
/*
static int ovlBppToRga(int bpp)//BIT per pixel
{
	int ret=-EINVAL;

	switch(bpp){
	case 1:
		ret = RK_FORMAT_BPP1;
		break;
	case 2:
		ret = RK_FORMAT_BPP2;
		break;
	case 4:
		ret = RK_FORMAT_BPP4;
		break;
	case 8:
		ret = RK_FORMAT_BPP8;
		break;
	case 16:
		ret = RK_FORMAT_RGB_565;
		break;
	case 24:
		ret = RK_FORMAT_RGB_888;
		break;
	case 32:
		ret = RK_FORMAT_RGBX_8888;
		break;

	}
	return ret;
}
//--------------------------------------------------------------------------------
int Ovl2dBlt( uint32_t *src_bits, uint32_t *dst_bits, int src_stride, int dst_stride,
		int src_bpp, int dst_bpp, int src_x, int src_y, int dst_x, int dst_y, int w, int h)
{


    int SrcFrmt, DstFrmt, ret;

    SrcFrmt = ovlBppToRga(src_bpp);
    DstFrmt = ovlBppToRga(dst_bpp);
	ovlRgaInitReg( &Ovl_priv.OvlLay[UI_L].RGA_req, (uint32_t)src_bits, SrcFrmt, DstFrmt,
			(uint32_t)dst_bits, src_x, src_y, w, h, dst_x, dst_y, src_stride, dst_stride, FALSE);

	ret = ovlRgaBlit( &Ovl_priv.OvlLay[UI_L].RGA_req, RGA_BLIT_SYNC);
	if(ret < 0)
		OVLDBG("rga ret:%d",ret);
	OVLDBG("\n src_x:%d, src_y:%d, w:%d, h:%d, dst_x:%d, dst_y:%d, src_stride:%d, dst_stride:%d",src_x, src_y, w, h, dst_x, dst_y, src_stride, dst_stride);
	return ret;
}
//--------------------------------------------------------------------------------
int OvlCpBufToDisp( OvlMemPgPtr PMemPg, OvlLayPg layout)
{


    OvlLayPg t;

    OVLDBG("OvlCpBufToDisp OvlPg:%d\n",layout);
//	overlay.OvlLay[layout].ResChange = FALSE;
    if(layout < MAX_OVERLAYs && layout >= 0){
    	Ovl_priv.OvlLay[layout].RGA_req.src.yrgb_addr = PMemPg->phy_addr;
    	return ovlRgaBlit( &Ovl_priv.OvlLay[layout].RGA_req, RGA_BLIT_SYNC);
    }
    OVLDBG("OvlCpBufToDisp Error");
    return -1;
}
*/
