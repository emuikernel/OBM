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
******************************************************************************/

#ifndef __SDHC2_H__
#define __SDHC2_H__

#include "misc.h"
#include "sdhc2_controller.h"
#include "SD.h"

// Prototypes
UINT_T MM4_CardInit(UINT_T BaseAddress, UINT_T InterruptMask);

// Helper / Internal
UINT MM4_IDCard(P_SDMMC_Properties_T pSDMMCP, UINT_T *pControllerVoltage);
UINT_T MM4SetBusWidth();
UINT_T MM4HighSpeedTiming();
UINT_T MM4_MMCReadEXTCSD (UINT_T *pBuffer);
UINT_T MM4_CheckVoltageCompatibility(P_SDMMC_Properties_T pSDMMCP, UINT_T ControllerVoltage);
UINT_T MM4_SetControllerVoltage (P_MM4_SDMMC_CONTEXT_T pContext);
UINT_T MM4_CheckCardStatus(P_SDMMC_Properties_T pSDMMCP, UINT_T R1_Resp_Match, UINT_T Mask);
UINT_T MM4_Interpret_Response(P_SDMMC_Properties_T pSDMMCP, UINT_T ResponseType, UINT_T ResponseTimeOut);
void MM4_PrepareMMCContext (P_SDMMC_Properties_T pSDMMCP, UINT_T BaseAddress);
UINT_T MM4_SelectSlot(UINT_T slot);
void MM4_ISR(P_SDMMC_Properties_T pSDMMCP);
void MM4_SendDataCommand (P_SDMMC_Properties_T pSDMMCP,
                  UINT_T Cmd,
                  UINT_T  Argument,
                  UINT_T BlockType,
                  UINT_T DataDirection,
                  UINT_T  ResType);

void MM4_SendDataCommandNoAuto12 (P_SDMMC_Properties_T pSDMMCP,
                  UINT_T Cmd,
                  UINT_T Argument,
                  UINT_T BlockType,
                  UINT_T DataDirection,
                  UINT_T ResType);

void MM4_SendSetupCommand(P_SDMMC_Properties_T pSDMMCP,
                  UINT_T Cmd,
                  UINT_T CmdType,
                  UINT_T Argument,
                  UINT_T ResType);

UINT_T MM4_MMCAlternateBootMode(UINT_T BaseAddress, UINT_T InterruptMask);
UINT_T MM4_MMCAlternateBootModeReadBlocks();
UINT_T MM4_ReadBlocks();
UINT_T MM4_WriteBlocks(void);
UINT_T MM4_EraseBlocks();
UINT_T MM4_CardShutdown();
void MM4_WriteFifo(P_SDMMC_Properties_T pSDMMCP);
void MM4_ReadFifo(P_SDMMC_Properties_T pSDMMCP);
void MM4_SendStopCommand(P_SDMMC_Properties_T pSDMMCP);
UINT_T MM4SwitchPartition(UINT_T PartitionNumber);
UINT_T MM4_WaitForOperationComplete(UINT_T TimeOutMillisec );
void MMCClocks(	int Clks, unsigned int nCmdState_period, GPIO_CFG_T *pMMCClkCfg, GPIO_CFG_T *pMMCCmdCfg	);

#if MMC_DEBUG
UINT_T MM4SwitchPartitionForAlternateBootMode();
UINT_T TestSamsungSetBootPartition();
#endif

#endif

