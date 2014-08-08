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
 
 ******************************************************************************
 *
 *
 *  FILENAME:	DDR_Init.h
 *
 *  PURPOSE: 	Header for DDR_Init.c
 *
******************************************************************************/

#ifndef __DDR_CFG_H__
#define __DDR_CFG_H__

#include "RegInstructions.h"
#include "tim.h"
#include "misc.h"
#include "mcu_extras.h"

//
// General operation guidelines
// Init, MemTest, IgnoreTimeOuts are flags
//		and when set to a 0b1 it means  perform operation
//		when set to 0b0 it means skip operation.
// MemTestStartAddr, MemTestSize, and Loops are parameters for controlling
//      operations when enabled.

typedef struct{
	unsigned int Init;
	unsigned int MemTest;
	unsigned int MemTestStartAddr;
	int MemTestSize;
	int Loops;
	unsigned int IgnoreTimeOuts;
}DDR_OPS_S, *pDDR_OPS_S;
//
//Prototypes 
//
unsigned int ConfigureDDRMemory(pDDR_PACKAGE DDR_PID); 
unsigned int ProcessDDROps(pDDR_OPERATION pDDR_OPS, unsigned int Num, pDDR_OPS_S Operations);
unsigned int CheckAndConfigureDDR(pTIM pTIM_h, pFUSE_SET pFuses);
#endif