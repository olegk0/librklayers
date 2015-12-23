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

#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "rk_layers_priv.h"
#include <pthread.h>

#include "fourcc.h"

OvlHWRec Ovl_priv;

#define MODPROBE_PATH_FILE      "/proc/sys/kernel/modprobe"
#define MAX_PATH                1024

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

	//	tmp = resToHDMImodes(Ovl_priv.cur_var.xres,Ovl_priv.cur_var.yres);
	//	ioctl(Ovl_priv.OvlFb[UserInterfaceFB].fd, FBIOSET_HDMI_MODE, &tmp);//use HDMI scaling

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

/*	fp = fopen(FB_SYS_HDMI"/mode", "w");
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
*/
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
uint32_t ovlToHWRkFormat(OvlLayoutFormatType format)
{
	uint32_t ret;

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
//-----------------------------------------------------------------
OvlLayoutFormatType ovlFromHWRkFormat(uint32_t format)
{
	uint32_t ret;

    switch(format) {
    case RGBA_8888:
		ret = RK_FORMAT_RGBA_8888;
		break;
    case RGBX_8888:
		ret = RK_FORMAT_RGBX_8888;
		break;
    case BGRA_8888:
		ret = RK_FORMAT_BGRA_8888;
		break;
    case RGB_888:
		ret = RK_FORMAT_RGB_888;
		break;
    case RGB_565:
		ret = RK_FORMAT_RGB_565;
		break;
    case RGBA_5551:
		ret = RK_FORMAT_RGBA_5551;
		break;
    case RGBA_4444:
		ret = RK_FORMAT_RGBA_4444;
		break;
    case YCbCr_422_SP:
		ret = RK_FORMAT_YCbCr_422_SP;
		break;
    case YCrCb_NV12_SP:
		ret = RK_FORMAT_YCrCb_NV12_SP;
		break;
    case YCrCb_444:
		ret = RK_FORMAT_YCrCb_444;
		break;
//    case RK_FORMAT_DEFAULT:
    default:
    	ret = 0;
    }

    return ret;
}
//-----------------------------------------------------------------
int OvlGetBppByLay(OvlLayPg layout)
{
	int ret;

	if(!LayValid(layout))
		return -1;

    switch(Ovl_priv.OvlLay[layout].var.nonstd) {
    case RGBA_8888:
    case RGBX_8888:
    case BGRA_8888:
		ret = 4;
		break;
    case RGB_888:
		ret = 3;
		break;
    case RGB_565:
    case RGBA_5551:
    case RGBA_4444:
		ret = 2;
		break;
    case YCbCr_422_SP:
    case YCrCb_NV12_SP:
    case YCrCb_444:
    case YCbCr_422_P:
    case YCrCb_NV12_P:
		ret = 1;
		break;
    default:
    	ret = 0;
    }

    return ret;
}
//--------------------------------------------------------------------------------

OvlMemPgPtr OvlGetBufByLay( OvlLayPg layout, OvlFbBufType BufType)
{
    if(LayValid(layout))
    	return Ovl_priv.OvlLay[layout].FbMemPgs[BufType];
//    	return MBufByLay(layout);
    else
    	return NULL;
}

//--------------------------------------------------------------------------------
uint32_t OvlGetVXresByLay( OvlLayPg layout)
{
	if(LayValid(layout))
		return Ovl_priv.OvlLay[layout].var.xres_virtual;
    else
    	return 0;
}
//--------------------------------------------------------------------------------
uint32_t OvlGetXresByLay( OvlLayPg layout)
{
	if(LayValid(layout))
		return Ovl_priv.OvlLay[layout].var.xres;
    else
    	return 0;
}
//--------------------------------------------------------------------------------
uint32_t OvlGetYresByLay( OvlLayPg layout)
{
	if(LayValid(layout))
		return Ovl_priv.OvlLay[layout].var.yres;
    else
    	return 0;
}
//--------------------------------------------------------------------------------

OvlLayoutFormatType OvlGetModeByLay( OvlLayPg layout)
{
	if(LayValid(layout))
		return ovlFromHWRkFormat(Ovl_priv.OvlLay[layout].var.nonstd);
    else
    	return 0;
}
//--------------------------------------------------------------------------------
uint32_t OvlGetSidByMemPg( OvlMemPgPtr PMemPg)
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
    if(layout >= 0 && layout < Ovl_priv.OvlsCnt){
    	if(Src){
#ifdef IPP_ENABLE
    		Ovl_priv.OvlLay[layout].IPP_req.src0.fmt = IPP_mode;
#endif
#ifdef RGA_ENABLE
    		Ovl_priv.OvlLay[layout].RGA_req.src.format = RGA_mode;
#endif
	}
	else{
#ifdef RGA_ENABLE
		Ovl_priv.OvlLay[layout].RGA_req.dst.format = RGA_mode;
#endif
	}
#ifdef IPP_ENABLE
    	Ovl_priv.OvlLay[layout].IPP_req.dst0.fmt = Ovl_priv.OvlLay[layout].IPP_req.src0.fmt;
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

    switch(Ovl_priv.cur_var.nonstd){
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
    return ioctl(Ovl_priv.OvlFb[UILayer].fd, FBIOSET_COLORKEY, &color);
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
//	memset(PMemPg->fb_mmap,0,PMemPg->buf_size);
	memset_neon (PMemPg->fb_mmap,0,PMemPg->buf_size);
	return 0;
}
//--------------------------------------------------------------------------------
int OvlClrMemPg(OvlMemPgPtr PMemPg)
{
	return ovlclearbuf( ToIntMemPg(PMemPg));
}
//--------------------------------------------------------------------
int ovlUpdatePanelSize(void)
{
	SPanelSize psize;
	int ret;

	ret = ioctl(Ovl_priv.OvlFb[UILayer].fd, RK_FBIOGET_PANEL_SIZE, &psize);
	if(!ret){
		Ovl_priv.Panel_w = psize.size_x;
		Ovl_priv.Panel_h = psize.size_y;
	}

	return ret;
}
//-------------------------------------------------------------------
int OvlSetModeFb( OvlLayPg layout, uint32_t xres, uint32_t yres, OvlLayoutFormatType format)
{
    int ret=0;

    if(LayValidAndNotUI(layout)){
    	memcpy(&Ovl_priv.OvlLay[layout].var, &Ovl_priv.cur_var, sizeof(struct fb_var_screeninfo));
    	if((xres > Ovl_priv.OvlLay[layout].var.xres_virtual)||(yres > Ovl_priv.OvlLay[layout].var.yres_virtual)) return -EINVAL;
//    if((xres > Ovl_priv.cur_var.xres)||(yres > Ovl_priv.cur_var.yres)) return -1;
    	if(format != RK_FORMAT_DEFAULT)
    		Ovl_priv.OvlLay[layout].var.nonstd = ovlToHWRkFormat(format);
    	if(xres>0){
    		Ovl_priv.OvlLay[layout].var.xres = xres;
    	}
    	Ovl_priv.OvlLay[layout].var.xres_virtual = (Ovl_priv.OvlLay[layout].var.xres + 7) & ~7;
    	if(yres>0){
    		Ovl_priv.OvlLay[layout].var.yres = yres;
    	}
    	Ovl_priv.OvlLay[layout].var.yres_virtual = Ovl_priv.OvlLay[layout].var.yres;

    	ret = ioctl(FbByLay(layout)->fd, FBIOPUT_VSCREENINFO, &Ovl_priv.OvlLay[layout].var);
//    	OVLDBG( "PanelW:%d PanelH:%d Xres:%d Yres:%d ret:%d", (Ovl_priv.OvlLay[layout].var.grayscale>>8) & 0xfff, (Ovl_priv.OvlLay[layout].var.grayscale>>20) & 0xfff, Ovl_priv.OvlLay[layout].var.xres, Ovl_priv.OvlLay[layout].var.yres, ret);
/*    	if(ret == 0){
    		ovlSelHwMods( Ovl_priv.OvlLay[layout].var.nonstd, layout, DST_MODE);
//    		Ovl_priv.OvlLay[layout].ResChange = FALSE;
    	}*/
        return ret;
    }
    else
    	return -1;
//    	ovlSelHwMods( Ovl_priv.cur_var.nonstd, layout, DST_MODE);

}
//--------------------------------------------------------------------------------
static int ovlUpdVarOnChangeRes( OvlLayPg layout)
{
	if(LayValid(layout)){
	    ioctl(Ovl_priv.OvlFb[UILayer].fd, FBIOGET_VSCREENINFO, &Ovl_priv.cur_var);
	    Ovl_priv.cur_var.vmode |= FB_VMODE_CONUPDATE;
		Ovl_priv.cur_var.activate = FB_ACTIVATE_NOW;
//		Ovl_priv.cur_var.grayscale = 0;

		ovlUpdatePanelSize();
		Ovl_priv.cur_var.grayscale = (Ovl_priv.Panel_w << 8) | (Ovl_priv.Panel_h << 20);

//    	memcpy(&Ovl_priv.OvlLay[layout].var, &Ovl_priv.cur_var, sizeof(struct fb_var_screeninfo));
//	Ovl_priv.OvlLay[layout].ResChange = FALSE;
		return 0;
    }else
    	return -1;
}
//----------------------------------------------------------------------------------
int OvlSetupFb( OvlLayPg layout, OvlLayoutFormatType SrcFrmt, OvlLayoutFormatType DstFrmt, uint32_t xres, uint32_t yres)
{
    int ret;

    if(LayValidAndNotUI(layout)){

    	ret = ovlUpdVarOnChangeRes( layout);
    	ret |= OvlSetModeFb( layout, xres , yres, DstFrmt);
//    if(!ret){
//    	ovlRgaInitReg( &Ovl_priv.OvlLay[layout].RGA_req, /*SrcYAddr*/0, 0, 0,
//    		FbByLay(layout)->CurMemPg->phy_addr, 0, 0, 0, 0, 0, 0, Ovl_priv.OvlLay[layout].var.xres_virtual/*TODO SRC*/, Ovl_priv.OvlLay[layout].var.xres_virtual, TRUE);
/*    	if(!SrcFrmt)
    		SrcFrmt = Ovl_priv.OvlLay[layout].var.nonstd;
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

    if(layout >= Ovl_priv.OvlsCnt || layout < 0)
	return -1;
    if(SrcPitch){
    	ovlRgaDrwAdd( &Ovl_priv.OvlLay[layout].RGA_req, Drw_w, Drw_h, Drw_x, Drw_y, SrcPitch);
    	return 0;
    }
    return -1;
//    OvlClearBuf( Ovl_priv.OvlFb[Ovl_priv.OvlLay[layout].OvlFb].OvlMemPg);
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

//    	pt.scale_w = (Src_w*Ovl_priv.OvlLay[layout].var.xres)/Drw_w;
//    	pt.scale_h = (Src_h*Ovl_priv.OvlLay[layout].var.yres)/Drw_h;

    	pt.scale_w = (Src_w*Ovl_priv.Panel_w)/Drw_w;
    	pt.scale_h = (Src_h*Ovl_priv.Panel_h)/Drw_h;

    	ret = ioctl(FbByLay(layout)->fd, FBIOSET_DISP_PSET, &pt);
//    OvlClearBuf( Ovl_priv.OvlFb[Ovl_priv.OvlLay[layout].OvlFb].OvlMemPg);
    }else
    	ret = -ENODEV;
    return ret;
}
//----------------------------------------------------------------------------------
int ovlFbLinkMemPg( OvlFbPtr PFb, OvlMemPgPtr MemPg)
{
    int ret;
    uint32_t tmp[2];

    if(!PFb || ToIntFb(PFb)->Type == UIL)
    	return -EINVAL;

	tmp[0] = ToIntMemPg(MemPg)->phy_addr;
	tmp[1] = tmp[0] + ToIntMemPg(MemPg)->offset_uv;
    ret = ioctl(ToIntFb(PFb)->fd, RK_FBIOSET_YUV_ADDR, &tmp);
    if(!ret)
    	ToIntFb(PFb)->CurMemPg = MemPg;
	return ret;
}
//-----------------
int OvlLayerLinkMemPg( OvlLayPg layout, OvlMemPgPtr MemPg)
{
	return ovlFbLinkMemPg( FbByLay(layout), MemPg);
}
//----------------------------------------------------------------------------------
int OvlFlipFb( OvlLayPg layout, OvlFbBufType flip, Bool clrPrev)
{       
    int ret, prev=-1;

    if(LayValidAndNotUI(layout)){

    	switch(flip){
    	case FRONT_FB:
    	case BACK_FB:
    		Ovl_priv.OvlLay[layout].FbBufUsed = flip;
    		break;
    	case NEXT_FB:
    	default:
    		prev = Ovl_priv.OvlLay[layout].FbBufUsed;
    		if(Ovl_priv.OvlLay[layout].FbBufUsed == FRONT_FB)
    			Ovl_priv.OvlLay[layout].FbBufUsed = BACK_FB;
    		else
    			Ovl_priv.OvlLay[layout].FbBufUsed = FRONT_FB;

    	}
    	ret = ovlFbLinkMemPg( FbByLay(layout) , Ovl_priv.OvlLay[layout].FbMemPgs[Ovl_priv.OvlLay[layout].FbBufUsed]);

    	if(clrPrev && prev >= FRONT_FB)
    		ovlclearbuf( Ovl_priv.OvlLay[layout].FbMemPgs[prev]);
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
    	ret = ioctl(FbByLay(layout)->fd, FBIOPUT_VSCREENINFO, &Ovl_priv.cur_var);
//    if(ret == 0 && dev == FBUI) //TODO res change by x func
//	ret =  OvlUpdSavMod();
    }else
    	ret = -ENODEV;

    return ret;
}
//--------------------------------------------------------------
/*OvlLayPg ovlSwapLay( OvlLayPg pg, OvlLayoutType type)
{

    OvlHWPtr	Ovl_priv = pMxv->OvlHW;
    OvlLayPg i,t;

    if(Ovl_priv.OvlLay[pg].ReqType==ANUL){
	for(i=0;i<OVLs;i++){
	    if(!Ovl_priv.OvlLay[i].InUse){
		t = FbByLay(i);
		Ovl_priv.OvlLay[i].OvlFb = FbByLay(pg);
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
    	for(i=0;i < Ovl_priv.OvlsCnt;i++){
    		if(FbByLay(i)->Type == type){
    			if(!Ovl_priv.OvlLay[i].InUse){
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
    	if(lay == Ovl_priv.OvlsCnt)
    		lay = ERRORL;
    	break;
    case ANYL://except UIL
    	for(i=1;i < Ovl_priv.OvlsCnt;i++){
    		if(!Ovl_priv.OvlLay[i].InUse){
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
    		if(!Ovl_priv.OvlLay[lay].FbMemPgs[FRONT_FB]){
        		if(lay == UIL){//User Interface layer
        			Ovl_priv.OvlLay[lay].FbMemPgs[FRONT_FB] = ovlInitMemPgDef();
        			if(Ovl_priv.OvlLay[lay].FbMemPgs[FRONT_FB]){
        				Ovl_priv.OvlLay[lay].FbMemPgs[FRONT_FB]->phy_addr = Ovl_priv.OvlFb[UIL].fix.smem_start;
        				Ovl_priv.OvlLay[lay].FbMemPgs[FRONT_FB]->buf_size = Ovl_priv.OvlFb[UIL].fix.smem_len;
        				Ovl_priv.OvlLay[lay].FbMemPgs[FRONT_FB]->MemPgType = UIFB_MEM;
        			}
        		}else{
        			Ovl_priv.OvlLay[lay].FbMemPgs[FRONT_FB] = OvlAllocMemPg( Ovl_priv.MaxPgSize, 0);//TODO size
        			if(Ovl_priv.OvlLay[lay].FbMemPgs[FRONT_FB])
        				Ovl_priv.OvlLay[lay].FbMemPgs[FRONT_FB]->MemPgType = FB_MEM;
        		}
    		}
    		if(!Ovl_priv.OvlLay[lay].FbMemPgs[FRONT_FB])
    			return ERRORL;

    		OvlFlipFb( lay, FRONT_FB, 0);
//and back fb if needed
    		if(FbBufAlloc > ALC_FRONT_FB && !Ovl_priv.OvlLay[lay].FbMemPgs[BACK_FB]){
    			Ovl_priv.OvlLay[lay].FbMemPgs[BACK_FB] = OvlAllocMemPg( Ovl_priv.MaxPgSize, 0);//TODO size
        		if(Ovl_priv.OvlLay[lay].FbMemPgs[BACK_FB])
        			Ovl_priv.OvlLay[lay].FbMemPgs[BACK_FB]->MemPgType = FB_MEM;
        		else{
        			OvlFreeMemPg( Ovl_priv.OvlLay[lay].FbMemPgs[FRONT_FB]);
        			Ovl_priv.OvlLay[lay].FbMemPgs[FRONT_FB] = NULL;
        			return ERRORL;
        		}
    		}
    	}

		Ovl_priv.OvlLay[lay].InUse = TRUE;
		Ovl_priv.OvlLay[lay].ReqType = type;
    }

    return lay;
}
//------------------------------------------------------------------
void OvlFreeLay( OvlLayPg layout)
{

    if(LayValid(layout)){
    	OvlFreeMemPg(  Ovl_priv.OvlLay[layout].FbMemPgs[FRONT_FB]);
    	Ovl_priv.OvlLay[layout].FbMemPgs[FRONT_FB] = NULL;
    	OvlFreeMemPg(  Ovl_priv.OvlLay[layout].FbMemPgs[BACK_FB]);
    	Ovl_priv.OvlLay[layout].FbMemPgs[BACK_FB] = NULL;
    	MBufByLay(layout) = NULL;
    	OvlEnable( layout, 0);
    	Ovl_priv.OvlLay[layout].InUse = FALSE;
    	Ovl_priv.OvlLay[layout].ReqType = ERRORL;
    }
}
//------------------------------------------------------------------
/*ump_secure_id ovlGetUmpId( OvlMemPg pg)
{

    OvlHWPtr	Ovl_priv = pMxv->OvlHW;
//    ump_secure_id ump_id = UMP_INVALID_SECURE_ID;

    if(pg >0 && pg < Ovl_priv.OvlMemPgs){
    	if(Ovl_priv.OvlMemPg[pg].ump_fb_secure_id == UMP_INVALID_SECURE_ID){
    		Ovl_priv.OvlMemPg[pg].ump_fb_secure_id = pg-1;
    		ioctl(Ovl_priv.OvlFb[MasterOvlFB].fd, GET_UMP_SECURE_ID_BUFn, &Ovl_priv.OvlMemPg[pg].ump_fb_secure_id);
    	}
    	return Ovl_priv.OvlMemPg[pg].ump_fb_secure_id;
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

    memcpy(&Ovl_priv.cur_var, var, sizeof(struct fb_var_screeninfo));
    OvlSetHDMI(var->xres, var->yres);
    Ovl_priv.cur_var.vmode |= FB_VMODE_CONUPDATE;
	Ovl_priv.cur_var.activate = FB_ACTIVATE_NOW;
	Ovl_priv.cur_var.grayscale = 0;
//	Ovl_priv.cur_var.grayscale &= 0xff;
//	Ovl_priv.cur_var.grayscale |= (var->xres << 8) | (var->yres << 20);
// 	Ovl_priv.ResChange = TRUE;
   	OVLDBG("Resolution changed to %dx%d ***", Ovl_priv.cur_var.xres, Ovl_priv.cur_var.yres);

}

//----------------------------------------------------------------------
void set_ovl_param()
{
    int i;

    for(i=0;i<Ovl_priv.OvlsCnt;i++){
    	ioctl(Ovl_priv.OvlFb[i].fd, FBIOGET_FSCREENINFO, &Ovl_priv.OvlFb[i].fix);
    	Ovl_priv.OvlFb[i].CurMemPg = NULL;

    	Ovl_priv.OvlLay[i].OvlFb = &Ovl_priv.OvlFb[i];
    	memcpy(&Ovl_priv.OvlLay[i].var, &Ovl_priv.cur_var, sizeof(struct fb_var_screeninfo));
    	Ovl_priv.OvlLay[i].InUse = FALSE;
    	Ovl_priv.OvlLay[i].ReqType = ERRORL;
//    	Ovl_priv.OvlLay[i].ResChange = FALSE;
    	Ovl_priv.OvlLay[i].FbBufUsed = FRONT_FB;
    	Ovl_priv.OvlLay[i].FbMemPgs[FRONT_FB] = NULL;
    	Ovl_priv.OvlLay[i].FbMemPgs[BACK_FB] = NULL;
    	if(i>0){
        	Ovl_priv.OvlFb[i].Type = SCALEL;//        UIL=0,    SCALEL=1,    NotSCALEL=2,
    		OvlSetModeFb(i,0,0,0);
    		OvlEnable( i, 0);
    	}else
        	Ovl_priv.OvlFb[i].Type = UIL;
    }

}

//------------------------------------------------------------------
int ovl_setup_ovl()
{
    int ret=0;
    Ovl_priv.OvlsCnt = 0;
    Ovl_priv.OvlFb[Ovl1Layer].fd = open(FB_DEV_O1, O_RDONLY); //main Ovl_priv
    if (Ovl_priv.OvlFb[Ovl1Layer].fd < 0){
    	ret = Ovl_priv.OvlFb[Ovl1Layer].fd;
    	goto err;
    }
    Ovl_priv.OvlsCnt++;
    Ovl_priv.OvlFb[UILayer].fd = open(FB_DEV_UI, O_RDWR);
    if (Ovl_priv.OvlFb[UILayer].fd < 0){
    	ret = Ovl_priv.OvlFb[UILayer].fd;
    	goto err1;
    }
    Ovl_priv.OvlsCnt++;
    Ovl_priv.OvlFb[Ovl2Layer].fd = open(FB_DEV_O2, O_RDONLY);
    if (Ovl_priv.OvlFb[Ovl2Layer].fd < 0){
    	ret = Ovl_priv.OvlFb[Ovl2Layer].fd;
    	goto err2;
    }
    Ovl_priv.OvlsCnt++;

    Ovl_priv.MaxPgSize = FB_MAXPGSIZE;

    ioctl(Ovl_priv.OvlFb[UILayer].fd, FBIOGET_VSCREENINFO, &Ovl_priv.cur_var);

//	ioctl(Ovl_priv.fd_o1, FBIOBLANK, FB_BLANK_UNBLANK);
/*	tmp=1;
	ioctl(Ovl_priv.OvlFb[UILayer].fd, RK_FBIOSET_OVERLAY_STATE, &tmp);
        return(TRUE);
    }
*/
    return 0;
//err3:
    close(Ovl_priv.OvlFb[Ovl2Layer].fd);
err2:
    close(Ovl_priv.OvlFb[UILayer].fd);
err1:
    close(Ovl_priv.OvlFb[Ovl1Layer].fd);
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
    pthread_mutex_init(&Ovl_priv.rgamutex, NULL);
#endif
#ifdef IPP_ENABLE
    pthread_mutex_init(&Ovl_priv.ippmutex, NULL);
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
	Ovl_priv.fd_USI = 0;
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
    Ovl_priv.fd_IPP = ovlInitIPPHW();
    if(Ovl_priv.fd_IPP <= 0){
	ERRMSG( "HW:Error IPP");
    }
#endif
#ifdef RGA_ENABLE
    OVLDBG( "HW:Initialize RGA");
    Ovl_priv.fd_RGA = ovlInitRGAHW();
    if(Ovl_priv.fd_RGA <= 0){
	ERRMSG( "HW:Error RGA");
    }
#endif
    OVLDBG( "HW:Initialize USI");
    if (!LoadKernelModule("rk_ump")){
    	OVLDBG("can't load usi_ump kernel module");
    	ret = -ENODEV;
    	goto err1;
    }
    Ovl_priv.fd_USI = ovlInitUSIHW();
    if(Ovl_priv.fd_USI <= 0){
    	ERRMSG( "HW:Error USI");
    	ret = -ENODEV;
    	goto err1;
    }

    return 0;

err1:
    for(i=0;i<Ovl_priv.OvlsCnt;i++)
        if(Ovl_priv.OvlFb[i].fd>0)
        	close(Ovl_priv.OvlFb[i].fd);
err:
	ump_close();
    return ret;
}

void Close_RkLayers()
{
    int i;

    OVLDBG("HW:Close");
    if(Ovl_priv.fd_USI){

#ifdef IPP_ENABLE
    	if(Ovl_priv.fd_IPP > 0)
    		close(Ovl_priv.fd_IPP);
#endif
#ifdef RGA_ENABLE
    	if(Ovl_priv.fd_RGA > 0)
    		close(Ovl_priv.fd_RGA);
#endif
    	for(i = 0;i < Ovl_priv.OvlsCnt;i++){
    		OvlFreeLay(i);
    		if(Ovl_priv.OvlFb[i].fd > 0){
    			if(i > 0)//except UI
    				OvlEnable( i, 0);
    			close(Ovl_priv.OvlFb[i].fd);
    		}
    	}
   		close(Ovl_priv.fd_USI);
        ump_close();
    }
}
