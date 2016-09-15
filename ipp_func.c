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

//#define DEBUGIPP

#ifdef DEBUGIPP
void DdgPrintIPP( struct rk29_ipp_req *IPP_req)
{
    OVLDBG("src_vir_w:%d",IPP_req->src_vir_w);
    OVLDBG("src0.fmt:%d",IPP_req->src0.fmt);
    OVLDBG("srcYrgbMst:%X",IPP_req->src0.YrgbMst);
	OVLDBG("src0.h:%d",IPP_req->src0.h);
	OVLDBG("src0.w:%d\n",IPP_req->src0.w);

    OVLDBG("dst_vir_w:%d",IPP_req->dst_vir_w);
    OVLDBG("dst0.fmt:%d",IPP_req->dst0.fmt);
    OVLDBG("dstYrgbMst:%X",IPP_req->dst0.YrgbMst);
	OVLDBG("dst0.h:%d",IPP_req->dst0.h);
	OVLDBG("dst0.w:%d\n",IPP_req->dst0.w);

	OVLDBG("flag:%d",IPP_req->flag);
}
#endif
//++++++++++++++++++++++++++++++++++++IPP++++++++++++++++++++++++++++++++++++++++++
int ovlInitIPPHW()
{
    return open(FB_DEV_IPP, O_RDWR);
}
//-------------------------------------------------------------
int ovlIppBlit()
{
    int ret, timeout = 0;

#ifdef DEBUGIPP
    DdgPrintIPP(&pOvl_priv->IPP_req);
#endif
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
    pOvl_priv->ipp_dst_addr = DstYAddr;
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
    case RKL_FORMAT_UV_NV12_SP:
    	IPP_mode = IPP_Y_CBCR_H2V2;//nearest suitable
        break;
    case RKL_FORMAT_UV_NV16_SP:
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
void ovlIPPSetDrw( int Drw_w, int Drw_h, int Drw_x, int Drw_y)
{
	uint32_t adr_offs;

    pOvl_priv->IPP_req.dst0.w = Drw_w;
    pOvl_priv->IPP_req.dst0.h = Drw_h;
    switch(pOvl_priv->IPP_req.src0.fmt){
    case IPP_RGB_565:
    	adr_offs = ((Drw_y*pOvl_priv->IPP_req.dst_vir_w+Drw_x)<<1);
    	break;
//    case IPP_XRGB_8888:
    default:
    	adr_offs = ((Drw_y*pOvl_priv->IPP_req.dst_vir_w+Drw_x)<<2);

/*	IPP_mode = IPP_Y_CBCR_H2V2;//nearest suitable
	IPP_mode = IPP_Y_CBCR_H2V1;//nearest suitable
	IPP_mode = IPP_Y_CBCR_H2V2;
	IPP_mode = IPP_Y_CBCR_H2V1;*/
    }
    OVLDBG("fmt:%d DstYAddr:0x%lX  adr_offs:%d",pOvl_priv->IPP_req.src0.fmt,pOvl_priv->ipp_dst_addr,adr_offs);
    pOvl_priv->IPP_req.dst0.YrgbMst = pOvl_priv->ipp_dst_addr + adr_offs;
}
//--------------------------------------------------------------------------------
void ovlIPPSetSrc(uint32_t SrcYAddr)
{
	pOvl_priv->IPP_req.src0.YrgbMst = SrcYAddr;
	//    IPP_req.src0.CbrMst = SrcUVAddr;
}
//--------------------------------------------------------------------------------
void ovlIPPSetDst(uint32_t DstYAddr, int Dst_vir)
{
    pOvl_priv->IPP_req.dst0.YrgbMst = DstYAddr;
    pOvl_priv->ipp_dst_addr = DstYAddr;
    if(Dst_vir)
    	pOvl_priv->IPP_req.dst_vir_w = Dst_vir;
}
