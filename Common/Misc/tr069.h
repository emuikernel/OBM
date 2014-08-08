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

#ifndef __TR069_H__
#define __TR069_H__

#include "Typedef.h"
#include "Errors.h"
#include "Flash.h"
#include "loadoffsets.h"
#include "downloader.h"
#include "TIMDownload.h"

#if LED_DISPLAY
#include "led.h"
#endif

#if OLED_SUPPORT
#include "oled.h"
#endif

#if DECOMPRESS_SUPPORT
#include "LzmaDecode.h"
#endif

#define FBF_HEADER_FLASH_ADDRESS	0x06b60000
#define FBF_HEADER_IDENTIFIER		0x54524657 // "TRFW"


typedef struct
{
	UINT_T Header;
	UINT_T Upgrade_Flag; // 1, upgrade; 2, backup boot
	UINT_T FBF_Flash_Address;
	UINT_T FBF_Size;
	UINT_T Erase_PSM;
	UINT_T PSM_Erase_Address;
	UINT_T PSM_Erase_Size;
	UINT_T Erase_FS;
	UINT_T FS_Erase_Address;
	UINT_T FS_Erase_Size;
	UINT_T Upgrade_Method; // 1, TR069; 2, SD; 3, WebUI.
	UINT_T Webdata_LZMA2;
	UINT_T Webdata_Address;
	UINT_T Webdata_Size;
	UINT_T WIFI_NoCal_LZMA2;
	UINT_T WIFI_NoCal_Address;
	UINT_T WIFI_NoCal_Size;
	UINT_T WIFI_LAMA2;
	UINT_T WIFI_Address;
	UINT_T WIFI_Size;
	UINT8_T Version[64];
} TR069_Firmware, *P_TR069_Firmware;

#define MAX_IMAGES_NUMBER	0x100

typedef struct
{
	UINT_T Image_ID;
	UINT_T Image_Size;
	UINT_T Image_Address;
}LZMA2_Image_t, *P_LZMA2_Image_t;

typedef struct
{
	LZMA2_Image_t LZMA2_List[MAX_IMAGES_NUMBER];
}LZMA2_Image_List, *P_LZMA2_Image_List;

UINT_T firmware_upgrade(pTIM pTIM_h);

#endif

