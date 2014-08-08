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


#include "sanremo.h"

extern UINT_T ONKey_pressed;

void SanremoWrite(UINT8_T reg_addr, UINT8_T value)
{
	// Set I2C address for the following i2c writing for Sanremo
	*(VUINT_T*)0xD401701c = 0x68;

	BBU_putPI2C(reg_addr, value);
}

UINT8_T SanremoRead(UINT8_T reg_addr)
{
	// Set I2C address for the following i2c writing for Sanremo
	*(VUINT_T*)0xD401701c = 0x68;

	return BBU_getPI2C(reg_addr);
}

void check_SanremoID()
{
	UINT8_T value;
	
	value = SanremoRead(SANREMO_ID);
	serial_outstr("value\n");
	serial_outnum(value);
	serial_outstr("\n");

}

UINT16_T sanremo_read_volt_meas_val(UINT8_T meaReg)
{
	UINT16_T meas_val;
	UINT8_T  reg_value[2];
	
	/* Read two registers, the alignment will be done as follows:
	 * Register 1 - bits 7:0 => 8 MSB bits <11:4> of measurement value
	 * Register 2 - bits 3:0 => 4 LSB bits <3:0> of measurement value
	 */
	reg_value[0] = SanremoRead(meaReg);
	reg_value[1] = SanremoRead(meaReg+1);

	meas_val = ((reg_value[0] << 4) | (reg_value[1] & 0x0F));

	return meas_val;
}

void ReadBatVolt(UINT16_T * vbat)
{
	UINT16_T meas_val; 
	UINT_T vol_GPADC, vol_cali;
	UINT16_T vbat_base;

	meas_val = sanremo_read_volt_meas_val(SANREMO_VSYS_MEAS1);	// Read the VSYS(Vbat)

	vol_GPADC = (UINT_T) (meas_val * 5400 >> 12);

	vbat_base = 3700;
	if (vol_GPADC >= vbat_base)
	{
		vol_cali = (UINT_T)(3700 + (vol_GPADC-vbat_base)); 
	}
	else
	{
		vol_cali = (UINT_T)(3700 - (vbat_base-vol_GPADC));
	}

	*vbat = vol_cali;
}

void GetBatInstantVolt(UINT16_T *vbat, System_Connect_Type_T usb_status)
{
	UINT8_T  i;
	UINT16_T vbat_mv[36];
	UINT_T	vbat_sum;

	//if (usb_status == USB_Connect)
	//	DisableCharger();
	
	vbat_sum = 0;
	for( i = 0; i < 36; i ++ )
	{
		ReadBatVolt( &vbat_mv[i] );
		
		Delay_us(100);	

	}

	//if (usb_status == USB_Connect)
	//	EnableCharger();

	bubble(vbat_mv, 36);

	for (i = 2; i < 34; i++)
		vbat_sum += vbat_mv[i];

	if (usb_status == USB_Connect)
		*vbat = (vbat_sum >> 5) - 100;
	else
		*vbat = vbat_sum >> 5;
	serial_outstr("vbat\n");
	serial_outnum(*vbat);
	serial_outstr("\n");
}

System_Connect_Type_T check_USBConnect(void)
{
	UINT8_T val;
	
	val = SanremoRead(SANREMO_STATUS);			// Read Status register2
#if PM8607
	if (val & SANREMO_VBUS_STATUS) // USB is connected
#elif PM8609
	if ((val & SANREMO_EXTON_STATUS) || (val & SANREMO_VBUS_STATUS))
#endif
	{
		serial_outstr("USB is connectted\n");
		return USB_Connect;
	}
	else
	{
		serial_outstr("USB is not connectted\n");
		return USB_NotConnect;
	}
}

System_Connect_Type_T check_BatteryConnect(void)
{
	UINT8_T val;
	
	val = SanremoRead(SANREMO_STATUS);			// Read Status register2
	if (val & SANREMO_BAT_STATUS) 				// Battery is connected
	{
		serial_outstr("Battery is connectted\n");
		return Battery_Connect;
	}
	else
	{
		serial_outstr("Battery is not connectted\n");
		return Battery_NotConnect;
	}
}


PowerUPType_t check_wakeup(UINT8_T bat_state)
{
	UINT8_T value;
	PowerUPType_t type = PowerUP_Unkown;
	
	value = SanremoRead(SANREMO_POWER_UP_LOG);
	SanremoWrite(SANREMO_POWER_UP_LOG, value);
	if (value == 0)
	{
		serial_outstr("Wakeup: Reset\n");
		type = PowerUP_Reset;
	}
	else if (value & SANREMO_BAT_WU_LOG)
	{
		if (bat_state)
		{
			serial_outstr("Wakeup: Battery\n");
			type = PowerUP_Battery;
		}
		else
		{
			/*
			* The external power wakeup signal is connected to battery wakeup signal,
			* such as external 5V power
			*/
			serial_outstr("Wakeup: External Power\n");
			type = PowerUP_External;
		}
	}
	else if (value & SANREMO_ONKEY_WU_LOG)
	{
		serial_outstr("Wakeup: ONKey\n");
		type = PowerUP_ONKEY;
	}
#if PM8607
	else if (value & SANREMO_CHG_WU_LOG) // Bit1 is reserved in PM8609
	{
		serial_outstr("Wakeup: Charger\n");
		type = PowerUP_Charger;
	}
#endif
	else if (value & SANREMO_SYSEN_WU_LOG)
	{
		serial_outstr("Wakeup: SystemE\n");
		type = PowerUP_SystemEnable;
	}
	else if (value & SANREMO_EXTON_WU_LOG)
	{
		serial_outstr("Wakeup: USB\n");
		type = PowerUP_USB;
	}

	return type;
}

UINT8_T check_BootONKey(UINT_T TimeOutValue, UINT8_T WaitGranularity)
{
	UINT8_T val;
	UINT_T startTime, endTime, timeWaited = 0;
	startTime = GetOSCR0();

	do
	{
		val = SanremoRead(SANREMO_STATUS);
		if ((val & SANREMO_ONKEY_STATUS) != SANREMO_ONKEY_STATUS)
		{
			return 0;
		}

		endTime = GetOSCR0();

		if (endTime < startTime)
			endTime += (0x0 - startTime);
		
		if (WaitGranularity == 0)
			timeWaited = OSCR0IntervalInMilli(startTime, endTime);
		else
			timeWaited = OSCR0IntervalInSec(startTime, endTime);
	} while (timeWaited < TimeOutValue );

	serial_outstr("Onkey\n");
	ONKey_pressed = 1;
	return 1;
}

void System_poweroff(void)
{
	UINT8_T value;
	serial_outstr("poweroff---------------\n");

#if OLED_SUPPORT
	Clear_Screen();
#endif

	value = SanremoRead(SANREMO_RESET_OUT);
	SanremoWrite(SANREMO_RESET_OUT, (value | SANREMO_SW_PDOWN));

	serial_outstr("if you see me, PMIC is not poweroff!!!\n");
	while (1){}
}

void Reset_Reg(void)
{
	SanremoWrite(SANREMO_MISCELLANEOUS, SANREMO_RESET_PMIC_REG);
}

void Charger_init(void)
{
	UINT8_T val;

	val = GP1_BIAS_SET_16UA | 0x3; 				// Enable GPADC1 bias , bias current =16uA
	SanremoWrite(SANREMO_GP_BIAS2, val);

	SanremoWrite(SANREMO_GPADC_MISC2, 0x1); 	// Disable GPADC0 bias output

	val = SanremoRead(SANREMO_GPADC_MISC1); 	// Enable ADC measurments
	val |= (SANREMO_GPADC_MISC1_GPFSM_EN);
	SanremoWrite(SANREMO_GPADC_MISC1, val); 	// GPADC is enabled

	val = SanremoRead(SANREMO_MEAS_ENABLE1);
	val = SANREMO_MEAS_EN1_GPADC1; 				// Measurement of Batt_NTC.(GPADC1)
	val |= SANREMO_MEAS_EN1_VCHG; 				// Measurement of VCHG_MEAS.Using for VBUS
	val |= SANREMO_MEAS_EN1_VSYS; 				// Measurement of VSY.Using for VBAT
	SanremoWrite(SANREMO_MEAS_ENABLE1, val); 	// Enable these ADCs measurement

	val = SanremoRead(SANREMO_MEAS_ENABLE3);
	val |= SANREMO_MEAS_EN3_BAT_DET_EN_B0; 		// Enable BAT_DET signal
	SanremoWrite(SANREMO_MEAS_ENABLE3, val);

	val = SanremoRead(SANREMO_CHG_CTRL6);
	val |= BD_MSK_MASK; 								// Both GPADC0/3 is enabled
	SanremoWrite(SANREMO_CHG_CTRL6, val);
}

Voltage_set_main(void)
{
	SanremoWrite(SANREMO_VBUCK1_SET, 0x17); // buck1 1.3V

	SanremoWrite(SANREMO_GO, SanremoRead(SANREMO_GO) | SANREMO_BUCK1_GO);

	while (SanremoRead(SANREMO_GO) & SANREMO_BUCK1_GO); // wait for buck1 set
}


UINT_T Enable_SMPL(void)
{
	// not support?
	return 1;
}

void HandleTempVoltageThreshold(void)
{
	// not support?
	return;
}


