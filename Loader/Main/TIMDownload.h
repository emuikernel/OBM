
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
 *  FILENAME:	TIMDownload.h
 *
 *  PURPOSE: 	Contains BootLoader's TIM based download definitions
 *
******************************************************************************/

#ifndef __timdownload_h
#define __timdownload_h

//////////////////////////////////////////////////////////////////////
// Library Support
//////////////////////////////////////////////////////////////////////
#include "tim.h"					// Library support for TIM structures/functions.
#include "Errors.h"					// Library support for defined Errors.
#include "Flash.h"					// Library support for Flash structures/functions, such as ConfigureFlashes().
#include "FM.h"						// Library support for FlashManagement structures/functions, such as InitializeFM().
#include "loadoffsets.h"			// Library support for the memory map addresses, such as DDR_DOWNLOAD_AREA_ADDR.
#include "Typedef.h"				// Library support for the defined types, such as FUNC_STATUS.
#include "downloader.h"
#include "BootLoader.h"

#define SPARE_AREA_SIZE(cmds) (cmds>>DLCMD_IMAGE_SPARE_AREA_SZ_BIT) & ((1<<DLCMD_IMAGE_SPARE_AREA_SZ_FIELD_BITS)-1)
#define IMAGE_TYPE(cmds) (cmds>>DLCMD_IMAGE_TYPE_FIELD_BIT) & ((1<<DLCMD_IMAGE_TYPE_FIELD_SIZE_BITS)-1)

//////////////////////////////////////////////////////////////////////
// Function Prototypes
//////////////////////////////////////////////////////////////////////
pIMAGE_INFO_3_4_0 TIMDownloadMain( pTIM pTIM_h);
pIMAGE_INFO_3_4_0 DownloadTIMImages (pTIM pTIM_h);

#endif
