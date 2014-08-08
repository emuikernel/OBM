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

#ifndef __platform_setup_h
#define __platform_setup_h

/*
 *  Slave Address definitions for I2C devices
 */

#if (ZYLONITE)
#define GPIO_EX_I2C_WRITE_SLAVE_ADDRESS          0xEA
#define GPIO_EX_I2C_READ_SLAVE_ADDRESS           0xEB
#endif
#if (LITTLETON)
#define GPIO_EX_I2C_WRITE_SLAVE_ADDRESS          0xA0
#define GPIO_EX_I2C_READ_SLAVE_ADDRESS           0xA1
#endif

/*
 * Prototypes
 */
UINT_T PlatformPreFlashSetup(void);
UINT_T PlatformPreCodeSetup(pTIM pTIM_h, P_FlashProperties_T pFlashProp);
UINT_T PlatformPostCodeSetup(pTIM pTIM_h, P_FlashProperties_T pFlashProp);
I2C_ReturnCode PMICSetup ( void );
I2C_ReturnCode ResetULPI ( void );
#endif



