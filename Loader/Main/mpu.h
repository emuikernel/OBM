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
/*
 *  Use this file for MPU definitions
 */

#ifndef __MPU_H__
#define __MPU_H__


#include "Typedef.h"

#define cp15_reg_read(primary_reg, secondary_reg, opcode, value) \
	__asm__ __volatile__("mrc	p15, 0, %0, " #primary_reg ", " #secondary_reg ", " #opcode "\n"	: "=r" (value)	:: "memory");

#define cp15_reg_write(primary_reg, secondary_reg, opcode, value) \
	__asm__ __volatile__("mcr	p15, 0, %0, " #primary_reg ", " #secondary_reg ", " #opcode "\n"	: : "r" (value) : "memory");


#define TRUE                		1
#define FALSE               		0
#define BOOL                		int

#define     SIZEOF_CACHE_LINE       32
#define     CACHE_LINE_MASK         (SIZEOF_CACHE_LINE-1)

#define REGION_BASE_REGISTER_OFFSET (12)
// Access permission encoding
#define ARM946E_MPU_ATT_SUP_NO_ACCESS_USR_NO_ACCESS (0x0L) // (b0000)
#define ARM946E_MPU_ATT_SUP_RW_USR_NO_ACCESS        (0x1L) // (b0001)
#define ARM946E_MPU_ATT_SUP_RW_USR_R                (0x2L) // (b0010)
#define ARM946E_MPU_ATT_SUP_RW_USR_RW               (0x3L) // (b0011)
#define ARM946E_MPU_ATT_SUP_R_USR_NO_ACCESS         (0x5L) // (b0101)
#define ARM946E_MPU_ATT_SUP_R_USR_R                 (0x6L) // (b0110)

// Region size encoding
#define ARM946E_MPU_REGION_SIZE_4K   (0xBL)  // (b01011)
#define ARM946E_MPU_REGION_SIZE_8K   (0xCL)  // (b01100)
#define ARM946E_MPU_REGION_SIZE_16K  (0xDL)  // (b01101)
#define ARM946E_MPU_REGION_SIZE_32K  (0xEL)  // (b01110)
#define ARM946E_MPU_REGION_SIZE_64K  (0xFL)  // (b01111)
#define ARM946E_MPU_REGION_SIZE_128K (0x10L) // (b10000)
#define ARM946E_MPU_REGION_SIZE_256K (0x11L) // (b10001)
#define ARM946E_MPU_REGION_SIZE_512K (0x12L) // (b10010)
#define ARM946E_MPU_REGION_SIZE_1M   (0x13L) // (b10011)
#define ARM946E_MPU_REGION_SIZE_2M   (0x14L) // (b10100)
#define ARM946E_MPU_REGION_SIZE_4M   (0x15L) // (b10101)
#define ARM946E_MPU_REGION_SIZE_8M   (0x16L) // (b10110)
#define ARM946E_MPU_REGION_SIZE_16M  (0x17L) // (b10111)
#define ARM946E_MPU_REGION_SIZE_32M  (0x18L) // (b11000)
#define ARM946E_MPU_REGION_SIZE_64M  (0x19L) // (b11001)
#define ARM946E_MPU_REGION_SIZE_128M (0x1AL) // (b11010)
#define ARM946E_MPU_REGION_SIZE_256M (0x1BL) // (b11011)
#define ARM946E_MPU_REGION_SIZE_512M (0x1CL) // (b11100)
#define ARM946E_MPU_REGION_SIZE_1G   (0x1DL) // (b11101)
#define ARM946E_MPU_REGION_SIZE_2G   (0x1EL) // (b11110)
#define ARM946E_MPU_REGION_SIZE_4G   (0x1FL) // (b11111)


// Instruction & data access permission masks
#define ARM946E_MPU_AP_MASK	 (0x0000000F)
#define ARM946E_MPU_AP0_MASK (0xFFFFFFF0)
#define ARM946E_MPU_AP1_MASK (0xFFFFFF0F)
#define ARM946E_MPU_AP2_MASK (0xFFFFF0FF)
#define ARM946E_MPU_AP3_MASK (0xFFFF0FFF)
#define ARM946E_MPU_AP4_MASK (0xFFF0FFFF)
#define ARM946E_MPU_AP5_MASK (0xFF0FFFFF)
#define ARM946E_MPU_AP6_MASK (0xF0FFFFFF)
#define ARM946E_MPU_AP7_MASK (0x0FFFFFFF)

#define SET_CLEAR_BIT(inpVALUE,bitNUM,toSET)     (  (toSET) ? (inpVALUE | (1<<bitNUM)) : (inpVALUE & (~(1<<bitNUM)) )  )
////////////             I-attr                            D-Attr
#define ACCESS_RW     {ARM946E_MPU_ATT_SUP_RW_USR_RW,	ARM946E_MPU_ATT_SUP_RW_USR_RW}

#define ACCESS_R      {ARM946E_MPU_ATT_SUP_R_USR_R  ,	ARM946E_MPU_ATT_SUP_R_USR_R  }

#define ACCESS_NOACCESS {ARM946E_MPU_ATT_SUP_NO_ACCESS_USR_NO_ACCESS, ARM946E_MPU_ATT_SUP_NO_ACCESS_USR_NO_ACCESS}


// TCM load mode
typedef enum{
	TCM_LOAD_MODE_DISABLE = 0,
	TCM_LOAD_MODE_ENABLE
}ETCMLoadMode;

typedef enum{
	TCM_DISABLE = 0,
	TCM_ENABLE
}ETCMMode;

typedef enum{
	CACHE_DISABLE = 0,
	CACHE_ENABLE
}ECacheMode;

typedef enum{
	VECTOR_SELECT_MODE0 = 0,
	VECTOR_SELECT_MODE1
}EVectorSelectMode;

typedef enum{
	LITTLE_ENDIAN =0,
	BIG_ENDIAN
}EEndianConfiguration;

typedef enum{
	PMU_DISABLE = 0,
	PMU_ENABLE
}EPMUMode;

typedef struct{
	UINT_T iattributs;
	UINT_T dattributs;
}CMpuRegionAttributes;

typedef enum {
	ARM946E_CP15_ID_CODE_REG =0 ,
	ARM946E_CP15_CACHE_DETAILS_REG,
	ARM946E_CP15_TCM_SIZE_REG,
	ARM946E_CP15_CONTROL_REG,
	ARM946E_CP15_DATA_CACHEABLE_REG,
	ARM946E_CP15_INST_CACHABLE_REG,
	ARM946E_CP15_DATA_BUFFERABLE_REG,
	ARM946E_CP15_DATA_ACCESS_PREMISSION_REG,
	ARM946E_CP15_INSTRUCTION_ACCESS_PREMISSION_REG,
	ARM946E_CP15_PROTECTION_REGION_0_REG,
	ARM946E_CP15_PROTECTION_REGION_1_REG,
	ARM946E_CP15_PROTECTION_REGION_2_REG,
	ARM946E_CP15_PROTECTION_REGION_3_REG,
	ARM946E_CP15_PROTECTION_REGION_4_REG,
	ARM946E_CP15_PROTECTION_REGION_5_REG,
	ARM946E_CP15_PROTECTION_REGION_6_REG,
	ARM946E_CP15_PROTECTION_REGION_7_REG,
	ARM946E_CP15_FLUSH_INST_CACHE_REG,
	ARM946E_CP15_INVALIDATE_INST_CACHE_SINGLE_ENTRY_REG,
	ARM946E_CP15_PREFETCH_INST_CACHE_LINE_REG,
	ARM946E_CP15_FLUSH_DATA_CACHE_REG,                      /*20*/
	ARM946E_CP15_INVALIDATE_DATA_CACHE_SINGLE_ENTRY_REG,
	ARM946E_CP15_CLEAN_DATA_CACHE_ENTRY_REG,
	/*Added by AlexR 15.07.2004*/
	ARM946E_CP15_CLEAN_AND_FLASH_DATA_CACHE__ENTRY_REG,
	ARM946E_CP15_FLUSH_DATA_CACHE_SINGLE_ENTRY_INDEX_AND_SEGMENT_REG,
	ARM946E_CP15_CLEAN_DATA_CACHE_INDEX_AND_SEGMENT_ENTRY_REG,
	ARM946E_CP15_DRAIN_WRITE_BUFFER_REG,
	ARM946E_CP15_LOW_POWER_STATE_REG,
	ARM946E_CP15_DATA_LOCKDOWN_CONTROL_REG,
	ARM946E_CP15_INST_LOCKDOWN_CONTROL_REG,
	ARM946E_CP15_DATA_TCM_REGION_REG,
	ARM946E_CP15_INST_TCM_REGION_REG,
	ARM946E_CP15_PID_REG,
	ARM946E_CP15_TAG_BIST_CONTROL_REG,
	ARM946E_CP15_TCM_BIST_CONTROL_REG,
	ARM946E_CP15_CACHE_RAM_BIST_CONTROL_REG,
	ARM946E_CP15_INST_TAG_BIST_ADDR_REG,
	ARM946E_CP15_INST_TAG_BIST_GENERAL_REG,
	ARM946E_CP15_DATA_TAG_BIST_ADDR_REG,
	ARM946E_CP15_DATA_TAG_BIST_GENERAL_REG,
	ARM946E_CP15_INST_TCM_BIST_ADDR_REG,
	ARM946E_CP15_INST_TCM_BIST_GENERAL_REG,
	ARM946E_CP15_DATA_TCM_BIST_ADDR_REG,
	ARM946E_CP15_DATA_TCM_BIST_GENERAL_REG,
	ARM946E_CP15_INST_CACHE_RAM_BIST_ADDR_REG,
	ARM946E_CP15_INST_CACHE_RAM_BIST_GENERAL_REG,
	ARM946E_CP15_DATA_CACHE_RAM_BIST_ADDR_REG,
	ARM946E_CP15_DATA_CACHE_RAM_BIST_GENERAL_REG,
	ARM946E_CP15_TEST_STATE_REG,
	ARM946E_CP15_CACHE_DEBUG_INDEX_REG,
	ARM946E_CP15_INST_TAG_WRITE_REG,
	ARM946E_CP15_DATA_TAG_WRITE_REG,
	ARM946E_CP15_INST_CACHE_WRITE_REG,
	ARM946E_CP15_DATA_CACHE_WRITE_REG,
	ARM946E_CP15_TRACE_CONTROL_REG,
	ARM946E_CP15_DCACHE_LN_INVALIDATE_REG,
	ARM946E_CP15_REGISTERS_NUM
} EARM946ECP15Regs;

// General definitions
typedef enum{
 ARM946E_MPU_NO_REGION =-1,
 ARM946E_MPU_REGION_0 = 0,
 ARM946E_MPU_REGION_1,
 ARM946E_MPU_REGION_2,
 ARM946E_MPU_REGION_3,
 ARM946E_MPU_REGION_4,
 ARM946E_MPU_REGION_5,
 ARM946E_MPU_REGION_6,
 ARM946E_MPU_REGION_7,
 ARM946E_MPU_REGIONS_NUM
}EMpuRegionsId;

typedef enum{
	ARM946E_MPU_REGION_DISABLE,
	ARM946E_MPU_REGION_ENABLE
}EMpuRegionMode;


typedef struct{
	EMpuRegionsId regionId;
	UINT_T baseAddress;
	UINT_T regionSize;
	BOOL   isICachable;
	BOOL   isDCachable;
	BOOL   isBufferable;
	CMpuRegionAttributes attributes;
	EMpuRegionMode regionEnableDisable;
}Arm946eMpuMemoryRegion;

typedef union
{
	struct
	{
		UINT_T pmuEnable:1;
		UINT_T :1 ; // reserved
		UINT_T dCacheEnable:1 ;
		UINT_T :4 ;	// reserved
		UINT_T bigEndian:1;
		UINT_T :3 ;	// reserved
		UINT_T bpuEnable:1;
		UINT_T iCacheEnable:1;
		UINT_T alternateVectorSelect:1;
		UINT_T roundRobinReplacement:1;
		UINT_T diableLoadingTIBT:1;
		UINT_T dTCMEnable:1;
		UINT_T dTCMLoadMode:1;
		UINT_T iTCMEnable:1;
		UINT_T iTCMLoadMode:1;
		UINT_T :12; // reserved
	}bits;

	UINT_T all;
} arm946eControlRegU;

typedef union
{
	struct
	{
		UINT_T regionEnableDisable:1;
		UINT_T regionSize:5 ;
		UINT_T :6; // reserved
		UINT_T regionBaseAddress:20 ;
	}bits;

	UINT_T all;
}_arm946eProtectionRegionBaseAndSizeReg_U;


extern UINT32 OBM_asm_Arm9WriteCP15_DS(UINT32, UINT32);
extern UINT32 OBM_asm_Arm9ReadCP15_DS(UINT32);
extern void CPUCleanDCacheLine(UINT32);
extern void CPUInvalidateDCacheLine(UINT32);
extern void L1_dcache_cleaninvalid_all(void);
extern void L1_dcache_clean_flush_all();
extern void FlushCache946();
extern void FlushIcache();
extern void CleanDcache();

void OBM_MPUCache_Init();
void OBM_Flush();
void OBM_MPUCache_Disable();

#endif
 
