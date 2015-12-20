 @*****************************************************************************
 @ 2015 olegk0 <olegvedi@gmail.com>
 @ based on:
 @
 @ deinterleave_chroma.S : ARM NEONv1 conversion of interleaved to planar chroma
 @*****************************************************************************
 @ Copyright (C) 2009-2011 Rémi Denis-Courmont
 @ Copyright (C) 2013 Martin Storsjö
 @
 @ This program is free software; you can redistribute it and/or modify
 @ it under the terms of the GNU Lesser General Public License as published by
 @ the Free Software Foundation; either version 2.1 of the License, or
 @ (at your option) any later version.
 @
 @ This program is distributed in the hope that it will be useful,
 @ but WITHOUT ANY WARRANTY; without even the implied warranty of
 @ MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 @ GNU Lesser General Public License for more details.
 @
 @ You should have received a copy of the GNU Lesser General Public License
 @ along with this program; if not, write to the Free Software Foundation,
 @ Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 @****************************************************************************/

	.syntax unified
	.fpu neon
	.text

#define UV	r0
#define COUNT	r1
#define WIDTH	r2
#define HEIGHT	r3
#define DPITCH	r4
#define DPAD	r4
#define U	r5
#define V	r6
#define SPITCH	lr
#define SPAD	lr

    .align 2
    .global interleave_chroma_neon
    .type	interleave_chroma_neon, %function
interleave_chroma_neon:
    push		{r4-r6,lr}
    ldmia		r0,	{UV, DPITCH}
    ldmia		r1,	{U, V, SPITCH}
    cmp		HEIGHT,	#0

    @	one UV block on two pixel by x
    mov		WIDTH,	WIDTH,	lsr #1
    @	and on two by y
    mov		HEIGHT,	HEIGHT,	lsr #1
    @ round the width up to a multiple of 8
    add		WIDTH,	WIDTH, #7
    bic		WIDTH,	WIDTH, #7

    sub		DPAD,	DPITCH,	WIDTH, lsl #1
    sub		SPAD,	SPITCH,	WIDTH
1:
    movsgt		COUNT,	WIDTH
    pople		{r4-r6,pc}
2:
    pld		[U, #64]
@	vld1.u8		{d0},		[U,:64]!
    vld1.u8		{d0},		[U]!
    pld		[V, #64]
@	vld1.u8		{d1},		[V,:64]!
    vld1.u8		{d1},		[V]!
@	vst2.u8		{d0, d1},	[UV,:128]!
    vst2.u8		{d0, d1},	[UV]!
    subs		COUNT,	COUNT,	#8
    bgt		2b

    subs		HEIGHT,	#1
    add		UV,	UV,	DPAD
    add		U,	U,	SPAD
    add		V,	V,	SPAD
    b		1b

@*************************************************************************
#define Y	r0
#define COUNT	r1
#define WIDTH	r2
#define HEIGHT	r3
#define UV2	r4
#define DPITCH2	r5
#define DPAD2	r5
#define S	r6
#define SPITCH	lr
#define SPAD	lr

	.global yuyv_semiplanar_neon
    .type	yuyv_semiplanar_neon, %function
yuyv_semiplanar_neon:
    push		{r4-r6,lr}
    ldmia		r0,	{Y, UV2, DPITCH2}
    ldmia		r1,	{S, SPITCH}
    cmp		HEIGHT,	#0
    sub		DPAD2,	DPITCH2,WIDTH
    sub		SPAD,	SPITCH,	WIDTH,	lsl #1
1:
    movsgt		COUNT,	WIDTH
    pople		{r4-r6,pc}
2:
    pld		[S, #64]
    subs		COUNT,	COUNT,	#8
    @ YUYV => d0:YY + d1:UV
    vld2.u8		{d0, d1},	[S]! @ 16
@	vld1.u8		{q0-q1},	[S,:128]!
    vst1.u8		{d0},		[Y]! @ 8
    vst1.u8		{d1},		[UV2]! @ 8
    bgt		2b

    subs		HEIGHT,	#1
    add		S,	S,	SPAD
    add		Y,	Y,	DPAD2
    add		UV2,	UV2,	DPAD2@,	lsr #1
    b		1b

@*************************************************************************
#define dY		r0
#define DPITCH	r1
#define DPAD	r1
#define WIDTH	r2
#define HEIGHT	r3
#define sY		r4
#define CNT		r5
#define BWIDTH	r6
#define SPITCH	lr
#define SPAD	lr

	.global copy_neon
    .type	copy_neon, %function
copy_neon:
    push		{r4-r7,lr}
    vpush		{d0-d3}
    ldmia		r0,	{dY, sY, SPITCH}
    cmp		HEIGHT,	#2
    ble		3f
	subs	HEIGHT,	#1

	mov		BWIDTH, WIDTH
	bic		BWIDTH, #3
@	round	up 32bytes align
	add		WIDTH,	#31
	bic		WIDTH, #31

	sub		DPAD, DPITCH, WIDTH
	sub		SPAD, SPITCH, WIDTH
1:
    mov		CNT,	WIDTH

@	copy with 32bytes accuracy
2:
@    pld		[sY, #128]@???
@    pld		[sY, #0xC0]
@    subs		COUNT,	COUNT,	#8
@    vld2.u8		{d0},	[sY]!
@    vst1.u8		{d0},	[dY]!
	vldm	sY!,{d0-d3}
	vstm	dY!,{d0-d3}
	subs	CNT,	#32
    bgt		2b

    add		dY, DPAD
    add		sY, SPAD
    subs	HEIGHT,	#1
    bne		1b
@	last line with 4bytes accuracy
4:
	ldr		CNT, [sY], #4
	str		CNT, [dY], #4
	subs	BWIDTH, #4
	bgt		4b
3:
	vpop	{d0-d3}
    pop		{r4-r7,pc}

@*************************************************************************
#define DST		r0
#define FILL	r1
#define SIZE	r2

	.global memset_neon
    .type	memset_neon, %function
memset_neon:
    push		{lr}
    vpush		{d0-d3}
@	round down
    bics	SIZE, #31
    beq		3f

	vdup.8	d0, FILL
	vmov	d1, d0
	vmov	d2, d0
	vmov	d3, d0

2:
    pld		[DST, #128]@???
	vstm	DST!,{d0-d3}
	subs	SIZE,	#32
    bgt		2b

3:
	vpop	{d0-d3}
    pop		{pc}
