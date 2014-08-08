/******************************************************************************
**
** INTEL CONFIDENTIAL
** Copyright 2003-2004 Intel Corporation All Rights Reserved.
**
** The source code contained or described herein and all documents
** related to the source code (Material) are owned by Intel Corporation
** or its suppliers or licensors.  Title to the Material remains with
** Intel Corporation or its suppliers and licensors. The Material contains
** trade secrets and proprietary and confidential information of Intel
** or its suppliers and licensors. The Material is protected by worldwide
** copyright and trade secret laws and treaty provisions. No part of the
** Material may be used, copied, reproduced, modified, published, uploaded,
** posted, transmitted, distributed, or disclosed in any way without Intel's
** prior express written permission.
**
** No license under any patent, copyright, trade secret or other intellectual
** property right is granted to or conferred upon you by disclosure or
** delivery of the Materials, either expressly, by implication, inducement,
** estoppel or otherwise. Any license under such intellectual property rights
** must be express and approved by Intel in writing.
**
**  FILENAME: nand_flash.h
**
**  PURPOSE:  Nand Flash specific Header File
**
******************************************************************************/

#ifndef __NAND_FLASH_H__
#define __NAND_FLASH_H__

#include "Typedef.h"
#include "Flash.h"
#include "xllp_dfc_defs.h"

//
//	Structure to keep track of NAND device parameters
//		To be initialized during the XllpDfcInit routine
//		then used by the work functions in xllp_dfc_support.c
//

#define FlashBusWidth16		16
#define FlashBusWidth8		8
#define NAND_DEFAULT_PART	0

typedef struct
{
 	unsigned int	PageSize; 
 	unsigned int	SpareAreaSize;
	unsigned int	BlockSize;
	unsigned int	NumOfBlocks;
	unsigned int	FlashBusWidth;
	unsigned int	PagesPerBlock;
	unsigned char	ManufacturerCode;
	unsigned char	FlashID;
	unsigned char	reserved[2];
 	DEVICE_TYPE		NAND_type;
	CMD_BLOCK		*device_cmds;
	ECC_MODE		ECCMode;
}NAND_Properties_T, *P_NAND_Properties_T;

//Generic Flash routines
UINT_T InitializeNANDDevice(UINT8_T FlashNumber, FlashBootType_T FlashBootType, UINT8_T* P_DefaultPartitionNum);
UINT_T FinalizeNANDDevice (FlashBootType_T FlashBootType);
UINT_T ReadNAND_nonDMA (UINT_T Address, UINT_T Buffer, UINT_T ReadAmount, FlashBootType_T FlashBootType);
UINT_T ReadNAND (UINT_T Address, UINT_T Destination, UINT_T ReadAmount, FlashBootType_T FlashBootType);
UINT_T WriteNAND (UINT_T Address, UINT_T LocalAddr, UINT_T WriteAmount, FlashBootType_T FlashBootType);
UINT_T EraseNAND (UINT_T flashoffset, UINT_T size, FlashBootType_T FlashBootType);
UINT_T ResetNAND (FlashBootType_T fbt);
UINT_T ScanNANDForBadBlocks(FlashBootType_T fbt);

//specific workhorses
P_NAND_Properties_T GetNANDProperties(void);
//UINT_T FindBlockInBBT(UINT_T BlockNum, P_RelocTable_T pBBT);
//UINT_T RelocateBlock(UINT_T Block, P_RelocTable_T pBBT);
#endif
