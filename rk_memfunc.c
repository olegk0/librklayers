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
#include <sys/mman.h>
#include <xf86drm.h>
#include <xf86drmMode.h>


struct drm_rockchip_gem_create {
    uint64_t size;
    uint32_t flags;
    uint32_t handle;
};

/* memory type definitions. */
enum e_drm_rockchip_gem_mem_type {
        /* Physically Continuous memory and used as default. */
        ROCKCHIP_BO_CONTIG      = 0 << 0,
        /* Physically Non-Continuous memory. */
        ROCKCHIP_BO_NONCONTIG   = 1 << 0,
        /* non-cachable mapping and used as default. */
        ROCKCHIP_BO_NONCACHABLE = 0 << 1,
        /* cachable mapping. */
        ROCKCHIP_BO_CACHABLE    = 1 << 1,
        /* write-combine mapping. */
        ROCKCHIP_BO_WC          = 1 << 2,
        ROCKCHIP_BO_MASK                = ROCKCHIP_BO_NONCONTIG | ROCKCHIP_BO_CACHABLE |
                                        ROCKCHIP_BO_WC
};

#define DRM_ROCKCHIP_GEM_CREATE 0x00
#define DRM_IOCTL_ROCKCHIP_GEM_CREATE DRM_IOWR(DRM_COMMAND_BASE + \
	DRM_ROCKCHIP_GEM_CREATE, struct drm_rockchip_gem_create)

//++++++++++++++++++++++++++++++++++++DRM++++++++++++++++++++++++++++++++++++++++++++
int ovlInitDRMHW()
{
    return open(FB_DEV_DRM, O_RDWR | O_CLOEXEC);
}
//-------------------------------------------------------------
int ovlDRMAllocMem( struct drm_rockchip_gem_create *cr)
{
    if(cr->size < DRM_MIN_ALLOC_SIZE)
    	cr->size = DRM_MIN_ALLOC_SIZE;

    cr->flags = ROCKCHIP_BO_CONTIG | ROCKCHIP_BO_WC;

    OVLDBG( "fd_DRM:%d size:%llu", pOvl_priv->fd_DRM, cr->size);
    return drmIoctl(pOvl_priv->fd_DRM, DRM_IOCTL_ROCKCHIP_GEM_CREATE, cr);
}
//-------------------------------------------------------------
int ovlDRMFreeMem( uint32_t handle)
{
    struct drm_mode_destroy_dumb arg;
//    int ret;
    memset(&arg, 0, sizeof(arg));
    arg.handle = handle;

    return drmIoctl(pOvl_priv->fd_DRM, DRM_IOCTL_MODE_DESTROY_DUMB, &arg);
}
//-------------------------------------------------------------
/*int ovlDRMGetStat( struct usi_ump_mbs_info *uumi)
{
//    int ret;
//    struct usi_ump_mbs uum;

    return ioctl(pOvl_priv->fd_DRM, DRM_GET_INFO, uumi);
}
*/
//-------------------------------------------------------------
/*int ovlDRMAllocRes(int res)
{
//    int ret;

    return ioctl(pOvl_priv->fd_DRM, DRM_ALLOC_RES, &res);
}
//-------------------------------------------------------------
int ovlDRMFreeRes(int res)
{
//    int ret;

    return ioctl(pOvl_priv->fd_DRM, DRM_FREE_RES, &res);
}
*/
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ovlMemPgPtr ovlInitMemPgDef()
{
	ovlMemPgPtr		FbMemPg;

	if(!(FbMemPg = calloc(1, sizeof(*FbMemPg) )))
		return NULL;

	//calloc =( malloc + zero) - there is no need set pointers to null
	FbMemPg->mem_id = 0;
	FbMemPg->mem_handle = 0;
	FbMemPg->MemPgType = BUF_MEM;//by def

	return FbMemPg;
}
//------------------------------------------------------------
int OvlUnMapBufMem( OvlMemPgPtr PMemPg)
{
    int ret = -ENODEV;

    if(PMemPg != NULL && ToIntMemPg(PMemPg)->fb_mmap != NULL){
    	ovlclearbuf( PMemPg);
//    	if(ToIntMemPg(PMemPg)->MemPgType == UIFB_MEM)
	{
    		ret = munmap(ToIntMemPg(PMemPg)->fb_mmap, ToIntMemPg(PMemPg)->buf_size);
    		//fbdevHWUnmapVidmem();
    	}
    	ToIntMemPg(PMemPg)->fb_mmap = NULL;
    }
    return ret;
}
//----------------------------------------------------------------------
void *OvlMapBufMem( OvlMemPgPtr PMemPg)
{
    struct drm_mode_map_dumb mreq;

    if(PMemPg != NULL){
    	if(ToIntMemPg(PMemPg)->fb_mmap == NULL){
    		if(ToIntMemPg(PMemPg)->MemPgType == UIFB_MEM){
    			ToIntMemPg(PMemPg)->fb_mmap = mmap( NULL, ToIntMemPg(PMemPg)->buf_size,
				 PROT_READ | PROT_WRITE, MAP_SHARED, pOvl_priv->OvlFb[UILayer].fd, 0);
    			//fbdevHWMapVidmem();
        	}else{
        		if( ToIntMemPg(PMemPg)->mem_id == 0)
        			return NULL;
			// prepare buffer for memory mapping 
			memset(&mreq, 0, sizeof(mreq));
			mreq.handle = ToIntMemPg(PMemPg)->mem_handle;
			if(drmIoctl(pOvl_priv->fd_DRM, DRM_IOCTL_MODE_MAP_DUMB, &mreq))
			    return NULL;
        		ToIntMemPg(PMemPg)->fb_mmap = mmap(0, ToIntMemPg(PMemPg)->buf_size,
				PROT_READ | PROT_WRITE, MAP_SHARED, pOvl_priv->fd_DRM, mreq.offset);
//ERRMSG("buf_size:%lu offset:%lld handle:%u phy:%lX",ToIntMemPg(PMemPg)->buf_size,mreq.offset,mreq.handle,ToIntMemPg(PMemPg)->phy_addr);
        	}

    		if(ToIntMemPg(PMemPg)->fb_mmap == MAP_FAILED){
    		    ERRMSG("Map failed:%d %m",errno);
    		    ToIntMemPg(PMemPg)->fb_mmap = NULL;
    		}else
		    ovlclearbuf( ToIntMemPg(PMemPg));
   		return ToIntMemPg(PMemPg)->fb_mmap;
    	}
    }
    return NULL;
}
//------------------------------------------------------------------
uint32_t OvlGetUVoffsetMemPg( OvlMemPgPtr PMemPg)
{

    if(PMemPg)
    	return ToIntMemPg(PMemPg)->offset_uv;
    else
    	return 0;
}
//------------------------------------------------------------------
uint32_t OvlGetPhyAddrMemPg( OvlMemPgPtr PMemPg)
{

    if(PMemPg)
    	return ToIntMemPg(PMemPg)->phy_addr;
    else
    	return 0;
}
//------------------------------------------------------------------
uint32_t OvlGetName(uint32_t handle)
{
    int ret;
    struct drm_gem_flink flink;

    flink.handle = handle;
    ret = drmIoctl(pOvl_priv->fd_DRM, DRM_IOCTL_GEM_FLINK, &flink);
    if (ret) {
        return 0;
    }

    return flink.name;
}
//------------------------------------------------------------------
OvlMemPgPtr OvlAllocMemPg( uint32_t size, uint32_t UV_offset)//except UI
{
    OvlMemPgPtr MemPg;
    struct drm_rockchip_gem_create cr;
    int ret;

    MemPg = ovlInitMemPgDef();
    if(MemPg){
    	cr.size = size + 4096 * 10; //~40kb save buf
    	ret = ovlDRMAllocMem( &cr );
    	if(!ret){
		ToIntMemPg(MemPg)->mem_id = OvlGetName(cr.handle);
		if(ToIntMemPg(MemPg)->mem_id <= 0){
		    ERRMSG( "Error get mem id for handle:%d, ret:%d", cr.handle,ToIntMemPg(MemPg)->mem_id);
		    ovlDRMFreeMem(cr.handle);
		    MFREE(MemPg);
		}else{
//    OVLDBG( "get mem id:%d for handle:%d, phy:%X",ToIntMemPg(MemPg)->mem_id, cr.handle, cr.flags);
		    ToIntMemPg(MemPg)->buf_size = cr.size;
		    ToIntMemPg(MemPg)->phy_addr = cr.flags;
		    ToIntMemPg(MemPg)->mem_handle = cr.handle;
		    if(UV_offset)
			ToIntMemPg(MemPg)->offset_uv = ((UV_offset + PAGE_MASK) & ~PAGE_MASK);
		    else
			ToIntMemPg(MemPg)->offset_uv = ((ToIntMemPg(MemPg)->buf_size / 2 + PAGE_MASK) & ~PAGE_MASK);
		}
	}else{
		ERRMSG( "Error DRMAllocMem, size:%d ret:%d", size, ret);
		MFREE(MemPg);
    	}
    }else{
    	ERRMSG( "Error InitMemPgDef");
    }
    return MemPg;
}
//------------------------------------------------------------------
int OvlFreeMemPg( OvlMemPgPtr PMemPg)
{
    int ret=0;

    if(PMemPg){
    	ret = OvlUnMapBufMem(  PMemPg);
    	if(ToIntMemPg(PMemPg)->MemPgType != UIFB_MEM)
    		ret |= ovlDRMFreeMem( ToIntMemPg(PMemPg)->mem_handle);
    	MFREE(PMemPg);
    }
    return ret;
}
