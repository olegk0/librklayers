/*****************************************************************************
 * chroma_neon.h
 *****************************************************************************
 * Copyright (C) 2011 Rémi Denis-Courmont
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

/* Planes must start on a 16-bytes boundary. Pitches must be multiples of 16
 * bytes even for subsampled components. */

/* Planar picture buffer.
 * Pitch corresponds to luminance component in bytes. Chrominance pitches are
 * inferred from the color subsampling ratio. */
struct yuv_planes
{
    void *y, *u, *v;
    size_t pitch;
};

/* Planar chroma buffers.
 * Pitch is in bytes. */
struct uv_planes
{
    void *u, *v;
    size_t pitch;
};

struct y_uv_planes
{
    void *y, *uv;
    size_t pitch;
};

/* Packed picture buffer. Pitch is in bytes (_not_ pixels). */
struct yuv_pack
{
    void *yuv;
    size_t pitch;
};

struct y_copy
{
    void *d_y, *s_y;
    size_t s_pitch;
};

/* UYVY to semiplanar  conversion. */
void yuyv_semiplanar_neon (struct y_uv_planes *const out,
                     const struct yuv_pack *const in,
                     int width, int height) asm("yuyv_semiplanar_neon");

/* Planar to semiplanar  conversion. */
void interleave_chroma_neon (struct yuv_pack *const out,
                               const struct uv_planes *const in,
                               int width, int height) asm("interleave_chroma_neon");

void copy_neon (struct y_copy *const in, int d_pitch,
		             int width, int height) asm("copy_neon");

void memset_neon (void *const dst, int fill, uint32_t size) asm("memset_neon");
