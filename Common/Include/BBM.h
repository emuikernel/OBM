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
**	 BBM.h
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
**  FILENAME:	BBM.h
**
**  PURPOSE: 	Holds all flash related definitions
**
******************************************************************************/
#ifndef __BBM_H__
#define __BBM_H__

#include "Typedef.h"

#define MBBT_VERSION_1	0x31303031

#define READ_DISTURB_LIST_ID 0x52444953	//'RDIS'

// Indicates the state of the BBT in volatile memory
typedef enum
{
    BBT_INVALID     = 1,
    BBT_UNCHANGED   = 2,
    BBT_CHANGED     = 3,
    BBT_ERASED      = 4
} ReloState_T;

// BBT Types
typedef enum
{
	BBT_TYPE_NA			= 0x00000000,		//Runtime BBT not being used
	BBT_TYPE_LEGACY		= 0x0000524E,		//Legacy version
	BBT_TYPE_MBBT		= 0x4D424254,		//"MBBT" Marvell Bad Block Table (NOTE: used as ID for factory AND runtime MBBT)
	BBT_TYPE_MBBT_FACT	= 0x46616374,		//"Fact" Factory Bad Block Table (sub type of the MBBT)
	BBT_TYPE_MBBT_RUN	= 0x52756E74,		//"Runt" Runtime Bad Block Table (sub type of the MBBT)
	BBT_TYPE_WNCE		= 0x574E4345,		//"WNCE" WinCE Bad Block Table
	BBT_TYPE_LINX		= 0x4C695E78		//"Linx" Linux Bad Block Table
} BBT_TYPE;

//Relocation Pairs - same for MBBT, FBBT, and Legacy BBT
typedef struct {
	USHORT	From;
	USHORT	To;
} ReloPair_T, *P_ReloPair_T;


// Marvell BBT Structure
typedef struct {
	UINT_T	Identifier;
	UINT_T	Version;
	UINT_T	Type;
	UINT_T	Reserved;
	UINT_T	PartitionID;
	UINT_T	NumRelo;
	UINT_T	BBTLocation;
	UINT_T	Reserved1;		//for now, the upper 32bits of 64bit address are reserved
	UINT_T	BackupBBTLoc;
	UINT_T	Reserved2;		//for now, the upper 32bits of 64bit address are reserved
	ReloPair_T Entry[1]; //need to make it an array
} MBBT_Table_T, *P_MBBT_Table_T;

// Factory BBT Structure
typedef struct {
	UINT_T	Identifier;
	UINT_T	Version;
	UINT_T  Type;
	UINT_T  Reserved0;
	UINT_T	Reserved1;
	UINT_T	NumRelo;
	UINT_T	BBTLocation;
	UINT_T	Reserved2;		//for now, the upper 32bits of 64bit address are reserved
	UINT_T	Reserved3;
	UINT_T	Reserved4;
	USHORT	BadBlocks[1]; //need to make it an array
} FBBT_Table_T, *P_FBBT_Table_T;


// Legacy BBT Structure
typedef struct
{
	USHORT			Header;
	USHORT			NumReloc;
   ReloPair_T		Relo[2];
} ReloTable_T, *P_ReloTable_T;


#if RVCT
#pragma anon_unions
#endif
//Read Disturb entries
typedef struct
{
	union
	{
		unsigned int value;
		struct
		{
			USHORT block;
			USHORT partition;
		};
	};
} ReadDisturbEntry_T, *P_ReadDisturbEntry_T;

// Read Distrub structure to keep track of read disturbed blocks
typedef struct
{
	UINT_T RDID;	//Read Disturb ID
	UINT_T num_entries;
	ReadDisturbEntry_T entries[10];
} ReadDisturbList_T, *P_ReadDisturbList_T;

#endif
