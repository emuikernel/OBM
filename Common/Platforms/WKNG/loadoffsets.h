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
/******************************************************************************
 *
 *	loadoffsets.h
 *
 *	This file contains all of the processor specific memory load addresses
 *	 used by the boot ROM or OBM code
 *
 ******************************************************************************/

#ifndef _LOADOFFSETS_H_
#define _LOADOFFSETS_H_

/* BootROM ISRAM Memory Map
0xD1000000 : ISRAM Start
0xD1000040 : Run Time Error Collection
0xD1000100 : Data + BSS, Transfer Struct (grows up)
0xD100A000 : Stack (grows down)
0xD100A000 : Initial TIM Load/Download Area	(Image Load Base)
*/

// ISRAM
#define ISRAM_PHY_ADDR						0xD1100000
#define ISRAM_IMAGE_LOAD_BASE				0xD1100000
#define PLATFORMISRAMLIMIT					0x7000     		// 28KB (0xD110_0000 - 0xD110_7000)
#define FLASH_STREAM_SIZE					0x7000 			// streaming flash data limit
#define FLASH_STREAM_ADDRESS				0xD1100000		// ISRAM load address for streaming flash

//  Download address for images and data
#define CS0Base								0x80000000
#define CS1Base								0x90000000
#define CS2Base								0xA0000000
#define CS3Base								0xB0000000

#define DDR_PHY_ADDR						0x00000000

/**************************************************************************/
/* mDOC device physical address window location                           */
/**************************************************************************/
#define MDOC_START_ADDRESS	CS0Base   /* Used for P2Sample platform */


//////////////////////////////////////////////////////////////////////
// 					BootLoader Memory Map							//
//////////////////////////////////////////////////////////////////////
// This map essentially lays out all the reserved portions of the DDR
// that are utilized by the BootLoader.
//////////////////////////////////////////////////////////////////////
/**
DDR layout update:

1.       0x00000000 ~ 0x00000024, for interrupt vectors

2.       0x00001000 ~ 0x01C00000, for download and upload regions

3.       0x01C00000 ~ 0x01C20000, 128KB for OBM code, data, bss..

4.       0x01C20000 ~ 0x01C28000, 32KB for stack

5.       0x01C28000 ~ 0x01C29000, 4KB for temp buffer used by erasing block0

6.       0x01C29000 ~ 0x01C2D000, 16KB for transfer struct

7.       0x01C2D000 ~ 0x01C2D400, 1KB for error code

8.	     0x01C2D400 ~ 0x01C31400, 16KB for transfer control

9.       0x01C31400 ~ 0x01C33400, 8KB for FM_SPACE

10. 	 0x01C33400 ~ 0x01C37400, 8KB for CI2 USB rx, 8KB for CI2 USB tx

11.		 0x01C37400 ~ 0x01C37404, force USB enumeration

12.		 0x01C2D000 ~ 0x01C2D400, reuse for TR069

13.		 0x01C60000 ~ 0x01C70000, 64KB for SPI DMA buffer

14.		 0x01C70000 ~ 0x01C80000, 64KB for QP buffer

15.		 0x01C60000 ~ 0x01C62000, 8KB for DisturbList, re-use SPI DMA buffer

**/

#define FORCE_USB_ENUM_ADDR				0x01C37400

#if PRODUCTION_MODE_SUPPORT
#if MIFI_V2R1 || MIFI_V2R0
#define PRODUCTION_MODE_FLAG_ADDR		0x07D7F004 // usbmode
#else
#define PRODUCTION_MODE_FLAG_ADDR		0x07D7F000 // Daseul
#endif
#endif

#if MIFI_V3R1 || MIFI_V3R2 || MIFI_V3R3
#define ATCMD_FLAG_ADDR					0x07D7F008
#define ATCMD_FLAG_VALID_VALUE			0xaabbccdd

#else
#define ATCMD_FLGA_ADDR					0x01EE0000
#define ATCMD_FLAG_VALID_VALUE			0xaabbccdd
#endif

#define OBM_VERSION_TO_CP_ADDR			0x07D7F00C
#define LTG_LWG_FLAG_ADDR				0x07D7F010
#define MRD_FLASH_ADDRESS				0x07D7F014
#define RESET_FLAG_ADDRESS				0x07D7F018

#define FIRST_MRD_FLASH_ADDRESS			0x07D7F024
#define SECOND_MRD_FLASH_ADDRESS		0x07D7F028

#if SILENT_RESET
#error "123321"
#define SILENT_RESET0_ADDRESS			0x07D7F02C
#define SILENT_RESET0_MAGIC				0x5A5AA5A5
#define SILENT_RESET1_ADDRESS			0x07D7F030
#define SILENT_RESET1_MAGIC				0xA5A55A5A
#endif

#define TR069_FLAG_ADDR					0x07D7F03C
#define BACKUP_IMAGES_ADDR				0x07D7F040
#if ZIMI_PB05
#define ZIMI_ONKEY_LONG_PRESS_FLAG_ADDR		0x07D7F100    //xyl define   caution here, if this address is used by marvell in newer version, we must change this addr
#define ZIMI_OBM_VERSION_FLAG_ADDR			0x07D7F104	//xyl define -----
#define ZIMI_BATVOL_INIT_FLAG_ADDR			0x07D7F108	//xyl define pass init bat vol
#define ZIMI_BQ_HARDWARE_VER					0x07D7F10C	//xyl define bq hardare version
#endif

#define DDR_DOWNLOAD_AREA_ADDR 			0x00001000
#define DDR_TEMP_ERASE_BLOCK0_ADDR		0x01C28000  // 4KB temporary space for erasing block0
#define DDR_TRANSFER_STRUCT_ADDR		0x01C29000	// 16KB temporary space for the TS.
#define DDR_ERROR_CODE_ADDR				0x01C2D000	// We only need 4 bytes for the Error Code. 
#define DDR_TEMP_ADDR					0x01C2D000
#define DDR_FM_SPACE_ADDR				0x01C31400  // FM_SPACE
#define DDR_CI2_USB_DDR_BUF_ADDR		0x01C33400  // CI2_USB_DDR_BUF

#define DDR_SPI_DMA_BUFFER_ADDR			0x01C60000
#define DDR_QPRESS_DDR					0x01C70000

#define DDR_DISTURBLIST_DDR				0x01C60000

#define OBM1_TRANSFER_STRUCT_ADDR		0xd110a000  // only for wukong y0

// IMPORTANT:
// #define DDR_BL_TRANSFER_CODE_ADDR		0x00150000	// 64 KB temporary space for the BL Transfer Code.
// Even though it is commented out here, this address is assigned in
// Platform_defs.inc and should not be overwritten by anything else. 
// In case we want to modify it, it needs to be changed in Platform_defs.inc

#endif

