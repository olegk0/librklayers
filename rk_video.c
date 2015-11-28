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

//convert packed U0Y0V0Y1 U2Y2V2Y3 to SemiPlanar for display
void OvlCopyPackedToFb(OvlMemPgPtr PMemPg, const void *src, int dstPitch, int h, int w, Bool reverse)
{
	uint32_t Fvars[4];
	void *dst_Y = ToIntMemPg(PMemPg)->fb_mmap;
	void *dst_UV = dst_Y;

	if(reverse)
		dst_Y += ToIntMemPg(PMemPg)->offset_mio;
	else
		dst_UV += ToIntMemPg(PMemPg)->offset_mio;

    Fvars[0]= (w/*px*/)>>2;//stride; aligned by 4px (4px (8bytes) per pass)
    Fvars[1]= h;//lines;
    Fvars[2]= (w/*px*/)<<1;//SrcPitch;
    Fvars[3]= dstPitch/*px*/;
#ifdef __arm__
        asm volatile (
        "up0: \n\t"
        "mov	v5,#0 \n\t"//counter
        "push {%[Yvar],%[Src],%[UVvar]}\n\t"
        "up1: \n\t"
        "ldr	v1,[%[Src]]\n\t"
        "bic	v2,v1,#0xFFFFFF00\n\t"
        "bic	v3,v1,#0xFF00FFFF\n\t"
        "orr	v3,v2,v3,lsr #8\n\t"
        "bic	v1,v1,#0x00FF00FF\n\t"
        "orr	v1,v1,v1,lsl #8\n\t"
        "mov	v6,v1,lsr #16\n\t"
//
        "ldr	v1,[%[Src],#4]\n\t"//4byte skip
        "bic	v2,v1,#0xFF00FF00\n\t"
        "orr	v2,v2,v2,lsr #8\n\t"
        "orr	v3,v3,v2,lsl #16\n\t"
        "str	v3,[%[Yvar]]\n\t"
        "bic	v2,v1,#0xFFFF00FF\n\t"
        "bic	v1,v1,#0x00FFFFFF\n\t"
        "orr	v1,v1,v2,lsl #8\n\t"
        "orr	v6,v6,v1\n\t"
        "str	v6,[%[UVvar]]\n\t"

        "add	%[Src],%[Src],#8 \n\t"//8byte skip
        "add	%[Yvar],%[Yvar],#4 \n\t"//4byte skip
        "add	%[UVvar],%[UVvar],#4 \n\t"//4byte skip
        "add	v5,v5,#1 \n\t"//+1
        "ldr	v1,[%[Fvars],%[Stride]]\n\t"
        "cmp	v5,v1 \n\t"// = stride?
        "bne	up1 \n\t"//if no - jmp up

    "pop {%[Yvar],%[Src],%[UVvar]}\n\t"
        "ldr	v1,[%[Fvars],%[SrcPitch]]\n\t"
        "add	%[Src],%[Src],v1\n\t"
        "ldr	v1,[%[Fvars],%[DstPitch]]\n\t"
        "add	%[Yvar],%[Yvar],v1\n\t"
        "add	%[UVvar],%[UVvar],v1\n\t"

        "ldr	v2,[%[Fvars],%[Lines]]\n\t"
        "subs	v2,v2,#1 \n\t"//-1 and set zero flag if 0
        "str	v2,[%[Fvars],%[Lines]]\n\t"
        "bne	up0 \n\t"//if no - jmp up0
    : : [Src] "r"(src), [Yvar] "r"(dst_Y), [UVvar] "r"(dst_UV), [Fvars] "r"(&Fvars), [Stride] "J"(0),
     [Lines] "J"(4*1), [SrcPitch] "J"(4*2), [DstPitch] "J"(4*3)
        : "v1","v2","v3","v6","v5","memory"

        );
#endif
}
//-----------------------------------------------------------------
void OvlCopyPlanarToFb(OvlMemPgPtr PMemPg, const void *src_Y, unsigned int offs_U, unsigned int offs_V,
		int dstPitch, int h, int w)
{
    uint32_t Fvars[4];
    void *src_U = src_Y + offs_U;
	void *src_V = src_Y + offs_V;
	void *dst_Y = ToIntMemPg(PMemPg)->fb_mmap;
	void *dst_UV = dst_Y + ToIntMemPg(PMemPg)->offset_mio;

    Fvars[0]= (w/*px*/)>>2;//stride;
    Fvars[1]= h;//lines;
    Fvars[2]= (w/*px*/);//SrcPitch;
    Fvars[3]= dstPitch/*px*/;
#ifdef __arm__
        asm volatile (
//************Y block
        "ldr	v3,[%[Fvars],%[Lines]]\n\t"
        "ldr	v5,[%[Fvars],%[Stride]]\n\t"
        "up20: \n\t"
        "mov	v6,#0 \n\t"//counter
        "push {%[SrcY],%[Yvar]}\n\t"
        "up21: \n\t"
        "ldr	v1,[%[SrcY]]\n\t"
        "str	v1,[%[Yvar]]\n\t"
        "add	%[SrcY],%[SrcY],#4\n\t"
        "add	%[Yvar],%[Yvar],#4\n\t"

        "add	v6,v6,#1 \n\t"//+1
        "cmp	v6,v5 \n\t"// = stride?
        "bne	up21 \n\t"//if no - jmp up

    "pop {%[SrcY],%[Yvar]}\n\t"
        "ldr	v1,[%[Fvars],%[SrcPitch]]\n\t"
        "add	%[SrcY],%[SrcY],v1\n\t"
        "ldr	v2,[%[Fvars],%[DstPitch]]\n\t"
        "add	%[Yvar],%[Yvar],v2\n\t"

        "subs	v3,v3,#1 \n\t"//-1 and set zero flag if 0
        "bne	up20 \n\t"//if no - jmp up0
//************UV block
        "ldr	v3,[%[Fvars],%[Lines]]\n\t"
        "mov	v3,v3,lsr #1 \n\t"
        "str	v3,[%[Fvars],%[Lines]]\n\t"
        "ldr	v5,[%[Fvars],%[Stride]]\n\t"
        "up30: \n\t"
        "mov	v6,#0 \n\t"//counter
    "push {%[SrcU],%[SrcV],%[UVvar]}\n\t"
        "up31: \n\t"

    "ldrh	v1,[%[SrcU]]\n\t"
    "add	%[SrcU],%[SrcU],#2\n\t"
    "bic	v3,v1,#0xFFFFFF00\n\t"
    "bic	v2,v1,#0xFFFF00FF\n\t"
    "orr	v3,v3,v2,lsl #8\n\t"//v3=0x00uu00uu
    "ldrh	v1,[%[SrcV]]\n\t"
    "add	%[SrcV],%[SrcV],#2\n\t"
    "bic	v2,v1,#0xFFFFFF00\n\t"
    "orr	v3,v3,v2,lsl #8\n\t"//v3=0x00uuvvuu
    "bic	v2,v1,#0xFFFF00FF\n\t"
    "orr	v3,v3,v2,lsl #16\n\t"//v3=0xvvuuvvuu
    "str	v3,[%[UVvar]]\n\t"
    "add	%[UVvar],%[UVvar],#4 \n\t"//4byte skip

        "add	v6,v6,#1 \n\t"//+1
        "cmp	v6,v5 \n\t"// = stride?
        "bne	up31 \n\t"//if no - jmp up

    "pop {%[SrcU],%[SrcV],%[UVvar]}\n\t"
        "ldr	v1,[%[Fvars],%[SrcPitch]]\n\t"
    "add	%[SrcU],%[SrcU],v1,lsr #1\n\t"
    "add	%[SrcV],%[SrcV],v1,lsr #1\n\t"
        "ldr	v2,[%[Fvars],%[DstPitch]]\n\t"
    "add	%[UVvar],%[UVvar],v2\n\t"
        "ldr	v2,[%[Fvars],%[Lines]]\n\t"
        "subs	v2,v2,#1 \n\t"//-1 and set zero flag if 0
        "str	v2,[%[Fvars],%[Lines]]\n\t"
        "bne	up30 \n\t"//if no - jmp up0
    : : [SrcY] "r"(src_Y), [SrcU] "r"(src_U), [SrcV] "r"(src_V), [Yvar] "r"(dst_Y), [UVvar] "r"(dst_UV),
    [Fvars] "r"(&Fvars), [Stride] "J"(0), [Lines] "J"(4*1), [SrcPitch] "J"(4*2), [DstPitch] "J"(4*3)
        : "a1","v1","v2","v3","v5","v6","memory"

        );
#endif
}
