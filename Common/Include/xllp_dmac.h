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
**	 xllp_dmac.h
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
 
 ******************************************************************************
**
**  FILENAME:       xllp_dmac.h
**
**  PURPOSE: contains all DMA Controller specific macros, typedefs, and prototypes.
**           Declares no storage.
**                  
**
******************************************************************************/

#ifndef __DMAC_H__
#define __DMAC_H__

#include "xllp_defs.h"
#include "dma.h"

#define MAX_DMA_CHANNELS  			32
#define DMA_STATUS_INTERRUPTS_MASK	0x01000217

/**
 * DCSR Register 
 **/
#define XLLP_DMAC_DCSR_BUS_ERR_INTR		 (1U<<0)
#define XLLP_DMAC_DCSR_START_INTR		 (1U<<1)
#define XLLP_DMAC_DCSR_END_INTR			 (1U<<2)
#define XLLP_DMAC_DCSR_STOP_INTR		 (1U<<3)
#define XLLP_DMAC_DCSR_RAS_INTR          (1U<<4)
#define XLLP_DMAC_DCSR_REQ_PEND			 (1U<<8)
#define XLLP_DMAC_DCSR_EOR_INTR			 (1U<<9)
#define XLLP_DMAC_DCSR_CMP_ST 			 (1U<<10)
#define XLLP_DMAC_DCSR_MASK_RUN          (1U<<22)
#define XLLP_DMAC_DCSR_RAS_EN            (1U<<23)
#define XLLP_DMAC_DCSR_CLR_CMP_ST		 (1U<<24)
#define XLLP_DMAC_DCSR_SET_CMP_ST		 (1U<<25)
#define XLLP_DMAC_DCSR_EOR_STOP_EN		 (1U<<26)
#define XLLP_DMAC_DCSR_EOR_JMP_ENT		 (1U<<27)
#define XLLP_DMAC_DCSR_EOR_IRQ_EN		 (1U<<28)
#define XLLP_DMAC_DCSR_STOP_IRQ_EN		 (1U<<29)
#define XLLP_DMAC_DCSR_NO_DESC_FETCH	 (1U<<30)
#define XLLP_DMAC_DCSR_RUN 		  		 (1U<<31)
#define XLLP_DMAC_DCSR_WRITABLES_MSK	 0xFFC00617 

/**
 * DCMD Register
 **/
#define XLLP_DMAC_DCMD_LEN				 (1U<<0)
#define XLLP_DMAC_DCMD_WIDTH			 (1U<<14)
#define XLLP_DMAC_DCMD_SIZE				 (1U<<16)
#define XLLP_DMAC_DCMD_FLY_BY_T			 (1U<<19)
#define XLLP_DMAC_DCMD_FLY_BY_S			 (1U<<20)
#define XLLP_DMAC_DCMD_END_IRQ_EN		 (1U<<21)
#define XLLP_DMAC_DCMD_START_IRQ_EN		 (1U<<22)
#define XLLP_DMAC_DCMD_ADDR_MODE		 (1U<<23)
#define XLLP_DMAC_DCMD_CMP_EN			 (1U<<25)
#define XLLP_DMAC_DCMD_FLOW_TRG			 (1U<<28)
#define XLLP_DMAC_DCMD_FLOW_SRC			 (1U<<29)
#define XLLP_DMAC_DCMD_INC_TRG_ADDR		 (1U<<30)
#define XLLP_DMAC_DCMD_INC_SRC_ADDR		 (1U<<31)


#define XLLP_DMAC_DDADR_RESERVED_MASK	 0xFFFFFFF0
#define XLLP_DMAC_DRCMR_ENABLE     		 0x80
#define XLLP_DMAC_DRCMR_DISABLE    		 0x00

/**
 * DMAC DDADRx bit fields 
 **/
#define XLLP_DMAC_DDADR_STOP		(1<<0)
#define XLLP_DMAC_DDADR_BREN		(1<<1)

/**
 * Used in DMA handler definition
 **/
#define XLLP_DMAC_CHANNEL_NUM		32
#define XLLP_DMAC_DRCMR1_NUM		64
#define XLLP_DMAC_DRCMR2_NUM		36

/**
 * Some error codes
 **/
#define XLLP_ERR_BAD_CHANNEL           	-1
#define XLLP_ERR_CHANNEL_NOT_ALLOCATED  -2
#define XLLP_ERR_NO_HANDLER             -3
#define XLLP_ERR_MEM_ALLOC              -4
//#define XLLP_DMAC_CHANNEL_INVALID		-5
#define XLLP_DMAC_CHANNEL_INVALID		99

/**
 * Max length that can be transferred by a single DMA descriptor
 **/
#define XLLP_DCMD_LEN_MASK					0x1FFFU    
#define XLLP_DMAC_ALIGN_MASK                0xFFFFFFF0U

#define XLLP_ALL_INTERRUPTS        (XLLP_DMAC_DCSR_BUS_ERR_INTR|\
                                    XLLP_DMAC_DCSR_START_INTR  |\
                                    XLLP_DMAC_DCSR_END_INTR    |\
                                    XLLP_DMAC_DCSR_STOP_INTR   |\
                                    XLLP_DMAC_DCSR_EOR_INTR)

/**
 * Mask of all writable bits in DCSR; others must be written as 0
 **/
#define XLLP_DCSR_WRITABLES_MSK    (XLLP_DMAC_DCSR_BUS_ERR_INTR |\
                                    XLLP_DMAC_DCSR_START_INTR   |\
                                    XLLP_DMAC_DCSR_END_INTR     |\
									XLLP_DMAC_DCSR_RAS_INTR		|\
									XLLP_DMAC_DCSR_EOR_INTR     |\
									XLLP_DMAC_DCSR_CMP_ST		|\
									XLLP_DMAC_DCSR_MASK_RUN		|\
                                    XLLP_DMAC_DCSR_STOP_IRQ_EN  |\
                                    XLLP_DMAC_DCSR_NO_DESC_FETCH|\
                                    XLLP_DMAC_DCSR_RUN         )

/**
 * DMA Descriptor
 **/
typedef struct 
{
 	XLLP_VUINT32_T   DDADR;  // descriptor address reg
    XLLP_VUINT32_T   DSADR;  // source address register
    XLLP_VUINT32_T   DTADR;  // target address register
    XLLP_VUINT32_T   DCMD;   // command address register
}XLLP_DMAC_DESCRIPTOR_T, *P_XLLP_DMAC_DESCRIPTOR_T;

/**
 * DMAC Register Definitions
 **/
typedef struct 
{
	XLLP_VUINT32_T DCSR[XLLP_DMAC_CHANNEL_NUM]; /* DMA Control/Status Registers 0-31	*/
    XLLP_VUINT32_T RESERVED0[0x8]; 				/* RESERVED0                        	*/
    XLLP_VUINT32_T DALGN;        				/* DMA Alignment Register 31        	*/
    XLLP_VUINT32_T DPCSR;        				/* DMA Programmed IO control status reg	*/
    XLLP_VUINT32_T RESERVED1[0xE]; 				/* RESERVED0                        	*/
    XLLP_VUINT32_T DRQSR0;       				/* DMA DREQ(0) Status Register      	*/
    XLLP_VUINT32_T DRQSR1;       				/* DMA DREQ(1) Status Register      	*/
    XLLP_VUINT32_T DRQSR2;       				/* DMA DREQ(2) Status Register      	*/
    XLLP_VUINT32_T RESERVED2[0x1]; 				/* RESERVED1                        	*/
    XLLP_VUINT32_T DINT;         				/* DMA Interrupt Register           	*/
    XLLP_VUINT32_T RESERVED3[0x3]; 				/* RESERVED2                        	*/
    XLLP_VUINT32_T DRCMR1[XLLP_DMAC_DRCMR1_NUM];/* Request to Channel Map for DREQ 0-63	*/
	XLLP_DMAC_DESCRIPTOR_T DDG[XLLP_DMAC_CHANNEL_NUM]; /* DMA Desc Group for channel 0-31 */
    XLLP_VUINT32_T RESERVED4[0x340]; 			/* RESERVED3                            */
    XLLP_VUINT32_T DRCMR2[XLLP_DMAC_DRCMR2_NUM];/* Request to Channel Map 64-99			*/
} XLLP_DMAC_T, *P_XLLP_DMAC_T;


/**
 *  DMAC peripheral device width
 **/
typedef enum
{
    XLLP_DMAC_WIDTH_0 = 0,
    XLLP_DMAC_WIDTH_8,
    XLLP_DMAC_WIDTH_16,
    XLLP_DMAC_WIDTH_32
}XLLP_DMAC_DEVICE_WIDTH_T;

/**
 * DMAC status
 **/
typedef enum
{
    XLLP_NO_ERROR = 0,
    XLLP_NO_DMA_CHANNEL_AVAILABLE
}XLLP_DMA_STATUS_T;
//}XLLP_STATUS_T;


/**
 * DMAC peripheral burst size
 **/
typedef enum
{
    XLLP_DMAC_RESERVED_SIZE = 0,
    XLLP_DMAC_BURSTSIZE_8,
    XLLP_DMAC_BURSTSIZE_16,
    XLLP_DMAC_BURSTSIZE_32
}XLLP_DMAC_DEVICE_BURSTSIZE_T;

/**
 * DMAC channels
 **/
typedef enum
{
    XLLP_DMAC_CHANNEL_0 = 0,
    XLLP_DMAC_CHANNEL_1,
    XLLP_DMAC_CHANNEL_2,
    XLLP_DMAC_CHANNEL_3,
    XLLP_DMAC_CHANNEL_4,
    XLLP_DMAC_CHANNEL_5,
    XLLP_DMAC_CHANNEL_6,
    XLLP_DMAC_CHANNEL_7,
    XLLP_DMAC_CHANNEL_8,
    XLLP_DMAC_CHANNEL_9,
    XLLP_DMAC_CHANNEL_10,
    XLLP_DMAC_CHANNEL_11,
    XLLP_DMAC_CHANNEL_12,
    XLLP_DMAC_CHANNEL_13,
    XLLP_DMAC_CHANNEL_14,
    XLLP_DMAC_CHANNEL_15,
    XLLP_DMAC_CHANNEL_16,
    XLLP_DMAC_CHANNEL_17,
    XLLP_DMAC_CHANNEL_18,
    XLLP_DMAC_CHANNEL_19,
    XLLP_DMAC_CHANNEL_20,
    XLLP_DMAC_CHANNEL_21,
    XLLP_DMAC_CHANNEL_22,
    XLLP_DMAC_CHANNEL_23,
    XLLP_DMAC_CHANNEL_24,
    XLLP_DMAC_CHANNEL_25,
    XLLP_DMAC_CHANNEL_26,
    XLLP_DMAC_CHANNEL_27,
    XLLP_DMAC_CHANNEL_28,
    XLLP_DMAC_CHANNEL_29,
    XLLP_DMAC_CHANNEL_30,
    XLLP_DMAC_CHANNEL_31
}XLLP_DMAC_CHANNEL_T, *P_XLLP_DMAC_CHANNEL_T;

#define	MEM2MEM_CHANNEL		XLLP_DMAC_CHANNEL_20
#define WTM_CMD_CHANNEL		XLLP_DMAC_CHANNEL_0
#define WTM_RX_CHANNEL		XLLP_DMAC_CHANNEL_1
#define WTM_TX_CHANNEL		XLLP_DMAC_CHANNEL_2
#define USB_RX_CHANNEL		XLLP_DMAC_CHANNEL_4
#define USB_TX_CHANNEL		XLLP_DMAC_CHANNEL_5

#define SSP_TX_CHANNEL		XLLP_DMAC_CHANNEL_6
#define SSP_RX_CHANNEL		XLLP_DMAC_CHANNEL_7

#define NFU_DATA_CHANNEL	XLLP_DMAC_CHANNEL_8
#define NFU_CMD_CHANNEL		XLLP_DMAC_CHANNEL_9

/**
 * DMAC Interrupts
 **/
typedef enum
{
    XLLP_DMAC_BUS_ERR_INT = 0,
    XLLP_DMAC_START_INT,
    XLLP_DMAC_END_INT,
    XLLP_DMAC_STOP_INT,
    XLLP_DMAC_EOR_INT = 9
}XLLP_DMAC_INTERRUPT_T;

/**
 * DMAC Transfer Type
 **/
typedef enum
{
    XLLP_DMAC_TRANSFER_MEM_TO_MEM = 0,
    XLLP_DMAC_TRANSFER_IO_TO_MEM,
    XLLP_DMAC_TRANSFER_MEM_TO_IO
}XLLP_DMAC_TRANSFER_TYPE_T;

/**
 * DMAC Transfer Mode
 **/
typedef enum
{
    XLLP_DMAC_DESCRIPTOR_MODE = 0,
    XLLP_DMAC_NO_DESCRIPTOR_MODE
}XLLP_DMAC_TRANSFER_MODE_T;

/**
 * DMAC Descriptor Branching Mode
 **/
typedef enum
{
    XLLP_DMAC_DISABLE_DESC_BRANCH = 0,
    XLLP_DMAC_ENABLE_DESC_BRANCH
}XLLP_DMAC_DESC_BRANCH_T;

/**
 * DMAC Descriptor Enable
 **/
typedef enum
{
    XLLP_DMAC_DESC_RUN_CHANNEL = 0,
    XLLP_DMAC_DESC_STOP_CHANNEL
}XLLP_DMAC_DESC_ENABLE_T;

/**
 * DMA Command - old format
 **/
typedef struct 
{
	XLLP_INT16_T     aLen;        // Length of transfer in bytes. Max 0x1FFF
	XLLP_UINT8_T     aWidth;      // Width of on-chip peripheral
	XLLP_UINT8_T     aSize;       // Max. burst size of each data transferred
	XLLP_UINT8_T     aEndian;     // Device endianness. 0=Little, 1=Big endian
	XLLP_UINT8_T     aFlyByT;     // Fly-By target bit
	XLLP_UINT8_T     aFlyByS;     // Fly-By source bit
	XLLP_UINT8_T     aEndIrqEn;   // End Interrupt Enable. When set, 
	XLLP_UINT8_T     aStartIrqEn; // Start Int. enable. When set, generate
	XLLP_UINT8_T     aAddrMode;   // Addressing mode for descriptor comparison
	XLLP_UINT8_T     aCmpEn;      // Descriptor compare enable bit
	XLLP_UINT8_T     aFlowTrg;    // Flow control of the target.
	XLLP_UINT8_T     aFlowSrc;    // Flow control of the source
	XLLP_UINT8_T     aIncTrgAddr; // Target address increment setting
	XLLP_UINT8_T     aIncSrcAddr; // Source address increment setting
}XLLP_DMAC_COMMAND_T, *P_XLLP_DMAC_COMMAND_T;

/**
 * DMA CMDx register bit layout
 **/
struct DMA_CMD_BITS 
{
	unsigned int Length			:13; //Bits [12:0]: Length of transfer in Bytes
	unsigned int Reserved4		:1; //Bit 13: reserved
	unsigned int Width			:2; //Bits [15:14]: Width of onchip peripheral - 0=reserved, 1=1B, 2=2B, 3=4B
	unsigned int MaxBurstSize	:2; //Bits [17:16]: Max Burst Size - 0=reserved, 1=8B, 2=16B, 3=32B
	unsigned int Reserved3		:3; //Bits [20:18]: reserved
	unsigned int EndIRQEn		:1; //Bit 21: end interrupt enable
	unsigned int StartIRQEn		:1; //Bit 22: start interrupt enable
	unsigned int AddrMode		:1; //Bit 23: Addressing mode for descriptor compare - 0=target has addr/1=target has data
	unsigned int Reserved2		:1; //Bit 24: reserved
	unsigned int CmpEn			:1; //Bit 25: Compare Enable - descriptor branching compare enable
	unsigned int Reserved1		:2; //Bits [27:26]: reserved
	unsigned int FlowTrg		:1;	//Bit 28: Target flow control - 1=wait for request signal
	unsigned int FlowSrc		:1;	//Bit 29: Source flow control - 1=wait for request signal
	unsigned int IncTrgAddr		:1;	//Bit 30: Target increment - 0=no increment, 1=increment
	unsigned int IncSrcAddr		:1;	//Bit 31: Source increment - 0=no increment, 1=increment
};

typedef union
{
	unsigned int value;
	struct DMA_CMD_BITS bits;	
} DMA_CMDx_T, *P_DMA_CMDx_T;

/**
 * DMAC Channel Status
 **/
typedef enum
{
 XLLP_DMAC_STATUS_BUSERRINTR = 0, // Bus error causing int.
 XLLP_DMAC_STATUS_STARTINTR,      // Successful Descriptor fetch int.
 XLLP_DMAC_STATUS_ENDINTR,        // Successful completion int.
 XLLP_DMAC_STATUS_STOPINTR,       // Channel state, 0=Running, 1=Stop
 XLLP_DMAC_STATUS_REQPEND = 8,    // Channel Request pending state.
 								  // 1=Channel has pending req, 0=No Req
 XLLP_DMAC_STATUS_EORINT,         // Indicates status of peripheral Rx data
 XLLP_DMAC_STATUS_CMPST           // Descriptor compare status
}XLLP_DMAC_CHANNEL_STATUS_T, *P_XLLP_DMAC_CHANNEL_STATUS_T;


/**
 * DMAC External Request Pins
 **/
typedef enum
{
   XLLP_DMAC_EXTERNAL_PIN_0 = 0,
   XLLP_DMAC_EXTERNAL_PIN_1
}XLLP_DMAC_EXT_PIN_T;

/**
 * DMAC Data Alignment
 **/
typedef enum
{
   XLLP_DMAC_ALIGNMENT_OFF = 0,
   XLLP_DMAC_ALIGNMENT_ON
}XLLP_DMAC_ALIGNMENT_T;


/**
 * DMAC Channel Allocation Priority
 **/
typedef enum
{
   XLLP_DMAC_CHANNEL_PRIORITY_HIGH = 0,
   XLLP_DMAC_CHANNEL_PRIORITY_MEDIUM,
   XLLP_DMAC_CHANNEL_PRIORITY_LOW,
   XLLP_DMAC_CHANNEL_PRIORITY_LOWEST
}XLLP_DMAC_CHANNEL_PRIORITY_T;

#define GlobalDmaBufferSize	1800
char *GlobalDmaDescAlloc(int RequestedSize);


/**
 * XLLP DMA Primitive Functions
 **/
P_XLLP_DMAC_T XLLP_Return_PDmacHandle();
void alignChannel(unsigned int channel, unsigned int user_align);
void configDescriptor(P_XLLP_DMAC_DESCRIPTOR_T ,P_XLLP_DMAC_DESCRIPTOR_T, unsigned int, \
					  unsigned int, P_DMA_CMDx_T, unsigned int, unsigned int);
void loadDescriptor (P_XLLP_DMAC_DESCRIPTOR_T pDesc, XLLP_DMAC_CHANNEL_T aChannel);

void XllpDmacMapDeviceToChannel(DMAC_DEVICE_T aDeviceDrcmr,XLLP_DMAC_CHANNEL_T aChannel);
void XllpDmacUnMapDeviceToChannel(DMAC_DEVICE_T aDeviceDrcmr, XLLP_DMAC_CHANNEL_T aChannel);
void XllpDmacStartTransfer( XLLP_DMAC_CHANNEL_T aChannel );
void XllpDmacStopTransfer( XLLP_DMAC_CHANNEL_T aChannel );
void XllpDmacEnableStopIrqInterrupt( XLLP_DMAC_CHANNEL_T aChannel );
void XllpDmacDisableStopIrqInterrupt( XLLP_DMAC_CHANNEL_T aChannel );
void XllpDmacDisableEORInterrupt( XLLP_DMAC_CHANNEL_T aChannel );
void XllpDmacEnableEORInterrupt( XLLP_DMAC_CHANNEL_T aChannel );
void XllpDmacClearDmaInterrupt( XLLP_DMAC_CHANNEL_T aChannel );
unsigned int readDmaStatusRegister(XLLP_DMAC_CHANNEL_T aChannel);


#endif //__DMAC_H__
