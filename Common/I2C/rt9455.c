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

#include "rt9455.h"
#include "GPIO.h"
#include "predefines.h"

void RT9455Write(UINT8_T reg_addr, UINT8_T value)
{
	// Set I2C address for the following i2c writing for RT9455
	*(VUINT_T*)0xD401701c = 0x44;

	BBU_putPI2C(reg_addr, value);
}

UINT8_T RT9455Read(UINT8_T reg_addr)
{
	// Set I2C address for the following i2c writing for RT9455
	*(VUINT_T*)0xD401701c = 0x44;

	return BBU_getPI2C(reg_addr);
}

void EnableRT9455Charger(void)
{
	RT9455Write( RT9455_CONTRL7, RT9455Read(RT9455_CONTRL7) | CHG_EN);
}

void DisableRT9455Charger(void)
{
	RT9455Write( RT9455_CONTRL7, RT9455Read(RT9455_CONTRL7) & (~CHG_EN));
}

void resetTimer(void)
{
	RT9455Write(RT9455_CONTRL1, RT9455Read(RT9455_CONTRL1) |TMR_RST | EN_STAT); // Reset 32s timer.
}

void check_RT9455ID()
{
	UINT8_T ICInfo;
	
	ICInfo = RT9455Read(RT9455_DEVICE_ID); 		// Read the RT9455 Chip information

	serial_outstr("Device ID\n");
	serial_outnum(ICInfo);
	serial_outstr("\n");

}

void ConfigRT9455Charger(UINT8_T bat_connected_status)
{
	UINT_T value;
	UINT8_T reg_val;

	RT9455Write( RT9455_CONTRL6, RT9455Read(RT9455_CONTRL6) | ICHRG_650mA); // default charger limmit 650mA
	RT9455Write( RT9455_CONTRL7, RT9455Read(RT9455_CONTRL7) | VMREG_4V36); // Maximum battery regulation voltage
	RT9455Write( RT9455_CONTRL2, RT9455Read(RT9455_CONTRL2) & (~RT_TE)); // disable charge current termination
	
	if( bat_connected_status ) // The Battery is connected with the charger
	{
		RT9455Write( RT9455_CONTRL1, RT9455Read(RT9455_CONTRL1) |TMR_RST | EN_STAT);	// Reset 32s timer
		RT9455Write( RT9455_CONTRL2, RT9455Read(RT9455_CONTRL2) | IINLIM_500mA | IAICR_INT);	// Iinlim = 500mA
		RT9455Write( RT9455_CONTRL3, OREG_4V35);	// Voreg = 4.35v
	}
	else
	{
		RT9455Write( RT9455_CONTRL2, RT9455Read(RT9455_CONTRL2) | IINLIM_500mA | IAICR_INT);	// Iinlim = 500mA
		RT9455Write( RT9455_CONTRL3, OREG_4V18 );						// Voreg
	}
	
#if 1
	PlatformCI2Init();

	Delay_us(2000);

	value = *(VUINT_T *)USB_PORTSC;
	serial_outstr("USB_PORTSC\n");
	serial_outnum(value);
	serial_outstr("\n");
	if ((value & 0xC00) == 0xC00)
	{
		serial_outstr("Wall Charger\n");
		reg_val = RT9455Read( RT9455_CONTRL2);
		reg_val &= 0x3F;
		reg_val |= IINLIM_1A;
		RT9455Write( RT9455_CONTRL2, reg_val | IAICR_INT);
	}
	else if ((value & 0xC00) == 0x800)
	{
		serial_outstr("USB Charger\n");
		reg_val = RT9455Read( RT9455_CONTRL2); 
		reg_val &= 0x3F;
		reg_val |= IINLIM_500mA;
		RT9455Write( RT9455_CONTRL2, reg_val | IAICR_INT);
	}
	else
		serial_outstr("No Charger\n");

	PlatformCI2Shutdown();
#endif
}

void Charger_Voltage_Set(UINT8_T voltage)
{
	RT9455Write(RT9455_CONTRL3, voltage);

	Delay_us(1000 * 10);
}
 
