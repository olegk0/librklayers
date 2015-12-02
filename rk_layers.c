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

#include <stdio.h>

#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "rk_layers_priv.h"
#include <pthread.h>

#include "fourcc.h"

OvlHWRec overlay;

#define MODPROBE_PATH_FILE      "/proc/sys/kernel/modprobe"
#define MAX_PATH                1024

#ifdef DEBUG
#define OVLDBG(format, args...)		printf("RK_LAY(%s):" format "\n", __func__, ## args)
#else
#define OVLDBG(format, args...)
#endif

#define ERRMSG(format, args...)		printf("RK_ERR(%s):"format "\n", __func__, ## args)

//******************************************************************************
//int xf86LoadKernelModule(const char *modName)
int LoadKernelModule(const char *modName)
{
    char mpPath[MAX_PATH] = "";
    int fd = -1, status, n;
    pid_t pid;

    /* get the path to the modprobe program */
    fd = open(MODPROBE_PATH_FILE, O_RDONLY);
    if (fd >= 0) {
        int count = read(fd, mpPath, MAX_PATH - 1);

        if (count <= 0) {
            mpPath[0] = 0;
        }
        else if (mpPath[count - 1] == '\n') {
            mpPath[count - 1] = 0;      /* replaces \n with \0 */
        }
        close(fd);
        /* if this worked, mpPath will be "/sbin/modprobe" or similar. */
    }

    if (mpPath[0] == 0) {
        /* we failed to get the path from the system, use a default */
        strcpy(mpPath, "/sbin/modprobe");
    }

    /* now fork/exec the modprobe command */
    /*
     * It would be good to capture stdout/stderr so that it can be directed
     * to the log file.  modprobe errors currently are missing from the log
     * file.
     */
    switch (pid = fork()) {
    case 0:                    /* child */
        /* change real/effective user ID to 0/0 as we need to
         * preinstall agpgart module for some DRM modules
         */
        if (setreuid(0, 0)) {
        	OVLDBG( "LoadKernelModule: "
                    "Setting of real/effective user Id to 0/0 failed");
        }
        setenv("PATH", "/sbin", 1);
        n = execl(mpPath, "modprobe", modName, NULL);
        OVLDBG( "LoadKernelModule %s\n", strerror(errno));
        exit(EXIT_FAILURE);     /* if we get here the child's exec failed */
        break;
    case -1:                   /* fork failed */
        return 0;
    default:                   /* fork worked */
    {
        /* XXX we loop over waitpid() because it sometimes fails on
         * the first attempt.  Don't know why!
         */
        int count = 0, p;

        do {
            p = waitpid(pid, &status, 0);
        } while (p == -1 && count++ < 4);

        if (p == -1) {
            return 0;
        }

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            return 1;           /* success! */
        }
        else {
            return 0;
        }
    }
    }

    /* never get here */
    return 0;
}

//--------------------------------------------------------------------------------
int OvlSetHDMI(int xres,int yres)
{
	int ret=0;
    FILE *fp;

	//	tmp = resToHDMImodes(overlay.cur_var.xres,overlay.cur_var.yres);
	//	ioctl(overlay.OvlFb[UserInterfaceFB].fd, FBIOSET_HDMI_MODE, &tmp);//use HDMI scaling

/*    switch(xres){
    case 640://"640x480p@60Hz"
	ret = 1;
    break;
    case 720://"720x480p@60Hz"
	if(yres == 480)
	    ret = 2;
	else
	    ret = 17;//"720x576p@50Hz"
    break;
    case 1280://"1280x720p@60Hz"=4
	ret = 4;//"1280x720p@50Hz"=19;"1280x720p@24Hz"=60;"1280x720p@25Hz"=61;"1280x720p@30Hz"=62
    break;
    case 1920://"1920x1080p@60Hz"=16
	ret = 16;//"1920x1080p@24Hz"=32;"1920x1080p@25Hz"=33;"1920x1080p@30Hz"=34;"1920x1080p@50Hz"=31
    break;
    default://"1920x1080p@60Hz"
	ret = 16;
    }
    */

	fp = fopen(FB_SYS_HDMI"/mode", "w");
	if(fp){
		fprintf(fp,HDMI_MODE_TMPL"\n",1280,720);//bug workarround
		fclose(fp);
	}
	usleep(10000);
	fp = fopen(FB_SYS_HDMI"/mode", "w");
	if(fp){
		fprintf(fp,HDMI_MODE_TMPL"\n",1920,1080);
		fclose(fp);
	}
		usleep(10000);
	fp = fopen(FB_SYS_HDMI"/mode", "w");
	if(fp){
		fprintf(fp,HDMI_MODE_TMPL"\n", xres, yres);
		fclose(fp);
	}else
		ERRMSG("Do not open "FB_SYS_HDMI"/mode");

	fp = fopen(FB_SYS_HDMI"/scale", "w");
	if(fp){
		fprintf(fp,"scalex=100");
		fprintf(fp,"scaley=100");
		fclose(fp);
	}

    return ret;
}
//-----------------------------------------------------------------
int ovlToHWRkFormat(OvlLayoutFormatType format)
{
	int ret;

    switch(format) {
    case RK_FORMAT_RGBA_8888:
		ret = RGBA_8888;
		break;
    case RK_FORMAT_RGBX_8888:
		ret = RGBX_8888;
		break;
    case RK_FORMAT_BGRA_8888:
		ret = BGRA_8888;
		break;
    case RK_FORMAT_RGB_888:
		ret = RGB_888;
		break;
    case RK_FORMAT_RGB_565:
		ret = RGB_565;
		break;
    case RK_FORMAT_RGBA_5551:
		ret = RGBA_5551;
		break;
    case RK_FORMAT_RGBA_4444:
		ret = RGBA_4444;
		break;
    case RK_FORMAT_YCbCr_422_SP:
		ret = YCbCr_422_SP;
		break;
    case RK_FORMAT_YCrCb_NV12_SP:
		ret = YCrCb_NV12_SP;
		break;
    case RK_FORMAT_YCrCb_444:
		ret = YCrCb_444;
		break;
    case RK_FORMAT_DEFAULT:
    default:
    	ret = 0;
    }

    return ret;
}
//--------------------------------------------------------------------------------

OvlMemPgPtr OvlGetBufByLay( OvlLayPg layout, OvlFbBufType BufType)
{
    if(LayValid(layout))
    	return overlay.OvlLay[layout].FbMemPgs[BufType];
//    	return MBufByLay(layout);
    else
    	return NULL;
}

//--------------------------------------------------------------------------------
int OvlGetVXresByLay( OvlLayPg layout)
{
	if(LayValid(layout))
		return overlay.OvlLay[layout].var.xres_virtual;
    else
    	return -1;
}

//--------------------------------------------------------------------------------
int OvlGetSidByMemPg( OvlMemPgPtr PMemPg)
{
    if(PMemPg == NULL)
    	return UMP_INVALID_SECURE_ID;
   	return ToIntMemPg(PMemPg)->ump_fb_secure_id;
}

//-----------------------------------------------------------------------------------
#if defined( RGA_ENABLE ) || defined( IPP_ENABLE )
static void ovlSelHwMods(OvlLayoutFormatType format, OvlLayPg layout, Bool Src)
{


    uint8_t		IPP_mode;
    uint8_t		RGA_mode;

    switch(format) {
    case RK_FORMAT_RGBX_8888:
    case RK_FORMAT_RGBA_8888:
    	RGA_mode = RK_FORMAT_RGBX_8888;
    	IPP_mode = IPP_XRGB_8888;
    	break;
    case RK_FORMAT_RGB_888:
    	RGA_mode = RK_FORMAT_RGB_888;
    	IPP_mode = 0;//TODO: add support to ipp
    	break;
    case RK_FORMAT_RGB_565:
    	RGA_mode = RK_FORMAT_RGB_565;
    	IPP_mode = IPP_RGB_565;
    	break;
    case RK_FORMAT_YCrCb_NV12_SP:
    	RGA_mode = RK_FORMAT_YCbCr_420_SP;
    	IPP_mode = IPP_Y_CBCR_H2V2;//nearest suitable
        break;
    case RK_FORMAT_YCbCr_422_SP:
    	RGA_mode = RK_FORMAT_YCrCb_422_SP;
    	IPP_mode = IPP_Y_CBCR_H2V1;//nearest suitable
    	break;
    case RK_FORMAT_YCrCb_NV12_P:
    	RGA_mode = RK_FORMAT_YCbCr_420_P;
    	IPP_mode = IPP_Y_CBCR_H2V2;
        break;
    case RK_FORMAT_YCbCr_422_P:
    	RGA_mode = RK_FORMAT_YCrCb_422_P;
    	IPP_mode = IPP_Y_CBCR_H2V1;
    	break;
    case RK_FORMAT_YCrCb_444:
    	break;
    default:
    	RGA_mode = RK_FORMAT_RGBX_8888;
    	IPP_mode = IPP_XRGB_8888;
    }
    if(layout >= 0 && layout < overlay.OvlsCnt){
    	if(Src){
#ifdef IPP_ENABLE
    		overlay.OvlLay[layout].IPP_req.src0.fmt = IPP_mode;
#endif
#ifdef RGA_ENABLE
    		overlay.OvlLay[layout].RGA_req.src.format = RGA_mode;
#endif
	}
	else{
#ifdef RGA_ENABLE
		overlay.OvlLay[layout].RGA_req.dst.format = RGA_mode;
#endif
	}
#ifdef IPP_ENABLE
    	overlay.OvlLay[layout].IPP_req.dst0.fmt = overlay.OvlLay[layout].IPP_req.src0.fmt;
#endif
    }
}
#endif
//*******************************************************************************
/*int OvlPanBufSync( OvlMemPgPtr PMemPg, OvlLayPg layout)
{
    uint32_t tmp[2];

    if(layout < OVLs && layout >= 0){
	tmp[0] = PMemPg->offset;
	tmp[1] = tmp[0];
	if(!LayIsUIfb( layout))
	    return ioctl(FbByLay(layout)->fd, FBIOSET_FBMEM_OFFS_SYNC, &tmp);
    }
	return -1;
}*/
//--------------------------------------------------------------------------------
int OvlGetUIBpp()
{
    int ret;

    switch(overlay.cur_var.nonstd){
    case RGB_565:
    	ret = 16;
    	break;
    case RGB_888:
    	ret = 24;
    	break;
    default:
    	ret = 32;
    }

    return ret;
}
//--------------------------------------------------------------------------------
int OvlSetColorKey( uint32_t color)
{
    return ioctl(overlay.OvlFb[UILayer].fd, FBIOSET_COLORKEY, &color);
}
//------------------------------------------------------------------
/*int OvlWaitSync( OvlLayPg layout)
{
    uint32_t tmp=0;

    if(!pMxv->WaitForSync)
	return -1;

    return ioctl(FbByLay(layout)->fd, FBIO_WAITFORVSYNC, &tmp);
}*/
//--------------------------------------------------------------------------------
int ovlclearbuf( ovlMemPgPtr PMemPg)
{
    if(PMemPg == NULL || PMemPg->fb_mmap == NULL || MemPgIsUI(PMemPg))
    	return -ENODEV;
	memset(PMemPg->fb_mmap,0,PMemPg->buf_size);
	return 0;
}
//--------------------------------------------------------------------
int OvlSetModeFb( OvlLayPg layout, unsigned short xres, unsigned short yres, OvlLayoutFormatType format)
{
    int ret=0;

    if(LayValidAndNotUI(layout)){/*TODO !UIL*/
    	if((xres > overlay.OvlLay[layout].var.xres_virtual)||(yres > overlay.OvlLay[layout].var.yres_virtual)) return -EINVAL;
//    if((xres > overlay.cur_var.xres)||(yres > overlay.cur_var.yres)) return -1;
    	if(format != RK_FORMAT_DEFAULT)
    		overlay.OvlLay[layout].var.nonstd = ovlToHWRkFormat(format);
    	if(xres>0){
    		overlay.OvlLay[layout].var.xres = xres;
//    		overlay.OvlLay[layout].var.xres_virtual = xres;
    	}
    	if(yres>0){
    		overlay.OvlLay[layout].var.yres = yres;
//    		overlay.OvlLay[layout].var.yres_virtual = yres;
    	}
    	ret = ioctl(FbByLay(layout)->fd, FBIOPUT_VSCREENINFO, &overlay.OvlLay[layout].var);

/*    	if(ret == 0){
    		ovlSelHwMods( overlay.OvlLay[layout].var.nonstd, layout, DST_MODE);
//    		overlay.OvlLay[layout].ResChange = FALSE;
    	}*/
        return ret;
    }
    else
    	return -ENODEV;
//    	ovlSelHwMods( overlay.cur_var.nonstd, layout, DST_MODE);

}
//--------------------------------------------------------------------------------
static int ovlUpdVarOnChangeRes( OvlLayPg layout)
{
	if(LayValid(layout)){
		memcpy(&overlay.OvlLay[layout].var, &overlay.cur_var, sizeof(struct fb_var_screeninfo));
//	overlay.OvlLay[layout].ResChange = FALSE;
		return 0;
    }else
    	return -ENODEV;
}
//----------------------------------------------------------------------------------
int OvlSetupFb( OvlLayPg layout, OvlLayoutFormatType SrcFrmt, OvlLayoutFormatType DstFrmt, unsigned short xres, unsigned short yres)
{
    int ret;

    if(LayValidAndNotUI(layout)){

    	ret = ovlUpdVarOnChangeRes( layout);
    	ret |= OvlSetModeFb( layout, xres , yres, DstFrmt);
//    if(!ret){
//    	ovlRgaInitReg( &overlay.OvlLay[layout].RGA_req, /*SrcYAddr*/0, 0, 0,
//    		FbByLay(layout)->CurMemPg->phy_addr, 0, 0, 0, 0, 0, 0, overlay.OvlLay[layout].var.xres_virtual/*TODO SRC*/, overlay.OvlLay[layout].var.xres_virtual, TRUE);
/*    	if(!SrcFrmt)
    		SrcFrmt = overlay.OvlLay[layout].var.nonstd;
    	ovlSelHwMods( SrcFrmt, layout, SRC_MODE);
    	*/
    }else
    	ret = -ENODEV;

    return ret;
}
//--------------------------------------------------------------------------------
/*int OvlSetupBufDrw( OvlLayPg layout, int Drw_x, int Drw_y, int Drw_w, int Drw_h, int SrcPitch)
{


    SFbioDispSet pt;
    OvlFbPg FbPg;

    if(layout >= overlay.OvlsCnt || layout < 0)
	return -1;
    if(SrcPitch){
    	ovlRgaDrwAdd( &overlay.OvlLay[layout].RGA_req, Drw_w, Drw_h, Drw_x, Drw_y, SrcPitch);
    	return 0;
    }
    return -1;
//    OvlClearBuf( overlay.OvlFb[overlay.OvlLay[layout].OvlFb].OvlMemPg);
}*/
//--------------------------------------------------------------------------------
int OvlSetupDrw( OvlLayPg layout, int Drw_x, int Drw_y, int Drw_w, int Drw_h, int Src_w, int Src_h)
{

    SFbioDispSet pt;
    int ret=0;
//    OvlFbPg FbPg;

    if(LayValidAndNotUI(layout)){

    	pt.poffset_x = Drw_x;
    	pt.poffset_y = Drw_y;
    	pt.ssize_w = Src_w;
    	pt.ssize_h = Src_h;
//    pt.scale_w = (Src_w*PANEL_SIZE_X)/Drw_w;
    	pt.scale_w = (Src_w*overlay.OvlLay[layout].var.xres)/Drw_w;
//    pt.scale_h = (Src_h*PANEL_SIZE_Y)/Drw_h;
    	pt.scale_h = (Src_h*overlay.OvlLay[layout].var.yres)/Drw_h;
    	ret = ioctl(FbByLay(layout)->fd, FBIOSET_DISP_PSET, &pt);
//    OvlClearBuf( overlay.OvlFb[overlay.OvlLay[layout].OvlFb].OvlMemPg);
    }else
    	ret = -ENODEV;
    return ret;
}
//----------------------------------------------------------------------------------
int OvlFbLinkMemPg( OvlFbPtr PFb, OvlMemPgPtr MemPg)
{
    int ret;
    unsigned long tmp[2];

    if(!PFb || ToIntFb(PFb)->Type == UIL)
    	return -EINVAL;

	tmp[0] = ToIntMemPg(MemPg)->phy_addr;
	tmp[1] = tmp[0] + ToIntMemPg(MemPg)->offset_mio;
    ret = ioctl(ToIntFb(PFb)->fd, RK_FBIOSET_YUV_ADDR, &tmp);
    if(!ret)
    	ToIntFb(PFb)->CurMemPg = MemPg;
	return ret;
}
//----------------------------------------------------------------------------------
int OvlFlipFb( OvlLayPg layout, OvlFbBufType flip, Bool clrPrev)
{       
    int ret, prev=-1;

    if(LayValidAndNotUI(layout)){

    	switch(flip){
    	case FRONT_FB:
    	case BACK_FB:
    		overlay.OvlLay[layout].FbBufUsed = flip;
    		break;
    	case NEXT_FB:
    	default:
    		prev = overlay.OvlLay[layout].FbBufUsed;
    		if(overlay.OvlLay[layout].FbBufUsed == FRONT_FB)
    			overlay.OvlLay[layout].FbBufUsed = BACK_FB;
    		else
    			overlay.OvlLay[layout].FbBufUsed = FRONT_FB;

    	}
    	ret = OvlFbLinkMemPg( FbByLay(layout) , overlay.OvlLay[layout].FbMemPgs[overlay.OvlLay[layout].FbBufUsed]);

    	if(clrPrev && prev >=FRONT_FB)
    		ovlclearbuf( overlay.OvlLay[layout].FbMemPgs[prev]);
//TODO sync ?
    }else
    	ret = -ENODEV;

    return ret;
}

//---------------------------------------------------------------------
int OvlEnable( OvlLayPg layout, int enable)
{       
    if(LayValidAndNotUI(layout))
    	return ioctl(FbByLay(layout)->fd, RK_FBIOSET_ENABLE, &enable);
    else
    	return -ENODEV;
}
//---------------------------------------------------------------------
int OvlResetFB( OvlLayPg layout)
{       
    int ret;

    if(LayValid(layout)){
	//    OvlClearBuf( 1);
    	ret = ioctl(FbByLay(layout)->fd, FBIOPUT_VSCREENINFO, &overlay.cur_var);
//    if(ret == 0 && dev == FBUI) //TODO res change by x func
//	ret =  OvlUpdSavMod();
    }else
    	ret = -ENODEV;

    return ret;
}
//--------------------------------------------------------------
/*OvlLayPg ovlSwapLay( OvlLayPg pg, OvlLayoutType type)
{

    OvlHWPtr	overlay = pMxv->OvlHW;
    OvlLayPg i,t;

    if(overlay.OvlLay[pg].ReqType==ANUL){
	for(i=0;i<OVLs;i++){
	    if(!overlay.OvlLay[i].InUse){
		t = FbByLay(i);
		overlay.OvlLay[i].OvlFb = FbByLay(pg);
		FbByLay(pg) = t;
		OvlSetupFb( 0, 0, pg);//TODO  call hw init fb, rga ipp init
		return i;
	    }
	}
    }
    return ERRORL;
}
*/
//------------------------------------------------------------------
OvlLayPg OvlAllocLay( OvlLayoutType type, OvlFbBufAllocType FbBufAlloc)
{
    OvlLayPg i;
    int lay=ERRORL;

    switch(type){
    case UIL:
    case SCALEL:
//    case NOT_SCALEL:
    	for(i=0;i<overlay.OvlsCnt;i++){
    		if(FbByLay(i)->Type == type){
    			if(!overlay.OvlLay[i].InUse){
    				lay = i;
    				break;
    			}
/*    			else{
//		    t = ovlSwapLay( i, type);
//		    if(t==ERRORL)
    				lay = ERRORL;
    			}*/
    		}
    	}
    	if(lay == overlay.OvlsCnt)
    		lay = ERRORL;
    	break;
    case ANYL://except UIL
    	for(i=1;i<overlay.OvlsCnt;i++){
    		if(!overlay.OvlLay[i].InUse){
    			lay = i;
    		}
    	}
    	break;
    default:
    	return ERRORL;
    }
    if(lay != ERRORL){
    	if(FbBufAlloc > ALC_NONE_FB){
//front fb first by def
    		if(!overlay.OvlLay[lay].FbMemPgs[FRONT_FB]){
        		if(lay == UIL){//User Interface layer
        			overlay.OvlLay[lay].FbMemPgs[FRONT_FB] = ovlInitMemPgDef();
        			if(overlay.OvlLay[lay].FbMemPgs[FRONT_FB]){
        				overlay.OvlLay[lay].FbMemPgs[FRONT_FB]->phy_addr = overlay.OvlFb[UIL].fix.smem_start;
        				overlay.OvlLay[lay].FbMemPgs[FRONT_FB]->buf_size = overlay.OvlFb[UIL].fix.smem_len;
        				overlay.OvlLay[lay].FbMemPgs[FRONT_FB]->MemPgType = UIFB_MEM;
        			}
        		}else{
        			overlay.OvlLay[lay].FbMemPgs[FRONT_FB] = OvlAllocMemPg( FB_MAXPGSIZE);//TODO size
        			if(overlay.OvlLay[lay].FbMemPgs[FRONT_FB])
        				overlay.OvlLay[lay].FbMemPgs[FRONT_FB]->MemPgType = FB_MEM;
        		}
    		}
    		if(!overlay.OvlLay[lay].FbMemPgs[FRONT_FB])
    			return ERRORL;

    		OvlFlipFb( lay, FRONT_FB, 0);
//and back fb if needed
    		if(FbBufAlloc > ALC_FRONT_FB && !overlay.OvlLay[lay].FbMemPgs[BACK_FB]){
    			overlay.OvlLay[lay].FbMemPgs[BACK_FB] = OvlAllocMemPg( FB_MAXPGSIZE);//TODO size
        		if(overlay.OvlLay[lay].FbMemPgs[BACK_FB])
        			overlay.OvlLay[lay].FbMemPgs[BACK_FB]->MemPgType = FB_MEM;
        		else{
        			OvlFreeMemPg( overlay.OvlLay[lay].FbMemPgs[FRONT_FB]);
        			overlay.OvlLay[lay].FbMemPgs[FRONT_FB] = NULL;
        			return ERRORL;
        		}
    		}
    	}

		overlay.OvlLay[lay].InUse = TRUE;
		overlay.OvlLay[lay].ReqType = type;
    }

    return lay;
}
//------------------------------------------------------------------
void OvlFreeLay( OvlLayPg layout)
{

    if(LayValid(layout)){
    	OvlFreeMemPg(  overlay.OvlLay[layout].FbMemPgs[FRONT_FB]);
    	overlay.OvlLay[layout].FbMemPgs[FRONT_FB] = NULL;
    	OvlFreeMemPg(  overlay.OvlLay[layout].FbMemPgs[BACK_FB]);
    	overlay.OvlLay[layout].FbMemPgs[BACK_FB] = NULL;
    	MBufByLay(layout) = NULL;
    	OvlEnable( layout, 0);
    	overlay.OvlLay[layout].InUse = FALSE;
    	overlay.OvlLay[layout].ReqType = ERRORL;
    }
}
//------------------------------------------------------------------
/*ump_secure_id ovlGetUmpId( OvlMemPg pg)
{

    OvlHWPtr	overlay = pMxv->OvlHW;
//    ump_secure_id ump_id = UMP_INVALID_SECURE_ID;

    if(pg >0 && pg < overlay.OvlMemPgs){
    	if(overlay.OvlMemPg[pg].ump_fb_secure_id == UMP_INVALID_SECURE_ID){
    		overlay.OvlMemPg[pg].ump_fb_secure_id = pg-1;
    		ioctl(overlay.OvlFb[MasterOvlFB].fd, GET_UMP_SECURE_ID_BUFn, &overlay.OvlMemPg[pg].ump_fb_secure_id);
    	}
    	return overlay.OvlMemPg[pg].ump_fb_secure_id;
    }
    else
    	return UMP_INVALID_SECURE_ID;
}
*/
//++++++++++++++++++++++++++++++init/close+++++++++++++++++++++++++
int OvlInitMainFB(const char *dev_name, int depth)
{
	int fd ,ret=0;
	struct fb_var_screeninfo tvar;

	fd = open(dev_name, O_RDONLY, 0);
	if (fd <= 0) {
		return -ENODEV;
	}
	/* get current fb device settings */
	ret = ioctl(fd, FBIOGET_VSCREENINFO, &tvar);
	if(ret){
		ERRMSG("ioctl FBIOGET_VSCREENINFO");
		goto err;
	}

	switch (depth){
	case 16:
		tvar.nonstd = RGB_565;
		tvar.red.length = 5;
		tvar.red.offset = 11;
		tvar.green.length = 6;
		tvar.green.offset = 5;
		tvar.blue.length = 5;
		tvar.blue.offset = 0;
		tvar.transp.length = 0;
		tvar.transp.offset = 0;
		tvar.bits_per_pixel = 16;
		break;
	case 24:
		tvar.nonstd = RGB_888;
		tvar.red.length = 8;
		tvar.red.offset = 0;
		tvar.green.length = 8;
		tvar.green.offset = 8;
		tvar.blue.length = 8;
		tvar.blue.offset = 16;
		tvar.transp.length = 0;
		tvar.transp.offset = 0;
		tvar.bits_per_pixel = 24;
		break;
	case 32:
	default:
//		tvar.nonstd = RGBX_8888;
		tvar.nonstd = BGRA_8888;
		tvar.bits_per_pixel = 32;
		tvar.red.length = 8;
		tvar.red.offset = 16;
		tvar.green.length = 8;
		tvar.green.offset = 8;
		tvar.blue.length = 8;
		tvar.blue.offset = 0;
		tvar.transp.length = 0;
		tvar.transp.offset = 0;
		tvar.bits_per_pixel = 32;
	}

	ret = ioctl(fd, FBIOPUT_VSCREENINFO, &tvar);
	if(ret){
		ERRMSG("ioctl FBIOPUT");
	}

 err:
	close(fd);
	return ret;
 }

//-----------------------------------------------------------------

void OvlUpdFbMod(struct fb_var_screeninfo *var)
{

    memcpy(&overlay.cur_var, var, sizeof(struct fb_var_screeninfo));
    OvlSetHDMI(var->xres, var->yres);
    overlay.cur_var.vmode |= FB_VMODE_CONUPDATE;
	overlay.cur_var.activate = FB_ACTIVATE_NOW;
	overlay.cur_var.grayscale = 0;

// 	overlay.ResChange = TRUE;
   	OVLDBG("Resolution changed to %dx%d ***", overlay.cur_var.xres, overlay.cur_var.yres);

}

//----------------------------------------------------------------------
void set_ovl_param()
{
    int i;

    for(i=0;i<overlay.OvlsCnt;i++){
    	ioctl(overlay.OvlFb[i].fd, FBIOGET_FSCREENINFO, &overlay.OvlFb[i].fix);
    	overlay.OvlFb[i].CurMemPg = NULL;

    	overlay.OvlLay[i].OvlFb = &overlay.OvlFb[i];
    	memcpy(&overlay.OvlLay[i].var, &overlay.cur_var, sizeof(struct fb_var_screeninfo));
    	overlay.OvlLay[i].InUse = FALSE;
    	overlay.OvlLay[i].ReqType = ERRORL;
//    	overlay.OvlLay[i].ResChange = FALSE;
    	overlay.OvlLay[i].FbBufUsed = FRONT_FB;
    	overlay.OvlLay[i].FbMemPgs[FRONT_FB] = NULL;
    	overlay.OvlLay[i].FbMemPgs[BACK_FB] = NULL;
    	if(i>0){
        	overlay.OvlFb[i].Type = SCALEL;//        UIL=0,    SCALEL=1,    NotSCALEL=2,
    		OvlSetModeFb(i,0,0,0);
    		OvlEnable( i, 0);
    	}else
        	overlay.OvlFb[i].Type = UIL;
    }

}

//------------------------------------------------------------------
int ovl_setup_ovl()
{
    int ret=0;
    overlay.OvlsCnt = 0;
    overlay.OvlFb[Ovl1Layer].fd = open(FB_DEV_O1, O_RDONLY); //main overlay
    if (overlay.OvlFb[Ovl1Layer].fd < 0){
    	ret = overlay.OvlFb[Ovl1Layer].fd;
    	goto err;
    }
    overlay.OvlsCnt++;
    overlay.OvlFb[UILayer].fd = open(FB_DEV_UI, O_RDONLY);
    if (overlay.OvlFb[UILayer].fd < 0){
    	ret = overlay.OvlFb[UILayer].fd;
    	goto err1;
    }
    overlay.OvlsCnt++;
    overlay.OvlFb[Ovl2Layer].fd = open(FB_DEV_O2, O_RDONLY);
    if (overlay.OvlFb[Ovl2Layer].fd < 0){
    	ret = overlay.OvlFb[Ovl2Layer].fd;
    	goto err2;
    }
    overlay.OvlsCnt++;

    overlay.MaxPgSize = FB_MAXPGSIZE;

    ioctl(overlay.OvlFb[UILayer].fd, FBIOGET_VSCREENINFO, &overlay.cur_var);

//	ioctl(overlay.fd_o1, FBIOBLANK, FB_BLANK_UNBLANK);
/*	tmp=1;
	ioctl(overlay.OvlFb[UILayer].fd, RK_FBIOSET_OVERLAY_STATE, &tmp);
        return(TRUE);
    }
*/
    return 0;
//err3:
    close(overlay.OvlFb[Ovl2Layer].fd);
err2:
    close(overlay.OvlFb[UILayer].fd);
err1:
    close(overlay.OvlFb[Ovl1Layer].fd);
err:
    return ret;
}
//-------------------------------------------------------
int ovl_init_ovl()
{
    int ret;

    ret = ovl_setup_ovl();
    if(ret) goto err;
    set_ovl_param();

#ifdef RGA_ENABLE
    pthread_mutex_init(&overlay.rgamutex, NULL);
#endif
#ifdef IPP_ENABLE
    pthread_mutex_init(&overlay.ippmutex, NULL);
#endif
//    OvlReset();//TODO

    return 0;

err:
    return ret;
}

//----------------------------main init--------------------------
int Open_RkLayers(void)
{
	int ret, i;
//    struct usi_ump_mbs_info uumi;
	OVLDBG("");
	OVLDBG("overlay:%p",&overlay);
	OVLDBG("overlay.fd_USI:%p",&overlay.fd_USI);
	overlay.fd_USI = 0;
    ret = ump_open();
    if (ret != UMP_OK){
    	ERRMSG( "HW:Error open UMP");
    	return ret;
    }

    OVLDBG( "HW:Initialize overlays");

    ret = ovl_init_ovl();
    if(ret){
    	ERRMSG( "HW:Error overlays");
    	goto err;
    }

#ifdef IPP_ENABLE
    OVLDBG( "HW:Initialize IPP");
    if (!LoadKernelModule("rk29-ipp"))
    	OVLDBG( "can't load 'rk29-ipp' kernel module");
    overlay.fd_IPP = ovlInitIPPHW();
    if(overlay.fd_IPP <= 0){
	ERRMSG( "HW:Error IPP");
    }
#endif
#ifdef RGA_ENABLE
    OVLDBG( "HW:Initialize RGA");
    overlay.fd_RGA = ovlInitRGAHW();
    if(overlay.fd_RGA <= 0){
	ERRMSG( "HW:Error RGA");
    }
#endif
    OVLDBG( "HW:Initialize USI");
    if (!LoadKernelModule("rk_ump")){
    	OVLDBG("can't load usi_ump kernel module");
    	ret = -ENODEV;
    	goto err1;
    }
    overlay.fd_USI = ovlInitUSIHW();
    if(overlay.fd_USI <= 0){
    	ERRMSG( "HW:Error USI");
    	ret = -ENODEV;
    	goto err1;
    }

    return 0;

err1:
    for(i=0;i<overlay.OvlsCnt;i++)
        if(overlay.OvlFb[i].fd>0)
        	close(overlay.OvlFb[i].fd);
err:
	ump_close();
    return ret;
}

void Close_RkLayers()
{
    int i;

    OVLDBG("HW:Close");
    if(overlay.fd_USI){

#ifdef IPP_ENABLE
    	if(overlay.fd_IPP > 0)
    		close(overlay.fd_IPP);
#endif
#ifdef RGA_ENABLE
    	if(overlay.fd_RGA > 0)
    		close(overlay.fd_RGA);
#endif
    	for(i = 0;i < overlay.OvlsCnt;i++){
    		OvlFreeLay(i);
    		if(overlay.OvlFb[i].fd > 0){
    			if(i > 0)//except UI
    				OvlEnable( i, 0);
    			close(overlay.OvlFb[i].fd);
    		}
    	}
   		close(overlay.fd_USI);
        ump_close();
    }
}
