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
**  FILENAME: dma.c
**
**
******************************************************************************/
/*
(C) Copyright [date] Marvell International Ltd.  
 All Rights Reserved  
*/

/*
Filename:	dma.c
Purpose:	Contains DMA functions configuring Dynamic Memory Accesses
*/


#include "xllp_dmac.h"
#include "Interrupts.h"
#include "Errors.h"
#include "misc.h"

static volatile P_XLLP_DMAC_T pDmacHandle = (P_XLLP_DMAC_T)(volatile unsigned long *)( DMA_BASE );

P_XLLP_DMAC_T XLLP_Return_PDmacHandle()
{
	return pDmacHandle;
}

/* alignChannel
 *
 *  This function aligns the channel to either default or user alignment
 *
 *	Inputs: Channel    : channel number
 *  		user_align : 1 = user aligned (source and target addresses)
 *			           : 0 = default aligned (8 byte alignment)
 */
void alignChannel(unsigned int channel, unsigned int user_align)
{
	if(user_align != 0)
	{
		user_align = 1;
		pDmacHandle->DALGN |= (user_align << channel);
	}
	else
		pDmacHandle->DALGN &= ~(1 << channel);
}

/*  configDescriptor
 *
 *
 * This function fills out a full (4 word) descriptor:
 *  DADR - pointer to the NEXT descriptor
 *  TADR - address of Target
 *  SADR - address of Source
 *  CMD  - Command value
 */
void configDescriptor(	P_XLLP_DMAC_DESCRIPTOR_T	pDesc,
						P_XLLP_DMAC_DESCRIPTOR_T 	pNextDesc,
						unsigned int				aSrcAddr,
						unsigned int				aTargetAddr,
						P_DMA_CMDx_T				pCMD,
						unsigned int				length,			  
						unsigned int				StopBit
						)
{
	pDesc->DDADR = ((unsigned int)pNextDesc & XLLP_DMAC_DDADR_RESERVED_MASK) | (StopBit & 0x1);
	pDesc->DSADR = aSrcAddr;
	pDesc->DTADR = aTargetAddr;
	
	pCMD->bits.Length = length;
	pDesc->DCMD = pCMD->value;
}

/* loadDescriptor
 *
 *	This function loads a descriptor into the correct DDADR register (based on Channel #)
 *
 *	NOTE: also ensure the Descriptor fetch is enabled
 *
 */
void loadDescriptor (P_XLLP_DMAC_DESCRIPTOR_T pDesc, XLLP_DMAC_CHANNEL_T aChannel)
{
	pDmacHandle->DDG[aChannel].DDADR = (unsigned int) pDesc;
	XllpDmacDescriptorFetch( aChannel );
}

/* loadNonDescriptor
 * 
 *   This function is used for NON descriptor fetch DMA operations in order to set the
 *      correct Source, Target and Command registers
 */
void loadNonDescriptor (unsigned int Source, unsigned int Target, P_DMA_CMDx_T pCMD, XLLP_DMAC_CHANNEL_T aChannel)
{
	pDmacHandle->DDG[aChannel].DCMD = pCMD->value;
	pDmacHandle->DDG[aChannel].DSADR = Source;
	pDmacHandle->DDG[aChannel].DTADR = Target;
}
/******************************************************************************
XLLP_DOC_HDR_BEGIN

  Function Name: XllpDmacMapDeviceToChannel

  Description: XllpDmacMapDeviceToChannel is used to map the specified device
  			   to the specified DMA channel.

  Global Registers Modified:
	DRCMRx register, where x denotes the channel number.

  Input Arguments:
	aDeviceDrcmr:  Specifies the device to be mapped to the DMA channel.  Valid
			  parameters are values of type  XLLP_DMAC_DRCMR_T, otherwise
			  invalid.
	aChannel: Specifies the DMA channel to map the specified device to.  Valid
			  channel values range from 0-31.

  Output Arguments:
	None
  Return Value:
	None

XLLP_DOC_HDR_END
*******************************************************************************/
void XllpDmacMapDeviceToChannel(
							   DMAC_DEVICE_T   aDeviceDrcmr,
							   XLLP_DMAC_CHANNEL_T aChannel
						       )
{
	XLLP_UINT32_T DrcmrValue = 0;
	if (aDeviceDrcmr < XLLP_DMAC_DRCMR1_NUM)
	{
		/* Enable device-to-channel mapping */
		pDmacHandle->DRCMR1[aDeviceDrcmr] = (XLLP_DMAC_DRCMR_ENABLE + aChannel);
	}
	else
	{
		DrcmrValue = (aDeviceDrcmr - XLLP_DMAC_DRCMR1_NUM);
		/* Enable device-to-channel mapping */
		pDmacHandle->DRCMR2[DrcmrValue] = (XLLP_DMAC_DRCMR_ENABLE + aChannel);
	}
}

/******************************************************************************
XLLP_DOC_HDR_BEGIN

  Function Name: XllpDmacUnMapDeviceToChannel

  Description: XllpDmacUnMapDeviceToChannel is used to unmap the specified
  			   device from the specified DMA channel.

  Global Registers Modified:
	DRCMRx register, where x denotes the channel number.

  Input Arguments:
	aDeviceDrcmr:  Specifies the device to be unmapped from the DMA channel.  Valid
			  parameters are values of type XLLP_DMAC_DRCMR_T, otherwise invalid.
	aChannel: Specifies the DMA channel to unmap the specified device from.
			  Valid channel values range from 0-31.

  Output Arguments:
	None
  Return Value:
	None

XLLP_DOC_HDR_END
*******************************************************************************/
void XllpDmacUnMapDeviceToChannel(
								 DMAC_DEVICE_T   aDeviceDrcmr,
								 XLLP_DMAC_CHANNEL_T aChannel
								 )
{
	XLLP_UINT32_T DrcmrValue = 0;
	if (aDeviceDrcmr < XLLP_DMAC_DRCMR1_NUM)
	{
		/* Disable device-to-channel mapping */
		pDmacHandle->DRCMR1[aDeviceDrcmr] = (XLLP_DMAC_DRCMR_DISABLE + aChannel);
	}
	else
	{
		DrcmrValue = (aDeviceDrcmr - XLLP_DMAC_DRCMR1_NUM);
		/* Disable device-to-channel mapping */
		pDmacHandle->DRCMR2[DrcmrValue] = (XLLP_DMAC_DRCMR_DISABLE + aChannel);
	}
}

/******************************************************************************
XLLP_DOC_HDR_BEGIN

  Function Name: XllpDmacStartTransfer

  Description: XllpDmacStartTransfer starts the DMA transfer process for the
  			   specified channel.

  Global Registers Modified:
	DCSRx register, where x indicate the channel number

  Input Arguments:
	aChannel:  Specifies the channel to start the DMA transfer on.

  Output Arguments:
  	None
  Return Value:
	None

XLLP_DOC_HDR_END
*******************************************************************************/
void XllpDmacStartTransfer( XLLP_DMAC_CHANNEL_T aChannel )
{
	XLLP_VUINT32_T aTargetData;

	aTargetData  = pDmacHandle->DCSR[aChannel];
	aTargetData |= XLLP_DMAC_DCSR_RUN;
	pDmacHandle->DCSR[aChannel]  =	aTargetData;
	//pDmacHandle->DCSR[aChannel]  |= XLLP_DMAC_DCSR_RUN;
}

/******************************************************************************
XLLP_DOC_HDR_BEGIN

  Function Name: XllpDmacStopTransfer

  Description: XllpDmacStopTransfer stops the DMA transfer process for the
  			   specified channel.

  Global Registers Modified:
	DCSRx register, where x indicate the channel number

  Input Arguments:
	aChannel:  Specifies the channel to stop the DMA transfer on.

  Output Arguments:
  	None
  Return Value:
	None

XLLP_DOC_HDR_END
*******************************************************************************/

void XllpDmacStopTransfer( XLLP_DMAC_CHANNEL_T aChannel )
{
	XLLP_VUINT32_T aTargetValue;

	aTargetValue  = pDmacHandle->DCSR[aChannel];
	aTargetValue &= ~XLLP_DMAC_DCSR_RUN;
	pDmacHandle->DCSR[aChannel]  =	aTargetValue;
	//pDmacHandle->DCSR[aChannel]  &= ~XLLP_DMAC_DCSR_RUN;
}

void XllpDmacEnableStopIrqInterrupt( XLLP_DMAC_CHANNEL_T aChannel )
{
	XLLP_VUINT32_T aTargetData;

	aTargetData  = pDmacHandle->DCSR[aChannel];
	aTargetData |= XLLP_DMAC_DCSR_STOP_IRQ_EN;
	pDmacHandle->DCSR[aChannel]  =	aTargetData;
}

void XllpDmacNoDescriptorFetch( XLLP_DMAC_CHANNEL_T aChannel )
{
	XLLP_VUINT32_T aTargetData;

	aTargetData  = pDmacHandle->DCSR[aChannel];
	aTargetData |= XLLP_DMAC_DCSR_NO_DESC_FETCH;
	pDmacHandle->DCSR[aChannel]  =	aTargetData;
}

void XllpDmacDescriptorFetch( XLLP_DMAC_CHANNEL_T aChannel )
{
	XLLP_VUINT32_T aTargetData;

	aTargetData  = pDmacHandle->DCSR[aChannel];
	aTargetData &= ~XLLP_DMAC_DCSR_NO_DESC_FETCH;
	pDmacHandle->DCSR[aChannel]  =	aTargetData;
}


void XllpDmacDisableStopIrqInterrupt( XLLP_DMAC_CHANNEL_T aChannel )
{
	XLLP_VUINT32_T aTargetData;

	aTargetData  = pDmacHandle->DCSR[aChannel];
	aTargetData &= ~XLLP_DMAC_DCSR_STOP_IRQ_EN;
	pDmacHandle->DCSR[aChannel]  =	aTargetData;
}

void XllpDmacDisableEORInterrupt( XLLP_DMAC_CHANNEL_T aChannel )
{
	XLLP_VUINT32_T aTargetData;

	aTargetData  = pDmacHandle->DCSR[aChannel];
	aTargetData &= ~XLLP_DMAC_DCSR_EOR_IRQ_EN;
	pDmacHandle->DCSR[aChannel]  =	aTargetData;
}

void XllpDmacEnableEORInterrupt( XLLP_DMAC_CHANNEL_T aChannel )
{
	XLLP_VUINT32_T aTargetData;

	aTargetData  = pDmacHandle->DCSR[aChannel];
	aTargetData |= XLLP_DMAC_DCSR_EOR_IRQ_EN;
	pDmacHandle->DCSR[aChannel]  =	aTargetData;
}

void XllpDmacClearDmaInterrupt( XLLP_DMAC_CHANNEL_T aChannel )
{
	XLLP_VUINT32_T aTargetData;

	aTargetData  = pDmacHandle->DCSR[aChannel];
	aTargetData &= ~XLLP_DMAC_DCSR_STOP_IRQ_EN;
	aTargetData |= DMA_STATUS_INTERRUPTS_MASK;
	pDmacHandle->DCSR[aChannel]  =	aTargetData;
}

unsigned int readDmaStatusRegister(XLLP_DMAC_CHANNEL_T aChannel)
{
	return pDmacHandle->DCSR[aChannel];
}








//don't compile in MEM2MEM stuff
#if 0

void Mem2Mem_ISR();
// This is the number of descriptors used for Memory to Memory moves
#define NumMem2MemDesc  10

extern void EnableIrqInterrupts();
extern void DisableIrqInterrupts();

// Used to move data from XIP to ISRAM if needed
static volatile XLLP_UINT8_T Mem2Mem_Complete = XLLP_FALSE;
static volatile XLLP_UINT32_T Mem2Mem_Error = NoError;

// ISR for a MEM2MEM descriptor based DMA move
void Mem2Mem_ISR()
{
	XLLP_UINT32_T DCSRValue;
	P_XLLP_DMAC_T pDmac_h = XLLP_Return_PDmacHandle();

	// Get copy of the DCSR
	DCSRValue = pDmac_h->DCSR[MEM2MEM_CHANNEL];

	// Wait for the channel to stop
	while ( (pDmac_h->DCSR[MEM2MEM_CHANNEL] & (1 << XLLP_DMAC_STOP_INT)) == 0)
	{}

	// Clear the interrupts source
	XllpDmacClearDmaInterrupt(MEM2MEM_CHANNEL);

	// This set of descriptors completed.
	Mem2Mem_Complete = XLLP_TRUE;

	// Return if last set failed
	if (Mem2Mem_Error == ReadError)
		return;

	if ((DCSRValue & (1 << XLLP_DMAC_END_INT)) == (1 << XLLP_DMAC_END_INT))
		Mem2Mem_Error = NoError;
	if ((DCSRValue & (1 << XLLP_DMAC_BUS_ERR_INT)) == (1 << XLLP_DMAC_BUS_ERR_INT))
		Mem2Mem_Error = ReadError;

	return;
}

XLLP_UINT32_T Mem2Mem(XLLP_UINT32_T source, XLLP_UINT32_T destination, XLLP_UINT32_T size)
{
	XLLP_UINT32_T TransferSize = 0;	// Transfer Size of the specific descriptor
	XLLP_UINT32_T i;
	XLLP_DMAC_COMMAND_T LocalCopyDmaCommand;
	XLLP_DMAC_DESC_ENABLE_T ChanControl;
	XLLP_DMAC_CHANNEL_T LocalMemCopyChannel = MEM2MEM_CHANNEL;
	P_XLLP_DMAC_DESCRIPTOR_T pLocalCopyDesc;
	//attribute aligned command not compatible with MS compiler
	//static  __attribute__((aligned(16))) XLLP_DMAC_DESCRIPTOR_T LocalCopyDesc[NumMem2MemDesc];
	//instead, we need to allocate space and overlay the descriptor at the desired 16 byte offset 
	XLLP_UINT32_T buffer[4*NumMem2MemDesc+4];
	pLocalCopyDesc = (P_XLLP_DMAC_DESCRIPTOR_T)(((XLLP_UINT32_T)buffer & 0xFFFFFFF0) + 0x10);

	// Enable DMA interrupts
	EnablePeripheralIRQInterrupt(DMA_CNTL_INT);

	// Set up the Command - clear the stack based struct
	//--------------------------------------------------
	memset( &LocalCopyDmaCommand, 0, sizeof(XLLP_DMAC_COMMAND_T));
	LocalCopyDmaCommand.aWidth      = XLLP_DMAC_WIDTH_16;
	LocalCopyDmaCommand.aSize       = XLLP_DMAC_BURSTSIZE_16;
	LocalCopyDmaCommand.aIncTrgAddr = 1;
	LocalCopyDmaCommand.aIncSrcAddr = 1;

	while (size > 0)
	{
		Mem2Mem_Complete = FALSE;					   // This set of descriptors have not completed
		LocalCopyDmaCommand.aEndIrqEn   = 0;		   // Don't produce an interrupt until the last descriptor
		ChanControl = XLLP_DMAC_DESC_RUN_CHANNEL;	   // Run until last descriptor

		// Configure descriptors for 4K transfers
		for (i=0; (size > 0) && (i < NumMem2MemDesc - 1); i++)
		{
			source += TransferSize;
			destination += TransferSize;
			TransferSize = (size > 0x1F00) ? 0x1F00 : size;	 // Transfer about 8KB at a time, descriptor limit 8K-1
			size -= TransferSize;
			LocalCopyDmaCommand.aLen = TransferSize;

			if ((size == 0) || (i == NumMem2MemDesc-2))
			{
				LocalCopyDmaCommand.aEndIrqEn   = 1;
				ChanControl = XLLP_DMAC_DESC_STOP_CHANNEL;
			}

			// Fill chained descriptors
			XllpDmacFillLinkedDesc(
				&pLocalCopyDesc[i],
				&pLocalCopyDesc[i+1],
				ChanControl,
				XLLP_DMAC_DISABLE_DESC_BRANCH,
				source,
				destination,
				&LocalCopyDmaCommand
				);
		}

		// Configure for descriptor fetch
		XllpDmacCfgChannelDescTransfer(
			&pLocalCopyDesc[0],
			LocalMemCopyChannel,
			XLLP_DMAC_MEM2MEM_MOVE,
			XLLP_DMAC_ALIGNMENT_ON
			);

		XllpDmacStartTransfer(LocalMemCopyChannel);

		// Wait for the ISR...
		while (!Mem2Mem_Complete) {}
		if (Mem2Mem_Error != NoError)
		{
			DisablePeripheralIRQInterrupt(DMA_CNTL_INT);
			return Mem2Mem_Error;
		}
	}

	DisablePeripheralIRQInterrupt(DMA_CNTL_INT);
	return Mem2Mem_Error;
}

#endif
