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

#include "rk_ump.h"
/*
fb0:win0	
fb1:win1	
ARGB888
RGB888
RGB565
YCbCr420
YCbCr422
YCbCr444

fb2:win2 	
ARGB888
RGB888
RGB565
8bpp
4bpp
2bpp
1bpp

*/

#define USI_MIN_ALLOC_SIZE		(UMP_MINIMUM_SIZE * 100)

#define FB_DEV_O2	"/dev/fb2" //second overlay(not scalable)
#define FB_DEV_O1	"/dev/fb1" //main overlay
#define FB_DEV_UI	"/dev/fb0"
#define FB_DEV_IPP	"/dev/rk29-ipp"
#define FB_DEV_RGA	"/dev/rga"
#define FB_DEV_USI	"/dev/"USI_UMP_DRV_NAME
#define FB_SYS_HDMI	"/sys/class/display/HDMI"

#define HDMI_MODE_TMPL	"%dx%dp-60"

enum {
    FBUI,
    FBO1,
    FBO2,
};
/*
#define FB_MAXPGS_O1 2
#define FB_MAXPGS_O2 2
#define FB_MAXPGS_UI 1
*/

#define MAX_PANEL_SIZE_X 1920
//#define PANEL_SIZE_X 1280
#define MAX_PANEL_SIZE_Y 1080
//#define PANEL_SIZE_Y 720

#define FB_MAXPGSIZE MAX_PANEL_SIZE_X*MAX_PANEL_SIZE_Y*4

#define RK_FBIOPUT_COLOR_KEY_CFG    0x4626
#define RK_FBIOGET_PANEL_SIZE	0x5001
#define RK_FBIOSET_YUV_ADDR	0x5002

#define RK_FBIOSET_OVERLAY_STATE   0x5018
#define RK_FBIOSET_ENABLE          0x5019
#define RK_FBIOGET_ENABLE          0x5020
#define FBIO_WAITFORVSYNC       _IOW('F', 0x20, __u32)

struct color_key_cfg {
	uint32_t win0_color_key_cfg;
	uint32_t win1_color_key_cfg;
	uint32_t win2_color_key_cfg;
};

typedef struct
{
    uint32_t	size_x;
    uint32_t	size_y;
} SPanelSize;

enum {
    RGBA_8888          = 1,
    RGBX_8888          = 2,
    RGB_888            = 3,
    RGB_565            = 4,
	BGRA_8888          = 5,
	RGBA_5551          = 6,
	RGBA_4444          = 7,
    /* Legacy formats (deprecated), used by ImageFormat.java */
    YCbCr_422_SP       = 0x10, // NV16	16
    YCrCb_NV12_SP      = 0x20, // YUY2	32
    YCrCb_444          = 0x22, //yuv444 34
//add formats NOT for display
    YCbCr_422_P       = 0x11, // NV16	16
    YCrCb_NV12_P      = 0x21, // YUY2	32

};
