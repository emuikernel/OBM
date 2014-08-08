/******************************************************************************
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
 *
 *
 *  FILENAME:	RegInstructions.h
 *
 *  PURPOSE: 	Header for RegInstructions.c
 *
******************************************************************************/


#include "tim.h"
#include "Errors.h"
#include "misc.h"
#include "ProtocolManager.h"
#include "PlatformConfig.h"

#define MAX_SCRATCH_MEMORY_ID	3  // define max of 4 Scratch Memory locations 

//Protoypes
void WriteInstruction(UINT_T Address, UINT_T Value);
void ReadInstruction(UINT_T Address, UINT_T  NumReads);
UINT_T WaitForBitSetInstruction(UINT_T Address, UINT_T Mask, UINT_T Timeout, UINT_T IgnoreTimeoutFlag);
UINT_T  WaitForBitClearInstruction(UINT_T Address, UINT_T Mask, UINT_T Timeout, UINT_T IgnoreTimeoutFlag);
void AndInstruction(UINT_T Address, UINT_T  Value);
void OrInstruction(UINT_T Address, UINT_T  Value);
UINT_T  ProcessInstructions(pINSTRUCTION_S pInstructions, UINT_T Num, UINT_T TimeoutFlag);
UINT_T Set_Bitfield_Instruction (UINT_T Address, UINT_T Mask, UINT_T Value);
UINT_T Wait_For_Bit_Pattern_Instruction (UINT_T Address, UINT_T Mask, UINT_T Value, UINT_T Timeout, UINT_T IgnoreTimeoutFlag);
UINT_T Test_If_Zero_And_Set (UINT_T TestAddress, UINT_T TestMask,UINT_T SetAddress, UINT_T SetMask,	UINT_T Value);
UINT_T* GetSMPtr(UINT_T ScratchMemoryId);
UINT_T Load_SM_Addr_Instruction (UINT_T ScratchMemoryId, UINT_T Address);
UINT_T Store_SM_Addr_Instruction (UINT_T ScratchMemoryId, UINT_T Address);
UINT_T Load_SM_Val_Instruction (UINT_T ScratchMemoryId, UINT_T Value);
UINT_T Mov_SM_SM_Instruction(UINT_T DestinationScratchMemoryId,UINT_T SourceScratchMemoryId);
UINT_T Rshift_SM_Val_Instruction(UINT_T ScratchMemoryId, UINT_T Value);
UINT_T Lshift_SM_Val_Instruction(UINT_T ScratchMemoryId, UINT_T Value);
UINT_T And_SM_Val_Instruction (UINT_T ScratchMemoryId, UINT_T Value);
UINT_T Or_SM_Val_Instruction (UINT_T ScratchMemoryId, UINT_T Value);
UINT_T And_SM_SM_Instruction(UINT_T DestinationScratchMemoryId,UINT_T SourceScratchMemoryId);
UINT_T Or_SM_SM_Instruction(UINT_T DestinationScratchMemoryId, UINT_T SourceScratchMemoryId); 
UINT_T Test_SM_If_Zero_And_Set (UINT_T TestScratchMemoryId, UINT_T TestMask, UINT_T SetScratchMemoryId, UINT_T SetMask,	UINT_T Value );
UINT_T Test_SM_If_Not_Zero_And_Set (UINT_T TestScratchMemoryId, UINT_T TestMask, UINT_T SetScratchMemoryId, UINT_T SetMask,	UINT_T Value );

