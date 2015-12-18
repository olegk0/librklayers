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


#ifndef __RK_LAYER_PRIV_H_
#define __RK_LAYER_PRIV_H_

#define HW_TIMEOUT 100

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>

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

#ifdef DEBUG
#define OVLDBG(format, args...)		printf("RK_LAY(%s):" format "\n", __func__, ## args)
#else
#define OVLDBG(format, args...)
#endif

#define ERRMSG(format, args...)		printf("RK_ERR(%s):"format "\n", __func__, ## args)

#define PAGE_MASK    (getpagesize() - 1)
#define MFREE(p)	{free(p);p=NULL;}

#define TRUE 1
#define FALSE 0

#define MAX_OVERLAYs 3
/*
#define SRC_MODE TRUE
#define DST_MODE FALSE
*/

#define DEBUG 1

enum {
	UILayer=0,
	Ovl1Layer=1,
	Ovl2Layer=2,
};

typedef struct {
	ump_secure_id	ump_fb_secure_id;
	ump_handle		ump_handle;
	unsigned char	*fb_mmap;
	unsigned long	buf_size;
	unsigned long	phy_addr;
	OvlMemPgType	MemPgType;
	unsigned long	offset_mio;
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
#ifdef RGA_ENABLE
	struct rga_req	RGA_req;
#endif
#ifdef IPP_ENABLE
	struct rk29_ipp_req	IPP_req;
#endif
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
	Bool			ResChange;
	int				OvlsCnt;
#ifdef RGA_ENABLE
	int				fd_RGA;
	pthread_mutex_t	rgamutex;
#endif
#ifdef IPP_ENABLE
	int				fd_IPP;
	pthread_mutex_t	ippmutex;
#endif
} OvlHWRec, *OvlHWPtr;

#define ToIntMemPg(mpg)	((ovlMemPgPtr)mpg)
#define ToIntFb(fb)	((ovlFbPtr)fb)
#define FbByLay(layout) (Ovl_priv.OvlLay[layout].OvlFb)
#define MBufByLay(layout) (FbByLay(layout)->CurMemPg)

#define LayIsUIfb(layout)	(FbByLay(layout)->Type == UIL)
#define LayValid(lay) (lay < Ovl_priv.OvlsCnt && lay >= 0)
#define LayValidAndNotUI(lay) (LayValid(lay) && !LayIsUIfb(lay))

#define MemPgIsUI(mpg)	(ToIntMemPg(mpg)->MemPgType == UIFB_MEM)

extern OvlHWRec Ovl_priv;

ovlMemPgPtr ovlInitMemPgDef();
int ovlInitUSIHW();
int ovlUSIAllocMem( struct usi_ump_mbs *uum);
int ovlUSIFreeMem( ump_secure_id	secure_id);
int ovlUSIGetStat( struct usi_ump_mbs_info *uumi);
int ovlclearbuf( ovlMemPgPtr PMemPg);


#endif
