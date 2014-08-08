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
**	 xllp_dfc_defs.h
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
 *  FILENAME: xllp_dfc_defs.h
 *
 *  PURPOSE:  XLLP DFC definitions. REFERENCE: Boerne PX Unit IAS, chapter 20 (DFC)
 *
******************************************************************************/

#ifndef __XLLP_DFC_DEFS_H__
#define __XLLP_DFC_DEFS_H__

#include "Errors.h"
#include "NFC.h"

typedef enum {DMA_MODE, NONDMA_MODE}     TRANSFER_MODE;

// Buffer type passed in to the xllp routines for writing/reading.
typedef unsigned int *  P_DFC_BUFFER;

//Device Types
typedef enum {LARGE, SMALL, DEVICE_TYPE_SIZE} DEVICE_TYPE;

// Specific Manufactorer ID's
typedef enum {
   SAMSUNG_CODE		= 0xEC,
//   SANDISK_CODE = 0xFF,  // NOTE: find this out
   TOSHIBA_CODE		= 0x98,
   HYNIX_CODE		= 0xAD,
   ST_CODE			= 0x20,
   MICRON_CODE		= 0x2C,
   EMST_CODE		= 0xC8,
   SPANSION_CODE	= 0x01
}MAKER_CODE;

//
// DFC state information.
//
typedef struct {
   int  DMA_EN;     // DMA, or nonDMA mode.
   int  ECC_EN;     // Error correction on, off.
   int  SPARE_EN;   // Spare enable.
   int  ND_MODE;    // NAND, CARBONDALE, PIXLEY...
   int  DWIDTH_C;   // Control bus width.
   int  ND_ARB_EN;  // Data flash bus arbiter enable.
   int  chip;       // Monahans or Tavor
   } FLASH_STATE;


//
// Flash specifications.
//
typedef struct {
   /* timing */
   int  tCH;              // Enable signal hold time.
   int  tCS;              // Enable signal setup time.
   int  tWH;              // ND_nWE high duration.
   int  tWP;              // ND_nWE pulse time.
   int  tRH;              // ND_nRE high duration.
   int  tRP;              // ND_nRE pulse width.
   int  tR;               // ND_nWE high to ND_nRE low for read.
   int  tWHR;             // ND_nWE high to ND_nRE low delay for status read.
   int  tAR;              // ND_ALE low to ND_nRE low delay.
    } FLASH_TIMING;

#define TIMING_MAX_tCH       7
#define TIMING_MAX_tCS       7
#define TIMING_MAX_tWH       7
#define TIMING_MAX_tWP       7
#define TIMING_MAX_tRH       7
#define TIMING_MAX_tRP       7
#define TIMING_MAX_tR    65535
#define TIMING_MAX_tWHR     15
#define TIMING_MAX_tAR      15


#define STATUS_PAD_2048  10
#define STATUS_PAD_512    2


typedef enum {
   DFC_DMA_COMMAND = 16,
   DFC_DMA_DATA    = 17,
   } DFC_CHANNELS;

//
// Masks to extract the address segments.
//
#define DFC_COL_MASK  0x000000FF
#define DFC_ROW_MASK  0x01FFFE00 // NOTE: Shift or mask bit 8 ? ?

//Timeout for reset command in milliseconds

#define RESET_TIMEOUT	2

//
//  The following 4 lines are for the DFC workaround I.E. bucket #4
//
// Bucket #4 – “NAND Clock gating issue” – present on MHN-L and MHN-LV
//
//Believed to be root-caused to missing sync logic for register writes between PXB and NAND clock domains 
//Option #1 – Restrict SW from writing the NDCR register while the NAND clock is enabled 
//				MHN-L requires this SW work-around for ALL NDCR writes, except when setting only the RUN bit.  SV automation results with SW work-around = 100%! 
//				MHN-LV requires this SW work-around if the “READ_ID_CNT” value changes in the NDCR.  SV automation results with SW work-around = 100%! 
//Option #2 – Make “metal-only” silicon change to force the NAND clock off during all NAND “IDLE” states.  This is still under evaluation. 
//Option #3 – Make “full-layer” silicon change to sync register writes between the PXB and NAND clock domains. 
//
//
//


#define	ADDR_CCU_D0CKEN_A (volatile unsigned long *)(0x4134000C)
#define ADDR_DOCKEN_A_NAND_MASK 0xFFFFFFEF
#define DCF_ARB_MASK 0xFFFFEFFF


//
// DFC Initial timing parameters used in XllpDfcInit() are located in PlatformConfig.h for each platform
//

//
// Timing register parameter 0.
//
//  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
// +-------------------+-----+-----+---+-----+-----+---+-----+-----+
// |                   | tCH | tCS |   | tWH | tWP |   | tRH | tRP |
// +-------------------+-----+-----+---+-----+-----+---+-----+-----+
//
#define DFC_TIMING_tRP   0
#define DFC_TIMING_tRH   3
#define DFC_TIMING_tWP   8
#define DFC_TIMING_tWH  11
#define DFC_TIMING_tCS  16
#define DFC_TIMING_tCH  19

struct NDTR0CS0_DEF{
	int tRP 		: 3;
	int tRH			: 3;
	int etRP		: 1;
	int reserved0	: 1;
	int tWP			: 3;
	int tWH			: 3;
	int tRHW		: 2;
	int tCS			: 3;
	int tCH			: 3;
	int Rd_Cnt_Del	: 4;
	int SelCntr		: 1;
	int tADL		: 5;
};

typedef union {
	unsigned int value;
	struct NDTR0CS0_DEF bits;
}NDTR0CS0_REG;



//
// Timing register parameter 1.
//
//  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
// +-------------------------------+---------------+-------+-------+
// |               tR              |               | tWHR  |  tAR  |
// +-------------------------------+---------------+-------+-------+
//
#define DFC_TIMING_tAR   0
#define DFC_TIMING_tWHR  4
#define DFC_TIMING_tR   16

struct NDTR1CS0_DEF
{
	int tAR 		: 4;
	int tWHR		: 4;
	int reserved1	: 6;
	int prescale	: 1;
	int wait_mode	: 1;
	int tR			: 16;
};

typedef union {
	unsigned int value;
	struct NDTR1CS0_DEF bits;
}NDTR1CS0_REG;





//
// DFC register values for each control point.
//
//  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
// +-+-+-+-+-+-+---+-+---+-+-+-----+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | |   | |   | | |     | | | | | | | | | | | | | | | | |
// +-+-+-+-+-+-+---+-+---+-+-+-----+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  S E D N D D  P  N  N  C C   R   R P r N R C C C C C C D S W R W
//  P C M D W W  A  C  D  L L   D   A G e D D S S S S S S B B R D R
//  A C A _ I I  G  S  _  R R   _   _ _ s _ Y 0 1 0 1 0 1 E E D D C
//  R _ _ R D D  E  X  M  _ _   I   S P e A M _ _ _ _ _ _ R R R R M
//  E E E U T T  _     O  P E   D   T E r R   P P C C B B R R E E D
//  _ N N N H H  S     D  G C   _   A R v B   A A M M B B M M Q Q R
//  E       _ _  Z     E  _ C   C   R _ e _   G G D D D D     M M E
//  N       C M           C     N   T B d E   E E D D M M         Q
//                        N     T     L   N   D D M M             M
//                        T           K       M M
//
#define DFC_CTRL_WRCMDREQM    0x00000001
#define DFC_CTRL_RDDREQM      0x00000002
#define DFC_CTRL_WRDREQM      0x00000004
#define DFC_CTRL_SBERRM       0x00000008
#define DFC_CTRL_DBERRM       0x00000010
#define DFC_CTRL_CS1_BBDM     0x00000020
#define DFC_CTRL_CS0_BBDM     0x00000040
#define DFC_CTRL_CS1_CMDDM    0x00000080
#define DFC_CTRL_CS0_CMDDM    0x00000100
#define DFC_CTRL_CS1_PAGEDM   0x00000200
#define DFC_CTRL_CS0_PAGEDM   0x00000400
#define DFC_CTRL_RDYM         0x00000800
#define DFC_CTRL_ND_ARB_EN    0x00001000
#define DFC_CTRL_PG_PER_BLK   0x00004000
#define DFC_CTRL_RA_START     0x00008000
#define DFC_CTRL_RD_ID_CNT0   0x00000000
#define DFC_CTRL_RD_ID_CNT1   0x00010000
#define DFC_CTRL_RD_ID_CNT2   0x00020000
#define DFC_CTRL_RD_ID_CNT3   0x00030000
#define DFC_CTRL_RD_ID_CNT4   0x00040000
#define DFC_CTRL_RD_ID_CNT5   0x00050000
#define DFC_CTRL_RD_ID_CNT6   0x00060000
#define DFC_CTRL_RD_ID_CNT7   0x00070000
#define DFC_CTRL_CLR_ECC      0x00080000 // Note: This bit is now reserved and should always be written as 0.
#define DFC_CTRL_CLR_PG_CNT   0x00100000
#define DFC_CTRL_ND_MODE0     0x00000000
#define DFC_CTRL_ND_MODE1     0x00200000
#define DFC_CTRL_ND_MODE2     0x00400000
#define DFC_CTRL_ND_MODE3     0x00600000
#define DFC_CTRL_NCSX         0x00800000
#define DFC_CTRL_PAGE_SZ0     0x00000000
#define DFC_CTRL_PAGE_SZ1     0x01000000
#define DFC_CTRL_PAGE_SZ2     0x02000000
#define DFC_CTRL_PAGE_SZ3     0x03000000
#define DFC_CTRL_DWIDTH_M     0x04000000
#define DFC_CTRL_DWIDTH_C     0x08000000
#define DFC_CTRL_ND_RUN       0x10000000
#define DFC_CTRL_DMA_EN       0x20000000
#define DFC_CTRL_ECC_EN       0x40000000
#define DFC_CTRL_SPARE_EN     0x80000000

#define DFC_OFFSET_RD_ID_CNT          16

struct NDCR_DEF{
	unsigned WRCMDREQM 	: 1;
	unsigned RDDREQM	: 1;
	unsigned WRDREQM	: 1;
	unsigned SBERRM		: 1;
	unsigned DBERRM		: 1;
	unsigned CS1_BBDM	: 1;
	unsigned CS0_BBDM	: 1;
	unsigned CS1_CMDDM	: 1;
	unsigned CS0_CMDDM	: 1;
	unsigned CS1_PAGEDM	: 1;
	unsigned CS0_PAGEDM	: 1;
	unsigned RDYM		: 1;
	unsigned ND_ARB_EN	: 1;
	unsigned PG_PER_BLK	: 2;
	unsigned RA_START	: 1;
	int		 RD_ID_CNT	: 3;
	unsigned CLR_ECC	: 1;
	unsigned CLR_PG_CNT	: 1;
	int		 ND_MODE	: 2;
	unsigned NCSX		: 1;
	int		 PAGE_SZ	: 2;
	unsigned DMWIDTH_M	: 1;
	unsigned DMWIDTH_C	: 1;
	unsigned ND_RUN		: 1;
	unsigned DMA_EN		: 1;
	unsigned ECC_EN		: 1;
	unsigned SPARE_EN	: 1;
};

typedef union {
	unsigned int value;
	struct NDCR_DEF bits;
}NDCR_REG;

//
// DFC register offsets for each control point in command buffer 0.
//
//  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
// +-----+-+---+-+-+-----+-+-+-----+---------------+---------------+
// |     | |   | | |     | | |     |               |               |
// +-----+-+---+-+-+-----+-+-+-----+---------------+---------------+
//   C    L res A C   C   N D   A         CMD2            CMD1
//   M    E     U S   M   C B   D
//   D    N     T E   D     C   D
//   _    _     O L   _         R
//   X    O     _     T         _
//   T    V     R     Y         C
//   Y    R     S     P         Y
//   P    D           E         C
//	 E
#define DFC_CMD_OFFSET_LEN_OVRD  28
#define DFC_CMD_OFFSET_AUTO_RS   25
#define DFC_CMD_OFFSET_CSEL      24
#define DFC_CMD_OFFSET_CMD_TYPE  21
#define DFC_CMD_OFFSET_NC        20
#define DFC_CMD_OFFSET_DBC       19
#define DFC_CMD_OFFSET_ADDR_CYC  16
#define DFC_CMD_OFFSET_CMD2       8
#define DFC_CMD_OFFSET_CMD1       0

#define DFC_CMD_LEN_OVRD    0x10000000

// To set the AUTO_RS bit, the flash's read status command must be equal to this.
#define DFC_AUTO_RS_CMD 0x70

//Command XTYPES for read type operations
#define DFC_CMDXT_MONOLITHIC_READ					0x0
#define DFC_CMDXT_LAST_NAKED_READ					0x1
#define	DFC_CMDXT_READ								0x4		// FIXME: does this include a dispatch phase?
#define DFC_CMDXT_NAKED_READ 						0x5

//Command XTYPES for write type operations
#define DFC_CMDXT_MONOLITHIC_WRITE					0x0
#define DFC_CMDXT_NAKED_WRITE_WITH_FINAL_COMMAND	0x1
#define	DFC_CMDXT_FINAL_COMMAND						0x3		// FIXME: is there a read equivalent xtype?
#define DFC_CMDXT_COMMAND_DISPATCH_WITH_WRITE		0x4
#define DFC_CMDXT_NAKED_WRITE 						0x5

//Command XTYPES for all types of operations
#define DFC_CMDXT_COMMAND_DISPATCH					0x6


struct NDCB0_DEF{
	int CMD1 			: 8;
	int CMD2			: 8;
	int ADDR_CYC		: 3;
	unsigned DBC		: 1;
	unsigned NC			: 1;
	int CMD_TYPE		: 3;
	unsigned CSEL		: 1;
	unsigned AUTO_RS	: 1;
	unsigned ST_ROW_EN  : 1;
	unsigned RDY_BYP	: 1;
	unsigned LEN_OVRD   : 1;
	int CMD_XTYPE		: 3;
};

typedef union {
	unsigned int value;
	struct NDCB0_DEF bits;
}NDCB0_REG;




//
// DFC register offsets for each control point in command buffer 1.
//
//  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
// +---------------+---------------+---------------+---------------+
// |               |               |               |               |
// +---------------+---------------+---------------+---------------+
//      ADDR4           ADDR3           ADDR2           ADDR1 
//
#define DFC_CMD_OFFSET_ADDR4  24
#define DFC_CMD_OFFSET_ADDR3  16
#define DFC_CMD_OFFSET_ADDR2   8
#define DFC_CMD_OFFSET_ADDR1   0

struct NDCB1_DEF {
	int ADDR1		: 8;
	int ADDR2		: 8;
	int ADDR3		: 8;
	int ADDR4		: 8;
};

typedef union {
	unsigned int value;
	struct NDCB1_DEF bits;
}NDCB1_REG;



//
// DFC register offsets for each control point in command buffer 2.
//
//  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
// +-----------------------------------+-----------+---------------+
// |                                   |           |               |
// +-----------------------------------+-----------+---------------+
//               reserved                PAGE_COUNT     ADDR5    
//
#define DFC_CMD_OFFSET_PAGE_COUNT  8
#define DFC_CMD_OFFSET_ADDR5       0

struct NDCB2_DEF {
	int ADDR5		: 8;
	int PG_CNT		: 6;
	int reserved1	: 18;
};

typedef union {
	unsigned int value;
	struct NDCB2_DEF bits;
}NDCB2_REG;

//
// DFC register offsets for each control point in command buffer 3.
//
//  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
// +---------------+---------------+-------------------------------+
// |               |               |                               |
// +---------------+---------------+-------------------------------+
//       ADDR7           ADDR6                NDLENCNT    
//

#define DFC_CMD_OFFSET_NDLENCNT     0
#define DFC_CMD_OFFSET_ADDR6       16
#define DFC_CMD_OFFSET_ADDR7       24

struct NDCB3_DEF {
	int NDLENCNT    : 16;
	int ADDR6		:  8;
	int ADDR7		:  8;
};

typedef union {
	unsigned int value;
	struct NDCB3_DEF bits;
}NDCB3_REG;


typedef enum {
   DFC_CMDTYPE_READ,    // b000
   DFC_CMDTYPE_WRITE,   // b001
   DFC_CMDTYPE_ERASE,   // b010
   DFC_CMDTYPE_READID,  // b011
   DFC_CMDTYPE_STATUS,  // b100
   DFC_CMDTYPE_RESET,   // b101
   DFC_CMDTYPE_NAKED	// b110
   } DFC_CMD_TYPE;

/*
 *  Pre-set structures for commands
 * 			Since the boot ROM only uses a few commands they 
 *			can be kept generic.  If necessary commands could
 * 			be defined for a particular manufacture.
 *
 */

typedef struct {
	NDCB0_REG read;
	NDCB0_REG read_id;
	NDCB0_REG read_status;
	NDCB0_REG pg_program;	
	NDCB0_REG blk_erase;
	NDCB0_REG reset;
	NDCB0_REG read_onfi_parameter_pages;
}CMD_BLOCK;

//
// DFC status register masks for each status bit.
//
//  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
// +---------------------+---------+-------+-+-+-+-+-+-+-+-+-+-+-+-+
// |                     |         |       | | | | | | | | | | | | |
// +---------------------+---------+-------+-+-+-+-+-+-+-+-+-+-+-+-+
//         reserved          ECC            R C C C C C C D S W R W
//                          Count           D S S S S S S B B R D R
//                                          Y 0 1 0 1 0 1 E E D D C
//                                            _ _ _ _ _ _ R R R R M
//                                            P P C C B B R R E E D
//                                            A A M M B B     Q Q R
//                                            G G D D D D         E
//                                            E E D D             Q
//                                            D D                  
//
#define DFC_SR_ECC_CNT    0x001F0000
#define DFC_SR_RDY0       0x00000800
#define DFC_SR_RDY1       0x00001000
#define DFC_SR_CS0_PAGED  0x00000400
#define DFC_SR_CS1_PAGED  0x00000200
#define DFC_SR_CS0_CMDD   0x00000100
#define DFC_SR_CS1_CMDD   0x00000080
#define DFC_SR_CS0_BBD    0x00000040
#define DFC_SR_CS1_BBD    0x00000020
#define DFC_SR_DBERR      0x00000010
#define DFC_SR_SBERR      0x00000008
#define DFC_SR_WRDREQ     0x00000004
#define DFC_SR_RDDREQ     0x00000002
#define DFC_SR_WRCMDREQ   0x00000001

struct NDSR_DEF{
	unsigned  WRCMDREQ 	:1;
	unsigned  RDDREQ 	:1;
	unsigned  WRDREQ	:1;
	unsigned  SBERR		:1;
	unsigned  DBERR		:1;
	unsigned  CS1_BBD	:1;
	unsigned  CS0_BBD	:1;
	unsigned  CS1_CMDD	:1;
	unsigned  CS0_CMDD	:1;
	unsigned  CS1_PAGED	:1;
	unsigned  CS0_PAGED	:1;
	unsigned  RDY1		:1;
	unsigned  RDY0		:1;
	int		  Reserved1	:2;
	unsigned  TRUSTVIO	:1;
	int		  ERR_CNT	:5;
	int		  Reserved2 :11; 
};

typedef union{ 
	unsigned int value;
	struct NDSR_DEF bits;
}NDSR_REG;

//
// DFC ECC CTRL register masks for each status bit.
//
//  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
// +---------------------------------+---------------+-----------+-+
// |                                 |               |           | |
// +---------------------------------+---------------+-----------+-+
//                 reserved               ECC_SPARE    ECC_THRESH B
//                                                                C
//                                                                H
//                                                                _
//                                                                E
//                                                                N
//                                                     
//                                                        
//                                                              
//


struct NDECCCTRL_DEF{
 	unsigned  BCH_EN		:1;
	int		  ECC_THRESH	:6;
	int		  ECC_SPARE		:8;
	int		  Reserved1 	:17; 
};

typedef union{ 
	unsigned int value;
	struct NDECCCTRL_DEF bits;
}NDECCCTRL_REG;

//
// Data flash controller register locations.
//
#define DFC_TIMING_0  ((volatile int *)NFU_DTR0CS0)  	// NDTR0CS0 : Timing reg 0.
#define DFC_TIMING_1  ((volatile int *)NFU_DTR1CS0)		// NDTR1CS0 : Timing reg 1.
#define DFC_CONTROL   ((volatile int *)NFU_DCR)			// NDCR     : Control.
#define DFC_STATUS    ((volatile int *)NFU_DSR)  		// NDSR     : Status.
#define DFC_PAGES     ((volatile int *)NFU_DPCR)  		// NDPCR    : Page count.
#define DFC_BADBLOCK0 ((volatile int *)NFU_DBBRX)  		// NDBDR0   : Bad block 0.
#define DFC_BADBLOCK1 ((volatile int *)(NFU_DBBRX + 4)) // NDBDR1   : Bad block 1.
#define DFC_DREDEL 	  ((volatile int *)NFU_DREDEL)  	// DREDEL 	 : Read Enable Return Delay 
#define DFC_DATA      ((volatile int *)NFU_DDB)  		// NDDB     : Data buffer.
#define DFC_COMMAND0  ((volatile int *)NFU_DCB0)  		// NDCB0    : Command buffer.
#define DFC_COMMAND1  ((volatile int *)NDCB1)  			// NDCB1    : Command buffer.
#define DFC_COMMAND2  ((volatile int *)NDCB1)		  	// NDCB2    : Command buffer.
#define DFC_ECCCTRL	  ( (volatile int *)NFU_DECCCTRL)  	// NDECCCTRL : ECC Control 	
#endif
