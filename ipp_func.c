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

//++++++++++++++++++++++++++++++++++++IPP++++++++++++++++++++++++++++++++++++++++++
int ovlInitIPPHW()
{
    return open(FB_DEV_IPP, O_RDWR);
}
//-------------------------------------------------------------
int ovlIppBlit()
{
    int ret, timeout = 0;

    while(pthread_mutex_trylock(&pOvl_priv->ippmutex) ==  EBUSY){
	timeout++;
	if(timeout > HW_TIMEOUT){
	    OVLDBG("Timeout ipp");
	    return -1;
	}
    }
    ret = ioctl(pOvl_priv->OvlFb[EMU1Layer_IPP].fd, IPP_BLIT_SYNC, &pOvl_priv->IPP_req);
    pthread_mutex_unlock(&pOvl_priv->ippmutex);
#ifdef DEBUG
    if(ret)
    	OVLDBG("ret:%d",ret);
#endif
    return ret;
}
//-----------------------------------------------------------------------
void ovlIppInitReg(uint32_t SrcYAddr, int SrcFrmt, int Src_w, int Src_h,
		uint32_t DstYAddr, int Src_vir, int Dst_vir)
{
    memset(&pOvl_priv->IPP_req, 0, sizeof(struct rk29_ipp_req));

    pOvl_priv->IPP_req.src0.w = Src_w;
    pOvl_priv->IPP_req.src0.h = Src_h;
    pOvl_priv->IPP_req.src_vir_w = Src_vir;

    pOvl_priv->IPP_req.src0.fmt = SrcFrmt;
    pOvl_priv->IPP_req.dst0.fmt = pOvl_priv->IPP_req.src0.fmt;
    pOvl_priv->IPP_req.dst_vir_w = Dst_vir;
    pOvl_priv->IPP_req.timeout = 100;
    pOvl_priv->IPP_req.flag = IPP_ROT_0;

    pOvl_priv->IPP_req.src0.YrgbMst = SrcYAddr;
//    IPP_req.src0.CbrMst = SrcUVAddr;
    pOvl_priv->IPP_req.dst0.YrgbMst = DstYAddr;
//    IPP_req.dst0.CbrMst = DstUVAddr;
    pOvl_priv->IPP_req.dst0.w = pOvl_priv->IPP_req.src0.w;
    pOvl_priv->IPP_req.dst0.h = pOvl_priv->IPP_req.src0.h;
}
//--------------------------------------------------------------------------------
void ovlIPPSetFormats(OvlLayoutFormatType format)
{
    uint32_t IPP_mode=IPP_XRGB_8888;

    switch(format) {
/*    case RK_FORMAT_RGB_888:
    	IPP_mode = 0;//TODO: add support to ipp
    	break;*/
    case RKL_FORMAT_RGB_565:
    	IPP_mode = IPP_RGB_565;
    	break;
    case RKL_FORMAT_YCrCb_NV12_SP:
    	IPP_mode = IPP_Y_CBCR_H2V2;//nearest suitable
        break;
    case RKL_FORMAT_YCbCr_422_SP:
    	IPP_mode = IPP_Y_CBCR_H2V1;//nearest suitable
    	break;
/*    case RK_FORMAT_YCrCb_NV12_P:
    	IPP_mode = IPP_Y_CBCR_H2V2;
        break;
    case RKL_FORMAT_YCbCr_422_P:
    	IPP_mode = IPP_Y_CBCR_H2V1;
    	break;
    	*/
/*    case RK_FORMAT_YCrCb_444:
    	break;*/
//    case RKL_FORMAT_RGBX_8888:
//    case RKL_FORMAT_RGBA_8888:
    default:
    	IPP_mode = IPP_XRGB_8888;
    }

    pOvl_priv->IPP_req.src0.fmt = IPP_mode;
	pOvl_priv->IPP_req.dst0.fmt = IPP_mode;
}
//--------------------------------------------------------------------------------
void ovlIPPSetDrw(uint32_t DstYAddr, int Drw_w, int Drw_h, int Drw_x, int Drw_y, int Dst_vir)
{
	uint32_t adr_offs;

    pOvl_priv->IPP_req.dst0.w = Drw_w;
    pOvl_priv->IPP_req.dst0.h = Drw_h;
    switch(pOvl_priv->IPP_req.src0.fmt){
    case IPP_RGB_565:
    	adr_offs = ((Drw_y*Dst_vir+Drw_x)<<1);
    	break;
//    case IPP_XRGB_8888:
    default:
    	adr_offs = ((Drw_y*Dst_vir+Drw_x)<<2);

/*	IPP_mode = IPP_Y_CBCR_H2V2;//nearest suitable
	IPP_mode = IPP_Y_CBCR_H2V1;//nearest suitable
	IPP_mode = IPP_Y_CBCR_H2V2;
	IPP_mode = IPP_Y_CBCR_H2V1;*/
    }
    OVLDBG("fmt:%d DstYAddr:0x%X  adr_offs:%d",pOvl_priv->IPP_req.src0.fmt,DstYAddr,adr_offs);
    pOvl_priv->IPP_req.dst0.YrgbMst = DstYAddr + adr_offs;
}
//--------------------------------------------------------------------------------
void ovlIPPSetSrc(uint32_t SrcYAddr)
{
	pOvl_priv->IPP_req.src0.YrgbMst = SrcYAddr;
	//    IPP_req.src0.CbrMst = SrcUVAddr;
}
/*
static int ovlcopyhwbufscale(ScrnInfoPtr pScrn,
				unsigned int SrcYAddr, unsigned int SrcUVAddr, int SrcFrmt,
				unsigned int DstYAddr, unsigned int DstUVAddr,
				int Src_w, int Src_h, int Drw_w, int Drw_h, int Src_vir, int Dst_vir)
{
    FBDevPtr pMxv = FBDEVPTR(pScrn);
    OvlHWPtr overlay = pMxv->OvlHW;
    struct rk29_ipp_req ipp_req;
    int ret, timeout = 0;

    while(pthread_mutex_trylock(&overlay->ippmutex) ==  EBUSY){
	timeout++;
	if(timeout > HW_TIMEOUT) return -1;
    }

    memset(&ipp_req, 0, sizeof(struct rk29_ipp_req));

    ipp_req.src0.w = Src_w;
    ipp_req.src0.h = Src_h;
    ipp_req.src_vir_w = Src_vir;

//IPP_XRGB_8888
    ipp_req.src0.fmt = SrcFrmt;
    ipp_req.dst0.fmt = ipp_req.src0.fmt;
    ipp_req.dst_vir_w = Dst_vir;
    ipp_req.timeout = 100;
    ipp_req.flag = IPP_ROT_0;

    ipp_req.src0.YrgbMst = SrcYAddr;
    ipp_req.src0.CbrMst = SrcUVAddr;
    ipp_req.dst0.YrgbMst = DstYAddr;
    ipp_req.dst0.CbrMst = DstUVAddr;
    ipp_req.dst0.w = ipp_req.src0.w;
    ipp_req.dst0.h = ipp_req.src0.h;

    ret = IppBlit(pScrn, &ipp_req);
    pthread_mutex_unlock(&overlay->ippmutex);
    return ret;
}
//--------------------------------------------------------------------------------
int OvlPutBufToSrcn(ScrnInfoPtr pScrn, unsigned int SrcBuf, int Src_vir,
				int Drw_w, int Drw_h, int Drw_x, int Drw_y, int pa_code)
{
    FBDevPtr pMxv = FBDEVPTR(pScrn);
    OvlHWPtr overlay = pMxv->OvlHW;

//ErrorF("-----Enter===SrcBuf:%X RGA_mode:%d Drw_w:%d Drw_h:%d Drw_x:%d Drw_y:%d Src_vir:%d Dst_vir:%d  pa_l:%d pa_in:%d\n",
//    SrcBuf, overlay->RGA_mode, Drw_w, Drw_h, Drw_x, Drw_y, Src_vir, overlay->var.xres_virtual ,overlay->rga_pa ,pa_code);

    if(overlay->rga_pa == 0){
	overlay->rga_pa = 1;
    }
    else
	if(overlay->rga_pa != pa_code){
	    ovlcopyhwbufscale(pScrn, SrcBuf, 0, overlay->IPP_mode,
		overlay->phadr_mem[0]+((Drw_y*overlay->var.xres_virtual+Drw_x)<<2), 0,
		Drw_w, Drw_h, Drw_w, Drw_h, Src_vir, overlay->var.xres_virtual);
	    return 0;
	}
    ovlcopyhwbufchfrmt(pScrn, SrcBuf, 0, 0, overlay->RGA_mode, overlay->RGA_mode,
	overlay->phadr_mem[0], Drw_w, Drw_h, Drw_x, Drw_y, Src_vir, overlay->var.xres_virtual);
    return 1;
}
*/
