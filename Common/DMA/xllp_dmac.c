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
**  FILENAME: xllp_dmac.c
**
**
******************************************************************************/
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
**  FILENAME:       xllp_dmac.c
**
**  PURPOSE: contains all primitive functions for Bulverde DMA Controller register
**           access and control
**
******************************************************************************/

#include "xllp_dmac.h"

#define XLLP_DMAC_DDADR_RESERVED_MASK	 0xFFFFFFF0
#define XLLP_DMAC_DRCMR_ENABLE     		 0x80
#define XLLP_DMAC_DRCMR_DISABLE    		 0x00

// This buffer is set aside for use in allocating memory
// for descriptor used in the JTAG re-enabling process
char 	GlobalDmaReservedSpace[GlobalDmaBufferSize];
int 	CurrentAddr = 0;


/**
 * DMA Controller channels
 **/
//static XLLP_DMAC_CHANNEL_T pArrayChannel[32];

/**
 * Points to shared area with all DMAC registers
 *
 * MUST DO:
 * Modify to reflect the DMAC register block of your operating environment.
 **/
static volatile P_XLLP_DMAC_T		pDmacHandle = (P_XLLP_DMAC_T)(volatile unsigned long *)( 0x40000000 );

P_XLLP_DMAC_T XLLP_Return_PDmacHandle()
{
	return pDmacHandle;
}

void XllpDmacEnableStopIrqInterrupt( XLLP_DMAC_CHANNEL_T aChannel )
{
	XLLP_VUINT32_T aTargetData;

	aTargetData  = pDmacHandle->DCSR[aChannel];
	aTargetData |= XLLP_DMAC_DCSR_STOP_IRQ_EN;
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

void XllpDmacReadStatusRegister(XLLP_DMAC_CHANNEL_T aChannel, XLLP_UINT32_T *aStatus)
{
	*aStatus  = pDmacHandle->DCSR[aChannel];
}


/******************************************************************************
XLLP_DOC_HDR_BEGIN

  Function Name: XllpDmacFillLinkedDesc

  Description: XllpDmacFillLinkedDesc is used to setup a descriptor or chained
  			   descriptors for DMA transfer operations. To setup a descriptor
  			   (ie. no chaining), the pointer to the next descriptor pNextDesc
  			   is expected to be zero, otherwise pNextDesc expected to be a
  			   valid descriptor pointer.  Users wanting to chain multiple
  			   descriptors will call this function to accomplish that task.

  Global Registers Modified:
	None

  Input Arguments:
	pDesc: Hold the pointer to the descriptor to be configured, cannot be a
		   null pointer.
	pNextDesc: Holds the pointer to the next descriptor used in the DMA transfer
			   process after completely transferring data in the current descriptor
			   pDesc if the channel is not programmed to be stop.  pNextDesc can
			   be a null pointer only if this descriptor is not meant to be chained.
	aStopContinue:  Specifies whether or not to configure this descriptor to stop
				    the channel after completely transferring data in this descriptor.
	aBranch: Specifies whether to configure this descriptor to enable or disable
			 the descriptor branching mechanism.  Valid values are element of type
			 XLLP_DMAC_DESC_BRANCH_T (1 or 0).
	aSrcAddr: Holds the source address for this descriptor, cannot be null.
	aTargetAddr: Holds the target address for this descriptor, cannot be null.
	pCmd: Holds the commands for this descriptor.  Users are required to initialize
		  the various fields of pCmd data structure before making this call.

  Output Arguments:
  	None
  Return Value:
	None

XLLP_DOC_HDR_END
*******************************************************************************/
void XllpDmacFillLinkedDesc(
						   P_XLLP_DMAC_DESCRIPTOR_T pDesc,
						   P_XLLP_DMAC_DESCRIPTOR_T pNextDescPhyAddr,
						   XLLP_DMAC_DESC_ENABLE_T  aStopContinue,
						   XLLP_DMAC_DESC_BRANCH_T  aBranch,
						   XLLP_UINT32_T            aSrcAddr,
						   XLLP_UINT32_T            aTargetAddr,
						   XLLP_DMAC_COMMAND_T*     pCmd
						  )
{
	XLLP_VUINT32_T aTargetValue;

	/* Construct command value from pCmd  */
	XLLP_UINT32_T aCommand =  (XLLP_UINT32_T)(
								( pCmd->aLen) | ((pCmd->aWidth) << XLLP_BIT_FIELD_14)|
								((pCmd->aSize) << XLLP_BIT_FIELD_16)      |
								((pCmd->aEndian) << XLLP_BIT_FIELD_18)    |
								((pCmd->aFlyByT) << XLLP_BIT_FIELD_19)    |
								((pCmd->aFlyByS) << XLLP_BIT_FIELD_20)    |
								((pCmd->aEndIrqEn)   << XLLP_BIT_FIELD_21)|
								((pCmd->aStartIrqEn) << XLLP_BIT_FIELD_22)|
								((pCmd->aAddrMode)   << XLLP_BIT_FIELD_23)|
								((pCmd->aCmpEn)   << XLLP_BIT_FIELD_25)   |
								((pCmd->aFlowTrg) << XLLP_BIT_FIELD_28)   |
								((pCmd->aFlowSrc) << XLLP_BIT_FIELD_29)   |
								((pCmd->aIncTrgAddr) << XLLP_BIT_FIELD_30)|
								((pCmd->aIncSrcAddr) << XLLP_BIT_FIELD_31)
							   );

	/* Clear reserved bit fields of DDADR */
	XLLP_VUINT32_T aNextDesc = (XLLP_VUINT32_T)pNextDescPhyAddr;
	aNextDesc = (XLLP_VUINT32_T)(aNextDesc & XLLP_DMAC_DDADR_RESERVED_MASK);
	aTargetValue = (XLLP_VUINT32_T)(aNextDesc + (aBranch<<XLLP_BIT_FIELD_1) + aStopContinue);

	/* Fill descriptor entries            */
	pDesc->DDADR = (XLLP_VUINT32_T)(aNextDesc) + (aBranch<<XLLP_BIT_FIELD_1) + aStopContinue;
	pDesc->DSADR = (XLLP_VUINT32_T)aSrcAddr;
	pDesc->DTADR = (XLLP_VUINT32_T)aTargetAddr;
	pDesc->DCMD  = (XLLP_VUINT32_T)aCommand;
}


/******************************************************************************
XLLP_DOC_HDR_BEGIN

  Function Name: XllpDmacCfgChannelDescTransfer

  Description: XllpDmacCfgChannelDescTransfer configures the specified DMA
  			   channel for a Descriptor Mode transfer operation.  To configure
  			   a channel for a no descriptor mode transfer, use the
  			   XllpDmacCfgChannelNoDescTransfer primitive.  It is expected that
  			   the developer has already made calls to XllpDmacAllocChannel for
  			   an available channel and bound it to a service routine for interrupt
  			   that may occur on the channel, and XllpDmacFillLinkedDesc to
  			   configure the descriptor before calling this primitives.

  Global Registers Modified:
	DDADRx, DRCMRx registers.  DSADRx, DTADRx, and DCMDx register will be modified
	when the descriptor is loaded.  x indicates the channel number

  Input Arguments:
	pDesc:  Hold the pointer to the descriptor to be used in configuring the
			specified DMA channel.  The pointer cannot be null.
	aChannel:  Specifies the channel to be configured for the DMA transfer.
	aDeviceDrcmr:  Specifies the device to be mapped to the DMA channel for the transfer
			  	   operation.
	aLignment: Indicated whether to enable byte alignment for memory accesses

  Output Arguments:
  	None
  Return Value:
	None

XLLP_DOC_HDR_END
*******************************************************************************/
void XllpDmacCfgChannelDescTransfer(
					    		   P_XLLP_DMAC_DESCRIPTOR_T  pDesc,
								   XLLP_DMAC_CHANNEL_T       aChannel,
								   //XLLP_DMAC_DRCMR_T         aDeviceDrcmr,
								   XLLP_DMAC_DEVICE_T        aDeviceDrcmr,
								   XLLP_DMAC_ALIGNMENT_T     aLignment
				     	  		   )
{
	XLLP_VUINT32_T aTargetData;

	if (aLignment == XLLP_DMAC_ALIGNMENT_ON)
	{
		aTargetData = pDmacHandle->DALGN;
		aTargetData |= (1 << aChannel);
		pDmacHandle->DALGN = aTargetData;
	}

	/* Map device to channel                 */
	/* No mapping required for mem2mem moves */
	if (aDeviceDrcmr != XLLP_DMAC_MEM2MEM_MOVE)
	{
		XllpDmacMapDeviceToChannel(aDeviceDrcmr, aChannel);
	}

	pDmacHandle->DDG[aChannel].DDADR = (XLLP_VUINT32_T)pDesc;
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
							   XLLP_DMAC_DRCMR_T   aDeviceDrcmr,
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
								 XLLP_DMAC_DRCMR_T   aDeviceDrcmr,
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

// -----------------------------------------------------------------------
// Function Name: GlobalDmaDescAlloc
//
// Description: Function is used to allocate a buffer for DMA descriptors
//
// Input Arguments:
//			RequestedSize: Size of buffer being requested
//
// Output Arguments: None
//
// Return Value:
//			Function returns a pointer to the start of the allocated buffer
// -----------------------------------------------------------------------
char *GlobalDmaDescAlloc(int RequestedSize)
{
	// Compute next buffer allocatable address & return starting address
	// of allocated buffer
	int AllocatedAddr;
	RequestedSize += 15;
	RequestedSize &= ~0xf;
	if (CurrentAddr == 0)
	{
		CurrentAddr = (0x10-((int)GlobalDmaReservedSpace))&0xf;
	}
	AllocatedAddr = CurrentAddr;
	CurrentAddr  += RequestedSize;
	return &GlobalDmaReservedSpace[AllocatedAddr];
}

