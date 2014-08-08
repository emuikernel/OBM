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
**	 xllp_udc.h
******************************************************************************//******************************************************************************
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
**  FILENAME:  		xllp_udc.h
**
**  PURPOSE:   		This files contains the definitions for UDC
**					(USB Device controller)
**
**  LAST MODIFIED:  1/25/2001
**
******************************************************************************/

#ifndef _xllp_udc_h
#define _xllp_udc_h

#include "xllp_defs.h"
#include "xllp_usbdefs.h"


/*
************************************************************************************
*							CONSTANTS
************************************************************************************
*/

// Masks for UDC Registers

// UDC Control Register (UDCCR)
#define XLLP_UDC_UDCCR_UDE          ( 0x1U << 0 )	// UDC enabled
#define XLLP_UDC_UDCCR_UDA			( 0x1U << 1 )	// READ-ONLY: udc is active
#define XLLP_UDC_UDCCR_UDR			( 0x1U << 2 )	// Forces the usb out of suspend state
#define XLLP_UDC_UDCCR_EMCE			( 0x1U << 3 )	// The Endpoint memory config. has an error
#define XLLP_UDC_UDCCR_SMAC			( 0x1U << 4 )	// Switch Endpoint memory to Active config.
                                                    // Active interface and Alternate Interface
#define XLLP_UDC_UDCCR_AAISN_SHIFT  5               // Shift and a mask for the Alternate Interface
#define XLLP_UDC_UDCCR_AAISN_MASK   ( 0x7U << XLLP_UDC_UDCCR_AAISN_SHIFT )  // Settings  (0-7)
#define XLLP_UDC_UDCCR_AIN_SHIFT    8               // Shift and a mask for the Interface
#define XLLP_UDC_UDCCR_AIN_MASK     ( 0x7U << XLLP_UDC_UDCCR_AIN_SHIFT )    // Number    (0-7)
#define XLLP_UDC_UDCCR_ACN_SHIFT    11              // Shift and a mask for the Configuration
#define XLLP_UDC_UDCCR_ACN_MASK     ( 0x3U << XLLP_UDC_UDCCR_ACN_SHIFT )    // Number    (0-3)
#define XLLP_UDC_UDCCR_DRWF			( 0x1U << 16 )	// Device Remote Wakeup Feature

// UDC Interrupt Control Register 0 (UDCICR0)
#define XLLP_UDC_UDCICR0_IE0_0		( 0x1U << 0 )	// Packet Complete Interrupt Enable - Endpoint 0
#define XLLP_UDC_UDCICR0_IE0_1		( 0x1U << 1 )   // FIFO Error Interrupt Enable - Endpoint 0
#define XLLP_UDC_UDCICR0_IEA_0		( 0x1U << 2 )	// Packet Complete Interrupt Enable - Endpoint A
#define XLLP_UDC_UDCICR0_IEA_1		( 0x1U << 3 )   // FIFO Error Interrupt Enable - Endpoint A
#define XLLP_UDC_UDCICR0_IEB_0		( 0x1U << 4 )	// Packet Complete Interrupt Enable - Endpoint B
#define XLLP_UDC_UDCICR0_IEB_1		( 0x1U << 5 )   // FIFO Error Interrupt Enable - Endpoint B
//#define XLLP_UDC_UDCICR0_IEC_0		( 0x1U << 6 )	// Packet Complete Interrupt Enable - Endpoint C
//#define XLLP_UDC_UDCICR0_IEC_1		( 0x1U << 7 )   // FIFO Error Interrupt Enable - Endpoint C
//#define XLLP_UDC_UDCICR0_IED_0		( 0x1U << 8 )	// Packet Complete Interrupt Enable - Endpoint D
//#define XLLP_UDC_UDCICR0_IED_1		( 0x1U << 9 )   // FIFO Error Interrupt Enable - Endpoint D
//#define XLLP_UDC_UDCICR0_IEE_0		( 0x1U << 10 )	// Packet Complete Interrupt Enable - Endpoint E
//#define XLLP_UDC_UDCICR0_IEE_1		( 0x1U << 11 )  // FIFO Error Interrupt Enable - Endpoint E
//#define XLLP_UDC_UDCICR0_IEF_0		( 0x1U << 12 )	// Packet Complete Interrupt Enable - Endpoint F
//#define XLLP_UDC_UDCICR0_IEF_1		( 0x1U << 13 )  // FIFO Error Interrupt Enable - Endpoint F
//#define XLLP_UDC_UDCICR0_IEG_0		( 0x1U << 14 )	// Packet Complete Interrupt Enable - Endpoint G
//#define XLLP_UDC_UDCICR0_IEG_1		( 0x1U << 15 )  // FIFO Error Interrupt Enable - Endpoint G
//#define XLLP_UDC_UDCICR0_IEH_0		( 0x1U << 16 )	// Packet Complete Interrupt Enable - Endpoint H
//#define XLLP_UDC_UDCICR0_IEH_1		( 0x1U << 17 )  // FIFO Error Interrupt Enable - Endpoint H
//#define XLLP_UDC_UDCICR0_IEI_0		( 0x1U << 18 )	// Packet Complete Interrupt Enable - Endpoint I
//#define XLLP_UDC_UDCICR0_IEI_1		( 0x1U << 19 )  // FIFO Error Interrupt Enable - Endpoint I
//#define XLLP_UDC_UDCICR0_IEJ_0		( 0x1U << 20 )	// Packet Complete Interrupt Enable - Endpoint J
//#define XLLP_UDC_UDCICR0_IEJ_1		( 0x1U << 21 )  // FIFO Error Interrupt Enable - Endpoint J
//#define XLLP_UDC_UDCICR0_IEK_0		( 0x1U << 22 )	// Packet Complete Interrupt Enable - Endpoint K
//#define XLLP_UDC_UDCICR0_IEK_1		( 0x1U << 23 )  // FIFO Error Interrupt Enable - Endpoint K
//#define XLLP_UDC_UDCICR0_IEL_0		( 0x1U << 24 )	// Packet Complete Interrupt Enable - Endpoint L
//#define XLLP_UDC_UDCICR0_IEL_1		( 0x1U << 25 )  // FIFO Error Interrupt Enable - Endpoint L
//#define XLLP_UDC_UDCICR0_IEM_0		( 0x1U << 26 )	// Packet Complete Interrupt Enable - Endpoint M
//#define XLLP_UDC_UDCICR0_IEM_1		( 0x1U << 27 )  // FIFO Error Interrupt Enable - Endpoint M
//#define XLLP_UDC_UDCICR0_IEN_0		( 0x1U << 28 )	// Packet Complete Interrupt Enable - Endpoint N
//#define XLLP_UDC_UDCICR0_IEN_1		( 0x1U << 29 )  // FIFO Error Interrupt Enable - Endpoint N
//#define XLLP_UDC_UDCICR0_IEP_0		( 0x1U << 30 )	// Packet Complete Interrupt Enable - Endpoint P
//#define XLLP_UDC_UDCICR0_IEP_1		( 0x1U << 31 )  // FIFO Error Interrupt Enable - Endpoint P

#define XLLP_UDC_UDCICR0_ENABLE_ALL 0xFFFFFFFE      // Mask to enable all endpoint A - P interrupts

// UDC Interrupt Control Register 1 (UDCICR1)
//#define XLLP_UDC_UDCICR1_IEQ_0		( 0x1U << 0 )	// Packet Complete Interrupt Enable - Endpoint Q
//#define XLLP_UDC_UDCICR1_IEQ_1		( 0x1U << 1 )   // FIFO Error Interrupt Enable - Endpoint Q
//#define XLLP_UDC_UDCICR1_IER_0		( 0x1U << 2 )	// Packet Complete Interrupt Enable - Endpoint R
//#define XLLP_UDC_UDCICR1_IER_1		( 0x1U << 3 )   // FIFO Error Interrupt Enable - Endpoint R
//#define XLLP_UDC_UDCICR1_IES_0		( 0x1U << 4 )	// Packet Complete Interrupt Enable - Endpoint S
//#define XLLP_UDC_UDCICR1_IES_1		( 0x1U << 5 )   // FIFO Error Interrupt Enable - Endpoint S
//#define XLLP_UDC_UDCICR1_IET_0		( 0x1U << 6 )	// Packet Complete Interrupt Enable - Endpoint T
//#define XLLP_UDC_UDCICR1_IET_1		( 0x1U << 7 )   // FIFO Error Interrupt Enable - Endpoint T
//#define XLLP_UDC_UDCICR1_IEU_0		( 0x1U << 8 )	// Packet Complete Interrupt Enable - Endpoint U
//#define XLLP_UDC_UDCICR1_IEU_1		( 0x1U << 9 )   // FIFO Error Interrupt Enable - Endpoint U
//#define XLLP_UDC_UDCICR1_IEV_0		( 0x1U << 10 )	// Packet Complete Interrupt Enable - Endpoint V
//#define XLLP_UDC_UDCICR1_IEV_1		( 0x1U << 11 )  // FIFO Error Interrupt Enable - Endpoint V
//#define XLLP_UDC_UDCICR1_IEW_0		( 0x1U << 12 )	// Packet Complete Interrupt Enable - Endpoint W
//#define XLLP_UDC_UDCICR1_IEW_1		( 0x1U << 13 )  // FIFO Error Interrupt Enable - Endpoint W
//#define XLLP_UDC_UDCICR1_IEX_0		( 0x1U << 14 )	// Packet Complete Interrupt Enable - Endpoint X
//#define XLLP_UDC_UDCICR1_IEX_1		( 0x1U << 15 )  // FIFO Error Interrupt Enable - Endpoint X
#define XLLP_UDC_UDCICR1_IERS		( 0x1U << 27 )  // Interrupt Enable - Reset
#define XLLP_UDC_UDCICR1_IESU		( 0x1U << 28 )  // Interrupt Enable - Suspend
#define XLLP_UDC_UDCICR1_IERU		( 0x1U << 29 )  // Interrupt Enable - Resume
#define XLLP_UDC_UDCICR1_IESOF		( 0x1U << 30 )  // Interrupt Enable - SOF
#define XLLP_UDC_UDCICR1_IECC		( 0x1U << 31 )  // Interrupt Enable - Configuration Change

#define XLLP_UDC_UDCICR1_EVENTS     ( 0x1FU << 27 ) // Mask to enable all event interrupts
#define XLLP_UDC_UDCICR1_ENABLE_ALL 0xFFFF          // Mask to enable all endpoint Q - X interrupts

// UDC Interrupt Status Register 0 (UDCICR0)
#define XLLP_UDC_UDCISR0_IR0_0		( 0x1U << 0 )	// Packet Complete Interrupt Request - Endpoint 0
#define XLLP_UDC_UDCISR0_IR0_1		( 0x1U << 1 )   // FIFO Error Interrupt Request - Endpoint 0
#define XLLP_UDC_UDCISR0_IRA_0		( 0x1U << 2 )	// Packet Complete Interrupt Request - Endpoint A
#define XLLP_UDC_UDCISR0_IRA_1		( 0x1U << 3 )   // FIFO Error Interrupt Request - Endpoint A
#define XLLP_UDC_UDCISR0_IRB_0		( 0x1U << 4 )	// Packet Complete Interrupt Request - Endpoint B
#define XLLP_UDC_UDCISR0_IRB_1		( 0x1U << 5 )   // FIFO Error Interrupt Request - Endpoint B
//#define XLLP_UDC_UDCISR0_IRC_0		( 0x1U << 6 )	// Packet Complete Interrupt Request - Endpoint C
//#define XLLP_UDC_UDCISR0_IRC_1		( 0x1U << 7 )   // FIFO Error Interrupt Request - Endpoint C
//#define XLLP_UDC_UDCISR0_IRD_0		( 0x1U << 8 )	// Packet Complete Interrupt Request - Endpoint D
//#define XLLP_UDC_UDCISR0_IRD_1		( 0x1U << 9 )   // FIFO Error Interrupt Request - Endpoint D
//#define XLLP_UDC_UDCISR0_IRE_0		( 0x1U << 10 )	// Packet Complete Interrupt Request - Endpoint E
//#define XLLP_UDC_UDCISR0_IRE_1		( 0x1U << 11 )  // FIFO Error Interrupt Request - Endpoint E
//#define XLLP_UDC_UDCISR0_IRF_0		( 0x1U << 12 )	// Packet Complete Interrupt Request - Endpoint F
//#define XLLP_UDC_UDCISR0_IRF_1		( 0x1U << 13 )  // FIFO Error Interrupt Request - Endpoint F
//#define XLLP_UDC_UDCISR0_IRG_0		( 0x1U << 14 )	// Packet Complete Interrupt Request - Endpoint G
//#define XLLP_UDC_UDCISR0_IRG_1		( 0x1U << 15 )  // FIFO Error Interrupt Request - Endpoint G
//#define XLLP_UDC_UDCISR0_IRH_0		( 0x1U << 16 )	// Packet Complete Interrupt Request - Endpoint H
//#define XLLP_UDC_UDCISR0_IRH_1		( 0x1U << 17 )  // FIFO Error Interrupt Request - Endpoint H
//#define XLLP_UDC_UDCISR0_IRI_0		( 0x1U << 18 )	// Packet Complete Interrupt Request - Endpoint I
//#define XLLP_UDC_UDCISR0_IRI_1		( 0x1U << 19 )  // FIFO Error Interrupt Request - Endpoint I
//#define XLLP_UDC_UDCISR0_IRJ_0		( 0x1U << 20 )	// Packet Complete Interrupt Request - Endpoint J
//#define XLLP_UDC_UDCISR0_IRJ_1		( 0x1U << 21 )  // FIFO Error Interrupt Request - Endpoint J
//#define XLLP_UDC_UDCISR0_IRK_0		( 0x1U << 22 )	// Packet Complete Interrupt Request - Endpoint K
//#define XLLP_UDC_UDCISR0_IRK_1		( 0x1U << 23 )  // FIFO Error Interrupt Request - Endpoint K
//#define XLLP_UDC_UDCISR0_IRL_0		( 0x1U << 24 )	// Packet Complete Interrupt Request - Endpoint L
//#define XLLP_UDC_UDCISR0_IRL_1		( 0x1U << 25 )  // FIFO Error Interrupt Request - Endpoint L
//#define XLLP_UDC_UDCISR0_IRM_0		( 0x1U << 26 )	// Packet Complete Interrupt Request - Endpoint M
//#define XLLP_UDC_UDCISR0_IRM_1		( 0x1U << 27 )  // FIFO Error Interrupt Request - Endpoint M
//#define XLLP_UDC_UDCISR0_IRN_0		( 0x1U << 28 )	// Packet Complete Interrupt Request - Endpoint N
//#define XLLP_UDC_UDCISR0_IRN_1		( 0x1U << 29 )  // FIFO Error Interrupt Request - Endpoint N
//#define XLLP_UDC_UDCISR0_IRP_0		( 0x1U << 30 )	// Packet Complete Interrupt Request - Endpoint P
//#define XLLP_UDC_UDCISR0_IRP_1		( 0x1U << 31 )  // FIFO Error Interrupt Request - Endpoint P

// UDC Interrupt Status Register 1 (UDCICR1)
//#define XLLP_UDC_UDCISR1_IRQ_0		( 0x1U << 0 )	// Packet Complete Interrupt Request - Endpoint Q
//#define XLLP_UDC_UDCISR1_IRQ_1		( 0x1U << 1 )   // FIFO Error Interrupt Request - Endpoint Q
//#define XLLP_UDC_UDCISR1_IRR_0		( 0x1U << 2 )	// Packet Complete Interrupt Request - Endpoint R
//#define XLLP_UDC_UDCISR1_IRR_1		( 0x1U << 3 )   // FIFO Error Interrupt Request - Endpoint R
//#define XLLP_UDC_UDCISR1_IRS_0		( 0x1U << 4 )	// Packet Complete Interrupt Request - Endpoint S
//#define XLLP_UDC_UDCISR1_IRS_1		( 0x1U << 5 )   // FIFO Error Interrupt Request - Endpoint S
//#define XLLP_UDC_UDCISR1_IRT_0		( 0x1U << 6 )	// Packet Complete Interrupt Request - Endpoint T
//#define XLLP_UDC_UDCISR1_IRT_1		( 0x1U << 7 )   // FIFO Error Interrupt Request - Endpoint T
//#define XLLP_UDC_UDCISR1_IRU_0		( 0x1U << 8 )	// Packet Complete Interrupt Request - Endpoint U
//#define XLLP_UDC_UDCISR1_IRU_1		( 0x1U << 9 )   // FIFO Error Interrupt Request - Endpoint U
//#define XLLP_UDC_UDCISR1_IRV_0		( 0x1U << 10 )	// Packet Complete Interrupt Request - Endpoint V
//#define XLLP_UDC_UDCISR1_IRV_1		( 0x1U << 11 )  // FIFO Error Interrupt Request - Endpoint V
//#define XLLP_UDC_UDCISR1_IRW_0		( 0x1U << 12 )	// Packet Complete Interrupt Request - Endpoint W
//#define XLLP_UDC_UDCISR1_IRW_1		( 0x1U << 13 )  // FIFO Error Interrupt Request - Endpoint W
//#define XLLP_UDC_UDCISR1_IRX_0		( 0x1U << 14 )	// Packet Complete Interrupt Request - Endpoint X
//#define XLLP_UDC_UDCISR1_IRX_1		( 0x1U << 15 )  // FIFO Error Interrupt Request - Endpoint X
#define XLLP_UDC_UDCISR1_IRRS		( 0x1U << 27 )  // Interrupt Request - Reset
#define XLLP_UDC_UDCISR1_IRSU		( 0x1U << 28 )  // Interrupt Request - Suspend
#define XLLP_UDC_UDCISR1_IRRU		( 0x1U << 29 )  // Interrupt Request - Resume
#define XLLP_UDC_UDCISR1_IRSOF		( 0x1U << 30 )  // Interrupt Request - SOF
#define XLLP_UDC_UDCISR1_IRCC		( 0x1U << 31 )  // Interrupt Request - Configuration Change

#define XLLP_UDC_UDCISR1_EVENTS     ( 0x1FU << 27 ) // Mask to clear all event interrupts

// UDC Endpoint 0 Control/Status Register (UDCCSR0)
#define XLLP_UDC_UDCCSR0_OPC		( 0x1U << 0 )	// OUT packet to endpoint zero received
#define XLLP_UDC_UDCCSR0_IPR		( 0x1U << 1 )	// Packet has been written to endpoint zero FIFO
#define XLLP_UDC_UDCCSR0_FTF		( 0x1U << 2 )	// Flush the Tx FIFO
#define XLLP_UDC_UDCCSR0_SST		( 0x1U << 4 )	// UDC sent stall handshake
#define XLLP_UDC_UDCCSR0_FST		( 0x1U << 5 )	// Force the UDC to issue a stall handshake
#define XLLP_UDC_UDCCSR0_RNE		( 0x1U << 6 )	// There is unread data in the Rx FIFO
#define XLLP_UDC_UDCCSR0_SA			( 0x1U << 7 )	// Current packet in FIFO is part of UDC setup command

// UDC Endpoint Control/Status Registers A-X
#define XLLP_UDC_UDCCSR_FS			( 0x1U << 0 )	// FIFO needs service
#define XLLP_UDC_UDCCSR_PC			( 0x1U << 1 )	// Packet Complete
#define XLLP_UDC_UDCCSR_TRN			( 0x1U << 2 )	// Receive data packet cannot be stored
#define XLLP_UDC_UDCCSR_EFE			( 0x1U << 2 )	// Endpoint FIFO error
#define XLLP_UDC_UDCCSR_DME			( 0x1U << 3 )	// DMA Enable
#define XLLP_UDC_UDCCSR_SST			( 0x1U << 4 )	// Sent STALL
#define XLLP_UDC_UDCCSR_FST			( 0x1U << 5 )	// Force STALL
#define XLLP_UDC_UDCCSR_BNE		    ( 0x1U << 6 )	// Buffer not empty/full
#define XLLP_UDC_UDCCSR_SP			( 0x1U << 7 )	// Short Packet
#define XLLP_UDC_UDCCSR_FEF			( 0x1U << 8 )	// Flash Endpoint FIFO
#define XLLP_UDC_UDCCSR_DPE			( 0x1U << 9 )	// Data Packet Error

// UDC Endpoint A-X Configuration Registers
#define XLLP_UDC_UDCCRZ_EE			( 0x1U << 0 )	// Endpoint Enable
#define XLLP_UDC_UDCCRZ_DE_SHIFT    1
#define XLLP_UDC_UDCCRZ_DE			( 0x1U << 1 )	// Double-buffering Enable
#define XLLP_UDC_UDCCRZ_MPS_SHIFT	2
#define XLLP_UDC_UDCCRZ_MPS_MASK    ( 0x3FFU << XLLP_UDC_UDCCRZ_MPS_SHIFT) // Maximum Packet Size
#define XLLP_UDC_UDCCRZ_ED_SHIFT    12
#define XLLP_UDC_UDCCRZ_ED			( 0x1U << 12 )	// Endpoint Direction
#define XLLP_UDC_UDCCRZ_ET_SHIFT    13
#define XLLP_UDC_UDCCRZ_ET_MASK     ( 0x3U << XLLP_UDC_UDCCRZ_ET_SHIFT)  // Endoint Type
#define XLLP_UDC_UDCCRZ_EN_SHIFT    15
#define XLLP_UDC_UDCCRZ_EN_MASK     ( 0xFU << XLLP_UDC_UDCCRZ_EN_SHIFT)  // Endoint Number
#define XLLP_UDC_UDCCRZ_AISN_SHIFT  19
#define XLLP_UDC_UDCCRZ_AISN_MASK   ( 0x7U << XLLP_UDC_UDCCRZ_AISN_SHIFT)  // Interface Alternate Settings Number
#define XLLP_UDC_UDCCRZ_IN_SHIFT    22
#define XLLP_UDC_UDCCRZ_IN_MASK     ( 0x7U << XLLP_UDC_UDCCRZ_IN_SHIFT)    // Interface Number
#define XLLP_UDC_UDCCRZ_CN_SHIFT    25
#define XLLP_UDC_UDCCRZ_CN_MASK     ( 0x3U << XLLP_UDC_UDCCRZ_CN_SHIFT)    // Configuration Number


#define XLLP_UDC_UDCBCR_BC_MASK     0x3ff

//#define XLLP_UDC_MAX_EP_NUM         24
#define XLLP_UDC_MAX_EP_NUM         4
#define XLLP_UDC_USB_MAX_EP_NUM     16

#define XLLP_UDC_REQ_TYPE_MASK      (0x3 << 5)      // Used to identify a Request Type
#define XLLP_USB_REQ_RECIPIENT_MASK	0x1f

#define XLLP_DMA_BUFF_SIZE_MIN      32
#define XLLP_OUT_EP_BUFF_SIZE       4064
#define XLLP_OUT_EP_NUM_BUFF        4

// temp define XLLP_STATUS_T
#define XLLP_STATUS_T XLLP_UINT32_T

// Enumerate USB Endpoints
typedef enum XLLP_UDC_EP_E
{
    ENDPOINT_0 = 0,
    ENDPOINT_A,
    ENDPOINT_B,
} XLLP_UDC_EP_T;

// Enumerate USB Endpoints
typedef enum XLLP_UDC_USB_EP_E
{
    USB_EP_0 = 0,
    USB_EP_1,
    USB_EP_2,
  //  USB_EP_3,
  //  USB_EP_4,
  //  USB_EP_5,
  //  USB_EP_6,
  //  USB_EP_7,
  //  USB_EP_8,
  //  USB_EP_9,
  //  USB_EP_10,
  //  USB_EP_11,
  //  USB_EP_12,
  //  USB_EP_13,
  //  USB_EP_14,
  //  USB_EP_15
} XLLP_UDC_USB_EP_T;

// Enumerate UDC Interrupts
typedef enum XLLP_UDC_INTERRUPTS_E
{
    INT_ENDPOINT_0  = 0,
    INT_ENDPOINT_A,
    INT_ENDPOINT_B,
    //INT_ENDPOINT_C,
    //INT_ENDPOINT_D,
    //INT_ENDPOINT_E,
    //INT_ENDPOINT_F,
    //INT_ENDPOINT_G,
    //INT_ENDPOINT_H,
    //INT_ENDPOINT_I,
    //INT_ENDPOINT_J,
    //INT_ENDPOINT_K,
    //INT_ENDPOINT_L,
    //INT_ENDPOINT_M,
    //INT_ENDPOINT_N,
    //INT_ENDPOINT_P  = 15,

    INT_ENDPOINT_Q  = 16,
    //INT_ENDPOINT_R,
    //INT_ENDPOINT_S,
    //INT_ENDPOINT_T,
    //INT_ENDPOINT_U,
    //INT_ENDPOINT_V,
   // INT_ENDPOINT_W,
    INT_ENDPOINT_X,

    INT_RESET       = 27,
    INT_SUSPEND     = 28,
    INT_RESUME      = 29,
    INT_SOF         = 30,
    INT_CONFIG      = 31
} XLLP_UDC_INTERRUPTS_T;

// Enumerate UDC Endpoint Max. Packet Size
typedef enum XLLP_UDC_EP_MPS_E
{
    MPS_CNTRL_8  = 8,
    MPS_CNTRL_16 = 16,
    MPS_BULK_8  = 8,
    MPS_BULK_16 = 16,
    MPS_BULK_32 = 32,
    MPS_BULK_64 = 64,
    //MPS_INT_8   = 8,
    //MPS_INT_16  = 16,
    //MPS_INT_32  = 32,
    //MPS_INT_64  = 64,
    //MPS_ISO_256 = 256,
    //MPS_ISO_512 = 512,
    //MPS_ISO_1023 = 1023,
    //MPS_ISO_LSB_256 = 0x00,
    //MPS_ISO_MSB_256 = 0x01,
    //MPS_ISO_LSB_512 = 0x00,
    //MPS_ISO_MSB_512 = 0x02,
    //MPS_ISO_LSB_1023 = 0xff,
    //MPS_ISO_MSB_1023 = 0x03
} XLLP_UDC_EP_MPS_T;

// Enumerate UDC Endpoint Interrupts
/*typedef enum XLLP_UDC_EP_INTERRUPT_TYPE_E
{
    PACKET_COMPL_INT = 0,
    FIFO_ERROR_INT   = 1
} XLLP_UDC_EP_INTERRUPT_TYPE_T;

// Enumerate Vendor Requests
typedef enum XLLP_UDC_VENDOR_REQ_E
{
    VENDOR_SETUP_IN_EP     = 0x01,
    VENDOR_SETUP_OUT_EP    = 0x02,
    VENDOR_SETUP_INT_EP    = 0x03,
    VENDOR_SETUP_LOOPBACK  = 0x04
} XLLP_UDC_VENDOR_REQ_T;
*/
/*
************************************************************************************
*							DATA TYPES
************************************************************************************
*/

// USB Device Descriptor structure
typedef struct XLLP_UDC_USB_DEVICE_DESCRIPTOR_S
{
    XLLP_UINT8_T    bLength;
    XLLP_UINT8_T    bDescriptorType;
    XLLP_UINT16_T   bcdUSB;
    XLLP_UINT8_T    bDeviceClass;
    XLLP_UINT8_T    bDeviceSubClass;
    XLLP_UINT8_T    bDeviceProtocol;
    XLLP_UINT8_T    bMaxPacketSize0;
    XLLP_UINT16_T   idVendor;
    XLLP_UINT16_T   idProduct;
    XLLP_UINT16_T   bcdDevice;
    XLLP_UINT8_T    iManufacturer;
    XLLP_UINT8_T    iProduct;
    XLLP_UINT8_T    iSerialNumber;
    XLLP_UINT8_T    bNumConfigurations;
} XLLP_UDC_USB_DEVICE_DESCRIPTOR_T, P_XLLP_UDC_USB_DEVICE_DESCRIPTOR_T;

// UDC Control Transfer structure
typedef struct XLLP_UDC_USB_CTRL_XFER_S
{
    XLLP_UINT32_T   statusEp0;
    P_XLLP_UINT32_T pTxBuffEp0;
    P_XLLP_UINT32_T pRxBuffEp0;
    XLLP_UINT32_T   outDataEp0[64];
    XLLP_UINT16_T   descIndexEp0;
    XLLP_UINT16_T   descTypeEp0;
    XLLP_UINT32_T   dataLengthEp0;
} XLLP_UDC_USB_CTRL_XFER_T, *P_XLLP_UDC_USB_CTRL_XFER_T;

// UDC Vendor Request structure
typedef struct XLLP_UDC_VENDOR_REQ_DATA_S
{
    XLLP_UINT8_T	recipient;
    XLLP_UINT8_T    vendorReqType;
    XLLP_UINT16_T   vendorReqEndpointNum;
    XLLP_UINT32_T   vendorReqXferLength;
    XLLP_BOOL_T     vendorReqComplete;
} XLLP_UDC_VENDOR_REQ_DATA_T, *P_XLLP_UDC_VENDOR_REQ_DATA_T;

// UDC Interrupt statistics structure
typedef struct XLLP_UDC_INT_STATISTICS_S
{
    XLLP_UINT32_T ResetIntCount;
    XLLP_UINT32_T SuspendIntCount;
    XLLP_UINT32_T ResumeIntCount;
    XLLP_UINT32_T SOFIntCount;
    XLLP_UINT32_T ConfigIntCount;
    XLLP_UINT32_T EpIntCount[XLLP_UDC_MAX_EP_NUM];

	P_XLLP_UINT32_T pRegDbg;
	XLLP_UINT32_T   totalCapturedCnt;
} XLLP_UDC_INT_STATISTICS_T, *P_XLLP_UDC_INT_STATISTICS_T;

// UDC Endpoints transfers structure
typedef struct XLLP_UDC_XFER_S
{
//    XsDmaDescriptorElementsT * firstDescVtP;
    P_XLLP_UINT32_T pDataEp;
    XLLP_UINT32_T   dmaChannel;
    XLLP_UINT32_T   xferLength;
    XLLP_UINT32_T   xferDataCounter;
    XLLP_UINT32_T   maxPacketSize;
    XLLP_BOOL_T   TxXferComplete;
    XLLP_BOOL_T   RxXferComplete;
    XLLP_BOOL_T   RxBlockXferComplete;
    XLLP_BOOL_T   enableLoopback;
} XLLP_UDC_XFER_T, *P_XLLP_UDC_XFER_T;

// UDC Registers
typedef struct XLLP_UDC_REGISTERS_S {
    XLLP_VUINT32_T UDCCR;           // UDC Control Register
    XLLP_VUINT32_T UDCICR0;         // UDC Interrrupt Control Register 0
    XLLP_VUINT32_T UDCICR1;         // UDC Interrrupt Control Register 1
    XLLP_VUINT32_T UDCISR0;         // UDC Interrrupt Status Register 0
    XLLP_VUINT32_T UDCISR1;         // UDC Interrrupt Status Register 1
    XLLP_VUINT32_T UDCFNR;          // UDC Frame Number Register
    XLLP_VUINT32_T RESERVED0[58];   // Reserved 0x40600018-0x406000FF
    XLLP_VUINT32_T UDCCSR[24];      // UDC Control/Status registers 0, A-X
    XLLP_VUINT32_T RESERVED1[40];   // Reserved 0x4060015F-0x406001FF
    XLLP_VUINT32_T UDCBCR[24];      // UDC Byte Count Registers 0, A-X
    XLLP_VUINT32_T RESERVED2[40];   // Reserved 0x4060025F-0x406002FF
    XLLP_VUINT32_T UDCDR[24];       // UDC Data Registers 0, A-X
    XLLP_VUINT32_T RESERVED3[40];   // Reserved 0x4060035F-0x406003FF,
    XLLP_VUINT32_T UDCCRZ[24];      // Reserved 0x40600400-0x40600403,
                                    // 23 UDC Configuration Registers A-X
} XLLP_UDC_REGISTERS_T, *P_XLLP_UDC_REGISTERS_T;

// Note:
// Reserved 0x4060045F-0x406FFFFF

// UDC Endpoint Configuration Structure
typedef struct XLLP_UDC_EP_CONFIG_TABLE_S {
    XLLP_UDC_EP_T       udcEndpointNum;             // UDC Endpoint Number: (0-23)
    XLLP_UINT8_T        usbConfigNum;               // Usb Configuration Number: (1-3)
    XLLP_UINT8_T        usbInterfaceNum;            // Usb Interface Number: (1-7)
    XLLP_UINT8_T        usbIntAltSettingsNum;       // Usb Interface Alternate Settings Number: (1-7)
    XLLP_UDC_EP_T   usbEndpointNum;             // Usb Endpoint Number: (1-15)
    XLLP_UDC_EP_TYPE_T  usbEndpointType;            // Usb Endpoint type: Bulk, Iso, Interrupt
    XLLP_UDC_EP_DIR_T   usbEndpointDirection;       // Usb Endpoint direction: IN, OUT
    XLLP_UINT16_T       maxPacketSize;              // Max. Packet Size: (1-1023)
    XLLP_UDC_EP_DOUBLE_BUFF_T doubleBuffEnabled;    // Double Buffering Enabled if set to one
    XLLP_UDC_EP_ENABLED_T   endpointEnabled;        // Endpoint Enabled if set to one
    XLLP_UINT32_T           endpointConfigValue;    // Endpoint Configuration Register Value
} XLLP_UDC_EP_CONFIG_TABLE_T, *P_XLLP_UDC_EP_CONFIG_TABLE_T;

typedef struct XLLP_UDC_USB_BIND_ENDPOINTS_S {
    XLLP_UDC_EP_T       udcEndpointNum;             // UDC Endpoint Number: (0-23)
    XLLP_UDC_EP_T   usbEndpointNum;             // Usb Endpoint Number: (1-15)
} XLLP_UDC_USB_BIND_ENDPOINTS_T;


// UDC Handle Structure
typedef struct XLLP_UDC_S {
    P_XLLP_UDC_REGISTERS_T  pRegsBase;          // Pointer to UDC's registers
    P_XLLP_UDC_EP_CONFIG_TABLE_T pConfigTable;  // Pointer to the Endpoints Config. table
    XLLP_UDC_USB_BIND_ENDPOINTS_T listOfActiveEndpoints[XLLP_UDC_USB_MAX_EP_NUM];
                                                // Used to keep track of endpoints in the active configuration
    XLLP_UDC_INT_STATISTICS_T interruptStat;    // Used to keep track ot the interrupts statistics
	XLLP_UDC_XFER_T EpXferTable[XLLP_UDC_MAX_EP_NUM];  // Used to support the transfers
	XLLP_UDC_USB_CTRL_XFER_T controlXfer;       // Used to process control transfers
	XLLP_UDC_VENDOR_REQ_DATA_T vendorReq;       // Used to process vendor requests
	XLLP_BOOL_T enableDma;				        // Selects DMA to service FIFOs if set
	XLLP_BOOL_T setupComplete;			        // Used to track the complition of the setup
    XLLP_BOOL_T enumerateComplete;		        // Used to track the complition of the enumerate command
    XLLP_BOOL_T cableAttached;                  // Used to monitor the cable attachment and disconnect
    XLLP_UINT8_T    usbConfigNumActive;         // Active Usb Configuration Number: (1-3)
    XLLP_UINT8_T    usbInterfaceNumActive;      // Active Usb Interface Number: (1-7)
    XLLP_UINT8_T    usbIntAltSettingsNumActive; // Active Usb Interface Alternate Settings Number: (1-7)
} XLLP_UDC_T, *P_XLLP_UDC_T;

/*
************************************************************************************
*                            FUNCTION PROTOTYPES
************************************************************************************
*/
void XllpUdcComputeConfigRegisterValueSingle (
                                    XLLP_UDC_EP_T   udcEndpointNum,
                                    XLLP_UINT8_T    usbConfigNum,
                                    XLLP_UINT8_T    usbInterfaceNum,
                                    XLLP_UINT8_T    usbIntAltSettingsNum,
                                    XLLP_UDC_EP_T   usbEndpointNum,
                                    XLLP_UDC_EP_TYPE_T  usbEndpointType,
                                    XLLP_UDC_EP_DIR_T   usbEndpointDirection,
                                    XLLP_UINT16_T   maxPacketSize,
                                    XLLP_UDC_EP_DOUBLE_BUFF_T   doubleBuffEnabled,
                                    XLLP_UDC_EP_ENABLED_T       endpointEnabled,
                                    P_XLLP_UINT32_T pEndpointConfigRegValue);
void XllpUdcComputeConfigRegisterValue (P_XLLP_UDC_EP_CONFIG_TABLE_T pEndpointsConfigTable,
                                        P_XLLP_UINT32_T pEndpointConfigRegValue);
XLLP_STATUS_T XllpUdcConfigureEndpoints (P_XLLP_UDC_T pUdcHandle);
void XllpUdcEnableInterrupt (P_XLLP_UDC_T pUdcHandle,
                             XLLP_UDC_INTERRUPTS_T udcEndpointNum,
                             XLLP_UDC_EP_INTERRUPT_TYPE_T interruptType);
void XllpUdcDisableInterrupt (P_XLLP_UDC_T pUdcHandle,
                              XLLP_UDC_INTERRUPTS_T udcEndpointNum,
                              XLLP_UDC_EP_INTERRUPT_TYPE_T interruptType);
void XllpUdcClearInterrupt (P_XLLP_UDC_T pUdcHandle,
                            XLLP_UDC_INTERRUPTS_T udcEndpointNum,
                            XLLP_UDC_EP_INTERRUPT_TYPE_T interruptType);

void XllpUdcGetStatusInterrupt (P_XLLP_UDC_T pUdcHandle,
                                XLLP_UDC_INTERRUPTS_T udcEndpointNum,
                                XLLP_UDC_EP_INTERRUPT_TYPE_T interruptType,
                                P_XLLP_UINT32_T intStatus);
void XllpUdcBuildListOfActiveEndpoints (P_XLLP_UDC_T pUdcHandle,
                                        XLLP_UINT32_T configuration,
                                        XLLP_UINT32_T interface,
                                        XLLP_UINT32_T settings,
                                        P_XLLP_UINT32_T pNumActiveEndpoints);
void XllpUdcControlProcessIdle (P_XLLP_UDC_T pUdcHandle,
                                P_XLLP_UINT32_T pControlXferStatus);

XLLP_STATUS_T XllpUdcProcessVendorRequest (P_XLLP_UDC_T pUdcHandle);

void XllpUdcControlProcessOutData (P_XLLP_UDC_T pUdcHandle,
                                   P_XLLP_UINT32_T pControlXferStatus);
void XllpUdcControlProcessInData (P_XLLP_UDC_T pUdcHandle,
                                  P_XLLP_UINT32_T pControlXferStatus);
void XllpUdcControlProcessEndXfer (P_XLLP_UDC_T pUdcHandle,
                                   P_XLLP_UINT32_T pControlXferStatus);
void XllpUdcEp0InterruptHandler (P_XLLP_UDC_T pUdcHandle);
void XllpUdcBusDevSoftConnect (XLLP_BOOL_T connect);
void XllpUdcForceEndpointStall (P_XLLP_UDC_T pUdcHandle,
                                XLLP_UDC_EP_T udcEndpointNum);
void XllpUdcFlashEndpointFifo (P_XLLP_UDC_T pUdcHandle,
                               XLLP_UDC_EP_T udcEndpointNum);
void XllpUdcFillFifo (P_XLLP_UDC_T pUdcHandle,
                      XLLP_UDC_EP_T udcEndpointNum,
                      XLLP_BOOL_T enableZLP);
void XllpUdcReadDwordFifo (P_XLLP_VUINT32_T pReg,
						   void *buf,
						   XLLP_INT32_T len);
void XllpUdcUnloadFifo (P_XLLP_UDC_T pUdcHandle,
                        XLLP_UDC_EP_T udcEndpointNum);
void XllpUdcWriteDwordFifo(P_XLLP_VUINT32_T pReg,
						   void *src,
						   XLLP_INT32_T len);
void XllpUdcConfigInterruptHandler (P_XLLP_UDC_T pUdcHandle);
XLLP_STATUS_T XllpUdcHWSetup (P_XLLP_UDC_T pUdcHandle);
XLLP_STATUS_T XllpUdcHWShutdown (P_XLLP_UDC_T pUdcHandle);
void XllpUdcSWInit (P_XLLP_UDC_T pUdcHandle, P_XLLP_UDC_REGISTERS_T pRegs);

#endif 	/* _xllp_udc_h */
