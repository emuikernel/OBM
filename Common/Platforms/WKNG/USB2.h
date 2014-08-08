/******************************************************************************
 *
 * Name:	USB2.h
 * Project:	WUKONG  ( CP828 )
 * Purpose:
 *
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
 *
 ******************************************************************************/

/******************************************************************************
 *
 * History:
 *
 ********* PLEASE INSERT THE CVS HISTORY OF THE PREVIOUS VERSION HERE. *********
 *******************************************************************************/

#ifndef	__INC_USB2_H
#define	__INC_USB2_H


/*
 *
 *	THE BASE ADDRESSES
 *
 */
#define	USB2_BASE	0xD4207000

/*
 *
 *	THE 16-bit WUKONG REGISTER DEFINES
 *
 */
#define USB2_PLL_CTRL_REG0	(USB2_BASE+0x04)
#define USB2_PLL_CTRL_REG1	(USB2_BASE+0x08)
#define USB2_TX_CTRL_REG0	(USB2_BASE+0x10)
#define USB2_TX_CTRL_REG1	(USB2_BASE+0x14)
#define USB2_TX_CTRL_REG2	(USB2_BASE+0x18)
#define USB2_RX_CTRL_REG0	(USB2_BASE+0x20)
#define USB2_RX_CTRL_REG1	(USB2_BASE+0x24)
#define USB2_RX_CTRL_REG2	(USB2_BASE+0x28)
#define USB2_ANA_REG0		(USB2_BASE+0x30)
#define USB2_ANA_REG1		(USB2_BASE+0x34)
#define USB2_ANA_REG2		(USB2_BASE+0x38)
#define USB2_DIG_REG0		(USB2_BASE+0x3C)
#define USB2_DIG_REG1		(USB2_BASE+0x40)
#define USB2_DIG_REG2		(USB2_BASE+0x44)
#define USB2_DIG_REG3		(USB2_BASE+0x48)
#define USB2_TEST_REG0		(USB2_BASE+0x4C)
#define USB2_TEST_REG1		(USB2_BASE+0x50)
#define USB2_TEST_REG2		(USB2_BASE+0x54)
#define USB2_CHARGER_REG0	(USB2_BASE+0x58)
#define USB2_OTG_REG0		(USB2_BASE+0x5C)
#define USB2_PHY_MON0		(USB2_BASE+0x60)
#define USB2_RESETVE_REG0	(USB2_BASE+0x64)

#define USB2_ICID_REG0		(USB2_BASE+0x78)
#define USB2_ICID_REG1		(USB2_BASE+0x7C)


/*
 *
 *	THE BIT DEFINES
 *
 */

/*		USB2_PLL_CTRL_REG0	0x04  	*/
#define USB2_PLL_FBDIV_BASE				0
#define USB2_PLL_FBDIV_MASK				SHIFT0(0x1FF)
#define USB2_PLL_REFDIV_BASE			9
#define USB2_PLL_REFDIV_MASK			SHIFT9(0x1F)
#define USB2_PLL_VDD18_BASE				14
#define USB2_PLL_VDD18_MASK				SHIFT14(0x3)

/*		USB2_PLL_CTRL_REG1	0x08  	*/
#define USB2_PLL_CAL12_BASE				0
#define USB2_PLL_CAL12_MASK				SHIFT0(0x3)
#define USB2_PLL_VCOCAL_START_BASE		2
#define USB2_PLL_VCOCAL_START_MASK      SHIFT2(0x1)
#define USB2_PLL_CLK_BLK_EN_BASE		3
#define USB2_PLL_CLK_BLK_EN_MASK        SHIFT3(0x1)
#define USB2_PLL_KVCO_BASE				4
#define USB2_PLL_KVCO_MASK				SHIFT4(0x7)
#define USB2_PLL_KVCO_EXT_BASE			7
#define USB2_PLL_KVCO_EXT_MASK          SHIFT7(0x1)
#define USB2_PLL_ICP_BASE				8
#define USB2_PLL_ICP_MASK				SHIFT8(0x7)
#define USB2_PLL_LOCKDET_ISEL_BASE		11
#define USB2_PLL_LOCKDET_ISEL_MASK		SHIFT11(0x1)
#define USB2_PLL_LOCK_BYPASS_BASE		12
#define USB2_PLL_LOCK_BYPASS_MASK		SHIFT12(0x1)
#define USB2_PLL_PU_PLL_BASE			13
#define USB2_PLL_PU_PLL_MASK			SHIFT13(0x1)
#define USB2_PLL_CONTROL_BY_PIN_BASE	14
#define USB2_PLL_CONTROL_BY_PIN_MASK	SHIFT14(0x1)
#define USB2_PLL_READY_BASE				15
#define USB2_PLL_READY_MASK				SHIFT15(0x1)

/*		USB2_TX_CTRL_REG0	0x10  	*/
#define USB2_TX_EXT_FS_RCAL_BASE		0
#define USB2_TX_EXT_FS_RCAL_MASK		SHIFT0(0xF)
#define USB2_TX_HS_RCAL_BASE			4
#define USB2_TX_HS_RCAL_MASK			SHIFT4(0xF)
#define USB2_TX_IMPCAL_VTH_BASE			8
#define USB2_TX_IMPCAL_VTH_MASK			SHIFT8(0x7)
#define USB2_TX_EXT_FS_RCAL_EN_BASE		11
#define USB2_TX_EXT_FS_RCAL_EN_MASK		SHIFT11(0x1)
#define USB2_TX_EXT_HS_RCAL_EN_BASE		12
#define USB2_TX_EXT_HS_RCAL_EN_MASK		SHIFT12(0x1)
#define USB2_TX_RCAL_START_BASE			13
#define USB2_TX_RCAL_START_MASK			SHIFT13(0x1)
#define USB2_TX_DATA_BLOCK_EN_BASE		14
#define USB2_TX_DATA_BLOCK_EN_MASK		SHIFT14(0x1)

/*		USB2_TX_CTRL_REG1	0x14	*/
#define USB2_TX_CK60_PHSEL_BASE			0
#define USB2_TX_CK60_PHSEL_MASK			SHIFT0(0xF)
#define USB2_TX_AMP_BASE				4
#define USB2_TX_AMP_MASK				SHIFT4(0x7)
#define USB2_TX_LOW_VDD_EN_BASE			7
#define USB2_TX_LOWVDD_MASK				SHIFT7(0x1)
#define USB2_TX_VDD12_BASE				8
#define USB2_TX_VDD12_MASK				SHIFT8(0x3)
#define USB2_TX_VDD15_BASE				10
#define USB2_TX_VDD15_MASK				SHIFT10(0x3)

/*		USB2_TX_CTRL_REG2	0x18 	*/
#define USB2_TX_DRV_SLEWRATE_BASE       10
#define USB2_TX_DRV_SLEWRATE_MASK       SHIFT10(0x3)

/*		USB2_RX_CTRL_REG0	0x20  	*/
#define USB2_RX_INTP_BASE				0
#define USB2_RX_INTP_MASK				SHIFT0(0x3)
#define USB2_RX_LPF_COEF_BASE			2
#define USB2_RX_LPF_COEF_MAKS			SHIFT2(0x3)
#define USB2_RX_SQ_THRESH_BASE			4
#define USB2_RX_SQ_THRESH_MASK			SHIFT4(0xF)
#define USB2_RX_DISCON_THRESH_BASE		8
#define USB2_RX_DISCON_THRESH_MASK		SHIFT8(0x3)
#define USB2_RX_SQ_LENGTH_BASE			10
#define USB2_RX_SQ_LENGTH_MASK			SHIFT10(0x3)
#define USB2_RX_ACQ_LENGTH_BASE			12
#define USB2_RX_ACQ_LENGTH_MASK			SHIFT12(0x3)
#define USB2_RX_USQ_LENGTH_BASE			14									   
#define USB2_RX_USQ_LENGTH_MASK			SHIFT14(0x1)
#define USB2_RX_PHASE_FREEZE_DLY_BASE	15									   
#define USB2_RX_PHASE_FREEZE_DLY_MASK	SHIFT15(0x1)

/*		USB2_RX_CTRL_REG1	0x24	*/
#define USB2_RX_S2T03_DLY_SEL_BASE		0
#define USB2_RX_S2T03_DLY_SEL_MASK		SHIFT0(0x3)
#define USB2_RX_CDR_FASTLOCK_EN_BASE	2
#define USB2_RX_CDR_FASTLOCK_EN_MASK	SHIFT2(0x1)
#define USB2_RX_CDR_COEF_SEL_BASE		3
#define USB2_RX_CDR_COEF_SEL_MASK		SHIFT3(0x1)
#define USB2_RX_EDGE_DET_SEL_BASE		4
#define USB2_RX_EDGE_DET_SEL_MASK		SHIFT4(0x3)
#define USB2_RX_DATA_BLOCK_LENGTH_BASE	6
#define USB2_RX_DATA_BLOCK_LENGTH_MASK	SHIFT6(0x3)
#define USB2_RX_CAP_SEL_BASE			8
#define USB2_RX_CAP_SEL_MASK			SHIFT8(0x7)
#define USB2_RX_EDGE_DET_EN_BASE		11
#define USB2_RX_EDGE_DET_EN_MASK		SHIFT11(0x1)
#define USB2_RX_DATA_BLOCK_EN_BASE		12
#define USB2_RX_DATA_BLOCK_EN_MASK		SHIFT12(0x1)
#define USB2_RX_EARLY_VOS_ON_EN_BASE	13
#define USB2_RX_EARLY_VOS_ON_EN_MASK	SHIFT13(0x1)

/*		USB2_RX_CTRL_REG2	0x28 	*/
#define USB2_RX_VDD12_BASE				0
#define USB2_RX_VDD12_MASK				SHIFT0(0x3)
#define USB2_RX_VDD18_BASE				2
#define USB2_RX_VDD18_MASK				SHIFT2(0x3)
#define USB2_RX_SQ_ALWAYS_ON_BASE		4
#define USB2_RX_SQ_ALWAYS_ON_MASK		SHIFT4(0x1)
#define USB2_RX_SQ_BUFFER_EN_BASE		5
#define USB2_RX_SQ_BUFFER_EN_MASK		SHIFT5(0x1)
#define USB2_RX_SAMPLER_CTRL_BASE		6
#define USB2_RX_SAMPLER_CTRL_MASK		SHIFT6(0x1)
#define USB2_RX_SQ_CM_BASE				7
#define USB2_RX_SQ_CM_MASK				SHIFT7(0x1)
#define USB2_RX_USQ_FILTER_BASE			8
#define USB2_RX_USQ_FILTER_MASK			SHIFT8(0x1)

/*		USB2_ANA					*/
/*		USB2_ANA_REG0	0x30  		*/
#define USB2_ANA_IPTAT_SEL_BASE			0
#define USB2_ANA_VDD_DIG_TOP_SEL_BASE	3
#define USB2_ANA_TOPVDD18_BASE			4
#define USB2_ANA_DIG_SEL_BASE			6
#define USB2_ANA_BG_VSEL_BASE			8

/*		USB2_ANA_REG1	0x34  		*/
#define USB2_ANA_PU_BASE				14

/*		USB2_OTG_REG0		0x5C 	*/
#define USB2_OTG_PU_BASE				3

/*		USB2_TEST_REG0		0x4C 	*/
#define USB2_TEST_PATTERN_BASE			0
#define USB2_TEST_MODE_BASE				8
#define USB2_TEST_BYPASS_BASE			11
#define USB2_TEST_LENGTH_BASE			12
#define USB2_TEST_LPBK_EN_BASE			14
#define USB2_TEST_DIG_LPBK_BASE			15
/*		USB2_TEST_REG1		0x50 	*/
#define USB2_TEST_XCVR_SELECT_BASE		0
#define USB2_TEST_OP_MODE_BASE			2
#define USB2_TEST_TERM_SELECT_BASE		4
#define USB2_TEST_TX_BITSTUFF_EN_BASE	5
#define USB2_TEST_SUSPENDM_BASE			6
#define USB2_TEST_UTMI_SEL_BASE			7
#define USB2_TEST_SKIP_BASE				8
#define USB2_TEST_RESET_BASE			12
#define USB2_TEST_EN_BASE				13
#define USB2_TEST_FLAG_BASE				14
#define USB2_TEST_DONE_BASE				15

/*		USB2_RESERVE_REG0		0x64 	*/
/* [11:0] Reserved */
#define USB2_PLL_VDD12_BASE				12
#define USB2_PLL_VDD12_MASK				SHIFT12(0x3)
#define USB2_DP_DM_SWAP_CTRL_BASE		15
#define USB2_DP_DM_SWAP_CTRL_MASK		SHIFT15(0x1)
/* -------------------- */


#endif	/* __INC_USB2_H */
