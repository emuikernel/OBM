/******************************************************************************
**
**  COPYRIGHT (C) 2002, 2003 Intel Corporation.
**
**  This software as well as the software described in it is furnished under
**  license and may only be used or copied in accordance with the terms of the
**  license. The information in this file is furnished for informational use
**  only, is subject to change without notice, and should not be construed as
**  a commitment by Intel Corporation. Intel Corporation assumes no
**  responsibility or liability for any errors or inaccuracies that may appear
**  in this document or any software that may be provided in association with
**  this document.
**  Except as permitted by such license, no part of this document may be
**  reproduced, stored in a retrieval system, or transmitted in any form or by
**  any means without the express written consent of Intel Corporation.
**
**  FILENAME:	Flash.h
**
**  PURPOSE: 	Holds all flash related definitions
**
******************************************************************************/
#ifndef __Flash_H__
#define __Flash_H__

#include "general.h"
#include "Typedef.h"

#define ONE_KB	1024
#define TWO_KB	2048
#define TWO_KB_MASK ( TWO_KB-1 )

#define MAX_FLASH_PAGES 1024

#define HAMMING_MAX_BITS_ECC_CORRECTABILITY 1

typedef enum {ECC_DISABLE, ECC_HAMMING, ECC_BCH, ECC_BCH_1K, ECC_CHIP}   ECC_MODE;


// Flash Types
typedef enum
{
	CS0_XIP_FLASH	= 1,
	CS2_XIP_FLASH	= 2,
	NAND_FLASH		= 3,
	ONENAND_FLASH	= 4,
	MSYS_DOC_FLASH	= 5,
	SDMMC_FLASH		= 6,
	SPI_FLASH		= 7
} FlashType_T;

// Flash Fuses
typedef enum
{
	NO_FLASH_P				= 0,
	HSI_FLASH_P				= 1,
	ONENAND_FLASH_P			= 2,
	CS0_XIP_FLASH_P			= 3,
	NAND_FLASH_X16_HM_P		= 4,
	CS2_XIP_SIBLEY_P		= 5,
	NAND_FLASH_X8_HM_P		= 6,
	SDMMC_FLASH_OPT0_P		= 7,		// Used to be CS2 Intel XIP
	SDMMC_FLASH_OPT1_P		= 8,
	SDMMC_FLASH_OPT2_P		= 9,
	SPI_FLASH_P				= 10,
	SDMMC_FLASH_OPT3_P		= 11, 		// Reserved for eMMC Alternate Boot Mode from a platform specific Boot Partition, should not be included in the probe loop.
	SDMMC_FLASH_OPT4_P  	= 12,		// Reserved for eMMC Alternate Boot Mode from a platform specific Boot Partition, should not be included in the probe loop.
	NAND_FLASH_X16_BCH_P	= 13,
	NAND_FLASH_X8_BCH_P		= 14,

	NAND_FLASH_X16_BCH_OPT1_P = 16, // x16 NAND BCH (16 bit correction per 1K)
	NAND_FLASH_X8_BCH_OPT1_P  = 17, // x8 NAND BCH (16 bit correction per 1K)
	NAND_FLASH_X16_NO_ECC_P   = 18, // x16 NAND No ECC (ECC is done by flash device)
	NAND_FLASH_X8_NO_ECC_P    = 19, // x8 NAND No ECC (ECC is done by flash device)
    SDIO_P    = 20 //SDIO_P

} FlashProbeList_T;

// Flash Structure Types
typedef enum
{
	BOOT_FLASH			= 0,
	SAVE_STATE_FLASH	= 1
} FlashBootType_T;

typedef struct
{
	UINT Reserved			: 13;
	UINT SASize				: 11;	//size of spare area, in bytes, PER PAGE!!
	UINT UseSpareArea		: 1;	//tells driver whether to write spare area or not (1 = yes, 0 = no)
	UINT UseHwEcc			: 1;	//tells driver whether to use HW ECC during read and writes (1 = yes, 0 = no)
	UINT UseBBM				: 1;	//driver sets to determine if the flash should use BBM (1 = yes, 0 = no)
	UINT FlashNum			: 4;	//see above enum
	UINT FlashInitialized	: 1;	//1 = Flash initialized, 0 = Flash NOT initialized
} FlashAttributes_T;


typedef UINT_T (*InitFlash_F) (UINT8_T FlashNum, FlashBootType_T FlashBootType, UINT8_T* P_DefaultPartitionNum);
typedef UINT_T (*FinalizeFlash_F) (FlashBootType_T FlashBootType);
typedef UINT_T (*ReadFlash_F) (UINT_T FlashOffset, UINT_T LocalBuffer, UINT_T Size, FlashBootType_T FlashBootType);
typedef UINT_T (*WriteFlash_F) (UINT_T FlashOffset, UINT_T LocalBuffer, UINT_T Size, FlashBootType_T FlashBootType);
typedef UINT_T (*EraseFlash_F) (UINT_T FlashOffset, UINT_T Size, FlashBootType_T FlashBootType);
typedef UINT_T (*ResetFlash_F) (FlashBootType_T FlashBootType);
typedef void (*ChangePartition_F) (UINT_T PartitionNum, FlashBootType_T FlashBootType);
typedef UINT_T (*WriteOTP_F) (UINT_T FlashOffset, UINT_T LocalBuffer, UINT_T Size);
typedef UINT_T (*ReadOTP_F) (UINT_T FlashOffset, UINT_T LocalBuffer, UINT_T Size);
typedef UINT_T (*LockOTP_F) (void);
typedef UINT_T (*ConvertToLogicalAddr_F) (UINT_T FlashLocation, FlashBootType_T FlashBootType);

#if COPYIMAGESTOFLASH
//-----------------------------------------
// application defined helper func for API.
// in our case FindPackageInReserved()
//-----------------------------------------
typedef UINT_T (*DataSearchFunc_F)(UINT_T *pResult, void *pResvHeader, UINT_T pkgID);
typedef UINT_T (*DataSearchFindNext_F)(UINT_T *pResult);

//------------------------------------------------------------------------------------------------------------
//
// For DKB functionality, the M-sys device needs to be formatted into partitions of various sizes
// which are defined by the customer, and stored in the downloaded TIM by the DKB.
//
// This API func expects to recieve:
//              a ptr to a searchable database, in our case, this is a downloaded TIM module.
//              a pointer to a function which searches the database (FindPackageInReserved()), and
//              the application defined package ID of the applicable reserved area.
//
// If the searh function returns a non-zero value, the API func then casts the return value to pointer to
// a data structure which it understands, and uses the data to format and partition the device.
//------------------------------------------------------------------------------------------------------------
typedef UINT_T (*FlashInitAfterTimDownload_F) ( void *pResvHeader, DataSearchFunc_F, DataSearchFunc_F, DataSearchFindNext_F, UINT_T *myPkgIDs );

#endif



typedef struct
{
 	UINT_T BlockSize;
	UINT_T PageSize;
	UINT_T NumBlocks;
	UINT_T TimFlashAddress;
	UINT_T					StreamingFlash; 			// Means a flash where TIM and DATA are streamed using a single operation. (eMMC Alternate Boot)
	UINT_T					StagedInitRequired; 		// Flash requires staged initialization based on initial TIM data.
	FlashType_T FlashType;
   	UINT_T *ReservedPkgIDs;
	FlashAttributes_T FlashSettings;

    // Function pointers
	FinalizeFlash_F FinalizeFlash;
	ReadFlash_F ReadFromFlash;
	WriteFlash_F WriteToFlash;
	EraseFlash_F EraseFlash;
	ResetFlash_F ResetFlash;
	ChangePartition_F ChangePartition;
	WriteOTP_F WriteOTP;
	ReadOTP_F ReadOTP;
	LockOTP_F LockOTP;
	ConvertToLogicalAddr_F pConvertToLogicalAddr;

}
FlashProperties_T, *P_FlashProperties_T;

// Function Proto's
	UINT_T Configure_Flashes (UINT8_T FlashNum, FlashBootType_T FlashBootType);
	UINT_T Finalize_Flashes (FlashBootType_T FlashBootType);

	UINT_T ReadFlash(UINT_T FlashOffset, UINT_T buffer, UINT_T size, FlashBootType_T fbt);
	UINT_T WriteFlash(UINT_T FlashOffset, UINT_T buffer, UINT_T size, FlashBootType_T fbt);
	UINT_T EraseFlash(UINT_T FlashOffset, UINT_T size, FlashBootType_T fbt);
	UINT_T EraseAllFlash(FlashBootType_T fbt);
	UINT_T ResetFlash(FlashBootType_T fbt);
	UINT_T ReadOTP(UINT_T FlashOffset, UINT_T buffer, UINT_T size, FlashBootType_T fbt);
	UINT_T WriteOTP(UINT_T FlashOffset, UINT_T buffer, UINT_T size, FlashBootType_T fbt);
	UINT_T LockOTP(FlashBootType_T fbt);

	UINT_T GetBlockSize(FlashBootType_T fbt);
	UINT_T GetPageSize(FlashBootType_T fbt);
	UINT_T GetEraseRegionSize(FlashBootType_T fbt);
	UINT_T ConvertToLogicalAddr(UINT_T FlashLocation, FlashBootType_T FlashBootType);
#if COPYIMAGESTOFLASH	
	UINT_T GetUseSpareArea(FlashBootType_T fbt);
    UINT_T SetUseSpareArea( UINT_T bUseSpareArea, FlashBootType_T fbt);
    UINT_T GetSpareAreaSize(FlashBootType_T fbt);
    UINT_T GetUseHwEcc(FlashBootType_T fbt);
    UINT_T SetUseHwEcc( UINT_T bUseHwEcc, FlashBootType_T fbt);
	UINT_T FlashInitAfterDownload( void *pTim, DataSearchFunc_F SearchFunc, DataSearchFunc_F FindFirstFunc, DataSearchFindNext_F FindNextFunc, FlashBootType_T fbt  );
#endif
//Helper functions
	P_FlashProperties_T GetFlashProperties(FlashBootType_T FlashBootType);
	FlashBootType_T     GetCurrentFlashBootType();
    void                SetCurrentFlashBootType(FlashBootType_T FlashBootType);


#endif


