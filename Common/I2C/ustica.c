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

 #include "ustica.h"

#if LED_DISPLAY
#include "led.h"
#endif

#if OLED_SUPPORT
#include "oled.h"
#endif

#define  IBAT_DANAMIC                   /* Ibias danymaic setting.      */

UINT_T power_up_flag = 0xff;

extern UINT_T ONKey_pressed;

int BatteryResistor_NTC_P[90][2] = 
{
	{102115, -28}, 
	{ 96978, -27}, 
	{ 92131, -26}, 
	{ 87559, -25}, 
	{ 83242, -24}, 
	{ 97166, -23}, 
	{ 75316, -22}, 
	{ 71677, -21}, 
	{ 68237, -20}, 
	{ 64991, -19}, 
	{ 61919, -18}, 
	{ 59011, -17}, 
	{ 56258, -16}, 
	{ 53650, -15}, 
	{ 51178, -14}, 
	{ 48835, -13}, 
	{ 46613, -12}, 
	{ 44506, -11}, 
	{ 42506, -10}, 
	{ 40600, -9 }, 
	{ 38790, -8 }, 
	{ 37073, -7 }, 
	{ 35442, -6 }, 
	{ 33892, -5 }, 
	{ 32420, -4 }, 
	{ 31020, -3 }, 
	{ 29689, -2 }, 
	{ 28423, -1 }, 
	{ 27219,  0 }, 
	{ 26076,  1 }, 
	{ 24988,  2 }, 
	{ 23951,  3 }, 
	{ 22963,  4 }, 
	{ 22021,  5 }, 
	{ 21123,  6 }, 
	{ 20267,  7 }, 
	{ 19450,  8 }, 
	{ 18670,  9 }, 
	{ 17925,  10}, 
	{ 17214,  11}, 
	{ 16534,  12}, 
	{ 15885,  13}, 
	{ 15266,  14}, 
	{ 14673,  15}, 
	{ 14107,  16}, 
	{ 13566,  17}, 
	{ 13049,  18}, 
	{ 12554,  19}, 
	{ 12080,  20}, 
	{ 11628,  21}, 
	{ 11195,  22}, 
	{ 10780,  23}, 
	{ 10381,  24}, 
	{ 10000,  25}, 
	{  9634,  26}, 
	{  9283,  27}, 
	{  8947,  28}, 
	{  8624,  29}, 
	{  8314,  30}, 
	{  8018,  31}, 
	{  7733,  32}, 
	{  7460,  33}, 
	{  7199,  34}, 
	{  6947,  35}, 
	{  6706,  36}, 
	{  6475,  37}, 
	{  6252,  38}, 
	{  6039,  39}, 
	{  5833,  40}, 
	{  5635,  41}, 
	{  5445,  42}, 
	{  5262,  43}, 
	{  5086,  44}, 
	{  4916,  45}, 
	{  4753,  46}, 
	{  4597,  47}, 
	{  4446,  48}, 
	{  4300,  49}, 
	{  4160,  50}, 
	{  4026,  51}, 
	{  3896,  52}, 
	{  3771,  53}, 
	{  3651,  54}, 
	{  3535,  55}, 
	{  3423,  56}, 
	{  3315,  57}, 
	{  3211,  58}, 
	{  3111,  59}, 
	{  3014,  60}, 
	{  2922,  61}
};

void UsticaBaseWrite(UINT8_T reg_addr, UINT8_T value)
{
	// Set I2C address for the following i2c writing for Ustica
	*(VUINT_T*)0xD401701c = 0x60;

	BBU_putPI2C(reg_addr, value);
}

UINT8_T UsticaBaseRead(UINT8_T reg_addr)
{
	// Set I2C address for the following i2c writing for Ustica
	*(VUINT_T*)0xD401701c = 0x60;

	return BBU_getPI2C(reg_addr);
}

void UsticaPowerWrite(UINT8_T reg_addr, UINT8_T value)
{
	// Set I2C address for the following i2c writing for Ustica
	*(VUINT_T*)0xD401701c = 0x62;

	BBU_putPI2C(reg_addr, value);
}

UINT8_T UsticaPowerRead(UINT8_T reg_addr)
{
	// Set I2C address for the following i2c writing for Ustica
	*(VUINT_T*)0xD401701c = 0x62;

	return BBU_getPI2C(reg_addr);
}

void UsticaGpadcWrite(UINT8_T reg_addr, UINT8_T value)
{
	// Set I2C address for the following i2c writing for Ustica
	*(VUINT_T*)0xD401701c = 0x64;

	BBU_putPI2C(reg_addr, value);
}

UINT8_T UsticaGpadcRead(UINT8_T reg_addr)
{
	// Set I2C address for the following i2c writing for Ustica
	*(VUINT_T*)0xD401701c = 0x64;

	return BBU_getPI2C(reg_addr);
}

void UsticaTestWrite(UINT8_T reg_addr, UINT8_T value)
{
	// Set I2C address for the following i2c writing for Ustica
	*(VUINT_T*)0xD401701c = 0x6E;

	BBU_putPI2C(reg_addr, value);
}

UINT8_T UsticaTestRead(UINT8_T reg_addr)
{
	// Set I2C address for the following i2c writing for Ustica
	*(VUINT_T*)0xD401701c = 0x6E;

	return BBU_getPI2C(reg_addr);
}


void check_UsticaID()
{
	UINT8_T value;
	
	value = UsticaBaseRead(USTICA_ID);
	serial_outstr("value\n");
	serial_outnum(value);
	serial_outstr("\n");
	if (value == 0x64)
		serial_outstr("Ustica version D0\n");
	else if(value == 0x63)
		serial_outstr("Ustica version C0\n");
	else
		serial_outstr("Ustica version not supported\n");

}

UINT16_T ustica_read_volt_meas_val(UINT8_T meaReg)
{
	UINT16_T meas_val;
	UINT8_T  reg_value[2];
	
	/* Read two registers, the alignment will be done as follows:
	 * Register 1 - bits 7:0 => 8 MSB bits <11:4> of measurement value
	 * Register 2 - bits 3:0 => 4 LSB bits <3:0> of measurement value
	 */
	reg_value[0] = UsticaGpadcRead(meaReg);
	reg_value[1] = UsticaGpadcRead(meaReg+1);

	meas_val = ((reg_value[0] << 4) | (reg_value[1] & 0x0F));

	return meas_val;
}

void ReadBatVolt(UINT16_T * vbat)
{
	UINT16_T meas_val; 
	UINT_T vol_GPADC, vol_cali;
	UINT16_T vbat_base;

	meas_val = ustica_read_volt_meas_val(USTICA_VSYS_MEAS1);	// Read the VSYS(Vbat)

	vol_GPADC = (UINT_T) ((meas_val * 5600) >> 12);

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


#if ZIMI_PB05
void ReadBatTemp(UINT16_T *tmp)
{
		int i;
		UINT16 max, min, tt, at;
		
	for(i = 0; i < 10; i++)
	{
		tt = ustica_read_volt_meas_val(USTICA_GPADC0_MEAS_REG1);
		if(i == 0)
		{
			max = min = at = tt;
		}
		else
		{
			if(tt > max )
			{
				max = tt;
			}
			if(min > tt)
			{
				min = tt;
			}

			at += tt;
		}
	}

	at = (at - min - max) >> 3;

	*tmp = at;
}
#endif
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
	
	val = UsticaBaseRead(USTICA_STATUS);	// Read Status register1
	#if MIFI_V2R1 || MIFI_V3R0 || MIFI_V3R1 || MIFI_V3R2 || MIFI_V3R3
	if ((val & (USTICA_EXTON1_STATUS | USTICA_EXTON2_STATUS)) == (USTICA_EXTON1_STATUS | USTICA_EXTON2_STATUS)) // USB is connected
	{
		serial_outstr("USB is connectted\n");
		return USB_Connect;
	}
	#else
	if ((val & USTICA_EXTON1_STATUS) == USTICA_EXTON1_STATUS) // USB is connected
	{
		serial_outstr("USB is connectted\n");
		return USB_Connect;
	}
	#endif
	else
	{
		serial_outstr("USB is not connectted\n");
		return USB_NotConnect;
	}
}

System_Connect_Type_T check_BatteryConnect(void)
{
	UINT8_T val = 0;

#if MIFI_V3R3
	val = PlatformGetBatteryStatus();
	if (val)
		return Battery_Connect;
	else
		return Battery_NotConnect;
#else
	val = UsticaBaseRead(USTICA_STATUS);	// Read Status register1
	if (val & USTICA_BAT_STATUS)	// Battery is connected
	{
		serial_outstr("Battery is connectted\n");
		return Battery_Connect;
	}
	else
	{
		serial_outstr("Battery is not connectted\n");
		return Battery_NotConnect;
	}
#endif
}

PowerUPType_t check_wakeup(UINT8_T bat_state)
{
	UINT8_T value;
	PowerUPType_t type = PowerUP_Unkown;

	if (power_up_flag == 0x1)
	{
		serial_outstr("Wakeup: Reset\n");
		type = PowerUP_Reset;
		return type;
	}
	
	value = UsticaBaseRead(USTICA_POWER_UP_LOG);
	serial_outstr("wakeup value\n");
	serial_outnum(value);
	serial_outstr("\n");
	if (value == 0)
	{
		serial_outstr("Wakeup: Reset\n");
		type = PowerUP_Reset;
	}
	else if (value & USTICA_BAT_WAKEUP)
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
	else if (value & USTICA_ONKEY_WAKEUP)
	{
		serial_outstr("Wakeup: ONKey\n");
		type = PowerUP_ONKEY;
	}
	else if (value & USTICA_EXTON1_WAKEUP)
	{
	#if MIFI_V2R1 || MIFI_V3R0 || MIFI_V3R1 || MIFI_V3R2 || MIFI_V3R3
		value = UsticaBaseRead(USTICA_STATUS);
		if ((value & (USTICA_EXTON1_STATUS | USTICA_EXTON2_STATUS)) == (USTICA_EXTON1_STATUS | USTICA_EXTON2_STATUS))
		{
			serial_outstr("Wakeup: USB\n");
			type = PowerUP_USB;
		}
		else if ((value & USTICA_EXTON1_STATUS) == USTICA_EXTON1_STATUS)
		{
			serial_outstr("Wakeup: JigBox\n");
			type = PowerUP_JIG;
		}
		else
			serial_outstr("Wakeup: Check HW\n");
	#else
		serial_outstr("Wakeup: USB\n");
		type = PowerUP_USB;
	#endif
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
		val = UsticaBaseRead(USTICA_STATUS);
		if ((val & USTICA_ONKEY_STATUS) != USTICA_ONKEY_STATUS)
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

#if LED_DISPLAY || OLED_SUPPORT
	PowerOnBoot();
#endif

	ONKey_pressed = 1;
	return 1;
}

void System_poweroff(void)
{
	UINT8_T value;
	serial_outstr("poweroff---------------\n");

#if OLED_SUPPORT
	I2COLED_DisplayOff();
#endif

	value = UsticaBaseRead(USTICA_WAKEUP_REG);
	UsticaBaseWrite(USTICA_WAKEUP_REG, (value | USTICA_SW_PDOWN));

	serial_outstr("if you see me, PMIC is not poweroff!!!\n");
	while (1){}
}

void Reset_Reg(void)
{
	UINT8_T value;
	
	UsticaBaseWrite(USTICA_WAKEUP_REG, USTICA_RSET_PMIC_REG);

	value = UsticaBaseRead(USTICA_WATCHDOG_REG);
	if ((value & BIT0) == 0)
	{
		serial_outstr("Cold power up\n");
		UsticaBaseWrite(USTICA_WATCHDOG_REG, USTICA_WD_DIS);
	}
	else
	{
		serial_outstr("Hot power up\n");
		power_up_flag = 0x1;
	}

	UsticaPowerWrite(USTICA_BUCK4_MISC1, (UsticaPowerRead(USTICA_BUCK4_MISC1) | USTICA_BUCK4_PWM));
}

void Charger_init(void)
{
	UINT8_T val, i;

	#if ZIMI_PB05
	val = UsticaGpadcRead(USTICA_MEAS_ENABLE1);
	val |= USTICA_VINLDO_MEAS_EN | USTICA_VBBAT_MEAS_EN;	// Enable VINLDO, VBBAT_MEAS_EN
	UsticaGpadcWrite(USTICA_MEAS_ENABLE1, val);

	val = UsticaGpadcRead(USTICA_MEAS_ENABLE2);
	val = USTICA_INT_TEMP_MEAS_EN				// Measurement of INT_TEMP
		| USTICA_GPADC1_MEAS_EN
		| USTICA_GPADC0_MEAS_EN; 				// Measurement of GPADC1
	UsticaGpadcWrite(USTICA_MEAS_ENABLE2, val); // Enable these ADCs measurement

	UsticaGpadcWrite(USTICA_GPADC_MEAS_REG2, USTICA_BD_PREBIAS | USTICA_BD_EN | USTICA_BD_GP1_EN); // Enable battery detection
	UsticaGpadcWrite(USTICA_GPADC_MEAS_REG3, USTICA_GP_END1 | USTICA_GP_END0);
	
	UsticaGpadcWrite(USTICA_GPADC_BIAS_REG2, 0x3); // GPADC1 bias , bias current =1uA

	UsticaGpadcWrite(USTICA_BIAS_ENABLE1, USTICA_GP_BIAS_OUT1);

	val = UsticaGpadcRead(USTICA_GPADC_MIS_CONG2);
	val |= USTICA_GPADC_EN | 0x02;   //NON_STOP
	UsticaGpadcWrite(USTICA_GPADC_MIS_CONG2, val); 	// GPADC is enabled

	#if MIFI_V3R3
	PlatformConfigBatteryDetectGPIO();
	#endif

	val = 3;   //xyl add to disable BIAS_OUT current
    	UsticaGpadcWrite( USTICA_BIAS_ENABLE1, val );
	#endif	
	
}

Voltage_set_main(void)
{
	UsticaPowerWrite(USTICA_VBUCK1_SET_0, 0x38); // set buck1 to 1.3V

	Delay(1000 * 10);
}

#if MIFI_V3R2 || MIFI_V3R3
UINT8_T check_attached(void)
{
	UINT8_T val, mode = 0;
	
	val = UsticaBaseRead(USTICA_STATUS);
	serial_outstr("USTICA_STATUS\n");
	serial_outnum(val);
	serial_outstr("\n");
	if ((val & (USTICA_EXTON1_STATUS | USTICA_EXTON2_STATUS)) == (USTICA_EXTON1_STATUS | USTICA_EXTON2_STATUS))
	{
		serial_outstr("Charger is connectted\n");
		mode = 0;
	}
	else if ((val & USTICA_EXTON1_STATUS) == USTICA_EXTON1_STATUS)
	{
		serial_outstr("JigBox is connectted\n");
		mode = 1;
	}

	return mode;
}
#endif

// SW workaround for SMPL implementation
UINT_T Enable_SMPL(void)
{
	UINT8_T value, value1, value2;

	//return 1;

	value = UsticaBaseRead(0xD0);
	value |= 0x80;
	UsticaBaseWrite(0xD0, value); //Set USE_XO (bit 7, register 0xD0, base page)

	//if ((UsticaBaseRead(0x10) & 0x60) == 0x1);
	//	serial_outstr("Wakeup by battery insertion\n");

	value = UsticaBaseRead(0xe8);
	value |= 0x70;
	UsticaBaseWrite(0xe8, value); //22pF

	value = UsticaBaseRead(0xE7); //VSYS_OFF_CNT (bits 7:4, register 0xE7, base page)
	value1 = value >> 4;
	serial_outstr("VSYS_OFF_CNT:\n");
	serial_outnum(value1);
	serial_outstr("\n");

	UsticaBaseWrite(0xE7, (value & 0x0F)); //, clear the counter by writing 4b0000

	value2 = UsticaBaseRead(0xE5);
	serial_outstr("Power down log:\n");
	serial_outnum(value2);
	serial_outstr("\n");
	UsticaBaseWrite(0xE5, value2); // clean power down log
	
	if ((value1 > 0x5)|| (value1 == 0x0) ||((value2 | BIT2 | BIT1) == (BIT2 | BIT1)))
	{
		//serial_outstr("power down for normal battery wakeup\n");
		//value = UsticaBaseRead(0x0D);
		//value |= 0x20;
		//UsticaBaseWrite(0x0D, value);   //force a software powerdown 
		return 1;
	}
	else
		return 0;
}

void ustica_read_gpadc1(UINT16_T * tbat, UINT8_T gpadc_ibias )
{
	UINT32_T meas_val = 0;
    UINT8_T     val   = 0;

#ifdef IBAT_DANAMIC
	val = UsticaGpadcRead(USTICA_GPADC_MEAS_EN_REG2);
	val &= ~USTICA_GPADC1_MEAS_EN;
	UsticaGpadcWrite(USTICA_GPADC_MEAS_EN_REG2, val );
    
    /* The current is modified.         */
	val = UsticaGpadcRead(USTICA_GPADC_BIAS_REG2);
    val &= 0xF0;
    val |= gpadc_ibias;
    UsticaGpadcWrite(USTICA_GPADC_BIAS_REG2, val );

	val = UsticaGpadcRead(USTICA_GPADC_MEAS_EN_REG2);      
    val |= USTICA_GPADC1_MEAS_EN;
    UsticaGpadcWrite(USTICA_GPADC_MEAS_EN_REG2, val );
#endif/* IBAT_DANAMIC*/    

    meas_val = ustica_read_volt_meas_val( USTICA_GPADC1_MEAS_REG1 );
    meas_val = ustica_read_volt_meas_val( USTICA_GPADC1_MEAS_REG1 );

    /*unit in mv.                       */
	*tbat = (UINT16_T) (((UINT32_T) meas_val * 1800) >> 12);	
	
}

void Readtbat(UINT16_T *tbat)
{
    UINT8_T i = 0;
    UINT8_T j = 0;
    
    UINT32_T adc_value  = 0;
    UINT32_T adc_max    = 0;
    UINT32_T adc_min    = 0;
    UINT32_T adc_total  = 0;
    UINT32_T adc_ave    = 0;
    
    /* Bias register value.     */
    UINT8_T bias_value[5] = {0x06, 0x0C, 0x03, 0x02, 0x01};

    /* Bias burrent value.      */
    UINT8_T bias_current[5] = {31, 61, 16, 11, 6};

    for( i = 0; i < 10; i++ )
    {
#ifdef IBAT_DANAMIC
        for( j = 0; j < 5; j++ )
        {
            ustica_read_gpadc1( &adc_value, bias_value[j] );

            /* Get right NTC value with different current setting.  */
            if( (adc_value > 300) && (adc_value < 1250 ))
            {
                /* mv -> uv.            */    
                adc_value *= 1000; 
                /* uv -> Ohm.           */
                adc_value /= bias_current[j];
                
                break;
            }
        }
#else
        ustica_read_gpadc1( &adc_value, bias_value[j] );
        
        /* mv -> uv.            */    
        adc_value *= 1000; 
        /* uv -> Ohm.           */
        adc_value /= 16;
        
#endif /* IBAT_DANAMIC  */
        if( i != 0 )
        {
            if( adc_value > adc_max )
            {
                adc_max = adc_value;
            }
            else if( adc_value > adc_min )
            {
                adc_min = adc_value;
            }
        }
        else
        {
            adc_max = adc_value;
            adc_min = adc_value;
        }

        adc_total += adc_value;
        
    }

    adc_ave = (adc_total - adc_min - adc_max) >> 3;

    *tbat = adc_ave;
}

int GetBatInstantTemp(void)
{
	UINT16_T tbat_ohm;
	UINT8_T	i;
	UINT32_T	temp1, temp2;
	int Temperature = 0;

	Readtbat(&tbat_ohm );

	//Turn into Temperature
	if( tbat_ohm >= BatteryResistor_NTC_P[0][0] )
	{
		Temperature = -28;
	}
	else if( tbat_ohm <= BatteryResistor_NTC_P[89][0] )
	{
		Temperature = 61;
	}
	else
	{
		for(i=0;i<89;i++)
		{
			temp1 = BatteryResistor_NTC_P[i][0];
			temp2 = BatteryResistor_NTC_P[i+1][0];
			if( (tbat_ohm<temp1) && (tbat_ohm>=temp2) )
			{
				Temperature = BatteryResistor_NTC_P[i+1][1];
				
                if( Temperature <= -10 )
                {
                    Temperature -= 12;
                }
                else
                {
                    Temperature -= 15;
                }
			}
		}
	}
	return Temperature;
}

int GetBatTemp(void)
{
	UINT8_T  i;
	int temp, temp_sum = 0, Temperature;
	int temp_min;
	int temp_max;

	for (i = 0; i < 10; i++)
	{
		temp = GetBatInstantTemp();

		if (i == 0)
		{
			temp_min = temp;
			temp_max = temp;
		}

		if (temp < temp_min)
			temp_min = temp;
		else if (temp > temp_max)
			temp_max = temp;

		temp_sum += temp;
	}

	temp_sum = temp_sum - temp_max - temp_min;

	Temperature = temp_sum >> 3;
}

void HandleTempVoltageThreshold(void)
{
	int tbat_mv;

	tbat_mv = GetBatTemp();
	serial_outstr("Temperature\n");
	serial_outnum(tbat_mv);
	serial_outstr("\n");
	if (tbat_mv >= TBAT_TEM_P_60) // > 60C
	{
		DisableCharger();
	}
	else if (tbat_mv >= TBAT_TEM_P_40) // > 40C
	{
		// nothing to do
	}
	else if (tbat_mv >= TBAT_TEM_P_0) // > 0C
	{
		EnableCharger();
	}
	else if (tbat_mv >= TBAT_TEM_N_5) // > -5C
	{
		// nothing to do
	}
	else // < -5C
	{
		DisableCharger();
	}
}

