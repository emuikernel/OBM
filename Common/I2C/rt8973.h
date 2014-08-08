/******************************************************************************
 *
 *  (C)Copyright 2013 Marvell. All Rights Reserved.
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

 #ifndef __RT8973_H__
 #define __RT8973_H__

#include "Typedef.h"
#include "timer.h"
#include "USB1.h"

/************************************************************************************************/
/* 						The address of Richtek RT8973 micro USB switch device.									*/
/************************************************************************************************/

#define RT8973_REG_CHIP_ID         0x01
#define RT8973_REG_CONTROL_1       0x02
#define RT8973_REG_INT_FLAG        0x03
#define RT8973_ATTACH				(1 << 0)
#define RT8973_CONNECT 				(1 << 5)
#define RT8973_ADC_CHG		 		(1 << 6)

#define RT8973_REG_INT_FLAG2       0x04
#define RT8973_REG_INTERRUPT_MASK  0x05
#define RT8973_REG_INTERRUPT_MASK2 0x06
#define RT8973_REG_ADC             0x07
#define RT8973_REG_DEVICE_1        0x0A
#define RT8973_SDPORT				(1 << 2)
#define RT8973_REG_DEVICE_2        0x0B
#define RT8973_REG_MANUAL_SW1      0x13
#define RT8973_REG_MANUAL_SW2      0x14
#define RT8973_REG_RESET           0x1B


/*****************************************************************************/
void RT8973Write(UINT8_T reg_addr, UINT8_T value);
UINT8_T RT8973Read(UINT8_T reg_addr);


 #endif