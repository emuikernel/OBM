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

#include "fan540x.h"
#include "GPIO.h"
#include "predefines.h"

void FAN540XWrite(UINT8_T reg_addr, UINT8_T value)
{
	// Set I2C address for the following i2c writing for Fan5405
#if FAN_54013
	*(VUINT_T*)0xD401701c = 0xd6;
#else
	*(VUINT_T*)0xD401701c = 0xd4;
#endif

	BBU_putPI2C(reg_addr, value);
}

UINT8_T FAN540XRead(UINT8_T reg_addr)
{
	// Set I2C address for the following i2c writing for Fan5405
#if FAN_54013
	*(VUINT_T*)0xD401701c = 0xd6;
#else
	*(VUINT_T*)0xD401701c = 0xd4;
#endif

	return BBU_getPI2C(reg_addr);
}

void EnableFAN540XCharger(void)
{
#if MIFI_V2R1
	FAN540XWrite( FAN5405_CONTRL1, FAN540XRead(FAN5405_CONTRL1) & (~FAN_CEn));
#else
	VUINT_T *r;

#if MIFI_V3R0
	FAN540XWrite( FAN5405_CONTRL1, FAN540XRead(FAN5405_CONTRL1) & (~FAN_CEn));
	//r = (VUINT_T *)(GPIO0_BASE + GPIO_PCR); 	// GPIO25(CHG_DISABLE)
	//*r |= (0x01 << 25); // GPIO25 is low level
#else
	r = (VUINT_T *)(GPIO0_BASE + GPIO_PCR); 	// MMC_DAT7 as GPIO24(CHG_DISABLE)
	*r |= (0x01 << 24); // GPIO24 is low level
#endif
#endif
}

void DisableFAN540XCharger(void)
{
#if MIFI_V2R1
	FAN540XWrite( FAN5405_CONTRL1, FAN540XRead(FAN5405_CONTRL1) | FAN_CEn);
#else
    VUINT_T *r;

#if MIFI_V3R0
	FAN540XWrite( FAN5405_CONTRL1, FAN540XRead(FAN5405_CONTRL1) | FAN_CEn);
	//r = (VUINT_T *)(GPIO0_BASE + GPIO_PSR); 	// GPIO25(CHG_DISABLE)
	//*r |= (0x01 << 25); //GPIO25 is high level
#else
	r = (VUINT_T *)(GPIO0_BASE + GPIO_PSR); 	// MMC_DAT7 as GPIO24(CHG_DISABLE)
	*r |= (0x01 << 24); //GPIO24 is high level
#endif
#endif
}

void resetTimer(void)
{
	FAN540XWrite(FAN5405_CONTRL0, TMR_RST_OTG | FAN_EN_STAT); // Reset 32s timer. FairChild
}

void check_Fan540XID()
{
	UINT8_T ICInfo;
	
	ICInfo = FAN540XRead(FAN5405_IC_INFO); 		// Read the FAN5405 Chip information

	serial_outstr("ICInfo\n");
	serial_outnum(ICInfo);
	serial_outstr("\n");
	serial_outstr("FairChild Charge IC vendor ID\n");
	serial_outnum((ICInfo & 0xE0) >> 5);
	serial_outstr("\n");
	serial_outstr("FairChild Charge IC Revision 1.x\n");
	serial_outnum(ICInfo & 0x07);
	serial_outstr("\n");

}

void ConfigFAN540XCharger(UINT8_T bat_connected_status)
{
	UINT_T value;
	UINT8_T reg_val;
	
	FAN540XWrite( FAN5405_SAFETY, IOCHARGE_1250mA | VSAFE_4V20 ); // default charger limmit 1.25A
	FAN540XWrite( FAN5405_CONTRL1, FAN540XRead(FAN5405_CONTRL1) & (~FAN_TE)); // disable charge current termination
	
	if( bat_connected_status ) // The Battery is connected with the charger
	{
		FAN540XWrite( FAN5405_CONTRL0, TMR_RST_OTG | FAN_EN_STAT ); 	// Reset 32s timer
		//FAN540XWrite( FAN5405_SAFETY, IOCHARGE_1250mA | VSAFE_4V20 );
		FAN540XWrite( FAN5405_CONTRL1, FAN540XRead(FAN5405_CONTRL1) | IINLIM_500mA);					// Iinlim = 500mA
		FAN540XWrite( FAN5405_OREG, OREG_4V20 );						// Voreg = 4.18v
		FAN540XWrite( FAN5405_IBAT, IOCHARGE_1050mA );
		FAN540XWrite( FAN5405_SP_CHANGER, VSP_4V533 );					// IOLEVEL = 0
		//FAN540XWrite( FAN5405_IBAT, IOCHARGE_1050mA );						// Iterm = 291mA
		//FAN540XWrite( FAN5405_IBAT, ITERM_49mA );
	}
	else
	{
		//FAN540XWrite( FAN5405_SAFETY, IOCHARGE_550mA | VSAFE_4V20 );
		//FAN540XWrite( FAN5405_CONTRL0, TMR_RST_OTG ); 				// Reset 32s timer
		FAN540XWrite( FAN5405_CONTRL1, FAN540XRead(FAN5405_CONTRL1) | IINLIM_500mA );					// Iinlim is no limit
		FAN540XWrite( FAN5405_OREG, OREG_4V18 );						// Voreg
		FAN540XWrite( FAN5405_SP_CHANGER, VSP_4V533 );					// IOLEVEL = 0
		FAN540XWrite( FAN5405_IBAT, IOCHARGE_550mA );
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
		reg_val = FAN540XRead( FAN5405_CONTRL1);
		reg_val &= 0x3F;
		reg_val |= IINLIM_NOLIM;
		FAN540XWrite( FAN5405_CONTRL1, reg_val );
	}
	else if ((value & 0xC00) == 0x800)
	{
		serial_outstr("USB Charger\n");
		reg_val = FAN540XRead( FAN5405_CONTRL1); 
		reg_val &= 0x3F;
		reg_val |= IINLIM_500mA;
		FAN540XWrite( FAN5405_CONTRL1, reg_val );
	}
	else
		serial_outstr("No Charger\n");

	PlatformCI2Shutdown();
#endif
}

void Charger_Voltage_Set(UINT8_T voltage)
{
	FAN540XWrite( FAN5405_OREG, voltage);

	Delay_us(1000 * 10);
}
 