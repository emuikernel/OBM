/******************************************************************************
 *
 *  (C)Copyright 2005 - 2008 Marvell. All Rights Reserved.
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
 *  Use this file for BootLoader definitions
 */

#ifndef __bootloader_h
#define __bootloader_h

//////////////////////////////////////////////////////////////////////
// Include Boot and Download Mode libraries
//////////////////////////////////////////////////////////////////////
#include "BootMode.h"
#include "DownloadMode.h"
#include "PlatformConfig.h"
#include "timer.h"
#include "loadoffsets.h"
#include "tim.h"
#include "Flash.h"
#include "DDR_Cfg.h"
#include "Typedef.h"
#include "PMUA.h"

//////////////////////////////////////////////////////////////////////
// Constants
//////////////////////////////////////////////////////////////////////
#define TEMP_BUFFER_AREA DDR_TEMP_ERASE_BLOCK0_ADDR
#define MY_CONSUMER_ID TBRIDENTIFIER // OBMIDENTIFIER
#define MAX_NUMBER_OF_DATA_PAIRS 0x100		// Max number of data pairs in the Transfer Struct

#define SWD_VERSION_INFO	"SWDownloader 4.8.5.2 for Nezha MIFI\n" // need update this info for new release
#define OBM_VERSION_INFO	"OBM 4.7.2.14 for Nezha MIFI\n" // need update this info for new release
#define OBM_COMPILE_INFO	"-- " __DATE__ " - " __TIME__ " --\n"

//////////////////////////////////////////////////////////////////////
// Function Prototypes
//////////////////////////////////////////////////////////////////////
void SetupEnvironment(UINT_T *TIM_Address, P_TRANSFER_STRUCT pTS_h, UINT_T *startOfDayTimerVal, TIM *pTIM_h, pFUSE_SET pFuses, UINT_T *perform_wtm_ver_adv);
void ParseTransferStruct(UINT_T *TIM_Address, P_TRANSFER_STRUCT pTS_h, UINT_T* PlatformSettings, UINT_T *startOfDayTimerVal);
void FatalError(UINT_T ErrorCode);
void PerformTIMBasedSetup(pTIM pTIM_h, pFUSE_SET pFuses);
OPERATING_MODE_T DetermineOperatingMode(FUSE_SET *pFuses, pTIM pTIM_h);	
void FinalizeSetup(pFUSE_SET pFuses, pTIM pTIM_h );
void SetupTransferStruct(pTIM pTIM_h, UINT_T FusesValue);


//////////////////////////////////////////////////////////////////////
// Methods defined outside
//////////////////////////////////////////////////////////////////////
extern void EnableIrqInterrupts(void);
extern void DisableIrqInterrupts(void);
extern void TransferControl(UINT_T, UINT_T, UINT_T);
extern void SaveProgramState();

#endif