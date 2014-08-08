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
**  FILENAME:
**
**  PURPOSE:   		This files contains the definitions for Monahans
**                  USB 2.0 Client Device Controller
**
**
**  LAST MODIFIED:
**
******************************************************************************/

#ifndef __XLLP_U2D_H
#define __XLLP_U2D_H

#include "xllp_defs.h"
#include "xllp_usbdefs.h"


// Monahans Interrupt Control Registers
//----------------------------------------
#define XLLP_MONAHANS_ICPR_REG  0x40d00010
#define XLLP_MONAHANS_ICPR2_REG 0x40d000ac
#define XLLP_MONAHANS_ICIP_REG  0x40d00000
#define XLLP_MONAHANS_ICIP2_REG 0x40d0009c
#define XLLP_MONAHANS_ICMR_REG  0x40d00004
#define XLLP_MONAHANS_ICMR2_REG 0x40d000a0
#define XLLP_MONAHANS_ICLR2_REG 0x40d000a4

// Bit 14 is used in the ICPR2,ICIP2,ICMR2 & ICLR2 interrupt control registers
//----------------------------------------------------------------------------
#define XLLP_U2D_INTERRUPT_BIT (1U << 14)

//-------------------------------------------
// Monahans USB 2.0 Device Register
// endpoint offsets
//-------------------------------------------
typedef enum XLLP_U2D_REGISTER_OFFSETS_
{
  U2D_ENDPOINT_0 = 0,
  U2D_ENDPOINT_A,
  U2D_ENDPOINT_B,
  U2D_ENDPOINT_C,
  U2D_ENDPOINT_D,
  U2D_ENDPOINT_E,
  U2D_ENDPOINT_F,
  U2D_ENDPOINT_G,
  U2D_ENDPOINT_BAD
} XLLP_U2D_REG_ENDPOINT_OFFSET_T, XLLP_U2D_ENDPOINT_ID_T, XLLP_U2D_DMA_CHANNEL_ID, XLLP_U2D_EP_T;

// Enumerate U2D Endpoint Interrupts
//----------------------------------
typedef enum XLLP_U2D_EP_INTERRUPT_TYPE_E
{
  U2D_SHORT_PACKET_INT = 0,
  U2D_PACKET_COMPL_INT = 1,
  U2D_FIFO_ERROR_INT   = 2
} XLLP_U2D_EP_INTERRUPT_TYPE_T;

// Enumerate U2D Interrupts
//-------------------------
typedef enum XLLP_U2D_INTERRUPTS_E
{
   U2D_INT_ENDPOINT_0  = 0,
   U2D_INT_ENDPOINT_A,
   U2D_INT_ENDPOINT_B,
   U2D_INT_ENDPOINT_C,
   U2D_INT_ENDPOINT_D,
   U2D_INT_ENDPOINT_E,
   U2D_INT_ENDPOINT_F,
   U2D_INT_ENDPOINT_G,
   U2D_INT_DATA_PACKET_ERROR = 25,
   U2D_INT_RESET             = 26,
   U2D_INT_SUSPEND           = 27,
   U2D_INT_RESUME            = 28,
   U2D_INT_USOF              = 29,
   U2D_INT_SOF               = 30,
   U2D_INT_CONFIG            = 31
} XLLP_U2D_INTERRUPTS_T;


//----------------------------------------------
// U2D DMA DESCRIPTOR STRUCT FOR RAM...
// This is part of the U2C register layout below
//----------------------------------------------
typedef struct XLLP_U2D_DMA_RAM_DESC_S
{
	XLLP_VUINT32_T descriptorAddr;     // This reg is read-write: pointer must be 16 byte aligned
	XLLP_VUINT32_T sourceAddr;         // These are place holders and read-write before transfer
	XLLP_VUINT32_T targetAddr;         // over to actual DMA H/W
	XLLP_VUINT32_T commandReg ;        //   "    "    "     "      "      "
}XLLP_U2D_DMA_RAM_DESCR_T, *P_XLLP_U2D_DMA_RAM_DESCR_T;

//----------------------------------------------
// U2D DMA CHANNEL STRUCT FOR REGISTERS...
// This is part of the U2C register layout below
//----------------------------------------------
typedef struct XLLP_U2D_DMA_REG_DESC_S
{
	XLLP_VUINT32_T  descriptorAddr;    // This reg is read-write: pointer must be 16 byte aligned
	XLLP_CVUINT32_T sourceAddr_ro;     // These are place holders and READ-ONLY
	XLLP_CVUINT32_T targetAddr_ro;     //   "    "    "     "      "      "
	XLLP_CVUINT32_T commandReg_ro;     //   "    "    "     "      "      "
}XLLP_U2D_DMA_REG_DESCR_T, *P_XLLP_U2D_DMA_REG_DESCR_T;


//This is where the register struct below is located for Monahans
//---------------------------------------------------------------
#define XLLP_MONAHANS_U2D_REG_BASE ((XLLP_UINT32_T)0x54100000)

//---------------------------------------------------
// Monahans USB 2.0 (U2D) Device Controller Registers
//---------------------------------------------------
typedef struct XLLP_U2D_REGISTERS_S
{
    XLLP_VUINT32_T U2DCR;             // U2D Control Register
    XLLP_VUINT32_T U2DICR;            // U2D Interrrupt Control Register
    XLLP_VUINT32_T RESERVED0;         // Reserved
    XLLP_VUINT32_T U2DISR;            // U2D Interrrupt Status Register
    XLLP_VUINT32_T RESERVED1;         // Reserved
    XLLP_VUINT32_T U2DFNR;            // U2D Frame Number Register
    XLLP_VUINT32_T RESERVED2[2];		// Reserved
	XLLP_VUINT32_T U2DOTGCR;			// U2D OTG Control Register
	XLLP_VUINT32_T U2DOTGICR;			// U2D OTG Interrupt Control Register
	XLLP_VUINT32_T U2DOTGISR;			// U2D OTG Interrupt Status Register
	XLLP_VUINT32_T RESERVED12[53];		// Reserved
    XLLP_VUINT32_T U2DCSR_X[8];       // U2D Control/Status registers; Endpoints 0,A,B,...,G in order
    XLLP_VUINT32_T RESERVED3[56];     // Reserved
    XLLP_VUINT32_T U2DBCR_X[8];       // U2D Byte count registers; Endpoints 0,A,B,...,G in order
    XLLP_VUINT32_T RESERVED4[56];     // Reserved
    XLLP_VUINT32_T U2DDR0;            // U2D Data Register Endpoint 0
    XLLP_VUINT32_T RESERVED5[63];     // Reserved
    XLLP_VUINT32_T U2DCR_X[8];        // U2D Configuration registers; Endpoints A,B,...,G in order
    XLLP_VUINT32_T RESERVED6[56];     // Reserved
    XLLP_VUINT32_T U2DSCA;            // U2D Setup Command Address r/o
    XLLP_VUINT32_T U2DNE_X[8];        // U2D Endpoint Information Registers; Endpoints 0,A,B,...,G in order
    XLLP_VUINT32_T RESERVED7[695];    // Reserved
    XLLP_VUINT32_T U2DMACSR_X[8];     // U2D DMA Control/Status registers; Channels 0->7 in order
    XLLP_VUINT32_T RESERVED8[24];     // Reserved
    XLLP_VUINT32_T U2DMACR;           // U2D DMA Control Register
    XLLP_VUINT32_T RESERVED9[27];     // Reserved
    XLLP_VUINT32_T U2DMAINT;          // U2D DMA Interrupt Register
    XLLP_VUINT32_T RESERVED10[3];     // Reserved
    XLLP_VUINT32_T U2DMABR_X[8];      // U2D DMA Branch Register channels 0->7
    XLLP_VUINT32_T RESERVED11[56];    // Reserved
    XLLP_U2D_DMA_REG_DESCR_T U2DMACHAN_X[8]; // U2DMA Channel Descriptor Addr, Source Addr, Target Addr, Command Addr regs
                                         // Channels 0->7  Source, Target, Command regs are READ ONLY!!!
} XLLP_U2D_REGISTERS_T, *P_XLLP_U2D_REGISTERS_T;

//-----------------------------
// U2D Register Bit definitions
//-----------------------------

//U2DCR register bits
//--------------------
#define XLLP_U2DCR_UDE          ( 0x1U << 0 )	// U2D enabled
#define XLLP_U2DCR_UDA			( 0x1U << 1 )	// READ-ONLY: u2d is active
#define XLLP_U2DCR_UDR			( 0x1U << 2 )	// Forces the usb out of suspend state
#define XLLP_U2DCR_EMCE			( 0x1U << 3 )	// The Endpoint memory config. has an error
#define XLLP_U2DCR_AAISN_SHIFT  4               // Shift and a mask for the Alternate Interface
#define XLLP_U2DCR_AAISN_MASK   ( 0xFU << XLLP_U2DCR_AAISN_SHIFT )  // Settings  (0-7)
#define XLLP_U2DCR_AIN_SHIFT    8               // Shift and a mask for the Interface
#define XLLP_U2DCR_AIN_MASK     ( 0xFU << XLLP_U2DCR_AIN_SHIFT )    // Number    (0-7)
#define XLLP_U2DCR_ACN_SHIFT    12              // Shift and a mask for the Configuration
#define XLLP_U2DCR_ACN_MASK     ( 0xFU << XLLP_U2DCR_ACN_SHIFT )    // Number    (0-3)
#define XLLP_U2DCR_DRWE			( 0x1U << 16 )	// Device Remote Wakeup Enable
#define XLLP_U2DCR_SMAC			( 0x1U << 17 )	// Switch Endpoint memory to Active config.
                                                // Active interface and Alternate Interface
#define XLLP_U2DCR_HS			( 0x1U << 18 )	// High Speed Detect 1=HighSpeed 0=FullSpeed read-only
#define XLLP_U2DCR_CC			( 0x1U << 19 )	// R/W 1 to clear - Config Change Detect from Setup Command
#define XLLP_U2DCR_ADD			( 0x1U << 20 )	// Application Device disconnect
#define XLLP_U2DCR_ABP			( 0x1U << 21 )	// R/W Application Bus Power: 0=Self-powered, 1=Bus Powered
#define XLLP_U2DCR_UCLKOVR	    ( 0x1U << 22 )  // UTM clock override: 0=No override, 1=Override
#define XLLP_U2DCR_RESV1	    ( 0x1U << 23 )  // reserved
#define XLLP_U2DCR_FSTC_SHIFT   24              // Shift for FSTC field
#define XLLP_U2DCR_FSTC_MASK    ( 0x7U << XLLP_U2DCR_FSTC_SHIFT )  // Full speed Timeout cal 0<->7
#define XLLP_U2DCR_RESV2		( 0x1U << 27 )  // reserved
#define XLLP_U2DCR_HSTC_SHIFT	28              // shift for HSTC field
#define XLLP_U2DCR_HSTC_MASK    ( 0x7U << XLLP_U2DCR_HSTC_SHIFT ) // High speed timeout cal, 0<->7
#define XLLP_U2DCR_RESV3        ( 0x1U << 31 )  // reserved


//U2DICR (Interrupt Control) register bits
//----------------------------------------
#define XLLP_U2DISR_EVENTS_MASK  0xFE000000    // Event Interrupts mask
#define XLLP_U2DICR_IECC       ( 0x1U << 31 )  // Interrupt Enable Config Change
#define XLLP_U2DICR_IESOF      ( 0x1U << 30 )  // Interrupt Enable	Start of Frame
#define XLLP_U2DICR_IEUSOF	   ( 0x1U << 29)   // Interrupt Enable u Start of Frame (HS only)
#define XLLP_U2DICR_IERU	   ( 0x1U << 28)   // Interrupt Enable Resume
#define XLLP_U2DICR_IESU	   ( 0x1U << 27)   // Interrupt Enable Suspend
#define XLLP_U2DICR_IERS       ( 0x1U << 26)   // Interrupt Enable Reset
#define XLLP_U2DICR_IEDPE      ( 0x1U << 25)   // Interrupt Enable Data Packet Error
#define XLLP_U2DICR_RESV       ( 0x1U << 24)   // Reserved
#define XLLP_U2DICR_IEx_MASK     0x7U          // Mask for IEx fields
#define XLLP_U2DICR_IEG_SHIFT    21            // IEx are 3 bit wide fields, these shift give low-order bit
#define XLLP_U2DICR_IEF_SHIFT    18            // One for each Endpoint 0, A, ... G
#define XLLP_U2DICR_IEE_SHIFT    15
#define XLLP_U2DICR_IED_SHIFT    12
#define XLLP_U2DICR_IEC_SHIFT    9
#define XLLP_U2DICR_IEB_SHIFT    6
#define XLLP_U2DICR_IEA_SHIFT    3
#define XLLP_U2DICR_IE0_SHIFT    0
#define XLLP_U2DICR_IEx_FEI_BIT  2              // These are the individual interrupt enable bits within the IEx fields
#define XLLP_U2DICR_IEx_PCI_BIT  1		        // Fifo Error, Packet Complete, Short Packet Complete
#define XLLP_U2DICR_IEx_SPC_BIT  0
#define XLLP_U2DICR_ENABLE_ALL ( 0xFFFFFFFFU & ~(XLLP_U2DICR_RESV))

//U2DISR (Interrupt Status) register bits
//----------------------------------------
#define XLLP_U2DISR_IRCC    ( 0x1U << 31 )	// Interrupt Request Config Change
#define XLLP_U2DISR_IRSOF   ( 0x1U << 30 )	// Interrupt Request Strart of Frame
#define XLLP_U2DISR_IRUSOF	( 0x1U << 29)	// Interrupt Request u Start of Frame (HS only)
#define XLLP_U2DISR_IRRU	( 0x1U << 28)	// Interrupt Request Resume
#define XLLP_U2DISR_IRSU	( 0x1U << 27)	// Interrupt Request Suspend
#define XLLP_U2DISR_IRRS    ( 0x1U << 26)   // Interrupt Request Reset
#define XLLP_U2DISR_IRDPE   ( 0x1U << 25)   // Interrupt Request Data Packet Error
#define XLLP_U2DISR_RESV    ( 0x1U << 24)   // Reserved
#define XLLP_U2DISR_IEx_MASK 0x7U           // Mask for IEx fields
#define XLLP_U2DISR_CHAN_SHIFT 3            // Each Channel Interrupt field is 3 bits
#define XLLP_U2DISR_IEG_SHIFT 21            // IEx are 3 bit wide fields, these shift give low-order bit
#define XLLP_U2DISR_IEF_SHIFT 18            // One for each Endpoint 0, A, ... G
#define XLLP_U2DISR_IEE_SHIFT 15
#define XLLP_U2DISR_IED_SHIFT 12
#define XLLP_U2DISR_IEC_SHIFT  9
#define XLLP_U2DISR_IEB_SHIFT  6
#define XLLP_U2DISR_IEA_SHIFT  3
#define XLLP_U2DISR_IE0_SHIFT  0
#define XLLP_U2DISR_IEx_FEI_BIT ( 0x1U << 2)// These are the individual interrupt request bits within the IEx fields
#define XLLP_U2DISR_IEx_PCI_BIT ( 0x1U << 1)// Fifo Error, Packet Complete, Short Packet Complete Interrupts
#define XLLP_U2DISR_IEx_SPI_BIT ( 0x1U << 0)

//U2DFNR - Frame Number Register
//------------------------------
#define XLLP_U2DFNR_MASK 0x7FFU          // Read Only -> USB Frame Number

//U2DCSR0 - (U2DCSR_X )Endpoint 0 Control/Status Register
//-------------------------------------------
#define XLLP_U2DCSR0_RESV_MASK  0x000001FF   // Bits 9<->31 are reserved
#define XLLP_U2DCSR0_IPA       (0x1U << 8)   // In Packet Adjust             R/W
#define XLLP_U2DCSR0_SA		   (0x1U << 7)   // Setup Active				 R/W, 1 to clear
#define XLLP_U2DCSR0_RNE	   (0x1U << 6)   // Recieve Fifo not Empty       Read Only
#define XLLP_U2DCSR0_FST	   (0x1U << 5)   // Force Stall					 R/W, 1 to clear
#define XLLP_U2DCSR0_SST	   (0x1U << 4)   // Sent Stall                   Read Only
#define XLLP_U2DCSR0_DME	   (0x1U << 3)   // DMA enable                   R/W
#define XLLP_U2DCSR0_FTF	   (0x1U << 2)   // Flush Transmit Fifo          Read 0, 1 to set
#define XLLP_U2DCSR0_IPR	   (0x1U << 1)   // IN packet Ready              R/W 1 to set
#define XLLP_U2DCSR0_OPC	   (0x1U << 0)   // OUT packet complete          R/W, 1 to clear

//U2DCSR_X (A-G endpoints) Control Status Register bits
//-----------------------------------------------------
#define XLLP_U2DCSRX_RESV_MASK 0xFFFFF800    // Bits 11<->31 reserved
#define XLLP_U2DCSRX_BFBE	   (0x1U << 10)	 // Buffer Full, Buffer Empty Depends on IN/OUT setting of EP R/O
#define XLLP_U2DCSRX_DPE	   (0x1U << 9)   // Data Packet Error                             R/O
#define XLLP_U2DCSRX_FEF	   (0x1U << 8)   //	Flush Endpoint Fifo
#define XLLP_U2DCSRX_SP		   (0x1U << 7)   // Short Packet Control/Status, OUT EP's only    R/O 1=SP ready
#define XLLP_U2DCSRX_BNEBNF	   (0x1U << 6)   // Buffer Not Empty, Buffer Not full             R/O
#define XLLP_U2DCSRX_FST	   (0x1U << 5)   // Force Stall                                   R/W, 1 to set
#define XLLP_U2DCSRX_SST	   (0x1U << 4)   // Sent Stall                                    R/W 1 to clear
#define XLLP_U2DCSRX_DME	   (0x1U << 3)   // DMA Enable                                    R/W
#define XLLP_U2DCSRX_TRN	   (0x1U << 2)   // Tx/Rx NAK                                     R/W 1 to Clear
#define XLLP_U2DCSRX_PC		   (0x1U << 1)   // Packet Complete                               R/W, 1 to clear
#define XLLP_U2DCSRX_FS		   (0x1U << 0)   // FIFO needs service                            R/O

//U2DBCR0->U2DBCRG  Byte count Register
//-------------------------------------
#define XLLP_U2DBCRX_BC_MASK    (0x000007FF)  // Byte count register, bits 11<->31 reserved

//U2DCRA-U2DCRG Registers
//-----------------------
#define XLLP_U2DCRX_BS_SHIFT   1		                        // Shift and mask for FIFO buffer size setting
#define XLLP_U2DCRX_BS_MASK    (0x3FF << XLLP_U2DCRX_BS_SHIFT )	// FIFO buffer size x 8 (min setting is 2 -> 16 bytes)
#define XLLP_U2DCRX_EE         (0x1U << 0 )					    // Endpoint Enable R/W


//U2DENA->U2DENG Endpoint info (configuration...) registers
//---------------------------------------------------------
#define XLLP_U2DNEX_HBW_SHIFT  30                                 // High Bandwith commands/frame or uframe
#define XLLP_U2DNEX_HBW_MASK   (0x3U << XLLP_U2DNEX_HB_SHIFT)     //
#define XLLP_U2DNEX_MPS_SHIFT  19                                 // Max Packet Size
#define XLLP_U2DNEX_MPS_MASK   (0x7FFU << XLLP_U2DNEX_MPS_SHIFT)  //
#define XLLP_U2DNEX_AISN_SHIFT 15                                 // Alternate Interface number
#define XLLP_U2DNEX_AISN_MASK  (0xFU << XLLP_U2DNEX_AISN_SHIFT)   //
#define XLLP_U2DNEX_IN_SHIFT   11                                 // Interface Number
#define XLLP_U2DNEX_IN_MASK    (0xFU << XLLP_U2DNEX_IN_SHIFT)     //
#define XLLP_U2DNEX_CN_SHIFT   7                                  // Configuration Number
#define XLLP_U2DNEX_CN_MASK    (0xFU << XLLP_U2DNEX_CN_SHIFT)     //
#define XLLP_U2DNEX_ET_SHIFT   5								  // Endpoint Type: 11->int,10-> bulk,01->ISOC
#define XLLP_U2DNEX_ET_MASK    (0x3U << XLLP_U2DNEX_ET_SHIFT)     //
#define XLLP_U2DNEX_ED_SHIFT   4                                  // Endpoint shift and mask
#define XLLP_U2DNEX_ED         (0x1U << XLLP_U2DNEX_ED_SHIFT)     // Endpoint Direction 1->IN, 0=OUT
#define XLLP_U2DNEX_EN_SHIFT   0                                  // Endpoint number shift and mask
#define XLLP_U2DNEX_EN_MASK    (0xFU << XLLP_U2DNEX_EN_SHIFT)     // USB Endpoint number field 0->15

//U2DMACR->DMA Control Register
//-----------------------------
#define XLLP_U2DMA_RETRYOEN     (0x1U << 2)   // Internal System bus Retry time-out Enable  R/W
#define XLLP_U2DENX_MAXOCT_MASK	(0x3U)        // Max # outstanding Concurrent transactions allowed on SYSTEM bus


//U2DMABRx -> DMA Branch Registers
//--------------------------------
// Not used in this effort

//U2DMADARx -> DMA Descriptor Address Register
//--------------------------------------------
#define XLLP_U2DMADADRX_STOP          (0x1U)          // Stop Bit, indicates last Descriptor in chain

//U2DMACMDx DMA Command Register Channels 0<->7
//    Note that these are part of the DESCRIPTOR in RAM, but are R/O in the actual register...
//--------------------------------------------------------------------------------------------
#define XLLP_U2DMACMDX_XFRDIR_TX       (0x1U << 31)
#define XLLP_U2DMACMDX_XFRDIR          (0x1U << 31)  // Transfer direction, 0-> from FIFO(USB OUT);
                                                     // 1-> to FIFO(USB IN)
#define XLLP_U2DMACMDX_LSTDES_EN_SHIFT 23
#define XLLP_U2DMACMDX_LSTDES_EN_MASK  (0x3U << XLLP_U2DMACMDX_LSTDES_EN_SHIFT )  // Last Descriptor Enable
#define XLLP_U2DMACMDX_STARTIRQEN      (0x1U << 22)  // Enable Interrupt when THIS descriptor loaded to CMD register
#define XLLP_U2DMACMDX_ENDIRQEN        (0x1U << 21)  // End Interrupt Enable
#define XLLP_U2DMACMDX_PACKCOMP        (0x1U << 13)  // Packet Complete  R/O
#define XLLP_U2DMACMDX_LEN_SHIFT       0
#define XLLP_U2DMACMDX_LEN_MASK        (0x7FFU << XLLP_U2DMACMDX_LEN_SHIFT ) // LEN of transfer bytes...


//U2DMACSRx  DMA Control/Status registers Channels 0<->7
//------------------------------------------------------
#define XLLP_U2DMACSRX_RUN			   (0x1U << 31)  // Start/Stop DMA channel  R/W
#define XLLP_U2DMACSRX_RESV1		   (0x1U << 30)  // reserved
#define XLLP_U2DMACSRX_STOPIRQEN	   (0x1U << 29)  // Stop interrupt Enable
#define XLLP_U2DMACSRX_EORIRQEN		   (0x1U << 28)  // End of Recieve Interrupt Enable
#define XLLP_U2DMACSRX_EORJMPEN		   (0x1U << 27)  // Enable Jump to next Rx descriptor on EOR interrupt
#define XLLP_U2DMACSRX_EORSTOPEN	   (0x1U << 26)  // Stop Channel on EOR Interrupt
#define XLLP_U2DMACSRX_RESV2		   (0x1U << 25)  // reserved
#define XLLP_U2DMACSRX_RESV3		   (0x1U << 24)  //
#define XLLP_U2DMACSRX_RASIRQEN        (0x1U << 23)  // Request after Channel stopped IRQ Enable
#define XLLP_U2DMACSRX_MASKRUN         (0x1U << 22)	 // Mask Run: if 0 enables s/w to set RUN bit while DMA transfer
#define XLLP_U2DMACSRX_RESV4		   (0x1U << 21)
#define XLLP_U2DMACSRX_RESV5		   (0x1U << 20)
#define XLLP_U2DMACSRX_SCEMC_SHIFT     18            // Shift and mask for System Bus Split Completion Error msg.
#define XLLP_U2DMACSRX_SCEMC_MASK      (0x3U << XLLP_U2DMACSRX_SCEMC_SHIFT)
#define XLLP_U2DMACSRX_SCEMI_SHIFT     13            // Shift + mask for SCEM Error msg. type
#define XLLP_U2DMACSRX_SCEMI_MASK      (0x1FU << XLLP_U2DMACSRX_SCEMI_SHIFT)
#define XLLP_U2DMACSRX_BUSERTYPE_SHIFT 10            //  shift & mask for Px bus error type
#define XLLP_U2DMACSRX_BUSERTYPE_MASK  (0x7U << XLLP_U2DMACSRX_BUSERTYPE_SHIFT)
#define XLLP_U2DMACSRX_EORINTR         (0x1U << 9)   // End of Recieve interrupt status R/W 1 to clear
#define XLLP_U2DMACSRX_REQPEND		   (0x1U << 8)   // Request Pending for channel  R/O
#define XLLP_U2DMACSRX_RESV6		   (0x1U << 7)   // reserved
#define XLLP_U2DMACSRX_RESV7		   (0x1U << 6)   //
#define XLLP_U2DMACSRX_RESV8           (0x1U << 5)	 //
#define XLLP_U2DMACSRX_RASINTR		   (0x1U << 4)   // Fifo req. but chan stopped... R/W, 1 to clear
#define XLLP_U2DMACSRX_STOPINTR		   (0x1U << 3)	 // Channel Stopped Interrupt.  R/O
#define XLLP_U2DMACSRX_ENDINTR		   (0x1U << 2)   // End Interrupt, Descriptor req. complete... R/W, 1 to clear
#define XLLP_U2DMACSRX_STARTINTR	   (0x1U << 1)   // Start Interrupt, Descriptor loaded successfully R/W 1 to clear
#define XLLP_U2DMACSRX_BUSERRINTR	   (0x1U << 0)   // Bus Error interrupt, bad descriptor content...


//U2DMAINT -> U2DMA interrrupt register -> Interrupt per channel
//--------------------------------------------------------------
#define XLLP_U2DMAINT_INTERRUPT_MASK (0x000000FFU)  // Interrupt pending on channel bit0 == CHAN0, bit7 == CHAN7

typedef enum XLLP_U2D_EP_MPS_E
{
	U2D_MPS_0		  = 0,
    U2D_MPS_CNTRL_8   = 8,
    U2D_MPS_CNTRL_16  = 16,
    U2D_MPS_CNTRL_32  = 32,
    U2D_MPS_CNTRL_64  = 64,
    U2D_MPS_BULK_8    = 8,
    U2D_MPS_BULK_16   = 16,
    U2D_MPS_BULK_32   = 32,
    U2D_MPS_BULK_64   = 64,
    U2D_MPS_BULK_512  = 512
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
} XLLP_U2D_EP_MPS_T;


// Enumerate U2D Transfers Types
typedef enum XLLP_U2D_EP_TYPE_E
{
    U2D_CNTRL   = 0x00,
    U2D_ISO     = 0x01,
    U2D_BULK    = 0x02,
    U2D_INTERPT = 0x03
} XLLP_U2D_EP_TYPE_T;

// Enumerate U2D Endpoint Direction
typedef enum XLLP_U2D_EP_DIR_E
{
    U2D_OUT = 0x00,
    U2D_IN  = 0x01
} XLLP_U2D_EP_DIR_T;

// Enumerate U2D Endpoints
typedef enum XLLP_U2D_USB_EP_E
{
    U2D_USB_EP_0 = 0,
    U2D_USB_EP_1,
    U2D_USB_EP_2
} XLLP_U2D_USB_EP_T;


// U2D Endpoint Configuration Structure
typedef struct XLLP_U2D_EP_CONFIG_TABLE_S
{
    XLLP_U2D_EP_T             udcEndpointNum;        // U2D Endpoint Number: (0-7)
    XLLP_UINT8_T              usbConfigNum;          // Usb Configuration Number: (1-3)
    XLLP_UINT8_T              usbInterfaceNum;       // Usb Interface Number: (1-7)
    XLLP_UINT8_T              usbIntAltSettingsNum;  // Usb Interface Alternate Settings Number: (1-7)
    XLLP_U2D_USB_EP_T         usbEndpointNum;        // Usb Endpoint Number: (1-15)
    XLLP_U2D_EP_TYPE_T        usbEndpointType;       // Usb Endpoint type: Bulk, Iso, Interrupt
    XLLP_U2D_EP_DIR_T         usbEndpointDirection;  // Usb Endpoint direction: IN, OUT
    XLLP_U2D_EP_MPS_T         maxPacketSize;         // Max. Packet Size: (1-1023)
    XLLP_U2D_EP_DOUBLE_BUFF_T doubleBuffEnabled;     // Double Buffering Enabled if set to one
    XLLP_U2D_EP_ENABLED_T     endpointEnabled;       // Endpoint Enabled if set to one
    XLLP_UINT16_T             dmaFifoSize;           // Fifo allocation size
    XLLP_UINT32_T             endpointConfigValue;   // Endpoint Configuration Register Value
    XLLP_UINT32_T             endpointInfoValue;     // Endpoint Info Register Value
} XLLP_U2D_EP_CONFIG_TABLE_T, *P_XLLP_U2D_EP_CONFIG_TABLE_T;

// Endpoint binding
typedef struct XLLP_U2D_USB_BIND_ENDPOINTS_S
{
    XLLP_U2D_EP_T       udcEndpointNum;             // U2D Endpoint Number: (0-7)
    XLLP_U2D_USB_EP_T   usbEndpointNum;             // Usb Endpoint Number: (1-15)
} XLLP_U2D_USB_BIND_ENDPOINTS_T;

#define XLLP_U2D_USB_MAX_EP_NUM 16
//#define XLLP_U2D_MAX_EP_NUM     8
#define XLLP_U2D_MAX_EP_NUM     4
#define XLLP_U2D_MAX_EP_COUNT   8


// U2D Interrupt statistics structure
typedef struct XLLP_U2D_INT_STATISTICS_S
{
    XLLP_UINT32_T ResetIntCount;
    XLLP_UINT32_T SuspendIntCount;
    XLLP_UINT32_T ResumeIntCount;
    XLLP_UINT32_T SOFIntCount;
    XLLP_UINT32_T ConfigIntCount;
    XLLP_UINT32_T EpIntCount[XLLP_U2D_MAX_EP_NUM];
    P_XLLP_UINT32_T pRegDbg;
    XLLP_UINT32_T   totalCapturedCnt;
} XLLP_U2D_INT_STATISTICS_T, *P_XLLP_U2D_INT_STATISTICS_T;

// U2D Endpoints transfers structure
typedef struct XLLP_U2D_XFER_S
{
    P_XLLP_UINT32_T pDataEp;
    XLLP_UINT32_T   dmaChannel;
    XLLP_UINT32_T   xferLength;
    XLLP_UINT32_T   xferDataCounter;
    XLLP_UINT32_T   maxPacketSize;
    XLLP_BOOL_T     TxXferComplete;
    XLLP_BOOL_T     RxXferComplete;
    XLLP_BOOL_T     RxBlockXferComplete;
    XLLP_BOOL_T     enableLoopback;
} XLLP_U2D_XFER_T, *P_XLLP_U2D_XFER_T;

// U2D Control Transfer structure
typedef struct XLLP_U2D_USB_CTRL_XFER_S
{
    XLLP_UINT32_T   statusEp0;
    P_XLLP_UINT32_T pTxBuffEp0;
    P_XLLP_UINT32_T pRxBuffEp0;
    XLLP_UINT32_T   outDataEp0[64];
    XLLP_UINT16_T   descIndexEp0;
    XLLP_UINT16_T   descTypeEp0;
    XLLP_UINT32_T   dataLengthEp0;
} XLLP_U2D_USB_CTRL_XFER_T, *P_XLLP_U2D_USB_CTRL_XFER_T;

// U2D Vendor Request structure
typedef struct XLLP_U2D_VENDOR_REQ_DATA_S
{
    XLLP_UINT8_T    recipient;
    XLLP_UINT8_T    vendorReqType;
    XLLP_UINT16_T   vendorReqEndpointNum;
    XLLP_UINT32_T   vendorReqXferLength;
    XLLP_BOOL_T     vendorReqComplete;
} XLLP_U2D_VENDOR_REQ_DATA_T, *P_XLLP_U2D_VENDOR_REQ_DATA_T;


// U2D Device Context structure
//-----------------------------
typedef struct XLLP_U2D_S
{
    P_XLLP_U2D_REGISTERS_T        pRegsBase;                                  // Pointer to U2D's registers
    P_XLLP_U2D_EP_CONFIG_TABLE_T  pConfigTable;                               // Pointer to the Endpoints Config. table
    XLLP_U2D_USB_BIND_ENDPOINTS_T listOfActiveEndpoints[XLLP_U2D_MAX_EP_NUM]; // Used to keep track of endpoints in the active configuration
    XLLP_U2D_INT_STATISTICS_T     interruptStat;                              // Used to keep track ot the interrupts statistics
    //XLLP_U2D_XFER_T               EpXferTable[XLLP_U2D_MAX_EP_NUM];         // Not needed for U2D implementation
    XLLP_U2D_USB_CTRL_XFER_T      controlXfer;                                // Used to process control transfers
    XLLP_U2D_VENDOR_REQ_DATA_T    vendorReq;                                  // Used to process vendor requests
    XLLP_BOOL_T                   enableDma;                                  // Selects DMA to service FIFOs if set
    XLLP_BOOL_T                   setupComplete;                              // Used to track the complition of the setup
    XLLP_BOOL_T                   enumerateComplete;                          // Used to track the complition of the enumerate command
    XLLP_BOOL_T                   cableAttached;                              // Used to monitor the cable attachment and disconnect
    XLLP_UINT8_T                  usbConfigNumActive;                         // Active Usb Configuration Number: (1-3)
    XLLP_UINT8_T                  usbInterfaceNumActive;                      // Active Usb Interface Number: (1-7)
    XLLP_UINT8_T                  usbIntAltSettingsNumActive;                 // Active Usb Interface Alternate Settings Number: (1-7)
} XLLP_U2D_T, *P_XLLP_U2D_T;

// U2D Generic Error codes...
//---------------------------
typedef enum U2D_ERR_ENUM
{
    U2D_ERR_NO_ERROR = 0,
    U2D_ERR_BAD_BUFFER_ALIGN,
    U2D_ERR_DESCRIPTOR_ALLOC_FAIL,
    U2D_ERR_ENDPOINT_FAIL,
    U2D_ERR_ENDPOINT_NO_FIFO,
    U2D_ERR_ENDPOINT_CONFIG_ERROR,
    U2D_ERR_ENDPOINT_BIND,
    U2D_ERR_FIFO_SIZE_TOO_SMALL,
    U2D_ERR_TRANSFER_BUFF_NULL,
    U2D_ERR_DMA_BUS_ERR,
    U2D_DMA_UNKNOWN_INTERRUPT,
    U2D_ERR_ENDPOINT_FIFO_ERR
}XLLP_U2D_ERR_CODE_T;


// Reserve memory for U2D DMA descriptors and transfer buffers
//--------------------------------------------------------------
#define SYS_NUM_ACTIVE_A_TO_G_ENDPOINTS 2        /* endpoints A & B for DKB */
#define SYS_U2D_NUM_DMA_DESCRIPTORS ((SYS_NUM_ACTIVE_A_TO_G_ENDPOINTS)*15)
#define SYS_U2D_DMA_DESCRIPTOR_MEM  (SYS_U2D_NUM_DMA_DESCRIPTORS*sizeof(XLLP_U2D_DMA_RAM_DESCR_T))
#define SYS_U2D_DMA_ENDPOINT_A_TRANSFER_MEM_SIZE   (1024)
#define SYS_U2D_DMA_ENDPOINT_B_TRANSFER_MEM_SIZE   (5*1024)
#define SYS_U2D_TRANSFER_BUFFER_MEM (SYS_U2D_DMA_ENDPOINT_A_TRANSFER_MEM_SIZE+SYS_U2D_DMA_ENDPOINT_B_TRANSFER_MEM_SIZE)

//---------------------------------------------------------------------------------------------
// Function Prototypes, not generic Public func's
//
//--------------------------------------------------------------------------------------------
XLLP_UINT32_T UnitTestU2Ddma( void );
P_XLLP_UINT8_T U2DGlobalMemoryAlloc( XLLP_UINT32_T requestSizeBytes, XLLP_UINT32_T byteAlign );
XLLP_UINT8_T XllpU2DInit( P_XLLP_U2D_T pHandle );
XLLP_UINT8_T XllpU2DMAxfer( XLLP_U2D_USB_EP_T  USBEndPtId, // USB Endpoint
                            P_XLLP_UINT8_T     BufPtr,     // Memory buffer
                            XLLP_UINT32_T      BufSize,    // Buffer Size
                            XLLP_UINT8_T       SynchronusTransfer, // return after transmit complete?
                            XLLP_INT32_T       *RxByteCount        // actual # bytes received
                          );
XLLP_UINT8_T U2DLogError( XLLP_UINT8_T Error );
void XllpU2DClearInterrupt (P_XLLP_U2D_T pUdcHandle,
                            XLLP_U2D_INTERRUPTS_T udcEndpointNum,
                            XLLP_U2D_EP_INTERRUPT_TYPE_T interruptType);
void XllpU2DEp0InterruptHandler (P_XLLP_U2D_T pUdcHandle);
void XllpU2DConfigInterruptHandler (P_XLLP_U2D_T pUdcHandle);
void XllpU2DEnableInterrupt( P_XLLP_U2D_T pUdcHandle,
                             XLLP_U2D_INTERRUPTS_T udcEndpointNum,
                             XLLP_U2D_EP_INTERRUPT_TYPE_T interruptType);
void XllpU2DDisableInterrupt( P_XLLP_U2D_T pUdcHandle,
                              XLLP_U2D_INTERRUPTS_T udcEndpointNum,
                              XLLP_U2D_EP_INTERRUPT_TYPE_T interruptType);
XLLP_STATUS_T XllpU2DHWSetup(P_XLLP_U2D_T pUdcHandle);
XLLP_STATUS_T XllpU2DHWShutdown (P_XLLP_U2D_T pUdcHandle);
void XllpU2DSWInit (P_XLLP_U2D_T pUdcHandle, P_XLLP_U2D_REGISTERS_T pRegs);
XLLP_UINT32_T XllpU2DGetEndPointTransferDirection( XLLP_U2D_EP_T u2dEndPtId );
XLLP_U2D_EP_T XllpU2DGetBinding( XLLP_U2D_USB_EP_T USBEndPtId );
XLLP_STATUS_T XllpU2DProcessVendorRequest (P_XLLP_U2D_T pU2DHandle);

//------------------------------------------------------------------------------------------------------
// PUBLIC FUNCTIONS
//------------------------------------------------------------------------------------------------------
XLLP_UINT32_T U2DTransmit(XLLP_UINT32_T TxDataLength, P_XLLP_UINT8_T pTxBuff, XLLP_UINT8_T WaitState);
XLLP_UINT32_T U2DReceive(XLLP_UINT32_T RxDataLength, XLLP_UINT8_T  *pRxBuff, XLLP_INT32_T *byteCount );
XLLP_UINT8_T USBEndPointPacketInterruptReceived( XLLP_U2D_USB_EP_T usbEp );
XLLP_UINT32_T U2DDownload( void );
XLLP_UINT32_T BootRomU2DHWShutdown (void);
XLLP_UINT32_T BootRomU2DHWSetup(void);
XLLP_STATUS_T XllpU2DEnableFirstLevelInterrupt(void);
XLLP_STATUS_T XllpU2DDisableFirstLevelInterrupt(void);
void BootRomU2DSWInit (void);
void U2DReInit( void );
void U2DDownloadInit(void);                         // Application (DKB) starts the U2D world with this

//---------------------------------------------------------------
// For interrupt context handling only
//---------------------------------------------------------------
void U2DInterruptHandler (void);
void ImageU2DInterruptHandler(void);
void XllpU2DmaInterruptHandler( void );

#endif /* _xllp_u2d_h */
