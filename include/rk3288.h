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

#ifndef _RK3288_H_
#define _RK3288_H_

/*
win2	
win3	
ARGB888
RGB888
RGB565
YCbCr420
YCbCr422
YCbCr444

win0 	
win1 	
ARGB888
RGB888
RGB565
8bpp
4bpp
2bpp
1bpp
*/
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define DRM_MIN_ALLOC_SIZE		(4096 * 100)

#define HW_OVERLAYs 5

#define FB_DEV	"/dev/fb"
#define TO_FB_DEV(n)	FB_DEV STR(n)

#define FB_DEV_UI	TO_FB_DEV(0)
#define FB_DEV_UI1	TO_FB_DEV(1)

#define FB_DEV_O1	TO_FB_DEV(2)
#define FB_DEV_O2	TO_FB_DEV(3)

#define FB_DEV_HWC	TO_FB_DEV(4)
//#define FB_DEV_IPP	"/dev/rk29-ipp"
//#define FB_DEV_RGA	"/dev/rga"
#define FB_DEV_DRM	"/dev/dri/card0"
#define FB_SYS_HDMI	"/sys/class/display/display0.HDMI"
#define HDMI_MODE_TMPL	"%dx%dp-60"

#define LAYERS_HW_TYPE	HW_UI,HW_NONE,HW_SCALE|HW_YUV,HW_SCALE|HW_YUV,HW_HWC


#define RK_FBIOPUT_COLOR_KEY_CFG    0x4626
#define RK_FBIOSET_VSYNC_ENABLE     0x4629
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
    YCrCb_444          = 0x25, //yuv444 34
//add formats NOT for display
    YCbCr_422_P       = 0x23, //
    YCrCb_NV12_P      = 0x22, // YUY2	32
/*
        HAL_PIXEL_FORMAT_YCbCr_422_SP = 0x10,   // NV16
        HAL_PIXEL_FORMAT_YCrCb_NV12 = 0x20,     // YUY2

        HAL_PIXEL_FORMAT_YCrCb_NV12_10      = 0x22, // YUV420_1obit
        HAL_PIXEL_FORMAT_YCbCr_422_SP_10        = 0x23, // YUV422_1obit
        HAL_PIXEL_FORMAT_YCrCb_420_SP_10        = 0x24, //YUV444_1obit

        HAL_PIXEL_FORMAT_YCrCb_444 = 0x25,      //yuv444

*/
};
#endif