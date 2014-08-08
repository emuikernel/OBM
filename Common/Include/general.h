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
**	 general.h
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
 
 ******************************************************************************/
#ifndef _general_h
#define _general_h

#include "Typedef.h"
//#include "loadoffsets.h"

/*
 *  Fuse definition structure
 */
struct PLATFORM_FUSES
{
	unsigned int DebugDisable		:1;		// Bit 0 of SYS_BOOT_CNTRL and bit 79 dis_strap_override of AP config bits 
	unsigned int PlatformState		:5;		// Bit 62-66 of AP config bits and bits 22-26 of SYS_BOOT_CNTRL
	unsigned int TZInitialized		:1;		// TrustZone Initialized
	unsigned int JTAG_Disable		:1;		// Bit 6 of SYS_BOOT_CNTRL	
	unsigned int UseWTM				:1;		// Bit 20 of SYS_BOOT_CNTRL (MMP2 and MRNA only)
	unsigned int CheckOEMKeyOnProbe :1; 	// Force checking Platform Verification keys on Probe.
	unsigned int EscapeSeqDisable	:1;		// Bit 17 of SYS_BOOT_CNTRL (Tavor TD only)
	unsigned int USBDisable			:1;		// Bits 12 USBDisable of SYS_BOOT_CNTRL
	unsigned int UARTDisable		:1;		// Bits 13 UARTDisable of SYS_BOOT_CNTRL
	unsigned int MMCDisable			:1;		// Bit 14 MMC Enable of SYS_BOOT_CNTRL
	unsigned int Resume				:1;		//  
	unsigned int USBWakeup			:1;		// Bit 16 of SYS_BOOT_CNTRL
	unsigned int PortEnabled		:1;		// Bit 17: 1 = Ports have been enabled by bootROM
	unsigned int DDRInitialized		:1; 	// Bit 18: If the BootROM already initializes the DDR
	unsigned int Download_Disable	:1;		// Bit 19 of SYS_BOOT_CNTRL
	unsigned int SBE				:1;		// Bit 20 of SYS_BOOT_CNTRL (Not for MRNA or MMP2)
	unsigned int OverrideDisable	:1;		// Bit 78 of AP fuse config bits dis_aib_override
	unsigned int BootPlatformState	:5;		// Bits 22-26: There are the bits the BootROM booted from
    unsigned int USBPort            :2;     // Bits 26-27: 0 - DIFFUSB 1 - SEUSB
    unsigned int UARTPort           :2;     // Bits 28-29: 0 - FFUART, 1 - AltUART
	unsigned int TBROpMode			:2;		// Bits 30-31: Bits 10,11 od SYS_BOOT_CNTRL (MRNA and MMP2) only.
};

typedef enum 
{
	FFUART_PORT = 0,
	ALTUART_PORT = 1
} UART_PORTS; 

typedef enum 
{
	USB_DIFF_PORT = 0,
	USB_SE_PORT = 1,
	USB_U2D_PORT = 2,
	USB_CI2_PORT = 3
} USB_PORTS; 


typedef union{
	unsigned int value;
	struct PLATFORM_FUSES bits;
}FUSE_SET, *pFUSE_SET;

//list of IDS for use in XFER structure
typedef enum
{
	TIM_DATA = 0x54494D48,	// "TIMH" - refers to tim image
	PT_DATA = 0x4D505420, 	// "MPT " - refers to partition table
	BBT_DATA = 0x4D424254, 	// "MBBT" - refers to bad block table
	RD_DATA = 0x52444953	// "RDIS" - refers to read disturb list
} XFER_DATA;

typedef struct
{
	XFER_DATA data_id;
	UINT_T location;
}	XFER_DATA_PAIR_T, *P_XFER_DATA_PAIR_T;

// This is the Transfer Structure.
//  The first word is the id = 'TBRX'
//  The next few words are fixed data
//  lastly, there is a key/value array to indicate what the bootrom is passing to next image
typedef struct
{
	UINT_T TransferID;
	UINT_T SOD_ACCR0;
	UINT_T FuseVal;
	UINT_T ErrorCode;
	UINT_T ResumeParam[4];
	UINT_T num_data_pairs;
	XFER_DATA_PAIR_T data_pairs[1];
} TRANSFER_STRUCT, *P_TRANSFER_STRUCT;

// **************** Flash Boot State Offsets *************
// Default TIM flash locations
#define TIMOffset_CS2						0x00000000
#define TIMOffset_NAND						0x00000000
#define TIMOffset_MDOC						0x00000000	// Could be in partition 2
#define TIMOffset_ONENAND					0x00000000
#define TIMOffset_SDMMC						0x00000000	// Could be in Partition 1

// For debug purposes only
#define HEX_LED_CONTROL (volatile unsigned long *)( 0x08000040 )
#define HEX_LED_WRITE (volatile unsigned long *)( 0x08000010 )

#define DDR_PHY_ADDR						0x00000000
#endif




