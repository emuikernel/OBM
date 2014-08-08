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

#include "rt8973.h"
#include "GPIO.h"
#include "predefines.h"

void RT8973Write(UINT8_T reg_addr, UINT8_T value)
{
	// Set I2C address for the following i2c writing for RT8973
	*(VUINT_T*)0xD401701c = 0x28;

	BBU_putCI2C(reg_addr, value);
}

UINT8_T RT8973Read(UINT8_T reg_addr)
{
	// Set I2C address for the following i2c writing for RT8973
	*(VUINT_T*)0xD401701c = 0x28;

	return BBU_getCI2C(reg_addr);
}

void check_RT8973ID(void)
{
	UINT8_T ICInfo;
	
	ICInfo = RT8973Read(RT8973_REG_CHIP_ID); 		// Read the RT8973 Chip information

	serial_outstr("Device ID\n");
	serial_outnum(ICInfo);
	serial_outstr("\n");
}

UINT8_T check_attached(void)
{
	UINT8_T regIntFlag, regDev1, regADC;
	UINT8_T attachFlag, productionMode = 0;

	regIntFlag = RT8973Read(RT8973_REG_INT_FLAG);
	regDev1 = RT8973Read(RT8973_REG_DEVICE_1);
	serial_outstr("regIntFlag\n");
	serial_outnum(regIntFlag);
	serial_outstr("\n");
	serial_outstr("regDev1\n");
	serial_outnum(regDev1);
	serial_outstr("\n");
	if (regDev1 & RT8973_SDPORT)
	{
		serial_outstr("USB Standard Downstream Port (SDP) charger detected\n");
	}

	attachFlag = 0;
	attachFlag |= RT8973_ATTACH | RT8973_CONNECT | RT8973_ADC_CHG;

	if (regIntFlag & RT8973_CONNECT)
		serial_outstr("Switch is connected\n");
	else
		serial_outstr("Switch is not connected\n");

	if (regIntFlag & attachFlag)
	{
		regADC = RT8973Read(RT8973_REG_ADC);
		serial_outstr("regADC\n");
		serial_outnum(regADC);
		serial_outstr("\n");

		switch (regADC & 0x1f)
		{
			case 0x1d:
				productionMode = 1;
				serial_outstr("Factory Mode : JIG UART ON = 1\n");
				break;

			case 0x1c:
				serial_outstr("Factory Mode : JIG UART OFF = 1\n");
				break;

			case 0x19:
				serial_outstr("Factory Mode : JIG USB ON = 1\n");
				break;

			case 0x18:
				serial_outstr("Factory Mode : JIG USB OFF= 1\n");
				break;

			default:
				break;
		}
	}

	return productionMode;
}


