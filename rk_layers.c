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

#include <sys/types.h>
#include <sys/wait.h>

#include "rk_layers_priv.h"

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
		ERRMSG("Does not open "FB_SYS_HDMI"/mode");

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
    case RKL_FORMAT_RGBA_8888:
		ret = RGBA_8888;
		break;
    case RKL_FORMAT_RGBX_8888:
		ret = RGBX_8888;
		break;
    case RKL_FORMAT_BGRA_8888:
		ret = BGRA_8888;
		break;
    case RKL_FORMAT_RGB_888:
		ret = RGB_888;
		break;
    case RKL_FORMAT_RGB_565:
		ret = RGB_565;
		break;
    case RKL_FORMAT_RGBA_5551:
		ret = RGBA_5551;
		break;
    case RKL_FORMAT_RGBA_4444:
		ret = RGBA_4444;
		break;
    case RKL_FORMAT_YCbCr_422_SP:
		ret = YCbCr_422_SP;
		break;
    case RKL_FORMAT_YCrCb_NV12_SP:
		ret = YCrCb_NV12_SP;
		break;
    case RKL_FORMAT_YCrCb_444:
		ret = YCrCb_444;
		break;
    case RKL_FORMAT_DEFAULT:
    default:
    	ret = 0;
    }

    return ret;
}
//-----------------------------------------------------------------
OvlLayoutFormatType ovlFromHWRkFormat(uint32_t format)
{
	uint32_t ret;

    switch(format & 0xff) {
    case RGBA_8888:
		ret = RKL_FORMAT_RGBA_8888;
		break;
    case RGBX_8888:
		ret = RKL_FORMAT_RGBX_8888;
		break;
    case BGRA_8888:
		ret = RKL_FORMAT_BGRA_8888;
		break;
    case RGB_888:
		ret = RKL_FORMAT_RGB_888;
		break;
    case RGB_565:
		ret = RKL_FORMAT_RGB_565;
		break;
    case RGBA_5551:
		ret = RKL_FORMAT_RGBA_5551;
		break;
    case RGBA_4444:
		ret = RKL_FORMAT_RGBA_4444;
		break;
    case YCbCr_422_SP:
		ret = RKL_FORMAT_YCbCr_422_SP;
		break;
    case YCrCb_NV12_SP:
		ret = RKL_FORMAT_YCrCb_NV12_SP;
		break;
    case YCrCb_444:
		ret = RKL_FORMAT_YCrCb_444;
		break;
//    case RKL_FORMAT_DEFAULT:
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

    switch(pOvl_priv->OvlLay[layout].var.nonstd & 0xff) {
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

    switch(pOvl_priv->cur_var.nonstd & 0xff){
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
	memset(PMemPg->fb_mmap,0,PMemPg->buf_size);
//	memset_neon (PMemPg->fb_mmap,0,PMemPg->buf_size);
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
uint32_t OvlVresByXres(uint32_t xres)
{
	 return (xres+ 7) & ~7;//round up
}
//-------------------------------------------------------------------
static int ovlSetModeFb( OvlLayPg layout, uint32_t xres, uint32_t yres, OvlLayoutFormatType format)
{
    int ret=0;

//    if(LayValidAndNotUI(layout)){
//    if((xres > pOvl_priv->cur_var.xres)||(yres > pOvl_priv->cur_var.yres)) return -1;
    	if(format != RKL_FORMAT_DEFAULT)
    		pOvl_priv->OvlLay[layout].var.nonstd = (pOvl_priv->OvlLay[layout].var.nonstd & ~0xff) | ovlToHWRkFormat(format);
    	if(xres>0){
    		pOvl_priv->OvlLay[layout].var.xres = xres;
    	}
    	pOvl_priv->OvlLay[layout].var.xres_virtual = OvlVresByXres(pOvl_priv->OvlLay[layout].var.xres);
    	if(yres>0){
    		pOvl_priv->OvlLay[layout].var.yres = yres;
    	}
    	pOvl_priv->OvlLay[layout].var.yres_virtual = pOvl_priv->OvlLay[layout].var.yres;

    	if(LayHWValid(layout))
    		ret = ioctl(FbByLay(layout)->fd, FBIOPUT_VSCREENINFO, &pOvl_priv->OvlLay[layout].var);
//    	OVLDBG( "PanelW:%d PanelH:%d Xres:%d Yres:%d ret:%d", (pOvl_priv->OvlLay[layout].var.grayscale>>8) & 0xfff, (pOvl_priv->OvlLay[layout].var.grayscale>>20) & 0xfff, pOvl_priv->OvlLay[layout].var.xres, pOvl_priv->OvlLay[layout].var.yres, ret);

        return ret;

}
//--------------------------------------------------------------------------------
static int ovlUpdVarOnChangeRes( OvlLayPg layout)
{
	int ret;

	ret = ioctl(pOvl_priv->OvlFb[UILayer].fd, FBIOGET_VSCREENINFO, &pOvl_priv->cur_var);
    pOvl_priv->cur_var.vmode |= FB_VMODE_CONUPDATE;
	pOvl_priv->cur_var.activate = FB_ACTIVATE_NOW;
	pOvl_priv->cur_var.grayscale = 0;

//		ovlUpdatePanelSize();
//		pOvl_priv->cur_var.grayscale = (pOvl_priv->Panel_w << 8) | (pOvl_priv->Panel_h << 20);

   	memcpy(&pOvl_priv->OvlLay[layout].var, &pOvl_priv->cur_var, sizeof(struct fb_var_screeninfo));
//	pOvl_priv->OvlLay[layout].ResChange = FALSE;
	return ret;
}
//----------------------------------------------------------------------------------
int OvlSetupFb( OvlLayPg layout, OvlLayoutFormatType format, uint32_t xres, uint32_t yres)
{
    int ret;

    if(LayValidAndNotUI(layout)){

    	ret = ovlUpdVarOnChangeRes( layout);
    	ret |= ovlSetModeFb( layout, xres , yres, format);

    	if(layout >= EMU1Layer_IPP){
    		pOvl_priv->OvlLay[layout].var.nonstd = pOvl_priv->OvlLay[UILayer].var.nonstd & 0xff;

    		switch(layout){
    		case EMU1Layer_IPP:
    			ovlIppInitReg( 0, 0, xres, yres, 0,
    				pOvl_priv->OvlLay[EMU1Layer_IPP].var.xres_virtual, pOvl_priv->OvlLay[UILayer].var.xres_virtual);
    			ovlIPPSetFormats(ovlFromHWRkFormat(pOvl_priv->OvlLay[UILayer].var.nonstd));
    			break;
    		case EMU2Layer_RGA:
    			ovlRgaInitReg( 0, 0, 0,
    					pOvl_priv->OvlFb[UILayer].fix.smem_start, xres, yres,
					pOvl_priv->OvlLay[EMU2Layer_RGA].var.xres_virtual, pOvl_priv->OvlLay[UILayer].var.xres_virtual, TRUE);
    			ovlRGASetFormats(ovlFromHWRkFormat(pOvl_priv->OvlLay[UILayer].var.nonstd), BOTH_MODE);
    			break;
    		}
    	}

    }else
    	ret = -ENODEV;

    return ret;
}

//--------------------------------------------------------------------------------
int OvlSetupDrw( OvlLayPg layout, int Drw_x, int Drw_y, int Drw_w, int Drw_h)
{
    int ret=0;

    if(LayValidAndNotUI(layout)){
    	if(layout >= EMU1Layer_IPP){
    		switch(layout){
    		case EMU1Layer_IPP:
    			ovlIPPSetDrw(pOvl_priv->OvlFb[UILayer].fix.smem_start, Drw_w, Drw_h, Drw_x, Drw_y,
    					pOvl_priv->OvlLay[UILayer].var.xres_virtual);
    			break;
    		case EMU2Layer_RGA:
    			ovlRGASetDrw( Drw_w, Drw_h, Drw_x, Drw_y);
    			break;
    		}
    	}else{
    		pOvl_priv->OvlLay[layout].var.grayscale = (Drw_w << 8) | (Drw_h << 20);
    		pOvl_priv->OvlLay[layout].var.nonstd &= 0xff;
    		pOvl_priv->OvlLay[layout].var.nonstd |= (Drw_x<<8) + (Drw_y<<20);
    		ovlSetModeFb(layout, 0, 0, RKL_FORMAT_DEFAULT);
    	}
//    OvlClearBuf( pOvl_priv->OvlFb[pOvl_priv->OvlLay[layout].OvlFb].OvlMemPg);
    }else
    	ret = -ENODEV;
    return ret;
}
//----------------------------------------------------------------------------------
int OvlLayerLinkMemPg( OvlLayPg layout, OvlMemPgPtr MemPg)
{
    int ret=-EINVAL;
    uint32_t tmp[2];
    ovlFbPtr PFb;

    if(LayValidAndNotUI(layout)){
    	PFb = FbByLay(layout);

    	switch(layout){
#ifdef IPP_ENABLE
    	case EMU1Layer_IPP:
    		if(pOvl_priv->IPP_req.dst0.YrgbMst){
    			ovlIPPSetSrc(ToIntMemPg(MemPg)->phy_addr);
    			ret = ovlIppBlit();
    		}
    		break;
#endif
#ifdef RGA_ENABLE
    	case EMU2Layer_RGA:
    		if(pOvl_priv->RGA_req.dst.yrgb_addr){
    			ovlRGASetSrc(ToIntMemPg(MemPg)->phy_addr);
    			ret = ovlRgaBlit(RGA_BLIT_SYNC);
    		}
    		break;
#endif
    	default:
    		tmp[0] = ToIntMemPg(MemPg)->phy_addr;
    		tmp[1] = tmp[0] + ToIntMemPg(MemPg)->offset_uv;
    		ret = ioctl(ToIntFb(PFb)->fd, RK_FBIOSET_YUV_ADDR, &tmp);
    	}

    	if(!ret){
    		PFb->CurMemPg = MemPg;
//    		OvlWaitSync(layout);
    	}
    }

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
	int ret;
    if(LayHWValidAndNotUI(layout)){
		ioctl(FbByLay(layout)->fd, RK_FBIOSET_VSYNC_ENABLE, &vsync_en);
    	ret = ioctl(FbByLay(layout)->fd, RK_FBIOSET_ENABLE, &enable);
    }
    else
    	ret = -ENODEV;
    OVLDBG("layout:%d en:%d vsync:%d ret:%d", layout, enable, vsync_en, ret);

    return ret;
}
//---------------------------------------------------------------------
int ovlIsUsedAlloc( OvlLayPg layout)
{
    int ret, used=0;

    ret = ovlUSIAllocRes(layout);
    if(ret < 0)//if already in use
	used = 1;

    OVLDBG("layout:%d used:%d ret:%d", layout, used, ret);

    return used;
}
//---------------------------------------------------------------------
int ovlIsUsedCheck( OvlLayPg layout)
{
	int used;

	used = ovlIsUsedAlloc(layout);

    if(!used){
    	OvlEnable(layout, 0, 1); // workaround layer activation when opening
    	ovlUSIFreeRes(layout);
    }

    return used;

}
//---------------------------------------------------------------------
int ovlFreeUse( OvlLayPg layout)
{
	return ovlUSIFreeRes(layout);
}
//---------------------------------------------------------------------
int OvlResetFB( OvlLayPg layout)
{       
    int ret;

    if(LayHWValid(layout)){
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
    OvlLayPg lay=MAX_OVERLAYs;

    switch(type){
    case UI_L:
    case SCALE_L:
    case EMU_L:
//    case NOT_SCALEL:
    	for(lay=0;lay < MAX_OVERLAYs;lay++){
    		if(FbByLay(lay)->Type == type && pOvl_priv->OvlsAvl[lay]){
    			if( lay == EMU1Layer_IPP && OvlGetUIBpp() != 24)//IPP does not support 24 bit pixel
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
    case ANY_HW_L:
    case ANY_L://except UIL
    	for(lay=0;lay < MAX_OVERLAYs;lay++){
    		if(FbByLay(lay)->Type != UI_L && pOvl_priv->OvlsAvl[lay])
    			if(!(type == ANY_HW_L && lay >= EMU1Layer_IPP)){
    				if( lay == EMU1Layer_IPP && OvlGetUIBpp() != 24)//IPP does not support 24 bit pixel
    				if(!pOvl_priv->OvlLay[lay].InUse && !ovlIsUsedAlloc(lay)){
    					break;
    				}
    			}
    	}
    	break;
    default:
    	return ERROR_L;
    }

    OVLDBG("Select lay:%d", lay);

    if(LayValid(lay)){
    	if(FbBufAlloc > ALC_NONE_FB){
//front fb first by def
    		if(!pOvl_priv->OvlLay[lay].FbMemPgs[FRONT_FB]){
        		if(lay == UILayer){//User Interface layer
        			pOvl_priv->OvlLay[UILayer].FbMemPgs[FRONT_FB] = ovlInitMemPgDef();
        			if(pOvl_priv->OvlLay[UILayer].FbMemPgs[FRONT_FB]){
        				pOvl_priv->OvlLay[UILayer].FbMemPgs[FRONT_FB]->phy_addr = pOvl_priv->OvlFb[UILayer].fix.smem_start;
        				pOvl_priv->OvlLay[UILayer].FbMemPgs[FRONT_FB]->buf_size = pOvl_priv->OvlFb[UILayer].fix.smem_len;
        				pOvl_priv->OvlLay[UILayer].FbMemPgs[FRONT_FB]->MemPgType = UIFB_MEM;
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
	ovlFreeUse(lay);
err1:
	return ERROR_L;

}
//------------------------------------------------------------------
int OvlFreeLay( OvlLayPg layout)
{
	int ret=0;

    if(LayValid(layout)){
    	if(pOvl_priv->OvlLay[layout].InUse){
    		OvlFreeMemPg(  pOvl_priv->OvlLay[layout].FbMemPgs[FRONT_FB]);
    		pOvl_priv->OvlLay[layout].FbMemPgs[FRONT_FB] = NULL;
    		OvlFreeMemPg(  pOvl_priv->OvlLay[layout].FbMemPgs[BACK_FB]);
    		pOvl_priv->OvlLay[layout].FbMemPgs[BACK_FB] = NULL;
    		MBufByLay(layout) = NULL;
    		OvlEnable( layout, 0, 1);
    		pOvl_priv->OvlLay[layout].InUse = FALSE;
    		pOvl_priv->OvlLay[layout].ReqType = ERROR_L;
    		ovlFreeUse(layout);
    		switch(layout){
    		case EMU1Layer_IPP:
    			pOvl_priv->IPP_req.src0.YrgbMst = pOvl_priv->IPP_req.dst0.YrgbMst = 0;
    			break;
    		case EMU2Layer_RGA:
    			pOvl_priv->RGA_req.src.yrgb_addr = pOvl_priv->RGA_req.dst.yrgb_addr = 0;
    			break;
    		}
    	}else
        	ret = -ENODEV;
    }else
    	ret = -EINVAL;

    return ret;
}

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

    for(i=0;i<MAX_OVERLAYs;i++){
    	pOvl_priv->OvlFb[i].CurMemPg = NULL;

    	pOvl_priv->OvlLay[i].OvlFb = &pOvl_priv->OvlFb[i];

    	pOvl_priv->OvlLay[i].InUse = FALSE;
    	pOvl_priv->OvlLay[i].ReqType = ERROR_L;
    		//    	pOvl_priv->OvlLay[i].ResChange = FALSE;
    	pOvl_priv->OvlLay[i].FbBufUsed = FRONT_FB;
    	pOvl_priv->OvlLay[i].FbMemPgs[FRONT_FB] = NULL;
    	pOvl_priv->OvlLay[i].FbMemPgs[BACK_FB] = NULL;

    	switch(i){// UI_L=0, SCALE_L=1, ANY_HW_L =3, EMU_L = 4, ANY_L =6
    	case UILayer:
    		pOvl_priv->OvlFb[i].Type = UI_L;
    		break;
    	case Ovl1Layer:
    	case Ovl2Layer:
    		pOvl_priv->OvlFb[i].Type = SCALE_L;
    		break;
    	case EMU1Layer_IPP:
    	case EMU2Layer_RGA:
    		pOvl_priv->OvlFb[i].Type = EMU_L;
    		break;
    	default:
    		ERRMSG( "HW:Error indeterminate overlay type");
    	}

    	if(pOvl_priv->OvlsAvl[i])
    	{
    		memcpy(&pOvl_priv->OvlLay[i].var, &pOvl_priv->cur_var, sizeof(struct fb_var_screeninfo));
    		if(i < EMU1Layer_IPP)
    			ioctl(pOvl_priv->OvlFb[i].fd, FBIOGET_FSCREENINFO, &pOvl_priv->OvlFb[i].fix);

    		if(MasterMode){
    			OvlEnable( i, 0, 1);
    		}
//    	else{
    		ovlIsUsedCheck(i);// deactivate nonused layers
    	}

    }

}

//------------------------------------------------------------------
int ovl_setup_ovl()
{
    int i /*,ret=0*/;

    pOvl_priv->OvlsCnt = 0;

    for(i=0;i<MAX_OVERLAYs;i++)
    	pOvl_priv->OvlsAvl[i]=FALSE;

    pOvl_priv->OvlFb[UILayer].fd = open(FB_DEV_UI, O_RDWR);
    if (pOvl_priv->OvlFb[UILayer].fd < 0){
//    	ret = pOvl_priv->OvlFb[UILayer].fd;
    	ERRMSG( "HW:Error open FB_DEV_UI");
//    	goto err;
    	return pOvl_priv->OvlFb[UILayer].fd;
    }
    pOvl_priv->OvlsCnt++;
    pOvl_priv->OvlsAvl[UILayer]=TRUE;

    pOvl_priv->OvlFb[Ovl1Layer].fd = open(FB_DEV_O1, O_RDONLY); //main Ovl_priv
    if (pOvl_priv->OvlFb[Ovl1Layer].fd < 0){
//    	ret = pOvl_priv->OvlFb[Ovl1Layer].fd;
    	ERRMSG( "HW:Error open FB_DEV_O1");
//    	goto err1;
    }else{
    	pOvl_priv->OvlsCnt++;
    	pOvl_priv->OvlsAvl[Ovl1Layer]=TRUE;
    }

    pOvl_priv->OvlFb[Ovl2Layer].fd = open(FB_DEV_O2, O_RDONLY);
    if (pOvl_priv->OvlFb[Ovl2Layer].fd < 0){
//    	ret = pOvl_priv->OvlFb[Ovl2Layer].fd;
    	ERRMSG( "HW:Error open FB_DEV_O2");
//    	goto err2;
    }
    else{
    	pOvl_priv->OvlsCnt++;
    	pOvl_priv->OvlsAvl[Ovl2Layer]=TRUE;
    }

    OVLDBG("HWOvlsCnt:%d", pOvl_priv->OvlsCnt);

    pOvl_priv->MaxPgSize = FB_MAXPGSIZE;

    ioctl(pOvl_priv->OvlFb[UILayer].fd, FBIOGET_VSCREENINFO, &pOvl_priv->cur_var);

//	ioctl(pOvl_priv->fd_o1, FBIOBLANK, FB_BLANK_UNBLANK);
/*	tmp=1;
	ioctl(pOvl_priv->OvlFb[UILayer].fd, RK_FBIOSET_OVERLAY_STATE, &tmp);
        return(TRUE);
    }
*/
    return 0;
/*
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
    */
}

//----------------------------main init--------------------------
int Open_RkLayers(Bool MasterMode)
{
	int ret=0;//, tmp=1;

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

    pOvl_priv->fd_USI = ovlInitUSIHW();
    if(pOvl_priv->fd_USI <= 0){
    	ERRMSG( "HW:Error USI");
    	ump_close();
    	MFREE(pOvl_priv);
    	return -ENODEV;
    }

    OVLDBG( "HW:Initialize overlays");

    ret = ovl_setup_ovl();
    if(ret< 0){
    	ERRMSG( "HW:Error overlays");
    	goto err;
    }

//    tmp = ret;

    pOvl_priv->IPP_req.src0.YrgbMst = pOvl_priv->IPP_req.dst0.YrgbMst = 0;
    pOvl_priv->RGA_req.src.yrgb_addr = pOvl_priv->RGA_req.dst.yrgb_addr = 0;
#ifdef IPP_ENABLE
    OVLDBG( "HW:Initialize IPP");
//    if (!LoadKernelModule("rk29-ipp"))
//    	OVLDBG( "can't load 'rk29-ipp' kernel module");
    pOvl_priv->OvlFb[EMU1Layer_IPP].fd = ovlInitIPPHW();
    if(pOvl_priv->OvlFb[EMU1Layer_IPP].fd < 0){
    	ERRMSG( "HW:Error IPP");
    }else{
    	pOvl_priv->OvlsCnt++;
    	pOvl_priv->OvlsAvl[EMU1Layer_IPP]=TRUE;
        pthread_mutex_init(&pOvl_priv->ippmutex, NULL);
    }
#endif
#ifdef RGA_ENABLE
    OVLDBG( "HW:Initialize RGA");
    pOvl_priv->OvlFb[EMU2Layer_RGA].fd = ovlInitRGAHW();
    if(pOvl_priv->OvlFb[EMU2Layer_RGA].fd < 0){
	ERRMSG( "HW:Error RGA");
    }else{
    	pOvl_priv->OvlsCnt++;
    	pOvl_priv->OvlsAvl[EMU2Layer_RGA]=TRUE;
        pthread_mutex_init(&pOvl_priv->rgamutex, NULL);
    }
#endif

    if(pOvl_priv->OvlsCnt < 2){
    	ERRMSG( "HW:Not found overlays");
    	ret = -ENODEV;
    	goto err;
    }

    OVLDBG("OvlsCnt:%d", pOvl_priv->OvlsCnt);
    set_ovl_param(MasterMode);

    OVLDBG( "HW:Initialize USI");
/*    if(tmp)
    if (!LoadKernelModule("rk_ump")){
    	ERRMSG("can't load usi_ump kernel module");
    	ret = -ENODEV;
    	goto err;
    }
*/


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

    	for(i = 0;i < MAX_OVERLAYs;i++){
    		if(pOvl_priv->OvlsAvl[i] && pOvl_priv->OvlFb[i].fd > 0){
    			if(OvlFreeLay(i))
    				ovlIsUsedCheck(i);// deactivate nonused layers
    			close(pOvl_priv->OvlFb[i].fd);
    		}
    	}

    	if(pOvl_priv->fd_USI > 0)
    		close(pOvl_priv->fd_USI);

    	ump_close();

        MFREE(pOvl_priv);
    }
}
