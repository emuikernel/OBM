/******************************************************************************
 *
 *  (C)Copyright 2008 - 2012 Marvell. All Rights Reserved.
 *
 *  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF MARVELL.
 *  The copyright notice above does not evidence any actual or intended
 *  publication of such source code.
 *  This Module contains Proprietary Information of Marvell and should be
 *  treated as Confidential.
 *  The information in this file is provided for the exclusive use of the
 *  licensees of Marvell.
 *  Such users have the right to use, modify, and incorporate this code into
 *  products for purposes authorized by the license agreement provided they
 *  include this notice and the associated copyright notice with any such
 *  product.
 *  The information in this file is provided "AS IS" without warranty.
 *
 ******************************************************************************/

#include "mpu.h"
#include "Typedef.h"


arm946eControlRegU OBM_arm946eControlReg_DS;


const Arm946eMpuMemoryRegion  OBM_arm946eMpuRegionsL_UpR0_DS[] =
{//regionNo 			BaseAddr						Size-CODE                   ICsh	 DCsh	Bufbl   I/D-attr

//Open all the address
{ARM946E_MPU_REGION_0,  0x00000000/*All address*/, ARM946E_MPU_REGION_SIZE_4G,	FALSE,	FALSE,	FALSE,  ACCESS_RW,	ARM946E_MPU_REGION_ENABLE},
{ARM946E_MPU_REGION_1,	0xD4000000/* registers*/, ARM946E_MPU_REGION_SIZE_32M,	FALSE,	FALSE,	FALSE,	ACCESS_RW,	ARM946E_MPU_REGION_ENABLE},
{ARM946E_MPU_REGION_2,	0x00000000/*DDR	 */, ARM946E_MPU_REGION_SIZE_32M,	TRUE,	TRUE,	TRUE,	ACCESS_RW,	ARM946E_MPU_REGION_ENABLE}
};
UINT32 OBM_Arm946eCP15GetControlReg_DS(void);
UINT32 OBM_Arm946eCP15SetDataCacheEnable_DS(void){
	OBM_Arm946eCP15GetControlReg_DS();
	OBM_arm946eControlReg_DS.bits.dCacheEnable = CACHE_ENABLE;
	return OBM_asm_Arm9WriteCP15_DS(OBM_arm946eControlReg_DS.all,ARM946E_CP15_CONTROL_REG);
}

UINT32 OBM_Arm946eCP15SetDataCacheDisable_DS(void){
	OBM_Arm946eCP15GetControlReg_DS();
	OBM_arm946eControlReg_DS.bits.dCacheEnable = CACHE_DISABLE;
	return OBM_asm_Arm9WriteCP15_DS(OBM_arm946eControlReg_DS.all,ARM946E_CP15_CONTROL_REG);
}

UINT32 OBM_Arm946eCP15SetInstCacheDisable_DS(void){
	OBM_Arm946eCP15GetControlReg_DS();
	OBM_arm946eControlReg_DS.bits.iCacheEnable = CACHE_DISABLE;
	return OBM_asm_Arm9WriteCP15_DS(OBM_arm946eControlReg_DS.all,ARM946E_CP15_CONTROL_REG);
}

UINT32 OBM_Arm946eCP15SetBPUDisable_DS(void){
	OBM_Arm946eCP15GetControlReg_DS();
	OBM_arm946eControlReg_DS.bits.bpuEnable = 0;
	return OBM_asm_Arm9WriteCP15_DS(OBM_arm946eControlReg_DS.all,ARM946E_CP15_CONTROL_REG);
}

UINT32 OBM_Arm946eCP15SetInstCacheEnable_DS(void){
	OBM_Arm946eCP15GetControlReg_DS();
	OBM_arm946eControlReg_DS.bits.iCacheEnable = CACHE_ENABLE;
	return OBM_asm_Arm9WriteCP15_DS(OBM_arm946eControlReg_DS.all,ARM946E_CP15_CONTROL_REG);
}

UINT32 OBM_Arm946eCP15SetMPUEnalbe_DS(void){
	OBM_Arm946eCP15GetControlReg_DS();
	OBM_arm946eControlReg_DS.bits.pmuEnable = PMU_ENABLE;
	return OBM_asm_Arm9WriteCP15_DS(OBM_arm946eControlReg_DS.all,ARM946E_CP15_CONTROL_REG);
}

UINT32 OBM_Arm946eCP15SetMPUDisable_DS(void){
	OBM_Arm946eCP15GetControlReg_DS();
	OBM_arm946eControlReg_DS.bits.pmuEnable = PMU_DISABLE;
	return OBM_asm_Arm9WriteCP15_DS(OBM_arm946eControlReg_DS.all,ARM946E_CP15_CONTROL_REG);
}


UINT32 OBM_Arm946eCP15GetControlReg_DS(void){
	OBM_arm946eControlReg_DS.all = OBM_asm_Arm9ReadCP15_DS(ARM946E_CP15_CONTROL_REG);
	return (OBM_arm946eControlReg_DS.all);
}


UINT32 OBM_Arm946eCP15GetMPUEnalbe_DS(void)
{
  OBM_Arm946eCP15GetControlReg_DS();
  return OBM_arm946eControlReg_DS.bits.pmuEnable;
}

UINT32 OBM_Arm946eCP15RestoreMPUEnableDisable_DS(UINT32 enableDisable){
	OBM_Arm946eCP15GetControlReg_DS();
	OBM_arm946eControlReg_DS.bits.pmuEnable = enableDisable;
	return OBM_asm_Arm9WriteCP15_DS(OBM_arm946eControlReg_DS.all,ARM946E_CP15_CONTROL_REG);
}

UINT32 OBM_Arm946eCP15SetRegionBaseAndSizeReg_DS(EMpuRegionsId region,UINT32 baseAddress,UINT32 regionSize,EMpuRegionMode regionEnableDisable)
{
	UINT32 res = 0x11111111; //init with any invalid value
	UINT32 regionN=ARM946E_CP15_PROTECTION_REGION_0_REG;

	_arm946eProtectionRegionBaseAndSizeReg_U _arm946eProtectionRegionBaseAndSizeReg;

	_arm946eProtectionRegionBaseAndSizeReg.all =0x0L;
	_arm946eProtectionRegionBaseAndSizeReg.bits.regionEnableDisable = regionEnableDisable;
	_arm946eProtectionRegionBaseAndSizeReg.bits.regionSize = regionSize;
	_arm946eProtectionRegionBaseAndSizeReg.bits.regionBaseAddress =baseAddress >> REGION_BASE_REGISTER_OFFSET;

	if (region==ARM946E_MPU_REGION_0)
		{
		  regionN = ARM946E_CP15_PROTECTION_REGION_0_REG;
		}
	else if (region==ARM946E_MPU_REGION_1)
		{
		  regionN = ARM946E_CP15_PROTECTION_REGION_1_REG;
		}
	else if (region==ARM946E_MPU_REGION_2)
		{
		  regionN = ARM946E_CP15_PROTECTION_REGION_2_REG;
		}
	else if (region==ARM946E_MPU_REGION_3)
		{
		  regionN = ARM946E_CP15_PROTECTION_REGION_3_REG;
		}
	else if (region==ARM946E_MPU_REGION_4)
		{
		  regionN = ARM946E_CP15_PROTECTION_REGION_4_REG;
		}
	else if (region==ARM946E_MPU_REGION_5)
		{
		  regionN = ARM946E_CP15_PROTECTION_REGION_5_REG;
		}
	else if (region==ARM946E_MPU_REGION_6)
		{
		  regionN = ARM946E_CP15_PROTECTION_REGION_6_REG;
		}
	else if (region==ARM946E_MPU_REGION_7)
		{
		  regionN = ARM946E_CP15_PROTECTION_REGION_7_REG;
		}
	res=OBM_asm_Arm9WriteCP15_DS(_arm946eProtectionRegionBaseAndSizeReg.all,regionN);

	return res;

}

UINT32 OBM_Arm946eCP15SetRegionAccessPermission_DS(EMpuRegionsId region,
                                             CMpuRegionAttributes attribute)
{
	UINT32 api;
	UINT32 apd;

	// Get the value of the CP15 regions
	apd=OBM_asm_Arm9ReadCP15_DS(ARM946E_CP15_DATA_ACCESS_PREMISSION_REG);
	api=OBM_asm_Arm9ReadCP15_DS(ARM946E_CP15_INSTRUCTION_ACCESS_PREMISSION_REG);

	attribute.dattributs = attribute.dattributs & ( ARM946E_MPU_AP_MASK );
	attribute.dattributs = attribute.dattributs << (region * 4);
	attribute.iattributs = attribute.iattributs & ( ARM946E_MPU_AP_MASK );
	attribute.iattributs = attribute.iattributs << (region * 4);
	if (region==ARM946E_MPU_REGION_0)
		{
			api = api & ARM946E_MPU_AP0_MASK;
			apd = apd & ARM946E_MPU_AP0_MASK;
		}
	else if (region==ARM946E_MPU_REGION_1)
		{
			api = api & ARM946E_MPU_AP1_MASK;
			apd = apd & ARM946E_MPU_AP1_MASK;
		}
	else if (region==ARM946E_MPU_REGION_2)
		{
			api = api & ARM946E_MPU_AP2_MASK;
			apd = apd & ARM946E_MPU_AP2_MASK;
		}
	else if (region==ARM946E_MPU_REGION_3)
		{
			api = api & ARM946E_MPU_AP3_MASK;
			apd = apd & ARM946E_MPU_AP3_MASK;
		}
	else if (region==ARM946E_MPU_REGION_4)
		{
			api = api & ARM946E_MPU_AP4_MASK;
			apd = apd & ARM946E_MPU_AP4_MASK;
		}
	else if (region==ARM946E_MPU_REGION_5)
		{
			api = api & ARM946E_MPU_AP5_MASK;
			apd = apd & ARM946E_MPU_AP5_MASK;
		}
	else if (region==ARM946E_MPU_REGION_6)
		{
			api = api & ARM946E_MPU_AP6_MASK;
			apd = apd & ARM946E_MPU_AP6_MASK;
		}
	else if (region==ARM946E_MPU_REGION_7)
		{
			api = api & ARM946E_MPU_AP7_MASK;
			apd = apd & ARM946E_MPU_AP7_MASK;
		}

	api = api | attribute.iattributs ;
	apd = apd | attribute.dattributs ;

	OBM_asm_Arm9WriteCP15_DS(apd,ARM946E_CP15_DATA_ACCESS_PREMISSION_REG);
	OBM_asm_Arm9WriteCP15_DS(api,ARM946E_CP15_INSTRUCTION_ACCESS_PREMISSION_REG);

	return 0;
}

void OBM_Arm946MpuInitRegion_DS(const Arm946eMpuMemoryRegion * region)
{
   UINT32 reg;

   if (region->regionId != ARM946E_MPU_NO_REGION)
   {

     // Set region base address & size & work mode (Enable\Disable)
     OBM_Arm946eCP15SetRegionBaseAndSizeReg_DS(region->regionId,
                        region->baseAddress,
                        region->regionSize,
                      region->regionEnableDisable);
     // Set access permissions
     OBM_Arm946eCP15SetRegionAccessPermission_DS(region->regionId,region->attributes);

     // Set D Cachable attributes
	 reg=OBM_asm_Arm9ReadCP15_DS(ARM946E_CP15_DATA_CACHEABLE_REG);
     reg=SET_CLEAR_BIT(reg, region->regionId, region->isDCachable);
	 OBM_asm_Arm9WriteCP15_DS(reg,ARM946E_CP15_DATA_CACHEABLE_REG);

     // Set I Cachable attributes
	 reg=OBM_asm_Arm9ReadCP15_DS(ARM946E_CP15_INST_CACHABLE_REG);
     reg=SET_CLEAR_BIT(reg, region->regionId, region->isICachable);
	 OBM_asm_Arm9WriteCP15_DS(reg,ARM946E_CP15_INST_CACHABLE_REG);

     // Set bufferable attributes
	 reg=OBM_asm_Arm9ReadCP15_DS(ARM946E_CP15_DATA_BUFFERABLE_REG);
     reg=SET_CLEAR_BIT(reg, region->regionId, region->isBufferable);
	 OBM_asm_Arm9WriteCP15_DS(reg,ARM946E_CP15_DATA_BUFFERABLE_REG);
   }
}

void OBM_Arm946eMpuInit_DS(char bEnableMpu)
{
  int ip;
  unsigned int mpuEnableDisable;
  const Arm946eMpuMemoryRegion*     mpuRegionTable=0;

  // Get the current MPU status
  mpuEnableDisable = OBM_Arm946eCP15GetMPUEnalbe_DS(); //(Arm946eCP15GetMPUEnalbe)();
  
  // verify that the MPU is disabled
  OBM_Arm946eCP15SetMPUDisable_DS();
  
  mpuRegionTable = OBM_arm946eMpuRegionsL_UpR0_DS;

  // Init memory regions
  for (ip=0 ; ip < 3 ; ip++ )
  {
    OBM_Arm946MpuInitRegion_DS(mpuRegionTable+ip);
  }

  // restore the MPU state
  OBM_Arm946eCP15RestoreMPUEnableDisable_DS(mpuEnableDisable);

  if (bEnableMpu == TRUE)
  {
    OBM_Arm946eCP15SetMPUEnalbe_DS();
  }
}

/************************************************************************
* Function: CacheInvalidateMemory										*
*************************************************************************
* Description: This function invalidates the cacheable memory           *
*              Does NOT write-back data caches (except "partial" line). *
*                                                                       *
*       ...otherBuff <32b |32-bytes|32-bytes| <32 otherBuff...          *
* Buffer:          begin--|========|========|--end                      *
* Action:  Clean & Invalid|   Invalidate    |Clean & Invalidate         *
*                                                                       *
* Notes: Clearing the pending interrupt source must be done in the ISR,	*
*		 and is the source's responsibility!!!							*
************************************************************************/
void CacheInvalidateMemory( void *pMem, UINT32 size)
{
    UINT32
        address = (UINT32)pMem & ~CACHE_LINE_MASK,
        cpsr,
        end_address = (UINT32)pMem + size,
        last_address_to_invalidate = end_address & ~CACHE_LINE_MASK ;

    if ( address != (UINT32)pMem )
    {
        DisableIrqInterrupts();
        CPUCleanDCacheLine( address ) ;
        CPUInvalidateDCacheLine( address ) ;
        EnableIrqInterrupts();
        address += SIZEOF_CACHE_LINE ;
    }

    for( ; address<last_address_to_invalidate ; address+=SIZEOF_CACHE_LINE )
        CPUInvalidateDCacheLine( address ) ;

    if ( last_address_to_invalidate != end_address )
    {
        DisableIrqInterrupts();
        CPUCleanDCacheLine( address ) ;
        CPUInvalidateDCacheLine( address ) ;
        EnableIrqInterrupts();
    }
}  /*CacheInvalidateMemory*/

/************************************************************************
* Function: CacheCleanMemory											*
*************************************************************************
* Description: This function evaluates the cacheable memory cleaning	*
*              Write-back cache data to memory  (FLUSH)                 *
*                                                                       *
*       ...otherBuff <32b |32-bytes|32-bytes| <32 otherBuff...          *
* Buffer:          begin--|========|========|--end                      *
* Action:       INT  Clean|     Clean       |Clean with INT dis/ena     *
*                                                                       *
************************************************************************/
void CacheCleanMemory( void *pMem ,UINT32 size)
{
    UINT32
        address = (UINT32)pMem & ~CACHE_LINE_MASK,
        end_address ;

    for( end_address=(UINT32)pMem+size ; address<end_address ; address+=SIZEOF_CACHE_LINE )
        CPUCleanDCacheLine( address ) ;
} /*CacheCleanMemory*/

UINT32 OBM_Arm946eCP15InvalidateInstCache(void)
{
	return OBM_asm_Arm9WriteCP15_DS(0x0L, ARM946E_CP15_FLUSH_INST_CACHE_REG);
}
UINT32 OBM_Arm946eCP15InvalidateDataCache(void)
{
	return OBM_asm_Arm9WriteCP15_DS(0x0L, ARM946E_CP15_FLUSH_DATA_CACHE_REG);
}
UINT32 OBM_Arm946eCP15DrainWriteBuffer(void)
{
	return OBM_asm_Arm9WriteCP15_DS(0x0L, ARM946E_CP15_DRAIN_WRITE_BUFFER_REG);

}


int L1_writebuffer_drain()
{
	/* CP15 Register 7 */
	register unsigned value=0;

	cp15_reg_write(c7, c10, 5, value);

	return 0;
}

void OBM_MPUCache_Init()
{
	/* MPU initialize.*/
	OBM_Arm946eMpuInit_DS(TRUE);

	/* Enable ICache (via CP15) */
	OBM_Arm946eCP15SetInstCacheEnable_DS();

	/* Enable DCache (via CP15) */
	OBM_Arm946eCP15SetDataCacheEnable_DS();
}

void OBM_Flush()
{
	L1_writebuffer_drain();
	FlushIcache();
	CleanDcache();
}

void OBM_MPUCache_Disable()
{
	OBM_Arm946eCP15SetDataCacheDisable_DS();
	OBM_Arm946eCP15SetInstCacheDisable_DS();
	OBM_Arm946eCP15SetBPUDisable_DS();
	OBM_Arm946eCP15SetMPUDisable_DS();
}


