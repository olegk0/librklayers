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
#include <ump/ump.h>
#include <ump/ump_ref_drv.h>
#include "rk3066.h"
#include <stdio.h>
#include "chroma_neon.h"

/*#define RGA_ENABLE 1
#define IPP_ENABLE 1
*/
#define DEBUG 1

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

#define MAX_OVERLAYs 5

#ifdef RGA_ENABLE
typedef enum {
	SRC_MODE=0,
	DST_MODE,
	BOTH_MODE,
} RGAUpdModeType;
#endif

enum {
	UILayer=0,
	Ovl1Layer=1,
	Ovl2Layer=2,
	EMU1Layer_RGA=3,
	EMU2Layer_IPP=4,
};

typedef struct
{
	int ucount;
	int lay_used[MAX_OVERLAYs];
} SHMdt;

typedef struct {
	ump_secure_id	ump_fb_secure_id;
	ump_handle		ump_handle;
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
} ovlLayRec, *ovlLayPtr;

typedef struct {
	int				fd_USI;
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
#ifdef RGA_ENABLE
//	int				fd_RGA;
	pthread_mutex_t	rgamutex;
	struct rga_req	RGA_req;
#endif
#ifdef IPP_ENABLE
//	int				fd_IPP;
	pthread_mutex_t	ippmutex;
	struct rk29_ipp_req	IPP_req;
#endif
} OvlHWRec, *OvlHWPtr;

#define ToIntMemPg(mpg)	((ovlMemPgPtr)mpg)
#define ToIntFb(fb)	((ovlFbPtr)fb)
#define FbByLay(layout) (pOvl_priv->OvlLay[layout].OvlFb)
#define MBufByLay(layout) (FbByLay(layout)->CurMemPg)

#define LayIsUIfb(layout)	(pOvl_priv->OvlFb[layout].Type == UI_L)

#define LayValid(lay) (lay < MAX_OVERLAYs && lay >= 0 && pOvl_priv->OvlsAvl[lay])
#define LayHWValid(lay) (lay < EMU1Layer_RGA && lay >= 0 && pOvl_priv->OvlsAvl[lay])
#define LayValidAndNotUI(lay) (LayValid(lay) && !LayIsUIfb(lay))
#define LayHWValidAndNotUI(lay) (LayHWValid(lay) && !LayIsUIfb(lay))

#define MemPgIsUI(mpg)	(ToIntMemPg(mpg)->MemPgType == UIFB_MEM)

extern OvlHWPtr pOvl_priv;

ovlMemPgPtr ovlInitMemPgDef();
int ovlInitUSIHW();
int ovlUSIAllocMem( struct usi_ump_mbs *uum);
int ovlUSIFreeMem( ump_secure_id	secure_id);
int ovlUSIGetStat( struct usi_ump_mbs_info *uumi);
int ovlUSIAllocRes(int res);
int ovlUSIFreeRes(int res);
int ovlclearbuf( ovlMemPgPtr PMemPg);

#ifdef IPP_ENABLE
int ovlInitIPPHW();
int ovlIppBlit();
void ovlIppInitReg( uint32_t SrcYAddr, int SrcFrmt, int Src_w, int Src_h,
		uint32_t DstYAddr, int Src_vir, int Dst_vir);
void ovlIPPSetFormats(OvlLayoutFormatType format);
void ovlIPPSetDrw(uint32_t DstYAddr, int Drw_w, int Drw_h, int Drw_x, int Drw_y, int Dst_vir);
void ovlIPPSetSrc(uint32_t SrcYAddr);
#endif

#ifdef RGA_ENABLE
int ovlInitRGAHW();
int ovlRgaBlit(int syncmode);
void ovlRgaInitReg(uint32_t SrcYAddr, int SrcFrmt, int DstFrmt,
		uint32_t DstYAddr, int Src_x, int Src_y, int Src_vir, int Dst_vir, Bool PhyAdr);
void ovlRGASetFormats(OvlLayoutFormatType format, RGAUpdModeType UpMode);
void ovlRGASetDrw( int Drw_w, int Drw_h, int Drw_x, int Drw_y);
void ovlRGASetSrc(uint32_t SrcYAddr);
#endif

#endif
