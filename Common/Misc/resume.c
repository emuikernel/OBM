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
 *  FILENAME:	resume.c
 *
 *  PURPOSE: 	Handle tasks relating to resume events
 *                  
******************************************************************************/

#include "Errors.h"
#include "resume.h"
#include "tim.h"
#include "PlatformConfig.h"
#include "BootROM.h"
#include "predefines.h"
#include "CIU.h"

static RESUME_PACKAGE RESUME_PACKAGE_h;

//-----------------------------------------------------------
//
//
//-----------------------------------------------------------
void SetupPowerRegisterPointers (P_RESUME_PARAM pResRegs)
{
	return; // Not used currently
}


//--------------------------------------------------------------
// ResumePackageCopy()
//
// Transfers Resume package details from package
// to TBR static data area.
//
//
//------------------------------------------------------------
void ResumePackageCopy( P_RESUME_PACKAGE pResumePkg )
{
  UINT_T i;
    
  // Copy the structure
  for (i=0; i < sizeof(RESUME_PACKAGE); i++)
	 ((UINT8_T *) pResumePkg)[i] = ((UINT8_T *) &RESUME_PACKAGE_h)[i];
	 
  // Make sure we have a valid Resume Package loaded
   
}

//--------------------------------------------------------------
// ResumePackageParamTransfer()
//
// Transfers Resume package details from package
// to TBR static data area.
//
//
//------------------------------------------------------------
void ResumePackageParamTransfer(P_TRANSFER_STRUCT pTS_h)
{
 UINT_T i;
  
 // Start by 0 out the paramters
 for (i=0; i<PLATFORM_MAX_RESUME_PARAMS; i++)
  	pTS_h->ResumeParam[i] = 0;
    
  // Did we find a resume package? 
 if(RESUME_PACKAGE_h.identifier != RESUMEID )
	return;

  // Copy the structure
 for (i=0; i < RESUME_PACKAGE_h.numResumeParams; i++)
 {
	if (RESUME_PACKAGE_h.ResumeArg[i].valueIsPointer)
		pTS_h->ResumeParam[i] = *((UINT32 *)RESUME_PACKAGE_h.ResumeArg[i].valuePointer);
	else
		pTS_h->ResumeParam[i] = RESUME_PACKAGE_h.ResumeArg[i].valuePointer;
 }
 return; 
}


//----------------------------------------------------------------------
//  TIMQuickBoot - This function is a place holder  
//	
//----------------------------------------------------------------------

UINT_T TIMQuickBoot_TTC(UINT_T* pTransferAddr, pTIM pTIM_h, P_RESUME_PARAM pResParam)
{
    UINT_T                      Retval = NoError;
    pWTP_RESERVED_AREA_HEADER   pWRAH = NULL;
    pOPT_RESUME_SET             pOPT_ResumeSet = NULL;
    pOPT_RESUME_DDR_INFO        pOPT_ResumeDDRInfo = NULL;

	// Find the OPT_RESUME_DDR_INFO Package.
    pWRAH = FindPackageInReserved (&Retval, pTIM_h, RESUMEBLID);
    if (Retval == NoError)
    {
		// Find the package.
		pOPT_ResumeSet = (pOPT_RESUME_SET) pWRAH;

		// Find the Package that has the address to where the OPT_RESUME_DDR_INFO is stored in DDR.		
        pOPT_ResumeDDRInfo = (pOPT_RESUME_DDR_INFO)pOPT_ResumeSet->ResumeDDRInfo.ResumeAddr;

		// Set BootROM's resume address
		*pTransferAddr = pOPT_ResumeDDRInfo->ResumeAddr;
        // Set up flag to indicate BootROM is resuming.
        pOPT_ResumeDDRInfo->ResumeFlag = ~pOPT_ResumeDDRInfo->ResumeFlag;

		// TBD. No use for this right now.
		// SetupPowerRegisterPointers (pResRegs);
	}

	// TBD. No use for this right now.
	RESUME_PACKAGE_h.identifier = NULL;

	// Not Supported
	return TRUE;
}

UINT_T QuickBoot_NotSupported(UINT_T* pTransferAddr, pFUSE_SET pFuses, P_RESUME_PARAM pResParam )
{
  //Not Supported for ASPEN 
  return FALSE;
}

#if BOOTROM
void CheckHibernate(pFUSE_SET pFuses, pTIM pTIM_h)
{
    UINT_T                      Retval = NoError;
    pWTP_RESERVED_AREA_HEADER   pWRAH = NULL;
    pOPT_RESUME_SET             pOPT_ResumeSet = NULL;
    pOPT_RESUME_DDR_INFO        pOPT_ResumeDDRInfo = NULL;


	// Find the OPT_RESUME_DDR_INFO Package.
    pWRAH = FindPackageInReserved (&Retval, pTIM_h, RESUMEBLID);
    if (Retval == NoError)
    {
		// Find the package.
		pOPT_ResumeSet = (pOPT_RESUME_SET) pWRAH;

		// Find the Package that has the address to where the OPT_RESUME_DDR_INFO is stored in DDR.		
        pOPT_ResumeDDRInfo = (pOPT_RESUME_DDR_INFO)pOPT_ResumeSet->ResumeDDRInfo.ResumeAddr;

        if( pOPT_ResumeDDRInfo->ResumeFlag == RESUME_FLAG_MASK )
        {
  			// It looks like we can resume.
			pFuses->bits.Resume = 1;
  
        }
    }
}
#endif


#if RVCT 
__asm void getCPUMode(UINT_T *CPU_mode, UINT_T *ID_mask )
{
	mrc	p15, 0, r3, c0, c0, 0
	ldr r2, [r1];
	and r2, r3, r2
	str	r2, [r0]	
}

#else
#if LINUX_BUILD
void getCPUMode(UINT_T *CPU_mode, UINT_T *ID_mask )
/* get CPU mode */
{
	asm ( 
		"mrc	p15, #0, r3, c0, c0, #0\n\t"
		"ldr r2, [r1]\n\t"
		"and r2, r3, r2\n\t"
		"str	r2, [r0]"
	);
}

#else
void getCPUMode(UINT_T *CPU_mode, UINT_T *ID_mask )
/* get CPU mode */
{
	__asm { 
		mrc	p15, 0, r3, c0, c0, 0
		ldr r2, [r1];
		and r2, r3, r2
		str	r2, [r0]
	}
}
#endif
#endif
