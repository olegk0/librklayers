/*
 *  For rk3066
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
static int ovlIppBlit( struct rk29_ipp_req *ipp_req)
{


    int ret, timeout = 0;

    while(pthread_mutex_trylock(&Ovl_priv.ippmutex) ==  EBUSY){
	timeout++;
	if(timeout > HW_TIMEOUT){
	    OVLDBG("Timeout ipp");
	    return -1;
	}
    }
    ret = ioctl(Ovl_priv.fd_IPP, IPP_BLIT_SYNC, ipp_req);
    pthread_mutex_unlock(&Ovl_priv.ippmutex);
    return ret;
}
//-----------------------------------------------------------------------
static void ovlIppInitReg( struct rk29_ipp_req *IPP_req, uint32_t SrcYAddr, int SrcFrmt, int Src_w, int Src_h,
		uint32_t DstYAddr, int Drw_w, int Drw_h, int Drw_x, int Drw_y, int Src_vir, int Dst_vir)
{



    memset(&IPP_req, 0, sizeof(struct rk29_ipp_req));

    IPP_req->src0.w = Src_w;
    IPP_req->src0.h = Src_h;
    IPP_req->src_vir_w = Src_vir;

    IPP_req->src0.fmt = SrcFrmt;
    IPP_req->dst0.fmt = IPP_req->src0.fmt;
    IPP_req->dst_vir_w = Dst_vir;
    IPP_req->timeout = 100;
    IPP_req->flag = IPP_ROT_0;

    IPP_req->src0.YrgbMst = SrcYAddr;
//    IPP_req->src0.CbrMst = SrcUVAddr;
    IPP_req->dst0.YrgbMst = DstYAddr;
//    IPP_req->dst0.CbrMst = DstUVAddr;
    IPP_req->dst0.w = IPP_req->src0.w;
    IPP_req->dst0.h = IPP_req->src0.h;
}
