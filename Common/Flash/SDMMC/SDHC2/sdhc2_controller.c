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

/******************************************************************************
**
**  FILENAME:       sdhc_controller.c
**
**  PURPOSE: MMC and SD specific low level controller routines for the MM4 controller
**
**
**
**
******************************************************************************/
#include "sdhc2_controller.h"
#include "misc.h"
#include "Errors.h"
#include "timer.h"

/******************************************************************************
  Description:
    Start MMC/SD Internal bus clock.  MUST be done to start MM4CLK!
    Only after starting bus clock, communication between
    controller and card is possible
  Input Parameters:
    pContext--Pointer to MMC context structure
  Output Parameters:
    None
  Returns:
    None
*******************************************************************************/
void MMC4StartInternalBusClock(P_MM4_SDMMC_CONTEXT_T pContext)
{
  P_MM4_CNTL2 pMM4_CNTL2;
  MM4_CNTL2_UNION MM4_cntl2;

  pMM4_CNTL2 = (P_MM4_CNTL2)((VUINT32_T) &pContext->pMMC4Reg->mm4_cntl2);
  MM4_cntl2.mm4_cntl2_value = *(VUINT_T *)pMM4_CNTL2;
  MM4_cntl2.mm4_cntl2_bits.inter_clk_en = 1;
  *(VUINT_T *)pMM4_CNTL2 = MM4_cntl2.mm4_cntl2_value;

  // Wait for clock to become stable. * TBD * Add timeout
  do
  {
  	MM4_cntl2.mm4_cntl2_value = *(VUINT_T *)pMM4_CNTL2;
  } while (!MM4_cntl2.mm4_cntl2_bits.inter_clk_stable);

  return;
}

/******************************************************************************
  Description:
    Stops the MMC/SD Internal bus clock.
  Input Parameters:
    pContext--Pointer to MMC context structure
  Output Parameters:
    None
  Returns:
    None
*******************************************************************************/
void MMC4StopInternalBusClock(P_MM4_SDMMC_CONTEXT_T pContext)
{
  P_MM4_CNTL2 pMM4_CNTL2;
  MM4_CNTL2_UNION MM4_cntl2;

  pMM4_CNTL2 = (P_MM4_CNTL2)((VUINT32_T) &pContext->pMMC4Reg->mm4_cntl2);
  MM4_cntl2.mm4_cntl2_value = *(VUINT_T *)pMM4_CNTL2;
  MM4_cntl2.mm4_cntl2_bits.inter_clk_en = 0;
  *(VUINT_T *)pMM4_CNTL2 = MM4_cntl2.mm4_cntl2_value;

  // Wait for clock to become stable. * TBD * Add timeout
  do
  {
  	MM4_cntl2.mm4_cntl2_value = *(VUINT_T *)pMM4_CNTL2;
  } while (MM4_cntl2.mm4_cntl2_bits.inter_clk_stable);

  return;
}


/******************************************************************************
  Description:
    Start MMC bus clock. Only after starting bus clock, communication between
    controller and card is possible
  Input Parameters:
    pContext--Pointer to MMC context structure
  Output Parameters:
    None
  Returns:
    None
*******************************************************************************/
void MMC4StartBusClock(P_MM4_SDMMC_CONTEXT_T pContext)
{
  P_MM4_CNTL2 pMM4_CNTL2;
  MM4_CNTL2_UNION MM4_cntl2;

  pMM4_CNTL2 = (P_MM4_CNTL2)((VUINT32_T) &pContext->pMMC4Reg->mm4_cntl2);
  MM4_cntl2.mm4_cntl2_value = *(VUINT_T *)pMM4_CNTL2;
  MM4_cntl2.mm4_cntl2_bits.mm4clken = 1;

  *(VUINT_T *)pMM4_CNTL2 = MM4_cntl2.mm4_cntl2_value;
  return;
}

/******************************************************************************
  Description:
    Stops MMC bus clock.
  Input Parameters:
    pContext--Pointer to MMC context structure
  Output Parameters:
    None
  Returns:
    None
*******************************************************************************/
void MMC4StopBusClock (P_MM4_SDMMC_CONTEXT_T pContext)
{
  UINT32_T retry_count = 0xff;
  P_MM4_CNTL2 pMM4_CNTL2;
  MM4_CNTL2_UNION MM4_cntl2;

  // Request bus clock stop
  pMM4_CNTL2 = (P_MM4_CNTL2)((VUINT32_T) &pContext->pMMC4Reg->mm4_cntl2);
  MM4_cntl2.mm4_cntl2_value = *(VUINT_T *)pMM4_CNTL2;
  MM4_cntl2.mm4_cntl2_bits.mm4clken = 0;
  *(VUINT_T *)pMM4_CNTL2 = MM4_cntl2.mm4_cntl2_value;

  return;
}

/******************************************************************************
  Description:
    Set a new MMC bus clock rate. This function stops and resumes bus clock.
  Input Parameters:
    pContext
      Pointer to MMC context structure
    rate
      bus clock speed
  Output Parameters:
    None
  Returns:
    None
*******************************************************************************/
void MMC4SetBusRate(P_MM4_SDMMC_CONTEXT_T pContext, UINT_T rate)
{
  P_MM4_CNTL2 pMM4_CNTL2;
  MM4_CNTL2_UNION MM4_cntl2;

  // Request bus clock stop, set rate, start clock.
  MMC4StopBusClock (pContext);
  pMM4_CNTL2 = (P_MM4_CNTL2)((VUINT32_T) &pContext->pMMC4Reg->mm4_cntl2);
  MM4_cntl2.mm4_cntl2_value = *(VUINT_T *)pMM4_CNTL2;

  // Update the rate and start the clock.
  MM4_cntl2.mm4_cntl2_bits.sd_freq_sel_lo = (rate & 0xFF);
  MM4_cntl2.mm4_cntl2_bits.sd_freq_sel_hi = ((rate >> 8) & 3);
  MM4_cntl2.mm4_cntl2_bits.mm4clken = 1;
  *(VUINT_T *)pMM4_CNTL2 = MM4_cntl2.mm4_cntl2_value;

  // Make sure internal bus clock also enabled.
  MMC4StartInternalBusClock(pContext);

  return;
}


/******************************************************************************
  Description:
    This routine unmasks and enables or masks and disables required interrupts
    needed by the driver
    Input Parameters:
    	pContext
      		Pointer to MMC context structure
			Desire - Enable or Disable the interrupts
  Output Parameters:
    None
  Returns:
    None
*******************************************************************************/
void MMC4EnableDisableIntSources(P_MM4_SDMMC_CONTEXT_T pContext, UINT8_T Desire)
{
  P_MM4_I_STAT pMM4_I_STAT;
  MM4_I_STAT_UNION MM4_i_stat;

  // Capture existing Value
  pMM4_I_STAT = (P_MM4_I_STAT)((VUINT32_T) &pContext->pMMC4Reg->mm4_i_sig_en);

  MM4_i_stat.mm4_i_stat_value = *(VUINT_T*)pMM4_I_STAT;
  // Route the interrupt signal enable register
  MM4_i_stat.mm4_i_stat_bits.cmdcomp = Desire;
  MM4_i_stat.mm4_i_stat_bits.xfrcomp = Desire;
  MM4_i_stat.mm4_i_stat_bits.dmaint = Desire;
  MM4_i_stat.mm4_i_stat_bits.bufwrrdy = Desire;
  MM4_i_stat.mm4_i_stat_bits.bufrdrdy = Desire;
  MM4_i_stat.mm4_i_stat_bits.errint = Desire;
  MM4_i_stat.mm4_i_stat_bits.ctoerr = Desire;
  MM4_i_stat.mm4_i_stat_bits.cenderr = Desire;
  MM4_i_stat.mm4_i_stat_bits.dtoerr = Desire;
  MM4_i_stat.mm4_i_stat_bits.dcrcerr = Desire;
  MM4_i_stat.mm4_i_stat_bits.denderr = Desire;
#if !BOOTROM
  // will enable this in the bootrom after any changes in behavior have been investigated
  MM4_i_stat.mm4_i_stat_bits.crc_stat_err = Desire;
#endif
  // Write it out
  *(VUINT_T*)pMM4_I_STAT = MM4_i_stat.mm4_i_stat_value;

  // Now remove the masks
  pMM4_I_STAT = (P_MM4_I_STAT)((VUINT32_T) &pContext->pMMC4Reg->mm4_i_stat_en);
  MM4_i_stat.mm4_i_stat_value = *(VUINT_T*)pMM4_I_STAT;
  MM4_i_stat.mm4_i_stat_bits.cmdcomp = Desire;
  MM4_i_stat.mm4_i_stat_bits.xfrcomp = Desire;
  MM4_i_stat.mm4_i_stat_bits.dmaint = Desire;
  MM4_i_stat.mm4_i_stat_bits.bufwrrdy = Desire;
  MM4_i_stat.mm4_i_stat_bits.bufrdrdy = Desire;
  MM4_i_stat.mm4_i_stat_bits.errint = Desire;
  MM4_i_stat.mm4_i_stat_bits.ctoerr = Desire;
  MM4_i_stat.mm4_i_stat_bits.cenderr = Desire;
  MM4_i_stat.mm4_i_stat_bits.dtoerr = Desire;
  MM4_i_stat.mm4_i_stat_bits.dcrcerr = Desire;
  MM4_i_stat.mm4_i_stat_bits.denderr = Desire;
#if !BOOTROM
  // will enable this in the bootrom after any changes in behavior have been investigated
  MM4_i_stat.mm4_i_stat_bits.crc_stat_err = Desire;
#endif
  // Write it out
  *(VUINT_T*)pMM4_I_STAT = MM4_i_stat.mm4_i_stat_value;

  return;
}

/******************************************************************************
  Description:
    Set the data response timeout value.
  Input Parameters:
    pContext
      Pointer to MMC context structure
    CounterValue
      the value which will be written into DTOCNTR
  Output Parameters:
    None
  Returns:
    None  
*******************************************************************************/
void MMC4SetDataTimeout(P_MM4_SDMMC_CONTEXT_T pContext, UINT8_T CounterValue)
{
 P_MM4_CNTL2 pMM4_CNTL2;
 MM4_CNTL2_UNION MM4_cntl2;

 // Set the register
 pMM4_CNTL2 = (P_MM4_CNTL2)((VUINT32_T) &pContext->pMMC4Reg->mm4_cntl2);
 MM4_cntl2.mm4_cntl2_value = *(VUINT_T *)pMM4_CNTL2;
 MM4_cntl2.mm4_cntl2_bits.dtocntr = CounterValue;

 // Write Back
 *(VUINT_T *)pMM4_CNTL2 = MM4_cntl2.mm4_cntl2_value;
 return;
}

/******************************************************************************
  Description:
    This function will induce a software reset of all MMC4 data lines
  Input Parameters:
    pContext
      Pointer to MMC context structure
  Output Parameters:
    None
  Returns:
    None
*******************************************************************************/
void MMC4CMDSWReset(P_MM4_SDMMC_CONTEXT_T pContext)
{
 P_MM4_CNTL2 pMM4_CNTL2;
 MM4_CNTL2_UNION MM4_cntl2;

 // Set the register
 pMM4_CNTL2 = (P_MM4_CNTL2)((VUINT32_T) &pContext->pMMC4Reg->mm4_cntl2);
 MM4_cntl2.mm4_cntl2_value = *(VUINT_T *)pMM4_CNTL2;
 MM4_cntl2.mm4_cntl2_bits.cmdswrst = 1;

 // Write Back
 *(VUINT_T *)pMM4_CNTL2 = MM4_cntl2.mm4_cntl2_value;
 return;
}

/******************************************************************************
  Description:
    This function will induce a software reset of all MMC4 data lines
  Input Parameters:
    pContext
      Pointer to MMC context structure
  Output Parameters:
    None
  Returns:
    None
*******************************************************************************/
void MMC4DataSWReset(P_MM4_SDMMC_CONTEXT_T pContext)
{
 P_MM4_CNTL2 pMM4_CNTL2;
 MM4_CNTL2_UNION MM4_cntl2;

 // Set the register
 pMM4_CNTL2 = (P_MM4_CNTL2)((VUINT32_T) &pContext->pMMC4Reg->mm4_cntl2);
 MM4_cntl2.mm4_cntl2_value = *(VUINT_T *)pMM4_CNTL2;
 MM4_cntl2.mm4_cntl2_bits.datswrst = 1;

 // Write Back
 *(VUINT_T *)pMM4_CNTL2 = MM4_cntl2.mm4_cntl2_value;
 return;
}

/******************************************************************************
  Description:
    This function will induce a full software reset of all MMC4 components except
	MM4_CAPX
  Input Parameters:
    pContext
      Pointer to MMC context structure
  Output Parameters:
    None
  Returns:
    None
*******************************************************************************/
void MMC4FullSWReset(P_MM4_SDMMC_CONTEXT_T pContext)
{
 P_MM4_CNTL2 pMM4_CNTL2;
 MM4_CNTL2_UNION MM4_cntl2;

 // Set the register
 pMM4_CNTL2 = (P_MM4_CNTL2)((VUINT32_T) &pContext->pMMC4Reg->mm4_cntl2);
 MM4_cntl2.mm4_cntl2_value = *(VUINT_T *)pMM4_CNTL2;
 MM4_cntl2.mm4_cntl2_bits.mswrst = 1;

 // Write Back
 *(VUINT_T *)pMM4_CNTL2 = MM4_cntl2.mm4_cntl2_value;
 return;
}

/******************************************************************************
  Description:
    Set up the registers of the controller to start the transaction to
    communicate to the card for data related command.  The commands are clearly defined in the MMC
    specification.
  Input Parameters:
    pContext
      	Pointer to MMC context structure
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
void MMC4SendDataCommand(P_MM4_SDMMC_CONTEXT_T pContext,
                  UINT_T Cmd,
                  UINT_T  Argument,
                  UINT_T BlockType,
                  UINT_T DataDirection,
                  UINT_T ResType, 
                  UINT_T SDMAMode)
{
 MM4_CMD_XFRMD_UNION xfrmd;
 P_MM4_STATE pMM4_STATE;

 // Make sure the controller is ready to accept the next command
 pMM4_STATE = (P_MM4_STATE) &pContext->pMMC4Reg->mm4_state;
 while (pMM4_STATE->dcmdinhbt)
 	{;}	// Wait.

 // Set the Argument Field
 pContext->pMMC4Reg->mm4_arg = Argument;

 // Set the Data Transfer Command fields.
 xfrmd.mm4_cmd_xfrmd_value = 0;
 xfrmd.mm4_cmd_xfrmd_bits.cmd_idx = Cmd;
 xfrmd.mm4_cmd_xfrmd_bits.cmd_type = MM4_CMD_TYPE_NORMAL;
 xfrmd.mm4_cmd_xfrmd_bits.dpsel = MM4_CMD_DATA;
// xfrmd.mm4_cmd_xfrmd_bits.idxchken = TRUE;
// xfrmd.mm4_cmd_xfrmd_bits.crcchken = TRUE;
 xfrmd.mm4_cmd_xfrmd_bits.res_type = ResType;
 xfrmd.mm4_cmd_xfrmd_bits.ms_blksel = BlockType;
 xfrmd.mm4_cmd_xfrmd_bits.dxfrdir = DataDirection;
 xfrmd.mm4_cmd_xfrmd_bits.autocmd12 = TRUE;
 xfrmd.mm4_cmd_xfrmd_bits.blkcbten = TRUE;

 if (SDMAMode)
	xfrmd.mm4_cmd_xfrmd_bits.dma_en = TRUE;

 // Kick off the command
 pContext->pMMC4Reg->mm4_cmd_xfrmd = xfrmd.mm4_cmd_xfrmd_value;
 return;
}


/******************************************************************************
  Description:
    Set up the registers of the controller to start the transaction to
    communicate to the card for data related command.  The commands are clearly defined in the MMC
    specification.
  Input Parameters:
    pContext
      	Pointer to MMC context structure
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
void MMC4SendDataCommandNoAuto12(P_MM4_SDMMC_CONTEXT_T pContext,
                  UINT_T Cmd,
                  UINT_T  Argument,
                  UINT_T BlockType,
                  UINT_T DataDirection,
                  UINT_T ResType, 
                  UINT_T SDMAMode)
{
 MM4_CMD_XFRMD_UNION xfrmd;
 P_MM4_STATE pMM4_STATE;

 // Make sure the controller is ready to accept the next command
 pMM4_STATE = (P_MM4_STATE) &pContext->pMMC4Reg->mm4_state;
 while (pMM4_STATE->dcmdinhbt)
 	{;}	// Wait.

 // Set the Argument Field
 pContext->pMMC4Reg->mm4_arg = Argument;

 // Set the Data Transfer Command fields.
 xfrmd.mm4_cmd_xfrmd_value = 0;
 xfrmd.mm4_cmd_xfrmd_bits.cmd_idx = Cmd;
 xfrmd.mm4_cmd_xfrmd_bits.cmd_type = MM4_CMD_TYPE_NORMAL;
 xfrmd.mm4_cmd_xfrmd_bits.dpsel = MM4_CMD_DATA;
// xfrmd.mm4_cmd_xfrmd_bits.idxchken = TRUE;
// xfrmd.mm4_cmd_xfrmd_bits.crcchken = TRUE;
 xfrmd.mm4_cmd_xfrmd_bits.res_type = ResType;
 xfrmd.mm4_cmd_xfrmd_bits.ms_blksel = BlockType;
 xfrmd.mm4_cmd_xfrmd_bits.dxfrdir = DataDirection;
 xfrmd.mm4_cmd_xfrmd_bits.autocmd12 = FALSE;
 xfrmd.mm4_cmd_xfrmd_bits.blkcbten = TRUE;

 if (SDMAMode)
	xfrmd.mm4_cmd_xfrmd_bits.dma_en = TRUE;

 // Kick off the command
 pContext->pMMC4Reg->mm4_cmd_xfrmd = xfrmd.mm4_cmd_xfrmd_value;
 return;
}

/******************************************************************************
  Description:
    Set up the registers of the controller to start the transaction to
    communicate to the card for setup related commands.
    The commands are clearly defined in the MMC specification.
  Input Parameters:
    pContext
      	Pointer to MMC context structure
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
void MMC4SendSetupCommand(P_MM4_SDMMC_CONTEXT_T pContext,
                  UINT_T Cmd,
                  UINT_T CmdType,
                  UINT_T Argument,
                  UINT_T ResType)
{
 MM4_CMD_XFRMD_UNION xfrmd;
 P_MM4_STATE pMM4_STATE;
 UINT_T startTime, endTime;

 startTime = GetOSCR0();  // Dummy read to flush potentially bad data
 startTime = GetOSCR0();  // get the start time
 endTime = startTime;

 // Make sure the controller is ready to accept the next command
 pMM4_STATE = (P_MM4_STATE) &pContext->pMMC4Reg->mm4_state;

 // Wait for the Response based on the CommandComplete interrupt signal
 while (pMM4_STATE->ccmdinhbt)
 {
       endTime = GetOSCR0();
       if (endTime < startTime)
              endTime += (0x0 - startTime);

       if (OSCR0IntervalInMilli(startTime, endTime) > 200)
              return;	// Let the getresponse routine catch the error

 }

 // Set the Argument Field
 pContext->pMMC4Reg->mm4_arg = Argument;

 // Set the Data Transfer Command fields.
 xfrmd.mm4_cmd_xfrmd_value = 0;
 xfrmd.mm4_cmd_xfrmd_bits.cmd_idx = Cmd;
 xfrmd.mm4_cmd_xfrmd_bits.cmd_type = CmdType;
// xfrmd.mm4_cmd_xfrmd_bits.idxchken = TRUE;
 xfrmd.mm4_cmd_xfrmd_bits.crcchken = TRUE;
 xfrmd.mm4_cmd_xfrmd_bits.res_type = ResType;

 // Kick off the command
 pContext->pMMC4Reg->mm4_cmd_xfrmd = xfrmd.mm4_cmd_xfrmd_value;
 return;
}
