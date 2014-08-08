/******************************************************************************
 *
 *  (C)Copyright 2005 - 2012 Marvell. All Rights Reserved.
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
#ifndef __CHARGER_H__
#define __CHARGER_H__

#include "Typedef.h"
#include "timer.h"
#include "USB1.h"

#if LED_DISPLAY
#include "led.h"
#endif

#if OLED_SUPPORT
#include "oled.h"
#endif

#if SANREMO
#include "sanremo.h"
#endif

#if USTICA
#include "ustica.h"
#endif

#if FAN540X
#include "fan540x.h"
#endif

#if RT9455
#include "rt9455.h"
#endif

void EnableCharger(void);
void DisableCharger(void);
void ConfigCharger(UINT8_T bat_connected_status);

#endif
