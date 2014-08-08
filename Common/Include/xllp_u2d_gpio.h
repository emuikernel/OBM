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
**  FILENAME:   xllp_u2d_gpio.h
**
**  PURPOSE:    Hardware platform specific configuration defines / routines
**
******************************************************************************/
#ifndef __XLLP_U2D_GPIO_H_
#define __XLLP_U2D_GPIO_H_

#include "xllp_defs.h"


#define XLLP_MFPR_PULL_SEL     (0x1U << 15)
#define XLLP_MFPR_PULLUP_EN    (0x1U << 14)
#define XLLP_MFPR_PULLDOWN_EN  (0x1U << 13)
#define XLLP_MFPR_DRIVE_SHIFT  10
#define XLLP_MFPR_DRIVE_MASK   0x7U
#define XLLP_MFPR_SLEEP_SEL    (0x1U << 9)
#define XLLP_MFPR_SLEEP_DATA   (0x1U << 8)
#define XLLP_MFPR_SLEEP_OE_N   (0x1U << 7)
#define XLLP_MFPR_EDGE_CLEAR   (0x1U << 6)
#define XLLP_MFPR_EDGE_FALL_EN (0x1U << 5)
#define XLLP_MFPR_RISE_EN      (0x1U << 4)
#define XLLP_MFPR_RESV         (0x1U << 3)
#define XLLP_MFPR_AF_SEL_SHIFT 0
#define XLLP_MFPR_AF_SEL_MASK  0x7U
#define MON_MFPR_ALT_FN_0      0x0U
#define MON_MFPR_ALT_FN_1      0x1U
#define MON_MFPR_ALT_FN_2      0x2U
#define MON_MFPR_ALT_FN_3      0x3U
#define MON_MFPR_ALT_FN_4      0x4U
#define MON_MFPR_ALT_FN_5      0x5U
#define MON_MFPR_ALT_FN_6      0x6U
#define MON_MFPR_ALT_FN_7      0x7U

#define GPIO_DIR_IN  0
#define GPIO_DIR_OUT 1

//-----------------------------------------------------------
// Put the registers in the order you want them initialized
// Encode the GPIO # in the upper 7 bits of gpio_dir_encode
// and the direction in the LSB of gpio_dir_encode
//-----------------------------------------------------------
typedef struct MONS_U2D_GPIO_PAD_SETUP_S
{
    P_XLLP_VUINT32_T  registerAddr;
    XLLP_UINT8_T      altFuncRegValue;
    XLLP_UINT8_T      gpio_dir_encode;
}XLLP_GPIO_SETUP_S, *P_XLLP_GPIO_SETUP_S;

void MonsU2DPlatformSetup( void );
void MonsU2DPlatformDefault( void );

#endif
