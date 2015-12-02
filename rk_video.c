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

#include "rk_layers_priv.h"
#include "chroma_neon.h"
#include <string.h>

//convert packed U0Y0V0Y1 U2Y2V2Y3 to SemiPlanar for display
void OvlCopyPackedToFb(OvlMemPgPtr PMemPg, const void *src, int dstPitch, int w, int h, Bool reverse)
{
	void *dst_Y = ToIntMemPg(PMemPg)->fb_mmap;
	void *dst_UV = dst_Y;

	if(reverse)
		dst_Y += ToIntMemPg(PMemPg)->offset_mio;
	else
		dst_UV += ToIntMemPg(PMemPg)->offset_mio;

    struct yuv_pack in = {src, w<<1};
    struct y_uv_planes out = {dst_Y, dst_UV, dstPitch};
    yuyv_semiplanar_neon (&out, &in, w, h);

}
//-----------------------------------------------------------------
void OvlCopyPlanarToFb(OvlMemPgPtr PMemPg, const void *src_Y, unsigned int offs_U, unsigned int offs_V,
		int dstPitch, int w, int h)
{
	int i;
    void *src_U = src_Y + offs_U;
	void *src_V = src_Y + offs_V;
	void *dst_Y = ToIntMemPg(PMemPg)->fb_mmap;
	void *dst_UV = dst_Y + ToIntMemPg(PMemPg)->offset_mio;

	if(dstPitch == w)
		memcpy(dst_Y, src_Y, w*h);
	else{
		struct y_copy in = {dst_Y, src_Y};
		copy_neon (&in, dstPitch, w, h);
	}

    struct yuv_pack out = {dst_UV, dstPitch};
    struct uv_planes in = {src_U, src_V, w>>1};
    interleave_chroma_neon (&out, &in, w, h);

}
