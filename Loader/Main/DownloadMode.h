
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
 *  FILENAME:	DownloadMode.h
 *
 *  PURPOSE: 	Contains BootLoader's DownloadMode definitions
 *
******************************************************************************/

#ifndef __downloadmode_h
#define __downloadmode_h
																				
//////////////////////////////////////////////////////////////////////
// Library Support
//////////////////////////////////////////////////////////////////////
#include "tim.h"					// Library support for TIM structures/functions.
#include "Errors.h"					// Library support for defined Errors.
#include "TIMDownload.h"			// Library support for TIMDownload functions, such as TIMDownloadMain().
#include "BootMode.h"				// Library support for BootMode functions, such as BootModeMain().
#include "Typedef.h"				// Library support for typedef's.
#include "ProtocolManager.h"

//////////////////////////////////////////////////////////////////////
// Function Prototypes
//////////////////////////////////////////////////////////////////////
pIMAGE_INFO_3_4_0 DownloadModeMain( pFUSE_SET fuses, pTIM pTIM_h, OPERATING_MODE_T BootMode);
pIMAGE_INFO_3_4_0 DetermineModeAndDownload( pFUSE_SET fuses, pTIM pTIM_h);

#endif