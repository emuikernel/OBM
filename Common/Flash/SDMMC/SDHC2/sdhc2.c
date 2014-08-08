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

#include "predefines.h"
#include "sdmmc_api.h"
#include "sdhc2.h"
#include "Errors.h"
#include "PlatformConfig.h"
#include "Interrupts.h"
#include "xllp_dfc_defs.h"

static MM4_SDMMC_CONTEXT_T MM4_Context;	// Only need one

/**********************************************************
*   MM4_MMCAlternateBootMode
*      This mode uses Alternate Boot Method mode for eMMC Partition 1 to stream in 
* 		It assumes 8 bit mode Operation
* 		High Speed Operations
* 		The address where images are streamed are defined in ISRAM_IMAGE_LOAD_BASE
* 		128 KB only will be loaded.
*   Input:
*	   none
*   Output:
*      none
*   Returns:
*      WTP recoginized Success/Fail return code
***********************************************************/
UINT_T MM4_MMCAlternateBootMode(UINT_T BaseAddress, UINT_T InterruptMask)
{
    UINT_T argument, cardStatus, resp, cmd55_resp;
	UINT_T attempts, controllervoltage;
	P_MM4_SDMMC_CONTEXT_T pContext;
	P_MM4_CNTL1 pMM4_CNTL1;
	P_MM4_SD_CFG_FIFO_PARAM pMM4_SD_CFG_FIFO_PARAM;
	P_MM4_SD_CE_ATA_1_2 pMM4_SD_CE_ATA_1_2;
	UINT_T startTime, endTime;
	UINT_T retval = NoError;

	// Initialize Flash Properties
	P_SDMMC_Properties_T pSDMMCP = GetSDMMCProperties();

    // Assign pContext and clock values
	MM4_PrepareMMCContext (pSDMMCP, BaseAddress);
	pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;
	pMM4_CNTL1 = (P_MM4_CNTL1) &pContext->pMMC4Reg->mm4_cntl1;
	pMM4_SD_CFG_FIFO_PARAM = (P_MM4_SD_CFG_FIFO_PARAM)&pContext->pMMC4Reg->mm4_sd_cfg_fifo_param;
	pMM4_SD_CE_ATA_1_2 = (P_MM4_SD_CE_ATA_1_2)&pContext->pMMC4Reg->mm4_sd_ce_ata_1_2;
	
	// enable ints moved out of ConfigureMMC to here
	//      because pending interrupts (left over from bootrom
	//      mmc activity) were kicking off before the context
	//		had enough info for the isr to service the request.
	// now pads have been set up, and the context has
	// enough information to service interrupts (specifically,
	// it has the mm4 base regs address). ok to enable ints.
	EnablePeripheralIRQInterrupt(InterruptMask);

	// Issue a full reset.
	MMC4FullSWReset(pContext);

    // Enable clocks to a higher speed
	// Check if High Speed is enabled in the fuses
	if(MMCHighSpeedTimingEnabled())
		retval = MM4HighSpeedTiming();
	else
		MMC4SetBusRate(pContext, MM4CLOCK12_5MHZRATE);

	// Set Read Response Timeout
	MMC4SetDataTimeout(pContext, CLOCK_27_MULT);

	// Unmask and Enable interrupts
	MMC4EnableDisableIntSources(pContext, ENABLE_INTS);

	// Set the controller voltage bits
	MM4_SetControllerVoltage(pContext);

	// Enable SDMA Mode based on support
#if MMC_SDMA_MODE
	pSDMMCP->SDMA_Mode = TRUE;
#else
	pSDMMCP->SDMA_Mode = FALSE;
#endif

	// Save off read Block Size, play it safe, for now hard code to 512 Bytes
	pSDMMCP->ReadBlockSize = HARD512BLOCKLENGTH;
	pSDMMCP->WriteBlockSize = HARD512BLOCKLENGTH;
	pSDMMCP->EraseSize = HARD512BLOCKLENGTH;
	pSDMMCP->pFlashP->BlockSize = pSDMMCP->WriteBlockSize;
	pSDMMCP->pFlashP->PageSize = pSDMMCP->ReadBlockSize;	 

	// Set the block length for the controller
	pContext->pMMC4Reg->mm4_blk_cntl = pSDMMCP->ReadBlockSize;

	// Switch to x8 mode.
	pMM4_CNTL1->ex_data_width = 1; 		// Move to 8-bit mode.

	// Enable BOOT ACK through the controller.
	pMM4_SD_CFG_FIFO_PARAM->boot_ack = 1;		

	// Enable capturing status for sending out 74 clock cycles.
	pMM4_SD_CE_ATA_1_2->mmc_card = 1; 
	pMM4_SD_CE_ATA_1_2->misc_int_en = 1;

	// Enable and start 74 clock cycles
	pMM4_SD_CFG_FIFO_PARAM->gen_pad_clk_cnt = 74;		// 74 clock cycles.	
	pMM4_SD_CFG_FIFO_PARAM->gen_pad_clk_on = 1;			// Start generating the clocks.

	// Wait for the 74 clock cycles to be generated.	
	startTime = GetOSCR0();
  	startTime = GetOSCR0();
  	do
  	{
		if (pMM4_SD_CE_ATA_1_2->misc_int)
			break;		  // 74 clock cycles have been generated.

		endTime = GetOSCR0();
		if (endTime < startTime)
			endTime += (0x0 - startTime);      

		if (OSCR0IntervalInMilli(startTime, endTime) >= 10)
			return SDMMCInitializationError;
  	}
  	while(1);

	// Set the State to READY
	pSDMMCP->State = READY;

	// Issue go pre-idle Add this back for V4.4 support.
	MM4_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD0, MM4_CMD_TYPE_NORMAL, MMC_CMD0_PRE_IDLE_ARGUMENT, MM4_NO_RES);
	retval = MM4_Interpret_Response(pSDMMCP, MMC_RESPONSE_NONE, 0x10);	 // No Response to this cmd.

	return retval;
}


/**********************************************************
*   MM4_CardInit
*      Initializes the inserted card
*   Input:
*	   none
*   Output:
*      none
*   Returns:
*      WTP recoginized Success/Fail return code
***********************************************************/
UINT_T MM4_CardInit(UINT_T BaseAddress, UINT_T InterruptMask)
{
    UINT_T argument, cardStatus, resp, cmd55_resp, Retval;
	UINT_T attempts, controllervoltage;
	P_MM4_SDMMC_CONTEXT_T pContext;

	// Initialize Flash Properties
	P_SDMMC_Properties_T pSDMMCP = GetSDMMCProperties();

    // Assign pContext and clock values
	MM4_PrepareMMCContext (pSDMMCP, BaseAddress);
	pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;

	// enable ints moved out of ConfigureMMC to here
	//      because pending interrupts (left over from bootrom
	//      mmc activity) were kicking off before the context
	//		had enough info for the isr to service the request.
	// now pads have been set up, and the context has
	// enough information to service interrupts (specifically,
	// it has the mm4 base regs address). ok to enable ints.
	EnablePeripheralIRQInterrupt(InterruptMask);

	// Issue a full reset.
	MMC4FullSWReset(pContext);

    // Enable and start clocks
	MMC4SetBusRate(pContext, MM4CLOCK200KHZRATE);

	// Set Read Response Timeout
	MMC4SetDataTimeout(pContext, CLOCK_27_MULT);

	// Unmask and Enable interrupts
	MMC4EnableDisableIntSources(pContext, ENABLE_INTS);

	// Enable SDMA Mode based on support
#if MMC_SDMA_MODE
	pSDMMCP->SDMA_Mode = TRUE;
#else
	pSDMMCP->SDMA_Mode = FALSE;
#endif

    Retval = MM4_IDCard(pSDMMCP, &controllervoltage);
	if (Retval != NoError)
		//return Retval;
		return SDMMCNotFound;

	// Set up State
	pSDMMCP->State = INITIALIZE;

   // SD and MMC joint functionality again
   // At this point we should have our OCR contents. See if they match the voltage range we choose for the controller
   Retval = MM4_CheckVoltageCompatibility(pSDMMCP, controllervoltage);
   if (Retval != NoError)
			return SDMMCInitializationError; // We couldn't find any cards

    //send CMD2 to get the CID numbers
    argument = NO_ARGUMENT;
	MM4_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD2, MM4_CMD_TYPE_NORMAL, argument, MM4_RT_R2 | MM4_136_RES);
	Retval = MM4_Interpret_Response(pSDMMCP, MMC_RESPONSE_R2, 0x10);
	if (Retval != NoError)
		return SDMMCInitializationError;

	// Next its CMD3 to assign an RCA to the cards
    if (pSDMMCP->SD == XLLP_SD)
    {
     	argument = NO_ARGUMENT;
	 	MM4_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD3, MM4_CMD_TYPE_NORMAL, argument, MM4_RT_R6 | MM4_48_RES);
	 	Retval = MM4_Interpret_Response(pSDMMCP, MMC_RESPONSE_R6, 0x10);
    }
    else
    {
      	pSDMMCP->CardReg.RCA = pSDMMCP->CardReg.CID.SerialNum;
        argument = pSDMMCP->CardReg.RCA;
	 	MM4_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD3, MM4_CMD_TYPE_NORMAL, argument, MM4_RT_R1 | MM4_48_RES);
	 	Retval = MM4_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1, 0x10);
    }
	if (Retval != NoError)
		return SDMMCInitializationError;


	//send CMD13 to check the status of the card
    Retval = MM4_CheckCardStatus(pSDMMCP, 0x700, R1_LOCKEDCARDMASK);		// Make sure card is stdby mode
	if (Retval != NoError)
		return SDMMCInitializationError;

	// now we are beyond the point where some cards have subtle non-compliance issues with the spec.
	// for example, some cards leave the error bits from unrecognized commands (like 55 & 41) set
	// until this point - which is correct (since CMD3 was the first R1 type command). 
	// other cards clear them earlier, which is incorrect - but presents fewer initialization failures. 
	// at this point it is safe, and necessary, to begin rigorously examining all error status bits.
	pSDMMCP->StrictErrorCheck = 1;


	// Send CMD 9 to retrieve the CSD
    argument = pSDMMCP->CardReg.RCA;
 	MM4_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD9, MM4_CMD_TYPE_NORMAL, argument, MM4_RT_R2 | MM4_136_RES);
 	Retval = MM4_Interpret_Response(pSDMMCP, MMC_RESPONSE_R2, 0x100);

    // Save off some information from the CSD
	// There is no page size for SDMMC. However save some information so slot type functionality will work
	pSDMMCP->pFlashP->BlockSize = pSDMMCP->pFlashP->PageSize = pSDMMCP->ReadBlockSize;	 

    //send CMD7 to get card into transfer state
    argument = pSDMMCP->CardReg.RCA;
 	MM4_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD7, MM4_CMD_TYPE_NORMAL, argument, MM4_RT_R1 | MM4_RT_BUSY | MM4_48_RES_WITH_BUSY);	// fixme: from standby to transfer state should be an r1, not r1b
 	Retval |= MM4_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1B, 0x100);

    //send CMD13 to check the status of the card
    Retval |= MM4_CheckCardStatus(pSDMMCP, 0x900, R1_LOCKEDCARDMASK);		// Make sure card is transfer mode
	if (Retval != NoError)
		return SDMMCInitializationError;

	// CMD 16 Set Block Length
    argument = pSDMMCP->ReadBlockSize;
 	MM4_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD16, MM4_CMD_TYPE_NORMAL, argument, MM4_RT_R1 | MM4_48_RES);
 	Retval = MM4_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1, 0x100);
	// Set the block length for the controller
	pContext->pMMC4Reg->mm4_blk_cntl = argument;

	// Check if High Speed is enabled in the fuses
	if(MMCHighSpeedTimingEnabled())
		Retval = MM4HighSpeedTiming();
	else
		MMC4SetBusRate(pContext, MM4CLOCK12_5MHZRATE);


    //send CMD13 to check the status of the card
    Retval = MM4_CheckCardStatus(pSDMMCP, 0x900, R1_LOCKEDCARDMASK);		// Make sure card is transfer mode
	if (Retval != NoError)
		return SDMMCInitializationError;

	// Attempt to Increase Bus width
	Retval = MM4SetBusWidth();

    //send CMD13 to check the status of the card
    Retval = MM4_CheckCardStatus(pSDMMCP, 0x900, R1_LOCKEDCARDMASK);		// Make sure card is transfer mode
	if (Retval != NoError)
		return SDMMCInitializationError;

	// Set up State, Ready for Data transfers
	pSDMMCP->State = READY;

#if 0

//	Retval = MM4SwitchPartition(1);
	Retval = MM4SwitchPartitionForAlternateBootMode();
	Retval = MM4_MMCReadEXTCSD ((UINT_T*) (0xd1000000));

	Retval = SDMMC_WRITE (0, 0xd1000000, 0x200, BOOT_FLASH);
	Retval = SDMMC_READ (0, 0xd1000000, 0x200, BOOT_FLASH);
//	Retval = SDMMC_WRITE (0, 0xd1001000, 0x200, BOOT_FLASH);
	Retval = SDMMC_READ (0, 0xd1001000, 0x200, BOOT_FLASH);


//	Retval = SDMMC_ERASE (0, 0x1000, BOOT_FLASH);
//	Retval = SDMMC_WRITE (0, 0xd102a000, 0x200, BOOT_FLASH);
//	Retval = SDMMC_WRITE (0x200, 0xd102a000, 0x200, BOOT_FLASH);
//	Retval = SDMMC_READ (0x200, 0xd102a000, 0x200, BOOT_FLASH);
#endif

    return NoError;
}

/**********************************************************
*   MM4_IDCard
*      Identifies which type of card was inserted
*   Input:
*	   none
*   Output:
*      none
*   Returns:
*      WTP recoginized Success/Fail return code
***********************************************************/
UINT MM4_IDCard(P_SDMMC_Properties_T pSDMMCP, UINT_T *pControllerVoltage)
{
	// Local Variables
	P_MM4_SDMMC_CONTEXT_T pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;	// Assign our context value
	UINT_T startTime, endTime, elapsedSec;
	P_MM4_SD_CE_ATA_1_2 pMM4_SD_CE_ATA_1_2;
	UINT_T AnyError = 0;
	UINT_T argument = 0;
	UINT_T Retval;
	UINT_T HighCapacity = 0;
	UINT_T attempts = 0;
	UINT_T Ncr = 0x10;			// command response timeout.

	// Assign some context
	pMM4_SD_CE_ATA_1_2 = (P_MM4_SD_CE_ATA_1_2)&pContext->pMMC4Reg->mm4_sd_ce_ata_1_2;

	// Enable power
	*pControllerVoltage = MM4_SetControllerVoltage(pContext);
	pContext->pMMC4Reg->mm4_blk_cntl = 0;

	// Send CMD0 (GO_IDLE_STATE) to get card into idle state
	MM4_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD0, MM4_CMD_TYPE_NORMAL, argument, MM4_RT_NONE | MM4_NO_RES);
	AnyError += MM4_Interpret_Response(pSDMMCP, MMC_RESPONSE_NONE, Ncr);

	// Check for High Capacity Cards First
	do
	{
		// Start with SD
		pMM4_SD_CE_ATA_1_2->mmc_card = 0;

		// Try High Voltage range first:
		// Note: this is a valid command for SD cards in the idle state.
		//       for mmc cards in the idle state, this is not valid, so no reponse is generated.
		//       however, even some SD cards may miss this command, so that's why it is
		//       sent out twice. See SD spec, step 4 of figure 9 in card init and id section.
		argument = (SDVHS_2_7_TO_3_6 << SDVHSARGSHIFT) | SDVHSCHECKPATTERN;		// 2.7-3.6V Range
		MM4_SendSetupCommand(pSDMMCP, XLLP_SD_CMD8, MM4_CMD_TYPE_NORMAL, argument, MM4_RT_R7 | MM4_48_RES);

		// get the response (if any) to XLLP_SD_CMD8.
		MM4_Interpret_Response(pSDMMCP, MMC_RESPONSE_R7, Ncr);

		// Check for High Capacity Cards
		HighCapacity = (pSDMMCP->SD_VHS == argument);
	} while (!HighCapacity && ++attempts < 2);


	// Capture start time and default to SD
	startTime = GetOSCR0(); // Dummy read to flush potentially bad data
    startTime = GetOSCR0();
	pSDMMCP->SD = XLLP_SD;

	// First time, pass NULL argument to get back values card is compatible with
	// Send appropriate CMD Sequence to Identify the type of card inserted
	argument = 0;
	pSDMMCP->CardReg.OCR = 0;			// Make sure to clear out OCR.
	
	do
	{
		switch (pSDMMCP->SD)
		{
		 case XLLP_SD:		// Assume SD
			MM4_SendSetupCommand(pSDMMCP, XLLP_SD_CMD55, MM4_CMD_TYPE_NORMAL, 0, MM4_RT_R1 | MM4_48_RES);
			MM4_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1, Ncr);


			MM4_SendSetupCommand(pSDMMCP, XLLP_SD_ACMD41, MM4_CMD_TYPE_NORMAL, argument, MM4_RT_R3 | MM4_48_RES);
			MM4_Interpret_Response(pSDMMCP, MMC_RESPONSE_R3, Ncr);

			if (pSDMMCP->CardReg.OCR == 0)
			{
				pSDMMCP->SD = XLLP_MMC;
			}
			else
			{
				Retval = NoError;
			}
			break;
		 case XLLP_MMC:		// Assume MMC
			pMM4_SD_CE_ATA_1_2->mmc_card = 1; 
			MM4_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD1, MM4_CMD_TYPE_NORMAL, argument, MM4_RT_R3 | MM4_48_RES);
			AnyError += MM4_Interpret_Response(pSDMMCP, MMC_RESPONSE_R3, Ncr);		   

			if (pSDMMCP->CardReg.OCR == 0)
			{
				Retval = NotFoundError;
			}
			else
			{
				Retval = NoError;
			}
			break;
		}

		endTime = GetOSCR0();
		if (endTime < startTime)
			endTime += (0x0 - startTime);      
		elapsedSec = OSCR0IntervalInSec(startTime, endTime);
		argument = pSDMMCP->CardReg.OCR | 0x40000000;

	} while (((pSDMMCP->CardReg.OCR & 0x80000000) != 0x80000000) && (AnyError == 0) && (elapsedSec < 1));
	if ((pSDMMCP->CardReg.OCR & 0x80000000) != 0x80000000)
		return NotFoundError;
	
	// Assign Access Mode.
	if (pSDMMCP->CardReg.OCR & OCR_ACCESS_MODE_MASK)
		pSDMMCP->AccessMode = SECTOR_ACCESS;
	else
		pSDMMCP->AccessMode = BYTE_ACCESS;

	return Retval;
}

/***************************************************************
*   MM4SetBusWidth()
* 	Sets the Bus width highest bus width supported.
*   Input:
*   Output:
*   Returns: NoError, ReadError or NotSupportedError
*      
*****************************************************************/
UINT_T MM4SetBusWidth()
{
	UINT_T argument, Cmd;
	UINT8_T SDBusWidth;
	UINT_T OriginalBlkSize;
	MMC_CMD6_OVERLAY Cmd6;
	P_MM4_SDMMC_CONTEXT_T pContext;
	P_MM4_CNTL1 pMM4_CNTL1;
	P_MM4_BLK_CNTL pMM4_BLK_CNTL;
	UINT_T Retval = NoError;
	
	// Initialize Flash Properties
	P_SDMMC_Properties_T pSDMMCP = GetSDMMCProperties();

 	// Assign our context value
 	pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;
	pMM4_CNTL1 = (P_MM4_CNTL1) &pContext->pMMC4Reg->mm4_cntl1;

	// Check supported configurations first
	if (pSDMMCP->SD == XLLP_SD)
	{

		// Assume 1 bit Mode.
		SDBusWidth = SCRSD1BITMODE;

		/*	For now disable 4 bit SD in PIO mode until we can root cause this issue.

		// Issue ACMD51 to read in the SCR
		MM4_SendSetupCommand(pSDMMCP, XLLP_SD_CMD55, MM4_CMD_TYPE_NORMAL, pSDMMCP->CardReg.RCA, MM4_RT_R1 | MM4_48_RES);
		Retval = MM4_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1, 0x10);

		// Set up State
		pSDMMCP->State = READ;

		// This requires a transfer over the data lines.
		argument = NO_ARGUMENT;
		pMM4_BLK_CNTL = (P_MM4_BLK_CNTL) &pContext->pMMC4Reg->mm4_blk_cntl;
		OriginalBlkSize = pMM4_BLK_CNTL->xfr_blksz;
		pMM4_BLK_CNTL->blk_cnt = 1;
		pMM4_BLK_CNTL->xfr_blksz = 8;
		pSDMMCP->Trans.StartDiscardWords = 0;
		pSDMMCP->Trans.EndDiscardWords = 0; //(512 - 8) / 4;	// 126 words - SCR is only 2 words of data.
		pSDMMCP->Trans.TransWordSize = 2; // pSDMMCP->ReadBlockSize / 4; 		// Total Transfer Size including pre and post bytes
		pSDMMCP->Trans.LocalAddr = (UINT_T) &pSDMMCP->CardReg.SCR.SCR_VALUE[0];
		pSDMMCP->Trans.WordIndex = 0;									// Stores Index of Current write position

		MM4_SendDataCommandNoAuto12(pSDMMCP, XLLP_SD_ACMD51, argument, MM4_SINGLE_BLOCK_TRAN, MM4_CARD_TO_HOST_DATA, MM4_RT_R1 | MM4_48_RES);

		// Wait for the Read to Complete
		Retval = MM4_WaitForOperationComplete(1000);

		// Restore back to original blk size.		
		pMM4_BLK_CNTL->xfr_blksz = OriginalBlkSize; 		

		// Get the Card Response
		Retval = MM4_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1, 0x100);
		if ((Retval != NoError) || (pSDMMCP->CardReponse.R1_RESP != 0x920))
			MM4_SendStopCommand(pSDMMCP);			// Send a stop command
		pSDMMCP->State = READY;

	    //send CMD13 to check the status of the card
    	Retval |= MM4_CheckCardStatus(pSDMMCP, 0x900, R1_LOCKEDCARDMASK);		// Make sure card is transfer mode
		if (Retval != NoError)
			return SDMMC_SWITCH_ERROR;

		// Swap the byte ordering
		ReverseBytes ((UINT8_T*) pSDMMCP->CardReg.SCR.SCR_VALUE, 8);

		// Check do we support 4 bit mode?
		SDBusWidth = ((pSDMMCP->CardReg.SCR.SCR_VALUE[1] >> 16) & 0xF);

		if (SDBusWidth == SCRSD4BITMODE)
		{
			// Issue ACMD 6 to set the bus width
			MM4_SendSetupCommand(pSDMMCP, XLLP_SD_CMD55, MM4_CMD_TYPE_NORMAL, pSDMMCP->CardReg.RCA, MM4_RT_R1 | MM4_48_RES);
			Retval = MM4_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1, 0x10);
			MM4_SendSetupCommand(pSDMMCP, XLLP_SD_ACMD6, MM4_CMD_TYPE_NORMAL, SD_CMD6_4BITMODE, MM4_RT_R1 | MM4_48_RES);
			Retval = MM4_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1, 0x10);
		}
*/
	}
	else
	{
		// Issue CMD 6 to set BUS WIDTH bits in EXT_CSD register byte 183
    	Cmd6.MMC_CMD6_Layout.Access = EXT_CSD_ACCESS_WRITE_BYTE;			//Write Byte
		Cmd6.MMC_CMD6_Layout.CmdSet = 0;									// Don't Care
		Cmd6.MMC_CMD6_Layout.Index = BUS_WIDTH_MMC_EXT_CSD_OFFSET; 			// Choose Bus Width
		Cmd6.MMC_CMD6_Layout.Reserved0 = 0;
		Cmd6.MMC_CMD6_Layout.Reserved1 = 0;
		Cmd6.MMC_CMD6_Layout.Value = (UINT8_T) 2;	// Choose 8 bit mode.			
    	 
 		MM4_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD6, MM4_CMD_TYPE_NORMAL, Cmd6.MMC_CMD6_Bits, MM4_RT_R1 | MM4_RT_BUSY | MM4_48_RES_WITH_BUSY);
		Retval = MM4_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1B, 0x10);
	}

    //send CMD13 to check the status of the card
    Retval |= MM4_CheckCardStatus(pSDMMCP, 0x900, R1_LOCKEDCARDMASK);		// Make sure card is transfer mode
	if (Retval != NoError)
	{
	 	pSDMMCP->State = READY;
		return SDMMC_SWITCH_ERROR;
	}

	// Now change the controller to boost bus width
	if (pSDMMCP->SD == XLLP_SD)
	{
	  	if (SDBusWidth == SCRSD4BITMODE)
		  	pMM4_CNTL1->datawidth  = 1;		// Move to 4-bit mode
	}
	else
		pMM4_CNTL1->ex_data_width = 1;		// Move to 8-bit mode.


	return NoError;
}

/***************************************************************
*   MM4HighSpeedTiming()
* 	Sets the Bus speed to high speed timing
*   Input:
*   Output:
*   Returns: NoError, ReadError or NotSupportedError
*      
*****************************************************************/
UINT_T MM4HighSpeedTiming()
{
	UINT_T argument, Cmd;
	MMC_CMD6_OVERLAY Cmd6;
	P_MM4_SDMMC_CONTEXT_T pContext;
	P_MM4_CNTL1 pMM4_CNTL1;
	P_MM4_BLK_CNTL pMM4_BLK_CNTL;
	UINT_T Retval = NoError;
	
	// Initialize Flash Properties
	P_SDMMC_Properties_T pSDMMCP = GetSDMMCProperties();

 	// Assign our context value
 	pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;
	pMM4_CNTL1 = (P_MM4_CNTL1) &pContext->pMMC4Reg->mm4_cntl1;

	// Check supported configurations first
	if (pSDMMCP->SD == XLLP_SD)
	{
		MMC4SetBusRate(pContext, MM4CLOCK12_5MHZRATE);
		return NoError;
	}
	else
	{
		// Issue CMD 6 to set BUS WIDTH bits in EXT_CSD register byte 183
    	Cmd6.MMC_CMD6_Layout.Access = EXT_CSD_ACCESS_WRITE_BYTE;			//Write Byte
		Cmd6.MMC_CMD6_Layout.CmdSet = 0;									// Don't Care
		Cmd6.MMC_CMD6_Layout.Index = HS_TIMING_MMC_EXT_CSD_OFFSET; 			// Choose Bus Width
		Cmd6.MMC_CMD6_Layout.Reserved0 = 0;
		Cmd6.MMC_CMD6_Layout.Reserved1 = 0;
		Cmd6.MMC_CMD6_Layout.Value = (UINT8_T) 1;	// Choose High Speed Timing.
    	 
 		MM4_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD6, MM4_CMD_TYPE_NORMAL, Cmd6.MMC_CMD6_Bits, MM4_RT_R1 | MM4_RT_BUSY | MM4_48_RES_WITH_BUSY);
		Retval = MM4_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1B, 0x10);
	}

    //send CMD13 to check the status of the card
    Retval |= MM4_CheckCardStatus(pSDMMCP, 0x900, R1_LOCKEDCARDMASK);		// Make sure card is transfer mode
	if (Retval != NoError)
	{
	 	pSDMMCP->State = READY;
		MMC4SetBusRate(pContext, MM4CLOCK12_5MHZRATE);						// Failed, stick with lower speed
		return NoError;
	}

	// Now change the speed to max through the controller 
	MMC4SetBusRate(pContext, MM4CLOCK50MHZRATE);

	return NoError;
}




/***************************************************************
*   MM4SwitchPartition
*		If the Card supports partitioning (eSD) this routine will switch to the appropriate 
* 		partition by using extended partition command set CMD37.
*   Input:
*		PartitionNumber - Contains the partition Number to switch to and enable bits for the boot partitions.
*   Output:
*   Returns: NoError, ReadError or NotSupportedError
*      
*****************************************************************/
UINT_T MM4SwitchPartition(UINT_T PartitionNumber)
{
 UINT_T argument, Cmd;
 UINT_T Retval = NoError;
 UINT_T flags = NO_FLAGS;
 P_MM4_SDMMC_CONTEXT_T pContext;
 MMC_CMD6_OVERLAY Cmd6;
 PARTITION_CONFIG_EXT_CSD MMCPartConfig;

 // Initialize Flash Properties
 P_SDMMC_Properties_T pSDMMCP = GetSDMMCProperties();

 // Assign our context value
 pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;

 // Must set partition
 if (pSDMMCP->SD == XLLP_SD)
 {
	// CMD 43 Select Partition
    Cmd = XLLP_eSD_CMD43;
    argument = PartitionNumber << 24;//partition number goes in bits [31:24], rest are 0's

    MM4_SendSetupCommand(pSDMMCP, Cmd, MM4_CMD_TYPE_NORMAL, argument, MM4_RT_R1 | MM4_RT_BUSY | MM4_48_RES_WITH_BUSY);
	Retval = MM4_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1B, 0x10);
 }
 else
 {
    // Issue CMD 6 to clear PARTITION_ACCESS bits in EXT_CSD register byte 179
 	Cmd6.MMC_CMD6_Layout.Access = EXT_CSD_ACCESS_CLEAR_BITS;				// Clear bits
    Cmd6.MMC_CMD6_Layout.CmdSet = 0;                		  				// Don't Care
    Cmd6.MMC_CMD6_Layout.Index = PARTITION_CONFIG_MMC_EXT_CSD_OFFSET;       // Choose Boot Config
    Cmd6.MMC_CMD6_Layout.Reserved0 = 0;
    Cmd6.MMC_CMD6_Layout.Reserved1 = 0;
    Cmd6.MMC_CMD6_Layout.Value = PARTITION_ACCESS_BITS;				 		// Clear out Partition Access bits

    MM4_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD6, MM4_CMD_TYPE_NORMAL, Cmd6.MMC_CMD6_Bits, MM4_RT_R1 | MM4_RT_BUSY | MM4_48_RES_WITH_BUSY);
	Retval = MM4_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1B, 0x10);

	// Now issue CMD 6 again to set the right bits.
 	Cmd6.MMC_CMD6_Layout.Access = EXT_CSD_ACCESS_SET_BITS;				// Clear bits
    Cmd6.MMC_CMD6_Layout.CmdSet = 0;                		  			// Don't Care
    Cmd6.MMC_CMD6_Layout.Index = PARTITION_CONFIG_MMC_EXT_CSD_OFFSET;   // Choose Boot Config
    Cmd6.MMC_CMD6_Layout.Reserved0 = 0;
    Cmd6.MMC_CMD6_Layout.Reserved1 = 0;
    Cmd6.MMC_CMD6_Layout.Value = PartitionNumber;			 			// Set the correct partition

    MM4_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD6, MM4_CMD_TYPE_NORMAL, Cmd6.MMC_CMD6_Bits, MM4_RT_R1 | MM4_RT_BUSY | MM4_48_RES_WITH_BUSY);
	Retval |= MM4_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1B, 0x10);
 }
 
 Retval |= MM4_CheckCardStatus(pSDMMCP, 0x900, R1_LOCKEDCARDMASK);
 if (Retval != NoError)
 {
 	pSDMMCP->State = READY;
 	return SDMMC_SWITCH_ERROR;
 }
 return NoError;
}

/**********************************************************
*   MM4_CardShutdown
*      Shuts down the MM4 hardware
*   Input:
*   Output:
*      none
*   Returns:
*      WTP recoginized Success/Fail return code
***********************************************************/
UINT_T MM4_CardShutdown()
{
	P_MM4_SDMMC_CONTEXT_T pContext;
	P_MM4_CNTL1 pMM4_CNTL1;

	// Initialize Flash Properties
	P_SDMMC_Properties_T pSDMMCP = GetSDMMCProperties();
	pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;
	pMM4_CNTL1 = (P_MM4_CNTL1) &pContext->pMMC4Reg->mm4_cntl1;


	// send a CMD0, go idle here so the card gets into the idle state.
	// even though that forces any subsequent software, like the os,
	// to rediscover and reinit the card, this is worth doing. by
	// putting the card into the idle state, it will be able to handle
	// any bootrom flash probe operation if a platform reset occurs
	// before the OS has completely initialized the device.
	//
	// Send CMD0 (GO_IDLE_STATE) to get card into idle state
	MM4_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD0, MM4_CMD_TYPE_NORMAL, 0, MM4_RT_NONE | MM4_NO_RES);
	MM4_Interpret_Response(pSDMMCP, MMC_RESPONSE_NONE, 1);

	// Disable Bus Power
	pMM4_CNTL1->buspwr = 1;

	// Stop Bus Clock
	MMC4StopBusClock (pSDMMCP->pContext);

	// Mask all interrupts
	MMC4EnableDisableIntSources(pSDMMCP->pContext, DISABLE_INTS);

	// Disable internal clocks.
	MMC4StopInternalBusClock(pSDMMCP->pContext);

	return NoError;
}



/****************************************************************
*    MM4PrepareMMCContext
*		Sets certain Peripheral register address
*    Input:
* 		P_SDMMC_Properties_T pSDMMCP - pointer to the SDMMC context structure
*    Output:
*		none
*    Returns:
*       none
*****************************************************************/

void MM4_PrepareMMCContext (P_SDMMC_Properties_T pSDMMCP, UINT_T BaseAddress)
{
	pSDMMCP->pContext = &MM4_Context;

	// First fill out pContext
	((P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext)->pMMC4Reg = (P_MM4_SDMMC_T) BaseAddress;
}

/****************************************************************
*    MM4_SetControllerVoltage
*		Inspects the Capabilities Register for supported voltage types by the
*		controller. Then programs the CNTL1 register with the desired range.
*		Enables bus power
*    Input:
* 		P_MM4_SDMMC_CONTEXT_T pContext
*    Output:
*		none
*    Returns:
*       none
*****************************************************************/
UINT_T MM4_SetControllerVoltage (P_MM4_SDMMC_CONTEXT_T pContext)
{
 UINT_T controllervoltage;
 P_MM4_CNTL1 pMM4_CNTL1 = (P_MM4_CNTL1) &pContext->pMMC4Reg->mm4_cntl1;
 MM4_CNTL1_UNION MM4_cntl1;

 // Capture the Value
 MM4_cntl1.mm4_cntl1_value = *(VUINT_T*) pMM4_CNTL1;

 // Note that this really doesn't control voltage, it just needs to match one of the supported values in the capabilities 2 register.
 controllervoltage = MM4_VLTGSEL_3_3;

 // Set the voltage to controller
 MM4_cntl1.mm4_cntl1_bits.vltgsel = controllervoltage;

 // Enable Bus Power
 MM4_cntl1.mm4_cntl1_bits.buspwr = 1;

 // Write back out.
 *(VUINT_T*) pMM4_CNTL1 = MM4_cntl1.mm4_cntl1_value;

 return controllervoltage;
}

/****************************************************************
*    MM4_CheckVoltageCompatibility
*		Checks to make sure that the OCR register of the device supports the
*		voltage range that was selected for the controller
*    Input:
* 		P_MM4_SDMMC_CONTEXT_T pContext, UINT_T ControllerVoltage
*    Output:
*		none
*    Returns:
*       none
*****************************************************************/
UINT_T MM4_CheckVoltageCompatibility(P_SDMMC_Properties_T pSDMMCP, UINT_T ControllerVoltage)
{
	// Check SD vs MMC
	if (pSDMMCP->SD == XLLP_SD)
	{
		switch (ControllerVoltage)
		{
			case MM4_VLTGSEL_3_3:
				if (pSDMMCP->CardReg.OCR & SD_OCR_VOLTAGE_3_3_TO_3_6)
					return NoError;

			case MM4_VLTGSEL_3_0:
				if (pSDMMCP->CardReg.OCR & SD_OCR_VOLTAGE_1_8_TO_3_3)
					return NoError;

			case MM4_VLTGSEL_1_8:
				if (pSDMMCP->CardReg.OCR & SD_OCR_VOLTAGE_1_8)
					return NoError;
		}
	}
	else
	{
		if ((pSDMMCP->CardReg.OCR & MMC_OCR_VOLTAGE_ALL) == MMC_OCR_VOLTAGE_ALL)
			return NoError;
	}
	return SDMMCDeviceVoltageNotSupported;
}


/******************************************************************************
  Description:
    Set up the registers of the controller to start the transaction to
    communicate to the card for data related command.  The commands are clearly defined in the MMC
    specification.
  Input Parameters:
	   P_SDMMC_Properties_T pSDMMCP - Generic SD/MMC driver properties structure
  Cmd
		Command Index - See MMC or SD specification
  argument
      	the argument of  the command. MSW is for ARGH and LSW is for ARGL
  BlockType
	  	Multiple or Single Block Type
  ResType
		Expected response type
  Output Parameters:
    None
  Returns:
    None
*******************************************************************************/

void MM4_SendDataCommand (P_SDMMC_Properties_T pSDMMCP,
                  UINT_T Cmd,
                  UINT_T  Argument,
                  UINT_T BlockType,
                  UINT_T DataDirection,
                  UINT_T ResType)
{
 // Assign our context value
 P_MM4_SDMMC_CONTEXT_T pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;

 // no need to clear out any fault state that may be left over from a previously failed transaction.
 // that's because the caller has set State to read or write before calling here.
 
 // No Response to the command yet
 pSDMMCP->CardReponse.CommandComplete = 0;

 // save the info for use by the isr:
 pSDMMCP->Trans.RespType = (ResType >> 8) & 0x000000ff;
 pSDMMCP->Trans.Cmd = Cmd;	// Fixme: how to know when to set the ACMD flag?

 MMC4SendDataCommand(pContext,
                  Cmd,
                  Argument,
                  BlockType,
                  DataDirection,
                  ResType & 0x000000ff, // clear out any bits not for the SD_CMD.RES_TYPE field
                  pSDMMCP->SDMA_Mode);	
}

/******************************************************************************
  Description:
    Set up the registers of the controller to start the transaction to
    communicate to the card for data related command.  The commands are clearly defined in the MMC
    specification.
  Input Parameters:
	   P_SDMMC_Properties_T pSDMMCP - Generic SD/MMC driver properties structure
  Cmd
		Command Index - See MMC or SD specification
  argument
      	the argument of  the command. MSW is for ARGH and LSW is for ARGL
  BlockType
	  	Multiple or Single Block Type
  ResType
		Expected response type
  Output Parameters:
    None
  Returns:
    None
*******************************************************************************/

void MM4_SendDataCommandNoAuto12 (P_SDMMC_Properties_T pSDMMCP,
                  UINT_T Cmd,
                  UINT_T  Argument,
                  UINT_T BlockType,
                  UINT_T DataDirection,
                  UINT_T ResType)
{
 // Assign our context value
 P_MM4_SDMMC_CONTEXT_T pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;

 // no need to clear out any fault state that may be left over from a previously failed transaction.
 // that's because the caller has set State to read or write before calling here.
 
 // No Response to the command yet
 pSDMMCP->CardReponse.CommandComplete = 0;

 // save the info for use by the isr:
 pSDMMCP->Trans.RespType = (ResType >> 8) & 0x000000ff;
 pSDMMCP->Trans.Cmd = Cmd;	// Fixme: how to know when to set the ACMD flag?

 MMC4SendDataCommandNoAuto12(pContext,
                  Cmd,
                  Argument,
                  BlockType,
                  DataDirection,
                  ResType & 0x000000ff, // clear out any bits not for the SD_CMD.RES_TYPE field
                  pSDMMCP->SDMA_Mode);	
}

/******************************************************************************
  Description:
    Set up the registers of the controller to start the transaction to
    communicate to the card for setup related commands.
    The commands are clearly defined in the MMC specification.
  Input Parameters:
	   P_SDMMC_Properties_T pSDMMCP - Generic SD/MMC driver properties structure
	Cmd
		Command Index - See MMC or SD specification
    argument
      	the argument of  the command. MSW is for ARGH and LSW is for ARGL
	ResType
		Expected response type
  Output Parameters:
    None
  Returns:
    None
*******************************************************************************/

void MM4_SendSetupCommand(P_SDMMC_Properties_T pSDMMCP,
                  UINT_T Cmd,
                  UINT_T CmdType,
                  UINT_T Argument,
                  UINT_T ResType)
{

 // Assign our context value
 P_MM4_SDMMC_CONTEXT_T pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;

 // clear out any fault status that may be left over from a previously failed transaction.
 pSDMMCP->State = READY;
 
 // No Response to the command yet
 pSDMMCP->CardReponse.CommandComplete = 0;

 // save the info for use by the isr:
 pSDMMCP->Trans.RespType = (ResType >> 8) & 0x000000ff;
 pSDMMCP->Trans.Cmd = Cmd;	// Fixme: how to know when to set the ACMD flag?

 MMC4SendSetupCommand(pContext,
                  Cmd,
                  CmdType,
                  Argument,
                  ResType & 0x000000ff);	// clear out any bits not for the SD_CMD.RES_TYPE field
}

/****************************************************************
*   MM4_Read_Response
*	   	Reads the response from the Controller Buffer Registers to the Local Buffer.
* 		According to the last command and response type it does the correct interpretation.
*		There is also a timeout as the routine waits for the ISR to signal last command completion.
*   Input:
*	   	P_SDMMC_Properties_T pSDMMCP - Generic SD/MMC driver properties structure
* 		ResponseType - See SD/MMC specifications
*		ResponseTimeOut - A time out value in millisec's
*   Output:
*      	none
*   Returns:
*      	TimeOutError or NoError
*****************************************************************/

UINT_T MM4_Interpret_Response(P_SDMMC_Properties_T pSDMMCP, UINT_T ResponseType, UINT_T ResponseTimeOut)
{
 UINT_T i, temp, temp2, temp3, startTime, endTime;
 UINT_T Retval = NoError;
 P_MM4_SDMMC_CONTEXT_T pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;		// Assign our context value
 P_MM4_CMD_XFRMD_UNION pMM4_XFRMD;

 startTime = GetOSCR0();   // Dummy read to flush potentially bad data
 startTime = GetOSCR0();   // get the start time
 endTime = startTime;

 // Overlap XFRMD register contents using uniun
 pMM4_XFRMD = (P_MM4_CMD_XFRMD_UNION) &pContext->pMMC4Reg->mm4_cmd_xfrmd;

 // Wait for the Response based on the CommandComplete interrupt signal
 while (!pSDMMCP->CardReponse.CommandComplete)
 {
	 // if the command had an error, the command may have aborted
	 // without setting the command complete bit. for example,
	 // if no response is received, then the command is aborted,
	 // the MM4_ISR sets the state to FAULT, and command complete
	 // will not assert. check for such a scenario here.
	 if( pSDMMCP->State == FAULT )
	 {
		Retval = SDMMC_GENERAL_ERROR;	// recommendation: return a more specific error.
		return Retval;
	}

	// check if too much time has elapsed. if so, timeout.
	endTime = GetOSCR0();
	if (endTime < startTime)
		  endTime += (0x0 - startTime);

	if (OSCR0IntervalInMilli(startTime, endTime) > ResponseTimeOut)
	{
		Retval = TimeOutError;
		return Retval;
	}
 }

 // Read in the Buffers
 switch (ResponseType)
 {
  	case MMC_RESPONSE_NONE:
		break;
	case MMC_RESPONSE_R1:
	case MMC_RESPONSE_R1B:
		pSDMMCP->CardReponse.R1_RESP = pSDMMCP->CardReponse.pBuffer[0];
		break;
	case MMC_RESPONSE_R2:			// This is for CID or CSD register
	{
	 	if (pMM4_XFRMD->mm4_cmd_xfrmd_bits.cmd_idx == XLLP_MMC_CMD9) //CSD
	 	{
			// Copy the CSD values from the buffer
			for (i=0; i<4; i++)
				pSDMMCP->CardReg.CSD.CSD_VALUE[i] = pSDMMCP->CardReponse.pBuffer[i];

			   // Optionally we could record maximum block lengths from the CSD.
			// But some devices cheat and put incorrect values in this field.
			// Save off read Block Size, play it safe, for now hard code to 512 Bytes
			pSDMMCP->ReadBlockSize = HARD512BLOCKLENGTH;
			// Save off Write Block Size
			pSDMMCP->WriteBlockSize = HARD512BLOCKLENGTH;

			// Capture Erase Granularity.
			if (pSDMMCP->SD == XLLP_SD)
			{
				// Check Erase Single Block Enable - Bit 46
				if ((pSDMMCP->CardReg.CSD.CSD_VALUE[1] >> 14) & 1)
					pSDMMCP->EraseSize = pSDMMCP->WriteBlockSize;
				else
				{
					pSDMMCP->EraseSize = ((pSDMMCP->CardReg.CSD.CSD_VALUE[1] >> 7) & 0x7F) + 1;
					pSDMMCP->EraseSize *= pSDMMCP->WriteBlockSize;
				}
			}
			else // MMC Card
			{
				pSDMMCP->EraseSize = ((pSDMMCP->CardReg.CSD.CSD_VALUE[1] >> 5) & 0x1F) + 1;  		// Get ERASE_GRP_MULT
				pSDMMCP->EraseSize *= (((pSDMMCP->CardReg.CSD.CSD_VALUE[1] >> 10) & 0x1F) + 1);	// Get ERASE_GRP_SIZE
				pSDMMCP->EraseSize *= pSDMMCP->WriteBlockSize;
			}

			// Now calculate the capacity of this card
			temp = ((pSDMMCP->CardReg.CSD.CSD_VALUE[2] >> 16) & 0xF); 		// Get READ_BL_LEN
			temp = 1 << temp; 										// Now we have Max Block Length
			temp2 = ((pSDMMCP->CardReg.CSD.CSD_VALUE[1] >> 15) & 0x7) + 2; 	// Get C_SIZE_MULT
			temp2 = 1 << temp2;
			temp3 = ((pSDMMCP->CardReg.CSD.CSD_VALUE[1] >> 30) & 0x3);		// Get C_SIZE
			temp3 |= ((pSDMMCP->CardReg.CSD.CSD_VALUE[2] & 0x3FF) << 2);	// Get C_SIZE
			temp3++;
			pSDMMCP->CardCapacity = temp3 * temp2 * temp;		// Total Size of the card in Bytes
		}
	 	else // Assume CID
	 	{
			// Copy the CSD values from the buffer
			for (i=0; i<4; i++)
				pSDMMCP->CardReg.CID.CID_VALUE[i] = pSDMMCP->CardReponse.pBuffer[i];

			// Now capture the serial number from the CID - 32 bit number
			if (pSDMMCP->SD == XLLP_MMC)
			{
				pSDMMCP->CardReg.CID.SerialNum = (pSDMMCP->CardReg.CID.CID_VALUE[0] >> 16) & (0xFF);
				pSDMMCP->CardReg.CID.SerialNum |= (pSDMMCP->CardReg.CID.CID_VALUE[1] << 16);
			}
			else
			{
				pSDMMCP->CardReg.CID.SerialNum = (pSDMMCP->CardReg.CID.CID_VALUE[0] >> 24) & (0xF);
				pSDMMCP->CardReg.CID.SerialNum |= (pSDMMCP->CardReg.CID.CID_VALUE[1] << 8);
			}
	 	}
	 	break;
	}
	case MMC_RESPONSE_R3:
	{
		pSDMMCP->CardReg.OCR = pSDMMCP->CardReponse.pBuffer[0];
		break;
	}
	case MMC_RESPONSE_R4:				// These modes are not supported by the driver
	case MMC_RESPONSE_R5:
	case MMC_RESPONSE_R5B:
		break;
	case MMC_RESPONSE_R6:				// Publishes RCA for SD cards
	{
		pSDMMCP->CardReg.RCA = pSDMMCP->CardReponse.pBuffer[0];
		break;
	}
	case MMC_RESPONSE_R7:
	{
		pSDMMCP->SD_VHS = pSDMMCP->CardReponse.pBuffer[0];
		break;
	}
 }
 return Retval;
}

/****************************************************************
*   SDMMCGetMatchWaitCardStatus
*      	Gets the status of the card by issuing CMD 13. The rerturn from the routine is based
* 		on a check against the expected value which is passed in
*   Input:
*      	pSDMMCP - pointer to the current context
*      	MaxWaitMSec - Maximum wait time in millisec
* 	   	R1_Resp_Match - Desired Value to be matched
*   Output:
*      	none
*   Returns:
*      	none
*****************************************************************/
UINT_T MM4_CheckCardStatus(P_SDMMC_Properties_T pSDMMCP, UINT_T R1_Resp_Match, UINT_T Mask)
{
    UINT_T flags, argument, cardStatus, retval;

   	//send CMD13 to check the status of the card
   	argument = pSDMMCP->CardReg.RCA;
	MM4_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD13, MM4_CMD_TYPE_NORMAL, argument, MM4_RT_R1 | MM4_48_RES);
	retval = MM4_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1, 0x100);

	// Mask out undesired check bits
	cardStatus = (pSDMMCP->CardReponse.R1_RESP) & Mask;

	if ((cardStatus == R1_Resp_Match) && (retval == NoError))
		return NoError;
	else
		return TimeOutError;
}

/***********************************************************
*    MM4_ReadBlocks()
*       Reads the given block off of the SD/MMC card and
*       into LocalAddr or empty buffer
*    input:
*		none
*    output:
*       LocalAddr will contain the contents of the block
*    returns:
*       none
************************************************************/
UINT_T MM4_ReadBlocks()
{
    UINT_T flags, argument;
	UINT_T Retval = NoError;
	P_MM4_SDMMC_CONTEXT_T pContext;
	P_MM4_BLK_CNTL pMM4_BLK_CNTL;

	// Initialize Flash Properties
	P_SDMMC_Properties_T pSDMMCP = GetSDMMCProperties();

 	// Assign our context value
 	pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;

	// Must set MMC NUMBLK
	pMM4_BLK_CNTL = (P_MM4_BLK_CNTL) &pContext->pMMC4Reg->mm4_blk_cntl;
	pMM4_BLK_CNTL->blk_cnt = pSDMMCP->Trans.NumBlocks;

	if (pSDMMCP->SDMA_Mode)
	{
		pMM4_BLK_CNTL->dma_bufsz = MM4_512_HOST_DMA_BDRY;
	    pContext->pMMC4Reg->mm4_sysaddr = pSDMMCP->Trans.LocalAddr;
	} 

	// Set up State
	pSDMMCP->State = READ;

	// Do a CMD 18 Read Multiple Block
    argument = pSDMMCP->Trans.CardAddress;
	if (pSDMMCP->AccessMode == SECTOR_ACCESS)
		argument /= HARD512BLOCKLENGTH; 	// In sector mode addressing; all addresses need to be specified as block offsets.

	MM4_SendDataCommand(pSDMMCP, XLLP_MMC_CMD18, argument, MM4_MULTI_BLOCK_TRAN, MM4_CARD_TO_HOST_DATA, MM4_RT_R1 | MM4_48_RES);

	// Wait for the Read to Complete
	while ((pSDMMCP->State != FAULT) && (pSDMMCP->State != READY));		// TBD add timeout Let the ISR run, we'll either get a fault or finish

	if( pSDMMCP->State == FAULT )	// This state entered if ISR detected an error.
	{
		return SDMMCReadError;
	}

	// Get the Card Response
	Retval = MM4_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1, 0x100);
	if ((Retval != NoError) || ((pSDMMCP->CardReponse.R1_RESP & R1_LOCKEDCARDMASK) != 0x900) || (pSDMMCP->State == FAULT))
	{
		Retval = SDMMCReadError;
		pSDMMCP->State = FAULT;
		// Send a stop command
		MM4_SendStopCommand(pSDMMCP);
	}
	else
	{
		pSDMMCP->State = READY;
	}

	return Retval;
}

/***********************************************************
*    MM4_MMCAlternateBootModeReadBlocks()
*       Reads the given block off of the SD/MMC card and
*       into LocalAddr or empty buffer
*    input:
*		none
*    output:
*       LocalAddr will contain the contents of the block
*    returns:
*       none
************************************************************/
UINT_T MM4_MMCAlternateBootModeReadBlocks()
{
    UINT_T flags, argument;
	UINT_T Retval = NoError;
	P_MM4_SDMMC_CONTEXT_T pContext;
	P_MM4_BLK_CNTL pMM4_BLK_CNTL;

	// Initialize Flash Properties
	P_SDMMC_Properties_T pSDMMCP = GetSDMMCProperties();

 	// Assign our context value
 	pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;

	// Must set MMC NUMBLK
	pMM4_BLK_CNTL = (P_MM4_BLK_CNTL) &pContext->pMMC4Reg->mm4_blk_cntl;
	pMM4_BLK_CNTL->blk_cnt = pSDMMCP->Trans.NumBlocks;

	// Set up State
	pSDMMCP->State = READ;

	// Are we SDMA mode enabled?
	if (pSDMMCP->SDMA_Mode)
	{
		pContext->pMMC4Reg->mm4_blk_cntl |= MM4_512_HOST_DMA_BDRY << 12;	// Set SDMA buffer size.
	    pContext->pMMC4Reg->mm4_sysaddr = pSDMMCP->Trans.LocalAddr;
	} 

	// Do a CMD 0 to start Alternate Boot Mode and read
    argument = MMC_ALTERNATE_BOOT_ARGUMENT;
	MM4_SendDataCommandNoAuto12(pSDMMCP, XLLP_MMC_CMD0, argument, MM4_MULTI_BLOCK_TRAN, MM4_CARD_TO_HOST_DATA, MM4_RT_NONE | MM4_NO_RES);

	Retval = MM4_WaitForOperationComplete(10000);

 	// Get the Card Response
	if (Retval != NoError)
		return Retval;

	// Issue CMD 0 again to stop sending data and terminate alternate boot mode.
	MM4_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD0, MM4_CMD_TYPE_NORMAL, 0x0, MM4_NO_RES);
	Retval = MM4_Interpret_Response(pSDMMCP, MMC_RESPONSE_NONE, 0x10);

	return Retval;
}


// fixme: change this to use an r1b type...
UINT_T MM4_WaitReady(UINT_T TimeOutMilliSec)
{
	UINT_T					startTime, endTime;
	UINT_T					writecomplete = 0;
	UINT_T					Retval = NoError;
	P_SDMMC_Properties_T	pSDMMCP = GetSDMMCProperties();
    UINT_T					argument = pSDMMCP->CardReg.RCA;

	// issue a series of get status commands until the (new) status indicates ready.
	// limit the amount of time to wait to the input parameter TimeOutMilliSec
	startTime = GetOSCR0();
	startTime = GetOSCR0();	// need to read timer twice because of a silicon bug. FIXME: need JIRA reference.
	do
	{
   		//send CMD13 to check the status of the card
		MM4_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD13, MM4_CMD_TYPE_NORMAL, argument, MM4_RT_R1 | MM4_48_RES);
		Retval = MM4_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1, 0x100);	// FIXME: this timeout value should be dependent on CSD and operating configuration
		if (Retval != NoError) break;	// failed to complete transaction

		// examine the new status, which was just extracted from the response field of the get stauts command.
		if( ( pSDMMCP->CardReponse.R1_RESP & 0x00000100 ) == 0x00000100 ) // card_status:READY_FOR_DATA asserted. write is complete.
		{
			writecomplete = 1;
			break;			
		}

		endTime = GetOSCR0();			// check for end of wait interval
		if (endTime < startTime)
			endTime += (0x0 - startTime);      
	}
	while( OSCR0IntervalInMilli(startTime, endTime) < TimeOutMilliSec );

	return writecomplete;
}


/***********************************************************
*    MM4_WriteBlocks()
*		Writes the required number of blocks to CardAddress
*    input:
*		none
*    output:
*       Address starting with CardAddress will contain content from LocalAddress
*    returns:
*       none
************************************************************/
UINT_T MM4_WriteBlocks(void)
{
    UINT_T					flags, argument;
	UINT_T					Retval = NoError;
	P_MM4_SDMMC_CONTEXT_T	pContext;
	P_MM4_BLK_CNTL			pMM4_BLK_CNTL;

	// Initialize Flash Properties
	P_SDMMC_Properties_T pSDMMCP = GetSDMMCProperties();

 	// Assign our context value
 	pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;

	// Must set MMC NUMBLK
	pMM4_BLK_CNTL = (P_MM4_BLK_CNTL) &pContext->pMMC4Reg->mm4_blk_cntl;
	pMM4_BLK_CNTL->blk_cnt = pSDMMCP->Trans.NumBlocks;
	if (pSDMMCP->SDMA_Mode)
	{
		pMM4_BLK_CNTL->dma_bufsz = MM4_512_HOST_DMA_BDRY;
	    pContext->pMMC4Reg->mm4_sysaddr = pSDMMCP->Trans.LocalAddr;
	} 

	// Set up State
	pSDMMCP->State = WRITE;

	// Do a CMD 25 Write Multiple Blocks
    argument = pSDMMCP->Trans.CardAddress;
	if (pSDMMCP->AccessMode == SECTOR_ACCESS)
		argument /= HARD512BLOCKLENGTH; 	// In sector mode addressing; all addresses need to be specified as block offsets.

	MM4_SendDataCommand(pSDMMCP, XLLP_MMC_CMD25, argument, MM4_MULTI_BLOCK_TRAN, MM4_HOST_TO_CARD_DATA, MM4_RT_R1 | MM4_48_RES);

	// Wait for the Write to Complete
	while ((pSDMMCP->State != FAULT) && (pSDMMCP->State != READY));		// TBD add timeout Let the ISR run, we'll either get a fault or finish

	if( pSDMMCP->State == FAULT )	// This state entered if ISR detected an error.
	{
		return SDMMCWriteError;
	}

	// Get the Card Response
	Retval = MM4_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1, 0x100);
	if ((Retval != NoError) || (pSDMMCP->State == FAULT) || ((pSDMMCP->CardReponse.R1_RESP & R1_NOMASK) != 0x900))
	{
		Retval = WriteError;
		pSDMMCP->State = FAULT;
		// Send a stop command
		MM4_SendStopCommand(pSDMMCP);
		return Retval;
	}

	// the write data transfer completed...now must wait for the card to assert the ready line.
	// (the ready status from the last data transfer is meaningless here because it was left 
	//  over from the write command that initiated the data transfer.)
	//
	// section 7.7.2 of the mmc 4.3 spec defines a formula for the timeout value.
	// for now, just use a constant. Samsung KLMxGxxExM defines max write timeout as 600 ms
	// FIXME: implement the formula, which is based on info from the CSD...

	if( !MM4_WaitReady(600) )
	{
		Retval = WriteError;
		pSDMMCP->State = FAULT;
		// Send a stop command
		MM4_SendStopCommand(pSDMMCP);
		return Retval;
	}

	pSDMMCP->State = READY;
	return NoError;
}


/***********************************************************
*    MM4_EraseBlocks()
*		Erases required number of blocks at CardAddress
*    input:
*		none
*    output:
*		Blocks erased on erase group alignment
*    returns:
*       none
************************************************************/
UINT_T MM4_EraseBlocks()
{
	UINT_T argument, Cmd;
	UINT_T Retval = NoError;
    UINT_T flags = NO_FLAGS;
	P_MM4_SDMMC_CONTEXT_T pContext;

	// Initialize Flash Properties
	P_SDMMC_Properties_T pSDMMCP = GetSDMMCProperties();

 	// Assign our context value
 	pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;

	// CMD 32/35 Define Erase Group Start Address
	if (pSDMMCP->SD == XLLP_SD)
		Cmd = XLLP_SD_CMD32;
	else
		Cmd = XLLP_MMC_CMD35;

    argument = pSDMMCP->Trans.CardAddress;
	if (pSDMMCP->AccessMode == SECTOR_ACCESS)
		argument /= HARD512BLOCKLENGTH; 	// In sector mode addressing; all addresses need to be specified as block offsets.

	MM4_SendSetupCommand(pSDMMCP, Cmd, MM4_CMD_TYPE_NORMAL, argument, MM4_RT_R1 | MM4_48_RES);
	Retval = MM4_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1, 0x100);
	if (Retval != NoError)
	{
		pSDMMCP->State = FAULT;
		return EraseError;
	}

	// CMD 33/36 Define Erase Group End Address
	if (pSDMMCP->SD == XLLP_SD)
		Cmd = XLLP_SD_CMD33;
	else
		Cmd = XLLP_MMC_CMD36;

    argument = pSDMMCP->Trans.CardAddress + pSDMMCP->Trans.TransWordSize*4;
	if (pSDMMCP->AccessMode == SECTOR_ACCESS)
		argument /= HARD512BLOCKLENGTH; 	// In sector mode addressing; all addresses need to be specified as block offsets.

	MM4_SendSetupCommand(pSDMMCP, Cmd, MM4_CMD_TYPE_NORMAL, argument, MM4_RT_R1 | MM4_48_RES);
	Retval = MM4_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1, 0x100);
	if (Retval != NoError)
	{
		pSDMMCP->State = FAULT;
		return EraseError;
	}

	// CMD 38
	Cmd = XLLP_MMC_CMD38;
	argument = NO_ARGUMENT;
	MM4_SendSetupCommand(pSDMMCP, Cmd, MM4_CMD_TYPE_NORMAL, argument, MM4_RT_R1 | MM4_RT_BUSY | MM4_48_RES_WITH_BUSY);
	Retval = MM4_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1B, 0x8000);
	if (Retval != NoError)
	{
		Retval = EraseError;
		pSDMMCP->State = FAULT;
	}
	else
	{
		pSDMMCP->State = READY;
	}
	return Retval;
}

/***********************************************************
*   SDMMCWriteFifo
*      Writes 2048 bytes (512 words) to the FIFO
*   Input:
*      P_SDMMC_Properties_T pSDMMCP - pointer to the SDMMC context structure
*   Output:
*      none
*   Returns:
*      none
*************************************************************/
void MM4_WriteFifo(P_SDMMC_Properties_T pSDMMCP)
{
    int i, t = 0;
    UINT_T Buffer =0x0;
	P_MM4_SDMMC_CONTEXT_T pContext;
	volatile UINT_T *pMMC_TX_Fifo;

 	// Assign our context value
 	pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;

	pMMC_TX_Fifo = (volatile UINT_T *)&(pContext->pMMC4Reg->mm4_dp);
	t = pSDMMCP->Trans.WordIndex;

	// Ignore Pre Bytes
	for (i=0; (i < MM4FIFOWORDSIZE) && (t < pSDMMCP->Trans.StartDiscardWords); i++, t++)
		*pMMC_TX_Fifo = Buffer;

	// Write Requested Data
	for (; ((i < MM4FIFOWORDSIZE) && (t < (pSDMMCP->Trans.TransWordSize-pSDMMCP->Trans.EndDiscardWords))); i++, t++)
		*pMMC_TX_Fifo = ((UINT_T*) pSDMMCP->Trans.LocalAddr)[t];

	// Ignore Trailing Bytes
	for (; (i < MM4FIFOWORDSIZE) && (t < pSDMMCP->Trans.TransWordSize); i++, t++)
		*pMMC_TX_Fifo = Buffer;

	pSDMMCP->Trans.WordIndex = t;

}


/***********************************************************
*   MM4_ReadFifo
*      Reads the contents of the read fifo (512 words)
*   Input:
*      P_SDMMC_Properties_T pSDMMCP - pointer to the SDMMC context structure
*   Output:
*      buffer will contain the contents of the read fifo
*   Returns:
*      none
*************************************************************/
void MM4_ReadFifo(P_SDMMC_Properties_T pSDMMCP)
{
    int i, t = 0;
    UINT_T Buffer =0x0;
	P_MM4_SDMMC_CONTEXT_T pContext;
	volatile UINT_T *pMMC_RX_Fifo;

 	// Assign our context value
 	pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;

	pMMC_RX_Fifo = (volatile UINT_T *)&(pContext->pMMC4Reg->mm4_dp);
	t = pSDMMCP->Trans.WordIndex;

	// Ignore Pre Bytes
	for (i=0; (i < MM4FIFOWORDSIZE) && (t < pSDMMCP->Trans.StartDiscardWords); i++, t++)
		Buffer = *pMMC_RX_Fifo;

	// Read Requested Data
	for (; ((i < MM4FIFOWORDSIZE) && (t < (pSDMMCP->Trans.TransWordSize-pSDMMCP->Trans.EndDiscardWords))); i++, t++)
		((UINT_T*) pSDMMCP->Trans.LocalAddr)[t] = *pMMC_RX_Fifo;

	// Ignore Trailing Bytes
	for (; (i < MM4FIFOWORDSIZE) && (t < pSDMMCP->Trans.TransWordSize); i++, t++)
		Buffer = *pMMC_RX_Fifo;

	pSDMMCP->Trans.WordIndex = t;
}

/****************************************************************
*   MM4_SendStopCommand
*	Issues a stop command for open ended read and write block operations
*
*   Input:
*      P_SDMMC_Properties_T pSDMMCP - pointer to the SDMMC context structure
*   Output:
*      none
*   Returns:
*      none
*****************************************************************/
void MM4_SendStopCommand(P_SDMMC_Properties_T pSDMMCP)
{
	P_MM4_SDMMC_CONTEXT_T pContext;
 	// Assign our context value
 	pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;

 	// Send a CMD 12 to stop transmissions.
	MM4_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD12, MM4_CMD_TYPE_NORMAL, NULL, MM4_RT_R1 | MM4_RT_BUSY | MM4_48_RES_WITH_BUSY);	// fixme: r1 for read, r1b for write.
	MM4_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1, 0x100);
}

/****************************************************************
*   MM4_ISR
*      	Interrupt Service Routine for SDMMC controller
*		Controls flow and catches faults asynchronously
*   Input:
*	   P_SDMMC_Properties_T pSDMMCP
*   Output:
*      none
*   Returns:
*      none
*****************************************************************/

void MM4_ISR(P_SDMMC_Properties_T pSDMMCP)
{
	UINT_T i;
	VUINT_T *pControllerBuffer;
	UINT_T i_stat_copy;												// Keep a copy of i stat register
	UINT_T i_err_stat;												// contains only error status bits.
	UINT_T i_acmd12_err_stat;
	P_MM4_I_STAT  p_i_stat_copy;										// Pointer to the copy.
	P_MM4_I_STAT_UNION pMM4_I_STAT_U;							
	P_MM4_SDMMC_CONTEXT_T pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;		// Assign our context value
	UINT_T  r1_resp_error_bits = 0xfdffc080;    // that strange mask is all possible error bits in the card stat field.
	UINT_T	cmderror = 0;
	UINT_T	resptype;

	p_i_stat_copy = (P_MM4_I_STAT)&i_stat_copy;

	// Save off the interrupt source to the copy
	pMM4_I_STAT_U = (P_MM4_I_STAT_UNION) &pContext->pMMC4Reg->mm4_i_stat;
	i_stat_copy = pMM4_I_STAT_U->mm4_i_stat_value;

	// Check for any error
	if( p_i_stat_copy->cmdcomp )
	{
		// if we're in strict error checking mode, and 		
		// if the completing command has an R1 or R1B status, 
		// look for any error bits in the card status field
		if( pSDMMCP->StrictErrorCheck )
		{
			resptype = (pSDMMCP->Trans.RespType << 8) & MM4_RT_MASK;		// convert to a value more convenient for comparisons...
			if( resptype == MM4_RT_R1 )
			{
				cmderror = ( pContext->pMMC4Reg->mm4_resp0 & r1_resp_error_bits );	// that strange mask is all possible error bits.
			}
		}
		else cmderror = 0;	// don't examine error because not strict error checking mode or this is not an R1 type of command.
	}

	if (p_i_stat_copy->errint || cmderror )
	{ 
		// each error condition has its own handling/recovery requirements....
		
		// move the error bits down to a low halfword so the defines in sd.h 
		// can be used. (they were based on 16 bit io access)
		i_err_stat = i_stat_copy >> 16;

		// check & handle each error condition

		// the following errors require an sd host controller reset:
		if( ( i_err_stat & SD_ERROR_INT_STATUS_AXI_RESP_ERR     ) ||
			( i_err_stat & SD_ERROR_INT_STATUS_DATA_TIMEOUT_ERR ) ||
			( i_err_stat & SD_ERROR_INT_STATUS_CMD_TIMEOUT_ERR  ) ||
			cmderror )
		{
			MMC4CMDSWReset(pContext);	// this cleas the command inhibit flag in sd_present_state_1.
			MMC4DataSWReset(pContext);	// this clears the data inhibit flag and stops mclk.
		}

		// acmd 12 error requires examining a separate error status register:
		else if( i_err_stat & SD_ERROR_INT_STATUS_AUTO_CMD12_ERR )
		{
			i_acmd12_err_stat = pContext->pMMC4Reg->mm4_acmd12_er;

			// any acmd12 error requires an sd host controller reset
			// because there's no way to know if the command was processed.
			MMC4CMDSWReset(pContext);	// this cleas the command inhibit flag in sd_present_state_1.
			MMC4DataSWReset(pContext);	// this clears the data inhibit flag and stops mclk.

			// clear the acmd12 error bits.
			pContext->pMMC4Reg->mm4_acmd12_er = i_acmd12_err_stat;
		}

		// clear the general error status bits
		pContext->pMMC4Reg->mm4_i_stat = i_stat_copy;
		i = pMM4_I_STAT_U->mm4_i_stat_value;			// Must make a dummy read to allow interrupts to clear.

		// set a flag so the caller knows this request failed.
		pSDMMCP->State = FAULT;

		// done with handling an error condition.
		// nothing more to do.
		return;
	}

	// is this an sdma interrupt event?
	if(pContext->pMMC4Reg->mm4_i_stat & (1u<<3))	// bit 3 is dmaint
	{
		// the transfer halted because the boundary specified in ... was reached.
		// rewriting the sysaddr with the next address allows the transfer to resume.
		// fortunately the sysaddr register itself contains the next address.
		// so, just re-write the sysaddr register with its own current contents.
	    pContext->pMMC4Reg->mm4_sysaddr = pContext->pMMC4Reg->mm4_sysaddr;	// sysaddr points to next addr to write.

		pContext->pMMC4Reg->mm4_i_stat = (1u<<3);	// bit 3 is dmaint
		i = pContext->pMMC4Reg->mm4_i_stat;		// read-back to ensure write completes

	}
	// fall through the rest of the isr.
	// end of dmaint handling.

	// Has the Command completed? If so read the response register
	if (p_i_stat_copy->cmdcomp)
	{
 		// Indicate that the response has been read
		pSDMMCP->CardReponse.CommandComplete = 1;
		pControllerBuffer = (VUINT_T *) &pContext->pMMC4Reg->mm4_resp0;
 		for (i = 0; i < 4; i++)
 			pSDMMCP->CardReponse.pBuffer[i] = pControllerBuffer[i];
	}

	// Are we SDMA mode enabled?
	if (pSDMMCP->SDMA_Mode)
	{
		if (p_i_stat_copy->xfrcomp)
	 		pSDMMCP->State = READY;
 	}

	// Clear the interrupts
	pMM4_I_STAT_U->mm4_i_stat_value = i_stat_copy; 	// Clear the interrupt source.
	i = pMM4_I_STAT_U->mm4_i_stat_value;			// Must make a dummy read to allow interrupts to clear.

	if (pSDMMCP->SDMA_Mode == FALSE)
	{
		// Handle State based interrupts XFRCOMP, BUFRDRDY, BUFWRRDY
		switch (pSDMMCP->State)
		{
			case WRITE:
			{
				if (p_i_stat_copy->bufwrrdy)
					MM4_WriteFifo(pSDMMCP);

				// Are we done sending all of data?
				if (pSDMMCP->Trans.TransWordSize == pSDMMCP->Trans.WordIndex)
					pSDMMCP->State = DATATRAN;

				break;
			}
			case READ:
			{
				if (p_i_stat_copy->bufrdrdy)
					MM4_ReadFifo(pSDMMCP);

				// Are we done sending all of data?
				if (pSDMMCP->Trans.TransWordSize == pSDMMCP->Trans.WordIndex)
					pSDMMCP->State = DATATRAN;

				break;
			}
			case DATATRAN:
			{
				// Wait for Transfer Complete Signal
				if (p_i_stat_copy->xfrcomp)
					pSDMMCP->State = READY;

				break;
			}
			default:
				break;
		}
	}
	return;
}


//----------------------------------------------------------------------------
// MM4_WaitForOperationComplete( unsigned int statusMask, UINT32 TimeOutMillisec )
//        Replaces method susceptible to "infinite loop"
//		Waits for either operation complete or fault condition
//
//----------------------------------------------------------------------------

UINT_T MM4_WaitForOperationComplete(UINT_T TimeOutMillisec )
{
  UINT32 startTime, endTime;
  NDSR_REG     status;

  // Initialize Flash Properties
  P_SDMMC_Properties_T pSDMMCP = GetSDMMCProperties();

  startTime = GetOSCR0();
  startTime = GetOSCR0();

  do
  {
	if ((pSDMMCP->State == FAULT) || (pSDMMCP->State == READY))
		return NoError;

	endTime = GetOSCR0();
	if (endTime < startTime)
		endTime += (0x0 - startTime);      
  }
  while( OSCR0IntervalInMilli(startTime, endTime) < TimeOutMillisec );

  return TimeOutError;
}   



#if MMC_DEBUG
UINT_T TestSamsungSetBootPartition()
{
	UINT_T argument, Cmd;
	UINT_T Retval;
	P_MM4_SDMMC_CONTEXT_T pContext;
 	P_MM4_CNTL1 pMM4_CNTL1;
 	P_MM4_BLK_CNTL pMM4_BLK_CNTL;
    UINT_T pBuffer[512];    

	// Initialize Flash Properties
	P_SDMMC_Properties_T pSDMMCP = GetSDMMCProperties();

	if (pSDMMCP->SD == XLLP_SD)
		return NoError; 			// This is an MMC command only!

	// Assign our context value
	pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;
	pMM4_CNTL1 = (P_MM4_CNTL1) &pContext->pMMC4Reg->mm4_cntl1;

    // Issue CMD62 
    MM4_SendSetupCommand(pSDMMCP, 62, MM4_CMD_TYPE_NORMAL, 0xEFAC62EC, MM4_48_RES_WITH_BUSY);
    Retval = MM4_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1, 0x1000);

    MM4_SendSetupCommand(pSDMMCP, 62, MM4_CMD_TYPE_NORMAL, 0xCBAEA7, MM4_48_RES_WITH_BUSY);
    Retval = MM4_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1, 0x1000);

    MM4_SendSetupCommand(pSDMMCP, 62, MM4_CMD_TYPE_NORMAL, 2, MM4_48_RES_WITH_BUSY);
    Retval = MM4_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1, 0x1000);

    // Check Card Status
    Retval = MM4_CheckCardStatus(pSDMMCP, 0x900, R1_LOCKEDCARDMASK);         // Make sure card is transfer mode

    Retval = MM4_MMCReadEXTCSD (pBuffer);

    pSDMMCP->State = READY;
    return Retval;
}
#endif

/***************************************************************
*   MM4SwitchPartitionForAlternateBootMode
*		eMMC Alternate Boot Mode Setup
*   Input:
*		PartitionNumber - Contains the partition Number to switch to and enable bits for the boot partitions.
*   Output:
*   Returns: NoError, ReadError or NotSupportedError
*      
*****************************************************************/

UINT_T MM4SwitchPartitionForAlternateBootMode()
{
 UINT_T argument, Cmd;
 UINT_T Retval = NoError;
 UINT_T flags = NO_FLAGS;
 P_MM4_SDMMC_CONTEXT_T pContext;
 MMC_CMD6_OVERLAY Cmd6;
 PARTITION_CONFIG_EXT_CSD MMCPartConfig;

 // Initialize Flash Properties
 P_SDMMC_Properties_T pSDMMCP = GetSDMMCProperties();

 // Assign our context value
 pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;

 // Issue CMD 6 write byte 179 (0xB3)
 Cmd6.MMC_CMD6_Layout.Access = EXT_CSD_ACCESS_WRITE_BYTE;				// Write byte
 Cmd6.MMC_CMD6_Layout.CmdSet = 0;                		  				// Don't Care
 Cmd6.MMC_CMD6_Layout.Index = PARTITION_CONFIG_MMC_EXT_CSD_OFFSET;       // Choose Boot Config
 Cmd6.MMC_CMD6_Layout.Reserved0 = 0;
 Cmd6.MMC_CMD6_Layout.Reserved1 = 0;
 Cmd6.MMC_CMD6_Layout.Value = BOOT_FROM_PARTITION_1_WITH_BOOTACK;	    // Boot from Partition 1
 MM4_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD6, MM4_CMD_TYPE_NORMAL, Cmd6.MMC_CMD6_Bits, MM4_48_RES_WITH_BUSY);
 Retval = MM4_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1B, 0x100);
 Retval = MM4_CheckCardStatus(pSDMMCP, 0x0900, R1_LOCKEDCARDMASK);


 // Issue CMD 6 to write byte 177 (0xB1)
 Cmd6.MMC_CMD6_Layout.Access = EXT_CSD_ACCESS_WRITE_BYTE;				// Clear bits
 Cmd6.MMC_CMD6_Layout.CmdSet = 0;                		  				// Don't Care
 Cmd6.MMC_CMD6_Layout.Index = BOOT_BUS_WIDTH_MMC_EXT_CSD_OFFSET;       // Choose Boot Config
 Cmd6.MMC_CMD6_Layout.Reserved0 = 0;
 Cmd6.MMC_CMD6_Layout.Reserved1 = 0;
 Cmd6.MMC_CMD6_Layout.Value = BUS_WIDTH_8_WITH_SDR; //0xA; //  Use 2 for Samsung 4.3+ parts, they don't allow setting fast timing bits 0x2;
 MM4_SendSetupCommand(pSDMMCP, XLLP_MMC_CMD6, MM4_CMD_TYPE_NORMAL, Cmd6.MMC_CMD6_Bits, MM4_48_RES_WITH_BUSY);
 Retval = MM4_Interpret_Response(pSDMMCP, MMC_RESPONSE_R1B, 0x100);



 Retval |= MM4_CheckCardStatus(pSDMMCP, 0x0900, R1_LOCKEDCARDMASK);
 //Not in the ready state?
 if(Retval != NoError)
 {
    pContext->pMMC4Reg->mm4_cntl2 |= (1 << 25); // set reset bit
    pSDMMCP->State = READY;           // Ignore the error if the card doesn't support it
    return SDMMC_SWITCH_ERROR;
 }

 return NoError;
}

/***************************************************************
*   MM4_MMCReadEXTCSD
* 	Reads in 512 bytes of Extended CSD 
*   Input: Pointer to 512 byte buffer
*   Output:
*   Returns: NoError, ReadError or NotSupportedError
*      
*****************************************************************/
UINT_T MM4_MMCReadEXTCSD (UINT_T *pBuffer)
{
	UINT_T Cmd;
	UINT_T Retval;
	P_MM4_SDMMC_CONTEXT_T pContext;
 	P_MM4_CNTL1 pMM4_CNTL1;
 	P_MM4_BLK_CNTL pMM4_BLK_CNTL;

	// Initialize Flash Properties
	P_SDMMC_Properties_T pSDMMCP = GetSDMMCProperties();

	if (pSDMMCP->SD == XLLP_SD)
		return NoError; 			// This is an MMC command only!

	// Assign our context value
	pContext = (P_MM4_SDMMC_CONTEXT_T) pSDMMCP->pContext;
	pMM4_CNTL1 = (P_MM4_CNTL1) &pContext->pMMC4Reg->mm4_cntl1;

	// Set up State
	pSDMMCP->State = READ;

	// This requires a transfer over the data lines.
	pMM4_BLK_CNTL = (P_MM4_BLK_CNTL) &pContext->pMMC4Reg->mm4_blk_cntl;
	pMM4_BLK_CNTL->blk_cnt = 0;
	if (pSDMMCP->SDMA_Mode)
	{
		pMM4_BLK_CNTL->dma_bufsz = MM4_512_HOST_DMA_BDRY;
	    pContext->pMMC4Reg->mm4_sysaddr = (UINT_T) pBuffer;
	} //MMC_SDMA_MODE
	pSDMMCP->Trans.StartDiscardWords = 0;
	pSDMMCP->Trans.EndDiscardWords = 0;								// We'll take all 512 bytes
	pSDMMCP->Trans.TransWordSize = pSDMMCP->ReadBlockSize / 4; 		// Total Transfer Size including pre and post bytes
	pSDMMCP->Trans.LocalAddr = (UINT_T) pBuffer;
	pSDMMCP->Trans.WordIndex = 0;									// Stores Index of Current write position

	MM4_SendDataCommandNoAuto12(pSDMMCP, XLLP_MMC_CMD8, NO_ARGUMENT, MM4_SINGLE_BLOCK_TRAN, MM4_CARD_TO_HOST_DATA, MM4_RT_R1 | MM4_48_RES);

	// Wait for the Read to Complete
	Retval = MM4_WaitForOperationComplete(1000);

	//send CMD13 to check the status of the card
	Retval |= MM4_CheckCardStatus(pSDMMCP, 0x900, R1_LOCKEDCARDMASK);		// Make sure card is transfer mode

	pSDMMCP->State = READY;

	if (Retval != NoError)
		return SDMMC_SWITCH_ERROR;

	return NoError;
}

