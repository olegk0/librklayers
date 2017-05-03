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


#ifndef __RK_LAYER_PRIV_H_
#define __RK_LAYER_PRIV_H_

#define HW_TIMEOUT 100

#define VERSION_MAJOR  0
#define VERSION_MINOR  20
#define VERSION_BUILD  0

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>

#include "rk_layers.h"
#ifdef IPP_ENABLE
#include "ipp.h"
#endif
#ifdef RGA_ENABLE
#include "rga.h"
#endif
#include <linux/fb.h>
#include <stdio.h>
#include "chroma_neon.h"
#include "out_cache_defs.h"

enum {
        HW_NONE=0,
        HW_UI=1,
        HW_SCALE= 2,
        HW_YUV= 4,
        HW_HWC= 8,
};

#include "rk3288.h"

#define FB_DEV_TMP FB_DEV "%d"

/*#define RGA_ENABLE 1
#define IPP_ENABLE 1
*/
//#define DEBUG 1

#ifdef DEBUG
#define OVLDBG(format, args...)		fprintf(stderr, "RK_LAY(%s):" format "\n", __func__, ## args)
#else
#define OVLDBG(format, args...)
#endif

#define ERRMSG(format, args...)		fprintf(stderr, "RK_ERR(%s):"format "\n", __func__, ## args)

#define PAGE_MASK    (getpagesize() - 1)
#define MFREE(p)	{free(p);p=NULL;}

#define TRUE 1
#define FALSE 0

//#define MAX_OVERLAYs HW_OVERLAYs
#ifdef IPP_ENABLE
    #ifdef RGA_ENABLE
	#define MAX_OVERLAYs HW_OVERLAYs + 2
    #else
	#define MAX_OVERLAYs HW_OVERLAYs + 1
    #endif
#else
    #ifdef RGA_ENABLE
	#define MAX_OVERLAYs HW_OVERLAYs + 1
    #else
	#define MAX_OVERLAYs HW_OVERLAYs
    #endif
#endif

#ifdef RGA_ENABLE
typedef enum {
	SRC_MODE=0,
	DST_MODE,
	BOTH_MODE,
} RGAUpdModeType;
#endif

#define UILayer 0
#define EMU1Layer_IPP HW_OVERLAYs
#define EMU2Layer_RGA EMU1Layer_IPP+1

typedef struct
{
	int ucount;
	int lay_used[MAX_OVERLAYs];
} SHMdt;

typedef struct {
	uint32_t	mem_id;
	uint32_t	mem_handle;
	unsigned char	*fb_mmap;
	unsigned long	buf_size;
	unsigned long	phy_addr;
	OvlMemPgType	MemPgType;
	unsigned long	offset_uv;
} ovlMemPgRec, *ovlMemPgPtr;

typedef struct {
	int				fd;
	ovlMemPgPtr		CurMemPg;
	struct fb_fix_screeninfo	fix;
//	unsigned long	offset_mio;
	OvlLayoutType	Type;
} ovlFbRec, *ovlFbPtr;

typedef struct {
	ovlFbPtr		OvlFb;
	OvlFbBufType	FbBufUsed;
	ovlMemPgPtr		FbMemPgs[2];
	struct fb_var_screeninfo	var;
	OvlLayoutType	ReqType;
	Bool			InUse;
//	Bool			ResChange;
	int			lock_fd;
} ovlLayRec, *ovlLayPtr;

typedef struct {
	int			fd_DRM;
	ovlLayRec		OvlLay[MAX_OVERLAYs];
	ovlFbRec		OvlFb[MAX_OVERLAYs];
	uint32_t		MaxPgSize;
	struct fb_var_screeninfo	cur_var;
//	struct fb_var_screeninfo	sav_var;
//	Bool			ResChange;
	Bool			OvlsAvl[MAX_OVERLAYs];
	int				OvlsCnt;
	uint32_t		Panel_w;
	uint32_t		Panel_h;
	cache_page_prm_t cache_page_params;
	uint32_t		*cache_mem_maps[MAX_CACHE_PAGES];
	OvlMemPgPtr		CacheMemPg;
#ifdef RGA_ENABLE
//	int				fd_RGA;
	pthread_mutex_t	rgamutex;
	struct rga_req	RGA_req;
#endif
#ifdef IPP_ENABLE
	unsigned long	ipp_dst_addr;
//	int				fd_IPP;
	pthread_mutex_t	ippmutex;
	struct rk29_ipp_req	IPP_req;
#endif
	uint32_t		ColorKeyDef;
} OvlHWRec, *OvlHWPtr;

#define ToIntMemPg(mpg)	((ovlMemPgPtr)mpg)
#define ToIntFb(fb)	((ovlFbPtr)fb)
#define FbByLay(layout) (pOvl_priv->OvlLay[layout].OvlFb)
#define MBufByLay(layout) (FbByLay(layout)->CurMemPg)

#define LayIsUIfb(layout)	(pOvl_priv->OvlFb[layout].Type == UI_L)

#define LayValid(lay) (lay < MAX_OVERLAYs && lay >= 0 && pOvl_priv->OvlsAvl[lay])
#define LayHWValid(lay) (lay < HW_OVERLAYs && lay >= 0 && pOvl_priv->OvlsAvl[lay])
#define LayValidAndNotUI(lay) (LayValid(lay) && !LayIsUIfb(lay))
#define LayHWValidAndNotUI(lay) (LayHWValid(lay) && !LayIsUIfb(lay))

#define MemPgIsUI(mpg)	(ToIntMemPg(mpg)->MemPgType == UIFB_MEM)

extern OvlHWPtr pOvl_priv;

ovlMemPgPtr ovlInitMemPgDef();
int ovlInitDRMHW();
//int ovlDRMAllocMem( struct usi_ump_mbs *uum);
int ovlDRMFreeMem( uint32_t	mem_id);
//int ovlDRMGetStat( struct usi_ump_mbs_info *uumi);
//int ovlDRMAllocRes(int res);
//int ovlDRMFreeRes(int res);
int ovlclearbuf( ovlMemPgPtr PMemPg);

#ifdef IPP_ENABLE
int ovlInitIPPHW();
int ovlIppBlit();
void ovlIppInitReg( uint32_t SrcYAddr, int SrcFrmt, int Src_w, int Src_h,
		uint32_t DstYAddr, int Src_vir, int Dst_vir);
void ovlIPPSetFormats(OvlLayoutFormatType format);
void ovlIPPSetDrw( int Drw_w, int Drw_h, int Drw_x, int Drw_y);
void ovlIPPSetSrc(uint32_t SrcYAddr);
void ovlIPPSetDst(uint32_t DstYAddr, int Dst_vir);
#endif

#ifdef RGA_ENABLE
int ovlInitRGAHW();
int ovlRgaBlit(int syncmode);
void ovlRgaInitReg(uint32_t SrcYAddr, int SrcFrmt, int DstFrmt,
		uint32_t DstYAddr, int Src_w, int Src_h, int Src_vir, int Dst_vir, Bool PhyAdr);
int ovlRGASetFormats(OvlLayoutFormatType format, RGAUpdModeType UpMode);
void ovlRGASetDrw( int Drw_w, int Drw_h, int Drw_x, int Drw_y);
void ovlRGASetSrc(uint32_t Y_RGB_Addr, uint32_t U_UV_Addr, uint32_t U_Addr);
void ovlRGASetDst(uint32_t Y_RGB_Addr, uint32_t U_UV_Addr, uint32_t U_Addr, int Dst_vir);
#endif

#endif
