/******************************************************************************
 *
 *  (C)Copyright 2013 Marvell Hefei Branch. All Rights Reserved.
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

#include "Typedef.h"
#include "APBC.h"
#include "TMR.h"
#include "PMUM.h"
#include "predefines.h"
#include "platform_interrupts.h"

#ifndef __WDT_H__
#define __WDT_H__

#define	WDT_BASE	0xD4080000

#define	WDT_WMER	(WDT_BASE+0x0064)
#define	WDT_WMR		(WDT_BASE+0x0068)
#define	WDT_WICR	(WDT_BASE+0x0080)
#define	WDT_CER		(WDT_BASE+0x0084)
#define	WDT_WCR		(WDT_BASE+0x0098)
#define	WDT_WFAR	(WDT_BASE+0x009C)
#define	WDT_WSAR	(WDT_BASE+0x00A0)
#endif
