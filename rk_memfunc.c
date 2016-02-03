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

//++++++++++++++++++++++++++++++++++++USI++++++++++++++++++++++++++++++++++++++++++++
int ovlInitUSIHW()
{
    return open(FB_DEV_USI, O_RDONLY);
}
//-------------------------------------------------------------
int ovlUSIAllocMem( struct usi_ump_mbs *uum)
{
//    int ret;

    if(uum->size < USI_MIN_ALLOC_SIZE)
    	uum->size = USI_MIN_ALLOC_SIZE;
//    	return -EINVAL;
    OVLDBG( "fd_USI:%d size:%d", pOvl_priv->fd_USI, uum->size);
    return ioctl(pOvl_priv->fd_USI, USI_ALLOC_MEM_BLK, uum);
}
//-------------------------------------------------------------
int ovlUSIFreeMem( ump_secure_id	secure_id)
{
//    int ret;

    return ioctl(pOvl_priv->fd_USI, USI_FREE_MEM_BLK, &secure_id);
}
//-------------------------------------------------------------
int ovlUSIGetStat( struct usi_ump_mbs_info *uumi)
{
//    int ret;
//    struct usi_ump_mbs uum;

    return ioctl(pOvl_priv->fd_USI, USI_GET_INFO, uumi);
}
//-------------------------------------------------------------
int ovlUSIAllocRes(int res)
{
//    int ret;

    return ioctl(pOvl_priv->fd_USI, USI_ALLOC_RES, &res);
}
//-------------------------------------------------------------
int ovlUSIFreeRes(int res)
{
//    int ret;

    return ioctl(pOvl_priv->fd_USI, USI_FREE_RES, &res);
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ovlMemPgPtr ovlInitMemPgDef()
{
	ovlMemPgPtr		FbMemPg;

	if(!(FbMemPg = calloc(1, sizeof(*FbMemPg) )))
		return NULL;

	//calloc =( malloc + zero) - there is no need set pointers to null
	FbMemPg->ump_fb_secure_id = UMP_INVALID_SECURE_ID;
	FbMemPg->ump_handle = UMP_INVALID_MEMORY_HANDLE;
	FbMemPg->MemPgType = BUF_MEM;//by def

	return FbMemPg;
}
//------------------------------------------------------------
int OvlUnMapBufMem( OvlMemPgPtr PMemPg)
{
    int ret = -ENODEV;

    if(PMemPg != NULL && ToIntMemPg(PMemPg)->fb_mmap != NULL){
    	ovlclearbuf( PMemPg);
    	if(ToIntMemPg(PMemPg)->MemPgType == UIFB_MEM){
    		ret = munmap(ToIntMemPg(PMemPg)->fb_mmap, ToIntMemPg(PMemPg)->buf_size);
    		//fbdevHWUnmapVidmem();
    	}
    	else{
    		if( ToIntMemPg(PMemPg)->ump_handle != UMP_INVALID_MEMORY_HANDLE){
    			ump_mapped_pointer_release(ToIntMemPg(PMemPg)->ump_handle);
    			ret = 0;
    		}
    	}
    	ToIntMemPg(PMemPg)->fb_mmap = NULL;
    }
    return ret;
}
//----------------------------------------------------------------------
void *OvlMapBufMem( OvlMemPgPtr PMemPg)
{

    if(PMemPg != NULL){
    	if(ToIntMemPg(PMemPg)->fb_mmap == NULL){
    		if(ToIntMemPg(PMemPg)->MemPgType == UIFB_MEM){
    			ToIntMemPg(PMemPg)->fb_mmap = mmap( NULL, ToIntMemPg(PMemPg)->buf_size, PROT_READ | PROT_WRITE, MAP_SHARED, pOvl_priv->OvlFb[UILayer].fd, 0);
    			//fbdevHWMapVidmem();
        		if(ToIntMemPg(PMemPg)->fb_mmap == MAP_FAILED){
        			ERRMSG("Map failed:%d",errno);
        			ToIntMemPg(PMemPg)->fb_mmap = NULL;
        		}
        	}else{
        		if( ToIntMemPg(PMemPg)->ump_fb_secure_id == UMP_INVALID_SECURE_ID)
        			return NULL;
        		ToIntMemPg(PMemPg)->ump_handle = ump_handle_create_from_secure_id(ToIntMemPg(PMemPg)->ump_fb_secure_id);
        		if(ToIntMemPg(PMemPg)->ump_handle == UMP_INVALID_MEMORY_HANDLE)
        			return NULL;
        		ToIntMemPg(PMemPg)->fb_mmap = ump_mapped_pointer_get(ToIntMemPg(PMemPg)->ump_handle);
        	}
    		ovlclearbuf( ToIntMemPg(PMemPg));
//    		return PMemPg->fb_mmap;
    	}
   		return ToIntMemPg(PMemPg)->fb_mmap;
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
OvlMemPgPtr OvlAllocMemPg( uint32_t size, uint32_t UV_offset)//except UI
{
    OvlMemPgPtr MemPg;
    struct usi_ump_mbs uum;
    int ret;

    MemPg = ovlInitMemPgDef();
    if(MemPg){
    	uum.size = size + UMP_MINIMUM_SIZE * 10; //~40kb save buf
    	ret = ovlUSIAllocMem( &uum);
    	if(!ret){
    		ToIntMemPg(MemPg)->buf_size = uum.size;
    		ToIntMemPg(MemPg)->phy_addr = uum.addr;
    		ToIntMemPg(MemPg)->ump_fb_secure_id = uum.secure_id;
    		if(UV_offset)
    			ToIntMemPg(MemPg)->offset_uv = ((UV_offset + PAGE_MASK) & ~PAGE_MASK);
    		else
    			ToIntMemPg(MemPg)->offset_uv = ((ToIntMemPg(MemPg)->buf_size / 2 + PAGE_MASK) & ~PAGE_MASK);
    	}else{
    		ERRMSG( "Error USIAllocMem, size:%d ret:%d", size, ret);
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
    		ret |= ovlUSIFreeMem( ToIntMemPg(PMemPg)->ump_fb_secure_id);
    	MFREE(PMemPg);
    }
    return ret;
}
