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


#ifndef __RK_LAYER_H_
#define __RK_LAYER_H_

#include <ump/ump.h>
#include <ump/ump_ref_drv.h>
#include <linux/fb.h>

typedef int Bool;
typedef int OvlMemPg;
typedef int OvlFbPg;
typedef int OvlLayPg;

typedef enum {
    UIFB_MEM,
    FB_MEM,
    BUF_MEM,
} OvlMemPgType;

typedef enum {
    ERRORL=-1,
    UIL=0,
    SCALEL=1,
//    NOT_SCALEL=2,
    ANYL =3,
//    IPPScale = 1+4
} OvlLayoutType;

typedef enum
{
    FRONT_FB=0,
    BACK_FB=1,
	NEXT_FB=10,
} OvlFbBufType;

typedef enum
{
	ALC_NONE_FB=0,
	ALC_FRONT_FB=1,
	ALC_FRONT_BACK_FB=2,
} OvlFbBufAllocType;

typedef void OvlMemPgRec, *OvlMemPgPtr;

typedef void OvlFbRec, *OvlFbPtr;

typedef void OvlLayRec, *OvlLayPtr;

int Open_RkLayers(void);
void Close_RkLayers(void);
void OvlUpdFbMod(struct fb_var_screeninfo *var);
int OvlInitMainFB(const char *dev_name, int depth);
int OvlSetHDMI(int xres,int yres);

//int OvlClearBuf(OvlMemPgPtr PMemPg);
//int OvlReset();
void OvlCopyPackedToFb(OvlMemPgPtr PMemPg, const void *src, int dstPitch, int h, int w, Bool reverse);
void OvlCopyPlanarToFb(OvlMemPgPtr PMemPg, const void *src_Y, unsigned int offs_U, unsigned int offs_V,
		int dstPitch, int h, int w);
int OvlSetModeFb(OvlLayPg layout, unsigned short xres, unsigned short yres, unsigned char mode);
int OvlResetFB(OvlLayPg layout);
int OvlCopyHWBufCF(uint32_t SrcYAddr, uint32_t SrcUVAddr, uint32_t SrcVAddr,
				int SrcFrmt, int DstFrmt, uint32_t DstYAddr,
				int Drw_w, int Drw_h, int Drw_x, int Drw_y, int Src_vir, int Dst_vir, Bool useMMU);
//-------------------------------------------------------------
OvlMemPgPtr OvlGetBufByLay(OvlLayPg layout, OvlFbBufType BufType);
int OvlGetVXresByLay(OvlLayPg layout);
int OvlGetUIBpp(void);
int OvlGetSidByMemPg( OvlMemPgPtr PMemPg);
int OvlRkModeByFOURCC(int fourcc);
//-------------------------------------------------------------
//int OvlWaitSync( OvlLayPg layout);
int OvlCpBufToDisp(OvlMemPgPtr PMemPg, OvlLayPg layout);
int OvlFlipFb(OvlLayPg layout, OvlFbBufType flip, Bool clrPrev);
int Ovl2dBlt(uint32_t *src_bits, uint32_t *dst_bits, int src_stride, int dst_stride, int src_bpp, int dst_bpp, int src_x, int src_y, int dst_x, int dst_y, int w, int h);
//-------------------------------------------------------------
int OvlSetColorKey(uint32_t color);
int OvlEnable(OvlLayPg layout, int enable);
int OvlSetupBufDrw(OvlLayPg layout, int Drw_x, int Drw_y, int Drw_w, int Drw_h, int SrcPitch);
int OvlSetupDrw(OvlLayPg layout, int Drw_x, int Drw_y, int Drw_w, int Drw_h, int Src_w, int Src_h);
int OvlSetupFb(OvlLayPg layout, int SrcFrmt, int DstFrmt, unsigned short xres, unsigned short yres);
//------------------------------------------------------------
void * OvlMapBufMem(OvlMemPgPtr PMemPg);
int OvlUnMapBufMem(OvlMemPgPtr PMemPg);
OvlLayPg OvlAllocLay(OvlLayoutType type, OvlFbBufAllocType FbBufAlloc);
void OvlFreeLay(OvlLayPg layout);
OvlMemPgPtr OvlAllocMemPg(unsigned long size);
int OvlFreeMemPg(OvlMemPgPtr PMemPg);

#endif
