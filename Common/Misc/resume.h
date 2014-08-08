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
**
**  FILENAME:	Resume.h
**
**  PURPOSE: 	Resume definitions for BootROM
**                  
******************************************************************************/
#ifndef __RESUME_H__
#define __RESUME_H__

#include "Typedef.h"
#include "tim.h"
#include "general.h"

#define PLATFORM_ISRAM_RESUME_PKT_LOCATION ISRAM_IMAGE_LOAD_BASE
#define PLATFORM_ISRAM_MASK ISRAM_PHY_ADDR
#define PLATFORM_MAX_RESUME_PARAMS 4
#define ARM926ID 0x00009260

typedef struct
{
  UINT32 valueIsPointer;
  UINT32 valuePointer;
} RESUME_ARG, *P_RESUME_ARG;

typedef struct
{
  UINT32 identifier;
  UINT32 packetSize;
  UINT32 resumeValue;
  UINT32 resumeValueIsPointer;
  UINT32 reserved0;
  UINT32 reserved1;
  UINT32 reserved2;
  UINT32 numResumeParams;
  RESUME_ARG ResumeArg[4];
} RESUME_PACKAGE, *P_RESUME_PACKAGE;

typedef struct 
{
 volatile unsigned long* pASCR;			// Application Subsystem Power Status/Configuration Register
 volatile unsigned long* pARSR;			// Application Subsystem Reset Status Register
 volatile unsigned long* pAD3SR;		// Application Subsystem Wake-Up from D3 Status Register
 volatile unsigned long* pAD3R;			// Application Subsystem D3 Configuration Register
 volatile unsigned long* pPSR; 			
 P_RESUME_PACKAGE pResumePkg;
} RESUME_PARAM, *P_RESUME_PARAM; 

typedef UINT_T (*pQuick_Resume_Function)(UINT_T* pTransferAddr, pFUSE_SET pFuses, P_RESUME_PARAM pResParam ); 
typedef UINT_T (*pTim_Resume_Function)(UINT_T* pTransferAddr, pTIM pTIM_h, P_RESUME_PARAM pResParam);
  
// Prototypes
void SetupPowerRegisterPointers (P_RESUME_PARAM pResRegs);
UINT_T TIMQuickBoot_TTC(UINT_T* pTransferAddr, pTIM pTIM_h, P_RESUME_PARAM pResParam);
UINT_T QuickBoot_NotSupported(UINT_T* pTransferAddr, pFUSE_SET pFuses, P_RESUME_PARAM pResParam );
void ResumePackageParamTransfer(P_TRANSFER_STRUCT pTS_h);
void getCPUMode(UINT_T *, UINT_T *);

#endif
