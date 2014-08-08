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

#ifndef	__INC_SMEMC_H
#define	__INC_SMEMC_H

#define SMC_BASE 			0xD4283800

#define SMC_MSCx 			(SMC_BASE | 0x20)
#define SMC_SXCNFGx			(SMC_BASE | 0x30)
#define SMC_MEMCLKCFGx		(SMC_BASE | 0x68)
#define SMC_CSDFICFGx		(SMC_BASE | 0x90)
#define SMC_CSDFICFG2		(SMC_BASE | 0x98)
#define SMC_CLK_RET_DEL		(SMC_BASE | 0xB0)
#define SMC_ADV_RET_DEL		(SMC_BASE | 0xB4)
#define SMC_CSADRMAPx		(SMC_BASE | 0xC0)
#define SMC_CSADRMAP2		(SMC_BASE | 0xC8)
#define SMC_TCVCFGREF		(SMC_BASE | 0xD0)
#define SMC_SMC_WE_Apx		(SMC_BASE | 0xE0)
#define SMC_OE_Apx			(SMC_BASE | 0xF0)
#define SMC_ADV_Apx			(SMC_BASE | 0x100)

#endif

