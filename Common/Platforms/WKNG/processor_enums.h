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

#ifndef __PROCESSOR_ENUMS_H__
#define __PROCESSOR_ENUMS_H__

typedef enum TTC_CLOCK_ID_E
{
	TTC_CLOCK_ID_MCU,
	TTC_CLOCK_ID_I2C
} TTC_CLOCK_ID_T;

typedef enum TTC_FREQ_ID_E
{
	TTC_FREQ_ID_MCU
} TTC_FREQ_ID_T;

typedef enum TTC_MCU_REGID_E
{
	TTC_SDRREVREG_ID,		// revision
	TTC_SDRADCREG_ID,		// address decode
	TTC_SDRCFGREG0_ID,		// sdram config reg 0
	TTC_SDRCFGREG1_ID,		// sdram config reg 1
	TTC_SDRCFGREG2_ID,		// sdram config reg 2
	TTC_SDRTMGREG1_ID,		// sdram timing reg 1
	TTC_SDRTMGREG2_ID,		// sdram timing reg 2
	TTC_SDRTMGREG3_ID,		// sdram timing reg 3
	TTC_SDRTMGREG4_ID,		// sdram timing reg 4
	TTC_SDRTMGREG5_ID,		// sdram timing reg 5
	TTC_SDRCTLREG1_ID,		// sdram control reg 1
	TTC_SDRCTLREG2_ID,		// sdram control reg 2
	TTC_SDRCTLREG3_ID,		// sdram control reg 3
	TTC_SDRCTLREG4_ID,		// sdram control reg 4
	TTC_SDRCTLREG5_ID,		// sdram control reg 5
	TTC_SDRCTLREG14_ID,		// sdram control reg 14
	TTC_SDRPADREG_ID,		// sdram pad calibration reg
	TTC_ADRMAPREG0_ID,		// address map cs0
	TTC_ADRMAPREG1_ID,		// address map cs1
	TTC_ADRMAPREG2_ID,		// address map cs2
	TTC_USRCMDREG0_ID,		// user initiated command registers
	TTC_SDRSTAREG_ID,		// sdram status register
	TTC_PHYCTLREG3_ID,		// phy control reg 3
	TTC_PHYCTLREG7_ID,		// phy control reg 7
	TTC_PHYCTLREG8_ID,		// phy control reg 8
	TTC_PHYCTLREG9_ID,		// phy control reg 9
	TTC_PHYCTLREG10_ID,		// phy control reg 10
	TTC_PHYCTLREG13_ID,		// phy control reg 13
	TTC_PHYCTLREG14_ID,		// phy control reg 14
	TTC_DLLCTLREG1_ID,		// dll control reg 1
	TTC_DLLCTLREG2_ID,		// dll control reg 2
	TTC_DLLCTLREG3_ID,		// dll control reg 3
	TTC_MCBCTLREG1_ID,		// mcb control reg 1
	TTC_MCBCTLREG2_ID,		// mcb control reg 2
	TTC_MCBCTLREG3_ID,		// mcb control reg 3
	TTC_MCBCTLREG4_ID,		// mcb control reg 4
	TTC_OPDELAY_ID,			// operation delay reg
	TTC_OPREAD_ID			// operation read reg
} TTC_MCU_REGID_T;

#endif

