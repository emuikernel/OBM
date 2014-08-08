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
 
 ******************************************************************************/



#ifndef __SDRAM_REGISTERS_DEFAULTS_H__
#define __SDRAM_REGISTERS_DEFAULTS_H__

// default mcu register values for TTC
#include "MCU.h"
#include "sdram_fields_defaults.h"


// group the fields so there is one definition per register:
	// SDRAM Configuration Register 0 - CS0, Type 1
	// offset 0x00000020
	// TTC value:	0x00002330
	//					0 000 000000 00 000 000 10 0011 0011 0000
#define TTC_SDRCFGREG0															\
			(																	\
			( TTC_NumBanks    << MCU_SDRAM_CONFIG_0_CS0_BANK_NUMBER_BASE  ) |	\
			( TTC_NumRows     << MCU_SDRAM_CONFIG_0_CS0_NO_OF_ROW_BASE    ) |	\
			( TTC_NumColumns  << MCU_SDRAM_CONFIG_0_CS0_NO_OF_COL_BASE    )		\
			)

	// SDRAM Configuration Register 0 - CS0, Type 1
	// offset 0x00000030
	// TTC value:	0
#define TTC_SDRCFGREG1																\
			(																		\
			( TTC_CFGR1_RSVD2		<< 16                                       ) |	\
			( TTC_NumBanks_CS1		<< MCU_SDRAM_CONFIG_0_CS0_BANK_NUMBER_BASE  ) |	\
			( TTC_NumRows_CS1		<< MCU_SDRAM_CONFIG_0_CS0_NO_OF_ROW_BASE    ) |	\
			( TTC_NumColumns_CS1	<< MCU_SDRAM_CONFIG_0_CS0_NO_OF_COL_BASE    )	\
			)

	// SDRAM Timing Register 1
	// offset 0x00000050
	// TTC value:	0x49150070
	//					010 010 0100 010101 0000000001110000
#define TTC_SDRTMGREG1												\
			(														\
			( TTC_tCCD		<< MCU_SDRAM_TIMING_1_TCCD_BASE)	|	\
			( TTC_tRTP		<< MCU_SDRAM_TIMING_1_TRTP_BASE)	|	\
			( TTC_tWTR		<< MCU_SDRAM_TIMING_1_TWTR_BASE)	|	\
			( TTC_tRC		<< MCU_SDRAM_TIMING_1_TRC_BASE)		|	\
			( TTC_tREFI	    << MCU_SDRAM_TIMING_1_TREFI_BASE)		\
			)

	// SDRAM Timing Register 2
	// offset 0x00000060
	// TTC value:	0x63330332
	//					0110 0011 0011 0011 000 000110011 0 010
#define TTC_SDRTMGREG2												\
			(														\
			( TTC_tRP	    << MCU_SDRAM_TIMING_2_TRP_BASE)		|	\
			( TTC_tRRD	    << MCU_SDRAM_TIMING_2_TRRD_BASE)	|	\
			( TTC_tRCD	    << MCU_SDRAM_TIMING_2_TRCD_BASE)	|	\
			( TTC_tWR	    << 16)								|	\
			( TTC_tRFC	    << MCU_SDRAM_TIMING_2_TRFC_BASE)	|	\
			( TTC_tMRD	    << MCU_SDRAM_TIMING_2_TMRD_BASE)		\
			)

	// SDRAM Control Register 2
	// offset 0x00000090
	// TTC value:	0x00080000
	//					0000 0 000 00 001000 000000 0 0 0 0 0 0 0 0 0 0
#define TTC_SDRCTLREG2																	\
			(																			\
			( TTC_REF_POSTED_EN		    << 27)										|	\
			( TTC_REF_POSTED_MAX		<< MCU_SDRAM_CONTROL_2_REF_POSTED_MAX_BASE)	|	\
			( TTC_SDRAM_LINE_BOUNDARY	<< 16)										|	\
			( TTC_RDIMM_MODE			<< 5) 										| 	\
			( TTC_APRECHARGE			<< 4) 										| 	\
			( TTC_TEST_MODE			    << 0) 											\
			)

	// SDRAM Control Register 4
	// offset 0x000001a0
	// TTC value:	0x40810005
	//                  0 1 0 0000 010 000 0 0 100 0 000 0 0 0 0 0 001 01
#if 0
#define TTC_SDRCTLREG4									\
			(											\
			( TTC_SDRAM_DLL_EN			<< 31)		| 	\
			( TTC_DQSB_EN				<< 30)		| 	\
			( TTC_FAST_BANK				<< 29)		| 	\
			( TTC_BURST_LENGTH			<< 22)		| 	\
			( TTC_AL_NUMBER			    << 19)		| 	\
			( TTC_AL_EN				    << 18)		| 	\
			( TTC_RQDS_EN				<< 17)		| 	\
			( TTC_CAS_LATENCY			<< 14)		| 	\
			( TTC_CAS_LATENCY_LOWER	    << 13)		| 	\
			( TTC_CWL					<< 10)		| 	\
			( TTC_S4_TYPE				<< 8)		| 	\
			( TTC_ASR					<< 7)		| 	\
			( TTC_SRT					<< 6)		| 	\
			( TTC_MPR					<< 5)		| 	\
			( TTC_SDRAM_TYPE			<< 2)		| 	\
			( TTC_DATA_WIDTH			<< 0)			\
			)
#endif
#define TTC_SDRCTLREG4									\
			(											\
			( TTC_FAST_BANK				<< 29)		| 	\
			( TTC_BURST_LENGTH			<< 22)		| 	\
			( TTC_RQDS_EN				<< 17)		| 	\
			( TTC_CTLREG4_RSVD4			<< 14)		| 	\
			( TTC_MOBILE_SDRAM			<< 4)		| 	\
			( TTC_SDRAM_TYPE			<< 2)		| 	\
			( TTC_DATA_WIDTH			<< 0)			\
			)

	// PHY Control Register 3
	// offset 0x00000140
	// TTC value:	0x200050A0
	//					0010000000000000 0 1 0 1 00 001 010 0000
#define TTC_PHYCTLREG3										\
			(												\
			( TTC_PHYCR3_RSVD1					<< 16)	|	\
			( TTC_PHY_RFIFO_RDRST_EARLY			<< 15)	|	\
			( TTC_PHY_RFIFO_RDRST_EN		    << 14)	|	\
			( TTC_DQ_OEN_EXTEND					<< 12)	|	\
			( TTC_DQ_OEN_DLY				    << 10)	|	\
			( TTC_RD_EXT_DLY				    << 7)	|	\
			( TTC_PHY_RFIFO_RPTR_DLY_VAL	    << 4)	|	\
			( TTC_DQ_EXT_DLY				    << 0)		\
			)


	// Mememory Address Map Register 0
	// offset 0x00000100
	// TTC value:	0x000C0001
	//					000000000 000 1100 000000000 000000 1
#define TTC_ADRMAPREG0																	\
			(																			\
			( TTC_MEMADDRMAPR0_START_ADDR		<< MMU_MMAP0_START_ADDRESS_BASE)	|	\
			( TTC_MEMADDRMAPR0_AREA_LENGTH		<< MMU_MMAP0_AREA_LENGTH_BASE)		|	\
			( TTC_MEMADDRMAPR0_ADDR_MASK		<< MMU_MMAP0_ADDRESS_MASK_BASE)		|	\
			( TTC_MEMADDRMAPR0_CS_VALID			<< 0)									\
			)

	// User Initiated Command Register 0
	// offset 0x00000120
	// TTC value:	0x00000001
	//					00 00 0000 0000000000 0 0 0 0 0 0 00 0 0 000 1
#define TTC_USRCMDREG0																	\
			(																			\
			( TTC_USER_DPD_REQ		<< 28) 											| 	\
			( TTC_CHIP_SELECT		<< 24) 											| 	\
			( TTC_USER_ZQ_SHORT	    << 13) 											| 	\
			( TTC_USER_ZQ_LONG		<< 12) 											| 	\
			( TTC_USER_LMR3_REQ	    << 11) 											| 	\
			( TTC_USER_LMR2_REQ	    << 10) 											| 	\
			( TTC_USER_LMR1_REQ	    << 9) 											| 	\
			( TTC_USER_LMR0_REQ	    << 8) 											| 	\
			( TTC_USER_SR_REQ		<< MCU_USER_INITIATED_COMMAND_USER_SR_REQ_BASE)	| 	\
			( TTC_USER_PRE_PS_REQ	<< 5) 											|	\
			( TTC_USER_ACT_PS_REQ	<< 4) 											|	\
			( TTC_SDRAM_INIT_REQ	<< 0) 												\
			)


#endif
