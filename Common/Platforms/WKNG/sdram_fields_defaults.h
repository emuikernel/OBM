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



#ifndef __TTC_SDRAM_FIELDS__
#define __TTC_SDRAM_FIELDS__


// default mcu field values for TTC

	
	// SDRAM Configuration Register 0 - CS0, Type 1
	// offset 0x00000020
	// TTC value:		0x00001330 
	//					0 000 000000000000 00 01 0011 0011 0000
#define TTC_CFGR0_RSVD1							0
#define	TTC_CFGR0_PASRn							0
#define TTC_CFGR0_RSVD2							0
#define	TTC_CFGR0_ODSn							0
#define TTC_NumBanks							1	// 4 banks
#define TTC_NumRows								3	// 13 row address bits	
#define TTC_NumColumns							3	// 10 column address bits
#define	TTC_CFGR0_RSVD2							0


// SDRAM Configuration Register 1 - CS1, Type 1
	// offset 0x00000030
	// TTC value:		0x00801320
	//					0 000 000010000000 00 01 0011 0010 0000
#define TTC_CFGR1_RSVD1							0
#define	TTC_CFGR1_PASRn							0
#define TTC_CFGR1_RSVD2							0x080
#define	TTC_CFGR1_ODSn							0
#define TTC_NumBanks_CS1						1	// 8 banks
#define TTC_NumRows_CS1							3	// 13 row address bits	
#define TTC_NumColumns_CS1						2	// 9 column address bits
#define	TTC_CFGR1_RSVD3							0


	// SDRAM Timing Register 1
	// offset 0x00000050
	// TTC value:		0x491505a0
	//					010 010 0100 010101 0000010110100000
#define TTC_tCCD								2
#define TTC_tRTP								2
#define TTC_tWTR								4
#define TTC_tRC									0x15
#define TTC_tREFI							    0x5a0		// FIXME: different from default. Why? 
															// Answer: refresh clock is not = dclk. it is flck, set to 26MHz.

	// SDRAM Timing Register 2
	// offset 0x00000060
	// Aspen value:		0x63330332
	//					0110 0011 0011 0011 000 000110011 0 010
#define TTC_tRP									6
#define TTC_tRRD								3
#define TTC_tRCD								3
#define TTC_tWR									3
#define TTC_TMGREG2_RSVD1						0
#define TTC_tRFC								0x33
#define TTC_TMGREG2_RSVD2						0
#define TTC_tMRD								2


	// SDRAM Control Register 2
	// offset 0x00000090
	// TTC value:		0x00080000*
	//					0000 0 000 00 000000 1000000000 0 0 00 0 0
	// note: in ddrinit.s, sdram control register 2 is 0x00008000. i think it should be 0x00080000.
#define TTC_CTLREG2_RSVD1						0
#define TTC_REF_POSTED_EN						0
#define TTC_REF_POSTED_MAX						0
#define TTC_CTLREG2_RSVD2						0
#define TTC_SDRAM_LINE_BOUNDARY					8
#define TTC_CTLREG2_RSVD3						0
#define TTC_RDIMM_MODE							0
#define TTC_APRECHARGE							0
#define TTC_CTLREG2_RSVD4						0
#define TTC_CTLREG2_RSVD5						0
#define TTC_TEST_MODE							0


	// SDRAM Control Register 4
	// offset 0x000001a0
	// TTC value:		0x0080C011
	//					00 0 0000 010 0000 0 011 000000000 1 00 01
#define TTC_CTLREG4_RSVD1						0
#define TTC_FAST_BANK							0
#define TTC_CTLRG4_RSVD2						0
#define TTC_BURST_LENGTH						2
#define TTC_CTLREG4_RSVD3						0
#define TTC_RQDS_EN								0
#define TTC_CTLREG4_RSVD4						3	// probably cas latency cycles
#define TTC_CTLREG4_RSVD5						0
#define TTC_MOBILE_SDRAM						1
#define TTC_SDRAM_TYPE							0
#define TTC_DATA_WIDTH							1	// FIXME: why isn't 1 used for 32 bit internal bus?


	// PHY Control Register 3
	// offset 0x00000140
	// TTC value:		0x20004411
	//					0010000000000000 0 1 0 0 01 000 001 0001
#define TTC_PHYCR3_RSVD1						0x2000			// FIXME: reserved is non-zero. Why?
#define TTC_PHY_RFIFO_RDRST_EARLY				0
#define TTC_PHY_RFIFO_RDRST_EN					1
#define TTC_PHYCR3_RSVD2						0
#define TTC_DQ_OEN_EXTEND						0
#define TTC_DQ_OEN_DLY							1
#define TTC_RD_EXT_DLY							0
#define TTC_PHY_RFIFO_RPTR_DLY_VAL				1
#define TTC_DQ_EXT_DLY							1


	// Mememory Address Map Register 0
	// offset 0x00000100
	// TTC value:		0x000b0781
	//					000000000 000 1011 000000000 000000 1
#define TTC_MEMADDRMAPR0_START_ADDR				0
#define TTC_MEMADDRMAPR0_RSVD1					0
#define TTC_MEMADDRMAPR0_AREA_LENGTH			0xb
#define TTC_MEMADDRMAPR0_ADDR_MASK				0
#define TTC_MEMADDRMAPR0_CS_VALID				1


	// User Initiated Command Register 0
	// offset 0x00000120
	// Morona value:	0x00000001
	//					00 00 0000 0000000000 0 0 0 0 0 0 00 0 0 000 1
	// TTC value:		0x000000001
	//					00 00 0000 0000000000 0 0 0 0 0 0 00 0 0 000 1
#define TTC_USER_DPD_REQ						0
#define TTC_CHIP_SELECT							0
#define TTC_USER_ZQ_SHORT						0
#define TTC_USER_ZQ_LONG						0
#define TTC_USER_LMR3_REQ						0
#define TTC_USER_LMR2_REQ						0
#define TTC_USER_LMR1_REQ						0
#define TTC_USER_LMR0_REQ						0
#define TTC_USER_SR_REQ							0
#define TTC_USER_PRE_PS_REQ						0
#define TTC_USER_ACT_PS_REQ						0
#define TTC_SDRAM_INIT_REQ						1


#endif
