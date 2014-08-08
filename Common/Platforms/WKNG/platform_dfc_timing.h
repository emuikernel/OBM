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
 
 ******************************************************************************
**
**  FILENAME:	PlatformConfig.h
**
**  PURPOSE: 	Platform specific header to set parameters
** 			
**                  
******************************************************************************/

//#include "dfc_defs.h"
#include "xllp_dfc_defs.h"

/*************** Flash Timing Override Examples ************************
* By default, very conservative timings are programmed - ie. BootROM
* These timings may be optimized.
* Below are some optimized flash timings for certain flash.
* Note that at it is impossible at the time of creation to cover all possible parts.
* Simply create a new structure for the desired part and call xdfc_setTiming in xllpdfcinit
*/

FLASH_TIMING SAMSUNG_MFG_TIMING[5] =
{
	{10, 0, 20, 40, 20, 40, 0, 60, 10},   		// Small Block Full Speed
	{20, 10, 40, 80, 40, 80, 10000, 120, 20},  	// Small Block Conservative
	{10, 35, 15, 25, 15, 25, 0, 60, 10}, 	  	// Large Block Full Speed
	{20, 70, 30, 50, 30, 50, 25000, 120, 20}, 	// Large Block Conservative
	{20, 10, 40, 80, 40, 80, 35090, 120, 13}  	// For JIL flash
};

// Micron MT29F1G08ABA, MT29F1G16ABA
FLASH_TIMING MICRON_MFG_TIMING[3] =
{
	{10, 25, 15, 25, 15, 25, 0, 60, 10},   		// Large Block Full Speed
	{20, 50, 30, 50, 30, 50, 25000, 120, 20},  	// Large Block Conservative
	{10, 25, 15, 17, 15, 17, 25000, 65, 15}     // Large Block Full Speed
//	{10, 25, 15, 17, 15, 17, 25000, 80, 10}    	// WAS -- Large Block Full Speed
};

FLASH_TIMING TOSHIBA_MFG_TIMING[1] =
{
	{5, 20, 10, 12, 10, 28, 120000, 60, 10}
};

FLASH_TIMING EMST_MFG_TIMING[1] =
{
	{10, 35, 15, 25, 15, 25, 25000, 60, 10}
};


