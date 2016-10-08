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

//#define DEBUGRGA

#ifdef DEBUGRGA
void DdgPrintRGA( struct rga_req *RGA_req)
{

    OVLDBG("src.format:%d",RGA_req->src.format);
    OVLDBG("src.act_w:%d",RGA_req->src.act_w);
    OVLDBG("src.act_h:%d",RGA_req->src.act_h);

    OVLDBG("src.yrgb_addr:0x%X",RGA_req->src.yrgb_addr);
    OVLDBG("src.uv_addr:0x%X",RGA_req->src.uv_addr);
    OVLDBG("src.v_addr:0x%X",RGA_req->src.v_addr);

    OVLDBG("src.vir_w:%d",RGA_req->src.vir_w);
    OVLDBG("src.vir_h:%d",RGA_req->src.vir_h);
//Dst
    OVLDBG("dst.vir_w:%d",RGA_req->dst.vir_w);
    OVLDBG("dst.vir_h:%d",RGA_req->dst.vir_h);
    OVLDBG("dst.x_offset:%d",RGA_req->dst.x_offset);
    OVLDBG("dst.y_offset:%d",RGA_req->dst.y_offset);
    OVLDBG("dst.act_w:%d",RGA_req->dst.act_w);
    OVLDBG("dst.act_h:%d",RGA_req->dst.act_h);//1/2

    OVLDBG("dst.format:%d",RGA_req->dst.format);
    OVLDBG("dst.yrgb_addr:0x%X",RGA_req->dst.yrgb_addr);

    OVLDBG("clip.xmax:%d",RGA_req->clip.xmax);
    OVLDBG("clip.ymax:%d",RGA_req->clip.ymax);
}
#endif


//++++++++++++++++++++++++++++++++++++RGA++++++++++++++++++++++++++++++++++++++++++
int ovlInitRGAHW()
{
	int fd;
	fd = open(FB_DEV_RGA, O_RDWR);
	if(fd < 0)
		fd = 0;
    return fd;
}
//-------------------------------------------------------------
int ovlRgaBlit(int syncmode)
{
    int ret, timeout = 0;

#ifdef DEBUGRGA
    DdgPrintRGA(&pOvl_priv->RGA_req);
#endif
    if(pOvl_priv->OvlFb[EMU2Layer_RGA].fd){
    	while(pthread_mutex_trylock(&pOvl_priv->rgamutex) ==  EBUSY){
    		timeout++;
    		if(timeout > HW_TIMEOUT){
    			OVLDBG("Timeout rga");
    			return -EBUSY;
    		}
    		usleep(1);
    	}
    	ret = ioctl(pOvl_priv->OvlFb[EMU2Layer_RGA].fd, syncmode, &pOvl_priv->RGA_req);
    	pthread_mutex_unlock(&pOvl_priv->rgamutex);
    }else
    	ret = -ENODEV;

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
    pOvl_priv->RGA_req.dst.vir_w = Dst_vir;
    pOvl_priv->RGA_req.dst.vir_h = pOvl_priv->OvlLay[UILayer].var.yres_virtual;
//    pOvl_priv->RGA_req.dst.vir_h = Src_h;
//    RGA_req.dst.x_offset = Drw_x;
//    RGA_req.dst.y_offset = Drw_y;
    pOvl_priv->RGA_req.dst.act_w = pOvl_priv->RGA_req.src.act_w;
    pOvl_priv->RGA_req.dst.act_h = pOvl_priv->RGA_req.src.act_h;//1/2

    pOvl_priv->RGA_req.dst.format = DstFrmt;
    pOvl_priv->RGA_req.dst.yrgb_addr = DstYAddr;

    pOvl_priv->RGA_req.clip.xmax = pOvl_priv->RGA_req.dst.vir_w -1;
    pOvl_priv->RGA_req.clip.ymax = pOvl_priv->RGA_req.dst.vir_h -1;

//    RGA_req.src_trans_mode = 1;
    pOvl_priv->RGA_req.scale_mode = 1; // 0 nearst / 1 bilnear / 2 bicubic
//    pOvl_priv->RGA_req.render_mode = pre_scaling_mode;

    if(!PhyAdr){
    	pOvl_priv->RGA_req.mmu_info.mmu_en = 1;
    	pOvl_priv->RGA_req.mmu_info.mmu_flag = 0b100001;  /* [0] mmu enable [1] src_flush [2] dst_flush [3] CMD_flush [4~5] page size*/
    }
}
//---------------------------------------------------------------
int ovlRGASetFormats(OvlLayoutFormatType format, RGAUpdModeType UpMode)
{
    uint8_t		RGA_mode;
    int ret = 0, rgb_mode=0;

    switch(format) {
    case RKL_FORMAT_RGBA_8888:
    	RGA_mode = RK_FORMAT_RGBA_8888;
    	rgb_mode = 1;
    	break;
    case RKL_FORMAT_RGBX_8888:
    	RGA_mode = RK_FORMAT_RGBX_8888;
    	rgb_mode = 1;
    	break;
    case RKL_FORMAT_BGRA_8888:
    	RGA_mode = RK_FORMAT_BGRA_8888;
    	rgb_mode = 1;
    	break;
    case RKL_FORMAT_RGB_888:
    	RGA_mode = RK_FORMAT_RGB_888;
    	rgb_mode = 1;
    	break;
    case RKL_FORMAT_RGB_565:
    	RGA_mode = RK_FORMAT_RGB_565;
    	rgb_mode = 1;
    	break;
    case RKL_FORMAT_UV_NV12_SP:
    	RGA_mode = RK_FORMAT_YCbCr_420_SP;
        break;
    case RKL_FORMAT_UV_NV16_SP:
    	RGA_mode = RK_FORMAT_YCbCr_422_SP;
    	break;
    case RKL_FORMAT_VU_NV21_SP:
    	RGA_mode = RK_FORMAT_YCrCb_420_SP;
        break;
    case RKL_FORMAT_VU_NV61_SP:
    	RGA_mode = RK_FORMAT_YCrCb_422_SP;
    	break;
    case RKL_FORMAT_420_P:
    	RGA_mode = RK_FORMAT_YCbCr_420_P;
        break;
    case RKL_FORMAT_422_P:
    	RGA_mode = RK_FORMAT_YCbCr_422_P;
    	break;

/*    case RK_FORMAT_YCrCb_444:
    	break;*/
    default:
    	ERRMSG( "HW:Error RGA format:%d",format);
    	RGA_mode = RK_FORMAT_RGBX_8888;
    	rgb_mode = 1;
    	ret = -1;
    }

    if(UpMode == SRC_MODE || UpMode == BOTH_MODE){
    	pOvl_priv->RGA_req.src.format = RGA_mode;
	}
    if(UpMode == DST_MODE || UpMode == BOTH_MODE){
    	if(rgb_mode)
    		pOvl_priv->RGA_req.dst.format = RGA_mode;
    	else{
        	ERRMSG( "HW:Error, only RGB mode for destination");
        	ret = -1;
    	}
	}
    return ret;
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
void ovlRGASetSrc(uint32_t Y_RGB_Addr, uint32_t U_UV_Addr, uint32_t V_Addr)
{
    pOvl_priv->RGA_req.src.yrgb_addr = Y_RGB_Addr;
    pOvl_priv->RGA_req.src.uv_addr  = U_UV_Addr;
    pOvl_priv->RGA_req.src.v_addr   = V_Addr;
}
//---------------------------------------------------------------
void ovlRGASetDst(uint32_t Y_RGB_Addr, uint32_t U_UV_Addr, uint32_t V_Addr, int Dst_vir)
{
	pOvl_priv->RGA_req.dst.yrgb_addr = Y_RGB_Addr;
	pOvl_priv->RGA_req.dst.uv_addr  = U_UV_Addr;
	pOvl_priv->RGA_req.dst.v_addr   = V_Addr;
	if(Dst_vir)
		pOvl_priv->RGA_req.dst.vir_w = Dst_vir;
}
