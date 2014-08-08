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
**	 xip.h
******************************************************************************//******************************************************************************
 *
 *  (C)Copyright 2005 - 2011 Marvell. All Rights Reserved.
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
 
 ******************************************************************************
**
**  FILENAME:	xip.h
**
**  PURPOSE: 	Contains XIP related flash constants
**
******************************************************************************/
#ifndef __XIP_H__
#define __XIP_H__

#include "general.h"
#include "Flash.h"

#define NUM_XIP_DEVICES_SUPPORTED	2	//update each time adding a new XIP device
#define MAX_ERASE_BLOCK_REGIONS		2	//update if supporting a device with more than this
#define MAX_PROT_LOCK_REG_FIELDS	2	//currently only two different protection register types
#define READQUERYCMD               	0x0098	//universal read query command
#define CMD_SET_ID_SIBLEY			0x0200
#define CMD_SET_ID_TYAX				0x0001
#define MIN_PAGE_SIZE				0x400  //for Sibley, the smallest 'object mode programmable' independent regions is 1k
//currently supported XIP commands
typedef struct
{
	unsigned short CommandSetID;
	unsigned short ReadArrayCmd;
	unsigned short ReadIdentifierCmd;
	unsigned short ReadCFICmd;	
	unsigned short ReadStatusRegisterCmd;
	unsigned short ClearStatusRegisterCmd;
	unsigned short WordProgramCmd;
	unsigned short BufferedProgramCmd;
	unsigned short BufferedProgramConfirmCmd;
	unsigned short BlockEraseSetupCmd;
	unsigned short BlockEraseConfirmCmd;
	unsigned short LockBlockSetupCmd;
	unsigned short LockBlockCmd;
	unsigned short UnlockBlockCmd;
	unsigned short ProgramProtectionRegCmd;
}XIP_COMMANDS_T, *P_XIP_COMMANDS_T;

//xip erase region info
typedef struct
{
	unsigned int num_blocks;
	unsigned int block_size;
	unsigned int region_size;
	unsigned int region_offset;
}XIP_ERASE_BLOCK_T, *P_XIP_ERASE_BLOCK_T;

//xip OTP field info
typedef struct
{
	unsigned short lock_reg_addr;
	unsigned short prot_reg_addr;
	unsigned short prot_reg_size;
	unsigned short num_prot_regs;
}XIP_PROT_LOCK_REGION_T, *P_XIP_PROT_LOCK_REGION_T;

//complete XIP Properties structure
typedef struct 
{
	unsigned int			BaseAddr;
	unsigned int			WriteBufferSize;
	P_XIP_COMMANDS_T		pXIPcmds;
	unsigned short			NumEraseRegions;
	XIP_ERASE_BLOCK_T		XIPEraseRegions[MAX_ERASE_BLOCK_REGIONS];
	XIP_PROT_LOCK_REGION_T	XIPProtLockRegisters[MAX_PROT_LOCK_REG_FIELDS];
}XIP_Properties_T, *P_XIP_Properties_T;

#define DeviceReadyStatus_16        0x0080
#define EraseSuspendStatus_16       0x0040
#define EraseErrorStatus_16		 	0x0020
#define ProgramErrorStatus_16		0x0010
#define VppErrorStatus_16		    0x0008
#define ProgramSuspendStatus_16	    0x0004
#define BlockLockErrorStatus_16    	0x0002

// Common Flash Interface address offsets of entry values
//	note: all values are shifted left 1 from spec 
#define CFI_Q					0x20
#define CFI_R					0x22
#define CFI_VID_lower			0x26
#define CFI_VID_upper			0x28
#define CFI_QTTable_lower		0x2A
#define CFI_QTTable_upper		0x2C
#define CFI_DEVICE_SIZE			0x4E
#define CFI_DEVICE_WIDTH_lower	0x50
#define CFI_DEVICE_WIDTH_upper	0x52
#define CFI_WRITE_BUFFER_lower	0x54
#define CFI_WRITE_BUFFER_upper	0x56
#define CFI_ERASE_REGIONS		0x58
//offsets from offsets
// first ones are offsets from CFI_ERASE_REGIONS
#define CFI_ERASE_BLK_lower		0x2
#define CFI_ERASE_BLK_upper		0x4
#define CFI_ERASE_SZ_lower		0x6
#define CFI_ERASE_SZ_upper		0x8
// offset from Intel Specific Extended Query Table
#define CFI_PROTECTION_REGION	0x1C	//number of protection register fields
#define CFI_PROT1_ADDR_lower	0x1E	//addr of lock register (lsB)
#define CFI_PROT1_ADDR_upper	0x20	//addr of lock register (msB)
#define CFI_PROT1_UNWRITABLE_SZ	0x22	//size that is unwriteable
#define CFI_PROT1_WRITABLE_SZ	0x24	//size that is writable
#define CFI_PROT2_ADDR_b0		0x26	//addr of lock register (lsB)
#define CFI_PROT2_ADDR_b1		0x28
#define CFI_PROT2_ADDR_b2		0x2A
#define CFI_PROT2_ADDR_b3		0x2C	//addr of lock register (msB)
#define CFI_PROT2_TOTAL_lower	0x34	//total reg. in this group (lsB)
#define CFI_PROT2_TOTAL_upper	0x36	//total reg. in this group (msb)
#define CFI_PROT2_WRITABLE_SZ	0x38	//2^n bytes per reg
#define XIP_DEFAULT_PART		0

//Generic Flash routines
UINT_T InitializeXIPDevice(UINT8_T FlashNum, UINT8_T* P_DefaultPartitionNum);
UINT_T ReadFromXIP(UINT_T offset, UINT_T Destination, UINT_T size);
UINT_T WriteToXIP(UINT_T offset, UINT_T Source, UINT_T size);
UINT_T Erase_XIP (UINT_T offset, UINT_T size);
UINT_T WriteToXIP_OTP(UINT_T offset, UINT_T data, UINT_T size);
UINT_T ReadXIP_OTP(UINT_T offset, UINT_T data, UINT_T size);
UINT_T LockXIP_OTP(void);
//XIP specific workhorses
UINT_T CFIQuery(void);
P_XIP_Properties_T GetXIPProperties(void);
UINT_T ProgramXIPBlock(UINT_T BufferProgramAddr, UINT_T source, UINT_T size);
UINT_T Program16bitWord(UINT_T WordProgramAddr, UINT_T WordToProgram);
UINT_T FindEraseRegion(UINT_T offset);
UINT_T EraseBlock(UINT_T flashoffset);
UINT_T FindOTPField(UINT_T offset, UINT_T *addr, UINT_T *amt_avail);

#endif