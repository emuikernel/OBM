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

#include "led.h"

extern UINT8_T battery_connect;

void DisplayBatstate(UINT8_T batpercent)
{
serial_outstr("batpercent\n");
serial_outnum(batpercent);
serial_outstr("\n");

	// if battery percent is lower than 10%, it can't boot up to this
	BatState_10(); // larger than 10%
}

void USB_NotConnect_Display(void)
{
	UINT16_T vbat_mv;

	if (battery_connect == 1)
	{
		GetBatInstantVolt(&vbat_mv, USB_NotConnect);

		DisplayBatstate(GetBatteryPercent(vbat_mv));
	}
}

#define Test_OLEDDisplay_Interval()		Delay_us(1000 * 1000 );

void testLED()
{
	LED_PWR_Blue();		Test_OLEDDisplay_Interval();
	LED_PWR_Red();			Test_OLEDDisplay_Interval();
	LED_PWR_Green();			Test_OLEDDisplay_Interval();
	LED_PWR_Yellow();		Test_OLEDDisplay_Interval();
}


