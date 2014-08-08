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

#include "charger.h"
#include "GPIO.h"
#include "predefines.h"

UINT_T charger_status = 0;
extern UINT_T ONKey_pressed;

void EnableCharger(void)
{
	if (charger_status == 0)
	{
	#if FAN540X
		EnableFAN540XCharger();
	#elif RT9455
		EnableRT9455Charger();
	#endif

	#if OLED_SUPPORT || LED_DISPLAY
		if (ONKey_pressed == 0)
			BatCharging_On();
	#endif

		serial_outstr("EnableCharger\n");
		charger_status = 1;
	}
}

void DisableCharger(void)
{
	if (charger_status == 1)
	{
	#if FAN540X
		DisableFAN540XCharger();
	#elif RT9455
		DisableRT9455Charger();
	#endif

		serial_outstr("DisableCharger\n");
		charger_status = 0;
	}
}

void ConfigCharger(UINT8_T bat_connected_status)
{
#if FAN540X
	ConfigFAN540XCharger(bat_connected_status);
#elif RT9455
	ConfigRT9455Charger(bat_connected_status);
#endif
}

