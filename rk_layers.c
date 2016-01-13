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

#define MODPROBE_PATH_FILE      "/proc/sys/kernel/modprobe"
#define MAX_PATH                1024

OvlHWPtr pOvl_priv;

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

	//	tmp = resToHDMImodes(pOvl_priv->cur_var.xres,pOvl_priv->cur_var.yres);
	//	ioctl(pOvl_priv->OvlFb[UserInterfaceFB].fd, FBIOSET_HDMI_MODE, &tmp);//use HDMI scaling

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

    switch(pOvl_priv->OvlLay[layout].var.nonstd) {
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
    	return pOvl_priv->OvlLay[layout].FbMemPgs[BufType];
//    	return MBufByLay(layout);
    else
    	return NULL;
}

//--------------------------------------------------------------------------------
uint32_t OvlGetVXresByLay( OvlLayPg layout)
{
	if(LayValid(layout))
		return pOvl_priv->OvlLay[layout].var.xres_virtual;
    else
    	return 0;
}
//--------------------------------------------------------------------------------
uint32_t OvlGetXresByLay( OvlLayPg layout)
{
	if(LayValid(layout))
		return pOvl_priv->OvlLay[layout].var.xres;
    else
    	return 0;
}
//--------------------------------------------------------------------------------
uint32_t OvlGetYresByLay( OvlLayPg layout)
{
	if(LayValid(layout))
		return pOvl_priv->OvlLay[layout].var.yres;
    else
    	return 0;
}
//--------------------------------------------------------------------------------

OvlLayoutFormatType OvlGetModeByLay( OvlLayPg layout)
{
	if(LayValid(layout))
		return ovlFromHWRkFormat(pOvl_priv->OvlLay[layout].var.nonstd);
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
    if(layout >= 0 && layout < pOvl_priv->OvlsCnt){
    	if(Src){
#ifdef IPP_ENABLE
    		pOvl_priv->OvlLay[layout].IPP_req.src0.fmt = IPP_mode;
#endif
#ifdef RGA_ENABLE
    		pOvl_priv->OvlLay[layout].RGA_req.src.format = RGA_mode;
#endif
	}
	else{
#ifdef RGA_ENABLE
		pOvl_priv->OvlLay[layout].RGA_req.dst.format = RGA_mode;
#endif
	}
#ifdef IPP_ENABLE
    	pOvl_priv->OvlLay[layout].IPP_req.dst0.fmt = pOvl_priv->OvlLay[layout].IPP_req.src0.fmt;
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

    switch(pOvl_priv->cur_var.nonstd){
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
	struct color_key_cfg colorkeys;
//    return ioctl(pOvl_priv->OvlFb[UILayer].fd, FBIOSET_COLORKEY, &color);
	colorkeys.win0_color_key_cfg = color;
	colorkeys.win1_color_key_cfg = color;
	colorkeys.win2_color_key_cfg = color;
	return ioctl(pOvl_priv->OvlFb[UILayer].fd, RK_FBIOPUT_COLOR_KEY_CFG, &colorkeys);

}
//------------------------------------------------------------------
int OvlWaitSync( OvlLayPg layout)
{
    uint32_t tmp=0;

    if(LayValid(layout))
    	return ioctl(FbByLay(layout)->fd, FBIO_WAITFORVSYNC, &tmp);
    else
    	return -1;
}
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

	ret = ioctl(pOvl_priv->OvlFb[UILayer].fd, RK_FBIOGET_PANEL_SIZE, &psize);
	if(!ret){
		pOvl_priv->Panel_w = psize.size_x;
		pOvl_priv->Panel_h = psize.size_y;
	}

	return ret;
}
//-------------------------------------------------------------------
int OvlSetModeFb( OvlLayPg layout, uint32_t xres, uint32_t yres, OvlLayoutFormatType format)
{
    int ret=0;

    if(LayValidAndNotUI(layout)){
//    if((xres > pOvl_priv->cur_var.xres)||(yres > pOvl_priv->cur_var.yres)) return -1;
    	if(format != RK_FORMAT_DEFAULT)
    		pOvl_priv->OvlLay[layout].var.nonstd = (pOvl_priv->OvlLay[layout].var.nonstd & ~0xff) | ovlToHWRkFormat(format);
    	if(xres>0){
    		pOvl_priv->OvlLay[layout].var.xres = xres;
    	}
    	pOvl_priv->OvlLay[layout].var.xres_virtual = (pOvl_priv->OvlLay[layout].var.xres + 7) & ~7;
    	if(yres>0){
    		pOvl_priv->OvlLay[layout].var.yres = yres;
    	}
    	pOvl_priv->OvlLay[layout].var.yres_virtual = pOvl_priv->OvlLay[layout].var.yres;

    	ret = ioctl(FbByLay(layout)->fd, FBIOPUT_VSCREENINFO, &pOvl_priv->OvlLay[layout].var);
//    	OVLDBG( "PanelW:%d PanelH:%d Xres:%d Yres:%d ret:%d", (pOvl_priv->OvlLay[layout].var.grayscale>>8) & 0xfff, (pOvl_priv->OvlLay[layout].var.grayscale>>20) & 0xfff, pOvl_priv->OvlLay[layout].var.xres, pOvl_priv->OvlLay[layout].var.yres, ret);
/*    	if(ret == 0){
    		ovlSelHwMods( pOvl_priv->OvlLay[layout].var.nonstd, layout, DST_MODE);
//    		pOvl_priv->OvlLay[layout].ResChange = FALSE;
    	}*/
        return ret;
    }
    else
    	return -1;
//    	ovlSelHwMods( pOvl_priv->cur_var.nonstd, layout, DST_MODE);

}
//--------------------------------------------------------------------------------
static int ovlUpdVarOnChangeRes( OvlLayPg layout)
{
	if(LayValid(layout)){
	    ioctl(pOvl_priv->OvlFb[UILayer].fd, FBIOGET_VSCREENINFO, &pOvl_priv->cur_var);
	    pOvl_priv->cur_var.vmode |= FB_VMODE_CONUPDATE;
		pOvl_priv->cur_var.activate = FB_ACTIVATE_NOW;
		pOvl_priv->cur_var.grayscale = 0;

//		ovlUpdatePanelSize();
//		pOvl_priv->cur_var.grayscale = (pOvl_priv->Panel_w << 8) | (pOvl_priv->Panel_h << 20);

    	memcpy(&pOvl_priv->OvlLay[layout].var, &pOvl_priv->cur_var, sizeof(struct fb_var_screeninfo));
//	pOvl_priv->OvlLay[layout].ResChange = FALSE;
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
//    	ovlRgaInitReg( &pOvl_priv->OvlLay[layout].RGA_req, /*SrcYAddr*/0, 0, 0,
//    		FbByLay(layout)->CurMemPg->phy_addr, 0, 0, 0, 0, 0, 0, pOvl_priv->OvlLay[layout].var.xres_virtual/*TODO SRC*/, pOvl_priv->OvlLay[layout].var.xres_virtual, TRUE);
/*    	if(!SrcFrmt)
    		SrcFrmt = pOvl_priv->OvlLay[layout].var.nonstd;
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

    if(layout >= pOvl_priv->OvlsCnt || layout < 0)
	return -1;
    if(SrcPitch){
    	ovlRgaDrwAdd( &pOvl_priv->OvlLay[layout].RGA_req, Drw_w, Drw_h, Drw_x, Drw_y, SrcPitch);
    	return 0;
    }
    return -1;
//    OvlClearBuf( pOvl_priv->OvlFb[pOvl_priv->OvlLay[layout].OvlFb].OvlMemPg);
}*/
//--------------------------------------------------------------------------------
int OvlSetupDrw( OvlLayPg layout, int Drw_x, int Drw_y, int Drw_w, int Drw_h)
{

    int ret=0;

    if(LayValidAndNotUI(layout)){
    	pOvl_priv->OvlLay[layout].var.grayscale = (Drw_w << 8) | (Drw_h << 20);
    	pOvl_priv->OvlLay[layout].var.nonstd &= 0xff;
    	pOvl_priv->OvlLay[layout].var.nonstd |= (Drw_x<<8) + (Drw_y<<20);
    	OvlSetModeFb(layout, 0, 0, RK_FORMAT_DEFAULT);
//    OvlClearBuf( pOvl_priv->OvlFb[pOvl_priv->OvlLay[layout].OvlFb].OvlMemPg);
    }else
    	ret = -ENODEV;
    return ret;
}
//----------------------------------------------------------------------------------
int OvlLayerLinkMemPg( OvlLayPg layout, OvlMemPgPtr MemPg)
{
    int ret;
    uint32_t tmp[2];
    ovlFbPtr PFb;

    if(LayValidAndNotUI(layout)){
    	PFb = FbByLay(layout);
		tmp[0] = ToIntMemPg(MemPg)->phy_addr;
    	tmp[1] = tmp[0] + ToIntMemPg(MemPg)->offset_uv;
    	ret = ioctl(ToIntFb(PFb)->fd, RK_FBIOSET_YUV_ADDR, &tmp);
    	if(!ret){
    		PFb->CurMemPg = MemPg;
//    		OvlWaitSync(layout);
    	}
    	return ret;
    }else
    	return -EINVAL;
}
//----------------------------------------------------------------------------------
int OvlFlipFb( OvlLayPg layout, OvlFbBufType flip, Bool clrPrev)
{       
    int ret, prev=-1;

    if(LayValidAndNotUI(layout)){
    	switch(flip){
    	case FRONT_FB:
    	case BACK_FB:
    		pOvl_priv->OvlLay[layout].FbBufUsed = flip;
    		break;
    	case NEXT_FB:
    	default:
    		prev = pOvl_priv->OvlLay[layout].FbBufUsed;
    		if(pOvl_priv->OvlLay[layout].FbBufUsed == FRONT_FB)
    			pOvl_priv->OvlLay[layout].FbBufUsed = BACK_FB;
    		else
    			pOvl_priv->OvlLay[layout].FbBufUsed = FRONT_FB;

    	}

    	ret = OvlLayerLinkMemPg( layout, pOvl_priv->OvlLay[layout].FbMemPgs[pOvl_priv->OvlLay[layout].FbBufUsed]);

    	if(clrPrev && prev >= FRONT_FB)
    		ovlclearbuf( pOvl_priv->OvlLay[layout].FbMemPgs[prev]);
    }else
    	ret = -ENODEV;

    return ret;
}
//---------------------------------------------------------------------
int OvlEnable( OvlLayPg layout, int enable, int vsync_en)
{
	OVLDBG("layout:%d en:%d vsync:%d", layout, enable, vsync_en);
    if(LayValidAndNotUI(layout)){
		ioctl(FbByLay(layout)->fd, RK_FBIOSET_VSYNC_ENABLE, &vsync_en);
    	return ioctl(FbByLay(layout)->fd, RK_FBIOSET_ENABLE, &enable);
    }
    else
    	return -ENODEV;
}
//---------------------------------------------------------------------
int ovlIsUsedNM( OvlLayPg layout)
{
	int used;

//for extern func    if(LayValidAndNotUI(layout))
    used = pOvl_priv->SHM_mem->lay_used[layout];

    OVLDBG("layout:%d used:%d", layout, used);
    if(!used)
    	OvlEnable(layout, 0, 1); // workaround layer activation when opening

    return used;

}
//---------------------------------------------------------------------
int ovlIsUsed( OvlLayPg layout)
{
	int used;

//for extern func    if(LayValidAndNotUI(layout))
	sem_wait(pOvl_priv->SEM_t);
    used = ovlIsUsedNM(layout);
    sem_post(pOvl_priv->SEM_t);
    return used;

}
//---------------------------------------------------------------------
int ovlIsUsedAlloc( OvlLayPg layout)
{
	int used;

//for extern func    if(LayValidAndNotUI(layout))
	sem_wait(pOvl_priv->SEM_t);
    used = pOvl_priv->SHM_mem->lay_used[layout];

    OVLDBG("layout:%d used:%d", layout, used);
    if(!used){
//    	OvlEnable(layout, 0, 1); // workaround layer activation when opening
    	pOvl_priv->SHM_mem->lay_used[layout] = 1;
    }

    sem_post(pOvl_priv->SEM_t);
    return used;

}
//---------------------------------------------------------------------
void ovlSetUsedNM( OvlLayPg layout, int used)
{
    pOvl_priv->SHM_mem->lay_used[layout] = used;
}
//---------------------------------------------------------------------
void ovlSetUsed( OvlLayPg layout, int used)
{
	sem_wait(pOvl_priv->SEM_t);
	ovlSetUsedNM( layout, used);
    sem_post(pOvl_priv->SEM_t);
}
//---------------------------------------------------------------------
int OvlResetFB( OvlLayPg layout)
{       
    int ret;

    if(LayValid(layout)){
	//    OvlClearBuf( 1);
    	ret = ioctl(FbByLay(layout)->fd, FBIOPUT_VSCREENINFO, &pOvl_priv->cur_var);
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

    if(pOvl_priv->OvlLay[pg].ReqType==ANUL){
	for(i=0;i<OVLs;i++){
	    if(!pOvl_priv->OvlLay[i].InUse){
		t = FbByLay(i);
		pOvl_priv->OvlLay[i].OvlFb = FbByLay(pg);
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
    OvlLayPg lay=pOvl_priv->OvlsCnt;

    switch(type){
    case UIL:
    case SCALEL:
//    case NOT_SCALEL:
    	for(lay=0;lay < pOvl_priv->OvlsCnt;lay++){
    		if(FbByLay(lay)->Type == type){
    			if(!pOvl_priv->OvlLay[lay].InUse && !ovlIsUsedAlloc(lay)){
    				break;
    			}
/*    			else{
//		    t = ovlSwapLay( i, type);
//		    if(t==ERRORL)
    				lay = ERRORL;
    			}*/
    		}
    	}
    	break;
    case ANYL://except UIL
    	for(lay=0;lay < pOvl_priv->OvlsCnt;lay++){
    		if(FbByLay(lay)->Type != UIL)
    			if(!pOvl_priv->OvlLay[lay].InUse && !ovlIsUsedAlloc(lay)){
    			}
    	}
    	break;
    default:
    	return ERRORL;
    }

    if(lay < pOvl_priv->OvlsCnt){
    	if(FbBufAlloc > ALC_NONE_FB){
//front fb first by def
    		if(!pOvl_priv->OvlLay[lay].FbMemPgs[FRONT_FB]){
        		if(lay == UIL){//User Interface layer
        			pOvl_priv->OvlLay[lay].FbMemPgs[FRONT_FB] = ovlInitMemPgDef();
        			if(pOvl_priv->OvlLay[lay].FbMemPgs[FRONT_FB]){
        				pOvl_priv->OvlLay[lay].FbMemPgs[FRONT_FB]->phy_addr = pOvl_priv->OvlFb[UIL].fix.smem_start;
        				pOvl_priv->OvlLay[lay].FbMemPgs[FRONT_FB]->buf_size = pOvl_priv->OvlFb[UIL].fix.smem_len;
        				pOvl_priv->OvlLay[lay].FbMemPgs[FRONT_FB]->MemPgType = UIFB_MEM;
        			}
        		}else{
        			pOvl_priv->OvlLay[lay].FbMemPgs[FRONT_FB] = OvlAllocMemPg( pOvl_priv->MaxPgSize, 0);//TODO size
        			if(pOvl_priv->OvlLay[lay].FbMemPgs[FRONT_FB])
        				pOvl_priv->OvlLay[lay].FbMemPgs[FRONT_FB]->MemPgType = FB_MEM;
        		}
    		}
    		if(!pOvl_priv->OvlLay[lay].FbMemPgs[FRONT_FB])
    			goto err;

    		OvlFlipFb( lay, FRONT_FB, 0);
//and back fb if needed
    		if(FbBufAlloc > ALC_FRONT_FB && !pOvl_priv->OvlLay[lay].FbMemPgs[BACK_FB]){
    			pOvl_priv->OvlLay[lay].FbMemPgs[BACK_FB] = OvlAllocMemPg( pOvl_priv->MaxPgSize, 0);//TODO size
        		if(pOvl_priv->OvlLay[lay].FbMemPgs[BACK_FB])
        			pOvl_priv->OvlLay[lay].FbMemPgs[BACK_FB]->MemPgType = FB_MEM;
        		else{
        			OvlFreeMemPg( pOvl_priv->OvlLay[lay].FbMemPgs[FRONT_FB]);
        			pOvl_priv->OvlLay[lay].FbMemPgs[FRONT_FB] = NULL;
        			goto err;
        		}
    		}
    	}
		pOvl_priv->OvlLay[lay].InUse = TRUE;
		pOvl_priv->OvlLay[lay].ReqType = type;
		return lay;
    }else
    	goto err1;
err:
	ovlSetUsed( lay, 0);
err1:
	return ERRORL;

}
//------------------------------------------------------------------
void OvlFreeLay( OvlLayPg layout)
{

    if(LayValid(layout)){
    	if(pOvl_priv->OvlLay[layout].InUse){
    		OvlFreeMemPg(  pOvl_priv->OvlLay[layout].FbMemPgs[FRONT_FB]);
    		pOvl_priv->OvlLay[layout].FbMemPgs[FRONT_FB] = NULL;
    		OvlFreeMemPg(  pOvl_priv->OvlLay[layout].FbMemPgs[BACK_FB]);
    		pOvl_priv->OvlLay[layout].FbMemPgs[BACK_FB] = NULL;
    		MBufByLay(layout) = NULL;
    		OvlEnable( layout, 0, 1);
    		pOvl_priv->OvlLay[layout].InUse = FALSE;
    		pOvl_priv->OvlLay[layout].ReqType = ERRORL;
    		ovlSetUsed( layout, 0);
    	}
    }
}
//------------------------------------------------------------------
/*ump_secure_id ovlGetUmpId( OvlMemPg pg)
{

    OvlHWPtr	Ovl_priv = pMxv->OvlHW;
//    ump_secure_id ump_id = UMP_INVALID_SECURE_ID;

    if(pg >0 && pg < pOvl_priv->OvlMemPgs){
    	if(pOvl_priv->OvlMemPg[pg].ump_fb_secure_id == UMP_INVALID_SECURE_ID){
    		pOvl_priv->OvlMemPg[pg].ump_fb_secure_id = pg-1;
    		ioctl(pOvl_priv->OvlFb[MasterOvlFB].fd, GET_UMP_SECURE_ID_BUFn, &pOvl_priv->OvlMemPg[pg].ump_fb_secure_id);
    	}
    	return pOvl_priv->OvlMemPg[pg].ump_fb_secure_id;
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

    memcpy(&pOvl_priv->cur_var, var, sizeof(struct fb_var_screeninfo));
    OvlSetHDMI(var->xres, var->yres);
    pOvl_priv->cur_var.vmode |= FB_VMODE_CONUPDATE;
	pOvl_priv->cur_var.activate = FB_ACTIVATE_NOW;
	pOvl_priv->cur_var.grayscale = 0;
//	pOvl_priv->cur_var.grayscale &= 0xff;
//	pOvl_priv->cur_var.grayscale |= (var->xres << 8) | (var->yres << 20);
// 	pOvl_priv->ResChange = TRUE;
   	OVLDBG("Resolution changed to %dx%d ***", pOvl_priv->cur_var.xres, pOvl_priv->cur_var.yres);

}

//----------------------------------------------------------------------
void set_ovl_param(Bool MasterMode)
{
    int i;

    for(i=0;i<pOvl_priv->OvlsCnt;i++){
    	ioctl(pOvl_priv->OvlFb[i].fd, FBIOGET_FSCREENINFO, &pOvl_priv->OvlFb[i].fix);
    	pOvl_priv->OvlFb[i].CurMemPg = NULL;

    	pOvl_priv->OvlLay[i].OvlFb = &pOvl_priv->OvlFb[i];
    	memcpy(&pOvl_priv->OvlLay[i].var, &pOvl_priv->cur_var, sizeof(struct fb_var_screeninfo));
    	pOvl_priv->OvlLay[i].InUse = FALSE;
    	pOvl_priv->OvlLay[i].ReqType = ERRORL;
//    	pOvl_priv->OvlLay[i].ResChange = FALSE;
    	pOvl_priv->OvlLay[i].FbBufUsed = FRONT_FB;
    	pOvl_priv->OvlLay[i].FbMemPgs[FRONT_FB] = NULL;
    	pOvl_priv->OvlLay[i].FbMemPgs[BACK_FB] = NULL;
    	if(MasterMode){
    		OvlEnable( i, 0, 1);
    		ovlSetUsedNM( i, 0);
    	}else{
    		ovlIsUsedNM(i);
    	}
    	if(i>0){
        	pOvl_priv->OvlFb[i].Type = SCALEL;//        UIL=0,    SCALEL=1,    NotSCALEL=2, TODO HWRGALayer
//    		OvlSetModeFb(i,0,0,0);
    	}else
        	pOvl_priv->OvlFb[i].Type = UIL;
    }

}

//------------------------------------------------------------------
int ovl_setup_ovl()
{
    int ret=0;
    pOvl_priv->OvlsCnt = 0;
    pOvl_priv->OvlFb[Ovl1Layer].fd = open(FB_DEV_O1, O_RDONLY); //main Ovl_priv
    if (pOvl_priv->OvlFb[Ovl1Layer].fd < 0){
    	ret = pOvl_priv->OvlFb[Ovl1Layer].fd;
    	goto err;
    }
    pOvl_priv->OvlsCnt++;
    pOvl_priv->OvlFb[UILayer].fd = open(FB_DEV_UI, O_RDWR);
    if (pOvl_priv->OvlFb[UILayer].fd < 0){
    	ret = pOvl_priv->OvlFb[UILayer].fd;
    	goto err1;
    }
    pOvl_priv->OvlsCnt++;
    pOvl_priv->OvlFb[Ovl2Layer].fd = open(FB_DEV_O2, O_RDONLY);
    if (pOvl_priv->OvlFb[Ovl2Layer].fd < 0){
    	ret = pOvl_priv->OvlFb[Ovl2Layer].fd;
    	goto err2;
    }
    pOvl_priv->OvlsCnt++;

    OVLDBG("OvlsCnt:%d", pOvl_priv->OvlsCnt);

    pOvl_priv->MaxPgSize = FB_MAXPGSIZE;

    ioctl(pOvl_priv->OvlFb[UILayer].fd, FBIOGET_VSCREENINFO, &pOvl_priv->cur_var);

//	ioctl(pOvl_priv->fd_o1, FBIOBLANK, FB_BLANK_UNBLANK);
/*	tmp=1;
	ioctl(pOvl_priv->OvlFb[UILayer].fd, RK_FBIOSET_OVERLAY_STATE, &tmp);
        return(TRUE);
    }
*/
    return 0;

    close(pOvl_priv->OvlFb[Ovl2Layer].fd);
    pOvl_priv->OvlFb[Ovl2Layer].fd = 0;
err2:
    close(pOvl_priv->OvlFb[UILayer].fd);
    pOvl_priv->OvlFb[UILayer].fd = 0;
err1:
    close(pOvl_priv->OvlFb[Ovl1Layer].fd);
    pOvl_priv->OvlFb[Ovl1Layer].fd = 0;
err:
    return ret;
}
//-------------------------------------------------------
int ovl_init_ovl()
{
    int ret, shme=0, i;

    ret = ovl_setup_ovl();
    if(ret) goto err;

    sem_wait(pOvl_priv->SEM_t);

    mode_t old_umask = umask(0);
    if((pOvl_priv->SHM_fd = shm_open( SHM_NAME, O_CREAT | O_RDWR |  O_EXCL, 0666)) < 0 ) {
    	OVLDBG("SHM exist");
    	shme = 1;
    	if((pOvl_priv->SHM_fd = shm_open( SHM_NAME, O_RDWR, 0666)) < 0 ) {
    		ERRMSG("Cannot open SHM");
    		ret = -1;
    		umask(old_umask);
    		goto err_mu;
    	}
    }
    umask(old_umask);
    if(!shme){
    	if (ftruncate(pOvl_priv->SHM_fd, sizeof(SHMdt)+1) < 0 ) {
    		ERRMSG("ftruncate SHM");
            ret = -1;
            goto err_mu1;
        }
    }

    pOvl_priv->SHM_mem = mmap(0, sizeof(SHMdt), PROT_WRITE | PROT_READ, MAP_SHARED, pOvl_priv->SHM_fd, 0);
    if ( pOvl_priv->SHM_mem == MAP_FAILED ) {
    	ERRMSG("mmap SHM");
		ret = -1;
		pOvl_priv->SHM_mem = NULL;
		goto err_mu1;
    }

    if(shme)//not first run
    	if(!pOvl_priv->SHM_mem->ucount) // == 0
    		shme = 0;

    OVLDBG("SHM ucount:%d shme:%d", pOvl_priv->SHM_mem->ucount, shme);
    if(shme){
    	pOvl_priv->SHM_mem->ucount++;
    }else{//init
    	pOvl_priv->SHM_mem->ucount = 1;
    	for(i=0;i<MAX_OVERLAYs;i++)
    		pOvl_priv->SHM_mem->lay_used[i] = 0;
    }
    shme = !shme;
    set_ovl_param(shme);

    sem_post(pOvl_priv->SEM_t);


#ifdef RGA_ENABLE
    pthread_mutex_init(&pOvl_priv->rgamutex, NULL);
#endif
#ifdef IPP_ENABLE
    pthread_mutex_init(&pOvl_priv->ippmutex, NULL);
#endif

    return shme;
err_mu1:
	close(pOvl_priv->SHM_fd);
	shm_unlink(SHM_NAME);
err_mu:
	sem_post(pOvl_priv->SEM_t);
err:
    return ret;
}

//----------------------------main init--------------------------
int Open_RkLayers(void)
{
	int ret, i, tmp=1;

	OVLDBG("");

	pOvl_priv = calloc(1, sizeof(OvlHWRec));
	if(!pOvl_priv){
		ERRMSG("Cannot alocate mem for ovl rec");
		return -ENOMEM;
	}

    if (ump_open() != UMP_OK){
    	ERRMSG( "HW:Error open UMP");
    	MFREE(pOvl_priv);
    	return -ENODEV;
    }

    mode_t old_umask = umask(0);
//	OVLDBG( "Semaphore %s try open",SEM_NAME);
    if ( (pOvl_priv->SEM_t = sem_open(SEM_NAME, O_CREAT |  O_EXCL, 0666, 1)) == SEM_FAILED ) {
    	OVLDBG( "Semaphore exist:%d",errno);
//    	tmp=0;
/*    	if(sem_unlink(SEM_NAME) < 0){
    		OVLDBG( "Semaphore is not deleted");
    	}else{
    		OVLDBG( "Semaphore deleted");
    	}
    	if ( (pOvl_priv->SEM_t = sem_open(SEM_NAME, O_CREAT, 0666, 1)) == SEM_FAILED ) {*/
    	if ( (pOvl_priv->SEM_t = sem_open(SEM_NAME, 0)) == SEM_FAILED ) {
    		ERRMSG("Error sem_open:%d",errno);
    		umask(old_umask);
    		goto err;
    	}else{
    		OVLDBG( "Semaphore re-open");
    	}
    }
    umask(old_umask);
    OVLDBG( "HW:Initialize overlays");

    ret = ovl_init_ovl();
    if(ret< 0){
    	ERRMSG( "HW:Error overlays");
    	goto err;
    }

    tmp = ret;

#ifdef IPP_ENABLE
    OVLDBG( "HW:Initialize IPP");
    if (!LoadKernelModule("rk29-ipp"))
    	OVLDBG( "can't load 'rk29-ipp' kernel module");
    pOvl_priv->fd_IPP = ovlInitIPPHW();
    if(pOvl_priv->fd_IPP <= 0){
	ERRMSG( "HW:Error IPP");
    }
#endif
#ifdef RGA_ENABLE
    OVLDBG( "HW:Initialize RGA");
    pOvl_priv->fd_RGA = ovlInitRGAHW();
    if(pOvl_priv->fd_RGA <= 0){
	ERRMSG( "HW:Error RGA");
    }
#endif

    OVLDBG( "HW:Initialize USI");
    if(tmp)
    if (!LoadKernelModule("rk_ump")){
    	ERRMSG("can't load usi_ump kernel module");
    	ret = -ENODEV;
    	goto err;
    }

    pOvl_priv->fd_USI = ovlInitUSIHW();
    if(pOvl_priv->fd_USI <= 0){
    	ERRMSG( "HW:Error USI");
    	ret = -ENODEV;
    	goto err;
    }

    return 0;
err:
	Close_RkLayers();
    return ret;
}

void Close_RkLayers()
{
    int i;

    OVLDBG("HW:Close");
    if(pOvl_priv){

#ifdef IPP_ENABLE
    	if(pOvl_priv->fd_IPP > 0)
    		close(pOvl_priv->fd_IPP);
#endif
#ifdef RGA_ENABLE
    	if(pOvl_priv->fd_RGA > 0)
    		close(pOvl_priv->fd_RGA);
#endif

    	for(i = 0;i < pOvl_priv->OvlsCnt;i++){
    		OvlFreeLay(i);
    		if(pOvl_priv->OvlFb[i].fd > 0)
    			close(pOvl_priv->OvlFb[i].fd);
    	}

    	if(pOvl_priv->fd_USI > 0)
    		close(pOvl_priv->fd_USI);

    	ump_close();

    	if(pOvl_priv->SEM_t > 0){
    		i=1;
    		sem_wait(pOvl_priv->SEM_t);
    		if(pOvl_priv->SHM_fd > 0){
    			if(pOvl_priv->SHM_mem){
    				pOvl_priv->SHM_mem->ucount--;
    				i = pOvl_priv->SHM_mem->ucount;
    				munmap(pOvl_priv->SHM_mem, sizeof(SHMdt));
    			}
    			close(pOvl_priv->SHM_fd);
    			if(!i)
    				shm_unlink(SHM_NAME);
    		}
    		sem_post(pOvl_priv->SEM_t);

    		sem_close(pOvl_priv->SEM_t);
    	}

/*    	if(sem_unlink(SEM_NAME) < 0){
    		OVLDBG("Error: sem_unlink");
    	}else{
    		OVLDBG("sem deleted");
    	}
*/
        MFREE(pOvl_priv);
    }
}
