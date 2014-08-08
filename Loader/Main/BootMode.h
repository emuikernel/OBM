
/******************************************************************************
 *
 *  (C)Copyright 2011 Marvell. All Rights Reserved.
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
 *
 *  FILENAME:	BootMode.h
 *
 *  PURPOSE: 	Contains BootLoader's BootMode definitions
 *
******************************************************************************/

#ifndef __bootmode_h
#define __bootmode_h

//////////////////////////////////////////////////////////////////////
// Library Support
//////////////////////////////////////////////////////////////////////
#include "tim.h"					// Library support for TIM structures/functions.
#include "Errors.h"					// Library support for defined Errors.
#include "Flash.h"					// Library support for Flash structures/functions, such as ConfigureFlashes().
#include "FM.h"						// Library support for FlashManagement structures/functions, such as InitializeFM().
#include "Typedef.h"				// Library support for typedef's.

#if TR069_SUPPORT
#include "tr069.h"
#endif

//////////////////////////////////////////////////////////////////////
// Function Prototypes
//////////////////////////////////////////////////////////////////////
pIMAGE_INFO_3_4_0 BootModeMain( pTIM pTIM_h, OPERATING_MODE_T BootMode, pFUSE_SET pFuses );
#endif