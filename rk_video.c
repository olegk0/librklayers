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
#include <string.h>

//convert packed U0Y0V0Y1 U2Y2V2Y3 to SemiPlanar for display
//void OvlCopyPackedToFb(OvlMemPgPtr PMemPg, const void *src, int srcPitch, int dstPitch, int w, int h, Bool reverse)
void OvlCopyPackedToFb(OvlMemPgPtr PMemPg, const void *src, int srcPitch, int w, int h, Bool reverse)
{
	void *dst_Y = ToIntMemPg(PMemPg)->fb_mmap;
	void *dst_UV = dst_Y;

	if(reverse)
		dst_Y += ToIntMemPg(PMemPg)->offset_uv;
	else
		dst_UV += ToIntMemPg(PMemPg)->offset_uv;

    struct yuv_pack in = {src, srcPitch};
//    struct y_uv_planes out = {dst_Y, dst_UV, dstPitch};
    struct y_uv_planes out = {dst_Y, dst_UV, w};
    yuyv_semiplanar_neon (&out, &in, w, h);

}
//-----------------------------------------------------------------
void OvlCopyPlanarToFb(OvlMemPgPtr PMemPg, const void *src_Y, const void *src_U, const void *src_V,
		int srcPitch, int w, int h)
//		int srcPitch, int dstPitch, int w, int h)
{
//	int i;
	void *dst_Y = ToIntMemPg(PMemPg)->fb_mmap;
	void *dst_UV = dst_Y + ToIntMemPg(PMemPg)->offset_uv;

//	if(dstPitch == w)
//		memcpy(dst_Y, src_Y, w*h);
//	else{
	struct y_copy inY = {dst_Y, src_Y, srcPitch};
//		copy_neon (&inY, dstPitch, w, h);
	copy_neon (&inY, w, w, h);
//	}
		/*
		for(i=0;i<h;i++){
			memcpy(dst_Y, src_Y, w);
			dst_Y += dstPitch;
			src_Y += w;
		}
*/
//    struct yuv_pack out = {dst_UV, dstPitch};
    struct yuv_pack out = {dst_UV, w};
    struct uv_planes in = {src_U, src_V, srcPitch>>1};
    interleave_chroma_neon (&out, &in, w, h);

}

void OvlCopyNV12SemiPlanarToFb(OvlMemPgPtr PMemPg, const void *src_Y, const void *src_UV,
		int srcPitch, int dstPitch, int w, int h)
{
	void *dst_Y = ToIntMemPg(PMemPg)->fb_mmap;
	void *dst_UV = dst_Y + ToIntMemPg(PMemPg)->offset_uv;

	struct y_copy inY = {dst_Y, src_Y, srcPitch};
	copy_neon (&inY, dstPitch, w, h);
//	copy_neon (&inY, w, w, h);

	struct y_copy in = {dst_UV, src_UV, srcPitch};
	copy_neon (&in, dstPitch, w, h>>1);
//	copy_neon (&in, w, w, h>>1);

}

void OvlCopyNV16SemiPlanarToFb(OvlMemPgPtr PMemPg, const void *src_Y, const void *src_UV,
		int srcPitch, int dstPitch, int w, int h)
{
	void *dst_Y = ToIntMemPg(PMemPg)->fb_mmap;
	void *dst_UV = dst_Y + ToIntMemPg(PMemPg)->offset_uv;

	struct y_copy inY = {dst_Y, src_Y, srcPitch};
	copy_neon (&inY, dstPitch, w, h);
//	copy_neon (&inY, w, w, h);

	struct y_copy in = {dst_UV, src_UV, srcPitch};
	copy_neon (&in, dstPitch, w, h);
//	copy_neon (&in, w, w, h);

}
